/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef APP_D3D12_CTX_H
#define APP_D3D12_CTX_H

#include "../DemoApp/appGraphCtx.h"

struct IDXGISwapChain3;

struct AppGraphProfiler;

struct AppDescriptorReserveHandleD3D12
{
	ID3D12DescriptorHeap* heap;
	UINT descriptorSize;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

struct AppDynamicDescriptorHeapD3D12
{
	ID3D12Device* m_device = nullptr;
	ID3D12DescriptorHeap* m_heap = nullptr;
	UINT m_descriptorSize = 0u;
	UINT m_startSlot = 0u;
	UINT m_heapSize = 0u;

	void init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT minHeapSize);
	void release();
	AppDescriptorReserveHandleD3D12 reserveDescriptors(UINT numDescriptors, UINT64 lastFenceCompleted, UINT64 nextFenceValue);

	AppDynamicDescriptorHeapD3D12() {}
	~AppDynamicDescriptorHeapD3D12() { release(); }
};

struct AppGraphCtx
{
	HWND                    m_hWnd = nullptr;

	int m_winW = 0;
	int m_winH = 0;
	bool m_fullscreen = false;
	bool m_valid = false;

	size_t m_dedicatedVideoMemory = 0u;

	// D3D12 non-replicated objects
	D3D12_VIEWPORT          m_viewport = {};
	D3D12_RECT              m_scissorRect = {};
	ID3D12Device*           m_device = nullptr;
	ID3D12CommandQueue*     m_commandQueue = nullptr;

	// D3D12 render target pipeline
	static const UINT       m_renderTargetCount = 6u;
	UINT                    m_renderTargetIndex = 0u;
	UINT64                  m_renderTargetID = 0u;
	IDXGISwapChain3*        m_swapChain = nullptr;
	HANDLE                  m_swapChainWaitableObject = nullptr;
	ID3D12DescriptorHeap*   m_rtvHeap = nullptr;
	UINT                    m_rtvDescriptorSize = 0u;
	ID3D12Resource*         m_renderTargets[m_renderTargetCount] = {};

	ID3D12Resource*			m_depthStencil = nullptr;
	ID3D12DescriptorHeap*	m_dsvHeap = nullptr;
	ID3D12DescriptorHeap*	m_depthSrvHeap = nullptr;

	// D3D12 frame pipeline objects
	static const UINT       m_frameCount = 8u;
	UINT                    m_frameIndex = 0u;
	UINT64                  m_frameID = 0u;
	ID3D12CommandAllocator* m_commandAllocators[m_frameCount] = {};

	// D3D12 synchronization objects
	ID3D12Fence*            m_fence = nullptr;
	HANDLE                  m_fenceEvent = 0u;
	UINT64					m_fenceValues[m_frameCount] = {};

	// fence values for library synchronization
	UINT64					m_lastFenceComplete = 1u;
	UINT64					m_thisFrameFenceID = 2u;

	// D3D12 per asset objects
	ID3D12GraphicsCommandList*	m_commandList = nullptr;
	ID3D12Resource*				m_current_renderTarget = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_current_rtvHandle;
	D3D12_RENDER_TARGET_VIEW_DESC m_current_rtvDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE m_current_dsvHandle;
	D3D12_DEPTH_STENCIL_VIEW_DESC m_current_dsvDesc;
	D3D12_CPU_DESCRIPTOR_HANDLE m_current_depth_srvHandle;
	D3D12_SHADER_RESOURCE_VIEW_DESC m_current_depth_srvDesc;

	DXGI_FORMAT m_rtv_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT m_dsv_format = DXGI_FORMAT_D32_FLOAT;
	DXGI_FORMAT m_depth_srv_format = DXGI_FORMAT_R32_FLOAT;
	DXGI_FORMAT m_depth_format = DXGI_FORMAT_R32_TYPELESS;

	AppDynamicDescriptorHeapD3D12 m_dynamicHeapCbvSrvUav;

	AppGraphProfiler* m_profiler = nullptr;

	AppGraphCtx();
	~AppGraphCtx();
};

#endif