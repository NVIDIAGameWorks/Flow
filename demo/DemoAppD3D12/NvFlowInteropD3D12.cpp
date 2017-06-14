/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <d3d12.h>

#include "../DemoApp/NvFlowInterop.h"

#include "NvFlowContextD3D12.h"
#include "appD3D12Ctx.h"

NV_FLOW_API NvFlowContext* NvFlowInteropCreateContextD3D12(AppGraphCtx* appctx);

NV_FLOW_API NvFlowDepthStencilView* NvFlowInteropCreateDepthStencilViewD3D12(AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API NvFlowRenderTargetView* NvFlowInteropCreateRenderTargetViewD3D12(AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API void NvFlowInteropUpdateContextD3D12(NvFlowContext* context, AppGraphCtx* appctx);

NV_FLOW_API void NvFlowInteropUpdateDepthStencilViewD3D12(NvFlowDepthStencilView* view, AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API void NvFlowInteropUpdateRenderTargetViewD3D12(NvFlowRenderTargetView* view, AppGraphCtx* appctx, NvFlowContext* flowctx);

NvFlowDescriptorReserveHandleD3D12 NvFlowInteropReserveDescriptors(void* userdata, NvFlowUint numDescriptors, NvFlowUint64 lastFenceCompleted, NvFlowUint64 nextFenceValue)
{
	auto appctx = cast_to_AppGraphCtxD3D12((AppGraphCtx*)userdata);
	auto srcHandle = appctx->m_dynamicHeapCbvSrvUav.reserveDescriptors(numDescriptors, lastFenceCompleted, nextFenceValue);
	NvFlowDescriptorReserveHandleD3D12 handle = {};
	handle.heap = srcHandle.heap;
	handle.descriptorSize = srcHandle.descriptorSize;
	handle.cpuHandle = srcHandle.cpuHandle;
	handle.gpuHandle = srcHandle.gpuHandle;
	return handle;
}

void NvFlowInteropUpdateContextDesc(NvFlowContextDescD3D12* desc, AppGraphCtx* appctxIn)
{
	auto appctx = cast_to_AppGraphCtxD3D12(appctxIn);

	desc->device = appctx->m_device;
	desc->commandQueue = appctx->m_commandQueue;
	desc->commandQueueFence = appctx->m_fence;
	desc->commandList = appctx->m_commandList;
	desc->lastFenceCompleted = appctx->m_lastFenceComplete;
	desc->nextFenceValue = appctx->m_thisFrameFenceID;

	desc->dynamicHeapCbvSrvUav.userdata = appctx;
	desc->dynamicHeapCbvSrvUav.reserveDescriptors = NvFlowInteropReserveDescriptors;
}

NvFlowContext* NvFlowInteropCreateContextD3D12(AppGraphCtx* appctx)
{
	NvFlowContextDescD3D12 desc = {};
	NvFlowInteropUpdateContextDesc(&desc, appctx);
	return NvFlowCreateContextD3D12(NV_FLOW_VERSION, &desc);
}

NvFlowDepthStencilView* NvFlowInteropCreateDepthStencilViewD3D12(AppGraphCtx* appctxIn, NvFlowContext* flowctx)
{
	auto appctx = cast_to_AppGraphCtxD3D12(appctxIn);

	NvFlowDepthStencilViewDescD3D12 desc = {};
	desc.dsvHandle = appctx->m_current_dsvHandle;
	desc.dsvDesc = appctx->m_current_dsvDesc;
	desc.dsvResource = appctx->m_depthStencil;
	desc.dsvCurrentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	desc.srvHandle = appctx->m_current_depth_srvHandle;
	desc.srvDesc = appctx->m_current_depth_srvDesc;
	desc.srvResource = appctx->m_depthStencil;
	desc.srvCurrentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	desc.viewport = appctx->m_viewport;
	return NvFlowCreateDepthStencilViewD3D12(flowctx, &desc);
}

NvFlowRenderTargetView* NvFlowInteropCreateRenderTargetViewD3D12(AppGraphCtx* appctxIn, NvFlowContext* flowctx)
{
	auto appctx = cast_to_AppGraphCtxD3D12(appctxIn);

	NvFlowRenderTargetViewDescD3D12 desc = {};
	desc.rtvHandle = appctx->m_current_rtvHandle;
	desc.rtvDesc = appctx->m_current_rtvDesc;
	desc.resource = appctx->m_current_renderTarget;
	desc.currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	desc.viewport = appctx->m_viewport;
	desc.scissor = appctx->m_scissorRect;
	return NvFlowCreateRenderTargetViewD3D12(flowctx, &desc);
}

void NvFlowInteropUpdateContextD3D12(NvFlowContext* context, AppGraphCtx* appctx)
{
	NvFlowContextDescD3D12 desc = {};
	NvFlowInteropUpdateContextDesc(&desc, appctx);
	NvFlowUpdateContextD3D12(context, &desc);
}

void NvFlowInteropUpdateDepthStencilViewD3D12(NvFlowDepthStencilView* view, AppGraphCtx* appctxIn, NvFlowContext* flowctx)
{
	auto appctx = cast_to_AppGraphCtxD3D12(appctxIn);

	NvFlowDepthStencilViewDescD3D12 desc = {};
	desc.dsvHandle = appctx->m_current_dsvHandle;
	desc.dsvDesc = appctx->m_current_dsvDesc;
	desc.dsvResource = appctx->m_depthStencil;
	desc.dsvCurrentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	desc.srvHandle = appctx->m_current_depth_srvHandle;
	desc.srvDesc = appctx->m_current_depth_srvDesc;
	desc.srvResource = appctx->m_depthStencil;
	desc.srvCurrentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

	desc.viewport = appctx->m_viewport;
	NvFlowUpdateDepthStencilViewD3D12(flowctx, view, &desc);
}

void NvFlowInteropUpdateRenderTargetViewD3D12(NvFlowRenderTargetView* view, AppGraphCtx* appctxIn, NvFlowContext* flowctx)
{
	auto appctx = cast_to_AppGraphCtxD3D12(appctxIn);

	NvFlowRenderTargetViewDescD3D12 desc = {};
	desc.rtvHandle = appctx->m_current_rtvHandle;
	desc.rtvDesc = appctx->m_current_rtvDesc;
	desc.resource = appctx->m_current_renderTarget;
	desc.currentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
	desc.viewport = appctx->m_viewport;
	desc.scissor = appctx->m_scissorRect;
	NvFlowUpdateRenderTargetViewD3D12(flowctx, view, &desc);
}