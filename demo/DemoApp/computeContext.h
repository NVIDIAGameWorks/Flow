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