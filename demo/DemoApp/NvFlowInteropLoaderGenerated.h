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

typedef NvFlowContext*  (*NvFlowInteropCreateContext_ptr_t)(AppGraphCtx*  appctx);
typedef NvFlowDepthStencilView*  (*NvFlowInteropCreateDepthStencilView_ptr_t)(AppGraphCtx*  appctx, NvFlowContext*  flowctx);
typedef NvFlowRenderTargetView*  (*NvFlowInteropCreateRenderTargetView_ptr_t)(AppGraphCtx*  appctx, NvFlowContext*  flowctx);
typedef void  (*NvFlowInteropUpdateContext_ptr_t)(NvFlowContext*  context, AppGraphCtx*  appctx);
typedef void  (*NvFlowInteropUpdateDepthStencilView_ptr_t)(NvFlowDepthStencilView*  view, AppGraphCtx*  appctx, NvFlowContext*  flowctx);
typedef void  (*NvFlowInteropUpdateRenderTargetView_ptr_t)(NvFlowRenderTargetView*  view, AppGraphCtx*  appctx, NvFlowContext*  flowctx);

struct NvFlowInteropLoader 
{ 
	void* module = nullptr; 
	const char* suffix = ""; 
	char buf[1024u]; 

	NvFlowInteropCreateContext_ptr_t NvFlowInteropCreateContext_ptr;
	NvFlowInteropCreateDepthStencilView_ptr_t NvFlowInteropCreateDepthStencilView_ptr;
	NvFlowInteropCreateRenderTargetView_ptr_t NvFlowInteropCreateRenderTargetView_ptr;
	NvFlowInteropUpdateContext_ptr_t NvFlowInteropUpdateContext_ptr;
	NvFlowInteropUpdateDepthStencilView_ptr_t NvFlowInteropUpdateDepthStencilView_ptr;
	NvFlowInteropUpdateRenderTargetView_ptr_t NvFlowInteropUpdateRenderTargetView_ptr;

}gNvFlowInteropLoader; 

NvFlowContext*  NvFlowInteropCreateContext(AppGraphCtx*  appctx)
{
	return gNvFlowInteropLoader.NvFlowInteropCreateContext_ptr(appctx);
}

NvFlowDepthStencilView*  NvFlowInteropCreateDepthStencilView(AppGraphCtx*  appctx, NvFlowContext*  flowctx)
{
	return gNvFlowInteropLoader.NvFlowInteropCreateDepthStencilView_ptr(appctx, flowctx);
}

NvFlowRenderTargetView*  NvFlowInteropCreateRenderTargetView(AppGraphCtx*  appctx, NvFlowContext*  flowctx)
{
	return gNvFlowInteropLoader.NvFlowInteropCreateRenderTargetView_ptr(appctx, flowctx);
}

void  NvFlowInteropUpdateContext(NvFlowContext*  context, AppGraphCtx*  appctx)
{
	return gNvFlowInteropLoader.NvFlowInteropUpdateContext_ptr(context, appctx);
}

void  NvFlowInteropUpdateDepthStencilView(NvFlowDepthStencilView*  view, AppGraphCtx*  appctx, NvFlowContext*  flowctx)
{
	return gNvFlowInteropLoader.NvFlowInteropUpdateDepthStencilView_ptr(view, appctx, flowctx);
}

void  NvFlowInteropUpdateRenderTargetView(NvFlowRenderTargetView*  view, AppGraphCtx*  appctx, NvFlowContext*  flowctx)
{
	return gNvFlowInteropLoader.NvFlowInteropUpdateRenderTargetView_ptr(view, appctx, flowctx);
}

void* nvFlowInteropLoaderLoadFunction(NvFlowInteropLoader* inst, const char* name)
{
	snprintf(inst->buf, 1024u, "%s%s", name, inst->suffix);

	return SDL_LoadFunction(inst->module, inst->buf);
}

void loadNvFlowInterop(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	gNvFlowInteropLoader.suffix = demoAppBackendSuffix(type);

	gNvFlowInteropLoader.module = SDL_LoadObject(moduleName);

	gNvFlowInteropLoader.NvFlowInteropCreateContext_ptr = (NvFlowInteropCreateContext_ptr_t)(nvFlowInteropLoaderLoadFunction(&gNvFlowInteropLoader, "NvFlowInteropCreateContext"));
	gNvFlowInteropLoader.NvFlowInteropCreateDepthStencilView_ptr = (NvFlowInteropCreateDepthStencilView_ptr_t)(nvFlowInteropLoaderLoadFunction(&gNvFlowInteropLoader, "NvFlowInteropCreateDepthStencilView"));
	gNvFlowInteropLoader.NvFlowInteropCreateRenderTargetView_ptr = (NvFlowInteropCreateRenderTargetView_ptr_t)(nvFlowInteropLoaderLoadFunction(&gNvFlowInteropLoader, "NvFlowInteropCreateRenderTargetView"));
	gNvFlowInteropLoader.NvFlowInteropUpdateContext_ptr = (NvFlowInteropUpdateContext_ptr_t)(nvFlowInteropLoaderLoadFunction(&gNvFlowInteropLoader, "NvFlowInteropUpdateContext"));
	gNvFlowInteropLoader.NvFlowInteropUpdateDepthStencilView_ptr = (NvFlowInteropUpdateDepthStencilView_ptr_t)(nvFlowInteropLoaderLoadFunction(&gNvFlowInteropLoader, "NvFlowInteropUpdateDepthStencilView"));
	gNvFlowInteropLoader.NvFlowInteropUpdateRenderTargetView_ptr = (NvFlowInteropUpdateRenderTargetView_ptr_t)(nvFlowInteropLoaderLoadFunction(&gNvFlowInteropLoader, "NvFlowInteropUpdateRenderTargetView"));
}

void unloadNvFlowInterop()
{
	gNvFlowInteropLoader.NvFlowInteropCreateContext_ptr = nullptr;
	gNvFlowInteropLoader.NvFlowInteropCreateDepthStencilView_ptr = nullptr;
	gNvFlowInteropLoader.NvFlowInteropCreateRenderTargetView_ptr = nullptr;
	gNvFlowInteropLoader.NvFlowInteropUpdateContext_ptr = nullptr;
	gNvFlowInteropLoader.NvFlowInteropUpdateDepthStencilView_ptr = nullptr;
	gNvFlowInteropLoader.NvFlowInteropUpdateRenderTargetView_ptr = nullptr;

	SDL_UnloadObject(gNvFlowInteropLoader.module);
}
