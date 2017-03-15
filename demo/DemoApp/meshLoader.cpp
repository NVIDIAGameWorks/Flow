/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <SDL.h>

#include "loader.h"

#include "mesh.h"
#include "meshInterop.h"

namespace
{
	ModuleLoader<16u, SDL_LoadObject, SDL_UnloadObject, SDL_LoadFunction> g_loader;
}

void loadMesh(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	g_loader.loadModule(moduleName);
}

void unloadMesh()
{
	g_loader.unloadModule();
}

MeshContext* MeshContextCreate(const MeshContextDesc* desc)
{
	return g_loader.function<0>(MeshContextCreate, "MeshContextCreate", desc);
}

void MeshContextUpdate(MeshContext* context, const MeshContextDesc* desc)
{
	return g_loader.function<1>(MeshContextUpdate, "MeshContextUpdate", context, desc);
}

void MeshContextRelease(MeshContext* context)
{
	return g_loader.function<2>(MeshContextRelease, "MeshContextRelease", context);
}

MeshIndexBuffer* MeshIndexBufferCreate(MeshContext* context, MeshUint* indices, MeshUint numIndices)
{
	return g_loader.function<3>(MeshIndexBufferCreate, "MeshIndexBufferCreate", context, indices, numIndices);
}

void MeshIndexBufferRelease(MeshIndexBuffer* buffer)
{
	return g_loader.function<4>(MeshIndexBufferRelease, "MeshIndexBufferRelease", buffer);
}

MeshVertexBuffer* MeshVertexBufferCreate(MeshContext* context, MeshVertex* vertices, MeshUint numVertices)
{
	return g_loader.function<5>(MeshVertexBufferCreate, "MeshVertexBufferCreate", context, vertices, numVertices);
}

void MeshVertexBufferRelease(MeshVertexBuffer* buffer)
{
	return g_loader.function<6>(MeshVertexBufferRelease, "MeshVertexBufferRelease", buffer);
}

void MeshContextDraw(MeshContext* context, const MeshContextDrawParams* params)
{
	return g_loader.function<7>(MeshContextDraw, "MeshContextDraw", context, params);
}

MeshContext* MeshInteropContextCreate(AppGraphCtx* appctx)
{
	return g_loader.function<8>(MeshInteropContextCreate, "MeshInteropContextCreate", appctx);
}

void MeshInteropContextUpdate(MeshContext* context, AppGraphCtx* appctx)
{
	return g_loader.function<9>(MeshInteropContextUpdate, "MeshInteropContextUpdate", context, appctx);
}