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

#include "appGraphCtx.h"

namespace
{
	ModuleLoader<16u, SDL_LoadObject, SDL_UnloadObject, SDL_LoadFunction> g_loader;
}

void loadAppGraphCtx(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	g_loader.loadModule(moduleName);
}

void unloadAppGraphCtx()
{
	g_loader.unloadModule();
}

AppGraphCtx* AppGraphCtxCreate(int deviceID)
{
	return g_loader.function<0>(AppGraphCtxCreate, "AppGraphCtxCreate", deviceID);
}

bool AppGraphCtxUpdateSize(AppGraphCtx* context, SDL_Window* window, bool fullscreen)
{
	return g_loader.function<1>(AppGraphCtxUpdateSize, "AppGraphCtxUpdateSize", context, window, fullscreen);
}

void AppGraphCtxReleaseRenderTarget(AppGraphCtx* context)
{
	return g_loader.function<2>(AppGraphCtxReleaseRenderTarget, "AppGraphCtxReleaseRenderTarget", context);
}

void AppGraphCtxRelease(AppGraphCtx* context)
{
	return g_loader.function<3>(AppGraphCtxRelease, "AppGraphCtxRelease", context);
}

void AppGraphCtxFrameStart(AppGraphCtx* context, float clearColor[4])
{
	return g_loader.function<4>(AppGraphCtxFrameStart, "AppGraphCtxFrameStart", context, clearColor);
}

void AppGraphCtxFramePresent(AppGraphCtx* context, bool fullsync)
{
	return g_loader.function<5>(AppGraphCtxFramePresent, "AppGraphCtxFramePresent", context, fullsync);
}

void AppGraphCtxWaitForFrames(AppGraphCtx* context, unsigned int maxFramesInFlight)
{
	return g_loader.function<6>(AppGraphCtxWaitForFrames, "AppGraphCtxWaitForFrames", context, maxFramesInFlight);
}

void AppGraphCtxProfileEnable(AppGraphCtx* context, bool enabled)
{
	return g_loader.function<7>(AppGraphCtxProfileEnable, "AppGraphCtxProfileEnable", context, enabled);
}

void AppGraphCtxProfileBegin(AppGraphCtx* context, const char* label)
{
	return g_loader.function<8>(AppGraphCtxProfileBegin, "AppGraphCtxProfileBegin", context, label);
}

void AppGraphCtxProfileEnd(AppGraphCtx* context, const char* label)
{
	return g_loader.function<9>(AppGraphCtxProfileEnd, "AppGraphCtxProfileEnd", context, label);
}

bool AppGraphCtxProfileGet(AppGraphCtx* context, const char** plabel, float* cpuTime, float* gpuTime, int index)
{
	return g_loader.function<10>(AppGraphCtxProfileGet, "AppGraphCtxProfileGet", context, plabel, cpuTime, gpuTime, index);
}

size_t AppGraphCtxDedicatedVideoMemory(AppGraphCtx* context)
{
	return g_loader.function<11>(AppGraphCtxDedicatedVideoMemory, "AppGraphCtxDedicatedVideoMemory", context);
}