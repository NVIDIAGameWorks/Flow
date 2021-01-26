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

#define COMPUTE_API extern "C" __declspec(dllexport)

struct ComputeContextDesc;
struct ComputeContext;

struct ComputeShaderDesc
{
	const void* cs;
	unsigned long long cs_length;
};
struct ComputeShader;

struct ComputeConstantBufferDesc
{
	unsigned int sizeInBytes;
};
struct ComputeConstantBuffer;

struct ComputeResourceDesc;
struct ComputeResource;

struct ComputeResourceRWDesc;
struct ComputeResourceRW;

static const unsigned int ComputeDispatchMaxResources = 16u;
static const unsigned int ComputeDispatchMaxResourcesRW = 8u;

struct ComputeDispatchParams
{
	ComputeShader* shader;
	unsigned int gridDim[3u];
	ComputeConstantBuffer* constantBuffer;
	ComputeResource* resources[ComputeDispatchMaxResources];
	ComputeResourceRW* resourcesRW[ComputeDispatchMaxResourcesRW];
};

COMPUTE_API ComputeContext* ComputeContextCreate(ComputeContextDesc* desc);

COMPUTE_API void ComputeContextUpdate(ComputeContext* context, ComputeContextDesc* desc);

COMPUTE_API void ComputeContextRelease(ComputeContext* context);


COMPUTE_API ComputeShader* ComputeShaderCreate(ComputeContext* context, const ComputeShaderDesc* desc);

COMPUTE_API void ComputeShaderRelease(ComputeShader* shader);


COMPUTE_API ComputeConstantBuffer* ComputeConstantBufferCreate(ComputeContext* context, const ComputeConstantBufferDesc* desc);

COMPUTE_API void ComputeConstantBufferRelease(ComputeConstantBuffer* constantBuffer);

COMPUTE_API void* ComputeConstantBufferMap(ComputeContext* context, ComputeConstantBuffer* constantBuffer);

COMPUTE_API void ComputeConstantBufferUnmap(ComputeContext* context, ComputeConstantBuffer* constantBuffer);


COMPUTE_API ComputeResource* ComputeResourceCreate(ComputeContext* context, const ComputeResourceDesc* desc);

COMPUTE_API void ComputeResourceUpdate(ComputeContext* context, ComputeResource* resource, const ComputeResourceDesc* desc);

COMPUTE_API void ComputeResourceRelease(ComputeResource* resource);


COMPUTE_API ComputeResourceRW* ComputeResourceRWCreate(ComputeContext* context, const ComputeResourceRWDesc* desc);

COMPUTE_API void ComputeResourceRWUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, const ComputeResourceRWDesc* desc);

COMPUTE_API void ComputeResourceRWRelease(ComputeResourceRW* resourceRW);

COMPUTE_API ComputeResource* ComputeResourceRWGetResource(ComputeResourceRW* resourceRW);


COMPUTE_API void ComputeContextDispatch(ComputeContext* context, const ComputeDispatchParams* params);

// interoperation with NvFlow
struct NvFlowContext;
struct NvFlowResource;
struct NvFlowResourceRW;

COMPUTE_API ComputeContext* ComputeContextNvFlowContextCreate(NvFlowContext* flowContext);

COMPUTE_API void ComputeContextNvFlowContextUpdate(ComputeContext* computeContext, NvFlowContext* flowContext);

COMPUTE_API ComputeResource* ComputeResourceNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource);

COMPUTE_API void ComputeResourceNvFlowUpdate(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource);

COMPUTE_API ComputeResourceRW* ComputeResourceRWNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW);

COMPUTE_API void ComputeResourceRWNvFlowUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW);