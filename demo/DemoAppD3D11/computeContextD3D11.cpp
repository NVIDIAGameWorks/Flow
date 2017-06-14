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

struct ComputeContextD3D11
{
	ComputeContextDescD3D11 m_desc = {};

	ID3D11SamplerState* m_sampler0 = nullptr;
	ID3D11SamplerState* m_sampler1 = nullptr;
	ID3D11SamplerState* m_sampler2 = nullptr;
	ID3D11SamplerState* m_sampler3 = nullptr;
	ID3D11SamplerState* m_sampler4 = nullptr;
	ID3D11SamplerState* m_sampler5 = nullptr;

	ComputeContextD3D11(const ComputeContextDesc* descIn)
	{
		const auto desc = cast_to_ComputeContextDescD3D11(descIn);

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
	~ComputeContextD3D11()
	{
		COMRelease(m_sampler0);
		COMRelease(m_sampler1);
		COMRelease(m_sampler2);
		COMRelease(m_sampler3);
		COMRelease(m_sampler4);
		COMRelease(m_sampler5);
	}
};

inline ComputeContextD3D11* cast_to_ComputeContextD3D11(ComputeContext* ctx)
{
	return (ComputeContextD3D11*)(ctx);
}

inline ComputeContext* cast_from_ComputeContextD3D11(ComputeContextD3D11* ctx)
{
	return (ComputeContext*)(ctx);
}

struct ComputeShaderD3D11
{
	ID3D11ComputeShader* m_shader = nullptr;
	ComputeShaderD3D11(ID3D11ComputeShader* shader)
	{
		m_shader = shader;
	}
	~ComputeShaderD3D11()
	{
		COMRelease(m_shader);
	}
};

inline ComputeShaderD3D11* cast_to_ComputeShaderD3D11(ComputeShader* ctx)
{
	return (ComputeShaderD3D11*)(ctx);
}

inline ComputeShader* cast_from_ComputeShaderD3D11(ComputeShaderD3D11* ctx)
{
	return (ComputeShader*)(ctx);
}

struct ComputeConstantBufferD3D11
{
	ID3D11Buffer* m_buffer;
	ComputeConstantBufferD3D11(ID3D11Buffer* buffer)
	{
		m_buffer = buffer;
	}
	~ComputeConstantBufferD3D11()
	{
		COMRelease(m_buffer);
	}
};

inline ComputeConstantBufferD3D11* cast_to_ComputeConstantBufferD3D11(ComputeConstantBuffer* ctx)
{
	return (ComputeConstantBufferD3D11*)(ctx);
}

inline ComputeConstantBuffer* cast_from_ComputeConstantBufferD3D11(ComputeConstantBufferD3D11* ctx)
{
	return (ComputeConstantBuffer*)(ctx);
}

struct ComputeResourceD3D11
{
protected:
	ID3D11ShaderResourceView* m_srv = nullptr;
public:
	void update(const ComputeResourceDesc* descIn)
	{
		const auto desc = cast_to_ComputeResourceDescD3D11(descIn);
		m_srv = desc->srv;
	}

	ComputeResourceD3D11(const ComputeResourceDesc* desc)
	{
		update(desc);
	}

	ID3D11ShaderResourceView* SRV()
	{
		return m_srv;
	}
};

inline ComputeResourceD3D11* cast_to_ComputeResourceD3D11(ComputeResource* ctx)
{
	return (ComputeResourceD3D11*)(ctx);
}

inline ComputeResource* cast_from_ComputeResourceD3D11(ComputeResourceD3D11* ctx)
{
	return (ComputeResource*)(ctx);
}

struct ComputeResourceRWD3D11 : public ComputeResourceD3D11
{
protected:
	ID3D11UnorderedAccessView* m_uav;
public:
	static const ComputeResourceRWDescD3D11* cast(const ComputeResourceRWDesc* descRW)
	{
		return cast_to_ComputeResourceRWDescD3D11(descRW);
	}

	void update(const ComputeResourceRWDesc* descRWIn)
	{
		const auto descRW = cast(descRWIn);
		m_uav = descRW->uav;
		ComputeResourceD3D11::update(cast_from_ComputeResourceDescD3D11(&descRW->resource));
	}

	ComputeResourceRWD3D11(const ComputeResourceRWDesc* descRWIn):
		ComputeResourceD3D11(cast_from_ComputeResourceDescD3D11(&cast(descRWIn)->resource))
	{
		const auto descRW = cast(descRWIn);
		m_uav = descRW->uav;
	}

	ID3D11UnorderedAccessView* UAV()
	{
		return m_uav;
	}
};

inline ComputeResourceRWD3D11* cast_to_ComputeResourceRWD3D11(ComputeResourceRW* ctx)
{
	return (ComputeResourceRWD3D11*)(ctx);
}

inline ComputeResourceRW* cast_from_ComputeResourceRWD3D11(ComputeResourceRWD3D11* ctx)
{
	return (ComputeResourceRW*)(ctx);
}

// ************* API functions ****************

ComputeContext* ComputeContextCreateD3D11(ComputeContextDesc* desc)
{
	return cast_from_ComputeContextD3D11(new ComputeContextD3D11(desc));
}

void ComputeContextUpdateD3D11(ComputeContext* contextIn, ComputeContextDesc* descIn)
{
	auto context = cast_to_ComputeContextD3D11(contextIn);
	auto desc = cast_to_ComputeContextDescD3D11(descIn);

	context->m_desc = *desc;
}

void ComputeContextReleaseD3D11(ComputeContext* context)
{
	delete cast_to_ComputeContextD3D11(context);
}

ComputeShader* ComputeShaderCreateD3D11(ComputeContext* contextIn, const ComputeShaderDesc* desc)
{
	auto context = cast_to_ComputeContextD3D11(contextIn);

	ID3D11ComputeShader* computeShader = nullptr;
	context->m_desc.device->CreateComputeShader(desc->cs, desc->cs_length, nullptr, &computeShader);
	return cast_from_ComputeShaderD3D11(new ComputeShaderD3D11(computeShader));
}

void ComputeShaderReleaseD3D11(ComputeShader* shader)
{
	delete cast_to_ComputeShaderD3D11(shader);
}

ComputeConstantBuffer* ComputeConstantBufferCreateD3D11(ComputeContext* contextIn, const ComputeConstantBufferDesc* desc)
{
	auto context = cast_to_ComputeContextD3D11(contextIn);

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
	return cast_from_ComputeConstantBufferD3D11(new ComputeConstantBufferD3D11(constantBuffer));
}

void ComputeConstantBufferReleaseD3D11(ComputeConstantBuffer* constantBuffer)
{
	delete cast_to_ComputeConstantBufferD3D11(constantBuffer);
}

void* ComputeConstantBufferMapD3D11(ComputeContext* contextIn, ComputeConstantBuffer* constantBufferIn)
{
	auto context = cast_to_ComputeContextD3D11(contextIn);
	auto constantBuffer = cast_to_ComputeConstantBufferD3D11(constantBufferIn);

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->m_desc.deviceContext->Map(constantBuffer->m_buffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mapped);
	return mapped.pData;
}

void ComputeConstantBufferUnmapD3D11(ComputeContext* contextIn, ComputeConstantBuffer* constantBufferIn)
{
	auto context = cast_to_ComputeContextD3D11(contextIn);
	auto constantBuffer = cast_to_ComputeConstantBufferD3D11(constantBufferIn);

	context->m_desc.deviceContext->Unmap(constantBuffer->m_buffer, 0u);
}

ComputeResource* ComputeResourceCreateD3D11(ComputeContext* context, const ComputeResourceDesc* desc)
{
	return cast_from_ComputeResourceD3D11(new ComputeResourceD3D11(desc));
}

void ComputeResourceUpdateD3D11(ComputeContext* context, ComputeResource* resourceIn, const ComputeResourceDesc* desc)
{
	auto resource = cast_to_ComputeResourceD3D11(resourceIn);

	resource->update(desc);
}

void ComputeResourceReleaseD3D11(ComputeResource* resource)
{
	delete cast_to_ComputeResourceD3D11(resource);
}

ComputeResourceRW* ComputeResourceRWCreateD3D11(ComputeContext* context, const ComputeResourceRWDesc* desc)
{
	return cast_from_ComputeResourceRWD3D11(new ComputeResourceRWD3D11(desc));
}

void ComputeResourceRWUpdateD3D11(ComputeContext* context, ComputeResourceRW* resourceRWIn, const ComputeResourceRWDesc* desc)
{
	auto resourceRW = cast_to_ComputeResourceRWD3D11(resourceRWIn);

	resourceRW->update(desc);
}

void ComputeResourceRWReleaseD3D11(ComputeResourceRW* resourceRW)
{
	delete cast_to_ComputeResourceRWD3D11(resourceRW);
}

ComputeResource* ComputeResourceRWGetResourceD3D11(ComputeResourceRW* resourceRWIn)
{
	auto resourceRW = cast_to_ComputeResourceRWD3D11(resourceRWIn);
	return cast_from_ComputeResourceD3D11(static_cast<ComputeResourceD3D11*>(resourceRW));
}

void ComputeContextDispatchD3D11(ComputeContext* contextIn, const ComputeDispatchParams* params)
{
	auto context = cast_to_ComputeContextD3D11(contextIn);

	auto& deviceContext = context->m_desc.deviceContext;

	if (params->shader) deviceContext->CSSetShader(cast_to_ComputeShaderD3D11(params->shader)->m_shader, nullptr, 0u);

	ID3D11ShaderResourceView* srvs[ComputeDispatchMaxResources] = { nullptr };
	ID3D11UnorderedAccessView* uavs[ComputeDispatchMaxResourcesRW] = { nullptr };
	for (unsigned int i = 0u; i < ComputeDispatchMaxResources; i++)
	{
		if (params->resources[i]) srvs[i] = cast_to_ComputeResourceD3D11(params->resources[i])->SRV();
	}
	for (unsigned int i = 0u; i < ComputeDispatchMaxResourcesRW; i++)
	{
		if (params->resourcesRW[i]) uavs[i] = cast_to_ComputeResourceRWD3D11(params->resourcesRW[i])->UAV();
	}
	deviceContext->CSSetShaderResources(0u, ComputeDispatchMaxResources, srvs);
	deviceContext->CSSetUnorderedAccessViews(0u, ComputeDispatchMaxResourcesRW, uavs, nullptr);

	if (params->constantBuffer) deviceContext->CSSetConstantBuffers(0u, 1u, &cast_to_ComputeConstantBufferD3D11(params->constantBuffer)->m_buffer);

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

inline void updateComputeContextDesc(NvFlowContext* flowContext, ComputeContextDescD3D11* desc)
{
	NvFlowContextDescD3D11 srcDesc = {};
	NvFlowUpdateContextDescD3D11(flowContext, &srcDesc);
	desc->device = srcDesc.device;
	desc->deviceContext = srcDesc.deviceContext;
}

ComputeContext* ComputeContextNvFlowContextCreateD3D11(NvFlowContext* flowContext)
{
	ComputeContextDescD3D11 desc = {};
	updateComputeContextDesc(flowContext, &desc);
	return ComputeContextCreateD3D11(cast_from_ComputeContextDescD3D11(&desc));
}

void ComputeContextNvFlowContextUpdateD3D11(ComputeContext* computeContext, NvFlowContext* flowContext)
{
	ComputeContextDescD3D11 desc = {};
	updateComputeContextDesc(flowContext, &desc);
	ComputeContextUpdateD3D11(computeContext, cast_from_ComputeContextDescD3D11(&desc));
}

ComputeResource* ComputeResourceNvFlowCreateD3D11(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	NvFlowResourceViewDescD3D11 flowViewDesc = {};
	NvFlowUpdateResourceViewDescD3D11(flowContext, flowResource, &flowViewDesc);
	ComputeResourceDescD3D11 desc = {};
	desc.srv = flowViewDesc.srv;
	return ComputeResourceCreateD3D11(context, cast_from_ComputeResourceDescD3D11(&desc));
}

void ComputeResourceNvFlowUpdateD3D11(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource)
{
	NvFlowResourceViewDescD3D11 flowViewDesc = {};
	NvFlowUpdateResourceViewDescD3D11(flowContext, flowResource, &flowViewDesc);
	ComputeResourceDescD3D11 desc = {};
	desc.srv = flowViewDesc.srv;
	ComputeResourceUpdateD3D11(context, resource, cast_from_ComputeResourceDescD3D11(&desc));
}

ComputeResourceRW* ComputeResourceRWNvFlowCreateD3D11(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	NvFlowResourceRWViewDescD3D11 flowViewDesc = {};
	NvFlowUpdateResourceRWViewDescD3D11(flowContext, flowResourceRW, &flowViewDesc);
	ComputeResourceRWDescD3D11 desc = {};
	desc.resource.srv = flowViewDesc.resourceView.srv;
	desc.uav = flowViewDesc.uav;
	return ComputeResourceRWCreateD3D11(context, cast_from_ComputeResourceRWDescD3D11(&desc));
}

void ComputeResourceRWNvFlowUpdateD3D11(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW)
{
	NvFlowResourceRWViewDescD3D11 flowViewDesc = {};
	NvFlowUpdateResourceRWViewDescD3D11(flowContext, flowResourceRW, &flowViewDesc);
	ComputeResourceRWDescD3D11 desc = {};
	desc.resource.srv = flowViewDesc.resourceView.srv;
	desc.uav = flowViewDesc.uav;
	ComputeResourceRWUpdateD3D11(context, resourceRW, cast_from_ComputeResourceRWDescD3D11(&desc));
}