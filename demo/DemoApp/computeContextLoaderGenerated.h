/*
* Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

typedef ComputeContext*  (*ComputeContextCreate_ptr_t)(ComputeContextDesc*  desc);
typedef void  (*ComputeContextUpdate_ptr_t)(ComputeContext*  context, ComputeContextDesc*  desc);
typedef void  (*ComputeContextRelease_ptr_t)(ComputeContext*  context);
typedef ComputeShader*  (*ComputeShaderCreate_ptr_t)(ComputeContext*  context, const ComputeShaderDesc*  desc);
typedef void  (*ComputeShaderRelease_ptr_t)(ComputeShader*  shader);
typedef ComputeConstantBuffer*  (*ComputeConstantBufferCreate_ptr_t)(ComputeContext*  context, const ComputeConstantBufferDesc*  desc);
typedef void  (*ComputeConstantBufferRelease_ptr_t)(ComputeConstantBuffer*  constantBuffer);
typedef void*  (*ComputeConstantBufferMap_ptr_t)(ComputeContext*  context, ComputeConstantBuffer*  constantBuffer);
typedef void  (*ComputeConstantBufferUnmap_ptr_t)(ComputeContext*  context, ComputeConstantBuffer*  constantBuffer);
typedef ComputeResource*  (*ComputeResourceCreate_ptr_t)(ComputeContext*  context, const ComputeResourceDesc*  desc);
typedef void  (*ComputeResourceUpdate_ptr_t)(ComputeContext*  context, ComputeResource*  resource, const ComputeResourceDesc*  desc);
typedef void  (*ComputeResourceRelease_ptr_t)(ComputeResource*  resource);
typedef ComputeResourceRW*  (*ComputeResourceRWCreate_ptr_t)(ComputeContext*  context, const ComputeResourceRWDesc*  desc);
typedef void  (*ComputeResourceRWUpdate_ptr_t)(ComputeContext*  context, ComputeResourceRW*  resourceRW, const ComputeResourceRWDesc*  desc);
typedef void  (*ComputeResourceRWRelease_ptr_t)(ComputeResourceRW*  resourceRW);
typedef ComputeResource*  (*ComputeResourceRWGetResource_ptr_t)(ComputeResourceRW*  resourceRW);
typedef void  (*ComputeContextDispatch_ptr_t)(ComputeContext*  context, const ComputeDispatchParams*  params);
typedef ComputeContext*  (*ComputeContextNvFlowContextCreate_ptr_t)(NvFlowContext*  flowContext);
typedef void  (*ComputeContextNvFlowContextUpdate_ptr_t)(ComputeContext*  computeContext, NvFlowContext*  flowContext);
typedef ComputeResource*  (*ComputeResourceNvFlowCreate_ptr_t)(ComputeContext*  context, NvFlowContext*  flowContext, NvFlowResource*  flowResource);
typedef void  (*ComputeResourceNvFlowUpdate_ptr_t)(ComputeContext*  context, ComputeResource*  resource, NvFlowContext*  flowContext, NvFlowResource*  flowResource);
typedef ComputeResourceRW*  (*ComputeResourceRWNvFlowCreate_ptr_t)(ComputeContext*  context, NvFlowContext*  flowContext, NvFlowResourceRW*  flowResourceRW);
typedef void  (*ComputeResourceRWNvFlowUpdate_ptr_t)(ComputeContext*  context, ComputeResourceRW*  resourceRW, NvFlowContext*  flowContext, NvFlowResourceRW*  flowResourceRW);

struct ComputeContextLoader 
{ 
	void* module = nullptr; 
	const char* suffix = ""; 
	char buf[1024u]; 

	ComputeContextCreate_ptr_t ComputeContextCreate_ptr;
	ComputeContextUpdate_ptr_t ComputeContextUpdate_ptr;
	ComputeContextRelease_ptr_t ComputeContextRelease_ptr;
	ComputeShaderCreate_ptr_t ComputeShaderCreate_ptr;
	ComputeShaderRelease_ptr_t ComputeShaderRelease_ptr;
	ComputeConstantBufferCreate_ptr_t ComputeConstantBufferCreate_ptr;
	ComputeConstantBufferRelease_ptr_t ComputeConstantBufferRelease_ptr;
	ComputeConstantBufferMap_ptr_t ComputeConstantBufferMap_ptr;
	ComputeConstantBufferUnmap_ptr_t ComputeConstantBufferUnmap_ptr;
	ComputeResourceCreate_ptr_t ComputeResourceCreate_ptr;
	ComputeResourceUpdate_ptr_t ComputeResourceUpdate_ptr;
	ComputeResourceRelease_ptr_t ComputeResourceRelease_ptr;
	ComputeResourceRWCreate_ptr_t ComputeResourceRWCreate_ptr;
	ComputeResourceRWUpdate_ptr_t ComputeResourceRWUpdate_ptr;
	ComputeResourceRWRelease_ptr_t ComputeResourceRWRelease_ptr;
	ComputeResourceRWGetResource_ptr_t ComputeResourceRWGetResource_ptr;
	ComputeContextDispatch_ptr_t ComputeContextDispatch_ptr;
	ComputeContextNvFlowContextCreate_ptr_t ComputeContextNvFlowContextCreate_ptr;
	ComputeContextNvFlowContextUpdate_ptr_t ComputeContextNvFlowContextUpdate_ptr;
	ComputeResourceNvFlowCreate_ptr_t ComputeResourceNvFlowCreate_ptr;
	ComputeResourceNvFlowUpdate_ptr_t ComputeResourceNvFlowUpdate_ptr;
	ComputeResourceRWNvFlowCreate_ptr_t ComputeResourceRWNvFlowCreate_ptr;
	ComputeResourceRWNvFlowUpdate_ptr_t ComputeResourceRWNvFlowUpdate_ptr;

}gComputeContextLoader; 

ComputeContext*  ComputeContextCreate(ComputeContextDesc*  desc)
{
	return gComputeContextLoader.ComputeContextCreate_ptr(desc);
}

void  ComputeContextUpdate(ComputeContext*  context, ComputeContextDesc*  desc)
{
	return gComputeContextLoader.ComputeContextUpdate_ptr(context, desc);
}

void  ComputeContextRelease(ComputeContext*  context)
{
	return gComputeContextLoader.ComputeContextRelease_ptr(context);
}

ComputeShader*  ComputeShaderCreate(ComputeContext*  context, const ComputeShaderDesc*  desc)
{
	return gComputeContextLoader.ComputeShaderCreate_ptr(context, desc);
}

void  ComputeShaderRelease(ComputeShader*  shader)
{
	return gComputeContextLoader.ComputeShaderRelease_ptr(shader);
}

ComputeConstantBuffer*  ComputeConstantBufferCreate(ComputeContext*  context, const ComputeConstantBufferDesc*  desc)
{
	return gComputeContextLoader.ComputeConstantBufferCreate_ptr(context, desc);
}

void  ComputeConstantBufferRelease(ComputeConstantBuffer*  constantBuffer)
{
	return gComputeContextLoader.ComputeConstantBufferRelease_ptr(constantBuffer);
}

void*  ComputeConstantBufferMap(ComputeContext*  context, ComputeConstantBuffer*  constantBuffer)
{
	return gComputeContextLoader.ComputeConstantBufferMap_ptr(context, constantBuffer);
}

void  ComputeConstantBufferUnmap(ComputeContext*  context, ComputeConstantBuffer*  constantBuffer)
{
	return gComputeContextLoader.ComputeConstantBufferUnmap_ptr(context, constantBuffer);
}

ComputeResource*  ComputeResourceCreate(ComputeContext*  context, const ComputeResourceDesc*  desc)
{
	return gComputeContextLoader.ComputeResourceCreate_ptr(context, desc);
}

void  ComputeResourceUpdate(ComputeContext*  context, ComputeResource*  resource, const ComputeResourceDesc*  desc)
{
	return gComputeContextLoader.ComputeResourceUpdate_ptr(context, resource, desc);
}

void  ComputeResourceRelease(ComputeResource*  resource)
{
	return gComputeContextLoader.ComputeResourceRelease_ptr(resource);
}

ComputeResourceRW*  ComputeResourceRWCreate(ComputeContext*  context, const ComputeResourceRWDesc*  desc)
{
	return gComputeContextLoader.ComputeResourceRWCreate_ptr(context, desc);
}

void  ComputeResourceRWUpdate(ComputeContext*  context, ComputeResourceRW*  resourceRW, const ComputeResourceRWDesc*  desc)
{
	return gComputeContextLoader.ComputeResourceRWUpdate_ptr(context, resourceRW, desc);
}

void  ComputeResourceRWRelease(ComputeResourceRW*  resourceRW)
{
	return gComputeContextLoader.ComputeResourceRWRelease_ptr(resourceRW);
}

ComputeResource*  ComputeResourceRWGetResource(ComputeResourceRW*  resourceRW)
{
	return gComputeContextLoader.ComputeResourceRWGetResource_ptr(resourceRW);
}

void  ComputeContextDispatch(ComputeContext*  context, const ComputeDispatchParams*  params)
{
	return gComputeContextLoader.ComputeContextDispatch_ptr(context, params);
}

ComputeContext*  ComputeContextNvFlowContextCreate(NvFlowContext*  flowContext)
{
	return gComputeContextLoader.ComputeContextNvFlowContextCreate_ptr(flowContext);
}

void  ComputeContextNvFlowContextUpdate(ComputeContext*  computeContext, NvFlowContext*  flowContext)
{
	return gComputeContextLoader.ComputeContextNvFlowContextUpdate_ptr(computeContext, flowContext);
}

ComputeResource*  ComputeResourceNvFlowCreate(ComputeContext*  context, NvFlowContext*  flowContext, NvFlowResource*  flowResource)
{
	return gComputeContextLoader.ComputeResourceNvFlowCreate_ptr(context, flowContext, flowResource);
}

void  ComputeResourceNvFlowUpdate(ComputeContext*  context, ComputeResource*  resource, NvFlowContext*  flowContext, NvFlowResource*  flowResource)
{
	return gComputeContextLoader.ComputeResourceNvFlowUpdate_ptr(context, resource, flowContext, flowResource);
}

ComputeResourceRW*  ComputeResourceRWNvFlowCreate(ComputeContext*  context, NvFlowContext*  flowContext, NvFlowResourceRW*  flowResourceRW)
{
	return gComputeContextLoader.ComputeResourceRWNvFlowCreate_ptr(context, flowContext, flowResourceRW);
}

void  ComputeResourceRWNvFlowUpdate(ComputeContext*  context, ComputeResourceRW*  resourceRW, NvFlowContext*  flowContext, NvFlowResourceRW*  flowResourceRW)
{
	return gComputeContextLoader.ComputeResourceRWNvFlowUpdate_ptr(context, resourceRW, flowContext, flowResourceRW);
}

void* computeContextLoaderLoadFunction(ComputeContextLoader* inst, const char* name)
{
	snprintf(inst->buf, 1024u, "%s%s", name, inst->suffix);

	return SDL_LoadFunction(inst->module, inst->buf);
}

void loadComputeContext(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	gComputeContextLoader.suffix = demoAppBackendSuffix(type);

	gComputeContextLoader.module = SDL_LoadObject(moduleName);

	gComputeContextLoader.ComputeContextCreate_ptr = (ComputeContextCreate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeContextCreate"));
	gComputeContextLoader.ComputeContextUpdate_ptr = (ComputeContextUpdate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeContextUpdate"));
	gComputeContextLoader.ComputeContextRelease_ptr = (ComputeContextRelease_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeContextRelease"));
	gComputeContextLoader.ComputeShaderCreate_ptr = (ComputeShaderCreate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeShaderCreate"));
	gComputeContextLoader.ComputeShaderRelease_ptr = (ComputeShaderRelease_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeShaderRelease"));
	gComputeContextLoader.ComputeConstantBufferCreate_ptr = (ComputeConstantBufferCreate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeConstantBufferCreate"));
	gComputeContextLoader.ComputeConstantBufferRelease_ptr = (ComputeConstantBufferRelease_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeConstantBufferRelease"));
	gComputeContextLoader.ComputeConstantBufferMap_ptr = (ComputeConstantBufferMap_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeConstantBufferMap"));
	gComputeContextLoader.ComputeConstantBufferUnmap_ptr = (ComputeConstantBufferUnmap_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeConstantBufferUnmap"));
	gComputeContextLoader.ComputeResourceCreate_ptr = (ComputeResourceCreate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceCreate"));
	gComputeContextLoader.ComputeResourceUpdate_ptr = (ComputeResourceUpdate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceUpdate"));
	gComputeContextLoader.ComputeResourceRelease_ptr = (ComputeResourceRelease_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceRelease"));
	gComputeContextLoader.ComputeResourceRWCreate_ptr = (ComputeResourceRWCreate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceRWCreate"));
	gComputeContextLoader.ComputeResourceRWUpdate_ptr = (ComputeResourceRWUpdate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceRWUpdate"));
	gComputeContextLoader.ComputeResourceRWRelease_ptr = (ComputeResourceRWRelease_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceRWRelease"));
	gComputeContextLoader.ComputeResourceRWGetResource_ptr = (ComputeResourceRWGetResource_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceRWGetResource"));
	gComputeContextLoader.ComputeContextDispatch_ptr = (ComputeContextDispatch_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeContextDispatch"));
	gComputeContextLoader.ComputeContextNvFlowContextCreate_ptr = (ComputeContextNvFlowContextCreate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeContextNvFlowContextCreate"));
	gComputeContextLoader.ComputeContextNvFlowContextUpdate_ptr = (ComputeContextNvFlowContextUpdate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeContextNvFlowContextUpdate"));
	gComputeContextLoader.ComputeResourceNvFlowCreate_ptr = (ComputeResourceNvFlowCreate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceNvFlowCreate"));
	gComputeContextLoader.ComputeResourceNvFlowUpdate_ptr = (ComputeResourceNvFlowUpdate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceNvFlowUpdate"));
	gComputeContextLoader.ComputeResourceRWNvFlowCreate_ptr = (ComputeResourceRWNvFlowCreate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceRWNvFlowCreate"));
	gComputeContextLoader.ComputeResourceRWNvFlowUpdate_ptr = (ComputeResourceRWNvFlowUpdate_ptr_t)(computeContextLoaderLoadFunction(&gComputeContextLoader, "ComputeResourceRWNvFlowUpdate"));
}

void unloadComputeContext()
{
	gComputeContextLoader.ComputeContextCreate_ptr = nullptr;
	gComputeContextLoader.ComputeContextUpdate_ptr = nullptr;
	gComputeContextLoader.ComputeContextRelease_ptr = nullptr;
	gComputeContextLoader.ComputeShaderCreate_ptr = nullptr;
	gComputeContextLoader.ComputeShaderRelease_ptr = nullptr;
	gComputeContextLoader.ComputeConstantBufferCreate_ptr = nullptr;
	gComputeContextLoader.ComputeConstantBufferRelease_ptr = nullptr;
	gComputeContextLoader.ComputeConstantBufferMap_ptr = nullptr;
	gComputeContextLoader.ComputeConstantBufferUnmap_ptr = nullptr;
	gComputeContextLoader.ComputeResourceCreate_ptr = nullptr;
	gComputeContextLoader.ComputeResourceUpdate_ptr = nullptr;
	gComputeContextLoader.ComputeResourceRelease_ptr = nullptr;
	gComputeContextLoader.ComputeResourceRWCreate_ptr = nullptr;
	gComputeContextLoader.ComputeResourceRWUpdate_ptr = nullptr;
	gComputeContextLoader.ComputeResourceRWRelease_ptr = nullptr;
	gComputeContextLoader.ComputeResourceRWGetResource_ptr = nullptr;
	gComputeContextLoader.ComputeContextDispatch_ptr = nullptr;
	gComputeContextLoader.ComputeContextNvFlowContextCreate_ptr = nullptr;
	gComputeContextLoader.ComputeContextNvFlowContextUpdate_ptr = nullptr;
	gComputeContextLoader.ComputeResourceNvFlowCreate_ptr = nullptr;
	gComputeContextLoader.ComputeResourceNvFlowUpdate_ptr = nullptr;
	gComputeContextLoader.ComputeResourceRWNvFlowCreate_ptr = nullptr;
	gComputeContextLoader.ComputeResourceRWNvFlowUpdate_ptr = nullptr;

	SDL_UnloadObject(gComputeContextLoader.module);
}
