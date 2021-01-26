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

#define THREAD_DIM_X 8
#define THREAD_DIM_Y 8
#define THREAD_DIM_Z 8

/// Begin Samplers supplied by ComputeContext 
SamplerState borderSampler : register(s0);
SamplerState borderPointSampler : register(s1);
SamplerState wrapSampler : register(s2);
SamplerState wrapPointSampler : register(s3);
SamplerState clampSampler : register(s4);
SamplerState clampPointSampler : register(s5);
/// End Samplers supplied by ComputeContext 

typedef uint4 NvFlowUint4;
typedef float4 NvFlowFloat4;

#include "../DemoApp/flowShaderParams.h"

struct Light
{
	float4 location;
	float4 intensity;
	float4 bias;
	float4 falloff;
};

cbuffer params : register(b0)
{
	NvFlowShaderLinearParams exportParams;
	NvFlowShaderLinearParams importParams;

	Light light[3];
};

Buffer<uint> exportBlockList : register(t0);
Texture3D<uint> exportBlockTable : register(t1);
Texture3D<float4> exportData : register(t2);

Buffer<uint> importBlockList : register(t3);
Texture3D<uint> importBlockTable : register(t4);
RWTexture3D<float4> importDataRW : register(u0);

NV_FLOW_DISPATCH_ID_TO_VIRTUAL(importBlockList, importParams);

NV_FLOW_VIRTUAL_TO_REAL_LINEAR(VirtualToRealExport, exportBlockTable, exportParams);
NV_FLOW_VIRTUAL_TO_REAL(VirtualToRealImport, importBlockTable, importParams);

float4 applyLight(uniform Light light, float3 vidxNorm)
{
	float3 offset = vidxNorm.xyz - light.location.xyz;
	float dist2 = dot(offset.xyz, offset.xyz);
	return light.intensity * light.bias / (light.bias + light.falloff * dist2);
}

[numthreads(THREAD_DIM_X, THREAD_DIM_Y, THREAD_DIM_Z)]
void customLightingCS(uint3 tidx : SV_DispatchThreadID)
{
	int3 vidx = DispatchIDToVirtual(tidx);
	float3 vidxf = float3(vidx)+0.5f.xxx;

	float3 vidxNorm = 2.f * vidxf * exportParams.vdimInv.xyz - 1.f;

	float3 ridxExport = VirtualToRealExport(vidxf);
	float4 value = exportData.SampleLevel(borderSampler,exportParams.dimInv.xyz * ridxExport,0);
	float temp = value.x;

	float4 color = float4(0.1f * temp, 0.1f * temp, 0.1f * temp, 0.25f);

	color += applyLight(light[0], vidxNorm);
	color += applyLight(light[1], vidxNorm);
	color += applyLight(light[2], vidxNorm);

	color.w *= 0.25f * max(temp - 0.25f, 0.f);

	int3 ridxImport = VirtualToRealImport(vidx);
	importDataRW[ridxImport] = color;
}