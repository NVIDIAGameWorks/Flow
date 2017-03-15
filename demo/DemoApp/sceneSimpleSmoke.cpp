/*
* Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include <stdio.h>
#include <string.h>

#include "loader.h"
#include "imgui.h"
#include "imguiser.h"

namespace PresetSmoke
{
#include "presetSmoke.h"
}
namespace PresetFireBall
{
#include "PresetFireBall.h"
}

#include "scene.h"

#include <SDL.h>

void SceneSimpleSmoke::initParams()
{
	m_flowGridActor.initParams(AppGraphCtxDedicatedVideoMemory(m_appctx));

	// set emitter defaults
	NvFlowGridEmitParamsDefaults(&m_emitParams);

	m_emitParams.bounds.x.x = 0.25f;
	m_emitParams.bounds.y.y = 0.25f;
	m_emitParams.bounds.z.z = 0.25f;
	m_emitParams.velocityLinear.y = 8.f;
	m_emitParams.fuel = 1.9f;
	m_emitParams.smoke = 0.5f;

	m_emitParams.allocationPredict = 0.3f;

	// grid actor overrides
	m_flowGridActor.m_renderParams.renderMode = eNvFlowVolumeRenderMode_colormap;
	m_flowGridActor.m_renderMaterialDefaultParams.intensityCompMask = { 0.f, 0.f, 1.f, 0.f };
	m_flowGridActor.m_renderMaterialDefaultParams.intensityBias = 0.f;
	m_flowGridActor.m_gridParams.gravity = NvFlowFloat3{ 0.f, -1.f, 0.f };

	m_flowGridActor.m_enableVolumeShadow = true;

	m_shouldLoadPreset = true;
}

void SceneSimpleSmoke::init(AppGraphCtx* appctx, int winw, int winh)
{
	m_appctx = appctx;

	if (!m_shouldReset || m_isFirstRun)
	{
		initParams();
		m_isFirstRun = false;
	}

	if (m_flowGridActor.m_enableVolumeShadow)
	{
		m_flowGridActor.m_renderMaterialDefaultParams.intensityCompMask = { 0.f, 0.f, 1.f, 0.f };
		m_flowGridActor.m_renderMaterialDefaultParams.intensityBias = 0.f;
	}
	else
	{
		m_flowGridActor.m_renderMaterialDefaultParams.intensityCompMask = { 0.f, 0.f, 0.f, 0.f };
		m_flowGridActor.m_renderMaterialDefaultParams.intensityBias = 1.f;
	}

	m_flowContext.init(appctx);

	m_flowGridActor.init(&m_flowContext, appctx);

	// create default color map
	{
		const int numPoints = 5;
		const CurvePoint pts[numPoints] = {
			{ 0.f, 0.f,0.f,0.f,0.f },
			{ 0.05f, 0.f,0.f,0.f,0.5f },
			{ 0.6f, 213.f / 255.f,100.f / 255.f,30.f / 255.f,0.8f },
			{ 0.85f, 255.f / 255.f,240.f / 255.f,0.f,0.8f },
			{ 1.f, 1.f,1.f,1.f,0.7f }
		};

		auto& colorMap = m_flowGridActor.m_colorMap;
		colorMap.initColorMap(m_flowContext.m_renderContext, pts, numPoints, (colorMap.m_curvePointsDefault.size() == 0));
	}

	m_projectile.init(m_appctx, m_flowContext.m_gridContext);

	resize(winw, winh);
}

NvFlowFloat3 SceneSimpleSmoke::positionFunc(float theta)
{
	return NvFlowFloat3{
		3.f * cosf(theta),
		0.f,
		3.f * sinf(theta)
	};
}

NvFlowFloat3 SceneSimpleSmoke::velocityFunc(float theta, float rate)
{
	return NvFlowFloat3{
		-3.f * rate * sinf(theta),
		0.f,
		+3.f * rate * cosf(theta)
	};
}

void SceneSimpleSmoke::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin();
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		m_flowGridActor.updatePreEmit(&m_flowContext, dt);

		NvFlowShapeDesc shapeDesc;
		shapeDesc.sphere.radius = 0.8f;

		if (m_animate)
		{
			const float pi2 = 2.f * 3.14159265f;
			const float emitterScale = 1.f;

			theta += pi2 * rate * dt;

			if (theta > pi2) theta -= pi2;

			// get position and velocity
			NvFlowFloat3 position = positionFunc(theta);
			NvFlowFloat3 velocity = velocityFunc(theta, pi2 * rate);

			// update emitter bounds
			m_emitParams.bounds.x.x = 0.25f * emitterScale;
			m_emitParams.bounds.y.y = 0.25f * emitterScale;
			m_emitParams.bounds.z.z = 0.25f * emitterScale;

			// modulate couple rate as a function of velocity and emitter width
			const float defaultCoupleRate = 0.5f;

			float emitWidth = m_emitParams.bounds.x.x;

			float velMagn = sqrtf(velocity.x*velocity.x + velocity.y*velocity.y + velocity.z*velocity.z);
			float coupleRate = fmaxf(coupleRateScale * fabsf(velMagn) / emitWidth, defaultCoupleRate);

			m_emitParams.smokeCoupleRate = coupleRate;
			m_emitParams.temperatureCoupleRate = coupleRate;
			m_emitParams.fuelCoupleRate = coupleRate;
			m_emitParams.velocityCoupleRate = { coupleRate, coupleRate, coupleRate };

			m_emitParams.bounds.w.x = position.x;
			m_emitParams.bounds.w.y = position.y;
			m_emitParams.bounds.w.z = position.z;
			m_emitParams.velocityLinear.x = velocity.x;
			//m_emitParams.velocityLinear.y = velocity.y;
			m_emitParams.velocityLinear.z = velocity.z;

			m_emitParams.predictVelocityWeight = 1.f;
			m_emitParams.predictVelocity.x = velocity.x;
			//m_emitParams.predictVelocity.y = velocity.y;
			m_emitParams.predictVelocity.z = velocity.z;
		}
		else if (m_animate != m_animateOld)
		{
			float coupleRate = 0.5f;

			m_emitParams.smokeCoupleRate = coupleRate;
			m_emitParams.temperatureCoupleRate = coupleRate;
			m_emitParams.fuelCoupleRate = coupleRate;
			m_emitParams.velocityCoupleRate = { coupleRate, coupleRate, coupleRate };

			m_emitParams.bounds.w.x = 0.f;
			m_emitParams.velocityLinear.x = 0.f;

			m_emitParams.predictVelocityWeight = 0.f;
			m_emitParams.predictVelocity.x = 0.f;
			m_emitParams.predictVelocity.y = 0.f;
			m_emitParams.predictVelocity.z = 0.f;
		}
		m_animateOld = m_animate;

		m_emitParams.localToWorld = m_emitParams.bounds;
		m_emitParams.shapeType = eNvFlowShapeTypeSphere;
		m_emitParams.deltaTime = dt;

		NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);

		m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);

		m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

		m_shouldGridReset = false;

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSimpleSmoke::preDraw()
{
	m_flowContext.preDrawBegin();

	m_flowGridActor.preDraw(&m_flowContext);

	m_flowContext.preDrawEnd();
}

void SceneSimpleSmoke::imguiFluidEmitterExtra()
{
	if (imguiserCheck("Animate", m_animate, true))
	{
		m_animate = !m_animate;
	}
}

void SceneSimpleSmoke::imguiFluidRenderExtra()
{
}

void SceneSimpleSmoke::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_projectile.draw(projection, view);

	m_flowContext.drawBegin();

	m_flowGridActor.draw(&m_flowContext, projection, view);

	m_flowContext.drawEnd();
}

void SceneSimpleSmoke::release()
{
	m_projectile.release();

	m_flowGridActor.release();

	m_flowContext.release();
}

void SceneSimpleSmoke::imgui(int xIn, int yIn, int wIn, int hIn)
{
	SceneFluid::imgui(xIn, yIn, wIn, hIn);

	if (m_shouldLoadPreset)
	{
		imguiserLoadC(PresetSmoke::g_root, sizeof(PresetSmoke::g_root));
		m_shouldLoadPreset = false;
	}
}

// ******************************* SceneSimpleFlameBall *************************

void SceneSimpleFlameBall::initParams()
{
	SceneSimpleFlame::initParams();

	m_flowGridActor.m_gridDesc.lowLatencyMapping = true;
	m_flowGridActor.m_memoryLimit = 4.f;

	m_flowGridActor.m_materialParams.smoke.allocWeight = 1.f;
	m_flowGridActor.m_materialParams.smoke.allocThreshold = 0.02f;

	m_flowGridActor.m_gridParams.gravity = { 0.f, -0.5f, 0.f };

	m_flowGridActor.m_gridParams.bigEffectMode = true;

	m_flowGridActor.m_materialParams.smoke.damping = 0.3f;
	m_flowGridActor.m_materialParams.smoke.fade = 0.65f;
	m_flowGridActor.m_materialParams.vorticityStrength = 20.f;

	m_emitParams.fuel = 2.0f;
	m_emitParams.temperature = 3.f;
	m_emitParams.smoke = 0.5f;

	m_emitParams.bounds.x.x = m_radius;
	m_emitParams.bounds.y.y = m_radius;
	m_emitParams.bounds.z.z = m_radius;

	m_emitParams.bounds.w.y = -2.f;

	m_emitParams.allocationScale = { 1.f, 1.f, 1.f };
	m_emitParams.allocationPredict = 0.05f;

	// grid actor overrides
	m_flowGridActor.m_renderParams.renderMode = eNvFlowVolumeRenderMode_colormap;
	m_flowGridActor.m_renderMaterialDefaultParams.intensityCompMask = { 0.f, 0.f, 1.f, 0.f };
	m_flowGridActor.m_renderMaterialDefaultParams.intensityBias = 0.f;
	
	m_flowGridActor.m_shadowBlendCompMask = { -2.f, 0.f, 0.f, 0.f };
	m_flowGridActor.m_shadowBlendBias = 1.f;

	m_flowGridActor.m_enableVolumeShadow = true;

	m_shouldLoadPreset = true;
}

void SceneSimpleFlameBall::init(AppGraphCtx* context, int winw, int winh)
{
	SceneSimpleFlame::init(context, winw, winh);

	if (m_flowGridActor.m_enableVolumeShadow)
	{
		m_flowGridActor.m_renderMaterialDefaultParams.intensityCompMask = { 0.f, 0.f, 1.f, 0.f };
		m_flowGridActor.m_renderMaterialDefaultParams.intensityBias = 0.f;
	}
	else
	{
		m_flowGridActor.m_renderMaterialDefaultParams.intensityCompMask = { 0.f, 0.f, 0.f, 0.f };
		m_flowGridActor.m_renderMaterialDefaultParams.intensityBias = 1.f;
	}
}

void SceneSimpleFlameBall::doUpdate(float dt)
{
	time += dt;

	m_emitParams.bounds.x.x = m_radius;
	m_emitParams.bounds.y.y = m_radius;
	m_emitParams.bounds.z.z = m_radius;

	if (time < 0.25f)
	{
		m_emitParams.fuelCoupleRate = 0.f;
		m_emitParams.velocityCoupleRate = { 0.f, 0.f, 0.f };
		m_emitParams.smokeCoupleRate = 0.f;
		m_emitParams.temperatureCoupleRate = 0.f;
		//m_emitParams.allocationScale = { 1.f, 1.f, 1.f };
	}
	else if (time < 0.75f)
	{
		const float coupleRate = 5.f;
		m_emitParams.fuelCoupleRate = coupleRate;
		m_emitParams.velocityCoupleRate = { coupleRate, coupleRate, coupleRate };
		m_emitParams.smokeCoupleRate = coupleRate;
		m_emitParams.temperatureCoupleRate = coupleRate;
		//m_emitParams.allocationScale = { 1.f, 1.f, 1.f };
	}
	else if (time < 4.f)
	{
		m_emitParams.fuelCoupleRate = 0.f;
		m_emitParams.velocityCoupleRate = { 0.f, 0.f, 0.f };
		m_emitParams.smokeCoupleRate = 0.f;
		m_emitParams.temperatureCoupleRate = 0.f;
		//m_emitParams.allocationScale = { 0.f, 0.f, 0.f };
	}
	else
	{
		time = 0.f;
	}

	SceneSimpleFlame::doUpdate(dt);
}

void SceneSimpleFlameBall::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	SceneSimpleFlame::draw(projection, view);
}

void SceneSimpleFlameBall::release()
{
	SceneSimpleFlame::release();
}

void SceneSimpleFlameBall::imguiFluidEmitterExtra()
{
	imguiserSlider("Radius", &m_radius, 0.1f, 2.f, 0.05f, true);
}

void SceneSimpleFlameBall::imguiFluidRenderExtra()
{
}

void SceneSimpleFlameBall::imguiFluidSimExtra()
{
}

void SceneSimpleFlameBall::imgui(int xIn, int yIn, int wIn, int hIn)
{
	SceneFluid::imgui(xIn, yIn, wIn, hIn);

	if (m_shouldLoadPreset)
	{
		imguiserLoadC(PresetFireBall::g_root, sizeof(PresetFireBall::g_root));
		m_shouldLoadPreset = false;
	}
}