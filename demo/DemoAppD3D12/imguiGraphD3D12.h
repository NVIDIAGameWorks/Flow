/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef IMGUI_GRAPH_D3D12_H
#define IMGUI_GRAPH_D3D12_H

#include <stdint.h>

#include "../DemoApp/imguiGraph.h"

struct ImguiDescriptorReserveHandleD3D12
{
	ID3D12DescriptorHeap* heap;
	UINT descriptorSize;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

struct ImguiDynamicDescriptorHeapD3D12
{
	void* userdata;
	ImguiDescriptorReserveHandleD3D12(*reserveDescriptors)(void* userdata, UINT numDescriptors, UINT64 lastFenceCompleted, UINT64 nextFenceValue);
};

struct ImguiGraphDesc
{
	ID3D12Device* device = nullptr;
	ID3D12GraphicsCommandList* commandList = nullptr;
	UINT64 lastFenceCompleted; 
	UINT64 nextFenceValue;
	int winW;
	int winH;

	uint32_t maxVertices = 64 * 4096u;

	ImguiDynamicDescriptorHeapD3D12 dynamicHeapCbvSrvUav = {};

	ImguiGraphDesc() {}
};

#endif