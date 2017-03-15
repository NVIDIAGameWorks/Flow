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
#include <d3d11.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d11.lib")

#include "../DemoApp/imguiInterop.h"

#include "appD3D11Ctx.h"
#include "imguiGraphD3D11.h"

bool imguiInteropGraphInit(imguiGraphInit_t func, const char* fontpath, AppGraphCtx* appctx)
{
	ImguiGraphDesc desc;
	desc.device = appctx->m_device;
	desc.deviceContext = appctx->m_deviceContext;
	desc.winW = appctx->m_winW;
	desc.winH = appctx->m_winH;

	return func(fontpath, &desc);
}

void imguiInteropGraphUpdate(imguiGraphUpdate_t func, AppGraphCtx* appctx)
{
	ImguiGraphDesc desc;
	desc.device = appctx->m_device;
	desc.deviceContext = appctx->m_deviceContext;
	desc.winW = appctx->m_winW;
	desc.winH = appctx->m_winH;

	return func(&desc);
}