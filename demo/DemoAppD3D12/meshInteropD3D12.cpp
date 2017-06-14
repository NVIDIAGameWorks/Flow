/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

//direct3d headers
#include <d3d12.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d12.lib")

#include "../DemoApp/meshInterop.h"

#include "appD3D12Ctx.h"
#include "meshD3D12.h"

MESH_API MeshContext* MeshInteropContextCreateD3D12(AppGraphCtx* appctx);

MESH_API void MeshInteropContextUpdateD3D12(MeshContext* context, AppGraphCtx* appctx);

MeshContext* MeshInteropContextCreateD3D12(AppGraphCtx* appctxIn)
{
	auto appctx = cast_to_AppGraphCtxD3D12(appctxIn);

	MeshContextDescD3D12 desc = {};
	desc.device = appctx->m_device;
	desc.commandList = appctx->m_commandList;

	return MeshContextCreateD3D12(cast_from_MeshContextDescD3D12(&desc));
}

void MeshInteropContextUpdateD3D12(MeshContext* context, AppGraphCtx* appctxIn)
{
	auto appctx = cast_to_AppGraphCtxD3D12(appctxIn);

	MeshContextDescD3D12 desc = {};
	desc.device = appctx->m_device;
	desc.commandList = appctx->m_commandList;

	return MeshContextUpdateD3D12(context, cast_from_MeshContextDescD3D12(&desc));
}