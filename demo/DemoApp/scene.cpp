/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "scene.h"

#include <stdio.h>

#include "loader.h"
#include "imgui.h"
#include "imguiser.h"

#include "scene.h"

#include <SDL.h>

/// Scene registry here

namespace Scenes
{
	SceneSimpleFlame sceneSimpleFlame;
	SceneSimpleFlameThrower sceneSimpleFlameThrower;
	SceneSimpleFlameAnimated sceneSimpleFlameAnimated;
	Scene2DTextureEmitter scene2DTextureEmitter1(false);
	Scene2DTextureEmitter scene2DTextureEmitter2(true);
	SceneSimpleFlameMesh sceneSimpleFlameMesh;
	SceneSimpleFlameCollision sceneSimpleFlameCollision;
	//SceneSimpleFlameCulling sceneSimpleFlameCulling;
	SceneSimpleFlameConvex sceneSimpleFlameConvex;
	SceneSimpleFlameCapsule sceneSimpleFlameCapsule;
	SceneSDFTest sceneSDFTest;
	SceneCustomLighting sceneCustomLighting;
	SceneSimpleSmoke sceneSimpleSmoke;
	SceneDynamicCoupleRate sceneDynamicCoupleRate;
	SceneCustomEmit sceneCustomEmit;
	SceneSimpleFlameDouble sceneSimpleFlame2;
	SceneSimpleFlameFuelMap sceneSimpleFlameFuelMap;
	SceneSimpleFlameParticleSurface sceneParticleSurface;
	SceneSimpleFlameBall sceneSimpleFlameBall;

	const int count = 18;
	Scene* list[count] = {
		&scene2DTextureEmitter1,
		&scene2DTextureEmitter2,
		&sceneSimpleFlame,
		&sceneSimpleFlameThrower,
		&sceneSimpleFlameAnimated,
		&sceneSimpleFlameMesh,
		&sceneSDFTest,
		&sceneSimpleFlameCollision,
		//&sceneSimpleFlameCulling,
		&sceneSimpleFlameConvex,
		&sceneSimpleFlameCapsule,
		&sceneSimpleSmoke,
		&sceneCustomLighting,
		&sceneDynamicCoupleRate,
		&sceneCustomEmit,
		&sceneSimpleFlame2,
		&sceneSimpleFlameFuelMap,
		&sceneParticleSurface,
		&sceneSimpleFlameBall
	};
};

Scene* getScene(int index)
{
	if (index < Scenes::count)
	{
		return Scenes::list[index];
	}
	return nullptr;
}

void pointsToImage(NvFlowFloat4* image, int imageDim, const CurvePoint* pts, int numPts)
{
	using namespace DirectX;
	for (int i = 0; i < imageDim; i++)
	{
		float u = (float(i) + 0.5f) / float(imageDim);

		// find the closest higher and lower control points
		int xloweri = -1;
		float xlowerVal = 0.f;
		int xhigheri = -1;
		float xhigherVal = 1.f;
		for (int pt = 0; pt < numPts; pt++)
		{
			if (pts[pt].x <= u && pts[pt].x >= xlowerVal)
			{
				xloweri = pt;
				xlowerVal = pts[pt].x;
			}
			if (pts[pt].x >= u && pts[pt].x <= xhigherVal)
			{
				xhigheri = pt;
				xhigherVal = pts[pt].x;
			}
		}

		// get values for interpolation
		XMVECTOR a = (xloweri >= 0) ? XMLoadFloat4((XMFLOAT4*)&pts[xloweri].r) : XMVectorSet(0.f, 0.f, 0.f, 0.f);
		XMVECTOR b = (xhigheri >= 0) ? XMLoadFloat4((XMFLOAT4*)&pts[xhigheri].r) : XMVectorSet(1.f, 1.f, 1.f, 1.f);
		float t = (u - xlowerVal) / (xhigherVal - xlowerVal);
		XMVECTOR c = XMVectorLerp(a, b, t);

		XMStoreFloat4((XMFLOAT4*)&image[i], c);
	}
}

void Scene::update(float dt)
{
	int numSteps = m_timeStepper.getNumSteps(dt);

	for (int i = 0; i < numSteps; i++)
	{
		doUpdate(m_timeStepper.m_fixedDt);
	}
}

void Scene::resize(int winw, int winh)
{
	m_winw = winw;
	m_winh = winh;
}

bool Scene::imguiMouse(int mx, int my, unsigned char mbut)
{
	m_mx = mx;
	m_my = my;
	m_mbut = mbut;
	return false;
}

// ************************* SceneFluid *****************************************

void SceneFluid::imguiDesc()
{
	{
		bool enableVTR = (m_flowGridActor.m_gridDesc.enableVTR != false);
		if (imguiCheck("Enable VTR", enableVTR, true))
		{
			if (enableVTR == false)
			{
				NvFlowSupport support;
				if (NvFlowGridQuerySupport(m_flowGridActor.m_grid, m_flowContext.m_gridContext, &support) == eNvFlowSuccess)
				{
					m_flowGridActor.m_gridDesc.enableVTR = support.supportsVTR;
				}
			}
			else
			{
				m_flowGridActor.m_gridDesc.enableVTR = false;
			}
			m_shouldReset = true;
		}
		bool lowResDensity = m_flowGridActor.m_gridDesc.densityMultiRes == eNvFlowMultiRes1x1x1;
		if (imguiCheck("Low Res Density", lowResDensity, true))
		{
			if (m_flowGridActor.m_gridDesc.densityMultiRes == eNvFlowMultiRes1x1x1)
			{
				m_flowGridActor.m_gridDesc.densityMultiRes = eNvFlowMultiRes2x2x2;
			}
			else
			{
				m_flowGridActor.m_gridDesc.densityMultiRes = eNvFlowMultiRes1x1x1;
			}
			m_shouldReset = true;
		}
		if (imguiCheck("Low Latency Mapping", m_flowGridActor.m_gridDesc.lowLatencyMapping, true))
		{
			m_flowGridActor.m_gridDesc.lowLatencyMapping = !m_flowGridActor.m_gridDesc.lowLatencyMapping;
			m_shouldReset = true;
		}
	}
	{
		if (!m_flowContext.m_multiGPUSupported)
		{
			m_flowContext.m_enableMultiGPU = false;
		}
		if (imguiCheck("Enable MultiGPU", m_flowContext.m_enableMultiGPU, true))
		{
			m_flowContext.m_enableMultiGPU = !m_flowContext.m_enableMultiGPU;
			if (!m_flowContext.m_multiGPUSupported)
			{
				m_flowContext.m_enableMultiGPU = false;
			}
			else
			{
				m_shouldReset = true;
			}
		}
	}
	{
		if (!m_flowContext.m_commandQueueSupported)
		{
			m_flowContext.m_enableCommandQueue = false;
		}
		if (imguiCheck("Enable MultiQueue", m_flowContext.m_enableCommandQueue, true))
		{
			m_flowContext.m_enableCommandQueue = !m_flowContext.m_enableCommandQueue;
			if (!m_flowContext.m_commandQueueSupported)
			{
				m_flowContext.m_enableCommandQueue = false;
			}
			else
			{
				m_shouldReset = true;
			}
		}
	}

	float oldMemoryLimit = m_flowGridActor.m_memoryLimit;
	imguiserSlider("Memory Limit", &m_flowGridActor.m_memoryLimit, 1.f, 6.f, 1.f, true);
	if (oldMemoryLimit != m_flowGridActor.m_memoryLimit)
	{
		m_flowGridActor.m_gridDesc.residentScale = m_flowGridActor.m_memoryLimit * m_flowGridActor.m_memoryScale;
		m_shouldReset = true;
	}

	if (imguiButton("Reset", true))
	{
		m_shouldReset = true;
	}

	if (imguiButton("Grid Reset", true))
	{
		m_shouldGridReset = true;
	}

	float cellSizeLogf = float(m_flowGridActor.m_cellSizeLogScale);
	if (imguiserSlider("Cell Size Log Scale", &cellSizeLogf, -6.f, 6.f, 1.f, true))
	{
		m_flowGridActor.m_cellSizeLogScale = int(cellSizeLogf);
		m_shouldGridReset = true;
	}
	if (imguiserSlider("Cell Size Scale", &m_flowGridActor.m_cellSizeScale, 0.8f, 1.25f, 0.001f, true))
	{
		m_shouldGridReset = true;
	}

	imguiDescExtra();
}

void SceneFluid::imguiFluidSim()
{
	imguiSeparatorLine();
	imguiLabel("Fluid Simulation");
	imguiserBeginGroup("Fluid Simulation", nullptr);

	imguiSeparator();
	imguiLabel("Damping");
	imguiserBeginGroup("Damping", nullptr);
	imguiserSlider("Velocity", &m_flowGridActor.m_materialParams.velocity.damping, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Smoke", &m_flowGridActor.m_materialParams.smoke.damping, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Temp", &m_flowGridActor.m_materialParams.temperature.damping, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel", &m_flowGridActor.m_materialParams.fuel.damping, 0.f, 1.f, 0.01f, true);
	imguiserEndGroup();

	imguiSeparator();
	imguiLabel("Fade");
	imguiserBeginGroup("Fade", nullptr);
	imguiserSlider("Velocity", &m_flowGridActor.m_materialParams.velocity.fade, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Smoke", &m_flowGridActor.m_materialParams.smoke.fade, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Temp", &m_flowGridActor.m_materialParams.temperature.fade, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel", &m_flowGridActor.m_materialParams.fuel.fade, 0.f, 1.f, 0.01f, true);
	imguiserEndGroup();

	imguiSeparator();
	imguiLabel("MacCormack Correction");
	imguiserBeginGroup("MacCormack Correction", nullptr);
	imguiserSlider("Velocity", &m_flowGridActor.m_materialParams.velocity.macCormackBlendFactor, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Smoke", &m_flowGridActor.m_materialParams.smoke.macCormackBlendFactor, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Temp", &m_flowGridActor.m_materialParams.temperature.macCormackBlendFactor, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel", &m_flowGridActor.m_materialParams.fuel.macCormackBlendFactor, 0.f, 1.f, 0.01f, true);
	imguiserEndGroup();
	imguiLabel("MacCormack Threshold");
	imguiserBeginGroup("MacCormack Threshold", nullptr);
	imguiserSlider("Velocity", &m_flowGridActor.m_materialParams.velocity.macCormackBlendThreshold, 0.f, 0.01f, 0.001f, true);
	imguiserSlider("Smoke", &m_flowGridActor.m_materialParams.smoke.macCormackBlendThreshold, 0.f, 0.01f, 0.001f, true);
	imguiserSlider("Temp", &m_flowGridActor.m_materialParams.temperature.macCormackBlendThreshold, 0.f, 0.01f, 0.001f, true);
	imguiserSlider("Fuel", &m_flowGridActor.m_materialParams.fuel.macCormackBlendThreshold, 0.f, 0.01f, 0.001f, true);
	imguiserEndGroup();

	imguiserSlider("Vorticity Strength", &m_flowGridActor.m_materialParams.vorticityStrength, 0.f, 20.f, 0.1f, true);
	imguiserSlider("Vorticity Vel Mask", &m_flowGridActor.m_materialParams.vorticityVelocityMask, 0.f, 1.f, 0.01f, true);

	if (imguiCheck("Legacy Pressure", m_flowGridActor.m_gridParams.pressureLegacyMode, true))
	{
		m_flowGridActor.m_gridParams.pressureLegacyMode = !m_flowGridActor.m_gridParams.pressureLegacyMode;
	}

	imguiSeparator();
	imguiLabel("Combustion");
	imguiserBeginGroup("Combustion", nullptr);
	imguiserSlider("Ignition Temp", &m_flowGridActor.m_materialParams.ignitionTemp, 0.f, 0.5f, 0.05f, true);
	imguiserSlider("Cooling Rate", &m_flowGridActor.m_materialParams.coolingRate, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Buoyancy", &m_flowGridActor.m_materialParams.buoyancyPerTemp, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Expansion", &m_flowGridActor.m_materialParams.divergencePerBurn, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Smoke Per Burn", &m_flowGridActor.m_materialParams.smokePerBurn, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Temp Per Burn", &m_flowGridActor.m_materialParams.tempPerBurn, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Fuel Per Burn", &m_flowGridActor.m_materialParams.fuelPerBurn, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Burn Per Temp", &m_flowGridActor.m_materialParams.burnPerTemp, 0.f, 10.f, 0.1f, true);
	imguiserEndGroup();

	imguiFluidSimExtra();

	imguiserEndGroup();
}

void SceneFluid::imguiFluidRender()
{
	imguiSeparatorLine();
	imguiLabel("Rendering");
	imguiserBeginGroup("Rendering", nullptr);
	imguiserSlider("Alpha Scale", &m_flowGridActor.m_renderMaterialDefaultParams.alphaScale, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Additive", &m_flowGridActor.m_renderMaterialDefaultParams.additiveFactor, 0.f, 1.f, 0.01f, true);
	float renderModef = (float)m_flowGridActor.m_renderParams.renderMode;
	if (imguiserSlider("Render Mode", &renderModef, 0.f, float(eNvFlowVolumeRenderModeCount) - 1.f, 1.f, true))
	{
		m_flowGridActor.m_renderParams.renderMode = (NvFlowVolumeRenderMode)((NvFlowUint)renderModef);
	}
	float renderChannelf = (float)m_flowGridActor.m_renderParams.renderChannel;
	if (imguiserSlider("Render Channel", &renderChannelf, 0.f, float(eNvFlowGridTextureChannelCount) - 1.f, 1.f, true))
	{
		m_flowGridActor.m_renderParams.renderChannel = (NvFlowGridTextureChannel)((NvFlowUint)renderChannelf);
	}
	if (imguiserCheck("Separate Lighting", m_flowGridActor.m_separateLighting, true))
	{
		m_flowGridActor.m_separateLighting = !m_flowGridActor.m_separateLighting;
	}
	if (imguiserCheck("Debug Render", m_flowGridActor.m_renderParams.debugMode, true))
	{
		m_flowGridActor.m_renderParams.debugMode = !m_flowGridActor.m_renderParams.debugMode;
	}
	if (m_flowGridActor.m_renderParams.debugMode)
	{
		imguiIndent();
		if (imguiserCheck("Blocks", (m_flowGridActor.m_gridParams.debugVisFlags & eNvFlowGridDebugVisBlocks) != 0, true))
		{
			m_flowGridActor.m_gridParams.debugVisFlags = NvFlowGridDebugVisFlags(m_flowGridActor.m_gridParams.debugVisFlags ^ eNvFlowGridDebugVisBlocks);
		}
		if (imguiserCheck("Emit Bounds", (m_flowGridActor.m_gridParams.debugVisFlags & eNvFlowGridDebugVisEmitBounds) != 0, true))
		{
			m_flowGridActor.m_gridParams.debugVisFlags = NvFlowGridDebugVisFlags(m_flowGridActor.m_gridParams.debugVisFlags ^ eNvFlowGridDebugVisEmitBounds);
		}
		if (imguiserCheck("Shapes Simple", (m_flowGridActor.m_gridParams.debugVisFlags & eNvFlowGridDebugVisShapesSimple) != 0, true))
		{
			m_flowGridActor.m_gridParams.debugVisFlags = NvFlowGridDebugVisFlags(m_flowGridActor.m_gridParams.debugVisFlags ^ eNvFlowGridDebugVisShapesSimple);
		}
		imguiUnindent();
	}
	if (imguiserCheck("Edit ColorMap Default", m_flowGridActor.m_colorMap.m_curveEditorActiveDefault, true))
	{
		m_flowGridActor.m_colorMap.m_curveEditorActiveDefault = !m_flowGridActor.m_colorMap.m_curveEditorActiveDefault;
	}
	if (imguiserCheck("Edit ColorMap Mat0", m_flowGridActor.m_colorMap.m_curveEditorActiveMat0, true))
	{
		m_flowGridActor.m_colorMap.m_curveEditorActiveMat0 = !m_flowGridActor.m_colorMap.m_curveEditorActiveMat0;
	}
	if (imguiserCheck("Edit ColorMap Mat1", m_flowGridActor.m_colorMap.m_curveEditorActiveMat1, true))
	{
		m_flowGridActor.m_colorMap.m_curveEditorActiveMat1 = !m_flowGridActor.m_colorMap.m_curveEditorActiveMat1;
	}
	if (imguiserCheck("Multires", m_flowGridActor.m_renderParams.multiRes.enabled, true))
	{
		m_flowGridActor.m_renderParams.multiRes.enabled = !m_flowGridActor.m_renderParams.multiRes.enabled;
	}
	if (imguiserCheck("Lens Matched", m_flowGridActor.m_renderParams.lensMatchedShading.enabled, true))
	{
		m_flowGridActor.m_renderParams.lensMatchedShading.enabled = !m_flowGridActor.m_renderParams.lensMatchedShading.enabled;
	}
	if (m_flowGridActor.m_renderParams.multiRes.enabled)
	{
		m_flowGridActor.m_renderParams.multiRes.viewport.width = (float)m_winw;
		m_flowGridActor.m_renderParams.multiRes.viewport.height = (float)m_winh;
		m_flowGridActor.m_renderParams.multiRes.nonMultiResWidth = (float)m_winw;
		m_flowGridActor.m_renderParams.multiRes.nonMultiResHeight = (float)m_winh;
	}
	if (m_flowGridActor.m_renderParams.lensMatchedShading.enabled)
	{
		m_flowGridActor.m_renderParams.lensMatchedShading.viewport.width = (float)m_winw;
		m_flowGridActor.m_renderParams.lensMatchedShading.viewport.height = (float)m_winh;
		m_flowGridActor.m_renderParams.lensMatchedShading.nonLMSWidth = (float)m_winw;
		m_flowGridActor.m_renderParams.lensMatchedShading.nonLMSHeight = (float)m_winh;
	}
	imguiserSlider("Screen Percentage", &m_flowGridActor.m_renderParams.screenPercentage, 0.1f, 1.f, 0.01f, true);
	float smoothVal = float(m_flowGridActor.m_renderParams.smoothColorUpsample);
	if (imguiserSlider("Upsample Smooth", &smoothVal, 0.f, 1.f, 1.f, true))
	{
		m_flowGridActor.m_renderParams.smoothColorUpsample = smoothVal > 0.5f;
	}
	if (imguiserCheck("Generate Depth", m_flowGridActor.m_renderParams.generateDepth, true))
	{
		m_flowGridActor.m_renderParams.generateDepth = !m_flowGridActor.m_renderParams.generateDepth;
	}
	if (imguiserCheck("Generate Depth Vis", m_flowGridActor.m_renderParams.generateDepthDebugMode, true))
	{
		m_flowGridActor.m_renderParams.generateDepthDebugMode = !m_flowGridActor.m_renderParams.generateDepthDebugMode;
	}

	imguiLabel("MultiRes Ray March");
	float multiResRayMarchf = (float)m_flowGridActor.m_renderParams.multiResRayMarch;
	if (imguiserSlider("MultiRes levels", &multiResRayMarchf, 0.f, 4.f, 1.f, true))
	{
		m_flowGridActor.m_renderParams.multiResRayMarch = (NvFlowMultiResRayMarch)NvFlowUint(multiResRayMarchf);
	}
	imguiserSlider("Sampling Rate", &m_flowGridActor.m_renderParams.multiResSamplingScale, 0.1f, 10.f, 0.1f, true);

	imguiLabel("Volume Shadow");
	if (imguiserCheck("Enabled", m_flowGridActor.m_enableVolumeShadow, true))
	{
		m_flowGridActor.m_enableVolumeShadow = !m_flowGridActor.m_enableVolumeShadow;
		m_shouldReset = true;
	}
	if (m_flowGridActor.m_enableVolumeShadow)
	{
		if (imguiserCheck("Force Apply", m_flowGridActor.m_forceApplyShadow, true))
		{
			m_flowGridActor.m_forceApplyShadow = !m_flowGridActor.m_forceApplyShadow;
		}
		imguiserSlider("Light Pan", &m_flowGridActor.m_shadowPan, -4.f, 4.f, 0.01f, true);
		imguiserSlider("Light Tilt", &m_flowGridActor.m_shadowTilt, -4.f, 4.f, 0.01f, true);
		if (imguiserSlider("Memory Scale", &m_flowGridActor.m_shadowResidentScale, 0.5f, 2.f, 0.1f, true))
		{
			m_shouldReset = true;
		}
		imguiserSlider("Intensity Scale", &m_flowGridActor.m_shadowIntensityScale, 0.f, 5.f, 0.01f, true);
		imguiserSlider("Min Intensity", &m_flowGridActor.m_shadowMinIntensity, 0.0f, 1.f, 0.01f, true);
		imguiserSlider("BlendTempFactor", &m_flowGridActor.m_shadowBlendCompMask.x, -5.f, 5.f, 0.1f, true);
		imguiserSlider("BlendBias", &m_flowGridActor.m_shadowBlendBias, -5.0f, 5.f, 0.1f, true);
		if (imguiserCheck("Debug Vis", m_flowGridActor.m_shadowDebugVis, true))
		{
			m_flowGridActor.m_shadowDebugVis = !m_flowGridActor.m_shadowDebugVis;
		}
	}

	imguiLabel("Cross Section");
	if (imguiserCheck("Enabled", m_flowGridActor.m_enableCrossSection, true))
	{
		m_flowGridActor.m_enableCrossSection = !m_flowGridActor.m_enableCrossSection;
	}
	if (m_flowGridActor.m_enableCrossSection)
	{
		if (imguiserCheck("Fullscreen", m_flowGridActor.m_crossSectionParams.fullscreen, true))
		{
			m_flowGridActor.m_crossSectionParams.fullscreen = !m_flowGridActor.m_crossSectionParams.fullscreen;
		}
		if (imguiserCheck("Point Filter", m_flowGridActor.m_crossSectionParams.pointFilter, true))
		{
			m_flowGridActor.m_crossSectionParams.pointFilter = !m_flowGridActor.m_crossSectionParams.pointFilter;
		}
		if (imguiserCheck("Velocity Vectors", m_flowGridActor.m_crossSectionParams.velocityVectors, true))
		{
			m_flowGridActor.m_crossSectionParams.velocityVectors = !m_flowGridActor.m_crossSectionParams.velocityVectors;
		}
		if (imguiserCheck("Outline Cells", m_flowGridActor.m_crossSectionParams.outlineCells, true))
		{
			m_flowGridActor.m_crossSectionParams.outlineCells = !m_flowGridActor.m_crossSectionParams.outlineCells;
		}

		float axis = float(m_flowGridActor.m_crossSectionParams.crossSectionAxis);
		if (imguiserSlider("Axis", &axis, 0.f, 2.f, 1.f, true))
		{
			m_flowGridActor.m_crossSectionParams.crossSectionAxis = NvFlowUint(axis);
		}
		imguiserSlider("PosX", &m_flowGridActor.m_crossSectionParams.crossSectionPosition.x, -1.f, 1.f, 0.01f, true);
		imguiserSlider("PosY", &m_flowGridActor.m_crossSectionParams.crossSectionPosition.y, -1.f, 1.f, 0.01f, true);
		imguiserSlider("PosZ", &m_flowGridActor.m_crossSectionParams.crossSectionPosition.z, -1.f, 1.f, 0.01f, true);
		imguiserSlider("Scale", &m_flowGridActor.m_crossSectionScale, 0.25f, 20.f, 0.1f, true);

		float renderModef = (float)m_flowGridActor.m_crossSectionParams.renderMode;
		if (imguiserSlider("Cross Render Mode", &renderModef, 0.f, float(eNvFlowVolumeRenderModeCount) - 1.f, 1.f, true))
		{
			m_flowGridActor.m_crossSectionParams.renderMode = (NvFlowVolumeRenderMode)((NvFlowUint)renderModef);
		}
		float renderChannelf = (float)m_flowGridActor.m_crossSectionParams.renderChannel;
		if (imguiserSlider("Cross Render Channel", &renderChannelf, 0.f, float(eNvFlowGridTextureChannelCount) - 1.f, 1.f, true))
		{
			m_flowGridActor.m_crossSectionParams.renderChannel = (NvFlowGridTextureChannel)((NvFlowUint)renderChannelf);
		}
		imguiserSlider("Intensity", &m_flowGridActor.m_crossSectionParams.intensityScale, 0.01f, 10.f, 0.01f, true);
		imguiserSlider("Background Color", &m_flowGridActor.m_crossSectionBackgroundColor, 0.f, 1.f, 1.f, true);
		imguiserSlider("Line Color R", &m_flowGridActor.m_crossSectionLineColor.x, 0.f, 1.f, 0.01f, true);
		imguiserSlider("Line Color G", &m_flowGridActor.m_crossSectionLineColor.y, 0.f, 1.f, 0.01f, true);
		imguiserSlider("Line Color B", &m_flowGridActor.m_crossSectionLineColor.z, 0.f, 1.f, 0.01f, true);
		imguiserSlider("Cell Color R", &m_flowGridActor.m_crossSectionParams.cellColor.x, 0.f, 1.f, 0.01f, true);
		imguiserSlider("Cell Color G", &m_flowGridActor.m_crossSectionParams.cellColor.y, 0.f, 1.f, 0.01f, true);
		imguiserSlider("Cell Color B", &m_flowGridActor.m_crossSectionParams.cellColor.z, 0.f, 1.f, 0.01f, true);

		imguiserSlider("Velocity Scale", &m_flowGridActor.m_crossSectionParams.velocityScale, 0.1f, 10.f, 0.01f, true);
		imguiserSlider("Vector Length", &m_flowGridActor.m_crossSectionParams.vectorLengthScale, 0.1f, 2.f, 0.01f, true);
	}

	imguiFluidRenderExtra();
	imguiserEndGroup();
}

void SceneFluid::imguiFluidEmitter()
{
	imguiSeparatorLine();
	imguiLabel("Emitter");
	imguiserBeginGroup("Emitter", nullptr);
	imguiserSlider("Emit Velocity", &m_emitParams.velocityLinear.y, -16.f, 16.f, 0.1f, true);
	imguiserSlider("Emit Smoke", &m_emitParams.smoke, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Emit Temp", &m_emitParams.temperature, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Emit Fuel", &m_emitParams.fuel, 0.f, 10.f, 0.1f, true);
	imguiserSlider("Max Emit Dist", &m_emitParams.maxActiveDist, -1.f, 1.f, 0.01f, true);
	imguiserSlider("Min Emit Dist", &m_emitParams.minActiveDist, -1.f, 1.f, 0.01f, true);
	imguiserSlider("Max Edge Dist", &m_emitParams.maxEdgeDist, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Min Edge Dist", &m_emitParams.minEdgeDist, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel Release Temp", &m_emitParams.fuelReleaseTemp, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel Release", &m_emitParams.fuelRelease, 0.f, 10.f, 0.1f, true);
	//imguiserSlider("Slip Thickness", &m_emitParams.slipThickness, 0.f, 1.f, 0.01f, true);
	//imguiserSlider("Slip Factor", &m_emitParams.slipFactor, 0.f, 1.f, 0.01f, true);
	float allocScale = m_emitParams.allocationScale.x;
	if (imguiserSlider("Alloc Scale", &allocScale, 0.f, 2.f, 0.01f, true))
	{
		m_emitParams.allocationScale = { allocScale, allocScale, allocScale };
	}
	imguiserSlider("Alloc Predict", &m_emitParams.allocationPredict, 0.f, 1.f, 0.01f, true);
	imguiFluidEmitterExtra();
	imguiserEndGroup();
}

void SceneFluid::imguiFluidAlloc()
{
	imguiSeparatorLine();
	imguiLabel("Fluid Allocation");
	imguiserBeginGroup("Fluid Allocation", nullptr);

	if (imguiserCheck("Big Effect Mode", m_flowGridActor.m_gridParams.bigEffectMode, true))
	{
		m_flowGridActor.m_gridParams.bigEffectMode = !m_flowGridActor.m_gridParams.bigEffectMode;
	}
	imguiserSlider("Predict Time", &m_flowGridActor.m_gridParams.bigEffectPredictTime, 0.f, 1.f, 0.01f, true);

	imguiserSlider("Velocity Weight", &m_flowGridActor.m_materialParams.velocity.allocWeight, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Smoke Weight", &m_flowGridActor.m_materialParams.smoke.allocWeight, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Temp Weight", &m_flowGridActor.m_materialParams.temperature.allocWeight, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel Weight", &m_flowGridActor.m_materialParams.fuel.allocWeight, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Velocity Threshold", &m_flowGridActor.m_materialParams.velocity.allocThreshold, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Smoke Threshold", &m_flowGridActor.m_materialParams.smoke.allocThreshold, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Temp Threshold", &m_flowGridActor.m_materialParams.temperature.allocThreshold, 0.f, 1.f, 0.01f, true);
	imguiserSlider("Fuel Threshold", &m_flowGridActor.m_materialParams.fuel.allocThreshold, 0.f, 1.f, 0.01f, true);

	imguiFluidAllocExtra();
	imguiserEndGroup();
}

void SceneFluid::imguiFluidTime()
{
	imguiSeparatorLine();
	imguiLabel("Simulation Update Time");
	{
		NvFlowQueryTime timeGPU, timeCPU;
		if (NvFlowGridQueryTime(m_flowGridActor.m_grid, &timeGPU, &timeCPU) == eNvFlowSuccess)
		{
			char buf[80];
			snprintf(buf, sizeof(buf), "GPU: %.3f ms", 1000.f * timeGPU.simulation);
			imguiValue(buf);
			snprintf(buf, sizeof(buf), "CPU: %.3f ms", 1000.f * timeCPU.simulation);
			imguiValue(buf);
		}
	}
	imguiFluidTimeExtra();
}

void SceneFluid::imguiLoadSave()
{
	imguiSeparatorLine();

	static float saveSlot = 0.f;
	imguiSlider("Save Slot", &saveSlot, 0.f, 7.f, 1.f, true);

	char buf[256u];
	snprintf(buf, 256u, "../../data/save%d.h", int(saveSlot));

	if (imguiButton("Save"))
	{
		imguiserSave(buf);
	}

	if (imguiButton("Load"))
	{
		imguiserLoad(buf);
	}
}

void SceneFluid::imgui(int xIn, int yIn, int wIn, int hIn)
{
	static int scroll = 0u;

	int border = xIn;
	int x = xIn;
	int y = border;
	int w = wIn;
	int h = m_winh - (hIn + border) - 2 * border;
	m_editorWidth = x + w;

	imguiBeginScrollArea("Options",
		x, y,
		w, h,
		&scroll);

	imguiserBeginGroup("Effect", nullptr);

	imguiDesc();

	imguiSeparatorLine();

	m_projectile.imgui();

	imguiFluidSim();

	imguiFluidEmitter();

	imguiFluidRender();

	imguiFluidAlloc();

	imguiLoadSave();

	imguiFluidTime();

	imguiEndScrollArea();

	m_flowGridActor.m_colorMap.imguiUpdate(this, m_flowContext.m_renderContext, border, x, y, w, h);

	imguiserEndGroup();
}

bool SceneFluid::imguiMouse(int mx, int my, unsigned char mbut)
{
	Scene::imguiMouse(mx, my, mbut);

	if (mx < m_editorWidth) return true;

	if (m_flowGridActor.m_colorMap.colorMapActive(mx, my, mbut)) return true;

	return false;
}

void SceneFluid::reset()
{
	release();
	SDL_Delay(30);
	init(m_appctx, m_winw, m_winh);
	m_shouldReset = false;
}

NvFlowUint64 SceneFluid::getGridGPUMemUsage()
{
	NvFlowUint64 totalBytes = 0u;

	if (m_flowGridActor.m_grid)
	{
		NvFlowGridGPUMemUsage(m_flowGridActor.m_grid, &totalBytes);
	}

	return totalBytes;
}

bool SceneFluid::getStats(int lineIdx, int statIdx, char* buf)
{
	switch (statIdx)
	{
		case 0:
		{
			NvFlowUint64 totalBytes = getGridGPUMemUsage();
			snprintf(buf, 79, "GridGPUMem: %llu bytes", totalBytes);
			return true;
		}
		case 1:
		{
			NvFlowUint numLayers = m_flowGridActor.m_statNumLayers;
			snprintf(buf, 79, "NumLayers: %d layers", numLayers);
			return true;
		}
		case 2:
		{
			NvFlowUint numBlocks = m_flowGridActor.m_statNumDensityBlocks;
			NvFlowUint maxBlocks = m_flowGridActor.m_statMaxDensityBlocks;
			snprintf(buf, 79, "Density: %d blocks active of %d", numBlocks, maxBlocks);
			return true;
		}
		case 3:
		{
			NvFlowUint numBlocks = m_flowGridActor.m_statNumVelocityBlocks;
			NvFlowUint maxBlocks = m_flowGridActor.m_statMaxVelocityBlocks;
			snprintf(buf, 79, "Velocity: %d blocks active of %d", numBlocks, maxBlocks);
			return true;
		}
		case 4:
		{
			NvFlowUint numCells = m_flowGridActor.m_statNumDensityCells;
			snprintf(buf, 79, "Density: %d cells active", numCells);
			return true;
		}
		case 5:
		{
			NvFlowUint numCells = m_flowGridActor.m_statNumVelocityCells;
			snprintf(buf, 79, "Velocity: %d cells active", numCells);
			return true;
		}
		case 6:
		{
			if (m_flowGridActor.m_statVolumeShadowBlocks > 0u)
			{
				NvFlowUint numBlocks = m_flowGridActor.m_statVolumeShadowBlocks;
				snprintf(buf, 79, "Shadow: %d blocks active", numBlocks);
				return true;
			}
			return false;
		}
		case 7:
		{
			if (m_flowGridActor.m_statVolumeShadowCells > 0u)
			{
				NvFlowUint numCells = m_flowGridActor.m_statVolumeShadowCells;
				snprintf(buf, 79, "Shadow: %d cells active", numCells);
				return true;
			}
			return false;
		}
		default:
		{
			return false;
		}
	}	
}

// ***************************** Projectile ********************************

void Projectile::init(AppGraphCtx* appctx, NvFlowContext* context)
{
	NvFlowGridEmitParamsDefaults(&m_emitParams);
	m_emitParams.bounds.x.x = m_radius;
	m_emitParams.bounds.y.y = m_radius;
	m_emitParams.bounds.z.z = m_radius;
	m_emitParams.bounds.w.y = 0.f;

	m_emitParams.velocityCoupleRate = { 10.f, 10.f, 10.f };

	m_emitParams.temperature = 0.f;
	m_emitParams.temperatureCoupleRate = 10.f;

	m_emitParams.fuel = 0.f;
	m_emitParams.fuelCoupleRate = 10.f;

	m_emitParams.smoke = 0.f;
	m_emitParams.smokeCoupleRate = 10.f;

	m_emitParams.allocationScale = { 0.f, 0.f, 0.f };

	updateEmitterMode();

	m_appctx = appctx;
	m_meshContext = MeshInteropContextCreate(appctx);
	m_mesh = MeshCreate(m_meshContext);

	MeshLoadFromFile(m_mesh, "../../data/sphere_high.ply");

	NvFlowShapeSDFDesc shapeDesc;
	NvFlowShapeSDFDescDefaults(&shapeDesc);
	shapeDesc.resolution = { 32u, 32u, 32u };
	m_shape = NvFlowCreateShapeSDF(context, &shapeDesc);

	// generate sphere SDF
	const float radius = 0.8f;
	auto mappedData = NvFlowShapeSDFMap(m_shape, context);
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
		NvFlowShapeSDFUnmap(m_shape, context);
	}
}

void Projectile::updatePosition(Path& path, float dt)
{
	if (path.m_active)
	{
		path.m_time -= dt;
		if (path.m_time > path.m_timeEnd)
		{
			float wx = path.m_direction[0];
			float wy = path.m_direction[1];
			float wz = path.m_direction[2];

			m_emitParams.smoke = 0.f;
			m_emitParams.temperature = m_temperature;
			m_emitParams.fuel = m_fuel;
			m_emitParams.velocityLinear.x = -m_speed * wx;
			m_emitParams.velocityLinear.y = -m_speed * wy;
			m_emitParams.velocityLinear.z = -m_speed * wz;
			float dist = m_speed * path.m_time;
			path.m_position[0] = dist * wx + path.m_offset[0];
			path.m_position[1] = dist * wy + path.m_offset[1];
			path.m_position[2] = dist * wz + path.m_offset[2];
			m_emitParams.bounds.w.x = path.m_position[0];
			m_emitParams.bounds.w.y = path.m_position[1];
			m_emitParams.bounds.w.z = path.m_position[2];
		}
		else
		{
			path.m_time = path.m_timeEnd;
			path.m_active = false;
		}
	}
}

void Projectile::update(NvFlowContext* context, NvFlowGrid* grid, float dt)
{
	int maxActive = -1;
	for (int i = 0; i < m_pathsSize; i++)
	{
		Path& path = m_paths[i];
		if (path.m_active)
		{
			maxActive = i;
			const int numSubSteps = 4;
			const float convert = 3.14159f / 180.f;
			float stepdt = dt / float(numSubSteps);
			for (int i = 0; i < numSubSteps; i++)
			{
				updatePosition(path, stepdt);

				NvFlowShapeDesc shapeDesc;
				shapeDesc.sphere.radius = 0.8f;

				m_emitParams.localToWorld = m_emitParams.bounds;
				m_emitParams.shapeType = eNvFlowShapeTypeSphere;
				m_emitParams.deltaTime = stepdt;

				NvFlowGridEmit(grid, &shapeDesc, 1u, &m_emitParams, 1u);
			}
		}
	}
	m_pathsSize = maxActive + 1;
}

void Projectile::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_view = view;
	m_projection = projection;

	MeshInteropContextUpdate(m_meshContext, m_appctx);

	if (m_shouldDrawMesh)
	{
		for (int i = 0; i < m_pathsSize; i++)
		{
			Path& path = m_paths[i];
			if (path.m_active)
			{
				MeshDrawParams meshDrawParams;
				meshDrawParams.renderMode = MESH_RENDER_SOLID;
				meshDrawParams.projection = projection;
				meshDrawParams.view = view;
				meshDrawParams.model = DirectX::XMMatrixMultiply(
					DirectX::XMMatrixScaling(0.8f * m_radius, 0.8f * m_radius, 0.8f * m_radius),
					DirectX::XMMatrixTranslation(path.m_position[0], path.m_position[1], path.m_position[2])
					);

				MeshDraw(m_mesh, &meshDrawParams);
			}
		}
	}
}

void Projectile::shoot()
{
	// find open path
	int allocIdx = 0;
	bool allocSuccess = false;
	for (; allocIdx < m_pathsCap; allocIdx++)
	{
		Path& path = m_paths[allocIdx];
		if (!path.m_active)
		{
			allocSuccess = true;
			break;
		}
	}
	if (allocSuccess)
	{
		// update size
		if (allocIdx + 1 > m_pathsSize)
		{
			m_pathsSize = allocIdx + 1;
		}
		// create path reference
		Path& path = m_paths[allocIdx];
		path.m_active = true;
		// compute direction
		{
			using namespace DirectX;
			// compute eye direction
			XMVECTOR screenDir = XMVectorSet(1.f, 0.f, 0.f, 0.f);
			XMMATRIX viewInv = XMMatrixInverse(nullptr, m_view);
			XMVECTOR worldDir = XMVector4Transform(screenDir, viewInv);
			worldDir = XMVector3Normalize(worldDir);

			XMMATRIX viewProj = XMMatrixMultiply(m_view, m_projection);
			XMMATRIX viewProjInv = XMMatrixInverse(nullptr, viewProj);

			float txp = 0.f;
			float txn = 0.f;
			{
				XMVECTOR pos = XMVectorSet(0.f, 0.f, 0.f, 1.f);
				XMVECTOR screencenter = XMVector4Transform(pos, viewProj);
				screencenter = screencenter / XMVectorSplatW(screencenter);

				pos = worldDir + XMVectorSet(0.f, 0.f, 0.f, 1.f);
				XMVECTOR screenpos = XMVector4Transform(pos, viewProj);
				screenpos = screenpos / XMVectorSplatW(screenpos);

				// compute world offset
				XMVECTOR offset = XMVectorSet(1.f, 0.f, 1.f, 1.f) * screencenter;
				offset = XMVector4Transform(offset, viewProjInv);
				offset = offset / XMVectorSplatW(offset);
				path.m_offset[0] = XMVectorGetX(offset);
				path.m_offset[1] = XMVectorGetY(offset);
				path.m_offset[2] = XMVectorGetZ(offset);

				float x0 = XMVectorGetX(screencenter);
				float x1 = XMVectorGetX(screenpos);
				txp = (+1.f - x0) / (x1 - x0);
				txn = (-1.f - x0) / (x1 - x0);
			}
			path.m_timeBegin = txp / m_speed;
			path.m_timeEnd = txn / m_speed;
			path.m_time = path.m_timeBegin;

			path.m_direction[0] = XMVectorGetX(worldDir);
			path.m_direction[1] = XMVectorGetY(worldDir);
			path.m_direction[2] = XMVectorGetZ(worldDir);
		}
		// set zeroth position
		updatePosition(path, 0.f);
	}
}

void Projectile::updateEmitterMode()
{
	if (m_fireBallMode)
	{
		if (m_prediction)
		{
			m_emitParams.allocationScale = { 1.f, 1.f, 1.f };
			m_emitParams.allocationPredict = 0.4f;
		}
		else
		{
			m_emitParams.allocationScale = { 1.f, 1.f, 1.f };
			m_emitParams.allocationPredict = 0.f;
		}
		m_temperature = 2.f;
		m_fuel = 2.f;
	}
	else
	{
		m_emitParams.allocationScale = { 0.f, 0.f, 0.f };
		m_emitParams.allocationPredict = 0.f;
		m_temperature = 10.f;
		m_fuel = 0.f;
	}
}

void Projectile::imgui()
{
	if (imguiButton("Shoot", true))
	{
		shoot();
	}
	if (imguiCheck("Fire Ball", m_fireBallMode))
	{
		m_fireBallMode = !m_fireBallMode;
		updateEmitterMode();
	}
	if (imguiSlider("Radius", &m_radius, 0.f, 1.f, 0.01f, true))
	{
		m_emitParams.bounds.x.x = m_radius;
		m_emitParams.bounds.y.y = m_radius;
		m_emitParams.bounds.z.z = m_radius;
	}
	imguiSlider("Speed", &m_speed, 0.f, 32.f, 0.1f, true);
	imguiSlider("Temperature", &m_temperature, 0.f, 10.f, 0.1f, true);
	if (m_fireBallMode)
	{	
		imguiSlider("Fuel", &m_fuel, 0.f, 10.f, 0.1f, true);
		if (imguiCheck("Prediction", m_prediction))
		{
			m_prediction = !m_prediction;
			updateEmitterMode();
		}
	}
	if (imguiCheck("Draw Projectile", m_shouldDrawMesh))
	{
		m_shouldDrawMesh = !m_shouldDrawMesh;
	}
}

void Projectile::release()
{
	// set all projectiles inactive
	for (int i = 0; i < m_pathsSize; i++)
	{
		Path& path = m_paths[i];
		path.m_active = false;
	}

	NvFlowReleaseShapeSDF(m_shape);

	MeshRelease(m_mesh);
	MeshContextRelease(m_meshContext);
}