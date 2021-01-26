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

typedef uint4 NvFlowUint4;
typedef float4 NvFlowFloat4;

/// Begin Samplers supplied by ComputeContext 
SamplerState borderSampler : register(s0);
SamplerState borderPointSampler : register(s1);
SamplerState wrapSampler : register(s2);
SamplerState wrapPointSampler : register(s3);
SamplerState clampSampler : register(s4);
SamplerState clampPointSampler : register(s5);
/// End Samplers supplied by ComputeContext 

#include "../DemoApp/flowShaderParams.h"

cbuffer params : register(b0)
{
	NvFlowShaderPointParams customEmitParams;

	NvFlowUint4 minVidx;
	NvFlowUint4 maxVidx;
	NvFlowFloat4 targetValue;
	NvFlowFloat4 blendRate;
};

Buffer<uint> blockList : register(t0);
Texture3D<uint> blockTable : register(t1);
Texture3D<float4> dataSRV : register(t2);
RWTexture3D<float4> dataUAV : register(u0);

NV_FLOW_DISPATCH_ID_TO_VIRTUAL(blockList, customEmitParams);

NV_FLOW_VIRTUAL_TO_REAL(VirtualToReal, blockTable, customEmitParams);

[numthreads(THREAD_DIM_X, THREAD_DIM_Y, THREAD_DIM_Z)]
void customEmitEmit2CS(uint3 tidx : SV_DispatchThreadID)
{
	int3 vidx = DispatchIDToVirtual(tidx);

	//int3 vidx = tidx.xyz + minVidx.xyz;

	int3 ridx = VirtualToReal(vidx);

	float4 value = dataSRV[ridx];

	if (all(vidx >= int3(minVidx.xyz)) && all(vidx < int3(maxVidx.xyz)))
	{
		value = (1.f.xxxx - blendRate) * value + blendRate * targetValue;
		value = (1.f.xxxx - blendRate) * value + blendRate * targetValue;
	}

	dataUAV[ridx] = value;
}