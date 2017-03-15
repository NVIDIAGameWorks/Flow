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

void SceneSimpleFlame::initParams()
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

void SceneSimpleFlame::init(AppGraphCtx* appctx, int winw, int winh)
{
	m_appctx = appctx;

	if (!m_shouldReset || m_isFirstRun)
	{
		initParams();
		m_isFirstRun = false;
	}

	m_flowContext.init(appctx);

	m_flowGridActor.init(&m_flowContext, appctx);

	// more compute resources
	NvFlowShapeSDFDesc shapeDesc;
	NvFlowShapeSDFDescDefaults(&shapeDesc);
	m_shape = NvFlowCreateShapeSDF(m_flowContext.m_gridContext, &shapeDesc);

	// generate sphere SDF
	const float radius = 0.8f;
	auto mappedData = NvFlowShapeSDFMap(m_shape, m_flowContext.m_gridContext);
	if (mappedData.data)
	{
		for (NvFlowUint k = 0; k < mappedData.dim.z; k++)
			for (NvFlowUint j = 0; j < mappedData.dim.y; j++)
				for (NvFlowUint i = 0; i < mappedData.dim.x; i++)
				{
					float& val = mappedData.data[k * mappedData.depthPitch + j * mappedData.rowPitch + i];

					float x = 2.f * (float(i) + 0.5f) / float(mappedData.dim.x) - 1.f;
					float y = 2.f * (float(j) + 0.5f) / float(mappedData.dim.y) - 1.f;
					float z = 2.f * (float(k) + 0.5f) / float(mappedData.dim.z) - 1.f;

					float d = sqrtf(x*x + y*y + z*z);
					float v = d - radius;

					val = v;
				}
		NvFlowShapeSDFUnmap(m_shape, m_flowContext.m_gridContext);
	}

	// create default color map
	{
		const int numPoints = 5;
		const CurvePoint pts[numPoints] = {
			{0.f, 0.f,0.f,0.f,0.f},
			{0.05f, 0.f,0.f,0.f,0.5f},
			{0.6f, 213.f / 255.f,100.f / 255.f,30.f / 255.f,0.8f},
			{0.85f, 255.f / 255.f,240.f / 255.f,0.f,0.8f},
			{1.f, 1.f,1.f,1.f,0.7f}
		};

		auto& colorMap = m_flowGridActor.m_colorMap;
		colorMap.initColorMap(m_flowContext.m_renderContext, pts, numPoints, (colorMap.m_curvePointsDefault.size() == 0));
	}

	m_projectile.init(m_appctx, m_flowContext.m_gridContext);

	resize(winw, winh);
}

void SceneSimpleFlame::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin();
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		m_flowGridActor.updatePreEmit(&m_flowContext, dt);

		// emit
		{
			NvFlowShapeDesc shapeDesc;
			shapeDesc.sphere.radius = 0.8f;

			m_emitParams.localToWorld = m_emitParams.bounds;
			m_emitParams.shapeType = eNvFlowShapeTypeSphere;
			m_emitParams.deltaTime = dt;

			NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);
		}

		m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

		m_shouldGridReset = false;

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSimpleFlame::preDraw()
{
	m_flowContext.preDrawBegin();

	m_flowGridActor.preDraw(&m_flowContext);

	m_flowContext.preDrawEnd();
}

void SceneSimpleFlame::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_projectile.draw(projection, view);

	m_flowContext.drawBegin();

	m_flowGridActor.draw(&m_flowContext, projection, view);

	m_flowContext.drawEnd();
}

void SceneSimpleFlame::release()
{
	m_projectile.release();

	m_flowGridActor.release();

	NvFlowReleaseShapeSDF(m_shape);

	m_flowContext.release();
}

void SceneSimpleFlame::imgui(int xIn, int yIn, int wIn, int hIn)
{
	SceneFluid::imgui(xIn, yIn, wIn, hIn);

	if (m_shouldLoadPreset)
	{
		imguiserLoadC(PresetFlame::g_root, sizeof(PresetFlame::g_root));
		m_shouldLoadPreset = false;
	}
}

// *************************** SceneSimpleFlameDouble **********************

void SceneSimpleFlameDouble::init(AppGraphCtx* context, int winw, int winh)
{
	SceneSimpleFlame::init(context, winw, winh);

	NvFlowGridMaterialParams materialParams = {};
	NvFlowGridMaterialParamsDefaults(&materialParams);

	m_materialA = NvFlowGridCreateMaterial(m_flowGridActor.m_grid, &materialParams);

	materialParams.vorticityStrength = 5.f;
	materialParams.vorticityVelocityMask = 0.f;
	materialParams.velocity.macCormackBlendFactor = 0.f;
	materialParams.smoke.macCormackBlendFactor = 0.75f;
	materialParams.buoyancyPerTemp *= 5.f;

	m_materialB = NvFlowGridCreateMaterial(m_flowGridActor.m_grid, &materialParams);

	m_flowGridActor.m_renderMaterialMat0Params.material = m_materialA;
	m_flowGridActor.m_renderMaterialMat1Params.material = m_materialB;
}

void SceneSimpleFlameDouble::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin();
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		m_flowGridActor.updatePreEmit(&m_flowContext, dt);

		// emit
		{
			NvFlowShapeDesc shapeDesc;
			shapeDesc.sphere.radius = 0.8f;

			m_emitParams.localToWorld = m_emitParams.bounds;
			m_emitParams.shapeType = eNvFlowShapeTypeSphere;
			m_emitParams.deltaTime = dt;

			m_emitParamsA = m_emitParams;
			m_emitParamsA.material = m_materialA;
			m_emitParamsA.bounds.w.x = +0.25f;
			m_emitParamsA.localToWorld = m_emitParamsA.bounds;
			m_emitParamsA.velocityLinear.x = -8.f;
			NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParamsA, 1u);

			m_emitParamsB = m_emitParams;
			m_emitParamsB.material = m_materialB;
			m_emitParamsB.bounds.w.x = -0.25f;
			m_emitParamsB.localToWorld = m_emitParamsB.bounds;
			m_emitParamsB.velocityLinear.x = +8.f;
			NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParamsB, 1u);

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);
		}

		m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

		m_shouldGridReset = false;

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSimpleFlameDouble::preDraw()
{
	SceneSimpleFlame::preDraw();
}

void SceneSimpleFlameDouble::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	SceneSimpleFlame::draw(projection, view);
}

void SceneSimpleFlameDouble::release()
{
	SceneSimpleFlame::release();
}

void SceneSimpleFlameDouble::imgui(int x, int y, int w, int h)
{
	SceneSimpleFlame::imgui(x, y, w, h);
}

void SceneSimpleFlameDouble::initParams()
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

// *************************** SceneSimpleFlameFuelMap **********************

void SceneSimpleFlameFuelMap::init(AppGraphCtx* context, int winw, int winh)
{
	SceneSimpleFlame::init(context, winw, winh);

	m_flowGridActor.m_renderMaterialMat0Params.material = NvFlowGridGetDefaultMaterial(m_flowGridActor.m_grid);

	m_flowGridActor.m_renderMaterialMat0Params.colorMapCompMask = { 0.f, 4.f, 0.f, 0.f };
	m_flowGridActor.m_renderMaterialMat0Params.alphaCompMask = { 0.f, 1.f, 0.f, 0.f };
	m_flowGridActor.m_renderMaterialMat0Params.alphaBias = 0.f;

	//m_flowGridActor.m_renderMaterialMat1Params.material = m_materialB;
}

void SceneSimpleFlameFuelMap::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin();
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		m_flowGridActor.updatePreEmit(&m_flowContext, dt);

		// emit
		{
			NvFlowShapeDesc shapeDesc;
			shapeDesc.sphere.radius = 0.8f;

			m_emitParams.localToWorld = m_emitParams.bounds;
			m_emitParams.shapeType = eNvFlowShapeTypeSphere;
			m_emitParams.deltaTime = dt;

			NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);
		}

		m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

		m_shouldGridReset = false;

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSimpleFlameFuelMap::preDraw()
{
	SceneSimpleFlame::preDraw();
}

void SceneSimpleFlameFuelMap::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	SceneSimpleFlame::draw(projection, view);
}

void SceneSimpleFlameFuelMap::release()
{
	SceneSimpleFlame::release();
}

void SceneSimpleFlameFuelMap::imgui(int x, int y, int w, int h)
{
	SceneSimpleFlame::imgui(x, y, w, h);
}

void SceneSimpleFlameFuelMap::initParams()
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

// *************************** SceneSimpleFlameParticleSurface **********************

void SceneSimpleFlameParticleSurface::init(AppGraphCtx* context, int winw, int winh)
{
	SceneSimpleFlame::init(context, winw, winh);

	NvFlowParticleSurfaceDesc surfaceDesc = {};
	surfaceDesc.initialLocation = { 0.f, 0.f, 0.f };
	surfaceDesc.halfSize = { 8.f, 8.f, 8.f };
	surfaceDesc.virtualDim = NvFlowDim{512u, 512u, 512u};
	surfaceDesc.residentScale = 0.125f * 0.125f;
	surfaceDesc.maxParticles = 64u * 1024u;	

	m_particleSurface = NvFlowCreateParticleSurface(m_flowContext.m_gridContext, &surfaceDesc);

	if (!m_visualizeSurface)
	{
		NvFlowGridEmitCustomRegisterAllocFunc(m_flowGridActor.m_grid, emitCustomAllocFunc, this);
		NvFlowGridEmitCustomRegisterEmitFunc(m_flowGridActor.m_grid, eNvFlowGridTextureChannelVelocity, emitCustomEmitVelocityFunc, this);
		NvFlowGridEmitCustomRegisterEmitFunc(m_flowGridActor.m_grid, eNvFlowGridTextureChannelDensity, emitCustomEmitDensityFunc, this);
	}

	// generate positions
	const NvFlowUint r = 32u;

	m_positions.clear();
	m_positions.resize(4u * r * r);
	const float scale = 4.f;
	const float scaleXInv = scale / float(r);
	const float scaleYInv = scale / float(r);
	for (NvFlowUint j = 0u; j < r; j++)
	{
		for (NvFlowUint i = 0u; i < r; i++)
		{
			float x = scaleXInv * float(i) - scale * 0.5f;
			float y = scaleYInv * float(j) - scale * 0.5f;

			m_positions[4 * (j * r + i) + 0u] = x;
			m_positions[4 * (j * r + i) + 1u] = 0.f;
			m_positions[4 * (j * r + i) + 2u] = y;
			m_positions[4 * (j * r + i) + 3u] = 1.f;
		}
	}

	m_flowGridActor.m_renderMaterialDefaultParams.material = NvFlowGridMaterialHandle{ nullptr, 1u };
	m_flowGridActor.m_renderMaterialMat0Params.material = NvFlowGridGetDefaultMaterial(m_flowGridActor.m_grid);

	m_particleParams.smoothRadius = 16.f;
	m_particleParams.surfaceThreshold = 0.001f;
	m_particleParams.separableSmoothing = true;
}

void SceneSimpleFlameParticleSurface::emitCustomAllocFunc(void* userdata, const NvFlowGridEmitCustomAllocParams* params)
{
	((SceneSimpleFlameParticleSurface*)(userdata))->doEmitCustomAllocFunc(params);
}

void SceneSimpleFlameParticleSurface::emitCustomEmitVelocityFunc(void* userdata, NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params)
{
	((SceneSimpleFlameParticleSurface*)(userdata))->doEmitCustomEmitVelocityFunc(dataFrontIdx, params);
}

void SceneSimpleFlameParticleSurface::emitCustomEmitDensityFunc(void* userdata, NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params)
{
	((SceneSimpleFlameParticleSurface*)(userdata))->doEmitCustomEmitDensityFunc(dataFrontIdx, params);
}

void SceneSimpleFlameParticleSurface::doEmitCustomAllocFunc(const NvFlowGridEmitCustomAllocParams* params)
{
	NvFlowParticleSurfaceAllocFunc(m_particleSurface, m_flowContext.m_gridContext, params);
}

void SceneSimpleFlameParticleSurface::doEmitCustomEmitVelocityFunc(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params)
{
	NvFlowParticleSurfaceEmitVelocityFunc(m_particleSurface, m_flowContext.m_gridContext, dataFrontIdx, params, &m_surfaceEmitParams);
}

void SceneSimpleFlameParticleSurface::doEmitCustomEmitDensityFunc(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params)
{
	NvFlowParticleSurfaceEmitDensityFunc(m_particleSurface, m_flowContext.m_gridContext, dataFrontIdx, params, &m_surfaceEmitParams);
}

void SceneSimpleFlameParticleSurface::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin();
	if (shouldUpdate)
	{
		// update emit params
		{
			// set surface emit params
			const float coupleRate = 0.5f;
			m_surfaceEmitParams.deltaTime = dt;
			m_surfaceEmitParams.velocityLinear = m_emitParams.velocityLinear;
			m_surfaceEmitParams.velocityCoupleRate = NvFlowFloat3{ coupleRate, coupleRate, coupleRate };
			m_surfaceEmitParams.smoke = m_emitParams.smoke;
			m_surfaceEmitParams.smokeCoupleRate = coupleRate;
			m_surfaceEmitParams.temperature = m_emitParams.temperature;
			m_surfaceEmitParams.temperatureCoupleRate = coupleRate;
			m_surfaceEmitParams.fuel = m_emitParams.fuel;
			m_surfaceEmitParams.fuelCoupleRate = coupleRate;
		}

		AppGraphCtxProfileBegin(m_appctx, "ParticleSurface");

		{
			// animate wave surface
			{
				const NvFlowUint r = 32u;
				const float k = 0.125f;
				const float timeScale = 4.f;
				const float spaceScale = 32.f;
				const float pi = 3.14159f;

				const float period = 2.f * pi / (timeScale);

				m_time += dt;

				float offsetX = 0.f; // 3.f * cosf(m_time);
				float offsetY = 0.f; // 3.f * sinf(m_time);

				const float scale = 4.f;
				const float scaleInv = 1.f / scale;
				const float scaleXInv = scale / float(r);
				const float scaleYInv = scale / float(r);
				for (NvFlowUint j = 0u; j < r; j++)
				{
					for (NvFlowUint i = 0u; i < r; i++)
					{
						float x = scaleXInv * float(i) - scale * 0.5f;
						float y = scaleYInv * float(j) - scale * 0.5f;
						float d = scaleInv * sqrtf(x * x + y * y);

						m_positions[4 * (j * r + i) + 0u] = x + offsetX;
						m_positions[4 * (j * r + i) + 2u] = y + offsetY;
						m_positions[4 * (j * r + i) + 1u] = k * cosf(-timeScale * m_time + spaceScale * d);
					}
				}
			}

			NvFlowParticleSurfaceData particleData = {};
			particleData.positions = &m_positions[0u];
			particleData.positionStride = 4 * sizeof(float);
			particleData.numParticles = NvFlowUint(m_positions.size() / 4u);

			NvFlowParticleSurfaceUpdateParticles(m_particleSurface, m_flowContext.m_gridContext, &particleData);

			NvFlowParticleSurfaceUpdateSurface(m_particleSurface, m_flowContext.m_gridContext, &m_particleParams);
		}

		AppGraphCtxProfileEnd(m_appctx, "ParticleSurface");

		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		m_flowGridActor.updatePreEmit(&m_flowContext, dt);

		// emit
		{
			/*
			NvFlowShapeDesc shapeDesc;
			shapeDesc.sphere.radius = 0.8f;

			m_emitParams.localToWorld = m_emitParams.bounds;
			m_emitParams.shapeType = eNvFlowShapeTypeSphere;
			m_emitParams.deltaTime = dt;

			NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);
			*/

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);
		}

		m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

		m_shouldGridReset = false;

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSimpleFlameParticleSurface::preDraw()
{
	SceneSimpleFlame::preDraw();
}

void SceneSimpleFlameParticleSurface::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_projectile.draw(projection, view);

	m_flowContext.drawBegin();

	if (!m_visualizeSurface)
	{
		m_flowGridActor.draw(&m_flowContext, projection, view);
	}
	else
	{
		AppGraphCtxProfileBegin(m_appctx, "Render");

		auto m_renderParams = m_flowGridActor.m_renderParams;

		memcpy(&m_renderParams.projectionMatrix, &projection, sizeof(m_renderParams.projectionMatrix));
		memcpy(&m_renderParams.viewMatrix, &view, sizeof(m_renderParams.viewMatrix));

		m_renderParams.depthStencilView = m_flowContext.m_dsv;
		m_renderParams.renderTargetView = m_flowContext.m_rtv;

		auto gridExport = NvFlowParticleSurfaceDebugGridExport(m_particleSurface, m_flowContext.m_renderContext);

		NvFlowVolumeRenderGridExport(m_flowGridActor.m_volumeRender, m_flowContext.m_renderContext, gridExport, &m_renderParams);

		AppGraphCtxProfileEnd(m_appctx, "Render");
	}

	m_flowContext.drawEnd();
}

void SceneSimpleFlameParticleSurface::release()
{
	SceneSimpleFlame::release();

	NvFlowReleaseParticleSurface(m_particleSurface);
}

void SceneSimpleFlameParticleSurface::imgui(int x, int y, int w, int h)
{
	SceneSimpleFlame::imgui(x, y, w, h);
}

void SceneSimpleFlameParticleSurface::imguiFluidRenderExtra()
{
	imguiLabel("Particle Surface");
	if (imguiserCheck("SurfaceVis", m_visualizeSurface, true))
	{
		m_visualizeSurface = !m_visualizeSurface;
		m_shouldReset = true;
	}
	imguiserSlider("Smooth Radius", &m_particleParams.smoothRadius, 1.f, 16.f, 1.f, true);
	imguiserSlider("Threshold", &m_particleParams.surfaceThreshold, 0.001f, 0.01f, 0.001f, true);
	if (imguiserCheck("SeparableSmooth", m_particleParams.separableSmoothing, true))
	{
		m_particleParams.separableSmoothing = !m_particleParams.separableSmoothing;
	}
}

void SceneSimpleFlameParticleSurface::initParams()
{
	m_flowGridActor.initParams(AppGraphCtxDedicatedVideoMemory(m_appctx));

	// set emitter defaults
	NvFlowGridEmitParamsDefaults(&m_emitParams);

	// configure emitter params
	m_emitParams.bounds.x.x = 0.25f;
	m_emitParams.bounds.y.y = 0.25f;
	m_emitParams.bounds.z.z = 0.25f;
	m_emitParams.velocityLinear.y = -1.f;
	m_emitParams.fuel = 0.2f;
	m_emitParams.smoke = 0.1f;
	m_emitParams.temperature = 8.f;

	m_flowGridActor.m_materialParams.coolingRate = 3.f;

	m_shouldLoadPreset = true;
}

// *************************** SceneDynamicCoupleRate **********************

void SceneDynamicCoupleRate::initParams()
{
	SceneSimpleFlame::initParams();
	m_emitParams.allocationPredict = 0.3f;
}

float SceneDynamicCoupleRate::positionFunc(float theta)
{
	return 2.f * cosf(theta);
}

float SceneDynamicCoupleRate::velocityFunc(float theta, float rate)
{
	return -2.f * rate * sinf(theta);
}

void SceneDynamicCoupleRate::doUpdate(float dt)
{
	const float pi2 = 2.f * 3.14159265f;

	theta += pi2 * rate * dt;

	if (theta > pi2) theta -= pi2;

	// get position and velocity
	float position = positionFunc(theta);
	float velocity = velocityFunc(theta, pi2 * rate);

	// update emitter bounds
	m_emitParams.bounds.x.x = 0.25f * emitterScale;
	m_emitParams.bounds.y.y = 0.25f * emitterScale;
	m_emitParams.bounds.z.z = 0.25f * emitterScale;

	// modulate couple rate as a function of velocity and emitter width
	const float defaultCoupleRate = 0.5f;

	float emitWidth = m_emitParams.bounds.x.x;

	float coupleRate = fmaxf(coupleRateScale * fabsf(velocity) / emitWidth, defaultCoupleRate);

	m_emitParams.smokeCoupleRate = coupleRate;
	m_emitParams.temperatureCoupleRate = coupleRate;
	m_emitParams.fuelCoupleRate = coupleRate;
	m_emitParams.velocityCoupleRate = { coupleRate, coupleRate, coupleRate };

	m_emitParams.bounds.w.x = position;
	m_emitParams.velocityLinear.x = velocity;

	m_emitParams.predictVelocityWeight = 1.f;
	m_emitParams.predictVelocity.x = velocity;
	m_emitParams.predictVelocity.y = 0.f;
	m_emitParams.predictVelocity.z = 0.f;

	SceneSimpleFlame::doUpdate(dt);
}

void SceneDynamicCoupleRate::imguiFluidEmitterExtra()
{
	imguiSeparatorLine();
	imguiLabel("Dynamic Couple Rate");

	imguiserSlider("Anim Cycle Time", &rate, 0.f, 0.5f, 0.01f, true);
	imguiserSlider("Couple Rate Scale", &coupleRateScale, 0.f, 1.0f, 0.01f, true);
	imguiserSlider("Emitter Scale", &emitterScale, 0.5f, 2.0f, 0.01f, true);
}

// *************************** SceneSimpleFlameMesh ************************

void SceneSimpleFlameMesh::initParams()
{
	SceneSimpleFlame::initParams();

	NvFlowGridEmitParamsDefaults(&m_teapotEmitParams);

	m_teapotEmitParams.allocationScale = { 0.f, 0.f, 0.f };

	m_teapotEmitParams.maxActiveDist = -0.08f;
	m_teapotEmitParams.slipThickness = 0.25f;
	m_teapotEmitParams.slipFactor = 0.9f;
}

void SceneSimpleFlameMesh::init(AppGraphCtx* context, int winw, int winh)
{
	using namespace DirectX;

	m_meshContext = MeshInteropContextCreate(context);
	m_mesh = MeshCreate(m_meshContext);

	MeshLoadFromFile(m_mesh, m_meshPath);

	SceneSimpleFlame::init(context, winw, winh);

	NvFlowSDFGenDesc sdfDesc;
	sdfDesc.resolution = { 128u, 128u, 128u };

	m_sdfGen = NvFlowCreateSDFGen(m_flowContext.m_gridContext, &sdfDesc);

	MeshData meshData;
	MeshGetData(m_mesh, &meshData);

	NvFlowSDFGenReset(m_sdfGen, m_flowContext.m_gridContext);

	XMMATRIX modelMatrix = XMMatrixMultiply(
		XMMatrixScaling(m_sdfScale.x, m_sdfScale.y, m_sdfScale.z),
		XMMatrixTranslation(0.f, 0.f, 0.f)
		);

	NvFlowSDFGenMeshParams meshParams;
	meshParams.numVertices = meshData.numVertices;
	meshParams.positions = meshData.positions;
	meshParams.positionStride = meshData.positionStride;
	meshParams.normals = meshData.normals;
	meshParams.normalStride = meshData.normalStride;
	meshParams.numIndices = meshData.numIndices;
	meshParams.indices = meshData.indices;
	XMStoreFloat4x4((XMFLOAT4X4*)&meshParams.modelMatrix, modelMatrix);
	meshParams.renderTargetView = m_flowContext.m_multiGPUActive ? nullptr : m_flowContext.m_rtv;
	meshParams.depthStencilView = m_flowContext.m_multiGPUActive ? nullptr : m_flowContext.m_dsv;

	NvFlowSDFGenVoxelize(m_sdfGen, m_flowContext.m_gridContext, &meshParams);

	NvFlowSDFGenUpdate(m_sdfGen, m_flowContext.m_gridContext);

	// create shape from SDF
	m_teapotShape = NvFlowCreateShapeSDFFromTexture3D(
		m_flowContext.m_gridContext, 
		NvFlowSDFGenShape(m_sdfGen, m_flowContext.m_gridContext)
	);
}

void SceneSimpleFlameMesh::release()
{
	MeshRelease(m_mesh);
	MeshContextRelease(m_meshContext);
	NvFlowReleaseShapeSDF(m_teapotShape);
	NvFlowReleaseSDFGen(m_sdfGen);
	SceneSimpleFlame::release();
}

void SceneSimpleFlameMesh::doUpdate(float dt)
{
	NvFlowGridEmitParams& emitParams = m_teapotEmitParams;
	emitParams.bounds.x.x = m_emitterScale.x;
	emitParams.bounds.y.y = m_emitterScale.y;
	emitParams.bounds.z.z = m_emitterScale.z;
	emitParams.bounds.w.x = m_emitterOffset.x;
	emitParams.bounds.w.y = m_emitterOffset.y;
	emitParams.bounds.w.z = m_emitterOffset.z;

	emitParams.velocityCoupleRate = { 1000.f, 1000.f, 1000.f };

	emitParams.temperature = 0.f;
	emitParams.temperatureCoupleRate = 10.f;

	emitParams.fuel = 0.f;
	emitParams.fuelCoupleRate = 10.f;

	emitParams.smoke = 0.f;
	emitParams.smokeCoupleRate = 10.f;

	NvFlowShapeDesc shapeDesc;
	shapeDesc.sdf.sdfOffset = 0u; // m_teapotShape;

	emitParams.localToWorld = emitParams.bounds;
	emitParams.shapeType = eNvFlowShapeTypeSDF;
	emitParams.deltaTime = dt;

	NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &emitParams, 1u);

	NvFlowShapeSDF* sdfs[] = { m_teapotShape };
	NvFlowGridUpdateEmitSDFs(m_flowGridActor.m_grid, sdfs, 1u);

	// animate emitter
	if (m_animate)
	{
		m_time += dt;

		const float rate = 2.f;
		const float period = 2.f * 3.14159265f / rate;
		if (m_time > period) m_time -= period;

		m_emitParams.bounds.w.x = 0.15f * sinf(rate * m_time);
		m_emitParams.bounds.w.z = 0.f;
	}
	else
	{
		m_emitParams.bounds.w.x = 0.f;
		m_emitParams.bounds.w.z = 0.f;

		m_time = 0.f;
	}

	SceneSimpleFlame::doUpdate(dt);
}

void SceneSimpleFlameMesh::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	MeshInteropContextUpdate(m_meshContext, m_appctx);

	MeshDrawParams meshDrawParams;
	meshDrawParams.renderMode = MESH_RENDER_SOLID;
	meshDrawParams.projection = projection;
	meshDrawParams.view = view;
	meshDrawParams.model = DirectX::XMMatrixMultiply(
		DirectX::XMMatrixScaling(m_emitterScale.x * m_sdfScale.x, m_emitterScale.y * m_sdfScale.y, m_emitterScale.z * m_sdfScale.z),
		DirectX::XMMatrixTranslation(m_emitterOffset.x, m_emitterOffset.y, m_emitterOffset.z)
		);

	if (m_shouldDrawMesh)
	{
		MeshDraw(m_mesh, &meshDrawParams);
	}

	SceneSimpleFlame::draw(projection, view);
}

void SceneSimpleFlameMesh::imguiFluidRenderExtra()
{
	if (imguiserCheck("Draw Mesh", m_shouldDrawMesh, true))
	{
		m_shouldDrawMesh = !m_shouldDrawMesh;
	}
}

void SceneSimpleFlameMesh::imguiFluidEmitterExtra()
{
	if (imguiserCheck("Animate", m_animate, true))
	{
		m_animate = !m_animate;
	}
	imguiSeparatorLine();
	imguiLabel("Teapot");
	imguiserBeginGroup("Teapot", nullptr);

	imguiserSlider("Max Emit Dist", &m_teapotEmitParams.maxActiveDist, -1.f, 1.f, 0.01f, true);
	imguiserSlider("Min Emit Dist", &m_teapotEmitParams.minActiveDist, -1.f, 1.f, 0.01f, true);
	imguiserSlider("Slip Thickness", &m_teapotEmitParams.slipThickness, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Slip Factor", &m_teapotEmitParams.slipFactor, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel Release Temp", &m_teapotEmitParams.fuelReleaseTemp, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel Release", &m_teapotEmitParams.fuelRelease, 0.f, 10.f, 0.1f, true);

	imguiserEndGroup();
}

// *************************** SceneSimpleFlameCulling ************************

void SceneSimpleFlameCulling::initParams()
{
	SceneSimpleFlame::initParams();

	m_emitParams.allocationScale = { 2.f, 2.f, 2.f };
	m_emitParams.allocationPredict = 0.4f;
}

void SceneSimpleFlameCulling::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin();
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		{
			m_flowGridActor.updatePreEmit(&m_flowContext, dt);

			// main emitter
			{
				NvFlowShapeDesc shapeDesc;
				shapeDesc.box.halfSize = { 0.8f, 0.8f, 0.8f };

				m_emitParams.localToWorld = m_emitParams.bounds;
				m_emitParams.shapeType = eNvFlowShapeTypeBox;
				m_emitParams.deltaTime = dt;

				NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);
			}

			// grid of emitters
			{
				const int r = m_emitGridR;
				const float scale = 0.125f;

				NvFlowShapeDesc shapeDesc;
				shapeDesc.sphere.radius = 0.8f;

				NvFlowGridEmitParams emitParams;

				NvFlowGridEmitParamsDefaults(&emitParams);

				emitParams.bounds.x.x = 0.25f;
				emitParams.bounds.y.y = 0.25f;
				emitParams.bounds.z.z = 0.25f;

				emitParams.maxActiveDist = -0.08f;
				emitParams.slipThickness = 0.25f;
				emitParams.slipFactor = 0.9f;

				emitParams.velocityCoupleRate = { 1000.f, 1000.f, 1000.f };

				emitParams.temperature = 0.f;
				emitParams.temperatureCoupleRate = 10.f;

				emitParams.fuel = 0.f;
				emitParams.fuelCoupleRate = 10.f;

				emitParams.smoke = 0.f;
				emitParams.smokeCoupleRate = 10.f;

				emitParams.allocationScale = { 1.f, 1.f, 1.f };
				emitParams.allocationPredict = 0.f;

				for (int j = -r; j <= r; j++)
				{
					for (int i = -r; i <= r; i++)
					{
						if (i == 0 && j == 0) continue;

						emitParams.bounds.w.x = scale * float(i);
						emitParams.bounds.w.y = 0.f;
						emitParams.bounds.w.z = scale * float(j);

						emitParams.localToWorld = emitParams.bounds;
						emitParams.shapeType = eNvFlowShapeTypeSphere;
						emitParams.deltaTime = dt;

						NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &emitParams, 1u);
					}
				}
			}

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);

			m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

			m_shouldGridReset = false;
		}

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSimpleFlameCulling::imguiFluidEmitterExtra()
{
	imguiSeparatorLine();
	imguiLabel("Culling");
	imguiserBeginGroup("Culling", nullptr);

	float r = float(m_emitGridR);
	imguiserSlider("Emit Grid R", &r, 0.f, 16.f, 1.f, true);
	m_emitGridR = NvFlowUint(r);

	imguiserEndGroup();
}

// *************************** SceneSimpleFlameConvex ************************

void SceneSimpleFlameConvex::initParams()
{
	SceneSimpleFlame::initParams();

	m_emitParams.minActiveDist = -0.3f;
	m_emitParams.fuel = 2.5f;

	m_flowGridActor.m_materialParams.smoke.damping = 0.5f;
	m_flowGridActor.m_materialParams.smoke.fade = 0.5f;
}

void SceneSimpleFlameConvex::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin();
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		{
			m_flowGridActor.updatePreEmit(&m_flowContext, dt);

			NvFlowShapeDesc shapeDesc[8];
			const float a = 0.866f;
			const float b = 0.5f;
			const float d = 4.f * m_size;
			shapeDesc[0].plane = { +1.f, 0.f, 0.f, d };
			shapeDesc[1].plane = { -1.f, 0.f, 0.f, d };
			shapeDesc[2].plane = { +b, 0.f, +a, d };
			shapeDesc[3].plane = { +b, 0.f, -a, d };
			shapeDesc[4].plane = { -b, 0.f, +a, d };
			shapeDesc[5].plane = { -b, 0.f, -a, d };
			shapeDesc[6].plane = { 0.f, +1.f, 0.f, 0.8f };
			shapeDesc[7].plane = { 0.f, -1.f, 0.f, 0.8f };

			m_emitParams.bounds.x.x = m_size + 0.25f;
			//m_emitParams.bounds.y.y = m_barLength;
			m_emitParams.bounds.z.z = m_size + 0.25f;

			m_emitParams.localToWorld = NvFlowFloat4x4{
				0.25f, 0.f, 0.f, 0.f,
				0.f, 0.25f, 0.f, 0.f,
				0.f, 0.f, 0.25f, 0.f,
				m_emitParams.bounds.w.x, m_emitParams.bounds.w.y, m_emitParams.bounds.w.z, 1.f
			};
			m_emitParams.shapeType = eNvFlowShapeTypePlane;
			m_emitParams.shapeDistScale = m_distanceScale;
			m_emitParams.shapeRangeOffset = 0u;
			m_emitParams.shapeRangeSize = 8u;
			m_emitParams.deltaTime = dt;

			NvFlowGridEmit(m_flowGridActor.m_grid, shapeDesc, 8u, &m_emitParams, 1u);

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);

			m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

			m_shouldGridReset = false;
		}

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSimpleFlameConvex::imguiFluidEmitterExtra()
{
	imguiSeparatorLine();
	imguiLabel("Bar");
	imguiserBeginGroup("Bar", nullptr);

	imguiserSlider("Size", &m_size, 0.f, 4.f, 0.1f, true);
	imguiserSlider("Distance Scale", &m_distanceScale, 0.1f, 3.f, 0.025f, true);

	imguiserEndGroup();
}

// *************************** SceneSimpleFlameCapsule ************************

void SceneSimpleFlameCapsule::initParams()
{
	SceneSimpleFlame::initParams();

	m_emitParams.minActiveDist = -0.3f;
	m_emitParams.fuel = 2.5f;

	m_flowGridActor.m_materialParams.smoke.damping = 0.5f;
	m_flowGridActor.m_materialParams.smoke.fade = 0.5f;
}

void SceneSimpleFlameCapsule::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin();
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		if (shouldUpdate)
		{
			m_flowGridActor.updatePreEmit(&m_flowContext, dt);

			NvFlowShapeDesc shapeDesc[1];
			shapeDesc[0].capsule.radius = m_capsuleRadius;
			shapeDesc[0].capsule.length = m_capsuleLength;

			// generate bounds based on radius and length
			const float boundInflate = 0.25f;
			float xbound = 2.f * m_capsuleRadius + m_capsuleLength + boundInflate;
			float yzbound = 2.f * m_capsuleRadius + boundInflate;
			m_emitParams.bounds.x.x = 0.5f * xbound;
			m_emitParams.bounds.y.y = 0.5f * yzbound;
			m_emitParams.bounds.z.z = 0.5f * yzbound;

			m_emitParams.localToWorld = NvFlowFloat4x4{
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				m_emitParams.bounds.w.x, m_emitParams.bounds.w.y, m_emitParams.bounds.w.z, 1.f
			};

			m_emitParams.shapeType = eNvFlowShapeTypeCapsule;
			m_emitParams.shapeDistScale = m_distanceScale;
			m_emitParams.deltaTime = dt;

			if (m_boxMode)
			{
				m_emitParams.shapeType = eNvFlowShapeTypeBox;
				shapeDesc[0].box.halfSize.x = 0.5f * m_capsuleLength + m_capsuleRadius;
				shapeDesc[0].box.halfSize.y = m_capsuleRadius;
				shapeDesc[0].box.halfSize.z = 0.5f * m_capsuleRadius;
			}

			NvFlowGridEmit(m_flowGridActor.m_grid, shapeDesc, 1u, &m_emitParams, 1u);

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);

			m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

			m_shouldGridReset = false;
		}

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSimpleFlameCapsule::imguiFluidEmitterExtra()
{
	imguiSeparatorLine();
	imguiLabel("Bar");
	imguiserBeginGroup("Bar", nullptr);

	imguiserSlider("Capsule Radius", &m_capsuleRadius, 0.f, 1.f, 0.1f, true);
	imguiserSlider("Capsule Length", &m_capsuleLength, 0.f, 3.f, 0.1f, true);
	imguiserSlider("Distance Scale", &m_distanceScale, 0.1f, 5.f, 0.025f, true);

	if (imguiserCheck("Box mode", m_boxMode, true))
	{
		m_boxMode = !m_boxMode;
	}

	imguiserEndGroup();
}