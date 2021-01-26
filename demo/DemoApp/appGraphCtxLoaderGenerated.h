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

typedef AppGraphCtx*  (*AppGraphCtxCreate_ptr_t)(int  deviceID);
typedef bool  (*AppGraphCtxUpdateSize_ptr_t)(AppGraphCtx*  context, SDL_Window*  window, bool  fullscreen);
typedef void  (*AppGraphCtxReleaseRenderTarget_ptr_t)(AppGraphCtx*  context);
typedef void  (*AppGraphCtxRelease_ptr_t)(AppGraphCtx*  context);
typedef void  (*AppGraphCtxFrameStart_ptr_t)(AppGraphCtx*  context, AppGraphColor  clearColor);
typedef void  (*AppGraphCtxFramePresent_ptr_t)(AppGraphCtx*  context, bool  fullsync);
typedef void  (*AppGraphCtxWaitForFrames_ptr_t)(AppGraphCtx*  context, unsigned int  maxFramesInFlight);
typedef void  (*AppGraphCtxProfileEnable_ptr_t)(AppGraphCtx*  context, bool  enabled);
typedef void  (*AppGraphCtxProfileBegin_ptr_t)(AppGraphCtx*  context, const char*  label);
typedef void  (*AppGraphCtxProfileEnd_ptr_t)(AppGraphCtx*  context, const char*  label);
typedef bool  (*AppGraphCtxProfileGet_ptr_t)(AppGraphCtx*  context, const char**  plabel, float*  cpuTime, float*  gpuTime, int  index);
typedef size_t  (*AppGraphCtxDedicatedVideoMemory_ptr_t)(AppGraphCtx*  context);

struct AppGraphCtxLoader 
{ 
	void* module = nullptr; 
	const char* suffix = ""; 
	char buf[1024u]; 

	AppGraphCtxCreate_ptr_t AppGraphCtxCreate_ptr;
	AppGraphCtxUpdateSize_ptr_t AppGraphCtxUpdateSize_ptr;
	AppGraphCtxReleaseRenderTarget_ptr_t AppGraphCtxReleaseRenderTarget_ptr;
	AppGraphCtxRelease_ptr_t AppGraphCtxRelease_ptr;
	AppGraphCtxFrameStart_ptr_t AppGraphCtxFrameStart_ptr;
	AppGraphCtxFramePresent_ptr_t AppGraphCtxFramePresent_ptr;
	AppGraphCtxWaitForFrames_ptr_t AppGraphCtxWaitForFrames_ptr;
	AppGraphCtxProfileEnable_ptr_t AppGraphCtxProfileEnable_ptr;
	AppGraphCtxProfileBegin_ptr_t AppGraphCtxProfileBegin_ptr;
	AppGraphCtxProfileEnd_ptr_t AppGraphCtxProfileEnd_ptr;
	AppGraphCtxProfileGet_ptr_t AppGraphCtxProfileGet_ptr;
	AppGraphCtxDedicatedVideoMemory_ptr_t AppGraphCtxDedicatedVideoMemory_ptr;

}gAppGraphCtxLoader; 

AppGraphCtx*  AppGraphCtxCreate(int  deviceID)
{
	return gAppGraphCtxLoader.AppGraphCtxCreate_ptr(deviceID);
}

bool  AppGraphCtxUpdateSize(AppGraphCtx*  context, SDL_Window*  window, bool  fullscreen)
{
	return gAppGraphCtxLoader.AppGraphCtxUpdateSize_ptr(context, window, fullscreen);
}

void  AppGraphCtxReleaseRenderTarget(AppGraphCtx*  context)
{
	return gAppGraphCtxLoader.AppGraphCtxReleaseRenderTarget_ptr(context);
}

void  AppGraphCtxRelease(AppGraphCtx*  context)
{
	return gAppGraphCtxLoader.AppGraphCtxRelease_ptr(context);
}

void  AppGraphCtxFrameStart(AppGraphCtx*  context, AppGraphColor  clearColor)
{
	return gAppGraphCtxLoader.AppGraphCtxFrameStart_ptr(context, clearColor);
}

void  AppGraphCtxFramePresent(AppGraphCtx*  context, bool  fullsync)
{
	return gAppGraphCtxLoader.AppGraphCtxFramePresent_ptr(context, fullsync);
}

void  AppGraphCtxWaitForFrames(AppGraphCtx*  context, unsigned int  maxFramesInFlight)
{
	return gAppGraphCtxLoader.AppGraphCtxWaitForFrames_ptr(context, maxFramesInFlight);
}

void  AppGraphCtxProfileEnable(AppGraphCtx*  context, bool  enabled)
{
	return gAppGraphCtxLoader.AppGraphCtxProfileEnable_ptr(context, enabled);
}

void  AppGraphCtxProfileBegin(AppGraphCtx*  context, const char*  label)
{
	return gAppGraphCtxLoader.AppGraphCtxProfileBegin_ptr(context, label);
}

void  AppGraphCtxProfileEnd(AppGraphCtx*  context, const char*  label)
{
	return gAppGraphCtxLoader.AppGraphCtxProfileEnd_ptr(context, label);
}

bool  AppGraphCtxProfileGet(AppGraphCtx*  context, const char**  plabel, float*  cpuTime, float*  gpuTime, int  index)
{
	return gAppGraphCtxLoader.AppGraphCtxProfileGet_ptr(context, plabel, cpuTime, gpuTime, index);
}

size_t  AppGraphCtxDedicatedVideoMemory(AppGraphCtx*  context)
{
	return gAppGraphCtxLoader.AppGraphCtxDedicatedVideoMemory_ptr(context);
}

void* appGraphCtxLoaderLoadFunction(AppGraphCtxLoader* inst, const char* name)
{
	snprintf(inst->buf, 1024u, "%s%s", name, inst->suffix);

	return SDL_LoadFunction(inst->module, inst->buf);
}

void loadAppGraphCtx(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	gAppGraphCtxLoader.suffix = demoAppBackendSuffix(type);

	gAppGraphCtxLoader.module = SDL_LoadObject(moduleName);

	gAppGraphCtxLoader.AppGraphCtxCreate_ptr = (AppGraphCtxCreate_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxCreate"));
	gAppGraphCtxLoader.AppGraphCtxUpdateSize_ptr = (AppGraphCtxUpdateSize_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxUpdateSize"));
	gAppGraphCtxLoader.AppGraphCtxReleaseRenderTarget_ptr = (AppGraphCtxReleaseRenderTarget_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxReleaseRenderTarget"));
	gAppGraphCtxLoader.AppGraphCtxRelease_ptr = (AppGraphCtxRelease_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxRelease"));
	gAppGraphCtxLoader.AppGraphCtxFrameStart_ptr = (AppGraphCtxFrameStart_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxFrameStart"));
	gAppGraphCtxLoader.AppGraphCtxFramePresent_ptr = (AppGraphCtxFramePresent_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxFramePresent"));
	gAppGraphCtxLoader.AppGraphCtxWaitForFrames_ptr = (AppGraphCtxWaitForFrames_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxWaitForFrames"));
	gAppGraphCtxLoader.AppGraphCtxProfileEnable_ptr = (AppGraphCtxProfileEnable_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxProfileEnable"));
	gAppGraphCtxLoader.AppGraphCtxProfileBegin_ptr = (AppGraphCtxProfileBegin_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxProfileBegin"));
	gAppGraphCtxLoader.AppGraphCtxProfileEnd_ptr = (AppGraphCtxProfileEnd_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxProfileEnd"));
	gAppGraphCtxLoader.AppGraphCtxProfileGet_ptr = (AppGraphCtxProfileGet_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxProfileGet"));
	gAppGraphCtxLoader.AppGraphCtxDedicatedVideoMemory_ptr = (AppGraphCtxDedicatedVideoMemory_ptr_t)(appGraphCtxLoaderLoadFunction(&gAppGraphCtxLoader, "AppGraphCtxDedicatedVideoMemory"));
}

void unloadAppGraphCtx()
{
	gAppGraphCtxLoader.AppGraphCtxCreate_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxUpdateSize_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxReleaseRenderTarget_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxRelease_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxFrameStart_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxFramePresent_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxWaitForFrames_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxProfileEnable_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxProfileBegin_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxProfileEnd_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxProfileGet_ptr = nullptr;
	gAppGraphCtxLoader.AppGraphCtxDedicatedVideoMemory_ptr = nullptr;

	SDL_UnloadObject(gAppGraphCtxLoader.module);
}
