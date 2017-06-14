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

// include the Direct3D Library file
#pragma comment (lib, "d3d12.lib")

#include <math.h>

#include <SDL.h>

#include "meshD3D12.h"

#include "meshVS.hlsl.h"
#include "meshPS.hlsl.h"

namespace
{
	template <class T>
	void inline COMRelease(T& t)
	{
		if (t) t->Release();
		t = nullptr;
	}
}

struct MeshConstantHeap
{
	MeshConstantHeap* m_next = nullptr;
	MeshContext* m_meshContext = nullptr;

	ID3D12Resource* m_constantBuffer = nullptr;
	UINT8* m_constantBufferData = nullptr;

	int m_allocIdx = 0;
	int m_size;

	MeshConstantHeap(MeshContext* meshContext, int size);
	~MeshConstantHeap()
	{
		COMRelease(m_constantBuffer);
		delete m_next;
	}

	void reset()
	{
		m_allocIdx = 0;
		if (m_next)
		{
			m_next->reset();
		}
	}

	UINT8* allocate(int size)
	{
		UINT8* ret = nullptr;
		size = 256 * ((size + 255) / 256);
		if (m_allocIdx + size <= m_size)
		{
			ret = m_constantBufferData + m_allocIdx;
			m_allocIdx += size;
		}
		if (ret == nullptr)
		{
			if (m_next)
			{
				return m_next->allocate(size);
			}
			else
			{
				m_next = new MeshConstantHeap(m_meshContext, m_size);
				return m_next->allocate(size);
			}
		}
		return ret;
	}

	D3D12_GPU_VIRTUAL_ADDRESS getVirtualAddress(UINT8* cpuAddress)
	{
		size_t offset = cpuAddress - m_constantBufferData;
		if (offset < m_size)
		{
			return m_constantBuffer->GetGPUVirtualAddress() + offset;
		}
		else
		{
			if (m_next)
			{
				return m_next->getVirtualAddress(cpuAddress);
			}
			else
			{
				return 0u;
			}
		}
	}
};

struct MeshContextD3D12
{
	ID3D12Device* m_device = nullptr;
	ID3D12GraphicsCommandList* m_commandList = nullptr;

	ID3D12RootSignature* m_rootSignature = nullptr;
	ID3D12PipelineState* m_pipelineStateLH = nullptr;
	ID3D12PipelineState* m_pipelineStateRH = nullptr;

	MeshConstantHeap* m_constantHeap = nullptr;

	MeshContextD3D12() {}
	~MeshContextD3D12()
	{
		COMRelease(m_rootSignature);
		COMRelease(m_pipelineStateLH);
		COMRelease(m_pipelineStateRH);
		if (m_constantHeap)
		{
			delete m_constantHeap;
			m_constantHeap = nullptr;
		}
	}
};

inline MeshContextD3D12* cast_to_MeshContextD3D12(MeshContext* ctx)
{
	return (MeshContextD3D12*)(ctx);
}

inline MeshContext* cast_from_MeshContextD3D12(MeshContextD3D12* ctx)
{
	return (MeshContext*)(ctx);
}

struct MeshIndexBufferD3D12
{
	ID3D12Resource* m_buffer = nullptr;
	MeshUint m_numElements = 0u;
	D3D12_INDEX_BUFFER_VIEW m_view = {};

	ID3D12Resource* m_upload = nullptr;

	MeshIndexBufferD3D12() {}
	~MeshIndexBufferD3D12()
	{
		COMRelease(m_buffer);
		COMRelease(m_upload);
	}
};

inline MeshIndexBufferD3D12* cast_to_MeshIndexBufferD3D12(MeshIndexBuffer* buf)
{
	return (MeshIndexBufferD3D12*)(buf);
}

inline MeshIndexBuffer* cast_from_MeshIndexBufferD3D12(MeshIndexBufferD3D12* buf)
{
	return (MeshIndexBuffer*)(buf);
}

struct MeshVertexBufferD3D12
{
	ID3D12Resource* m_buffer = nullptr;
	MeshUint m_numElements = 0u;
	D3D12_VERTEX_BUFFER_VIEW m_view = {};

	ID3D12Resource* m_upload = nullptr;

	MeshVertexBufferD3D12() {}
	~MeshVertexBufferD3D12()
	{
		COMRelease(m_buffer);
		COMRelease(m_upload);
	}
};

inline MeshVertexBufferD3D12* cast_to_MeshVertexBufferD3D12(MeshVertexBuffer* buf)
{
	return (MeshVertexBufferD3D12*)(buf);
}

inline MeshVertexBuffer* cast_from_MeshVertexBufferD3D12(MeshVertexBufferD3D12* buf)
{
	return (MeshVertexBuffer*)(buf);
}

MeshConstantHeap::MeshConstantHeap(MeshContext* meshContextIn, int size) : m_meshContext(meshContextIn)
{
	auto meshContext = cast_to_MeshContextD3D12(meshContextIn);

	// create a constant buffer
	{
		HRESULT hr = S_OK;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 0u;
		heapProps.VisibleNodeMask = 0u;

		m_size = 256 * ((size + 255) / 256);

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0u;
		desc.Width = m_size;
		desc.Height = 1u;
		desc.DepthOrArraySize = 1u;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1u;
		desc.SampleDesc.Quality = 0u;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (hr = meshContext->m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&m_constantBuffer)))
		{
		}

		UINT8* pdata;
		D3D12_RANGE readRange = {};
		if (hr = m_constantBuffer->Map(0, &readRange, (void**)&pdata))
		{
		}
		else
		{
			m_constantBufferData = pdata;
			//m_constantBuffer->Unmap(0, nullptr);		// leave mapped
		}
	}
}

MeshContext* MeshContextCreateD3D12(const MeshContextDesc* descIn)
{
	auto desc = cast_to_MeshContextDescD3D12(descIn);

	MeshContextD3D12* context = new MeshContextD3D12;

	context->m_device = desc->device;
	context->m_commandList = desc->commandList;

	// create the root signature
	{
		D3D12_ROOT_PARAMETER params[1];
		params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		params[0].Descriptor.ShaderRegister = 0u;
		params[0].Descriptor.RegisterSpace = 0u;
		params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC desc;
		desc.NumParameters = 1;
		desc.pParameters = params;
		desc.NumStaticSamplers = 0u;
		desc.pStaticSamplers = nullptr;
		desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		ID3DBlob* signature = nullptr;
		ID3DBlob* error = nullptr;
		HRESULT hr = S_OK;
		if (hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))
		{
			delete context;
			return nullptr;
		}
		if (hr = context->m_device->CreateRootSignature(0u, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&context->m_rootSignature)))
		{
			delete context;
			return nullptr;
		}
		COMRelease(signature);
		COMRelease(error);
	}

	// create the pipeline state object
	{
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		const bool wireFrame = false;

		D3D12_RASTERIZER_DESC rasterDesc;
		if (wireFrame)
		{
			rasterDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
			rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
		}
		else
		{
			rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
			rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
		}
		rasterDesc.FrontCounterClockwise = FALSE;
		rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterDesc.DepthClipEnable = TRUE;
		rasterDesc.MultisampleEnable = FALSE;
		rasterDesc.AntialiasedLineEnable = FALSE;
		rasterDesc.ForcedSampleCount = 0;
		rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		D3D12_BLEND_DESC blendDesc;
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		{
			const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
			{
				FALSE,FALSE,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};
			for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
				blendDesc.RenderTarget[i] = defaultRenderTargetBlendDesc;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout.NumElements = 2;
		psoDesc.InputLayout.pInputElementDescs = inputElementDescs;
		psoDesc.pRootSignature = context->m_rootSignature;
		psoDesc.VS.pShaderBytecode = g_meshVS;
		psoDesc.VS.BytecodeLength = sizeof(g_meshVS);
		psoDesc.PS.pShaderBytecode = g_meshPS;
		psoDesc.PS.BytecodeLength = sizeof(g_meshPS);
		psoDesc.RasterizerState = rasterDesc;
		psoDesc.BlendState = blendDesc;
		psoDesc.DepthStencilState.DepthEnable = TRUE;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		HRESULT hr = S_OK;

		if (hr = context->m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&context->m_pipelineStateLH)))
		{
			delete context;
			return nullptr;
		}

		psoDesc.RasterizerState.FrontCounterClockwise = TRUE;

		if (hr = context->m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&context->m_pipelineStateRH)))
		{
			delete context;
			return nullptr;
		}
	}

	// create constant heap
	{
		context->m_constantHeap = new MeshConstantHeap(cast_from_MeshContextD3D12(context), 4096u);
	}

	return cast_from_MeshContextD3D12(context);
}

void MeshContextUpdateD3D12(MeshContext* contextIn, const MeshContextDesc* descIn)
{
	auto context = cast_to_MeshContextD3D12(contextIn);
	auto desc = cast_to_MeshContextDescD3D12(descIn);

	context->m_device = desc->device;
	context->m_commandList = desc->commandList;

	context->m_constantHeap->reset();
}

void MeshContextReleaseD3D12(MeshContext* context)
{
	if (context == nullptr) return;

	delete cast_to_MeshContextD3D12(context);
}

MeshIndexBuffer* MeshIndexBufferCreateD3D12(MeshContext* contextIn, MeshUint* indices, MeshUint numIndices)
{
	auto context = cast_to_MeshContextD3D12(contextIn);

	MeshIndexBufferD3D12* buffer = new MeshIndexBufferD3D12;

	buffer->m_numElements = numIndices;
	// create an index buffer
	{
		HRESULT hr = S_OK;

		UINT bufferSize = (UINT)(numIndices) * sizeof(UINT);

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 0u;
		heapProps.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0u;
		desc.Width = bufferSize;
		desc.Height = 1u;
		desc.DepthOrArraySize = 1u;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1u;
		desc.SampleDesc.Quality = 0u;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (hr = context->m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&buffer->m_upload)))
		{
			delete buffer;
			return nullptr;
		}

		UINT8* pdata;
		D3D12_RANGE readRange = {};
		if (hr = buffer->m_upload->Map(0, &readRange, (void**)&pdata))
		{
			delete buffer;
			return nullptr;
		}
		else
		{
			memcpy(pdata, indices, numIndices * sizeof(UINT));

			buffer->m_upload->Unmap(0, nullptr);
		}

		// create a GPU memory buffer
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		if (hr = context->m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr, IID_PPV_ARGS(&buffer->m_buffer)))
		{
			delete buffer;
			return nullptr;
		}

		// copy from upload to new buffer
		context->m_commandList->CopyBufferRegion(buffer->m_buffer, 0, buffer->m_upload, 0, bufferSize);

		D3D12_RESOURCE_BARRIER barrier[1];
		barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier[0].Transition.pResource = buffer->m_buffer;
		barrier[0].Transition.Subresource = 0u;
		barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
		context->m_commandList->ResourceBarrier(1, barrier);

		bufferSize = (UINT)(numIndices) * sizeof(UINT);
		buffer->m_view.BufferLocation = buffer->m_buffer->GetGPUVirtualAddress();
		buffer->m_view.SizeInBytes = bufferSize;
		buffer->m_view.Format = DXGI_FORMAT_R32_UINT;
	}

	return cast_from_MeshIndexBufferD3D12(buffer);
}

void MeshIndexBufferReleaseD3D12(MeshIndexBuffer* buffer)
{
	if (buffer == nullptr) return;

	delete cast_to_MeshIndexBufferD3D12(buffer);
}

MeshVertexBuffer* MeshVertexBufferCreateD3D12(MeshContext* contextIn, MeshVertex* vertices, MeshUint numVertices)
{
	auto context = cast_to_MeshContextD3D12(contextIn);

	MeshVertexBufferD3D12* buffer = new MeshVertexBufferD3D12;

	buffer->m_numElements = numVertices;
	// create a vertex buffer
	{
		HRESULT hr = S_OK;

		UINT bufferSize = (UINT)(numVertices * sizeof(MeshVertex));

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 0u;
		heapProps.VisibleNodeMask = 0u;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0u;
		desc.Width = bufferSize;
		desc.Height = 1u;
		desc.DepthOrArraySize = 1u;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1u;
		desc.SampleDesc.Quality = 0u;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		if (hr = context->m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&buffer->m_upload)))
		{
			delete buffer;
			return nullptr;
		}

		UINT8* pdata;
		D3D12_RANGE readRange = {};
		if (hr = buffer->m_upload->Map(0, &readRange, (void**)&pdata))
		{
			delete buffer;
			return nullptr;
		}
		else
		{
			memcpy(pdata, vertices, numVertices * sizeof(MeshVertex));

			buffer->m_upload->Unmap(0, nullptr);
		}

		// create a GPU memory buffer
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		if (hr = context->m_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr, IID_PPV_ARGS(&buffer->m_buffer)))
		{
			delete buffer;
			return nullptr;
		}

		// copy from upload to new buffer
		context->m_commandList->CopyBufferRegion(buffer->m_buffer, 0, buffer->m_upload, 0, bufferSize);

		D3D12_RESOURCE_BARRIER barrier[1];
		barrier[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier[0].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier[0].Transition.pResource = buffer->m_buffer;
		barrier[0].Transition.Subresource = 0u;
		barrier[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier[0].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		context->m_commandList->ResourceBarrier(1, barrier);

		bufferSize = (UINT)(numVertices * sizeof(MeshVertex));
		buffer->m_view.BufferLocation = buffer->m_buffer->GetGPUVirtualAddress();
		buffer->m_view.StrideInBytes = 6 * sizeof(float);
		buffer->m_view.SizeInBytes = bufferSize;
	}

	return cast_from_MeshVertexBufferD3D12(buffer);
}

void MeshVertexBufferReleaseD3D12(MeshVertexBuffer* buffer)
{
	if (buffer == nullptr) return;

	delete cast_to_MeshVertexBufferD3D12(buffer);
}

void MeshContextDrawD3D12(MeshContext* contextIn, const MeshContextDrawParams* params)
{
	auto context = cast_to_MeshContextD3D12(contextIn);

	using namespace DirectX;

	XMMATRIX matrix = XMMatrixTranspose(XMMatrixMultiply(XMMatrixMultiply(
		params->params->model, 
		params->params->view),
		params->params->projection
		));

	UINT8* constantBufferData = context->m_constantHeap->allocate(256);

	if (constantBufferData == nullptr)
	{
		return;
	}

	XMStoreFloat4x4((XMFLOAT4X4*)(constantBufferData), matrix);

	ID3D12GraphicsCommandList* commandList = context->m_commandList;

	commandList->SetGraphicsRootSignature(context->m_rootSignature);

	float depthSign = DirectX::XMVectorGetW(params->params->projection.r[2]);
	if (depthSign < 0.f)
	{
		commandList->SetPipelineState(context->m_pipelineStateRH);
	}
	else
	{
		commandList->SetPipelineState(context->m_pipelineStateLH);
	}

	D3D12_GPU_VIRTUAL_ADDRESS cbvHandle = context->m_constantHeap->getVirtualAddress(constantBufferData);
	commandList->SetGraphicsRootConstantBufferView(0, cbvHandle);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &cast_to_MeshVertexBufferD3D12(params->vertexBuffer)->m_view);
	commandList->IASetIndexBuffer(&cast_to_MeshIndexBufferD3D12(params->indexBuffer)->m_view);

	commandList->DrawIndexedInstanced((UINT)cast_to_MeshIndexBufferD3D12(params->indexBuffer)->m_numElements, 1, 0, 0, 0);
}