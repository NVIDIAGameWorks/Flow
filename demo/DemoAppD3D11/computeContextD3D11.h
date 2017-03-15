/*
* Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#pragma once

#include "../DemoApp/computeContext.h"

struct ComputeContextDesc
{
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
};

struct ComputeResourceDesc
{
	ID3D11ShaderResourceView* srv;
};

struct ComputeResourceRWDesc
{
	ComputeResourceDesc resource;
	ID3D11UnorderedAccessView* uav;
};