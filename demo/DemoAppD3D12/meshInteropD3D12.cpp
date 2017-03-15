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

MeshContext* MeshInteropContextCreate(AppGraphCtx* appctx)
{
	MeshContextDesc desc = {};
	desc.device = appctx->m_device;
	desc.commandList = appctx->m_commandList;

	return MeshContextCreate(&desc);
}

void MeshInteropContextUpdate(MeshContext* context, AppGraphCtx* appctx)
{
	MeshContextDesc desc = {};
	desc.device = appctx->m_device;
	desc.commandList = appctx->m_commandList;

	return MeshContextUpdate(context, &desc);
}