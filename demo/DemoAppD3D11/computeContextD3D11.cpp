/*
* Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include <d3d11.h>

#include "computeContextD3D11.h"

// ************************** Compute Context Implementation ******************

template <class T>
void inline COMRelease(T& t)
{
	if (t) t->Release();
	t = nullptr;
}

struct ComputeContext
{
	ComputeContextDesc m_desc = {};

	ID3D11SamplerState* m_sampler0 = nullptr;
	ID3D11SamplerState* m_sampler1 = nullptr;
	ID3D11SamplerState* m_sampler2 = nullptr;
	ID3D11SamplerState* m_sampler3 = nullptr;
	ID3D11SamplerState* m_sampler4 = nullptr;
	ID3D11SamplerState* m_sampler5 = nullptr;

	ComputeContext(const ComputeContextDesc* desc)
	{
		m_desc = *desc;

		auto createSampler = [&](D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE mode)
		{
			D3D11_SAMPLER_DESC samplerDesc;
			samplerDesc.Filter = filter;
			samplerDesc.AddressU = mode;
			samplerDesc.AddressV = mode;
			samplerDesc.AddressW = mode;
			samplerDesc.MipLODBias = 0;
			samplerDesc.MaxAnisotropy = 0;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			samplerDesc.BorderColor[0] = 0.f;
			samplerDesc.BorderColor[1] = 0.f;
			samplerDesc.BorderColor[2] = 0.f;
			samplerDesc.BorderColor[3] = 0.f;
			samplerDesc.MinLOD = 0.f;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			ID3D11SamplerState* sampler = nullptr;
			m_desc.device->CreateSamplerState(&samplerDesc, &sampler);
			return sampler;
		};

		m_sampler0 = createSampler(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_BORDER);
		m_sampler1 = createSampler(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_BORDER);
		m_sampler2 = createSampler(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);
		m_sampler3 = createSampler(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);
		m_sampler4 = createSampler(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);
		m_sampler5 = createSampler(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);
	}
	~ComputeContext()
	{
		COMRelease(m_sampler0);
		COMRelease(m_sampler1);
		COMRelease(m_sampler2);
		COMRelease(m_sampler3);
		COMRelease(m_sampler4);
		COMRelease(m_sampler5);
	}
};

struct ComputeShader
{
	ID3D11ComputeShader* m_shader = nullptr;
	ComputeShader(ID3D11ComputeShader* shader)
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
	ID3D11Buffer* m_buffer;
	ComputeConstantBuffer(ID3D11Buffer* buffer)
	{
		m_buffer = buffer;
	}
	~ComputeConstantBuffer()
	{
		COMRelease(m_buffer);
	}
};

struct ComputeResource
{
protected:
	ID3D11ShaderResourceView* m_srv = nullptr;
public:
	void update(const ComputeResourceDesc* desc)
	{
		m_srv = desc->srv;
	}

	ComputeResource(const ComputeResourceDesc* desc)
	{
		update(desc);
	}

	ID3D11ShaderResourceView* SRV()
	{
		return m_srv;
	}
};

struct ComputeResourceRW : public ComputeResource
{
protected:
	ID3D11UnorderedAccessView* m_uav;
public:
	void update(const ComputeResourceRWDesc* descRW)
	{
		m_uav = descRW->uav;
		ComputeResource::update(&descRW->resource);
	}

	ComputeResourceRW(const ComputeResourceRWDesc* descRW):
		ComputeResource(&descRW->resource)
	{
		m_uav = descRW->uav;
	}

	ID3D11UnorderedAccessView* UAV()
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
	ID3D11ComputeShader* computeShader = nullptr;
	context->m_desc.device->CreateComputeShader(desc->cs, desc->cs_length, nullptr, &computeShader);
	return new ComputeShader(computeShader);
}

void ComputeShaderRelease(ComputeShader* shader)
{
	delete shader;
}

ComputeConstantBuffer* ComputeConstantBufferCreate(ComputeContext* context, const ComputeConstantBufferDesc* desc)
{
	ID3D11Buffer* constantBuffer = nullptr;
	{
		D3D11_BUFFER_DESC bufDesc;
		bufDesc.ByteWidth = desc->sizeInBytes;
		bufDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufDesc.MiscFlags = 0;

		context->m_desc.device->CreateBuffer(&bufDesc, nullptr, &constantBuffer);
	}
	return new ComputeConstantBuffer(constantBuffer);
}

void ComputeConstantBufferRelease(ComputeConstantBuffer* constantBuffer)
{
	delete constantBuffer;
}

void* ComputeConstantBufferMap(ComputeContext* context, ComputeConstantBuffer* constantBuffer)
{
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->m_desc.deviceContext->Map(constantBuffer->m_buffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mapped);
	return mapped.pData;
}

void ComputeConstantBufferUnmap(ComputeContext* context, ComputeConstantBuffer* constantBuffer)
{
	context->m_desc.deviceContext->Unmap(constantBuffer->m_buffer, 0u);
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
	auto& deviceContext = context->m_desc.deviceContext;

	if (params->shader) deviceContext->CSSetShader(params->shader->m_shader, nullptr, 0u);

	ID3D11ShaderResourceView* srvs[ComputeDispatchMaxResources] = { nullptr };
	ID3D11UnorderedAccessView* uavs[ComputeDispatchMaxResourcesRW] = { nullptr };
	for (unsigned int i = 0u; i < ComputeDispatchMaxResources; i++)
	{
		if (params->resources[i]) srvs[i] = params->resources[i]->SRV();
	}
	for (unsigned int i = 0u; i < ComputeDispatchMaxResourcesRW; i++)
	{
		if (params->resourcesRW[i]) uavs[i] = params->resourcesRW[i]->UAV();
	}
	deviceContext->CSSetShaderResources(0u, ComputeDispatchMaxResources, srvs);
	deviceContext->CSSetUnorderedAccessViews(0u, ComputeDispatchMaxResourcesRW, uavs, nullptr);

	if (params->constantBuffer) deviceContext->CSSetConstantBuffers(0u, 1u, &params->constantBuffer->m_buffer);

	ID3D11SamplerState* samplers[] = { 
		context->m_sampler0, context->m_sampler1, context->m_sampler2, 
		context->m_sampler3, context->m_sampler4, context->m_sampler5 
	};
	deviceContext->CSSetSamplers(0u, 6u, samplers);

	deviceContext->Dispatch(params->gridDim[0u], params->gridDim[1u], params->gridDim[2u]);

	ID3D11ShaderResourceView* nullsrvs[ComputeDispatchMaxResources] = { nullptr };
	ID3D11UnorderedAccessView* nulluavs[ComputeDispatchMaxResourcesRW] = { nullptr };
	deviceContext->CSSetShaderResources(0, ComputeDispatchMaxResources, nullsrvs);
	deviceContext->CSSetUnorderedAccessViews(0, ComputeDispatchMaxResourcesRW, nulluavs, nullptr);
}

// ******************************* NvFlow Interoperation ****************************************

#include "NvFlow.h"
#include "NvFlowContextD3D11.h"

inline void updateComputeContextDesc(NvFlowContext* flowContext, ComputeContextDesc* desc)
{
	NvFlowContextDescD3D11 srcDesc = {};
	NvFlowUpdateContextDescD3D11(flowContext, &srcDesc);
	desc->device = srcDesc.device;
	desc->deviceContext = srcDesc.deviceContext;
}

ComputeContext* ComputeContextNvFlowContextCreate(NvFlowContext* flowContext)
{
	ComputeContextDesc desc = {};
	updateComputeContextDesc(flowContext, &desc);
	return ComputeContextCreate(&desc);
}

void ComputeContextNvFlowContextUpdate(ComputeContext* computeContext, NvFlowContext* flowContext)
{
	ComputeContextDesc desc = {};
	updateComputeContextDesc(flowContext, &desc);
	ComputeContextUpdate(computeContext, &desc);
}

ComputeResource* ComputeResourceNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	NvFlowResourceViewDescD3D11 flowViewDesc = {};
	NvFlowUpdateResourceViewDescD3D11(flowContext, flowResource, &flowViewDesc);
	ComputeResourceDesc desc = {};
	desc.srv = flowViewDesc.srv;
	return ComputeResourceCreate(context, &desc);
}

void ComputeResourceNvFlowUpdate(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	NvFlowResourceViewDescD3D11 flowViewDesc = {};
	NvFlowUpdateResourceViewDescD3D11(flowContext, flowResource, &flowViewDesc);
	ComputeResourceDesc desc = {};
	desc.srv = flowViewDesc.srv;
	ComputeResourceUpdate(context, resource, &desc);
}

ComputeResourceRW* ComputeResourceRWNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	NvFlowResourceRWViewDescD3D11 flowViewDesc = {};
	NvFlowUpdateResourceRWViewDescD3D11(flowContext, flowResourceRW, &flowViewDesc);
	ComputeResourceRWDesc desc = {};
	desc.resource.srv = flowViewDesc.resourceView.srv;
	desc.uav = flowViewDesc.uav;
	return ComputeResourceRWCreate(context, &desc);
}

void ComputeResourceRWNvFlowUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	NvFlowResourceRWViewDescD3D11 flowViewDesc = {};
	NvFlowUpdateResourceRWViewDescD3D11(flowContext, flowResourceRW, &flowViewDesc);
	ComputeResourceRWDesc desc = {};
	desc.resource.srv = flowViewDesc.resourceView.srv;
	desc.uav = flowViewDesc.uav;
	ComputeResourceRWUpdate(context, resourceRW, &desc);
}