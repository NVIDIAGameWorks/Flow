/*
* Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

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
void customEmitEmitCS(uint3 tidx : SV_DispatchThreadID)
{
	//int3 vidx = DispatchIDToVirtual(tidx);

	int3 vidx = tidx.xyz + minVidx.xyz;

	if (all(vidx >= int3(minVidx.xyz)) && all(vidx < int3(maxVidx.xyz)))
	{
		int3 ridx = VirtualToReal(vidx);

		float4 value = dataSRV[ridx];

		value = (1.f.xxxx - blendRate) * value + blendRate * targetValue;

		dataUAV[ridx] = value;
	}
}