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

#include <DirectXMath.h>

#define MESH_API extern "C" __declspec(dllexport)

/// ****************** Mesh Context Public *******************************

typedef unsigned int MeshUint;
typedef unsigned char MeshUint8;

struct MeshContext;

struct MeshContextDesc;

MESH_API MeshContext* MeshContextCreate(const MeshContextDesc* desc);

MESH_API void MeshContextUpdate(MeshContext* context, const MeshContextDesc* desc);

MESH_API void MeshContextRelease(MeshContext* context);

/// ****************** Mesh Interface Public **********************

struct Mesh;

enum MeshRenderMode
{
	MESH_RENDER_SOLID = 0
};

struct MeshDrawParams
{
	MeshUint renderMode;
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX model;
};

struct MeshData
{
	MeshUint numVertices;
	float* positions;
	MeshUint positionStride;
	float* normals;
	MeshUint normalStride;

	MeshUint numIndices;
	MeshUint* indices;

	float boundsMin[3];
	float boundsMax[3];
};

MESH_API Mesh* MeshCreate(MeshContext* context);

MESH_API void MeshLoadFromFile(Mesh* mesh, const char* filename);

MESH_API void MeshGetData(Mesh* mesh, MeshData* data);

MESH_API void MeshDraw(Mesh* mesh, const MeshDrawParams* params);

MESH_API void MeshRelease(Mesh* mesh);

/// ****************** Mesh Context Implementation *******************************

struct MeshVertex
{
	float x, y, z;
	float nx, ny, nz;
};

struct MeshIndexBuffer;
struct MeshVertexBuffer;

MESH_API MeshIndexBuffer* MeshIndexBufferCreate(MeshContext* context, MeshUint* indices, MeshUint numIndices);

MESH_API void MeshIndexBufferRelease(MeshIndexBuffer* buffer);

MESH_API MeshVertexBuffer* MeshVertexBufferCreate(MeshContext* context, MeshVertex* vertices, MeshUint numVertices);

MESH_API void MeshVertexBufferRelease(MeshVertexBuffer* buffer);

struct MeshContextDrawParams
{
	const MeshDrawParams* params;
	MeshIndexBuffer* indexBuffer;
	MeshVertexBuffer* vertexBuffer;
};

MESH_API void MeshContextDraw(MeshContext* context, const MeshContextDrawParams* params);