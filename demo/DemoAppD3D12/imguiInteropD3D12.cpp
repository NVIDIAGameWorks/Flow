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

#include "../DemoApp/imguiInterop.h"

#include "appD3D12Ctx.h"
#include "imguiGraphD3D12.h"

ImguiDescriptorReserveHandleD3D12 imguiInteropReserveDescriptors(void* userdata, UINT numDescriptors, UINT64 lastFenceCompleted, UINT64 nextFenceValue)
{
	auto appctx = cast_to_AppGraphCtxD3D12((AppGraphCtx*)userdata);
	auto srcHandle = appctx->m_dynamicHeapCbvSrvUav.reserveDescriptors(numDescriptors, lastFenceCompleted, nextFenceValue);
	ImguiDescriptorReserveHandleD3D12 handle = {};
	handle.heap = srcHandle.heap;
	handle.descriptorSize = srcHandle.descriptorSize;
	handle.cpuHandle = srcHandle.cpuHandle;
	handle.gpuHandle = srcHandle.gpuHandle;
	return handle;
}

inline void imguiInteropUpdateDesc(ImguiGraphDescD3D12& desc, AppGraphCtx* appctxIn)
{
	auto appctx = cast_to_AppGraphCtxD3D12(appctxIn);

	desc.device = appctx->m_device;
	desc.commandList = appctx->m_commandList;
	desc.lastFenceCompleted = appctx->m_lastFenceComplete;
	desc.nextFenceValue = appctx->m_thisFrameFenceID;
	desc.winW = appctx->m_winW;
	desc.winH = appctx->m_winH;
	desc.dynamicHeapCbvSrvUav.userdata = appctx;
	desc.dynamicHeapCbvSrvUav.reserveDescriptors = imguiInteropReserveDescriptors;
}

IMGUI_GRAPH_API bool imguiInteropGraphInitD3D12(imguiGraphInit_t func, const char* fontpath, AppGraphCtx* appctx);

IMGUI_GRAPH_API void imguiInteropGraphUpdateD3D12(imguiGraphUpdate_t func, AppGraphCtx* appctx);

bool imguiInteropGraphInitD3D12(imguiGraphInit_t func, const char* fontpath, AppGraphCtx* appctx)
{
	ImguiGraphDescD3D12 desc;
	imguiInteropUpdateDesc(desc, appctx);

	return func(fontpath, cast_from_imguiGraphDescD3D12(&desc));
}

void imguiInteropGraphUpdateD3D12(imguiGraphUpdate_t func, AppGraphCtx* appctx)
{
	ImguiGraphDescD3D12 desc;
	imguiInteropUpdateDesc(desc, appctx);

	return func(cast_from_imguiGraphDescD3D12(&desc));
}