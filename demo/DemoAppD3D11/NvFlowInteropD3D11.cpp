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

#include <d3d11.h>

#include "../DemoApp/NvFlowInterop.h"

#include "NvFlowContextD3D11.h"
#include "appD3D11Ctx.h"

NV_FLOW_API NvFlowContext* NvFlowInteropCreateContextD3D11(AppGraphCtx* appctx);

NV_FLOW_API NvFlowDepthStencilView* NvFlowInteropCreateDepthStencilViewD3D11(AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API NvFlowRenderTargetView* NvFlowInteropCreateRenderTargetViewD3D11(AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API void NvFlowInteropUpdateContextD3D11(NvFlowContext* context, AppGraphCtx* appctx);

NV_FLOW_API void NvFlowInteropUpdateDepthStencilViewD3D11(NvFlowDepthStencilView* view, AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API void NvFlowInteropUpdateRenderTargetViewD3D11(NvFlowRenderTargetView* view, AppGraphCtx* appctx, NvFlowContext* flowctx);

NvFlowContext* NvFlowInteropCreateContextD3D11(AppGraphCtx* appctxIn)
{
	auto appctx = cast_to_AppGraphCtxD3D11(appctxIn);

	NvFlowContextDescD3D11 desc = {};
	desc.device = appctx->m_device;
	desc.deviceContext = appctx->m_deviceContext;
	return NvFlowCreateContextD3D11(NV_FLOW_VERSION, &desc);
}

NvFlowDepthStencilView* NvFlowInteropCreateDepthStencilViewD3D11(AppGraphCtx* appctxIn, NvFlowContext* flowctx)
{
	auto appctx = cast_to_AppGraphCtxD3D11(appctxIn);

	NvFlowDepthStencilViewDescD3D11 desc = {};
	desc.dsv = appctx->m_dsv;
	desc.srv = appctx->m_depthSRV;
	desc.viewport = appctx->m_viewport;
	return NvFlowCreateDepthStencilViewD3D11(flowctx, &desc);
}

NvFlowRenderTargetView* NvFlowInteropCreateRenderTargetViewD3D11(AppGraphCtx* appctxIn, NvFlowContext* flowctx)
{
	auto appctx = cast_to_AppGraphCtxD3D11(appctxIn);

	NvFlowRenderTargetViewDescD3D11 desc = {};
	desc.rtv = appctx->m_rtv;
	desc.viewport = appctx->m_viewport;
	return NvFlowCreateRenderTargetViewD3D11(flowctx, &desc);
}

void NvFlowInteropUpdateContextD3D11(NvFlowContext* context, AppGraphCtx* appctxIn)
{
	auto appctx = cast_to_AppGraphCtxD3D11(appctxIn);

	NvFlowContextDescD3D11 desc = {};
	desc.device = appctx->m_device;
	desc.deviceContext = appctx->m_deviceContext;
	NvFlowUpdateContextD3D11(context, &desc);
}

void NvFlowInteropUpdateDepthStencilViewD3D11(NvFlowDepthStencilView* view, AppGraphCtx* appctxIn, NvFlowContext* flowctx)
{
	auto appctx = cast_to_AppGraphCtxD3D11(appctxIn);

	NvFlowDepthStencilViewDescD3D11 desc = {};
	desc.dsv = appctx->m_dsv;
	desc.srv = appctx->m_depthSRV;
	desc.viewport = appctx->m_viewport;
	NvFlowUpdateDepthStencilViewD3D11(flowctx, view, &desc);
}

void NvFlowInteropUpdateRenderTargetViewD3D11(NvFlowRenderTargetView* view, AppGraphCtx* appctxIn, NvFlowContext* flowctx)
{
	auto appctx = cast_to_AppGraphCtxD3D11(appctxIn);

	NvFlowRenderTargetViewDescD3D11 desc = {};
	desc.rtv = appctx->m_rtv;
	desc.viewport = appctx->m_viewport;
	NvFlowUpdateRenderTargetViewD3D11(flowctx, view, &desc);
}