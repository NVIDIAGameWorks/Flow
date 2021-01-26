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

#include "../DemoApp/appGraphCtx.h"

struct AppGraphProfilerD3D11;

struct AppGraphCtxD3D11
{
	HWND                    m_hWnd = nullptr;

	int m_winW = 0;
	int m_winH = 0;
	bool m_fullscreen = false;
	bool m_valid = false;

	size_t m_dedicatedVideoMemory = 0u;

	// D3D11 objects
	D3D11_VIEWPORT				m_viewport = {};
	ID3D11Device*				m_device = nullptr;
	ID3D11DeviceContext*		m_deviceContext = nullptr;
	IDXGISwapChain*				m_swapChain = nullptr;
	ID3D11RenderTargetView*		m_rtv = nullptr;
	ID3D11Texture2D*			m_depthStencil = nullptr;
	ID3D11DepthStencilView*		m_dsv = nullptr;
	ID3D11ShaderResourceView*	m_depthSRV = nullptr;
	ID3D11DepthStencilState*	m_depthState = nullptr;

	AppGraphProfilerD3D11* m_profiler = nullptr;

	AppGraphCtxD3D11();
	~AppGraphCtxD3D11();
};

inline AppGraphCtxD3D11* cast_to_AppGraphCtxD3D11(AppGraphCtx* appctx)
{
	return (AppGraphCtxD3D11*)(appctx);
}

inline AppGraphCtx* cast_from_AppGraphCtxD3D11(AppGraphCtxD3D11* appctx)
{
	return (AppGraphCtx*)(appctx);
}

APP_GRAPH_CTX_API AppGraphCtx* AppGraphCtxCreateD3D11(int deviceID);

APP_GRAPH_CTX_API bool AppGraphCtxUpdateSizeD3D11(AppGraphCtx* context, SDL_Window* window, bool fullscreen);

APP_GRAPH_CTX_API void AppGraphCtxReleaseRenderTargetD3D11(AppGraphCtx* context);

APP_GRAPH_CTX_API void AppGraphCtxReleaseD3D11(AppGraphCtx* context);

APP_GRAPH_CTX_API void AppGraphCtxFrameStartD3D11(AppGraphCtx* context, AppGraphColor clearColor);

APP_GRAPH_CTX_API void AppGraphCtxFramePresentD3D11(AppGraphCtx* context, bool fullsync);

APP_GRAPH_CTX_API void AppGraphCtxWaitForFramesD3D11(AppGraphCtx* context, unsigned int maxFramesInFlight);

APP_GRAPH_CTX_API void AppGraphCtxProfileEnableD3D11(AppGraphCtx* context, bool enabled);

APP_GRAPH_CTX_API void AppGraphCtxProfileBeginD3D11(AppGraphCtx* context, const char* label);

APP_GRAPH_CTX_API void AppGraphCtxProfileEndD3D11(AppGraphCtx* context, const char* label);

APP_GRAPH_CTX_API bool AppGraphCtxProfileGetD3D11(AppGraphCtx* context, const char** plabel, float* cpuTime, float* gpuTime, int index);

APP_GRAPH_CTX_API size_t AppGraphCtxDedicatedVideoMemoryD3D11(AppGraphCtx* context);