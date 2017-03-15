/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <d3d11.h>

#include "../DemoApp/NvFlowInterop.h"

#include "NvFlowContextD3D11.h"
#include "appD3D11Ctx.h"

NvFlowContext* NvFlowInteropCreateContext(AppGraphCtx* appctx)
{
	NvFlowContextDescD3D11 desc = {};
	desc.device = appctx->m_device;
	desc.deviceContext = appctx->m_deviceContext;
	return NvFlowCreateContextD3D11(NV_FLOW_VERSION, &desc);
}

NvFlowDepthStencilView* NvFlowInteropCreateDepthStencilView(AppGraphCtx* appctx, NvFlowContext* flowctx)
{
	NvFlowDepthStencilViewDescD3D11 desc = {};
	desc.dsv = appctx->m_dsv;
	desc.srv = appctx->m_depthSRV;
	desc.viewport = appctx->m_viewport;
	return NvFlowCreateDepthStencilViewD3D11(flowctx, &desc);
}

NvFlowRenderTargetView* NvFlowInteropCreateRenderTargetView(AppGraphCtx* appctx, NvFlowContext* flowctx)
{
	NvFlowRenderTargetViewDescD3D11 desc = {};
	desc.rtv = appctx->m_rtv;
	desc.viewport = appctx->m_viewport;
	return NvFlowCreateRenderTargetViewD3D11(flowctx, &desc);
}

void NvFlowInteropUpdateContext(NvFlowContext* context, AppGraphCtx* appctx)
{
	NvFlowContextDescD3D11 desc = {};
	desc.device = appctx->m_device;
	desc.deviceContext = appctx->m_deviceContext;
	NvFlowUpdateContextD3D11(context, &desc);
}

void NvFlowInteropUpdateDepthStencilView(NvFlowDepthStencilView* view, AppGraphCtx* appctx, NvFlowContext* flowctx)
{
	NvFlowDepthStencilViewDescD3D11 desc = {};
	desc.dsv = appctx->m_dsv;
	desc.srv = appctx->m_depthSRV;
	desc.viewport = appctx->m_viewport;
	NvFlowUpdateDepthStencilViewD3D11(flowctx, view, &desc);
}

void NvFlowInteropUpdateRenderTargetView(NvFlowRenderTargetView* view, AppGraphCtx* appctx, NvFlowContext* flowctx)
{
	NvFlowRenderTargetViewDescD3D11 desc = {};
	desc.rtv = appctx->m_rtv;
	desc.viewport = appctx->m_viewport;
	NvFlowUpdateRenderTargetViewD3D11(flowctx, view, &desc);
}