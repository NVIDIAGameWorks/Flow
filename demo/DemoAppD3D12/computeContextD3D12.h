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

struct ComputeContextDescD3D12
{
	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;
	ID3D12Fence* commandQueueFence;
	ID3D12GraphicsCommandList* commandList;
	UINT64 lastFenceCompleted;
	UINT64 nextFenceValue;

	ComputeDynamicDescriptorHeapD3D12 dynamicHeapCbvSrvUav;
};

struct ComputeResourceDescD3D12
{
	D3D12_CPU_DESCRIPTOR_HANDLE srv;
	ID3D12Resource* resource;
	D3D12_RESOURCE_STATES* currentState;
};

struct ComputeResourceRWDescD3D12
{
	ComputeResourceDescD3D12 resource;
	D3D12_CPU_DESCRIPTOR_HANDLE uav;
};

inline const ComputeContextDescD3D12* cast_to_ComputeContextDescD3D12(const ComputeContextDesc* desc)
{
	return (const ComputeContextDescD3D12*)(desc);
}

inline ComputeContextDesc* cast_from_ComputeContextDescD3D12(ComputeContextDescD3D12* desc)
{
	return (ComputeContextDesc*)(desc);
}

inline const ComputeResourceDescD3D12* cast_to_ComputeResourceDescD3D12(const ComputeResourceDesc* desc)
{
	return (const ComputeResourceDescD3D12*)(desc);
}

inline const ComputeResourceDesc* cast_from_ComputeResourceDescD3D12(const ComputeResourceDescD3D12* desc)
{
	return (const ComputeResourceDesc*)(desc);
}

inline const ComputeResourceRWDescD3D12* cast_to_ComputeResourceRWDescD3D12(const ComputeResourceRWDesc* desc)
{
	return (const ComputeResourceRWDescD3D12*)(desc);
}

inline const ComputeResourceRWDesc* cast_from_ComputeResourceRWDescD3D12(const ComputeResourceRWDescD3D12* desc)
{
	return (const ComputeResourceRWDesc*)(desc);
}

COMPUTE_API ComputeContext* ComputeContextCreateD3D12(ComputeContextDesc* desc);

COMPUTE_API void ComputeContextUpdateD3D12(ComputeContext* context, ComputeContextDesc* desc);

COMPUTE_API void ComputeContextReleaseD3D12(ComputeContext* context);

COMPUTE_API ComputeShader* ComputeShaderCreateD3D12(ComputeContext* context, const ComputeShaderDesc* desc);

COMPUTE_API void ComputeShaderReleaseD3D12(ComputeShader* shader);

COMPUTE_API ComputeConstantBuffer* ComputeConstantBufferCreateD3D12(ComputeContext* context, const ComputeConstantBufferDesc* desc);

COMPUTE_API void ComputeConstantBufferReleaseD3D12(ComputeConstantBuffer* constantBuffer);

COMPUTE_API void* ComputeConstantBufferMapD3D12(ComputeContext* context, ComputeConstantBuffer* constantBuffer);

COMPUTE_API void ComputeConstantBufferUnmapD3D12(ComputeContext* context, ComputeConstantBuffer* constantBuffer);

COMPUTE_API ComputeResource* ComputeResourceCreateD3D12(ComputeContext* context, const ComputeResourceDesc* desc);

COMPUTE_API void ComputeResourceUpdateD3D12(ComputeContext* context, ComputeResource* resource, const ComputeResourceDesc* desc);

COMPUTE_API void ComputeResourceReleaseD3D12(ComputeResource* resource);

COMPUTE_API ComputeResourceRW* ComputeResourceRWCreateD3D12(ComputeContext* context, const ComputeResourceRWDesc* desc);

COMPUTE_API void ComputeResourceRWUpdateD3D12(ComputeContext* context, ComputeResourceRW* resourceRW, const ComputeResourceRWDesc* desc);

COMPUTE_API void ComputeResourceRWReleaseD3D12(ComputeResourceRW* resourceRW);

COMPUTE_API ComputeResource* ComputeResourceRWGetResourceD3D12(ComputeResourceRW* resourceRW);

COMPUTE_API void ComputeContextDispatchD3D12(ComputeContext* context, const ComputeDispatchParams* params);

COMPUTE_API ComputeContext* ComputeContextNvFlowContextCreateD3D12(NvFlowContext* flowContext);

COMPUTE_API void ComputeContextNvFlowContextUpdateD3D12(ComputeContext* computeContext, NvFlowContext* flowContext);

COMPUTE_API ComputeResource* ComputeResourceNvFlowCreateD3D12(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource);

COMPUTE_API void ComputeResourceNvFlowUpdateD3D12(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource);

COMPUTE_API ComputeResourceRW* ComputeResourceRWNvFlowCreateD3D12(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW);

COMPUTE_API void ComputeResourceRWNvFlowUpdateD3D12(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW);