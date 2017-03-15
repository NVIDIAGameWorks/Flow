/*
* Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

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

struct ComputeContext
{
	ComputeContextDesc m_desc = {};

	ID3D12RootSignature* m_rootSignatureCompute = nullptr;
	ID3D12DescriptorHeap* m_nullHeap = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE m_nullUAV;
	D3D12_CPU_DESCRIPTOR_HANDLE m_nullSRV;

	ComputeContextUserData* m_computeUserdata = nullptr;

	ComputeContext(const ComputeContextDesc* desc)
	{
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
	~ComputeContext()
	{
		COMRelease(m_rootSignatureCompute);
		COMRelease(m_nullHeap);
		COMRelease(m_computeUserdata);
	}
};

struct ComputeShader
{
	ID3D12PipelineState*  m_shader = nullptr;
	ComputeShader(ID3D12PipelineState* shader)
	{
		m_shader = shader;
	}
	~ComputeShader()
	{
		COMRelease(m_shader);
	}
};

struct ComputeConstantBuffer
{
	ComputeConstantBufferDesc m_desc = {};

	struct BufferData
	{
		ID3D12Resource* m_buffer = nullptr;
		void* m_mappedData = nullptr;

		BufferData() {}
	};
	VersionedBuffer<BufferData> m_buffers;

	ComputeConstantBuffer(ComputeContext* context, const ComputeConstantBufferDesc* desc)
	{
		m_desc = *desc;

		// map and unmap to trigger initial allocation
		ComputeConstantBufferMap(context, this);
		ComputeConstantBufferUnmap(context, this);
	}
	~ComputeConstantBuffer()
	{
		for (ComputeUint i = 0; i < m_buffers.m_buffers.m_size; i++)
		{
			COMRelease(m_buffers.m_buffers[i].bufferData.m_buffer);
		}
	}
};

struct ComputeResource
{
protected:
	D3D12_CPU_DESCRIPTOR_HANDLE m_srv;
	ID3D12Resource* m_resource;
	D3D12_RESOURCE_STATES* m_currentState;
public:
	void update(const ComputeResourceDesc* desc)
	{
		m_srv = desc->srv;
		m_resource = desc->resource;
		m_currentState = desc->currentState;
	}

	ComputeResource(const ComputeResourceDesc* desc)
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

struct ComputeResourceRW : public ComputeResource
{
protected:
	D3D12_CPU_DESCRIPTOR_HANDLE m_uav;
public:
	void update(const ComputeResourceRWDesc* descRW)
	{
		m_uav = descRW->uav;
		ComputeResource::update(&descRW->resource);
	}

	ComputeResourceRW(const ComputeResourceRWDesc* descRW) :
		ComputeResource(&descRW->resource)
	{
		m_uav = descRW->uav;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE UAV()
	{
		return m_uav;
	}
};

// ************* API functions ****************

ComputeContext* ComputeContextCreate(ComputeContextDesc* desc)
{
	return new ComputeContext(desc);
}

void ComputeContextUpdate(ComputeContext* context, ComputeContextDesc* desc)
{
	context->m_desc = *desc;
}

void ComputeContextRelease(ComputeContext* context)
{
	delete context;
}

ComputeShader* ComputeShaderCreate(ComputeContext* context, const ComputeShaderDesc* desc)
{
	ID3D12PipelineState* computeShader = nullptr;

	D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = context->m_rootSignatureCompute;
	psoDesc.CS.pShaderBytecode = desc->cs;
	psoDesc.CS.BytecodeLength = desc->cs_length;

	context->m_desc.device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&computeShader));

	return new ComputeShader(computeShader);
}

void ComputeShaderRelease(ComputeShader* shader)
{
	delete shader;
}

ComputeConstantBuffer* ComputeConstantBufferCreate(ComputeContext* context, const ComputeConstantBufferDesc* desc)
{
	return new ComputeConstantBuffer(context, desc);
}

void ComputeConstantBufferRelease(ComputeConstantBuffer* constantBuffer)
{
	delete constantBuffer;
}

void* ComputeConstantBufferMap(ComputeContext* context, ComputeConstantBuffer* constantBuffer)
{
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

void ComputeConstantBufferUnmap(ComputeContext* context, ComputeConstantBuffer* constantBuffer)
{
	constantBuffer->m_buffers.unmap(context->m_desc.lastFenceCompleted, context->m_desc.nextFenceValue);
}

ComputeResource* ComputeResourceCreate(ComputeContext* context, const ComputeResourceDesc* desc)
{
	return new ComputeResource(desc);
}

void ComputeResourceUpdate(ComputeContext* context, ComputeResource* resource, const ComputeResourceDesc* desc)
{
	resource->update(desc);
}

void ComputeResourceRelease(ComputeResource* resource)
{
	delete resource;
}

ComputeResourceRW* ComputeResourceRWCreate(ComputeContext* context, const ComputeResourceRWDesc* desc)
{
	return new ComputeResourceRW(desc);
}

void ComputeResourceRWUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, const ComputeResourceRWDesc* desc)
{
	resourceRW->update(desc);
}

void ComputeResourceRWRelease(ComputeResourceRW* resourceRW)
{
	delete resourceRW;
}

ComputeResource* ComputeResourceRWGetResource(ComputeResourceRW* resourceRW)
{
	return static_cast<ComputeResource*>(resourceRW);
}

void ComputeContextDispatch(ComputeContext* context, const ComputeDispatchParams* params)
{
	auto& commandList = context->m_desc.commandList;

	commandList->SetComputeRootSignature(context->m_rootSignatureCompute);
	if (params->shader) commandList->SetPipelineState(params->shader->m_shader);

	auto handles = context->m_desc.dynamicHeapCbvSrvUav.reserveDescriptors(context->m_desc.dynamicHeapCbvSrvUav.userdata,
		ComputeDispatchMaxResources + ComputeDispatchMaxResourcesRW, 
		context->m_desc.lastFenceCompleted, context->m_desc.nextFenceValue
	);

	commandList->SetDescriptorHeaps(1u, &handles.heap);

	for (ComputeUint i = 0u; i < ComputeDispatchMaxResources; i++)
	{
		auto r = params->resources[i];
		D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = r ? r->SRV() : context->m_nullSRV;
		context->m_desc.device->CopyDescriptorsSimple(1u, handles.cpuHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		handles.cpuHandle.ptr += handles.descriptorSize;
	}
	for (ComputeUint i = 0u; i < ComputeDispatchMaxResourcesRW; i++)
	{
		auto rw = params->resourcesRW[i];
		D3D12_CPU_DESCRIPTOR_HANDLE srcHandle = rw ? rw->UAV() : context->m_nullUAV;
		context->m_desc.device->CopyDescriptorsSimple(1u, handles.cpuHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		handles.cpuHandle.ptr += handles.descriptorSize;
	}

	commandList->SetComputeRootDescriptorTable(1u, handles.gpuHandle);
	handles.gpuHandle.ptr += handles.descriptorSize * ComputeDispatchMaxResources;
	commandList->SetComputeRootDescriptorTable(2u, handles.gpuHandle);

	if (params->constantBuffer)
	{
		auto cbv = params->constantBuffer->m_buffers.front()->m_buffer;
		commandList->SetComputeRootConstantBufferView(0u, cbv->GetGPUVirtualAddress());
	}

	ComputeUint barrierIdx = 0u;
	D3D12_RESOURCE_BARRIER barrier[ComputeDispatchMaxResources + ComputeDispatchMaxResourcesRW];
	for (ComputeUint i = 0u; i < ComputeDispatchMaxResources; i++)
	{
		auto r = params->resources[i];
		if (r)
		{
			if (!((*r->currentState()) & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE))
			{
				D3D12_RESOURCE_BARRIER& bar = barrier[barrierIdx++];
				bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				bar.Transition.pResource = r->resource();
				bar.Transition.Subresource = 0u;
				bar.Transition.StateBefore = *r->currentState();
				bar.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;

				*r->currentState() = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
			}
		}
	}
	for (ComputeUint i = 0u; i < ComputeDispatchMaxResourcesRW; i++)
	{
		auto rw = params->resourcesRW[i];
		if (rw)
		{
			if ((*rw->currentState()) == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
			{
				D3D12_RESOURCE_BARRIER& bar = barrier[barrierIdx++];
				bar.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				bar.UAV.pResource = rw->resource();
			}
			else
			{
				D3D12_RESOURCE_BARRIER& bar = barrier[barrierIdx++];
				bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				bar.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				bar.Transition.pResource = rw->resource();
				bar.Transition.Subresource = 0u;
				bar.Transition.StateBefore = *rw->currentState();
				bar.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;

				*rw->currentState() = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
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

ComputeDescriptorReserveHandleD3D12 ComputeReserveDescriptors(void* userdata, UINT numDescriptors, UINT64 lastFenceCompleted, UINT64 nextFenceValue)
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

ComputeContext* ComputeContextNvFlowContextCreate(NvFlowContext* flowContext)
{
	ComputeContextDesc desc = {};
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
	desc.dynamicHeapCbvSrvUav.reserveDescriptors = ComputeReserveDescriptors;

	auto computeContext = ComputeContextCreate(&desc);

	computeContext->m_computeUserdata = computeUserdata;

	return computeContext;
}

void ComputeContextNvFlowContextUpdate(ComputeContext* computeContext, NvFlowContext* flowContext)
{
	ComputeContextDesc desc = {};
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
	desc.dynamicHeapCbvSrvUav.reserveDescriptors = ComputeReserveDescriptors;

	ComputeContextUpdate(computeContext, &desc);
}

inline void updateComputeResourceDesc(NvFlowResourceViewDescD3D12* flowViewDesc, ComputeResourceDesc* desc)
{
	desc->srv = flowViewDesc->srvHandle;
	desc->resource = flowViewDesc->resource;
	desc->currentState = flowViewDesc->currentState;
}

ComputeResource* ComputeResourceNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	NvFlowResourceViewDescD3D12 flowViewDesc = {};
	NvFlowUpdateResourceViewDescD3D12(flowContext, flowResource, &flowViewDesc);
	ComputeResourceDesc desc = {};
	updateComputeResourceDesc(&flowViewDesc, &desc);
	return ComputeResourceCreate(context, &desc);
}

void ComputeResourceNvFlowUpdate(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	NvFlowResourceViewDescD3D12 flowViewDesc = {};
	NvFlowUpdateResourceViewDescD3D12(flowContext, flowResource, &flowViewDesc);
	ComputeResourceDesc desc = {};
	updateComputeResourceDesc(&flowViewDesc, &desc);
	ComputeResourceUpdate(context, resource, &desc);
}

inline void updateComputeResourceRWDesc(NvFlowResourceRWViewDescD3D12* flowViewDesc, ComputeResourceRWDesc* desc)
{
	updateComputeResourceDesc(&flowViewDesc->resourceView, &desc->resource);
	desc->uav = flowViewDesc->uavHandle;
}

ComputeResourceRW* ComputeResourceRWNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	NvFlowResourceRWViewDescD3D12 flowViewDesc = {};
	NvFlowUpdateResourceRWViewDescD3D12(flowContext, flowResourceRW, &flowViewDesc);
	ComputeResourceRWDesc desc = {};
	updateComputeResourceRWDesc(&flowViewDesc, &desc);
	return ComputeResourceRWCreate(context, &desc);
}

void ComputeResourceRWNvFlowUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	NvFlowResourceRWViewDescD3D12 flowViewDesc = {};
	NvFlowUpdateResourceRWViewDescD3D12(flowContext, flowResourceRW, &flowViewDesc);
	ComputeResourceRWDesc desc = {};
	updateComputeResourceRWDesc(&flowViewDesc, &desc);
	ComputeResourceRWUpdate(context, resourceRW, &desc);
}