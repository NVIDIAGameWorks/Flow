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

#include "imguiGraph.h"
#include "imguiInterop.h"

namespace
{
	ModuleLoader<24u, SDL_LoadObject, SDL_UnloadObject, SDL_LoadFunction> g_loader;
}

void loadImgui(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	g_loader.loadModule(moduleName);
}

void unloadImgui()
{
	g_loader.unloadModule();
}

// Below are the functions that must be implemented per graphics API

void imguiGraphContextInit(const ImguiGraphDesc* desc)
{
	return g_loader.function<0>(imguiGraphContextInit, "imguiGraphContextInit", desc);
}

void imguiGraphContextUpdate(const ImguiGraphDesc* desc)
{
	return g_loader.function<1>(imguiGraphContextUpdate, "imguiGraphContextUpdate", desc);
}

void imguiGraphContextDestroy()
{
	return g_loader.function<2>(imguiGraphContextDestroy, "imguiGraphContextDestroy");
}

void imguiGraphRecordBegin()
{
	return g_loader.function<3>(imguiGraphRecordBegin, "imguiGraphRecordBegin");
}

void imguiGraphRecordEnd()
{
	return g_loader.function<4>(imguiGraphRecordEnd, "imguiGraphRecordEnd");
}

void imguiGraphVertex2f(float x, float y)
{
	return g_loader.function<5>(imguiGraphVertex2f, "imguiGraphVertex2f", x, y);
}

void imguiGraphVertex2fv(const float* v)
{
	return g_loader.function<6>(imguiGraphVertex2fv, "imguiGraphVertex2fv", v);
}

void imguiGraphTexCoord2f(float u, float v)
{
	return g_loader.function<7>(imguiGraphTexCoord2f, "imguiGraphTexCoord2f", u, v);
}

void imguiGraphColor4ub(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	return g_loader.function<8>(imguiGraphColor4ub, "imguiGraphColor4ub", red, green, blue, alpha);
}

void imguiGraphColor4ubv(const uint8_t* v)
{
	return g_loader.function<9>(imguiGraphColor4ubv, "imguiGraphColor4ubv", v);
}

void imguiGraphFontTextureEnable()
{
	return g_loader.function<10>(imguiGraphFontTextureEnable, "imguiGraphFontTextureEnable");
}

void imguiGraphFontTextureDisable()
{
	return g_loader.function<11>(imguiGraphFontTextureDisable, "imguiGraphFontTextureDisable");
}

void imguiGraphEnableScissor(int x, int y, int width, int height)
{
	return g_loader.function<12>(imguiGraphEnableScissor, "imguiGraphEnableScissor", x, y, width, height);
}

void imguiGraphDisableScissor()
{
	return g_loader.function<13>(imguiGraphDisableScissor, "imguiGraphDisableScissor");
}

void imguiGraphFontTextureInit(unsigned char* data)
{
	return g_loader.function<14>(imguiGraphFontTextureInit, "imguiGraphFontTextureInit",data);
}

void imguiGraphFontTextureRelease()
{
	return g_loader.function<15>(imguiGraphFontTextureRelease, "imguiGraphFontTextureRelease");
}

bool imguiInteropGraphInit(imguiGraphInit_t func, const char* fontpath, AppGraphCtx* appctx)
{
	return g_loader.function<16>(imguiInteropGraphInit, "imguiInteropGraphInit", func, fontpath, appctx);
}

void imguiInteropGraphUpdate(imguiGraphUpdate_t func, AppGraphCtx* appctx)
{
	return g_loader.function<17>(imguiInteropGraphUpdate, "imguiInteropGraphUpdate", func, appctx);
}