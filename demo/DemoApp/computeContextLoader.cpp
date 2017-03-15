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

#include "computeContext.h"

namespace
{
	ModuleLoader<32u, SDL_LoadObject, SDL_UnloadObject, SDL_LoadFunction> g_loader;
}

void loadComputeContext(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	g_loader.loadModule(moduleName);
}

void unloadComputeContext()
{
	g_loader.unloadModule();
}

ComputeContext* ComputeContextCreate(ComputeContextDesc* desc)
{
	return g_loader.function<0>(ComputeContextCreate, "ComputeContextCreate", desc);
}

void ComputeContextUpdate(ComputeContext* context, ComputeContextDesc* desc)
{
	return g_loader.function<1>(ComputeContextUpdate, "ComputeContextUpdate", context, desc);
}

void ComputeContextRelease(ComputeContext* context)
{
	return g_loader.function<2>(ComputeContextRelease, "ComputeContextRelease", context);
}

ComputeShader* ComputeShaderCreate(ComputeContext* context, const ComputeShaderDesc* desc)
{
	return g_loader.function<3>(ComputeShaderCreate, "ComputeShaderCreate", context, desc);
}

void ComputeShaderRelease(ComputeShader* shader)
{
	return g_loader.function<4>(ComputeShaderRelease, "ComputeShaderRelease", shader);
}

ComputeConstantBuffer* ComputeConstantBufferCreate(ComputeContext* context, const ComputeConstantBufferDesc* desc)
{
	return g_loader.function<5>(ComputeConstantBufferCreate, "ComputeConstantBufferCreate", context, desc);
}

void ComputeConstantBufferRelease(ComputeConstantBuffer* constantBuffer)
{
	return g_loader.function<6>(ComputeConstantBufferRelease, "ComputeConstantBufferRelease", constantBuffer);
}

void* ComputeConstantBufferMap(ComputeContext* context, ComputeConstantBuffer* constantBuffer)
{
	return g_loader.function<7>(ComputeConstantBufferMap, "ComputeConstantBufferMap", context, constantBuffer);
}

void ComputeConstantBufferUnmap(ComputeContext* context, ComputeConstantBuffer* constantBuffer)
{
	return g_loader.function<8>(ComputeConstantBufferUnmap, "ComputeConstantBufferUnmap", context, constantBuffer);
}

ComputeResource* ComputeResourceCreate(ComputeContext* context, const ComputeResourceDesc* desc)
{
	return g_loader.function<9>(ComputeResourceCreate, "ComputeResourceCreate", context, desc);
}

void ComputeResourceUpdate(ComputeContext* context, ComputeResource* resource, const ComputeResourceDesc* desc)
{
	return g_loader.function<10>(ComputeResourceUpdate, "ComputeResourceUpdate", context, resource, desc);
}

void ComputeResourceRelease(ComputeResource* resource)
{
	return g_loader.function<11>(ComputeResourceRelease, "ComputeResourceRelease", resource);
}

ComputeResourceRW* ComputeResourceRWCreate(ComputeContext* context, const ComputeResourceRWDesc* desc)
{
	return g_loader.function<12>(ComputeResourceRWCreate, "ComputeResourceRWCreate", context, desc);
}

void ComputeResourceRWUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, const ComputeResourceRWDesc* desc)
{
	return g_loader.function<13>(ComputeResourceRWUpdate, "ComputeResourceRWUpdate", context, resourceRW, desc);
}

void ComputeResourceRWRelease(ComputeResourceRW* resourceRW)
{
	return g_loader.function<14>(ComputeResourceRWRelease, "ComputeResourceRWRelease", resourceRW);
}

void ComputeContextDispatch(ComputeContext* context, const ComputeDispatchParams* params)
{
	return g_loader.function<15>(ComputeContextDispatch, "ComputeContextDispatch", context, params);
}

ComputeContext* ComputeContextNvFlowContextCreate(NvFlowContext* flowContext)
{
	return g_loader.function<16>(ComputeContextNvFlowContextCreate, "ComputeContextNvFlowContextCreate", flowContext);
}

void ComputeContextNvFlowContextUpdate(ComputeContext* computeContext, NvFlowContext* flowContext)
{
	return g_loader.function<17>(ComputeContextNvFlowContextUpdate, "ComputeContextNvFlowContextUpdate", computeContext, flowContext);
}

ComputeResource* ComputeResourceNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	return g_loader.function<18>(ComputeResourceNvFlowCreate, "ComputeResourceNvFlowCreate", context, flowContext, flowResource);
}

void ComputeResourceNvFlowUpdate(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	return g_loader.function<19>(ComputeResourceNvFlowUpdate, "ComputeResourceNvFlowUpdate", context, resource, flowContext, flowResource);
}

ComputeResourceRW* ComputeResourceRWNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	return g_loader.function<20>(ComputeResourceRWNvFlowCreate, "ComputeResourceRWNvFlowCreate", context, flowContext, flowResourceRW);
}

void ComputeResourceRWNvFlowUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	return g_loader.function<21>(ComputeResourceRWNvFlowUpdate, "ComputeResourceRWNvFlowUpdate", context, resourceRW, flowContext, flowResourceRW);
}

ComputeResource* ComputeResourceRWGetResource(ComputeResourceRW* resourceRW)
{
	return g_loader.function<22>(ComputeResourceRWGetResource, "ComputeResourceRWGetResource", resourceRW);
}