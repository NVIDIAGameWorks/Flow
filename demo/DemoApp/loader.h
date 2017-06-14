/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

enum AppGraphCtxType
{
	APP_CONTEXT_D3D11 = 1,
	APP_CONTEXT_D3D12 = 2
};

#ifndef DEMOAPP_STR
#define XDEMOAPP_STR(s) DEMOAPP_STR(s)
#define DEMOAPP_STR(s) #s
#endif

namespace
{
	const char* demoAppDLLName(AppGraphCtxType type)
	{
		return (type == APP_CONTEXT_D3D12) ?
			"DemoAppD3D12" XDEMOAPP_STR(DLL_SUFFIX) ".dll" :
			"DemoAppD3D11" XDEMOAPP_STR(DLL_SUFFIX) ".dll";
	}

	const char* demoAppBackendSuffix(AppGraphCtxType type)
	{
		return (type == APP_CONTEXT_D3D12) ?
			"D3D12" :
			"D3D11";
	}

	const char* nvFlowDLLName(AppGraphCtxType type)
	{
		return (type == APP_CONTEXT_D3D12) ?
			"NvFlowD3D12" XDEMOAPP_STR(DLL_SUFFIX) ".dll" :
			"NvFlowD3D11" XDEMOAPP_STR(DLL_SUFFIX) ".dll";
	}
}

void loadModules(AppGraphCtxType type);
void unloadModules();

void loadAppGraphCtx(AppGraphCtxType type);
void unloadAppGraphCtx();

void loadNvFlowInterop(AppGraphCtxType type);
void unloadNvFlowInterop();

void loadMesh(AppGraphCtxType type);
void unloadMesh();

void loadImgui(AppGraphCtxType type);
void unloadImgui();

void loadComputeContext(AppGraphCtxType type);
void unloadComputeContext();