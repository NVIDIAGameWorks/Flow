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

#include "../DemoApp/mesh.h"

struct MeshContextDescD3D11
{
	ID3D11Device* device;
	ID3D11DeviceContext* context;
};

inline const MeshContextDescD3D11* cast_to_MeshContextDescD3D11(const MeshContextDesc* desc)
{
	return (const MeshContextDescD3D11*)(desc);
}

inline MeshContextDesc* cast_from_MeshContextDescD3D11(MeshContextDescD3D11* desc)
{
	return (MeshContextDesc*)(desc);
}

MESH_API MeshContext* MeshContextCreateD3D11(const MeshContextDesc* desc);

MESH_API void MeshContextUpdateD3D11(MeshContext* context, const MeshContextDesc* desc);

MESH_API void MeshContextReleaseD3D11(MeshContext* context);


MESH_API MeshIndexBuffer* MeshIndexBufferCreateD3D11(MeshContext* context, MeshUint* indices, MeshUint numIndices);

MESH_API void MeshIndexBufferReleaseD3D11(MeshIndexBuffer* buffer);

MESH_API MeshVertexBuffer* MeshVertexBufferCreateD3D11(MeshContext* context, MeshVertex* vertices, MeshUint numVertices);

MESH_API void MeshVertexBufferReleaseD3D11(MeshVertexBuffer* buffer);

MESH_API void MeshContextDrawD3D11(MeshContext* context, const MeshContextDrawParams* params);