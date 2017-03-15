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

#include "imguiGraph.h"

#include "appGraphCtx.h"

IMGUI_GRAPH_API bool imguiInteropGraphInit(imguiGraphInit_t func, const char* fontpath, AppGraphCtx* appctx);

IMGUI_GRAPH_API void imguiInteropGraphUpdate(imguiGraphUpdate_t func, AppGraphCtx* appctx);