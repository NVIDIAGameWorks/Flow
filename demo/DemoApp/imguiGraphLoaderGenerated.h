/*
* Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

typedef void  (*imguiGraphContextInit_ptr_t)(const ImguiGraphDesc*  desc);
typedef void  (*imguiGraphContextUpdate_ptr_t)(const ImguiGraphDesc*  desc);
typedef void  (*imguiGraphContextDestroy_ptr_t)();
typedef void  (*imguiGraphRecordBegin_ptr_t)();
typedef void  (*imguiGraphRecordEnd_ptr_t)();
typedef void  (*imguiGraphVertex2f_ptr_t)(float  x, float  y);
typedef void  (*imguiGraphVertex2fv_ptr_t)(const float*  v);
typedef void  (*imguiGraphTexCoord2f_ptr_t)(float  u, float  v);
typedef void  (*imguiGraphColor4ub_ptr_t)(uint8_t  red, uint8_t  green, uint8_t  blue, uint8_t  alpha);
typedef void  (*imguiGraphColor4ubv_ptr_t)(const uint8_t*  v);
typedef void  (*imguiGraphFontTextureEnable_ptr_t)();
typedef void  (*imguiGraphFontTextureDisable_ptr_t)();
typedef void  (*imguiGraphEnableScissor_ptr_t)(int  x, int  y, int  width, int  height);
typedef void  (*imguiGraphDisableScissor_ptr_t)();
typedef void  (*imguiGraphFontTextureInit_ptr_t)(unsigned char*  data);
typedef void  (*imguiGraphFontTextureRelease_ptr_t)();
typedef bool  (*imguiInteropGraphInit_ptr_t)(imguiGraphInit_t  func, const char*  fontpath, AppGraphCtx*  appctx);
typedef void  (*imguiInteropGraphUpdate_ptr_t)(imguiGraphUpdate_t  func, AppGraphCtx*  appctx);

struct ImguiLoader 
{ 
	void* module = nullptr; 
	const char* suffix = ""; 
	char buf[1024u]; 

	imguiGraphContextInit_ptr_t imguiGraphContextInit_ptr;
	imguiGraphContextUpdate_ptr_t imguiGraphContextUpdate_ptr;
	imguiGraphContextDestroy_ptr_t imguiGraphContextDestroy_ptr;
	imguiGraphRecordBegin_ptr_t imguiGraphRecordBegin_ptr;
	imguiGraphRecordEnd_ptr_t imguiGraphRecordEnd_ptr;
	imguiGraphVertex2f_ptr_t imguiGraphVertex2f_ptr;
	imguiGraphVertex2fv_ptr_t imguiGraphVertex2fv_ptr;
	imguiGraphTexCoord2f_ptr_t imguiGraphTexCoord2f_ptr;
	imguiGraphColor4ub_ptr_t imguiGraphColor4ub_ptr;
	imguiGraphColor4ubv_ptr_t imguiGraphColor4ubv_ptr;
	imguiGraphFontTextureEnable_ptr_t imguiGraphFontTextureEnable_ptr;
	imguiGraphFontTextureDisable_ptr_t imguiGraphFontTextureDisable_ptr;
	imguiGraphEnableScissor_ptr_t imguiGraphEnableScissor_ptr;
	imguiGraphDisableScissor_ptr_t imguiGraphDisableScissor_ptr;
	imguiGraphFontTextureInit_ptr_t imguiGraphFontTextureInit_ptr;
	imguiGraphFontTextureRelease_ptr_t imguiGraphFontTextureRelease_ptr;
	imguiInteropGraphInit_ptr_t imguiInteropGraphInit_ptr;
	imguiInteropGraphUpdate_ptr_t imguiInteropGraphUpdate_ptr;

}gImguiLoader; 

void  imguiGraphContextInit(const ImguiGraphDesc*  desc)
{
	return gImguiLoader.imguiGraphContextInit_ptr(desc);
}

void  imguiGraphContextUpdate(const ImguiGraphDesc*  desc)
{
	return gImguiLoader.imguiGraphContextUpdate_ptr(desc);
}

void  imguiGraphContextDestroy()
{
	return gImguiLoader.imguiGraphContextDestroy_ptr();
}

void  imguiGraphRecordBegin()
{
	return gImguiLoader.imguiGraphRecordBegin_ptr();
}

void  imguiGraphRecordEnd()
{
	return gImguiLoader.imguiGraphRecordEnd_ptr();
}

void  imguiGraphVertex2f(float  x, float  y)
{
	return gImguiLoader.imguiGraphVertex2f_ptr(x, y);
}

void  imguiGraphVertex2fv(const float*  v)
{
	return gImguiLoader.imguiGraphVertex2fv_ptr(v);
}

void  imguiGraphTexCoord2f(float  u, float  v)
{
	return gImguiLoader.imguiGraphTexCoord2f_ptr(u, v);
}

void  imguiGraphColor4ub(uint8_t  red, uint8_t  green, uint8_t  blue, uint8_t  alpha)
{
	return gImguiLoader.imguiGraphColor4ub_ptr(red, green, blue, alpha);
}

void  imguiGraphColor4ubv(const uint8_t*  v)
{
	return gImguiLoader.imguiGraphColor4ubv_ptr(v);
}

void  imguiGraphFontTextureEnable()
{
	return gImguiLoader.imguiGraphFontTextureEnable_ptr();
}

void  imguiGraphFontTextureDisable()
{
	return gImguiLoader.imguiGraphFontTextureDisable_ptr();
}

void  imguiGraphEnableScissor(int  x, int  y, int  width, int  height)
{
	return gImguiLoader.imguiGraphEnableScissor_ptr(x, y, width, height);
}

void  imguiGraphDisableScissor()
{
	return gImguiLoader.imguiGraphDisableScissor_ptr();
}

void  imguiGraphFontTextureInit(unsigned char*  data)
{
	return gImguiLoader.imguiGraphFontTextureInit_ptr(data);
}

void  imguiGraphFontTextureRelease()
{
	return gImguiLoader.imguiGraphFontTextureRelease_ptr();
}

bool  imguiInteropGraphInit(imguiGraphInit_t  func, const char*  fontpath, AppGraphCtx*  appctx)
{
	return gImguiLoader.imguiInteropGraphInit_ptr(func, fontpath, appctx);
}

void  imguiInteropGraphUpdate(imguiGraphUpdate_t  func, AppGraphCtx*  appctx)
{
	return gImguiLoader.imguiInteropGraphUpdate_ptr(func, appctx);
}

void* imguiLoaderLoadFunction(ImguiLoader* inst, const char* name)
{
	snprintf(inst->buf, 1024u, "%s%s", name, inst->suffix);

	return SDL_LoadFunction(inst->module, inst->buf);
}

void loadImgui(AppGraphCtxType type)
{
	const char* moduleName = demoAppDLLName(type);

	gImguiLoader.suffix = demoAppBackendSuffix(type);

	gImguiLoader.module = SDL_LoadObject(moduleName);

	gImguiLoader.imguiGraphContextInit_ptr = (imguiGraphContextInit_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphContextInit"));
	gImguiLoader.imguiGraphContextUpdate_ptr = (imguiGraphContextUpdate_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphContextUpdate"));
	gImguiLoader.imguiGraphContextDestroy_ptr = (imguiGraphContextDestroy_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphContextDestroy"));
	gImguiLoader.imguiGraphRecordBegin_ptr = (imguiGraphRecordBegin_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphRecordBegin"));
	gImguiLoader.imguiGraphRecordEnd_ptr = (imguiGraphRecordEnd_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphRecordEnd"));
	gImguiLoader.imguiGraphVertex2f_ptr = (imguiGraphVertex2f_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphVertex2f"));
	gImguiLoader.imguiGraphVertex2fv_ptr = (imguiGraphVertex2fv_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphVertex2fv"));
	gImguiLoader.imguiGraphTexCoord2f_ptr = (imguiGraphTexCoord2f_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphTexCoord2f"));
	gImguiLoader.imguiGraphColor4ub_ptr = (imguiGraphColor4ub_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphColor4ub"));
	gImguiLoader.imguiGraphColor4ubv_ptr = (imguiGraphColor4ubv_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphColor4ubv"));
	gImguiLoader.imguiGraphFontTextureEnable_ptr = (imguiGraphFontTextureEnable_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphFontTextureEnable"));
	gImguiLoader.imguiGraphFontTextureDisable_ptr = (imguiGraphFontTextureDisable_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphFontTextureDisable"));
	gImguiLoader.imguiGraphEnableScissor_ptr = (imguiGraphEnableScissor_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphEnableScissor"));
	gImguiLoader.imguiGraphDisableScissor_ptr = (imguiGraphDisableScissor_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphDisableScissor"));
	gImguiLoader.imguiGraphFontTextureInit_ptr = (imguiGraphFontTextureInit_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphFontTextureInit"));
	gImguiLoader.imguiGraphFontTextureRelease_ptr = (imguiGraphFontTextureRelease_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiGraphFontTextureRelease"));
	gImguiLoader.imguiInteropGraphInit_ptr = (imguiInteropGraphInit_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiInteropGraphInit"));
	gImguiLoader.imguiInteropGraphUpdate_ptr = (imguiInteropGraphUpdate_ptr_t)(imguiLoaderLoadFunction(&gImguiLoader, "imguiInteropGraphUpdate"));
}

void unloadImgui()
{
	gImguiLoader.imguiGraphContextInit_ptr = nullptr;
	gImguiLoader.imguiGraphContextUpdate_ptr = nullptr;
	gImguiLoader.imguiGraphContextDestroy_ptr = nullptr;
	gImguiLoader.imguiGraphRecordBegin_ptr = nullptr;
	gImguiLoader.imguiGraphRecordEnd_ptr = nullptr;
	gImguiLoader.imguiGraphVertex2f_ptr = nullptr;
	gImguiLoader.imguiGraphVertex2fv_ptr = nullptr;
	gImguiLoader.imguiGraphTexCoord2f_ptr = nullptr;
	gImguiLoader.imguiGraphColor4ub_ptr = nullptr;
	gImguiLoader.imguiGraphColor4ubv_ptr = nullptr;
	gImguiLoader.imguiGraphFontTextureEnable_ptr = nullptr;
	gImguiLoader.imguiGraphFontTextureDisable_ptr = nullptr;
	gImguiLoader.imguiGraphEnableScissor_ptr = nullptr;
	gImguiLoader.imguiGraphDisableScissor_ptr = nullptr;
	gImguiLoader.imguiGraphFontTextureInit_ptr = nullptr;
	gImguiLoader.imguiGraphFontTextureRelease_ptr = nullptr;
	gImguiLoader.imguiInteropGraphInit_ptr = nullptr;
	gImguiLoader.imguiInteropGraphUpdate_ptr = nullptr;

	SDL_UnloadObject(gImguiLoader.module);
}
