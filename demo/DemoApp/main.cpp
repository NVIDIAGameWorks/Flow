/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include <SDL.h>
#include <SDL_video.h>
#include <SDL_syswm.h>

#include "appGraphCtx.h"
#include "scene.h"

#include "imgui.h"
#include "imguiGraph.h"
#include "imguiInterop.h"
#include "imguiser.h"

#include "loader.h"

#include "camera.h"

#include <thread>

const int gWinWdefault = 2560 / 2;
const int gWinHdefault = 1600 / 2;
int gWinW = gWinWdefault;
int gWinH = gWinHdefault;

SDL_Window* gWin = nullptr;
bool gFullscreen = false;

bool gUseD3D12 = false;
int gAppRunCount = 1;

AppGraphCtx* gAppGraphCtx = nullptr;
Scene* gScene = nullptr;

Uint64 gTimerFreq = 0u;
Uint64 gTimerCount = 0;
float gDeltaTime = 0.f;
float gFixedDt = 1.f / 60.f;
bool gFixedTimeStepMode = false;

bool gPaused = false;
bool gProfileEnabled = false;

Camera gCamera;
bool gCameraActive = false;

int gMouseX = 0;
int gMouseY = 0;
unsigned char gMouseButton = 0;

const int g_imguiBorder = 20;
const int g_imguiWidth = 200;
const int g_imguiHeight = 250;
bool gImguiActive = false;
bool g_imguiHide = false;

bool gClearDark = false;
float gClearVal[4] = { 0.33f,0.33f,0.33f,1.f };

void toggleDark()
{
	gClearDark = !gClearDark;
	if (gClearDark)
	{
		gClearVal[0] = 0.f;
		gClearVal[1] = 0.f;
		gClearVal[2] = 0.f;
		gClearVal[3] = 1.f;
	}
	else
	{
		gClearVal[0] = 0.33f;
		gClearVal[1] = 0.33f;
		gClearVal[2] = 0.33f;
		gClearVal[3] = 1.f;
	}
}

bool appGraphCtxUpdateSize()
{
	return AppGraphCtxUpdateSize(gAppGraphCtx, gWin, gFullscreen);
}

void appReleaseRenderTargets()
{
	AppGraphCtxReleaseRenderTarget(gAppGraphCtx);
}

#define LEAK_TEST 0

#if LEAK_TEST
struct AppMemRefCount
{
	int refCount = 0;
	AppMemRefCount() {}
	~AppMemRefCount()
	{
		if (refCount != 0)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Flow Demo App", "Error: Memory Leak", nullptr);
		}
	}
};
AppMemRefCount gAppMemRefCount;

void* appMalloc(size_t size)
{
	gAppMemRefCount.refCount++;
	return malloc(size);
}

void appFree(void* ptr)
{
	if(ptr) gAppMemRefCount.refCount--;
	free(ptr);
}
#endif

void appInit()
{
	loadModules(gUseD3D12 ? APP_CONTEXT_D3D12 : APP_CONTEXT_D3D11);

#if LEAK_TEST
	NvFlowSetMallocFunc(appMalloc);
	NvFlowSetFreeFunc(appFree);
#endif

	// create app graph context
	gAppGraphCtx = AppGraphCtxCreate(0);

	appGraphCtxUpdateSize();

	gScene = getScene(0);

	AppGraphCtxFrameStart(gAppGraphCtx, gClearVal);

	gScene->init(gAppGraphCtx, gWinW, gWinH);

	// create imgui, connect to app graph context
	imguiInteropGraphInit(imguiGraphInit, "../../data/DroidSans.ttf", gAppGraphCtx);
	imguiserInit();

	AppGraphCtxFramePresent(gAppGraphCtx, true);
}

void appRender()
{
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX view;
	gCamera.getViewMatrix(view);
	gCamera.getProjectionMatrix(projection, gWinW, gWinH);

	bool valid = appGraphCtxUpdateSize();

	if (!valid)
	{
		return;
	}

	AppGraphCtxFrameStart(gAppGraphCtx, gClearVal);

	// interop update
	imguiInteropGraphUpdate(imguiGraphUpdate, gAppGraphCtx);

	// timer update
	{
		Uint64 oldCount = gTimerCount;
		gTimerFreq = SDL_GetPerformanceFrequency();
		gTimerCount = SDL_GetPerformanceCounter();
		if (oldCount > 0)
		{
			gDeltaTime = (float(gTimerCount - oldCount)) / float(gTimerFreq);
		}
		if (gFixedTimeStepMode)
		{
			gDeltaTime = gFixedDt;
		}
	}
	
	static unsigned int maxFramesInFlight = 4;

	// imgui update
	Scene* newScene = gScene;
	{
		imguiserBeginFrame();
		imguiBeginFrame(gMouseX, gMouseY, gMouseButton, 0);

		// print profile information as needed
		if (gProfileEnabled)
		{
			const int lineSpace = 16;

			imguiDrawText(g_imguiWidth + g_imguiBorder, gWinH - 2 * g_imguiBorder, IMGUI_ALIGN_LEFT, "Performance:", 0xFFFFFFFF);

			const char* label = nullptr;
			float cpuTime = 0.f;
			float gpuTime = 0.f;
			int index = 0;
			char buf[80];
			buf[79] = 0;
			// print frame time
			{
				static double frameSum = 0.0;
				static double frameCount = 0.0;
				frameSum += gDeltaTime;
				frameCount += 1.0;
				frameSum *= 0.99;
				frameCount *= 0.99;
				double frameAve = frameSum / frameCount;

				snprintf(buf, 79, "Frame: %.3f ms", 1000.f * frameAve);
				imguiDrawText(g_imguiWidth + g_imguiBorder, gWinH - 2 * g_imguiBorder - (index + 1) * lineSpace, IMGUI_ALIGN_LEFT, buf, 0xFFFFFFFF);
				index++;

				snprintf(buf, 79, "MaxFramesInFlight: %d", maxFramesInFlight);
				imguiDrawText(g_imguiWidth + g_imguiBorder, gWinH - 2 * g_imguiBorder - (index + 1) * lineSpace, IMGUI_ALIGN_LEFT, buf, 0xFFFFFFFF);
				index++;
			}
			int profileIndex = 0;
			while (AppGraphCtxProfileGet(gAppGraphCtx, &label, &cpuTime, &gpuTime, profileIndex))
			{
				snprintf(buf, 79, "%s: gpu(%.3f) cpu(%.3f) ms", label, 1000.f * gpuTime, 1000.f * cpuTime);
				imguiDrawText(g_imguiWidth + g_imguiBorder, gWinH - 2 * g_imguiBorder - (index + 1) * lineSpace, IMGUI_ALIGN_LEFT,buf, 0xFFFFFFFF);
				index++;
				profileIndex++;
			}

			// print memory usage
			int statIdx = 0;
			while(gScene && gScene->getStats(index, statIdx, buf))
			{
				imguiDrawText(g_imguiWidth + g_imguiBorder, gWinH - 2 * g_imguiBorder - (index + 1) * lineSpace, IMGUI_ALIGN_LEFT, buf, 0xFFFFFFFF);
				statIdx++;
				index++;
			}
		}

		static int scrollScene = 0u;

		int sceneX = g_imguiBorder;
		int sceneY = gWinH - g_imguiHeight;
		int sceneW = g_imguiWidth - g_imguiBorder;
		int sceneH = g_imguiHeight - g_imguiBorder;

		imguiBeginScrollArea("Scene",
			sceneX, sceneY,
			sceneW, sceneH,
			&scrollScene);

		for (int i = 0; true; i++)
		{
			Scene* scene = getScene(i);
			if (scene == nullptr) break;
			if (imguiItem(scene->m_name, true))
			{
				newScene = scene;
			}
		}

		imguiEndScrollArea();

		gScene->imgui(
			sceneX, sceneY,
			sceneW, sceneH
			);

		imguiEndFrame();
		imguiserEndFrame();
		imguiserUpdate();
	}

	// TODO: less synchronization preferred here
	if (gScene->shouldReset() || (newScene != gScene))
	{
		imguiGraphDraw();

		AppGraphCtxFramePresent(gAppGraphCtx, true);

		AppGraphCtxFrameStart(gAppGraphCtx, gClearVal);

		if (newScene != gScene)
		{
			gScene->release();
			gScene = newScene;
			gScene->init(gAppGraphCtx, gWinW, gWinH);
		}
		else
		{
			gScene->reset();
		}

		imguiGraphDraw();

		AppGraphCtxFramePresent(gAppGraphCtx, true);

		AppGraphCtxFrameStart(gAppGraphCtx, gClearVal);
	}

	if(!gPaused) gScene->update(gDeltaTime);

	gScene->preDraw();

	gScene->draw(projection,view);

	if (!g_imguiHide)
	{
		imguiGraphDraw();
	}

	AppGraphCtxFramePresent(gAppGraphCtx, false);

	// throttle frames in flight
	maxFramesInFlight = 4u;
	{
		static double frameSum = 0.0;
		static double frameCount = 0.0;
		frameSum += gDeltaTime;
		frameCount += 1.0;
		frameSum *= 0.99;
		frameCount *= 0.99;
		double frameAve = frameSum / frameCount;
		double targetLatency = 1.f / 60.f;
		double numFrames = targetLatency / frameAve;
		if (numFrames < 2.0)
		{
			maxFramesInFlight = 2;
		}
		else if (numFrames > 6.0)
		{
			maxFramesInFlight = 6;
		}
		else
		{
			maxFramesInFlight = (unsigned int)numFrames;
		}
	}
	AppGraphCtxWaitForFrames(gAppGraphCtx, maxFramesInFlight);
}

void appRelease()
{
	// do this first, since it flushes all GPU work
	appReleaseRenderTargets();

	gScene->release();

	imguiserDestroy();
	imguiGraphDestroy();

	AppGraphCtxRelease(gAppGraphCtx);

	gAppGraphCtx = nullptr;

	NvFlowDeferredRelease(2000.f);

	unloadModules();
}

bool imguiMouseEvent(SDL_Event& e)
{
	int x = 0;
	int y = 0;
	if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)	{
		x = e.button.x;
		y = e.button.y;
	}
	else if (e.type == SDL_MOUSEMOTION)	{
		x = e.motion.x;
		y = e.motion.y;
	}

	if (e.type == SDL_MOUSEBUTTONDOWN) {
		if (e.button.button == SDL_BUTTON_LEFT)	{
			gMouseButton = IMGUI_MBUT_LEFT;
		}
		else if (e.button.button == SDL_BUTTON_RIGHT) {
			gMouseButton = IMGUI_MBUT_RIGHT;
		}
	}
	else if (e.type == SDL_MOUSEBUTTONUP) {
		if (e.button.button == SDL_BUTTON_LEFT)	{
			gMouseButton = 0;
		}
		else if (e.button.button == SDL_BUTTON_RIGHT) {
			gMouseButton = 0;
		}
	}
	else if (e.type == SDL_MOUSEMOTION)	{
		gMouseX = e.motion.x;
		gMouseY = gWinH - 1 - e.motion.y;
	}

	bool active = gScene->imguiMouse(gMouseX, gMouseY, gMouseButton);

	active = active || (x < g_imguiWidth && y < g_imguiHeight);

	if (g_imguiHide)
	{
		active = false;
	}

	return active;
}

void mouseEvent(SDL_Event& e)
{
	if (e.type == SDL_MOUSEBUTTONDOWN)
	{
		if (!gCameraActive && !g_imguiHide)
		{
			gImguiActive = imguiMouseEvent(e);
		}
		if (!gImguiActive)
		{
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				gCamera.rotationStart(e.button.x, e.button.y);
				gCameraActive = true;
			}
			else if (e.button.button == SDL_BUTTON_MIDDLE)
			{
				gCamera.translateStart(e.button.x, e.button.y);
				gCameraActive = true;
			}
			else if (e.button.button == SDL_BUTTON_RIGHT)
			{
				gCamera.zoomStart(e.button.x, e.button.y);
				gCameraActive = true;
			}
		}
	}
	else if (e.type == SDL_MOUSEBUTTONUP)
	{
		{
			gImguiActive = imguiMouseEvent(e);
		}
		{
			if (e.button.button == SDL_BUTTON_LEFT)
			{
				gCamera.rotationEnd(e.button.x, e.button.y);
				gCameraActive = false;
			}
			else if (e.button.button == SDL_BUTTON_MIDDLE)
			{
				gCamera.translateEnd(e.button.x, e.button.y);
				gCameraActive = false;
			}
			else if (e.button.button == SDL_BUTTON_RIGHT)
			{
				gCamera.zoomEnd(e.button.x, e.button.y);
				gCameraActive = false;
			}
		}
	}
	else if (e.type == SDL_MOUSEMOTION)
	{
		gImguiActive = imguiMouseEvent(e);

		gCamera.rotationMove(e.motion.x, e.motion.y, gWinW, gWinH);
		gCamera.zoomMove(e.motion.x, e.motion.y, gWinW, gWinH);
		gCamera.translateMove(e.motion.x, e.motion.y, gWinW, gWinH);
	}
}

int main(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (0 == strcmp(argv[i], "-d3d12"))
		{
			gUseD3D12 = true;
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO))
	{
		fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
		return 1;
	}

	// preserve across transitions
	gCamera.init(gWinW, gWinH);

	for (; gAppRunCount > 0; gAppRunCount--)
	{
		const char* winName = gUseD3D12 ? "NvFlow Demo App D3D12" : "NvFlow Demo App D3D11";

		gWin = SDL_CreateWindow(winName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			gWinW, gWinH, SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_OPENGL*/);
		if (gWin == nullptr)
		{
			return -1;
		}

		appInit();

		bool shouldRun = true;
		while (shouldRun)
		{
			appRender();

			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				if (e.type == SDL_QUIT)
				{
					shouldRun = false;
					break;
				}
				else if (e.type == SDL_WINDOWEVENT)
				{
					if (e.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						gWinW = e.window.data1;
						gWinH = e.window.data2;

						gScene->resize(gWinW, gWinH);
					}
				}
				else if (e.type == SDL_KEYDOWN)
				{
					if (e.key.keysym.sym == SDLK_ESCAPE)
					{
						shouldRun = false;
					}
					if (e.key.keysym.sym == SDLK_F1)
					{
						gCamera.isProjectionRH = !gCamera.isProjectionRH;
					}
					if (e.key.keysym.sym == SDLK_F11)
					{
						gFullscreen = !gFullscreen;

						appReleaseRenderTargets();

						SDL_SetWindowFullscreen(gWin, gFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

						//appInitRenderTargets();
					}
					if (e.key.keysym.sym == SDLK_F10)
					{
						gAppRunCount++;
						shouldRun = false;
					}
					if (e.key.keysym.sym == SDLK_q)
					{
						gFixedTimeStepMode = !gFixedTimeStepMode;
					}
					if (e.key.keysym.sym == SDLK_w)
					{
						gProfileEnabled = !gProfileEnabled;
						if(gAppGraphCtx) AppGraphCtxProfileEnable(gAppGraphCtx, gProfileEnabled);
					}
					if (e.key.keysym.sym == SDLK_SPACE)
					{
						if (gScene) gScene->shoot();
					}
					if (e.key.keysym.sym == SDLK_p)
					{
						gPaused = !gPaused;
					}
					if (e.key.keysym.sym == SDLK_d)
					{
						toggleDark();
					}
					if (e.key.keysym.sym == SDLK_g)
					{
						g_imguiHide = !g_imguiHide;
					}
				}
				else if (e.type == SDL_KEYUP)
				{
				}
				else if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP || e.type == SDL_MOUSEMOTION)
				{
					mouseEvent(e);
				}
			}
		}

		appRelease();

		SDL_DestroyWindow(gWin);

		// mark module changes
		gUseD3D12 = !gUseD3D12;
		gFullscreen = false;
	}

	SDL_Quit();

	return 0;
}