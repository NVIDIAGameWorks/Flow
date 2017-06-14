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

#include "computeContext.h"

namespace
{
	// Need BYTE defined for shader bytecode
	typedef unsigned char       BYTE;
	#include "customEmitAllocCS.hlsl.h"
	#include "customEmitEmitCS.hlsl.h"
	#include "customEmitEmit2CS.hlsl.h"
}

void SceneCustomEmit::initParams()
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

	// grid parameter overrides
	m_flowGridActor.m_gridParams.gravity = NvFlowFloat3{ 0.f, -1.f, 0.f };

	m_shouldLoadPreset = true;
}

void SceneCustomEmit::init(AppGraphCtx* appctx, int winw, int winh)
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

	// create app compute resources
	{
		m_customContext = ComputeContextNvFlowContextCreate(m_flowContext.m_gridContext);

		ComputeShaderDesc shaderDesc = {};
		shaderDesc.cs = g_customEmitAllocCS;
		shaderDesc.cs_length = sizeof(g_customEmitAllocCS);
		m_customEmitAllocCS = ComputeShaderCreate(m_customContext, &shaderDesc);

		shaderDesc.cs = g_customEmitEmitCS;
		shaderDesc.cs_length = sizeof(g_customEmitEmitCS);
		m_customEmitEmitCS = ComputeShaderCreate(m_customContext, &shaderDesc);

		shaderDesc.cs = g_customEmitEmit2CS;
		shaderDesc.cs_length = sizeof(g_customEmitEmit2CS);
		m_customEmitEmit2CS = ComputeShaderCreate(m_customContext, &shaderDesc);

		ComputeConstantBufferDesc cbDesc = {};
		cbDesc.sizeInBytes = 1024 * sizeof(float);
		m_customConstantBuffer = ComputeConstantBufferCreate(m_customContext, &cbDesc);
	}

	// register callbacks
	{
		NvFlowGridEmitCustomRegisterAllocFunc(m_flowGridActor.m_grid, emitCustomAllocFunc, this);
		NvFlowGridEmitCustomRegisterEmitFunc(m_flowGridActor.m_grid, eNvFlowGridTextureChannelVelocity, emitCustomEmitVelocityFunc, this);
		NvFlowGridEmitCustomRegisterEmitFunc(m_flowGridActor.m_grid, eNvFlowGridTextureChannelDensity, emitCustomEmitDensityFunc, this);
	}
}

void SceneCustomEmit::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin(dt);
	if (shouldUpdate)
	{
		ComputeContextNvFlowContextUpdate(m_customContext, m_flowContext.m_gridContext);

		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		m_flowGridActor.updatePreEmit(&m_flowContext, dt);

		NvFlowShapeDesc shapeDesc;
		shapeDesc.sphere.radius = 0.8f;

		m_emitParams.localToWorld = m_emitParams.bounds;
		m_emitParams.shapeType = eNvFlowShapeTypeSphere;
		m_emitParams.deltaTime = dt;

		// Disable traditional emitters here
		//NvFlowGridEmit(m_grid, &shapeDesc, 1u, &m_emitParams, 1u);

		m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);

		m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

		m_shouldGridReset = false;

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

namespace
{
	void updateResource(ComputeResource*& computeResource, NvFlowResource* flowResource, ComputeContext* customContext, NvFlowContext* flowContext)
	{
		if (computeResource) {
			ComputeResourceNvFlowUpdate(customContext, computeResource, flowContext, flowResource);
		}
		else {
			computeResource = ComputeResourceNvFlowCreate(customContext, flowContext, flowResource);
		}
	}
	void updateResourceRW(ComputeResourceRW*& computeResourceRW, NvFlowResourceRW* flowResourceRW, ComputeContext* customContext, NvFlowContext* flowContext)
	{
		if (computeResourceRW) {
			ComputeResourceRWNvFlowUpdate(customContext, computeResourceRW, flowContext, flowResourceRW);
		}
		else {
			computeResourceRW = ComputeResourceRWNvFlowCreate(customContext, flowContext, flowResourceRW);
		}
	}
}

void SceneCustomEmit::doEmitCustomAllocFunc(const NvFlowGridEmitCustomAllocParams* params)
{
	updateResourceRW(m_allocMask, params->maskResourceRW, m_customContext, m_flowContext.m_renderContext);

	struct ShaderParams
	{
		NvFlowUint4 minMaskIdx;
		NvFlowUint4 maxMaskIdx;
	};

	NvFlowDim maskDim = params->maskDim;

	NvFlowUint radius = 1u;

	NvFlowUint4 minMaskIdx, maxMaskIdx;

	minMaskIdx.x = maskDim.x / 2 - radius;
	minMaskIdx.y = maskDim.y / 2 - radius;
	minMaskIdx.z = maskDim.z / 2 - radius;
	minMaskIdx.w = 0u;

	maxMaskIdx.x = maskDim.x / 2 + radius + 1;
	maxMaskIdx.y = maskDim.y / 2 + radius + 1;
	maxMaskIdx.z = maskDim.z / 2 + radius + 1;
	maxMaskIdx.w = 0u;

	auto mapped = (ShaderParams*)ComputeConstantBufferMap(m_customContext, m_customConstantBuffer);

	mapped->minMaskIdx = minMaskIdx;
	mapped->maxMaskIdx = maxMaskIdx;

	ComputeConstantBufferUnmap(m_customContext, m_customConstantBuffer);

	ComputeDispatchParams dparams = {};
	dparams.shader = m_customEmitAllocCS;
	dparams.constantBuffer = m_customConstantBuffer;
	dparams.gridDim[0] = (maxMaskIdx.x - minMaskIdx.x + 7) / 8;
	dparams.gridDim[1] = (maxMaskIdx.y - minMaskIdx.y + 7) / 8;
	dparams.gridDim[2] = (maxMaskIdx.z - minMaskIdx.z + 7) / 8;
	dparams.resourcesRW[0] = m_allocMask;

	ComputeContextDispatch(m_customContext, &dparams);
}

void SceneCustomEmit::doEmitCustomEmit(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* layeredParams, const CustomEmitParams* customParams)
{
	// for each layer
	for (NvFlowUint layerIdx = 0u; layerIdx < layeredParams->numLayers; layerIdx++)
	{
		NvFlowGridEmitCustomEmitLayerParams paramsInst = {};
		NvFlowGridEmitCustomGetLayerParams(layeredParams, 0u, &paramsInst);
		auto params = &paramsInst;

		updateResource(m_blockTable, params->blockTable, m_customContext, m_flowContext.m_renderContext);
		updateResource(m_blockList, params->blockList, m_customContext, m_flowContext.m_renderContext);
		updateResourceRW(m_dataRW[0u], params->dataRW[0u], m_customContext, m_flowContext.m_renderContext);
		updateResourceRW(m_dataRW[1u], params->dataRW[1u], m_customContext, m_flowContext.m_renderContext);

		struct ShaderParams
		{
			NvFlowShaderPointParams customEmitParams;

			NvFlowUint4 minVidx;
			NvFlowUint4 maxVidx;
			NvFlowFloat4 targetValue;
			NvFlowFloat4 blendRate;
		};

		const auto& gridDim = params->shaderParams.gridDim;
		const auto& blockDim = params->shaderParams.blockDim;

		NvFlowUint4 vdim;
		vdim.x = gridDim.x * blockDim.x;
		vdim.y = gridDim.y * blockDim.y;
		vdim.z = gridDim.z * blockDim.z;
		vdim.w = 1u;

		const NvFlowUint radius = customParams->radius;

		NvFlowUint4 minVidx, maxVidx;

		minVidx.x = vdim.x / 2 - radius;
		minVidx.y = vdim.y / 2 - radius;
		minVidx.z = vdim.z / 2 - radius;
		minVidx.w = 0u;

		maxVidx.x = vdim.x / 2 + radius + 1;
		maxVidx.y = vdim.y / 2 + radius + 1;
		maxVidx.z = vdim.z / 2 + radius + 1;
		maxVidx.w = 0u;

		auto mapped = (ShaderParams*)ComputeConstantBufferMap(m_customContext, m_customConstantBuffer);

		mapped->customEmitParams = params->shaderParams;

		mapped->minVidx = minVidx;
		mapped->maxVidx = maxVidx;
		mapped->targetValue = customParams->targetValue;
		mapped->blendRate = customParams->blendRate;

		ComputeConstantBufferUnmap(m_customContext, m_customConstantBuffer);

		if (m_fullDomain)
		{
			ComputeDispatchParams dparams = {};
			dparams.shader = m_customEmitEmit2CS;
			dparams.constantBuffer = m_customConstantBuffer;
			dparams.gridDim[0] = (params->numBlocks * params->shaderParams.blockDim.x + 7) / 8;
			dparams.gridDim[1] = (params->shaderParams.blockDim.y + 7) / 8;
			dparams.gridDim[2] = (params->shaderParams.blockDim.z + 7) / 8;
			dparams.resources[0] = m_blockList;
			dparams.resources[1] = m_blockTable;

			NvFlowUint localDataFrontIdx = *dataFrontIdx;
			// single pass
			{
				dparams.resources[2] = ComputeResourceRWGetResource(m_dataRW[localDataFrontIdx]);
				dparams.resourcesRW[0] = m_dataRW[localDataFrontIdx ^ 1u];
				ComputeContextDispatch(m_customContext, &dparams);
				localDataFrontIdx = localDataFrontIdx ^ 1u;
			}
		}
		else
		{
			ComputeDispatchParams dparams = {};
			dparams.shader = m_customEmitEmitCS;
			dparams.constantBuffer = m_customConstantBuffer;
			dparams.gridDim[0] = (maxVidx.x - minVidx.x + 7) / 8;
			dparams.gridDim[1] = (maxVidx.y - minVidx.y + 7) / 8;
			dparams.gridDim[2] = (maxVidx.z - minVidx.z + 7) / 8;
			dparams.resources[0] = m_blockList;
			dparams.resources[1] = m_blockTable;

			NvFlowUint localDataFrontIdx = *dataFrontIdx;
			// doing this twice allow the write to be incomplete
			for (int i = 0; i < 2; i++)
			{
				dparams.resources[2] = ComputeResourceRWGetResource(m_dataRW[localDataFrontIdx]);
				dparams.resourcesRW[0] = m_dataRW[localDataFrontIdx ^ 1u];
				ComputeContextDispatch(m_customContext, &dparams);
				localDataFrontIdx = localDataFrontIdx ^ 1u;
			}
		}
	}
	// dataFrontIdx must be uniform across layers
	if (m_fullDomain)
	{
		(*dataFrontIdx) = (*dataFrontIdx) ^ 1u;
	}
	else
	{
		(*dataFrontIdx) = (*dataFrontIdx) ^ 1u;
		(*dataFrontIdx) = (*dataFrontIdx) ^ 1u;
	}
}

void SceneCustomEmit::doEmitCustomEmitVelocityFunc(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params)
{
	CustomEmitParams customParams = {};
	customParams.radius = 4u;
	customParams.targetValue = { 0.f, 1.f, 0.f, 0.f };
	customParams.blendRate = { 0.01f, 0.01f, 0.01f, 0.01f };

	doEmitCustomEmit(dataFrontIdx, params, &customParams);
}

void SceneCustomEmit::doEmitCustomEmitDensityFunc(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params)
{
	CustomEmitParams customParams = {};
	customParams.radius = 8u;
	customParams.targetValue = { 1.f, 1.f, 0.f, 1.f };
	customParams.blendRate = { 0.01f, 0.01f, 0.01f, 0.01f };

	doEmitCustomEmit(dataFrontIdx, params, &customParams);
}

void SceneCustomEmit::emitCustomAllocFunc(void* userdata, const NvFlowGridEmitCustomAllocParams* params)
{
	((SceneCustomEmit*)(userdata))->doEmitCustomAllocFunc(params);
}

void SceneCustomEmit::emitCustomEmitVelocityFunc(void* userdata, NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params)
{
	((SceneCustomEmit*)(userdata))->doEmitCustomEmitVelocityFunc(dataFrontIdx, params);
}

void SceneCustomEmit::emitCustomEmitDensityFunc(void* userdata, NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params)
{
	((SceneCustomEmit*)(userdata))->doEmitCustomEmitDensityFunc(dataFrontIdx, params);
}

void SceneCustomEmit::preDraw()
{
	m_flowContext.preDrawBegin();

	m_flowGridActor.preDraw(&m_flowContext);

	m_flowContext.preDrawEnd();
}

void SceneCustomEmit::imguiFluidEmitterExtra()
{
	if (imguiserCheck("Full Domain", m_fullDomain, true))
	{
		m_fullDomain = !m_fullDomain;
	}
}

void SceneCustomEmit::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_projectile.draw(projection, view);

	m_flowContext.drawBegin();

	m_flowGridActor.draw(&m_flowContext, projection, view);

	m_flowContext.drawEnd();
}

void SceneCustomEmit::release()
{
	// release app compute resources
	{
		ComputeConstantBufferRelease(m_customConstantBuffer);

		ComputeShaderRelease(m_customEmitAllocCS);
		ComputeShaderRelease(m_customEmitEmitCS);
		ComputeShaderRelease(m_customEmitEmit2CS);

		ComputeContextRelease(m_customContext);

		if (m_allocMask) ComputeResourceRWRelease(m_allocMask);
		if (m_blockTable) ComputeResourceRelease(m_blockTable);
		if (m_blockList) ComputeResourceRelease(m_blockList);
		if (m_dataRW[0]) ComputeResourceRWRelease(m_dataRW[0u]);
		if (m_dataRW[1]) ComputeResourceRWRelease(m_dataRW[1u]);

		m_allocMask = nullptr;
		m_blockTable = nullptr;
		m_blockList = nullptr;
		m_dataRW[0] = nullptr;
		m_dataRW[1] = nullptr;
	}

	m_projectile.release();

	m_flowGridActor.release();

	m_flowContext.release();
}

void SceneCustomEmit::imgui(int xIn, int yIn, int wIn, int hIn)
{
	SceneFluid::imgui(xIn, yIn, wIn, hIn);

	if (m_shouldLoadPreset)
	{
		imguiserLoadC(PresetFlame::g_root, sizeof(PresetFlame::g_root));
		m_shouldLoadPreset = false;
	}
}