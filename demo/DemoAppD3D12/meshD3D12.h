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

struct MeshContextDescD3D12
{
	ID3D12Device* device;
	ID3D12GraphicsCommandList* commandList;
};

inline const MeshContextDescD3D12* cast_to_MeshContextDescD3D12(const MeshContextDesc* desc)
{
	return (const MeshContextDescD3D12*)(desc);
}

inline MeshContextDesc* cast_from_MeshContextDescD3D12(MeshContextDescD3D12* desc)
{
	return (MeshContextDesc*)(desc);
}

MESH_API MeshContext* MeshContextCreateD3D12(const MeshContextDesc* desc);

MESH_API void MeshContextUpdateD3D12(MeshContext* context, const MeshContextDesc* desc);

MESH_API void MeshContextReleaseD3D12(MeshContext* context);


MESH_API MeshIndexBuffer* MeshIndexBufferCreateD3D12(MeshContext* context, MeshUint* indices, MeshUint numIndices);

MESH_API void MeshIndexBufferReleaseD3D12(MeshIndexBuffer* buffer);

MESH_API MeshVertexBuffer* MeshVertexBufferCreateD3D12(MeshContext* context, MeshVertex* vertices, MeshUint numVertices);

MESH_API void MeshVertexBufferReleaseD3D12(MeshVertexBuffer* buffer);

MESH_API void MeshContextDrawD3D12(MeshContext* context, const MeshContextDrawParams* params);