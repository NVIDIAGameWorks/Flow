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

#include <d3d12.h>

#include "computeContextD3D12.h"

#include <vector>

// ************************** Compute Context Implementation ******************

typedef unsigned int ComputeUint;
typedef unsigned long long ComputeUint64;

namespace
{
	template<class T>
	struct VectorCached
	{
		std::vector<T> m_data;

		ComputeUint m_size = 0u;

		ComputeUint allocateBack()
		{
			m_size++;
			m_data.resize(m_size);
			return m_size - 1;
		}

		T& operator[](unsigned int idx)
		{
			return m_data[idx];
		}
	};

	template <class BufferData>
	struct VersionedBuffer
	{
		BufferData* map(ComputeUint64 lastFenceCompleted, ComputeUint64 nextFenceValue);
		void unmap(ComputeUint64 lastFenceCompleted, ComputeUint64 nextFenceValue);

		struct Buffer
		{
			ComputeUint64 releaseFenceValue = ~0llu;
			BufferData bufferData;

			Buffer() : bufferData() {}
		};

		VectorCached<Buffer> m_buffers;
		ComputeUint m_frontIdx = 0u;
		ComputeUint m_mappedIdx = 0u;

		BufferData* front();

		ComputeUint64 currentFrame = 0u;

		VersionedBuffer();
		~VersionedBuffer();
	};

	template <class BufferData>
	VersionedBuffer<BufferData>::VersionedBuffer()
	{
	}

	template <class BufferData>
	VersionedBuffer<BufferData>::~VersionedBuffer()
	{
	}

	template <class BufferData>
	BufferData* VersionedBuffer<BufferData>::front()
	{
		return &m_buffers[m_frontIdx].bufferData;
	}

	template <class BufferData>
	BufferData* VersionedBuffer<BufferData>::map(ComputeUint64 lastFenceCompleted, ComputeUint64 nextFenceValue)
	{
		// search for available buffer
		ComputeUint numBuffers = m_buffers.m_size;
		ComputeUint index = m_frontIdx + 1;
		for (; index < numBuffers; index++)
		{
			if (m_buffers[index].releaseFenceValue <= lastFenceCompleted)
			{
				break;
			}
		}
		if (index == numBuffers)
		{
			for (index = 0u; index < m_frontIdx; index++)
			{
				if (m_buffers[index].releaseFenceValue <= lastFenceCompleted)
				{
					break;
				}
			}
		}
		if (index == m_frontIdx || numBuffers == 0u)
		{
			index = m_buffers.allocateBack();
		}
		m_mappedIdx = index;
		return &m_buffers[index].bufferData;
	}

	template <class BufferData>
	void VersionedBuffer<BufferData>::unmap(ComputeUint64 lastFenceCompleted, ComputeUint64 nextFenceValue)
	{
		// mark front obsolete
		if (m_frontIdx != m_mappedIdx) m_buffers[m_frontIdx].releaseFenceValue = nextFenceValue;
		// set new front
		m_frontIdx = m_mappedIdx;
	}
}

namespace
{
	struct HeapPropsUpload : public D3D12_HEAP_PROPERTIES
	{
		HeapPropsUpload()
		{
			Type = D3D12_HEAP_TYPE_UPLOAD;
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
	struct DescriptorRange : public D3D12_DESCRIPTOR_RANGE
	{
		void init(D3D12_DESCRIPTOR_RANGE_TYPE type, UINT baseRegister, UINT numDescriptors)
		{
			RangeType = type;
			NumDescriptors = numDescriptors;
			BaseShaderRegister = baseRegister;
			RegisterSpace = 0u;
			OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}
	};
	struct RootParameter : public D3D12_ROOT_PARAMETER
	{
		void initRootConstant(D3D12_SHADER_VISIBILITY visibility, UINT shaderReg)
		{
			ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			Descriptor.ShaderRegister = shaderReg;
			Descriptor.RegisterSpace = 0u;
			ShaderVisibility = visibility;
		}
		void initTable(D3D12_DESCRIPTOR_RANGE* range, D3D12_SHADER_VISIBILITY visibility)
		{
			ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			DescriptorTable.NumDescriptorRanges = 1;
			DescriptorTable.pDescriptorRanges = range;
			ShaderVisibility = visibility;
		}
	};
	struct StaticSamplerDesc : public D3D12_STATIC_SAMPLER_DESC
	{
		void init(UINT shaderRegister, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE addressMode)
		{
			Filter = filter;
			AddressU = addressMode;
			AddressV = addressMode;
			AddressW = addressMode;
			MipLODBias = 0;
			MaxAnisotropy = 0;
			ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
			MinLOD = 0.f;
			MaxLOD = D3D12_FLOAT32_MAX;
			ShaderRegister = shaderRegister;
			RegisterSpace = 0;
			ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		}
	};
	struct ComputeRootSignature
	{
		static const int numRanges = 2;
		static const int numParams = 3; //4;
		static const int numSamplers = 6;

		DescriptorRange ranges[numRanges];
		RootParameter params[numParams];
		StaticSamplerDesc samplers[numSamplers];
		D3D12_ROOT_SIGNATURE_DESC desc;

		ComputeRootSignature()
		{
			ranges[0].init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0u, ComputeDispatchMaxResources);
			ranges[1].init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0u, ComputeDispatchMaxResourcesRW);

			params[0].initRootConstant(D3D12_SHADER_VISIBILITY_ALL, 0u);
			params[1].initTable(&ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			params[2].initTable(&ranges[1], D3D12_SHADER_VISIBILITY_ALL);
			//params[3].initRootConstant(D3D12_SHADER_VISIBILITY_ALL, 1u);

			samplers[0].init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER);
			samplers[1].init(1, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER);
			samplers[2].init(2, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
			samplers[3].init(3, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
			samplers[4].init(4, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
			samplers[5].init(5, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

			desc.NumParameters = numParams;
			desc.pParameters = params;
			desc.NumStaticSamplers = numSamplers;
			desc.pStaticSamplers = samplers;
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
		}
	};
};

template <class T>
void inline COMRelease(T& t)
{
	if (t) t->Release();
	t = nullptr;
}

struct ComputeContextUserData
{
	virtual void Release() = 0;
};

struct ComputeContextD3D12
{
	ComputeContextDescD3D12 m_desc = {};

	ID3D12RootSignature* m_rootSignatureCompute = nullptr;
	ID3D12DescriptorHeap* m_nullHeap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_nullUAV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_nullSRV;

	ComputeContextUserData* m_computeUserdata = nullptr;

	ComputeContextD3D12(const ComputeContextDesc* descIn)
	{
		auto desc = cast_to_ComputeContextDescD3D12(descIn);

		m_desc = *desc;

		auto createRootSignature = [&](D3D12_ROOT_SIGNATURE_DESC* desc, ID3D12RootSignature** root)
		{
			ID3DBlob* signature = nullptr;
			ID3DBlob* error = nullptr;
			HRESULT hr = S_OK;
			if (hr = D3D12SerializeRootSignature(desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))
			{
				return hr;
			}
			if (hr = m_desc.device->CreateRootSignature(0u, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(root)))
			{
				return hr;
			}
			COMRelease(signature);
			COMRelease(error);
			return HRESULT(0);
		};
		{
			ComputeRootSignature desc;
			createRootSignature(&desc.desc, &m_rootSignatureCompute);
		}

		// create CPU descriptor heap
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.NumDescriptors = 2u;
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			m_desc.device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_nullHeap));
		}

		// create null UAV
		{
			D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
			uavDesc.Format = DXGI_FORMAT_R32_UINT;
			uavDesc.Buffer.FirstElement = 0u;
			uavDesc.Buffer.NumElements = 256u;

			m_nullUAV = m_nullHeap->GetCPUDescriptorHandleForHeapStart();
			m_desc.device->CreateUnorderedAccessView(nullptr, nullptr, &uavDesc, m_nullUAV);
		}

		// create null SRV
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

			m_nullSRV = m_nullHeap->GetCPUDescriptorHandleForHeapStart();
			m_nullSRV.ptr += m_desc.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			m_desc.device->CreateShaderResourceView(nullptr, &srvDesc, m_nullSRV);
		}
	}
	~ComputeContextD3D12()
	{
		COMRelease(m_rootSignatureCompute);
		COMRelease(m_nullHeap);
		COMRelease(m_computeUserdata);
	}
};

inline ComputeContextD3D12* cast_to_ComputeContextD3D12(ComputeContext* ctx)
{
	return (ComputeContextD3D12*)(ctx);
}

inline ComputeContext* cast_from_ComputeContextD3D12(ComputeContextD3D12* ctx)
{
	return (ComputeContext*)(ctx);
}

struct ComputeShaderD3D12
{
	ID3D12PipelineState*  m_shader = nullptr;
	ComputeShaderD3D12(ID3D12PipelineState* shader)
	{
		m_shader = shader;
	}
	~ComputeShaderD3D12()
	{
		COMRelease(m_shader);
	}
};

inline ComputeShaderD3D12* cast_to_ComputeShaderD3D12(ComputeShader* ctx)
{
	return (ComputeShaderD3D12*)(ctx);
}

inline ComputeShader* cast_from_ComputeShaderD3D12(ComputeShaderD3D12* ctx)
{
	return (ComputeShader*)(ctx);
}

struct ComputeConstantBufferD3D12
{
	ComputeConstantBufferDesc m_desc = {};

	struct BufferData
	{
		ID3D12Resource* m_buffer = nullptr;
		void* m_mappedData = nullptr;

		BufferData() {}
	};
	VersionedBuffer<BufferData> m_buffers;

	ComputeConstantBufferD3D12(ComputeContext* context, const ComputeConstantBufferDesc* desc);
	~ComputeConstantBufferD3D12();
};

inline ComputeConstantBufferD3D12* cast_to_ComputeConstantBufferD3D12(ComputeConstantBuffer* ctx)
{
	return (ComputeConstantBufferD3D12*)(ctx);
}

inline ComputeConstantBuffer* cast_from_ComputeConstantBufferD3D12(ComputeConstantBufferD3D12* ctx)
{
	return (ComputeConstantBuffer*)(ctx);
}

ComputeConstantBufferD3D12::ComputeConstantBufferD3D12(ComputeContext* context, const ComputeConstantBufferDesc* desc)
{
	m_desc = *desc;

	// map and unmap to trigger initial allocation
	ComputeConstantBufferMapD3D12(context, cast_from_ComputeConstantBufferD3D12(this));
	ComputeConstantBufferUnmapD3D12(context, cast_from_ComputeConstantBufferD3D12(this));
}

ComputeConstantBufferD3D12::~ComputeConstantBufferD3D12()
{
	for (ComputeUint i = 0; i < m_buffers.m_buffers.m_size; i++)
	{
		COMRelease(m_buffers.m_buffers[i].bufferData.m_buffer);
	}
}

struct ComputeResourceD3D12
{
protected:
	D3D12_CPU_DESCRIPTOR_HANDLE m_srv;
	ID3D12Resource* m_resource;
	D3D12_RESOURCE_STATES* m_currentState;
public:
	void update(const ComputeResourceDesc* descIn)
	{
		auto desc = cast_to_ComputeResourceDescD3D12(descIn);

		m_srv = desc->srv;
		m_resource = desc->resource;
		m_currentState = desc->currentState;
	}

	ComputeResourceD3D12(const ComputeResourceDesc* desc)
	{
		update(desc);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE SRV()
	{
		return m_srv;
	}

	ID3D12Resource* resource()
	{
		return m_resource;
	}

	D3D12_RESOURCE_STATES* currentState()
	{
		return m_currentState;
	}
};

inline ComputeResourceD3D12* cast_to_ComputeResourceD3D12(ComputeResource* ctx)
{
	return (ComputeResourceD3D12*)(ctx);
}

inline ComputeResource* cast_from_ComputeResourceD3D12(ComputeResourceD3D12* ctx)
{
	return (ComputeResource*)(ctx);
}

struct ComputeResourceRWD3D12 : public ComputeResourceD3D12
{
protected:
	D3D12_CPU_DESCRIPTOR_HANDLE m_uav;
public:
	static const ComputeResourceRWDescD3D12* cast(const ComputeResourceRWDesc* descRW)
	{
		return cast_to_ComputeResourceRWDescD3D12(descRW);
	}

	void update(const ComputeResourceRWDesc* descRW)
	{
		m_uav = cast(descRW)->uav;
		ComputeResourceD3D12::update(cast_from_ComputeResourceDescD3D12(&cast(descRW)->resource));
	}

	ComputeResourceRWD3D12(const ComputeResourceRWDesc* descRW) :
		ComputeResourceD3D12(cast_from_ComputeResourceDescD3D12(&cast(descRW)->resource))
	{
		m_uav = cast(descRW)->uav;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE UAV()
	{
		return m_uav;
	}
};

inline ComputeResourceRWD3D12* cast_to_ComputeResourceRWD3D12(ComputeResourceRW* ctx)
{
	return (ComputeResourceRWD3D12*)(ctx);
}

inline ComputeResourceRW* cast_from_ComputeResourceRWD3D12(ComputeResourceRWD3D12* ctx)
{
	return (ComputeResourceRW*)(ctx);
}


// ************* API functions ****************

ComputeContext* ComputeContextCreateD3D12(ComputeContextDesc* desc)
{
	return cast_from_ComputeContextD3D12(new ComputeContextD3D12(desc));
}

void ComputeContextUpdateD3D12(ComputeContext* contextIn, ComputeContextDesc* descIn)
{
	auto context = cast_to_ComputeContextD3D12(contextIn);
	auto desc = cast_to_ComputeContextDescD3D12(descIn);

	context->m_desc = *desc;
}

void ComputeContextReleaseD3D12(ComputeContext* context)
{
	delete cast_to_ComputeContextD3D12(context);
}

ComputeShader* ComputeShaderCreateD3D12(ComputeContext* contextIn, const ComputeShaderDesc* desc)
{
	auto context = cast_to_ComputeContextD3D12(contextIn);

	ID3D12PipelineState* computeShader = nullptr;

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = context->m_rootSignatureCompute;
	psoDesc.CS.pShaderBytecode = desc->cs;
	psoDesc.CS.BytecodeLength = desc->cs_length;

	context->m_desc.device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&computeShader));

	return cast_from_ComputeShaderD3D12(new ComputeShaderD3D12(computeShader));
}

void ComputeShaderReleaseD3D12(ComputeShader* shader)
{
	delete cast_to_ComputeShaderD3D12(shader);
}

ComputeConstantBuffer* ComputeConstantBufferCreateD3D12(ComputeContext* context, const ComputeConstantBufferDesc* desc)
{
	return cast_from_ComputeConstantBufferD3D12(new ComputeConstantBufferD3D12(context, desc));
}

void ComputeConstantBufferReleaseD3D12(ComputeConstantBuffer* constantBuffer)
{
	delete cast_to_ComputeConstantBufferD3D12(constantBuffer);
}

void* ComputeConstantBufferMapD3D12(ComputeContext* contextIn, ComputeConstantBuffer* constantBufferIn)
{
	auto context = cast_to_ComputeContextD3D12(contextIn);
	auto constantBuffer = cast_to_ComputeConstantBufferD3D12(constantBufferIn);

	auto bufferData = constantBuffer->m_buffers.map(context->m_desc.lastFenceCompleted, context->m_desc.nextFenceValue);

	if (bufferData->m_buffer == nullptr)
	{
		HeapPropsUpload heapProps;
		ResourceDescBuffer resDesc(constantBuffer->m_desc.sizeInBytes);

		context->m_desc.device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
			&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&bufferData->m_buffer));

		UINT8* pdata = nullptr;
		D3D12_RANGE readRange = {};
		bufferData->m_buffer->Map(0, &readRange, (void**)&pdata);

		bufferData->m_mappedData = pdata;
	}
	return bufferData->m_mappedData;
}

void ComputeConstantBufferUnmapD3D12(ComputeContext* contextIn, ComputeConstantBuffer* constantBufferIn)
{
	auto context = cast_to_ComputeContextD3D12(contextIn);
	auto constantBuffer = cast_to_ComputeConstantBufferD3D12(constantBufferIn);

	constantBuffer->m_buffers.unmap(context->m_desc.lastFenceCompleted, context->m_desc.nextFenceValue);
}

ComputeResource* ComputeResourceCreateD3D12(ComputeContext* context, const ComputeResourceDesc* desc)
{
	return cast_from_ComputeResourceD3D12(new ComputeResourceD3D12(desc));
}

void ComputeResourceUpdateD3D12(ComputeContext* context, ComputeResource* resourceIn, const ComputeResourceDesc* desc)
{
	auto resource = cast_to_ComputeResourceD3D12(resourceIn);
	resource->update(desc);
}

void ComputeResourceReleaseD3D12(ComputeResource* resource)
{
	delete cast_to_ComputeResourceD3D12(resource);
}

ComputeResourceRW* ComputeResourceRWCreateD3D12(ComputeContext* context, const ComputeResourceRWDesc* desc)
{
	return cast_from_ComputeResourceRWD3D12(new ComputeResourceRWD3D12(desc));
}

void ComputeResourceRWUpdateD3D12(ComputeContext* context, ComputeResourceRW* resourceRWIn, const ComputeResourceRWDesc* desc)
{
	auto resourceRW = cast_to_ComputeResourceRWD3D12(resourceRWIn);
	resourceRW->update(desc);
}

void ComputeResourceRWReleaseD3D12(ComputeResourceRW* resourceRW)
{
	delete cast_to_ComputeResourceRWD3D12(resourceRW);
}

ComputeResource* ComputeResourceRWGetResourceD3D12(ComputeResourceRW* resourceRWIn)
{
	auto resourceRW = cast_to_ComputeResourceRWD3D12(resourceRWIn);
	return cast_from_ComputeResourceD3D12(static_cast<ComputeResourceD3D12*>(resourceRW));
}

void ComputeContextDispatchD3D12(ComputeContext* contextIn, const ComputeDispatchParams* params)
{
	auto context = cast_to_ComputeContextD3D12(contextIn);

	auto& commandList = context->m_desc.commandList;

	commandList->SetComputeRootSignature(context->m_rootSignatureCompute);
	if (params->shader) commandList->SetPipelineState(cast_to_ComputeShaderD3D12(params->shader)->m_shader);

	auto handles = context->m_desc.dynamicHeapCbvSrvUav.reserveDescriptors(context->m_desc.dynamicHeapCbvSrvUav.userdata,
		ComputeDispatchMaxResources + ComputeDispatchMaxResourcesRW, 
		context->m_desc.lastFenceCompleted, context->m_desc.nextFenceValue
	);

	commandList->SetDescriptorHeaps(1u, &handles.heap);

	for (ComputeUint i = 0u; i < ComputeDispatchMaxResources; i++)
	{
		auto r = params->resources[i];
		D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = r ? cast_to_ComputeResourceD3D12(r)->SRV() : context->m_nullSRV;
		context->m_desc.device->CopyDescriptorsSimple(1u, handles.cpuHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		handles.cpuHandle.ptr += handles.descriptorSize;
	}
	for (ComputeUint i = 0u; i < ComputeDispatchMaxResourcesRW; i++)
	{
		auto rw = params->resourcesRW[i];
		D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = rw ? cast_to_ComputeResourceRWD3D12(rw)->UAV() : context->m_nullUAV;
		context->m_desc.device->CopyDescriptorsSimple(1u, handles.cpuHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		handles.cpuHandle.ptr += handles.descriptorSize;
	}

	commandList->SetComputeRootDescriptorTable(1u, handles.gpuHandle);
	handles.gpuHandle.ptr += handles.descriptorSize * ComputeDispatchMaxResources;
	commandList->SetComputeRootDescriptorTable(2u, handles.gpuHandle);

	if (params->constantBuffer)
	{
		auto cbv = cast_to_ComputeConstantBufferD3D12(params->constantBuffer)->m_buffers.front()->m_buffer;
		commandList->SetComputeRootConstantBufferView(0u, cbv->GetGPUVirtualAddress());
	}

	ComputeUint barrierIdx = 0u;
	D3D12_RESOURCE_BARRIER barrier[ComputeDispatchMaxResources + ComputeDispatchMaxResourcesRW];
	for (ComputeUint i = 0u; i < ComputeDispatchMaxResources; i++)
	{
		auto r = params->resources[i];
		if (r)
		{
			if (!((*cast_to_ComputeResourceD3D12(r)->currentState()) & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
			{
				D3D12_RESOURCE_BARRIER& bar = barrier[barrierIdx++];
				bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				bar.Transition.pResource = cast_to_ComputeResourceD3D12(r)->resource();
				bar.Transition.Subresource = 0u;
				bar.Transition.StateBefore = *cast_to_ComputeResourceD3D12(r)->currentState();
				bar.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

				*cast_to_ComputeResourceD3D12(r)->currentState() = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}
		}
	}
	for (ComputeUint i = 0u; i < ComputeDispatchMaxResourcesRW; i++)
	{
		auto rw = params->resourcesRW[i];
		if (rw)
		{
			if ((*cast_to_ComputeResourceRWD3D12(rw)->currentState()) == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			{
				D3D12_RESOURCE_BARRIER& bar = barrier[barrierIdx++];
				bar.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				bar.UAV.pResource = cast_to_ComputeResourceRWD3D12(rw)->resource();
			}
			else
			{
				D3D12_RESOURCE_BARRIER& bar = barrier[barrierIdx++];
				bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				bar.Transition.pResource = cast_to_ComputeResourceRWD3D12(rw)->resource();
				bar.Transition.Subresource = 0u;
				bar.Transition.StateBefore = *cast_to_ComputeResourceRWD3D12(rw)->currentState();
				bar.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

				*cast_to_ComputeResourceRWD3D12(rw)->currentState() = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
			}
		}
	}

	commandList->ResourceBarrier(barrierIdx, barrier);

	commandList->Dispatch(params->gridDim[0], params->gridDim[1], params->gridDim[2]);
}

// ******************************* NvFlow Interoperation ****************************************

#include "NvFlow.h"
#include "NvFlowContextD3D12.h"

struct ComputeContextUserDataNvFlow : public ComputeContextUserData
{
	NvFlowDynamicDescriptorHeapD3D12 flowHeap = {};

	void Release() override
	{
		delete this;
	}
};

ComputeDescriptorReserveHandleD3D12 ComputeReserveDescriptorsD3D12(void* userdata, UINT numDescriptors, UINT64 lastFenceCompleted, UINT64 nextFenceValue)
{
	auto data = static_cast<ComputeContextUserDataNvFlow*>(userdata);
	auto srcHandle = data->flowHeap.reserveDescriptors(data->flowHeap.userdata, numDescriptors, lastFenceCompleted, nextFenceValue);
	ComputeDescriptorReserveHandleD3D12 handle = {};
	handle.heap = srcHandle.heap;
	handle.descriptorSize = srcHandle.descriptorSize;
	handle.cpuHandle = srcHandle.cpuHandle;
	handle.gpuHandle = srcHandle.gpuHandle;
	return handle;
}

ComputeContext* ComputeContextNvFlowContextCreateD3D12(NvFlowContext* flowContext)
{
	ComputeContextDescD3D12 desc = {};
	NvFlowContextDescD3D12 srcDesc = {};
	NvFlowUpdateContextDescD3D12(flowContext, &srcDesc);

	desc.device = srcDesc.device;
	desc.commandQueue = srcDesc.commandQueue;
	desc.commandQueueFence = srcDesc.commandQueueFence;
	desc.commandList = srcDesc.commandList;
	desc.lastFenceCompleted = srcDesc.lastFenceCompleted;
	desc.nextFenceValue = srcDesc.nextFenceValue;

	auto computeUserdata = new ComputeContextUserDataNvFlow;
	computeUserdata->flowHeap.userdata = srcDesc.dynamicHeapCbvSrvUav.userdata;
	computeUserdata->flowHeap.reserveDescriptors = srcDesc.dynamicHeapCbvSrvUav.reserveDescriptors;

	desc.dynamicHeapCbvSrvUav.userdata = computeUserdata;
	desc.dynamicHeapCbvSrvUav.reserveDescriptors = ComputeReserveDescriptorsD3D12;

	auto computeContext = cast_to_ComputeContextD3D12(ComputeContextCreateD3D12(cast_from_ComputeContextDescD3D12(&desc)));

	computeContext->m_computeUserdata = computeUserdata;

	return cast_from_ComputeContextD3D12(computeContext);
}

void ComputeContextNvFlowContextUpdateD3D12(ComputeContext* computeContextIn, NvFlowContext* flowContext)
{
	auto computeContext = cast_to_ComputeContextD3D12(computeContextIn);

	ComputeContextDescD3D12 desc = {};
	NvFlowContextDescD3D12 srcDesc = {};
	NvFlowUpdateContextDescD3D12(flowContext, &srcDesc);

	desc.device = srcDesc.device;
	desc.commandQueue = srcDesc.commandQueue;
	desc.commandQueueFence = srcDesc.commandQueueFence;
	desc.commandList = srcDesc.commandList;
	desc.lastFenceCompleted = srcDesc.lastFenceCompleted;
	desc.nextFenceValue = srcDesc.nextFenceValue;

	auto computeUserdata = static_cast<ComputeContextUserDataNvFlow*>(computeContext->m_computeUserdata);
	computeUserdata->flowHeap.userdata = srcDesc.dynamicHeapCbvSrvUav.userdata;
	computeUserdata->flowHeap.reserveDescriptors = srcDesc.dynamicHeapCbvSrvUav.reserveDescriptors;

	desc.dynamicHeapCbvSrvUav.userdata = computeUserdata;
	desc.dynamicHeapCbvSrvUav.reserveDescriptors = ComputeReserveDescriptorsD3D12;

	ComputeContextUpdateD3D12(cast_from_ComputeContextD3D12(computeContext), cast_from_ComputeContextDescD3D12(&desc));
}

inline void updateComputeResourceDesc(NvFlowResourceViewDescD3D12* flowViewDesc, ComputeResourceDescD3D12* desc)
{
	desc->srv = flowViewDesc->srvHandle;
	desc->resource = flowViewDesc->resource;
	desc->currentState = flowViewDesc->currentState;
}

ComputeResource* ComputeResourceNvFlowCreateD3D12(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	NvFlowResourceViewDescD3D12 flowViewDesc = {};
	NvFlowUpdateResourceViewDescD3D12(flowContext, flowResource, &flowViewDesc);
	ComputeResourceDescD3D12 desc = {};
	updateComputeResourceDesc(&flowViewDesc, &desc);
	return ComputeResourceCreateD3D12(context, cast_from_ComputeResourceDescD3D12(&desc));
}

void ComputeResourceNvFlowUpdateD3D12(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	NvFlowResourceViewDescD3D12 flowViewDesc = {};
	NvFlowUpdateResourceViewDescD3D12(flowContext, flowResource, &flowViewDesc);
	ComputeResourceDescD3D12 desc = {};
	updateComputeResourceDesc(&flowViewDesc, &desc);
	ComputeResourceUpdateD3D12(context, resource, cast_from_ComputeResourceDescD3D12(&desc));
}

inline void updateComputeResourceRWDesc(NvFlowResourceRWViewDescD3D12* flowViewDesc, ComputeResourceRWDescD3D12* desc)
{
	updateComputeResourceDesc(&flowViewDesc->resourceView, &desc->resource);
	desc->uav = flowViewDesc->uavHandle;
}

ComputeResourceRW* ComputeResourceRWNvFlowCreateD3D12(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	NvFlowResourceRWViewDescD3D12 flowViewDesc = {};
	NvFlowUpdateResourceRWViewDescD3D12(flowContext, flowResourceRW, &flowViewDesc);
	ComputeResourceRWDescD3D12 desc = {};
	updateComputeResourceRWDesc(&flowViewDesc, &desc);
	return ComputeResourceRWCreateD3D12(context, cast_from_ComputeResourceRWDescD3D12(&desc));
}

void ComputeResourceRWNvFlowUpdateD3D12(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	NvFlowResourceRWViewDescD3D12 flowViewDesc = {};
	NvFlowUpdateResourceRWViewDescD3D12(flowContext, flowResourceRW, &flowViewDesc);
	ComputeResourceRWDescD3D12 desc = {};
	updateComputeResourceRWDesc(&flowViewDesc, &desc);
	ComputeResourceRWUpdateD3D12(context, resourceRW, cast_from_ComputeResourceRWDescD3D12(&desc));
}