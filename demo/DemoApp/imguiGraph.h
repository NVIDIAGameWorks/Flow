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

#pragma once

#include <stdint.h>

#define IMGUI_GRAPH_API extern "C" __declspec(dllexport)

struct ImguiGraphDesc;

typedef bool (*imguiGraphInit_t)(const char* fontpath, const ImguiGraphDesc* desc);

typedef void (*imguiGraphUpdate_t)(const ImguiGraphDesc* desc);

bool imguiGraphInit(const char* fontpath, const ImguiGraphDesc* desc);

void imguiGraphUpdate(const ImguiGraphDesc* desc);

void imguiGraphDestroy();

void imguiGraphDraw();

// Below are the functions that must be implemented per graphics API

IMGUI_GRAPH_API void imguiGraphContextInit(const ImguiGraphDesc* desc);

IMGUI_GRAPH_API void imguiGraphContextUpdate(const ImguiGraphDesc* desc);

IMGUI_GRAPH_API void imguiGraphContextDestroy();

IMGUI_GRAPH_API void imguiGraphRecordBegin();

IMGUI_GRAPH_API void imguiGraphRecordEnd();

IMGUI_GRAPH_API void imguiGraphVertex2f(float x, float y);

IMGUI_GRAPH_API void imguiGraphVertex2fv(const float* v);

IMGUI_GRAPH_API void imguiGraphTexCoord2f(float u, float v);

IMGUI_GRAPH_API void imguiGraphColor4ub(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);

IMGUI_GRAPH_API void imguiGraphColor4ubv(const uint8_t* v);

IMGUI_GRAPH_API void imguiGraphFontTextureEnable();

IMGUI_GRAPH_API void imguiGraphFontTextureDisable();

IMGUI_GRAPH_API void imguiGraphEnableScissor(int x, int y, int width, int height);

IMGUI_GRAPH_API void imguiGraphDisableScissor();

IMGUI_GRAPH_API void imguiGraphFontTextureInit(unsigned char* data);

IMGUI_GRAPH_API void imguiGraphFontTextureRelease();