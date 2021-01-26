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

typedef MeshContext*  (*MeshContextCreate_ptr_t)(const MeshContextDesc*  desc);
typedef void  (*MeshContextUpdate_ptr_t)(MeshContext*  context, const MeshContextDesc*  desc);
typedef void  (*MeshContextRelease_ptr_t)(MeshContext*  context);
typedef MeshIndexBuffer*  (*MeshIndexBufferCreate_ptr_t)(MeshContext*  context, MeshUint*  indices, MeshUint  numIndices);
typedef void  (*MeshIndexBufferRelease_ptr_t)(MeshIndexBuffer*  buffer);
typedef MeshVertexBuffer*  (*MeshVertexBufferCreate_ptr_t)(MeshContext*  context, MeshVertex*  vertices, MeshUint  numVertices);
typedef void  (*MeshVertexBufferRelease_ptr_t)(MeshVertexBuffer*  buffer);
typedef void  (*MeshContextDraw_ptr_t)(MeshContext*  context, const MeshContextDrawParams*  params);
typedef MeshContext*  (*MeshInteropContextCreate_ptr_t)(AppGraphCtx*  appctx);
typedef void  (*MeshInteropContextUpdate_ptr_t)(MeshContext*  context, AppGraphCtx*  appctx);

struct MeshLoader 
{ 
	void* module = nullptr; 
	const char* suffix = ""; 
	char buf[1024u]; 

	MeshContextCreate_ptr_t MeshContextCreate_ptr;
	MeshContextUpdate_ptr_t MeshContextUpdate_ptr;
	MeshContextRelease_ptr_t MeshContextRelease_ptr;
	MeshIndexBufferCreate_ptr_t MeshIndexBufferCreate_ptr;
	MeshIndexBufferRelease_ptr_t MeshIndexBufferRelease_ptr;
	MeshVertexBufferCreate_ptr_t MeshVertexBufferCreate_ptr;
	MeshVertexBufferRelease_ptr_t MeshVertexBufferRelease_ptr;
	MeshContextDraw_ptr_t MeshContextDraw_ptr;
	MeshInteropContextCreate_ptr_t MeshInteropContextCreate_ptr;
	MeshInteropContextUpdate_ptr_t MeshInteropContextUpdate_ptr;

}gMeshLoader; 

MeshContext*  MeshContextCreate(const MeshContextDesc*  desc)
{
	return gMeshLoader.MeshContextCreate_ptr(desc);
}

void  MeshContextUpdate(MeshContext*  context, const MeshContextDesc*  desc)
{
	return gMeshLoader.MeshContextUpdate_ptr(context, desc);
}

void  MeshContextRelease(MeshContext*  context)
{
	return gMeshLoader.MeshContextRelease_ptr(context);
}

MeshIndexBuffer*  MeshIndexBufferCreate(MeshContext*  context, MeshUint*  indices, MeshUint  numIndices)
{
	return gMeshLoader.MeshIndexBufferCreate_ptr(context, indices, numIndices);
}

void  MeshIndexBufferRelease(MeshIndexBuffer*  buffer)
{
	return gMeshLoader.MeshIndexBufferRelease_ptr(buffer);
}

MeshVertexBuffer*  MeshVertexBufferCreate(MeshContext*  context, MeshVertex*  vertices, MeshUint  numVertices)
{
	return gMeshLoader.MeshVertexBufferCreate_ptr(context, vertices, numVertices);
}

void  MeshVertexBufferRelease(MeshVertexBuffer*  buffer)
{
	return gMeshLoader.MeshVertexBufferRelease_ptr(buffer);
}

void  MeshContextDraw(MeshContext*  context, const MeshContextDrawParams*  params)
{
	return gMeshLoader.MeshContextDraw_ptr(context, params);
}

MeshContext*  MeshInteropContextCreate(AppGraphCtx*  appctx)
{
	return gMeshLoader.MeshInteropContextCreate_ptr(appctx);
}

void  MeshInteropContextUpdate(MeshContext*  context, AppGraphCtx*  appctx)
{
	return gMeshLoader.MeshInteropContextUpdate_ptr(context, appctx);
}

void* meshLoaderLoadFunction(MeshLoader* inst, const char* name)
{
	snprintf(inst->buf, 1024u, "%s%s", name, inst->suffix);

	return SDL_LoadFunction(inst->module, inst->buf);
}

void loadMesh(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	gMeshLoader.suffix = demoAppBackendSuffix(type);

	gMeshLoader.module = SDL_LoadObject(moduleName);

	gMeshLoader.MeshContextCreate_ptr = (MeshContextCreate_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshContextCreate"));
	gMeshLoader.MeshContextUpdate_ptr = (MeshContextUpdate_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshContextUpdate"));
	gMeshLoader.MeshContextRelease_ptr = (MeshContextRelease_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshContextRelease"));
	gMeshLoader.MeshIndexBufferCreate_ptr = (MeshIndexBufferCreate_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshIndexBufferCreate"));
	gMeshLoader.MeshIndexBufferRelease_ptr = (MeshIndexBufferRelease_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshIndexBufferRelease"));
	gMeshLoader.MeshVertexBufferCreate_ptr = (MeshVertexBufferCreate_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshVertexBufferCreate"));
	gMeshLoader.MeshVertexBufferRelease_ptr = (MeshVertexBufferRelease_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshVertexBufferRelease"));
	gMeshLoader.MeshContextDraw_ptr = (MeshContextDraw_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshContextDraw"));
	gMeshLoader.MeshInteropContextCreate_ptr = (MeshInteropContextCreate_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshInteropContextCreate"));
	gMeshLoader.MeshInteropContextUpdate_ptr = (MeshInteropContextUpdate_ptr_t)(meshLoaderLoadFunction(&gMeshLoader, "MeshInteropContextUpdate"));
}

void unloadMesh()
{
	gMeshLoader.MeshContextCreate_ptr = nullptr;
	gMeshLoader.MeshContextUpdate_ptr = nullptr;
	gMeshLoader.MeshContextRelease_ptr = nullptr;
	gMeshLoader.MeshIndexBufferCreate_ptr = nullptr;
	gMeshLoader.MeshIndexBufferRelease_ptr = nullptr;
	gMeshLoader.MeshVertexBufferCreate_ptr = nullptr;
	gMeshLoader.MeshVertexBufferRelease_ptr = nullptr;
	gMeshLoader.MeshContextDraw_ptr = nullptr;
	gMeshLoader.MeshInteropContextCreate_ptr = nullptr;
	gMeshLoader.MeshInteropContextUpdate_ptr = nullptr;

	SDL_UnloadObject(gMeshLoader.module);
}
