// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2014-2021 NVIDIA Corporation. All rights reserved.

#include <stdio.h>
#include <string.h>

#include "loader.h"
#include "imgui.h"
#include "imguiser.h"

namespace Preset0
{
#include "preset0.h"
}
namespace Preset1
{
#include "preset1.h"
}

#include "scene.h"

#include <SDL.h>

void SceneSDFTest::initParams()
{
	m_flowGridActor.initParams(AppGraphCtxDedicatedVideoMemory(m_appctx));

	// set emitter defaults
	NvFlowGridEmitParamsDefaults(&m_emitParams);

	m_emitParams.bounds.x.x = 1.5f;
	m_emitParams.bounds.y.y = 1.5f;
	m_emitParams.bounds.z.z = 1.5f;
	m_emitParams.velocityLinear.y = -8.f;
	m_emitParams.fuel = 1.4f;
	m_emitParams.smoke = 0.5f;

	// grid parameter overrides
	m_flowGridActor.m_separateLighting = false;			// disable separate lighting, lower quality due to nonlinear colormap

	m_flowGridActor.m_gridDesc.halfSize.x = 16.f;
	m_flowGridActor.m_gridDesc.halfSize.y = 16.f;
	m_flowGridActor.m_gridDesc.halfSize.z = 16.f;

	m_flowGridActor.m_materialParams.smoke.damping = 0.25f;
	m_flowGridActor.m_materialParams.smoke.fade = 0.25f;
	m_flowGridActor.m_renderMaterialDefaultParams.alphaScale = 0.2f;

	m_flowGridActor.m_gridParams.gravity = NvFlowFloat3{ 0.f, -0.5f, 0.f };

	m_shouldLoadPreset = true;
}

void SceneSDFTest::init(AppGraphCtx* appctx, int winw, int winh)
{
	using namespace DirectX;

	m_appctx = appctx;

	if (!m_shouldReset || m_isFirstRun)
	{
		initParams();
		m_isFirstRun = false;
	}

	m_flowContext.init(appctx);

	m_flowGridActor.init(&m_flowContext, appctx);

	NvFlowSDFGenDesc sdfDesc;
	sdfDesc.resolution = { 128u, 128u, 128u };

	m_sdfGen = NvFlowCreateSDFGen(m_flowContext.m_gridContext, &sdfDesc);

	m_meshContext = MeshInteropContextCreate(appctx);
	m_mesh = MeshCreate(m_meshContext);

	MeshLoadFromFile(m_mesh, "../../data/bunny.ply");

	MeshData meshData;
	MeshGetData(m_mesh, &meshData);

	NvFlowSDFGenReset(m_sdfGen, m_flowContext.m_gridContext);

	XMMATRIX modelMatrix = XMMatrixMultiply(
		XMMatrixScaling(8.f, 8.f, 8.f),
		XMMatrixTranslation(0.f, -0.75f, 0.f)
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
	m_shape = NvFlowCreateShapeSDFFromTexture3D(
		m_flowContext.m_gridContext, 
		NvFlowSDFGenShape(m_sdfGen, m_flowContext.m_gridContext)
	);

	// create default color map
	{
		const int numPoints = 5;
		const CurvePoint pts[numPoints] = {
			{0.f,0.f,0.f,0.f,0.f},
			{0.05f,0.2f,0.2f,0.2f,0.25f},
			{0.6f,0.35f * 141.f / 255.f, 0.1f * 199.f / 255.f, 0.7f * 63.f / 255.f,0.8f},
			{0.85f,0.75f * 141.f / 255.f, 0.15f * 199.f / 255.f, 1.f * 63.f / 255.f,0.8f},
			{1.f,1.25f * 141.f / 255.f, 0.2f * 199.f / 255.f, 3.f * 63.f / 255.f,0.5f}
		};

		auto& colorMap = m_flowGridActor.m_colorMap;
		colorMap.initColorMap(m_flowContext.m_renderContext, pts, numPoints, (colorMap.m_curvePointsDefault.size() == 0));
	}

	m_projectile.init(m_appctx, m_flowContext.m_gridContext);

	resize(winw, winh);
}

void SceneSDFTest::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin(dt);
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		{
			m_flowGridActor.updatePreEmit(&m_flowContext, dt);

			NvFlowShapeDesc shapeDesc;
			shapeDesc.sdf.sdfOffset = 0u; // m_shape;

			m_emitParams.localToWorld = m_emitParams.bounds;
			m_emitParams.shapeType = eNvFlowShapeTypeSDF;
			m_emitParams.deltaTime = dt;

			NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);

			NvFlowShapeSDF* sdfs[] = { m_shape };
			NvFlowGridUpdateEmitSDFs(m_flowGridActor.m_grid, sdfs, 1u);

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);

			m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

			m_shouldGridReset = false;
		}

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void SceneSDFTest::preDraw()
{
	m_flowContext.preDrawBegin();

	m_flowGridActor.preDraw(&m_flowContext);

	m_flowContext.preDrawEnd();
}

void SceneSDFTest::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_projectile.draw(projection, view);

	//MeshInteropContextUpdate(m_meshContext, m_appctx);

	//MeshDrawParams meshDrawParams;
	//meshDrawParams.renderMode = MESH_RENDER_SOLID;
	//meshDrawParams.projection = projection;
	//meshDrawParams.view = view;
	//meshDrawParams.model = DirectX::XMMatrixMultiply(
	//	DirectX::XMMatrixScaling(0.25f, 0.25f, 0.25f),
	//	DirectX::XMMatrixTranslation(0.f, 0.0f, 0.f)
	//	);

	//MeshDraw(m_mesh, &meshDrawParams);

	m_flowContext.drawBegin();

#if 0
	memcpy(&m_renderParams.projectionMatrix, &projection, sizeof(m_renderParams.projectionMatrix));
	memcpy(&m_renderParams.viewMatrix, &view, sizeof(m_renderParams.viewMatrix));
	m_renderParams.modelMatrix = {
		1.5f, 0.f, 0.f, 0.f,
		0.f, 1.5f, 0.f, 0.f,
		0.f, 0.f, 1.5f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	m_renderParams.depthStencilView = m_dsv;
	m_renderParams.renderTargetView = m_rtv;

	auto texture = NvFlowSDFGenShape(m_sdfGen, m_context);

	NvFlowVolumeRenderTexture3D(m_volumeRender, m_context, texture, &m_renderParams);
#endif

	m_flowGridActor.draw(&m_flowContext, projection, view);

	m_flowContext.drawEnd();
}

void SceneSDFTest::release()
{
	m_projectile.release();

	m_flowGridActor.release();

	NvFlowReleaseShapeSDF(m_shape);

	NvFlowReleaseSDFGen(m_sdfGen);

	m_flowContext.release();

	MeshRelease(m_mesh);
	MeshContextRelease(m_meshContext);
}

void SceneSDFTest::imgui(int xIn, int yIn, int wIn, int hIn)
{
	SceneFluid::imgui(xIn, yIn, wIn, hIn);

	if (m_shouldLoadPreset)
	{
		imguiserLoadC(Preset0::g_root, sizeof(Preset0::g_root));
		m_shouldLoadPreset = false;
	}
}

// ************************** Scene Custom Lighting ******************************

#include "computeContext.h"

namespace
{
	// Need BYTE defined for shader bytecode
	typedef unsigned char       BYTE;
	#include "customLightingCS.hlsl.h"
}

void SceneCustomLighting::init(AppGraphCtx* context, int winw, int winh)
{
	SceneSDFTest::init(context, winw, winh);

	auto gridExport = NvFlowGridProxyGetGridExport(m_flowGridActor.m_gridProxy, m_flowContext.m_renderContext);

	NvFlowGridImportDesc importDesc = {};
	importDesc.gridExport = gridExport;

	m_import = NvFlowCreateGridImport(m_flowContext.m_renderContext, &importDesc);

	// create compute resources
	m_computeContext = ComputeContextNvFlowContextCreate(m_flowContext.m_renderContext);

	ComputeShaderDesc shaderDesc = {};
	shaderDesc.cs = g_customLightingCS;
	shaderDesc.cs_length = sizeof(g_customLightingCS);
	m_computeShader = ComputeShaderCreate(m_computeContext, &shaderDesc);

	ComputeConstantBufferDesc cbDesc = {};
	cbDesc.sizeInBytes = 1024 * sizeof(float);
	m_computeConstantBuffer = ComputeConstantBufferCreate(m_computeContext, &cbDesc);
}

void SceneCustomLighting::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin(dt);
	if (shouldUpdate)
	{
		ComputeContextNvFlowContextUpdate(m_computeContext, m_flowContext.m_renderContext);

		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		if (shouldUpdate)
		{
			m_flowGridActor.updatePreEmit(&m_flowContext, dt);

			NvFlowShapeDesc shapeDesc;
			shapeDesc.sdf.sdfOffset = 0u; // m_shape;

			m_emitParams.localToWorld = m_emitParams.bounds;
			m_emitParams.shapeType = eNvFlowShapeTypeSDF;
			m_emitParams.deltaTime = dt;

			NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);

			NvFlowShapeSDF* sdfs[] = { m_shape };
			NvFlowGridUpdateEmitSDFs(m_flowGridActor.m_grid, sdfs, 1u);

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);

			m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

			m_shouldGridReset = false;
		}

		AppGraphCtxProfileEnd(m_appctx, "Simulate");

		m_time += dt;
	}
	m_flowContext.updateEnd();
}

void SceneCustomLighting::preDraw()
{
	m_flowContext.preDrawBegin();

	//auto gridView = NvFlowGridGetGridView(m_grid, m_context);

	AppGraphCtxProfileBegin(m_appctx, "UpdateGridView");

	NvFlowGridProxyFlushParams flushParams = {};
	flushParams.gridContext = m_flowContext.m_gridContext;
	flushParams.gridCopyContext = m_flowContext.m_gridCopyContext;
	flushParams.renderCopyContext = m_flowContext.m_renderCopyContext;
	NvFlowGridProxyFlush(m_flowGridActor.m_gridProxy, &flushParams);

	auto gridExport = NvFlowGridProxyGetGridExport(m_flowGridActor.m_gridProxy, m_flowContext.m_renderContext);

	AppGraphCtxProfileEnd(m_appctx, "UpdateGridView");

	AppGraphCtxProfileBegin(m_appctx, "CustomLighting");

	// Only layer 0 for the moment
	NvFlowUint layerIdx = 0u;

	NvFlowGridExportHandle exportHandle = NvFlowGridExportGetHandle(gridExport, m_flowContext.m_renderContext, eNvFlowGridTextureChannelDensity);
	NvFlowGridExportLayeredView exportLayeredView;
	NvFlowGridExportGetLayeredView(exportHandle, &exportLayeredView);
	NvFlowGridExportLayerView exportLayerView;
	NvFlowGridExportGetLayerView(exportHandle, layerIdx, &exportLayerView);

	NvFlowGridImportParams importParams = {};
	importParams.gridExport = gridExport;
	importParams.channel = eNvFlowGridTextureChannelDensity;
	importParams.importMode = eNvFlowGridImportModePoint;
	NvFlowGridImportHandle importHandle = NvFlowGridImportGetHandle(m_import, m_flowContext.m_renderContext, &importParams);
	NvFlowGridImportLayeredView importLayeredView;
	NvFlowGridImportGetLayeredView(importHandle, &importLayeredView);
	NvFlowGridImportLayerView importLayerView;
	NvFlowGridImportGetLayerView(importHandle, layerIdx, &importLayerView);

	// convert resource from NvFlow types to ComputeResource types
	{
		auto updateResource = [&](ComputeResource*& computeResouce, NvFlowResource*& flowResource)
		{
			if (computeResouce) {
				ComputeResourceNvFlowUpdate(m_computeContext, computeResouce, m_flowContext.m_renderContext, flowResource);
			}
			else {
				computeResouce = ComputeResourceNvFlowCreate(m_computeContext, m_flowContext.m_renderContext, flowResource);
			}
		};
		auto updateResourceRW = [&](ComputeResourceRW*& computeResouceRW, NvFlowResourceRW*& flowResourceRW)
		{
			if (computeResouceRW) {
				ComputeResourceRWNvFlowUpdate(m_computeContext, computeResouceRW, m_flowContext.m_renderContext, flowResourceRW);
			}
			else {
				computeResouceRW = ComputeResourceRWNvFlowCreate(m_computeContext, m_flowContext.m_renderContext, flowResourceRW);
			}
		};
		updateResource(m_exportBlockList, exportLayerView.mapping.blockList);
		updateResource(m_exportBlockTable, exportLayerView.mapping.blockTable);
		updateResource(m_exportData, exportLayerView.data);
		updateResource(m_importBlockList, importLayerView.mapping.blockList);
		updateResource(m_importBlockTable, importLayerView.mapping.blockTable);
		updateResourceRW(m_importDataRW, importLayerView.dataRW);
	}

	// dispatch custom lighting operation
	{
		struct Light
		{
			NvFlowFloat4 location;
			NvFlowFloat4 intensity;
			NvFlowFloat4 bias;
			NvFlowFloat4 falloff;
		};

		struct ComputeShaderParams
		{
			NvFlowShaderLinearParams exportParams;
			NvFlowShaderLinearParams importParams;

			Light light[3];
		};

		auto mapped = (ComputeShaderParams*)ComputeConstantBufferMap(m_computeContext, m_computeConstantBuffer);

		mapped->exportParams = exportLayeredView.mapping.shaderParams;
		mapped->importParams = importLayeredView.mapping.shaderParams;

		if (m_time >= DirectX::XM_2PI)
		{
			m_time -= DirectX::XM_2PI;
		}
		const float radius = 0.025f;
		const float a = radius * cosf(m_time);
		const float b = radius * sinf(m_time);

		mapped->light[0].location = { a, b, 0.f, 1.f };
		mapped->light[0].intensity = { 1.25f, 0.f, 0.f, 1.f };
		mapped->light[0].bias = { 0.1f, 0.1f, 0.1f, 0.1f };
		mapped->light[0].falloff = { 200.f, 200.f, 200.f, 0.f };
		mapped->light[1].location = { 0.f, a, b, 1.f };
		mapped->light[1].intensity = { 0.f, 1.25f, 0.f, 1.f };
		mapped->light[1].bias = { 0.1f, 0.1f, 0.1f, 0.1f };
		mapped->light[1].falloff = { 200.f, 200.f, 200.f, 0.f };
		mapped->light[2].location = { b, 0.f, a, 1.f };
		mapped->light[2].intensity = { 0.f, 0.f, 1.1f, 1.f };
		mapped->light[2].bias = { 0.1f, 0.1f, 0.1f, 0.1f };
		mapped->light[2].falloff = { 200.f, 200.f, 200.f, 0.f };

		ComputeConstantBufferUnmap(m_computeContext, m_computeConstantBuffer);

		ComputeDispatchParams dparams = {};
		dparams.shader = m_computeShader;
		dparams.constantBuffer = m_computeConstantBuffer;
		dparams.gridDim[0] = (importLayerView.mapping.numBlocks * importLayeredView.mapping.shaderParams.blockDim.x + 7) / 8;
		dparams.gridDim[1] = (importLayeredView.mapping.shaderParams.blockDim.y + 7) / 8;
		dparams.gridDim[2] = (importLayeredView.mapping.shaderParams.blockDim.z + 7) / 8;
		dparams.resources[0] = m_exportBlockList;
		dparams.resources[1] = m_exportBlockTable;
		dparams.resources[2] = m_exportData;
		dparams.resources[3] = m_importBlockList;
		dparams.resources[4] = m_importBlockTable;
		dparams.resourcesRW[0] = m_importDataRW;

		ComputeContextDispatch(m_computeContext, &dparams);
	}

	AppGraphCtxProfileEnd(m_appctx, "CustomLighting");

	AppGraphCtxProfileBegin(m_appctx, "CustomImport");

	// override original gridExport
	gridExport = NvFlowGridImportGetGridExport(m_import, m_flowContext.m_renderContext);

	AppGraphCtxProfileEnd(m_appctx, "CustomImport");

	m_flowGridActor.m_gridExportOverride = gridExport;
	m_flowGridActor.m_renderParamsOverride = m_flowGridActor.m_renderParams;

	m_flowContext.preDrawEnd();
}

void SceneCustomLighting::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_projectile.draw(projection, view);

	m_flowContext.drawBegin();

	m_flowGridActor.draw(&m_flowContext, projection, view);

	m_flowContext.drawEnd();
}

void SceneCustomLighting::release()
{
	NvFlowReleaseGridImport(m_import);

	// release compute resources
	ComputeConstantBufferRelease(m_computeConstantBuffer);
	ComputeShaderRelease(m_computeShader);
	ComputeContextRelease(m_computeContext);

	ComputeResourceRelease(m_exportBlockList);
	ComputeResourceRelease(m_exportBlockTable);
	ComputeResourceRelease(m_exportData);
	ComputeResourceRelease(m_importBlockList);
	ComputeResourceRelease(m_importBlockTable);
	ComputeResourceRWRelease(m_importDataRW);

	m_exportBlockList = nullptr;
	m_exportBlockTable = nullptr;
	m_exportData = nullptr;
	m_importBlockList = nullptr;
	m_importBlockTable = nullptr;
	m_importDataRW = nullptr;

	SceneSDFTest::release();
}

void SceneCustomLighting::imgui(int xIn, int yIn, int wIn, int hIn)
{
	bool shouldLoadPreset = m_shouldLoadPreset;
	m_shouldLoadPreset = false;
	SceneSDFTest::imgui(xIn, yIn, wIn, hIn);
	m_shouldLoadPreset = shouldLoadPreset;
	if (m_shouldLoadPreset)
	{
		imguiserLoadC(Preset1::g_root, sizeof(Preset1::g_root));
		m_shouldLoadPreset = false;
	}
}

void SceneCustomLighting::initParams()
{
	SceneSDFTest::initParams();
}