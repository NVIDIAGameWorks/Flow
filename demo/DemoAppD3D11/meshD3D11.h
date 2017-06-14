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