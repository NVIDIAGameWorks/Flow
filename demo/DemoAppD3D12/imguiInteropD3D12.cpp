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
	auto appctx = static_cast<AppGraphCtx*>(userdata);
	auto srcHandle = appctx->m_dynamicHeapCbvSrvUav.reserveDescriptors(numDescriptors, lastFenceCompleted, nextFenceValue);
	ImguiDescriptorReserveHandleD3D12 handle = {};
	handle.heap = srcHandle.heap;
	handle.descriptorSize = srcHandle.descriptorSize;
	handle.cpuHandle = srcHandle.cpuHandle;
	handle.gpuHandle = srcHandle.gpuHandle;
	return handle;
}

void imguiInteropUpdateDesc(ImguiGraphDesc& desc, AppGraphCtx* appctx)
{
	desc.device = appctx->m_device;
	desc.commandList = appctx->m_commandList;
	desc.lastFenceCompleted = appctx->m_lastFenceComplete;
	desc.nextFenceValue = appctx->m_thisFrameFenceID;
	desc.winW = appctx->m_winW;
	desc.winH = appctx->m_winH;
	desc.dynamicHeapCbvSrvUav.userdata = appctx;
	desc.dynamicHeapCbvSrvUav.reserveDescriptors = imguiInteropReserveDescriptors;
}

bool imguiInteropGraphInit(imguiGraphInit_t func, const char* fontpath, AppGraphCtx* appctx)
{
	ImguiGraphDesc desc;
	imguiInteropUpdateDesc(desc, appctx);

	return func(fontpath, &desc);
}

void imguiInteropGraphUpdate(imguiGraphUpdate_t func, AppGraphCtx* appctx)
{
	ImguiGraphDesc desc;
	imguiInteropUpdateDesc(desc, appctx);

	return func(&desc);
}