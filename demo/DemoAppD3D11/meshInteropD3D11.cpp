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

//direct3d headers
#include <d3d11.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")

#include "../DemoApp/meshInterop.h"

#include "appD3D11Ctx.h"
#include "meshD3D11.h"

MESH_API MeshContext* MeshInteropContextCreateD3D11(AppGraphCtx* appctx);

MESH_API void MeshInteropContextUpdateD3D11(MeshContext* context, AppGraphCtx* appctx);

MeshContext* MeshInteropContextCreateD3D11(AppGraphCtx* appctxIn)
{
	auto appctx = cast_to_AppGraphCtxD3D11(appctxIn);

	MeshContextDescD3D11 desc = {};
	desc.device = appctx->m_device;
	desc.context = appctx->m_deviceContext;

	return MeshContextCreateD3D11(cast_from_MeshContextDescD3D11(&desc));
}

void MeshInteropContextUpdateD3D11(MeshContext* context, AppGraphCtx* appctxIn)
{
	auto appctx = cast_to_AppGraphCtxD3D11(appctxIn);

	MeshContextDescD3D11 desc = {};
	desc.device = appctx->m_device;
	desc.context = appctx->m_deviceContext;

	return MeshContextUpdateD3D11(context, cast_from_MeshContextDescD3D11(&desc));
}