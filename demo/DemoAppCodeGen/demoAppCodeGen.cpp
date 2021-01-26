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

#include <stdio.h>
#include <string.h>
#include <cstdlib>

#include "loaderCodeGen.h"
#include "functionDefinitionExtract.h"

LoaderType gLoaderType = eLoaderTypeDynamicLink;

void genLoaderAppGraphCtx()
{
	const unsigned int numFunctions = 12u;

	const char* functionDefinitions[numFunctions] = {
		"AppGraphCtx* AppGraphCtxCreate(int deviceID);",
		"bool AppGraphCtxUpdateSize(AppGraphCtx* context, SDL_Window* window, bool fullscreen);",
		"void AppGraphCtxReleaseRenderTarget(AppGraphCtx* context);",
		"void AppGraphCtxRelease(AppGraphCtx* context);",
		"void AppGraphCtxFrameStart(AppGraphCtx* context, AppGraphColor clearColor);",
		"void AppGraphCtxFramePresent(AppGraphCtx* context, bool fullsync);",
		"void AppGraphCtxWaitForFrames(AppGraphCtx* context, unsigned int maxFramesInFlight);",
		"void AppGraphCtxProfileEnable(AppGraphCtx* context, bool enabled);",
		"void AppGraphCtxProfileBegin(AppGraphCtx* context, const char* label);",
		"void AppGraphCtxProfileEnd(AppGraphCtx* context, const char* label);",
		"bool AppGraphCtxProfileGet(AppGraphCtx* context, const char** plabel, float* cpuTime, float* gpuTime, int index);",
		"size_t AppGraphCtxDedicatedVideoMemory(AppGraphCtx* context);"
	};

	StrHeap strHeap = allocateStrHeap(functionDefinitions, numFunctions);

	Function functions[numFunctions];
	for (unsigned int functionIdx = 0u; functionIdx < numFunctions; functionIdx++)
	{
		functions[functionIdx] = genFunction(&strHeap, functionDefinitions[functionIdx]);
	}

	GenerateCodeParams genCodeParams = {};
	genCodeParams.loaderType = gLoaderType;
	genCodeParams.file = nullptr;
	genCodeParams.functions = functions;
	genCodeParams.numFunctions = numFunctions;
	genCodeParams.filenameTmp = "appGraphCtxLoaderGenerated.tmp.h";
	genCodeParams.filenameDst = "appGraphCtxLoaderGenerated.h";
	genCodeParams.moduleNameUpperCase = "AppGraphCtx";
	genCodeParams.moduleNameLowerCase = "appGraphCtx";
	genCodeParams.instName = "gAppGraphCtxLoader";
	genCodeParams.apiMarker = "APP_GRAPH_CTX_API";

	fopen_s(&genCodeParams.file, genCodeParams.filenameTmp, "w");

	generateCode(&genCodeParams);

	fclose(genCodeParams.file);

	freeStrHeap(&strHeap);

	fileDiffAndWriteIfModified(&genCodeParams);
}

void genLoaderComputeContext()
{
	const unsigned int numFunctions = 23u;

	const char* functionDefinitions[numFunctions] = {
		"ComputeContext* ComputeContextCreate(ComputeContextDesc* desc);",
		"void ComputeContextUpdate(ComputeContext* context, ComputeContextDesc* desc);",
		"void ComputeContextRelease(ComputeContext* context);",
		"ComputeShader* ComputeShaderCreate(ComputeContext* context, const ComputeShaderDesc* desc);",
		"void ComputeShaderRelease(ComputeShader* shader);",
		"ComputeConstantBuffer* ComputeConstantBufferCreate(ComputeContext* context, const ComputeConstantBufferDesc* desc);",
		"void ComputeConstantBufferRelease(ComputeConstantBuffer* constantBuffer);",
		"void* ComputeConstantBufferMap(ComputeContext* context, ComputeConstantBuffer* constantBuffer);",
		"void ComputeConstantBufferUnmap(ComputeContext* context, ComputeConstantBuffer* constantBuffer);",
		"ComputeResource* ComputeResourceCreate(ComputeContext* context, const ComputeResourceDesc* desc);",
		"void ComputeResourceUpdate(ComputeContext* context, ComputeResource* resource, const ComputeResourceDesc* desc);",
		"void ComputeResourceRelease(ComputeResource* resource);",
		"ComputeResourceRW* ComputeResourceRWCreate(ComputeContext* context, const ComputeResourceRWDesc* desc);",
		"void ComputeResourceRWUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, const ComputeResourceRWDesc* desc);",
		"void ComputeResourceRWRelease(ComputeResourceRW* resourceRW);",
		"ComputeResource* ComputeResourceRWGetResource(ComputeResourceRW* resourceRW);",
		"void ComputeContextDispatch(ComputeContext* context, const ComputeDispatchParams* params);",
		"ComputeContext* ComputeContextNvFlowContextCreate(NvFlowContext* flowContext);",
		"void ComputeContextNvFlowContextUpdate(ComputeContext* computeContext, NvFlowContext* flowContext);",
		"ComputeResource* ComputeResourceNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResource* flowResource);",
		"void ComputeResourceNvFlowUpdate(ComputeContext* context, ComputeResource* resource, NvFlowContext* flowContext, NvFlowResource* flowResource);",
		"ComputeResourceRW* ComputeResourceRWNvFlowCreate(ComputeContext* context, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW);",
		"void ComputeResourceRWNvFlowUpdate(ComputeContext* context, ComputeResourceRW* resourceRW, NvFlowContext* flowContext, NvFlowResourceRW* flowResourceRW);"
	};

	StrHeap strHeap = allocateStrHeap(functionDefinitions, numFunctions);

	Function functions[numFunctions];
	for (unsigned int functionIdx = 0u; functionIdx < numFunctions; functionIdx++)
	{
		functions[functionIdx] = genFunction(&strHeap, functionDefinitions[functionIdx]);
	}

	GenerateCodeParams genCodeParams = {};
	genCodeParams.loaderType = gLoaderType;
	genCodeParams.file = nullptr;
	genCodeParams.functions = functions;
	genCodeParams.numFunctions = numFunctions;
	genCodeParams.filenameTmp = "computeContextLoaderGenerated.tmp.h";
	genCodeParams.filenameDst = "computeContextLoaderGenerated.h";
	genCodeParams.moduleNameUpperCase = "ComputeContext";
	genCodeParams.moduleNameLowerCase = "computeContext";
	genCodeParams.instName = "gComputeContextLoader";
	genCodeParams.apiMarker = "COMPUTE_API";

	fopen_s(&genCodeParams.file, genCodeParams.filenameTmp, "w");

	generateCode(&genCodeParams);

	fclose(genCodeParams.file);

	freeStrHeap(&strHeap);

	fileDiffAndWriteIfModified(&genCodeParams);
}

void genLoaderImgui()
{
	const unsigned int numFunctions = 18u;

	const char* functionDefinitions[numFunctions] = {
		"void imguiGraphContextInit(const ImguiGraphDesc* desc);",
		"void imguiGraphContextUpdate(const ImguiGraphDesc* desc);",
		"void imguiGraphContextDestroy();",
		"void imguiGraphRecordBegin();",
		"void imguiGraphRecordEnd();",
		"void imguiGraphVertex2f(float x, float y);",
		"void imguiGraphVertex2fv(const float* v);",
		"void imguiGraphTexCoord2f(float u, float v);",
		"void imguiGraphColor4ub(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);",
		"void imguiGraphColor4ubv(const uint8_t* v);",
		"void imguiGraphFontTextureEnable();",
		"void imguiGraphFontTextureDisable();",
		"void imguiGraphEnableScissor(int x, int y, int width, int height);",
		"void imguiGraphDisableScissor();",
		"void imguiGraphFontTextureInit(unsigned char* data);",
		"void imguiGraphFontTextureRelease();",
		"bool imguiInteropGraphInit(imguiGraphInit_t func, const char* fontpath, AppGraphCtx* appctx);",
		"void imguiInteropGraphUpdate(imguiGraphUpdate_t func, AppGraphCtx* appctx);"
	};

	StrHeap strHeap = allocateStrHeap(functionDefinitions, numFunctions);

	Function functions[numFunctions];
	for (unsigned int functionIdx = 0u; functionIdx < numFunctions; functionIdx++)
	{
		functions[functionIdx] = genFunction(&strHeap, functionDefinitions[functionIdx]);
	}

	GenerateCodeParams genCodeParams = {};
	genCodeParams.loaderType = gLoaderType;
	genCodeParams.file = nullptr;
	genCodeParams.functions = functions;
	genCodeParams.numFunctions = numFunctions;
	genCodeParams.filenameTmp = "imguiGraphLoaderGenerated.tmp.h";
	genCodeParams.filenameDst = "imguiGraphLoaderGenerated.h";
	genCodeParams.moduleNameUpperCase = "Imgui";
	genCodeParams.moduleNameLowerCase = "imgui";
	genCodeParams.instName = "gImguiLoader";
	genCodeParams.apiMarker = "IMGUI_GRAPH_API";

	fopen_s(&genCodeParams.file, genCodeParams.filenameTmp, "w");

	generateCode(&genCodeParams);

	fclose(genCodeParams.file);

	freeStrHeap(&strHeap);

	fileDiffAndWriteIfModified(&genCodeParams);
}

void genLoaderMesh()
{
	const unsigned int numFunctions = 10u;

	const char* functionDefinitions[numFunctions] = {
		"MeshContext* MeshContextCreate(const MeshContextDesc* desc);",
		"void MeshContextUpdate(MeshContext* context, const MeshContextDesc* desc);",
		"void MeshContextRelease(MeshContext* context);",
		"MeshIndexBuffer* MeshIndexBufferCreate(MeshContext* context, MeshUint* indices, MeshUint numIndices);",
		"void MeshIndexBufferRelease(MeshIndexBuffer* buffer);",
		"MeshVertexBuffer* MeshVertexBufferCreate(MeshContext* context, MeshVertex* vertices, MeshUint numVertices);",
		"void MeshVertexBufferRelease(MeshVertexBuffer* buffer);",
		"void MeshContextDraw(MeshContext* context, const MeshContextDrawParams* params);",
		"MeshContext* MeshInteropContextCreate(AppGraphCtx* appctx);",
		"void MeshInteropContextUpdate(MeshContext* context, AppGraphCtx* appctx);"
	};

	StrHeap strHeap = allocateStrHeap(functionDefinitions, numFunctions);

	Function functions[numFunctions];
	for (unsigned int functionIdx = 0u; functionIdx < numFunctions; functionIdx++)
	{
		functions[functionIdx] = genFunction(&strHeap, functionDefinitions[functionIdx]);
	}

	GenerateCodeParams genCodeParams = {};
	genCodeParams.loaderType = gLoaderType;
	genCodeParams.file = nullptr;
	genCodeParams.functions = functions;
	genCodeParams.numFunctions = numFunctions;
	genCodeParams.filenameTmp = "meshLoaderGenerated.tmp.h";
	genCodeParams.filenameDst = "meshLoaderGenerated.h";
	genCodeParams.moduleNameUpperCase = "Mesh";
	genCodeParams.moduleNameLowerCase = "mesh";
	genCodeParams.instName = "gMeshLoader";
	genCodeParams.apiMarker = "MESH_API";

	fopen_s(&genCodeParams.file, genCodeParams.filenameTmp, "w");

	generateCode(&genCodeParams);

	fclose(genCodeParams.file);

	freeStrHeap(&strHeap);

	fileDiffAndWriteIfModified(&genCodeParams);
}

void genNvFlowInteropLoader()
{
	const unsigned int numFunctions = 6u;

	const char* functionDefinitions[numFunctions] = {
		"NvFlowContext* NvFlowInteropCreateContext(AppGraphCtx* appctx);",
		"NvFlowDepthStencilView* NvFlowInteropCreateDepthStencilView(AppGraphCtx* appctx, NvFlowContext* flowctx);",
		"NvFlowRenderTargetView* NvFlowInteropCreateRenderTargetView(AppGraphCtx* appctx, NvFlowContext* flowctx);",
		"void NvFlowInteropUpdateContext(NvFlowContext* context, AppGraphCtx* appctx);",
		"void NvFlowInteropUpdateDepthStencilView(NvFlowDepthStencilView* view, AppGraphCtx* appctx, NvFlowContext* flowctx);",
		"void NvFlowInteropUpdateRenderTargetView(NvFlowRenderTargetView* view, AppGraphCtx* appctx, NvFlowContext* flowctx);"
	};

	StrHeap strHeap = allocateStrHeap(functionDefinitions, numFunctions);

	Function functions[numFunctions];
	for (unsigned int functionIdx = 0u; functionIdx < numFunctions; functionIdx++)
	{
		functions[functionIdx] = genFunction(&strHeap, functionDefinitions[functionIdx]);
	}

	GenerateCodeParams genCodeParams = {};
	genCodeParams.loaderType = gLoaderType;
	genCodeParams.file = nullptr;
	genCodeParams.functions = functions;
	genCodeParams.numFunctions = numFunctions;
	genCodeParams.filenameTmp = "NvFlowInteropLoaderGenerated.tmp.h";
	genCodeParams.filenameDst = "NvFlowInteropLoaderGenerated.h";
	genCodeParams.moduleNameUpperCase = "NvFlowInterop";
	genCodeParams.moduleNameLowerCase = "nvFlowInterop";
	genCodeParams.instName = "gNvFlowInteropLoader";
	genCodeParams.apiMarker = "NV_FLOW_API";

	fopen_s(&genCodeParams.file, genCodeParams.filenameTmp, "w");

	generateCode(&genCodeParams);

	fclose(genCodeParams.file);

	freeStrHeap(&strHeap);

	fileDiffAndWriteIfModified(&genCodeParams);
}

int main(int argc, char** argv)
{
	genLoaderAppGraphCtx();

	genLoaderComputeContext();

	genLoaderImgui();

	genLoaderMesh();

	genNvFlowInteropLoader();

	return 0;
}