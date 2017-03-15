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

#include "../DemoApp/appGraphCtx.h"

struct AppGraphProfiler;

struct AppGraphCtx
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

	AppGraphProfiler* m_profiler = nullptr;

	AppGraphCtx();
	~AppGraphCtx();
};