/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

enum AppGraphCtxType
{
	APP_CONTEXT_D3D11 = 1,
	APP_CONTEXT_D3D12 = 2
};

#ifndef DEMOAPP_STR
#define XDEMOAPP_STR(s) DEMOAPP_STR(s)
#define DEMOAPP_STR(s) #s
#endif

namespace
{
	const char* demoAppDLLName(AppGraphCtxType type)
	{
		return (type == APP_CONTEXT_D3D12) ?
			"DemoAppD3D12" XDEMOAPP_STR(DLL_SUFFIX) ".dll" :
			"DemoAppD3D11" XDEMOAPP_STR(DLL_SUFFIX) ".dll";
	}

	const char* nvFlowDLLName(AppGraphCtxType type)
	{
		return (type == APP_CONTEXT_D3D12) ?
			"NvFlowD3D12" XDEMOAPP_STR(DLL_SUFFIX) ".dll" :
			"NvFlowD3D11" XDEMOAPP_STR(DLL_SUFFIX) ".dll";
	}
}

void loadModules(AppGraphCtxType type);
void unloadModules();

void loadAppGraphCtx(AppGraphCtxType type);
void unloadAppGraphCtx();

void loadNvFlowInterop(AppGraphCtxType type);
void unloadNvFlowInterop();

void loadMesh(AppGraphCtxType type);
void unloadMesh();

void loadImgui(AppGraphCtxType type);
void unloadImgui();

void loadComputeContext(AppGraphCtxType type);
void unloadComputeContext();

template<unsigned int maxFunctionCount, void* loadobject(const char*), void unloadobject(void*), void* loadfunction(void*,const char*)>
struct ModuleLoader
{
	static const int m_functionCount = maxFunctionCount;
	void** m_functionPtrs[m_functionCount] = { nullptr };
	const char* m_functionNames[m_functionCount] = { nullptr };

	void* m_module = nullptr;

	void* loadFunction(const char* name, int uid, void** ptr)
	{
		m_functionPtrs[uid] = ptr;
		m_functionNames[uid] = name;
		return SDL_LoadFunction(m_module, name);
	}

	template <int uid, class ret, class ...Args>
	ret function(ret(*)(Args...args), const char* name, Args...args)
	{
		static void* func = loadFunction(name, uid, &func);

		return ((ret(*)(Args...args))func)(args...);
	}

	void loadModule(const char* moduleName)
	{
		m_module = loadobject(moduleName);

		// load functions with non-null names
		for (int i = 0; i < m_functionCount; i++)
		{
			const char* name = m_functionNames[i];
			void** funcPtr = m_functionPtrs[i];
			if (name && funcPtr)
			{
				*funcPtr = loadfunction(m_module, name);
			}
		}
	}

	void unloadModule()
	{
		unloadobject(m_module);

		for (int i = 0; i < m_functionCount; i++)
		{
			void** funcPtr = m_functionPtrs[i];
			if (funcPtr)
			{
				*funcPtr = nullptr;
			}
		}
	}

	ModuleLoader() {}
};