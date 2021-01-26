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

#include "../DemoApp/computeContext.h"

struct ComputeContextDescD3D11
{
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
};

struct ComputeResourceDescD3D11
{
	ID3D11ShaderResourceView* srv;
};

struct ComputeResourceRWDescD3D11
{
	ComputeResourceDescD3D11 resource;
	ID3D11UnorderedAccessView* uav;
};

inline const ComputeContextDescD3D11* cast_to_ComputeContextDescD3D11(const ComputeContextDesc* desc)
{
	return (const ComputeContextDescD3D11*)(desc);
}

inline ComputeContextDesc* cast_from_ComputeContextDescD3D11(ComputeContextDescD3D11* desc)
{
	return (ComputeContextDesc*)(desc);
}

inline const ComputeResourceDescD3D11* cast_to_ComputeResourceDescD3D11(const ComputeResourceDesc* desc)
{
	return (const ComputeResourceDescD3D11*)(desc);
}

inline const ComputeResourceDesc* cast_from_ComputeResourceDescD3D11(const ComputeResourceDescD3D11* desc)
{
	return (const ComputeResourceDesc*)(desc);
}

inline const ComputeResourceRWDescD3D11* cast_to_ComputeResourceRWDescD3D11(const ComputeResourceRWDesc* desc)
{
	return (const ComputeResourceRWDescD3D11*)(desc);
}

inline ComputeResourceRWDesc* cast_from_ComputeResourceRWDescD3D11(ComputeResourceRWDescD3D11* desc)
{
	return (ComputeResourceRWDesc*)(desc);
}

COMPUTE_API ComputeContext* ComputeContextCreateD3D11(ComputeContextDesc* desc);

COMPUTE_API void ComputeContextUpdateD3D11(ComputeContext* context, ComputeContextDesc* desc);

COMPUTE_API void ComputeContextReleaseD3D11(ComputeContext* context);

COMPUTE_API ComputeShader* ComputeShaderCreateD3D11(ComputeContext* context, const ComputeShaderDesc* desc);

COMPUTE_API void ComputeShaderReleaseD3D11(ComputeShader* shader);

COMPUTE_API ComputeConstantBuffer* ComputeConstantBufferCreateD3D11(ComputeContext* context, const ComputeConstantBufferDesc* desc);

COMPUTE_API void ComputeConstantBufferReleaseD3D11(ComputeConstantBuffer* constantBuffer);

COMPUTE_API void* ComputeConstantBufferMapD3D11(ComputeContext* context, ComputeConstantBuffer* constantBuffer);

COMPUTE_API void ComputeConstantBufferUnmapD3D11(ComputeContext* context, ComputeConstantBuffer* constantBuffer);

COMPUTE_API ComputeResource* ComputeResourceCreateD3D11(ComputeContext* context, const ComputeResourceDesc* desc);

COMPUTE_API void ComputeResourceUpdateD3D11(ComputeContext* context, ComputeResource* resource, const ComputeResourceDesc* desc);

COMPUTE_API void ComputeResourceReleaseD3D11(ComputeResource* resource);

COMPUTE_API ComputeResourceRW* ComputeResourceRWCreateD3D11(ComputeContext* context, const ComputeResourceRWDesc* desc);

COMPUTE_API void ComputeResourceRWUpdateD3D11(ComputeContext* context, ComputeResourceRW* resourceRW, const ComputeResourceRWDesc* desc);

COMPUTE_API void ComputeResourceRWReleaseD3D11(ComputeResourceRW* resourceRW);

COMPUTE_API ComputeResource* ComputeResourceRWGetResourceD3D11(ComputeResourceRW* resourceRW);

COMPUTE_API void ComputeContextDispatchD3D11(ComputeContext* context, const ComputeDispatchParams* params);

COMPUTE_API ComputeContext* ComputeContextNvFlowContextCreateD3D11(NvFlowContext* flowContext);

COMPUTE_API void ComputeContextNvFlowContextUpdateD3D11(ComputeContext* computeContext, NvFlowContext* flowContext);

COMPUTE_API ComputeResource* ComputeResourceNvFlowCreateD3D11(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource);

COMPUTE_API void ComputeResourceNvFlowUpdateD3D11(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource);

COMPUTE_API ComputeResourceRW* ComputeResourceRWNvFlowCreateD3D11(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW);

COMPUTE_API void ComputeResourceRWNvFlowUpdateD3D11(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW);