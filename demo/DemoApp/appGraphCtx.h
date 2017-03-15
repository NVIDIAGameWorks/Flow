/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#pragma once

#define APP_GRAPH_CTX_API extern "C" __declspec(dllexport)

struct SDL_Window;

struct AppGraphCtx;

APP_GRAPH_CTX_API AppGraphCtx* AppGraphCtxCreate(int deviceID);

APP_GRAPH_CTX_API bool AppGraphCtxUpdateSize(AppGraphCtx* context, SDL_Window* window, bool fullscreen);

APP_GRAPH_CTX_API void AppGraphCtxReleaseRenderTarget(AppGraphCtx* context);

APP_GRAPH_CTX_API void AppGraphCtxRelease(AppGraphCtx* context);

APP_GRAPH_CTX_API void AppGraphCtxFrameStart(AppGraphCtx* context, float clearColor[4]);

APP_GRAPH_CTX_API void AppGraphCtxFramePresent(AppGraphCtx* context, bool fullsync);

APP_GRAPH_CTX_API void AppGraphCtxWaitForFrames(AppGraphCtx* context, unsigned int maxFramesInFlight);

APP_GRAPH_CTX_API void AppGraphCtxProfileEnable(AppGraphCtx* context, bool enabled);

APP_GRAPH_CTX_API void AppGraphCtxProfileBegin(AppGraphCtx* context, const char* label);

APP_GRAPH_CTX_API void AppGraphCtxProfileEnd(AppGraphCtx* context, const char* label);

APP_GRAPH_CTX_API bool AppGraphCtxProfileGet(AppGraphCtx* context, const char** plabel, float* cpuTime, float* gpuTime, int index);

APP_GRAPH_CTX_API size_t AppGraphCtxDedicatedVideoMemory(AppGraphCtx* context);