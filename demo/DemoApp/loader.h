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