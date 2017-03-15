/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <SDL.h>

#include "loader.h"

#include "NvFlowInterop.h"

namespace
{
	ModuleLoader<16u, SDL_LoadObject, SDL_UnloadObject, SDL_LoadFunction> g_loader;
}

void loadNvFlowInterop(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	g_loader.loadModule(moduleName);
}

void unloadNvFlowInterop()
{
	g_loader.unloadModule();
}

// Functions
NvFlowContext* NvFlowInteropCreateContext(AppGraphCtx* appctx)
{
	return g_loader.function<0>(NvFlowInteropCreateContext, "NvFlowInteropCreateContext", appctx);
}

NvFlowDepthStencilView* NvFlowInteropCreateDepthStencilView(AppGraphCtx* appctx, NvFlowContext* flowctx)
{
	return g_loader.function<1>(NvFlowInteropCreateDepthStencilView, "NvFlowInteropCreateDepthStencilView", appctx, flowctx);
}

NvFlowRenderTargetView* NvFlowInteropCreateRenderTargetView(AppGraphCtx* appctx, NvFlowContext* flowctx)
{
	return g_loader.function<2>(NvFlowInteropCreateRenderTargetView, "NvFlowInteropCreateRenderTargetView", appctx, flowctx);
}

void NvFlowInteropUpdateContext(NvFlowContext* context, AppGraphCtx* appctx)
{
	return g_loader.function<3>(NvFlowInteropUpdateContext, "NvFlowInteropUpdateContext", context, appctx);
}

void NvFlowInteropUpdateDepthStencilView(NvFlowDepthStencilView* view, AppGraphCtx* appctx, NvFlowContext* flowctx)
{
	return g_loader.function<4>(NvFlowInteropUpdateDepthStencilView, "NvFlowInteropUpdateDepthStencilView", view, appctx, flowctx);
}

void NvFlowInteropUpdateRenderTargetView(NvFlowRenderTargetView* view, AppGraphCtx* appctx, NvFlowContext* flowctx)
{
	return g_loader.function<5>(NvFlowInteropUpdateRenderTargetView, "NvFlowInteropUpdateRenderTargetView", view, appctx, flowctx);
}