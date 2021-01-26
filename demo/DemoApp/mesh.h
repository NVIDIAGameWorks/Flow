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