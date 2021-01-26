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

#ifndef NV_FLOW_SHADER_H
#define NV_FLOW_SHADER_H

// --------------------------- NvFlow Shader Parameters -------------------------------
///@defgroup NvFlowContext
///@{

#if (NV_FLOW_SHADER_UTILS != 0)

int3 NvFlow_tableVal_to_coord(uint val)
{
	uint valInv = ~val;
	return int3(
		(valInv >> 0) & 0x3FF,
		(valInv >> 10) & 0x3FF,
		(valInv >> 20) & 0x3FF);
}

#define NV_FLOW_DISPATCH_ID_TO_VIRTUAL(blockListSRV, params) \
	int3 DispatchIDToVirtual(uint3 tidx) \
	{ \
		uint blockID = tidx.x >> params.blockDimBits.x; \
		int3 vBlockIdx = NvFlow_tableVal_to_coord(blockListSRV[blockID]); \
		int3 vidx = (vBlockIdx << params.blockDimBits.xyz) | (tidx & (params.blockDim.xyz - int3(1,1,1))); \
		return vidx; \
	}

#define NV_FLOW_VIRTUAL_TO_REAL(name, blockTableSRV, params) \
	int3 name(int3 vidx) \
	{ \
		if(params.isVTR.x != 0) \
		{ \
			return vidx; \
		} \
		else \
		{ \
			int3 vBlockIdx = vidx >> params.blockDimBits.xyz; \
			int3 rBlockIdx = NvFlow_tableVal_to_coord(blockTableSRV[vBlockIdx]); \
			int3 ridx = (rBlockIdx << params.blockDimBits.xyz) | (vidx & (params.blockDim.xyz - int3(1, 1, 1))); \
			return ridx; \
		} \
	}

#define NV_FLOW_VIRTUAL_TO_REAL_LINEAR(name, blockTableSRV, params) \
	float3 name(float3 vidx) \
	{ \
		if(params.isVTR.x != 0) \
		{ \
			return vidx; \
		} \
		else \
		{ \
			float3 vBlockIdxf = params.blockDimInv.xyz * vidx; \
			int3 vBlockIdx = int3(floor(vBlockIdxf)); \
			int3 rBlockIdx = NvFlow_tableVal_to_coord(blockTableSRV[vBlockIdx]); \
			float3 rBlockIdxf = float3(rBlockIdx); \
			float3 ridx = float3(params.linearBlockDim.xyz * rBlockIdx) + float3(params.blockDim.xyz) * (vBlockIdxf - float3(vBlockIdx)) + float3(params.linearBlockOffset.xyz); \
			return ridx; \
		} \
	}

#endif

//! Parameters for shaders using the point format (no linear interpolation)
struct NvFlowShaderPointParams
{
	NvFlowUint4 isVTR;
	NvFlowUint4 blockDim;
	NvFlowUint4 blockDimBits;
	NvFlowUint4 poolGridDim;
	NvFlowUint4 gridDim;
};

//! Parameters for shaders using the linear format (linear interpolation)
struct NvFlowShaderLinearParams
{
	NvFlowUint4 isVTR;
	NvFlowUint4 blockDim;
	NvFlowUint4 blockDimBits;
	NvFlowUint4 poolGridDim;
	NvFlowUint4 gridDim;

	NvFlowFloat4 blockDimInv;
	NvFlowUint4 linearBlockDim;
	NvFlowUint4 linearBlockOffset;
	NvFlowFloat4 dimInv;
	NvFlowFloat4 vdim;
	NvFlowFloat4 vdimInv;
};

///@}

#endif