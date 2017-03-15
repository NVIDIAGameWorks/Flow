/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef IMGUI_GRAPH_D3D11_H
#define IMGUI_GRAPH_D3D11_H

#include <stdint.h>

#include "../DemoApp/imguiGraph.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

struct ImguiGraphDesc
{
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	int winW;
	int winH;

	uint32_t maxVertices = 64 * 4096u;

	ImguiGraphDesc() {}
};

#endif