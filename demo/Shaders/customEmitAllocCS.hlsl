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

cbuffer params : register(b0)
{
	NvFlowUint4 minMaskIdx;
	NvFlowUint4 maxMaskIdx;
};

RWTexture3D<uint> maskUAV : register(u0);

[numthreads(THREAD_DIM_X, THREAD_DIM_Y, THREAD_DIM_Z)]
void customEmitAllocCS(uint3 tidx : SV_DispatchThreadID)
{
	int3 maskIdx = tidx + minMaskIdx.xyz;

	if (all(maskIdx >= int3(minMaskIdx.xyz)) && all(maskIdx < int3(maxMaskIdx.xyz)))
	{
		maskUAV[maskIdx] = 1u;
	}
}