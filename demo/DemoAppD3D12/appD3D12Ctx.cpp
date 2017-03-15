/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

//direct3d headers
#include <d3d12.h>
#include <dxgi1_4.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")

#include "appD3D12Ctx.h"

#include <stdio.h>

#include <SDL.h>
#include <SDL_video.h>
#include <SDL_syswm.h>

namespace
{
	// COM object release utilities
	template <class T>
	void inline COMRelease(T& t)
	{
		if (t) t->Release();
		t = nullptr;
	}

	template <class T>
	void inline COMRelease(T& t, UINT arraySize)
	{
		for (UINT i = 0; i < arraySize; i++)
		{
			if (t[i]) t[i]->Release();
			t[i] = nullptr;
		}
	}
}

AppGraphProfiler* appGraphCreateProfiler(AppGraphCtx* ctx);
void appGraphProfilerFrameBegin(AppGraphProfiler* profiler);
void appGraphProfilerFrameEnd(AppGraphProfiler* profiler);
void appGraphProfilerEnable(AppGraphProfiler* profiler, bool enabled);
void appGraphProfilerBegin(AppGraphProfiler* profiler, const char* label);
void appGraphProfilerEnd(AppGraphProfiler* profiler, const char* label);
bool appGraphProfilerGet(AppGraphProfiler* profiler, const char** plabel, float* cpuTime, float* gpuTime, int index);
void appGraphReleaseProfiler(AppGraphProfiler* profiler);

void AppGraphCtxInitRenderTarget(AppGraphCtx* context, SDL_Window* window, bool fullscreen);

AppGraphCtx::AppGraphCtx()
{
	m_profiler = appGraphCreateProfiler(this);
}

AppGraphCtx::~AppGraphCtx()
{
	AppGraphCtxReleaseRenderTarget(this);

	COMRelease(m_device);
	COMRelease(m_commandQueue);
	COMRelease(m_rtvHeap);
	COMRelease(m_dsvHeap);
	COMRelease(m_depthSrvHeap);
	COMRelease(m_commandAllocators, m_frameCount);

	COMRelease(m_fence);
	CloseHandle(m_fenceEvent);

	COMRelease(m_commandList);

	m_dynamicHeapCbvSrvUav.release();

	appGraphReleaseProfiler(m_profiler);
	m_profiler = nullptr;
}

AppGraphCtx* AppGraphCtxCreate(int deviceID)
{
	AppGraphCtx* context = new AppGraphCtx;

	HRESULT hr = S_OK;

#if defined(_DEBUG)
	// Enable the D3D12 debug layer.
	{
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
		COMRelease(debugController);
	}
#endif

	UINT debugFlags = 0;
#ifdef _DEBUG
	debugFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// enumerate devices
	IDXGIFactory4* pFactory = NULL;
	CreateDXGIFactory2(debugFlags, IID_PPV_ARGS(&pFactory));
	IDXGIAdapter1* pAdapterTemp = NULL;
	IDXGIAdapter1* pAdapter = NULL;
	DXGI_ADAPTER_DESC1 adapterDesc;
	int adapterIdx = 0;
	while (S_OK == pFactory->EnumAdapters1(adapterIdx, &pAdapterTemp))
	{
		pAdapterTemp->GetDesc1(&adapterDesc);

		context->m_dedicatedVideoMemory = (size_t)adapterDesc.DedicatedVideoMemory;

		if (deviceID == adapterIdx)
		{
			pAdapter = pAdapterTemp;
			break;
		}
		else
		{
			pAdapterTemp->Release();
		}
		adapterIdx++;
	}

	// create device
	if (hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&context->m_device))
	{
		delete context;
		return nullptr;
	}

	// to disable annoying warning
#if 0
	context->m_device->SetStablePowerState(TRUE);
#endif

	// create command queue
	{
		D3D12_COMMAND_QUEUE_DESC desc;
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		if (hr = context->m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&context->m_commandQueue)))
		{
			delete context;
			return nullptr;
		}
	}

	// cleanup adapter and factory
	COMRelease(pAdapter);
	COMRelease(pFactory);

	// create RTV descriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = context->m_renderTargetCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (hr = context->m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&context->m_rtvHeap)))
		{
			delete context;
			return nullptr;
		}
		context->m_rtvDescriptorSize = context->m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	// create DSV descriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (hr = context->m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&context->m_dsvHeap)))
		{
			delete context;
			return nullptr;
		}
	}

	// create depth SRV descriptor heap
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (hr = context->m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&context->m_depthSrvHeap)))
		{
			delete context;
			return nullptr;
		}
	}

	// Create per frame resources
	{
		for (UINT idx = 0; idx < context->m_frameCount; idx++)
		{
			if (hr = context->m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context->m_commandAllocators[idx])))
			{
				delete context;
				return nullptr;
			}
		}
	}

	// create dynamic heap
	{
		context->m_dynamicHeapCbvSrvUav.init(context->m_device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 256u * 1024u);
	}

	// Create command list and close it
	{
		if (hr = context->m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
			context->m_commandAllocators[context->m_frameIndex], nullptr, IID_PPV_ARGS(&context->m_commandList))
			)
		{
			delete context;
			return nullptr;
		}
		context->m_commandList->Close();
	}

	// create synchronization objects
	{
		if (hr = context->m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&context->m_fence)))
		{
			delete context;
			return nullptr;
		}

		context->m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (context->m_fenceEvent == nullptr)
		{
			delete context;
			return nullptr;
		}
	}

	return context;
}

bool AppGraphCtxUpdateSize(AppGraphCtx* context, SDL_Window* window, bool fullscreen)
{
	// TODO: fix iflip fullscreen support
	fullscreen = false;

	bool sizeChanged = false;

	// sync with window
	{
		HWND hWnd = nullptr;
		// get Windows handle to this SDL window
		SDL_SysWMinfo winInfo;
		SDL_VERSION(&winInfo.version);
		if (SDL_GetWindowWMInfo(window, &winInfo))
		{
			if (winInfo.subsystem == SDL_SYSWM_WINDOWS)
			{
				hWnd = winInfo.info.win.window;
			}
		}
		context->m_hWnd = hWnd;
		context->m_fullscreen = fullscreen;

		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect(context->m_hWnd, &rc);
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;

		if (context->m_winW != width || context->m_winH != height)
		{
			context->m_winW = width;
			context->m_winH = height;
			sizeChanged = true;
			context->m_valid = (context->m_winW != 0 && context->m_winH != 0);
		}
	}

	if (sizeChanged)
	{
		AppGraphCtxReleaseRenderTarget(context);
	}
	if (sizeChanged && context->m_valid)
	{
		AppGraphCtxInitRenderTarget(context, window, fullscreen);
	}

	return context->m_valid;
}

void AppGraphCtxInitRenderTarget(AppGraphCtx* context, SDL_Window* window, bool fullscreen)
{
	HWND hWnd = nullptr;
	// get Windows handle to this SDL window
	SDL_SysWMinfo winInfo;
	SDL_VERSION(&winInfo.version);
	if (SDL_GetWindowWMInfo(window, &winInfo))
	{
		if (winInfo.subsystem == SDL_SYSWM_WINDOWS)
		{
			hWnd = winInfo.info.win.window;
		}
	}
	context->m_hWnd = hWnd;
	context->m_fullscreen = fullscreen;

	HRESULT hr = S_OK;

	UINT debugFlags = 0;
#ifdef _DEBUG
	debugFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	// enumerate devices
	IDXGIFactory4* pFactory = NULL;
	CreateDXGIFactory2(debugFlags, IID_PPV_ARGS(&pFactory));

	// create the swap chain
	for (int i = 0; i < 2; i++)
	{
		DXGI_SWAP_CHAIN_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.BufferCount = context->m_renderTargetCount;
		desc.BufferDesc.Width = context->m_winW;
		desc.BufferDesc.Height = context->m_winH;
		desc.BufferDesc.Format = context->m_rtv_format; // DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.RefreshRate.Numerator = 0;
		desc.BufferDesc.RefreshRate.Denominator = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.OutputWindow = context->m_hWnd;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Windowed = TRUE; // m_fullscreen ? FALSE : TRUE;
		desc.Flags = context->m_fullscreen ? 0u : DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

		context->m_current_rtvDesc.Format = context->m_rtv_format;
		context->m_current_rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		context->m_current_rtvDesc.Texture2D.MipSlice = 0u;
		context->m_current_rtvDesc.Texture2D.PlaneSlice = 0u;

		if (hr = pFactory->CreateSwapChain(context->m_commandQueue, &desc, (IDXGISwapChain**)&context->m_swapChain))
		{
			COMRelease(context->m_swapChain);
			context->m_fullscreen = false;
			continue;
		}

		if (!context->m_fullscreen)
		{
			context->m_swapChainWaitableObject = context->m_swapChain->GetFrameLatencyWaitableObject();
			context->m_swapChain->SetMaximumFrameLatency(context->m_renderTargetCount - 2);
		}
		else
		{
			hr = context->m_swapChain->SetFullscreenState(true, nullptr);
			if (hr != S_OK)
			{
				COMRelease(context->m_swapChain);
				context->m_fullscreen = false;
				continue;
			}
			DXGI_SWAP_CHAIN_DESC desc = {};
			context->m_swapChain->GetDesc(&desc);
			context->m_swapChain->ResizeBuffers(context->m_renderTargetCount, context->m_winW, context->m_winH, desc.BufferDesc.Format, desc.Flags);
		}

		context->m_frameIndex = context->m_swapChain->GetCurrentBackBufferIndex();
		break;
	}

	// configure scissor and viewport
	{
		context->m_viewport.Width = float(context->m_winW);
		context->m_viewport.Height = float(context->m_winH);
		context->m_viewport.MaxDepth = 1.f;

		context->m_scissorRect.right = context->m_winW;
		context->m_scissorRect.bottom = context->m_winH;
	}

	COMRelease(pFactory);

	// create per render target resources
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = context->m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

		for (UINT idx = 0; idx < context->m_renderTargetCount; idx++)
		{
			if (hr = context->m_swapChain->GetBuffer(idx, IID_PPV_ARGS(&context->m_renderTargets[idx])))
			{
				return;
			}
			context->m_device->CreateRenderTargetView(context->m_renderTargets[idx], nullptr, rtvHandle);
			rtvHandle.ptr += context->m_rtvDescriptorSize;
		}
	}

	// create the depth stencil
	{
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 0u;
		heapProps.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.MipLevels = 1u;
		texDesc.Format = context->m_depth_format; // DXGI_FORMAT_R32_TYPELESS; // DXGI_FORMAT_R24G8_TYPELESS
		texDesc.Width = context->m_winW;
		texDesc.Height = context->m_winH;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL /*| D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE*/;
		texDesc.DepthOrArraySize = 1u;
		texDesc.SampleDesc.Count = 1u;
		texDesc.SampleDesc.Quality = 0u;
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = context->m_dsv_format; // DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.f;
		clearValue.DepthStencil.Stencil = 0;

		if (hr = context->m_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&context->m_depthStencil)
			))
		{
			return;
		}

		// create the depth stencil view
		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = context->m_dsv_format; // DXGI_FORMAT_D32_FLOAT;
		viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		viewDesc.Flags = D3D12_DSV_FLAG_NONE;
		viewDesc.Texture2D.MipSlice = 0;

		context->m_current_dsvDesc = viewDesc;

		context->m_device->CreateDepthStencilView(context->m_depthStencil, &viewDesc, context->m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

		// create the depth SRV
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = context->m_depth_srv_format; // DXGI_FORMAT_R32_FLOAT;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.f;

		context->m_current_depth_srvDesc = srvDesc;

		D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = context->m_depthSrvHeap->GetCPUDescriptorHandleForHeapStart();
		context->m_device->CreateShaderResourceView(context->m_depthStencil, &srvDesc, srvHandle);
	}
}

void AppGraphCtxReleaseRenderTarget(AppGraphCtx* context)
{
	if (context->m_swapChain == nullptr)
	{
		return;
	}

	// need to make sure the pipeline is flushed
	for (UINT i = 0; i < context->m_frameCount; i++)
	{
		// check dependencies
		UINT64 fenceCompleted = context->m_fence->GetCompletedValue();
		if (fenceCompleted < context->m_fenceValues[i])
		{
			context->m_fence->SetEventOnCompletion(context->m_fenceValues[i], context->m_fenceEvent);
			WaitForSingleObjectEx(context->m_fenceEvent, INFINITE, FALSE);
		}
	}

	BOOL bFullscreen = FALSE;
	context->m_swapChain->GetFullscreenState(&bFullscreen, nullptr);
	if (bFullscreen == TRUE) context->m_swapChain->SetFullscreenState(FALSE, nullptr);

	COMRelease(context->m_swapChain);
	COMRelease(context->m_renderTargets, context->m_renderTargetCount);
	COMRelease(context->m_depthStencil);

	context->m_valid = false;
	context->m_winW = 0u;
	context->m_winH = 0u;
}

void AppGraphCtxRelease(AppGraphCtx* context)
{
	if (context == nullptr) return;

	delete context;
}

void AppGraphCtxFrameStart(AppGraphCtx* context, float clearColor[4])
{
	// Get back render target index
	context->m_renderTargetIndex = context->m_swapChain->GetCurrentBackBufferIndex();

	// check dependencies
	UINT64 fenceCompleted = context->m_fence->GetCompletedValue();
	if (fenceCompleted < context->m_fenceValues[context->m_frameIndex])
	{
		context->m_fence->SetEventOnCompletion(context->m_fenceValues[context->m_frameIndex], context->m_fenceEvent);
		WaitForSingleObjectEx(context->m_fenceEvent, INFINITE, FALSE);
	}

	// The fence ID associated with completion of this frame
	context->m_thisFrameFenceID = context->m_frameID + 1;
	context->m_lastFenceComplete = context->m_fence->GetCompletedValue();

	// reset this frame's command allocator
	context->m_commandAllocators[context->m_frameIndex]->Reset();

	// reset command list with this frame's allocator
	context->m_commandList->Reset(context->m_commandAllocators[context->m_frameIndex], nullptr);

	appGraphProfilerFrameBegin(context->m_profiler);

	context->m_commandList->RSSetViewports(1, &context->m_viewport);
	context->m_commandList->RSSetScissorRects(1, &context->m_scissorRect);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = context->m_renderTargets[context->m_renderTargetIndex];
	barrier.Transition.Subresource = 0u;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	context->m_commandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = context->m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += context->m_renderTargetIndex * context->m_rtvDescriptorSize;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = context->m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	context->m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	context->m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	context->m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);

	/// to simplify interop implementation
	context->m_current_renderTarget = context->m_renderTargets[context->m_renderTargetIndex];
	context->m_current_rtvHandle = rtvHandle;
	context->m_current_dsvHandle = dsvHandle;
	context->m_current_depth_srvHandle = context->m_depthSrvHeap->GetCPUDescriptorHandleForHeapStart();
}

void AppGraphCtxFramePresent(AppGraphCtx* context, bool fullsync)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = context->m_renderTargets[context->m_renderTargetIndex];
	barrier.Transition.Subresource = 0u;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	context->m_commandList->ResourceBarrier(1, &barrier);

	context->m_commandList->Close();

	// submit command list
	ID3D12CommandList* cmdLists[] = { context->m_commandList };
	context->m_commandQueue->ExecuteCommandLists(1, cmdLists);

	// check if now is good time to present
	bool shouldPresent = context->m_fullscreen ? true : WaitForSingleObjectEx(context->m_swapChainWaitableObject, 0, TRUE) != WAIT_TIMEOUT;
	if (shouldPresent)
	{
		context->m_swapChain->Present(0, 0);
		context->m_renderTargetID++;
	}

	appGraphProfilerFrameEnd(context->m_profiler);

	// signal for this frame id
	context->m_frameID++;
	context->m_fenceValues[context->m_frameIndex] = context->m_frameID;
	context->m_commandQueue->Signal(context->m_fence, context->m_frameID);

	// increment frame index after signal
	context->m_frameIndex = (context->m_frameIndex + 1) % context->m_frameCount;

	if (fullsync)
	{
		// check dependencies
		for (int frameIndex = 0; frameIndex < context->m_frameCount; frameIndex++)
		{
			UINT64 fenceCompleted = context->m_fence->GetCompletedValue();
			if (fenceCompleted < context->m_fenceValues[frameIndex])
			{
				context->m_fence->SetEventOnCompletion(context->m_fenceValues[frameIndex], context->m_fenceEvent);
				WaitForSingleObjectEx(context->m_fenceEvent, INFINITE, FALSE);
			}
		}
	}
}

void AppGraphCtxWaitForFrames(AppGraphCtx* context, unsigned int maxFramesInFlight)
{
	unsigned int framesActive = maxFramesInFlight;
	while (framesActive >= maxFramesInFlight)
	{
		// reset count each cycle, and get latest fence value
		framesActive = 0u;
		UINT64 fenceCompleted = context->m_fence->GetCompletedValue();

		// determine how many frames are in flight
		for (int frameIndex = 0; frameIndex < context->m_frameCount; frameIndex++)
		{
			if (fenceCompleted < context->m_fenceValues[frameIndex])
			{
				framesActive++;
			}
		}

		if (framesActive >= maxFramesInFlight)
		{
			// find the active frame with the lowest fence ID
			UINT64 minFenceID = 0;
			unsigned int minFrameIdx = 0;
			for (int frameIndex = 0; frameIndex < context->m_frameCount; frameIndex++)
			{
				if (fenceCompleted < context->m_fenceValues[frameIndex])
				{
					if (minFenceID == 0)
					{
						minFenceID = context->m_fenceValues[frameIndex];
						minFrameIdx = frameIndex;
					}
					else if (context->m_fenceValues[frameIndex] < minFenceID)
					{
						minFenceID = context->m_fenceValues[frameIndex];
						minFrameIdx = frameIndex;
					}
				}
			}
			// Wait for min frame
			{
				unsigned int frameIndex = minFrameIdx;
				fenceCompleted = context->m_fence->GetCompletedValue();
				if (fenceCompleted < context->m_fenceValues[frameIndex])
				{
					context->m_fence->SetEventOnCompletion(context->m_fenceValues[frameIndex], context->m_fenceEvent);
					WaitForSingleObjectEx(context->m_fenceEvent, INFINITE, FALSE);
				}
			}
		}
	}
}

void AppGraphCtxProfileEnable(AppGraphCtx* context, bool enabled)
{
	appGraphProfilerEnable(context->m_profiler, enabled);
}

void AppGraphCtxProfileBegin(AppGraphCtx* context, const char* label)
{
	appGraphProfilerBegin(context->m_profiler, label);
}

void AppGraphCtxProfileEnd(AppGraphCtx* context, const char* label)
{
	appGraphProfilerEnd(context->m_profiler, label);
}

bool AppGraphCtxProfileGet(AppGraphCtx* context, const char** plabel, float* cpuTime, float* gpuTime, int index)
{
	return appGraphProfilerGet(context->m_profiler, plabel, cpuTime, gpuTime, index);
}

// ******************************* Dynamic descriptor heap ******************************

void AppDynamicDescriptorHeapD3D12::init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT minHeapSize)
{
	m_device = device;
	m_heapSize = minHeapSize;
	m_startSlot = 0u;
	m_descriptorSize = m_device->GetDescriptorHandleIncrementSize(heapType);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = m_heapSize;
	desc.Type = heapType;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap));
}

void AppDynamicDescriptorHeapD3D12::release()
{
	m_device = nullptr;
	COMRelease(m_heap);
	m_descriptorSize = 0u;
	m_startSlot = 0u;
	m_heapSize = 0u;
}

AppDescriptorReserveHandleD3D12 AppDynamicDescriptorHeapD3D12::reserveDescriptors(UINT numDescriptors, UINT64 lastFenceCompleted, UINT64 nextFenceValue)
{
	UINT endSlot = m_startSlot + numDescriptors;
	if (endSlot >= m_heapSize)
	{
		m_startSlot = 0u;
		endSlot = numDescriptors;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	cpuHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += m_startSlot * m_descriptorSize;
	gpuHandle = m_heap->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += m_startSlot * m_descriptorSize;

	// advance start slot
	m_startSlot = endSlot;

	AppDescriptorReserveHandleD3D12 handle = {};
	handle.heap = m_heap;
	handle.descriptorSize = m_descriptorSize;
	handle.cpuHandle = cpuHandle;
	handle.gpuHandle = gpuHandle;
	return handle;
}

// ******************************* Profiler *********************************

namespace
{
	struct TimerCPU
	{
		LARGE_INTEGER oldCount;
		LARGE_INTEGER count;
		LARGE_INTEGER freq;
		TimerCPU()
		{
			QueryPerformanceCounter(&count);
			QueryPerformanceFrequency(&freq);
			oldCount = count;
		}
		double getDeltaTime()
		{
			QueryPerformanceCounter(&count);
			double dt = double(count.QuadPart - oldCount.QuadPart) / double(freq.QuadPart);
			oldCount = count;
			return dt;
		}
	};

	struct TimerGPU
	{
		ID3D12QueryHeap* m_queryHeap = nullptr;
		ID3D12Resource* m_queryReadback = nullptr;
		UINT64 m_queryFrequency = 0;
		UINT64 m_queryReadbackFenceVal = ~0llu;

		TimerGPU() {}
		~TimerGPU()
		{
			COMRelease(m_queryHeap);
			COMRelease(m_queryReadback);
		}
	};

	struct Timer
	{
		TimerCPU m_cpu;
		TimerGPU m_gpu;

		const char* m_label = nullptr;
		float m_cpuTime = 0.f;
		float m_gpuTime = 0.f;

		Timer() {}
		~Timer() {}
	};

	struct TimerValue
	{
		const char* m_label = nullptr;
		float m_cpuTime = 0.f;
		float m_gpuTime = 0.f;

		struct Stat
		{
			float m_time = 0.f;
			float m_maxTime = 0.f;
			float m_maxTimeAge = 0.f;

			float m_smoothTime = 0.f;
			float m_smoothTimeSum = 0.f;
			float m_smoothTimeCount = 0.f;

			Stat() {}
			void push(float time)
			{
				m_time = time;

				if (m_time > m_maxTime)
				{
					m_maxTime = m_time;
					m_maxTimeAge = 0.f;
				}

				if (fabsf(m_time - m_maxTime) < 0.25f * m_maxTime)
				{
					m_smoothTimeSum += m_time;
					m_smoothTimeCount += 1.f;
					m_smoothTimeSum *= 0.98f;
					m_smoothTimeCount *= 0.98f;
					m_smoothTime = m_smoothTimeSum / m_smoothTimeCount;
				}
			}

			float pull(float frameTime)
			{
				m_maxTimeAge += frameTime;

				if (m_maxTimeAge > 1.f)
				{
					m_maxTimeAge = 0.f;
					m_maxTime = m_time;
					m_smoothTimeSum = 0.f;
					m_smoothTimeCount = 0.f;
				}
				return m_smoothTime;
			}
		};

		Stat m_cpu;
		Stat m_gpu;

		void push(float cpuTime, float gpuTime)
		{
			m_cpu.push(cpuTime);
			m_gpu.push(gpuTime);
		}

		void pull(float frameTime)
		{
			m_cpuTime = m_cpu.pull(frameTime);
			m_gpuTime = m_gpu.pull(frameTime);
		}
	};

	struct HeapPropsReadback : public D3D12_HEAP_PROPERTIES
	{
		HeapPropsReadback()
		{
			Type = D3D12_HEAP_TYPE_READBACK;
			CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			CreationNodeMask = 0u;
			VisibleNodeMask = 0u;
		}
	};
	struct ResourceDescBuffer : public D3D12_RESOURCE_DESC
	{
		ResourceDescBuffer(UINT64 size)
		{
			Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			Alignment = 0u;
			Width = size;
			Height = 1u;
			DepthOrArraySize = 1u;
			MipLevels = 1;
			Format = DXGI_FORMAT_UNKNOWN;
			SampleDesc.Count = 1u;
			SampleDesc.Quality = 0u;
			Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			Flags = D3D12_RESOURCE_FLAG_NONE;
		}
	};
}

struct AppGraphProfiler
{
	AppGraphCtx* m_context;

	int m_state = 0;
	bool m_enabled = false;

	TimerCPU m_frameTimer;
	float m_frameTime = 0.f;

	static const int m_timersCap = 64;
	Timer m_timers[m_timersCap];
	int m_timersSize = 0;

	TimerValue m_timerValues[m_timersCap];
	int m_timerValuesSize = 0;

	AppGraphProfiler(AppGraphCtx* context);
	~AppGraphProfiler();
};

AppGraphProfiler::AppGraphProfiler(AppGraphCtx* context) : m_context(context)
{
}

AppGraphProfiler::~AppGraphProfiler()
{
}

AppGraphProfiler* appGraphCreateProfiler(AppGraphCtx* ctx)
{
	return new AppGraphProfiler(ctx);
}

void appGraphReleaseProfiler(AppGraphProfiler* profiler)
{
	delete profiler;
}

void appGraphProfilerFrameBegin(AppGraphProfiler* p)
{
	p->m_frameTime = (float)p->m_frameTimer.getDeltaTime();

	if (p->m_state == 0 && p->m_enabled)
	{
		p->m_timersSize = 0;

		p->m_state = 1;
	}
}

void appGraphProfilerFrameEnd(AppGraphProfiler* p)
{
	if (p->m_state == 1)
	{
		p->m_state = 2;
	}
}

void appGraphProfilerEnable(AppGraphProfiler* p, bool enabled)
{
	p->m_enabled = enabled;
}

void appGraphProfilerBegin(AppGraphProfiler* p, const char* label)
{
	if (p->m_state == 1 && p->m_timersSize < p->m_timersCap)
	{
		auto& timer = p->m_timers[p->m_timersSize++];
		timer.m_label = label;
		timer.m_cpu.getDeltaTime();

		auto device = p->m_context->m_device;

		if (timer.m_gpu.m_queryHeap == nullptr)
		{
			D3D12_QUERY_HEAP_DESC queryDesc = {};
			queryDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
			queryDesc.Count = 2;
			queryDesc.NodeMask = 0;

			device->CreateQueryHeap(&queryDesc, IID_PPV_ARGS(&timer.m_gpu.m_queryHeap));

			HeapPropsReadback readbackProps;
			ResourceDescBuffer resDesc(2 * sizeof(UINT64));
			resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			device->CreateCommittedResource(&readbackProps, D3D12_HEAP_FLAG_NONE,
				&resDesc, D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr, IID_PPV_ARGS(&timer.m_gpu.m_queryReadback));
		}

		p->m_context->m_commandQueue->GetTimestampFrequency(&timer.m_gpu.m_queryFrequency);

		p->m_context->m_commandList->EndQuery(timer.m_gpu.m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0);
	}
}

void appGraphProfilerEnd(AppGraphProfiler* p, const char* label)
{
	if (p->m_state == 1)
	{
		Timer* timer = nullptr;
		for (int i = 0; i < p->m_timersSize; i++)
		{
			if (strcmp(p->m_timers[i].m_label, label) == 0)
			{
				timer = &p->m_timers[i];
				break;
			}
		}
		if (timer)
		{
			p->m_context->m_commandList->EndQuery(timer->m_gpu.m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 1);

			p->m_context->m_commandList->ResolveQueryData(timer->m_gpu.m_queryHeap, D3D12_QUERY_TYPE_TIMESTAMP, 0, 2, timer->m_gpu.m_queryReadback, 0u);

			timer->m_gpu.m_queryReadbackFenceVal = p->m_context->m_thisFrameFenceID;

			timer->m_cpuTime = (float)timer->m_cpu.getDeltaTime();
		}
	}
}

bool appGraphProfilerFlush(AppGraphProfiler* p)
{
	if (p->m_state == 2)
	{
		for (int i = 0; i < p->m_timersSize; i++)
		{
			Timer& timer = p->m_timers[i];

			if (timer.m_gpu.m_queryReadbackFenceVal > p->m_context->m_lastFenceComplete)
			{
				return false;
			}

			UINT64 tsBegin, tsEnd;
			{
				void* data;
				// Read range is nullptr, meaning full read access
				D3D12_RANGE readRange;
				readRange.Begin = 0u;
				readRange.End = 2 * sizeof(UINT64);
				timer.m_gpu.m_queryReadback->Map(0u, &readRange, &data);
				if (data)
				{
					auto mapped = (UINT64*)data;
					tsBegin = mapped[0];
					tsEnd = mapped[1];

					D3D12_RANGE writeRange{};
					timer.m_gpu.m_queryReadback->Unmap(0u, &writeRange);
				}
			}

			timer.m_gpuTime = float(tsEnd - tsBegin) / float(timer.m_gpu.m_queryFrequency);

			// update TimerValue
			int j = 0;
			for (; j < p->m_timerValuesSize; j++)
			{
				TimerValue& value = p->m_timerValues[j];
				if (strcmp(value.m_label, timer.m_label) == 0)
				{
					value.push(timer.m_cpuTime, timer.m_gpuTime);
					break;
				}
			}
			if (j >= p->m_timerValuesSize && p->m_timerValuesSize < p->m_timersCap)
			{
				TimerValue& value = p->m_timerValues[p->m_timerValuesSize++];
				value.m_label = timer.m_label;
				value.push(timer.m_cpuTime, timer.m_gpuTime);
			}
		}

		p->m_state = 0;
	}
	return false;
}

bool appGraphProfilerGet(AppGraphProfiler* p, const char** plabel, float* cpuTime, float* gpuTime, int index)
{
	appGraphProfilerFlush(p);
	{
		if (index < p->m_timerValuesSize)
		{
			TimerValue& timer = p->m_timerValues[index];

			timer.pull(p->m_frameTime);

			if (plabel) *plabel = timer.m_label;
			if (cpuTime) *cpuTime = timer.m_cpuTime;
			if (gpuTime) *gpuTime = timer.m_gpuTime;

			return true;
		}
	}
	return false;
}

size_t AppGraphCtxDedicatedVideoMemory(AppGraphCtx* context)
{
	return context->m_dedicatedVideoMemory;
}