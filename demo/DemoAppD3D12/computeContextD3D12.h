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

#include "../DemoApp/computeContext.h"

struct ComputeDescriptorReserveHandleD3D12
{
	ID3D12DescriptorHeap* heap;
	UINT descriptorSize;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

struct ComputeDynamicDescriptorHeapD3D12
{
	void* userdata;
	ComputeDescriptorReserveHandleD3D12(*reserveDescriptors)(void* userdata, UINT numDescriptors, UINT64 lastFenceCompleted, UINT64 nextFenceValue);
};

struct ComputeContextDesc
{
	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;
	ID3D12Fence* commandQueueFence;
	ID3D12GraphicsCommandList* commandList;
	UINT64 lastFenceCompleted;
	UINT64 nextFenceValue;

	ComputeDynamicDescriptorHeapD3D12 dynamicHeapCbvSrvUav;
};

struct ComputeResourceDesc
{
	D3D12_CPU_DESCRIPTOR_HANDLE srv;
	ID3D12Resource* resource;
	D3D12_RESOURCE_STATES* currentState;
};

struct ComputeResourceRWDesc
{
	ComputeResourceDesc resource;
	D3D12_CPU_DESCRIPTOR_HANDLE uav;
};