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

namespace PresetFlame
{
#include "presetFlame.h"
}

#include "scene.h"

#include <SDL.h>

void SceneEmitSubStep::initParams()
{
	m_flowGridActor.initParams(AppGraphCtxDedicatedVideoMemory(m_appctx));

	// set emitter defaults
	NvFlowGridEmitParamsDefaults(&m_emitParams);

	// configure emitter params
	m_emitParams.bounds.x.x = 0.25f;
	m_emitParams.bounds.y.y = 0.25f;
	m_emitParams.bounds.z.z = 0.25f;
	m_emitParams.velocityLinear.y = 8.f;
	m_emitParams.fuel = 1.9f;
	m_emitParams.smoke = 0.5f;

	m_shouldLoadPreset = true;
}

void SceneEmitSubStep::init(AppGraphCtx* appctx, int winw, int winh)
{
	m_appctx = appctx;

	if (!m_shouldReset || m_isFirstRun)
	{
		initParams();
		m_isFirstRun = false;
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

void SceneEmitSubStep::emitImpulse(float x, float impulse_dt)
{
	NvFlowShapeDesc shapeDesc;
	shapeDesc.sphere.radius = 0.8f;

	m_emitParams.bounds.w.x = x;

	m_emitParams.localToWorld = m_emitParams.bounds;
	m_emitParams.shapeType = eNvFlowShapeTypeSphere;
	m_emitParams.deltaTime = impulse_dt;

	NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);
}

void SceneEmitSubStep::emitSubSteps(float t_old, float x_old, float t_new, float x_new, float frame_dt)
{
	float emitImpulse_dt = m_emitterTimeStepper.m_fixedDt;

	int numSteps = m_emitterTimeStepper.getNumSteps(frame_dt);

	for (int i = 0; i < numSteps; i++)
	{
		int substep_i = numSteps - 1 - i;

		float substep_t = emitImpulse_dt * substep_i + m_emitterTimeStepper.m_timeError;

		float s = (substep_t - t_new) / (t_old - t_new);
		float x = (1.f - s) * x_new + s * x_old;

		emitImpulse(x, emitImpulse_dt);
	}
}

void SceneEmitSubStep::doFrameUpdate(float frame_dt)
{
	m_emitterTimeStepper.m_fixedDt = 1.f / 1200.f;
	m_emitterTimeStepper.m_maxSteps = 1000;

	anim_x_old = anim_x;

	anim_t += frame_dt;

	const float freq = 60.f;
	const float k = 2.f * freq;
	const float k_dt = 1.f / freq;

	if (anim_t > (2.f * k_dt))
	{
		anim_t -= (2.f * k_dt);
	}

	anim_x = k * fabs(anim_t - k_dt) - 1.f;

	float t_old = frame_dt;
	float t_new = 0.f;

	emitSubSteps(t_old, anim_x_old, t_new, anim_x, frame_dt);
}

void SceneEmitSubStep::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin(dt);
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		m_flowGridActor.updatePreEmit(&m_flowContext, dt);

		// emit
		{
			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);
		}

		m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

		m_shouldGridReset = false;

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneEmitSubStep::preDraw()
{
	m_flowContext.preDrawBegin();

	m_flowGridActor.preDraw(&m_flowContext);

	m_flowContext.preDrawEnd();
}

void SceneEmitSubStep::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_projectile.draw(projection, view);

	m_flowContext.drawBegin();

	m_flowGridActor.draw(&m_flowContext, projection, view);

	m_flowContext.drawEnd();
}

void SceneEmitSubStep::release()
{
	m_projectile.release();

	m_flowGridActor.release();

	m_flowContext.release();
}

void SceneEmitSubStep::imgui(int xIn, int yIn, int wIn, int hIn)
{
	SceneFluid::imgui(xIn, yIn, wIn, hIn);

	if (m_shouldLoadPreset)
	{
		imguiserLoadC(PresetFlame::g_root, sizeof(PresetFlame::g_root));
		m_shouldLoadPreset = false;
	}
}

void SceneEmitSubStep::imguiFluidEmitterExtra()
{

}