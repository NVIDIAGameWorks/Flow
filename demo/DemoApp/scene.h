/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef SCENE_H
#define SCENE_H

#include <DirectXMath.h>

#include <vector>

#include "mesh.h"
#include "meshInterop.h"
#include "bitmap.h"
#include "curveEditor.h"

#include "NvFlow.h"
#include "NvFlowInterop.h"

struct AppGraphCtx;

struct TimeStepper
{
	float m_deltaTime = 0.f;
	float m_timeError = 0.f;
	float m_fixedDt = (1.f / 60.f);
	int m_maxSteps = 1;
	int m_numSteps = 0;

	TimeStepper() {}

	int getNumSteps(float dt)
	{
		m_deltaTime = dt;

		// compute time steps
		m_timeError += m_deltaTime;

		m_numSteps = int(floorf((m_timeError / m_fixedDt)));

		if (m_numSteps < 0) m_numSteps = 0;

		m_timeError -= m_fixedDt * float(m_numSteps);

		if (m_numSteps > m_maxSteps) m_numSteps = m_maxSteps;

		return m_numSteps;
	}
};

struct Scene
{
	Scene(const char* name) : m_name(name) {}

	virtual void init(AppGraphCtx* context, int winw, int winh) = 0;
	virtual void resize(int winw, int winh);
	virtual void update(float dt) final;
	virtual void preDraw() = 0;
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view) = 0;
	virtual void release() = 0;

	virtual void shoot() = 0;

	virtual NvFlowUint64 getGridGPUMemUsage() = 0;
	virtual bool getStats(int lineIdx, int statIdx, char* buf) = 0;

	virtual void imgui(int x, int y, int w, int h) = 0;
	virtual bool imguiMouse(int mx, int my, unsigned char mbut);

	virtual bool shouldReset() = 0;
	virtual void reset() = 0;

	const char* m_name;

	int m_mx = 0;
	int m_my = 0;
	unsigned char m_mbut;
	int m_winw = 0;
	int m_winh = 0;

	float m_deltaTime = 0.f;

protected:
	virtual void doFrameUpdate(float dt) {}
	virtual void doUpdate(float dt) = 0;

	AppGraphCtx* m_context = nullptr;
	TimeStepper m_timeStepper;

	int m_editorWidth = 0;

	friend struct ColorMapped;
};

struct FlowColorMap
{
	NvFlowRenderMaterialPool* m_materialPool = nullptr;
	NvFlowRenderMaterialHandle m_materialDefault;
	NvFlowRenderMaterialHandle m_material0;
	NvFlowRenderMaterialHandle m_material1;

	unsigned int m_curveEditorHeight = 264u;

	bool m_curveEditorActiveDefault = false;
	bool m_curveEditorActiveMat0 = false;
	bool m_curveEditorActiveMat1 = false;
	CurveEditState m_editStateDefault = { 0, 0 };
	CurveEditState m_editStateMat0 = { 0, 0 };
	CurveEditState m_editStateMat1 = { 0, 0 };
	std::vector<CurvePoint> m_curvePointsDefault;
	std::vector<CurvePoint> m_curvePointsMat0;
	std::vector<CurvePoint> m_curvePointsMat1;

	void initColorMap(NvFlowContext* context, const CurvePoint* pts, int numPoints, bool ptsEnabled);
	void updateColorMap(NvFlowContext* context);
	void imguiUpdate(Scene* scene, NvFlowContext* context, int border, int x, int y, int w, int h);
	bool colorMapActive(int mx, int my, unsigned char mbut);
};

struct FlowContext
{
	AppGraphCtx* m_appctx = nullptr;

	NvFlowContext* m_renderContext = nullptr;
	NvFlowDepthStencilView* m_dsv = nullptr;
	NvFlowRenderTargetView* m_rtv = nullptr;

	NvFlowDevice* m_renderDevice = nullptr;
	NvFlowDevice* m_gridDevice = nullptr;
	NvFlowDeviceQueue* m_gridQueue = nullptr;
	NvFlowDeviceQueue* m_gridCopyQueue = nullptr;
	NvFlowDeviceQueue* m_renderCopyQueue = nullptr;
	NvFlowContext* m_gridContext = nullptr;
	NvFlowContext* m_gridCopyContext = nullptr;
	NvFlowContext* m_renderCopyContext = nullptr;

	bool m_multiGPUSupported = false;
	bool m_commandQueueSupported = false;
	bool m_enableMultiGPU = false;
	bool m_enableCommandQueue = false;
	bool m_multiGPUActive = false;
	bool m_commandQueueActive = false;

	int m_maxFramesInFlight = 3u;
	int m_framesInFlight = 0;

	double m_statUpdateAttemptCount = 0.0;
	double m_statUpdateSuccessCount = 0.0;
	float m_statUpdateDt = 0.f;

	FlowContext() {}
	~FlowContext() {}

	void init(AppGraphCtx* appctx);
	void release();

	bool updateBegin(float dt);
	void updateEnd();
	void preDrawBegin();
	void preDrawEnd();
	void drawBegin();
	void drawEnd();

protected:
	void createComputeContext();
	void releaseComputeContext();
	int computeContextBegin();
	void computeContextEnd();
};

struct FlowGridActor
{
	AppGraphCtx* m_appctx = nullptr;

	NvFlowGrid* m_grid = nullptr;
	NvFlowGridProxy* m_gridProxy = nullptr;
	NvFlowVolumeRender* m_volumeRender = nullptr;
	NvFlowVolumeShadow* m_volumeShadow = nullptr;	
	NvFlowCrossSection* m_crossSection = nullptr;
	NvFlowGridSummary* m_gridSummary = nullptr;
	NvFlowGridSummaryStateCPU* m_gridSummaryStateCPU = nullptr;

	NvFlowGridDesc m_gridDesc;
	NvFlowGridParams m_gridParams;
	NvFlowGridMaterialParams m_materialParams;
	NvFlowRenderMaterialParams m_renderMaterialDefaultParams;
	NvFlowRenderMaterialParams m_renderMaterialMat0Params;
	NvFlowRenderMaterialParams m_renderMaterialMat1Params;
	NvFlowVolumeRenderParams m_renderParams;
	NvFlowCrossSectionParams m_crossSectionParams;

	FlowColorMap m_colorMap;

	NvFlowGridExport* m_gridExportDebugVis = nullptr;
	NvFlowGridExport* m_gridExportOverride = nullptr;
	NvFlowVolumeRenderParams m_renderParamsOverride;
	bool m_separateLighting = false;

	float m_memoryLimit = 3.f;
	float m_memoryScale = 0.125f * 0.075f;
	int m_cellSizeLogScale = 0;
	float m_cellSizeScale = 1.f;

	bool m_enableVolumeShadow = false;
	bool m_forceApplyShadow = false;
	float m_shadowResidentScale = 1.f;
	float m_shadowIntensityScale = 0.5f;
	float m_shadowMinIntensity = 0.15f;
	NvFlowFloat4 m_shadowBlendCompMask = { 0.f, 0.f, 0.f, 0.f };
	float m_shadowBlendBias = 1.f;
	bool m_shadowDebugVis = false;
	float m_shadowPan = 0.f;
	float m_shadowTilt = 0.f;

	bool m_shadowWasForceApplied = false;
	NvFlowFloat4 m_forceIntensityCompMask = { 0.f, 0.f, 0.f, 0.f };
	float m_forceIntensityBias = 1.f;

	bool m_enableCrossSection = false;
	float m_crossSectionScale = 2.f;
	float m_crossSectionBackgroundColor = 0.f;
	NvFlowFloat3 m_crossSectionLineColor = { 141.f / 255.f, 199.f / 255.f, 63.f / 255.f };

	bool m_enableGridSummary = false;
	bool m_enableGridSummaryDebugVis = false;

	bool m_enableTranslationTest = false;
	float m_translationTimeScale = 1.f;
	bool m_enableTranslationTestOld = false;
	float m_translationTestTime = 0.f;
	NvFlowFloat3 m_translationOffsetA = { +4.f, 0.f, 0.f };
	NvFlowFloat3 m_translationOffsetB = { -4.f, 0.f, 0.f };

	NvFlowUint m_statNumLayers = 0u;
	NvFlowUint m_statNumDensityBlocks = 0u;
	NvFlowUint m_statNumDensityCells = 0u;
	NvFlowUint m_statMaxDensityBlocks = 0u;
	NvFlowUint m_statNumVelocityBlocks = 0u;
	NvFlowUint m_statNumVelocityCells = 0u;
	NvFlowUint m_statMaxVelocityBlocks = 0u;

	NvFlowUint m_statVolumeShadowBlocks = 0u;
	NvFlowUint m_statVolumeShadowCells = 0u;

	FlowGridActor() {}
	~FlowGridActor() {}

	void initParams(size_t vramAmount);
	void init(FlowContext* flowContext, AppGraphCtx* appctx);
	void release();

	void updatePreEmit(FlowContext* flowContext, float dt);
	void updatePostEmit(FlowContext* flowContext, float dt, bool shouldUpdate, bool shouldReset);
	void preDraw(FlowContext* flowContext);
	void draw(FlowContext* flowContext, DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
};

struct Projectile
{
	struct Path
	{
		float m_time = 0.f;
		float m_timeBegin = 0.f;
		float m_timeEnd = 0.f;
		bool m_active = false;

		float m_direction[3] = { 1.f, 0.f, 0.f };
		float m_offset[3] = { 0.f, 0.f, 0.f };
		float m_position[3] = { 0.f, 0.f, 0.f };
	};

	static const int m_pathsCap = 32u;
	Path m_paths[m_pathsCap];
	int m_pathsSize = 0;

	NvFlowShapeSDF* m_shape = nullptr;
	NvFlowGridEmitParams m_emitParams;

	float m_radius = 0.15f;
	float m_speed = 8.f;
	float m_temperature = 10.f;
	float m_fuel = 0.f;

	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_projection;

	AppGraphCtx* m_appctx = nullptr;
	MeshContext* m_meshContext = nullptr;
	Mesh* m_mesh = nullptr;
	bool m_shouldDrawMesh = true;
	bool m_fireBallMode = false;
	bool m_prediction = true;

	Projectile() {}

	void init(AppGraphCtx* appctx, NvFlowContext* context);

	void shoot();

	void update(NvFlowContext* context, NvFlowGrid* grid, float dt);

	void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);

	void imgui();

	void release();

protected:
	void updatePosition(Path& path, float dt);

	void updateEmitterMode();
};

struct SceneFluid : public Scene
{
	SceneFluid(const char* name) : Scene(name) {}

	virtual void initParams() = 0;

	void imguiDesc();
	void imguiFluidSim();
	void imguiFluidEmitter();
	void imguiFluidRender();
	void imguiFluidAlloc();
	void imguiFluidTime();
	void imguiLoadSave();

	virtual void imguiDescExtra() {}
	virtual void imguiFluidSimExtra() {}
	virtual void imguiFluidEmitterExtra() {}
	virtual void imguiFluidRenderExtra() {}
	virtual void imguiFluidAllocExtra() {}
	virtual void imguiFluidTimeExtra() {}

	virtual void imgui(int x, int y, int w, int h);
	virtual bool imguiMouse(int mx, int my, unsigned char mbut);

	virtual void shoot() { m_projectile.shoot(); }

	virtual NvFlowUint64 getGridGPUMemUsage();
	virtual bool getStats(int lineIdx, int statIdx, char* buf);

	AppGraphCtx* m_appctx = nullptr;

	FlowContext m_flowContext;
	FlowGridActor m_flowGridActor;

	Projectile m_projectile;

	NvFlowShapeSDF* m_shape = nullptr;

	NvFlowGridEmitParams m_emitParams;

	bool m_isFirstRun = true;
	bool m_shouldReset = false;
	bool m_shouldGridReset = false;
	bool m_shouldLoadPreset = false;

	bool m_autoResetMode = false;
	float m_autoResetTime = 0.f;
	float m_autoResetThresh = 0.f;

	virtual bool shouldReset() { return m_shouldReset; }
	virtual void reset();
};

struct SceneSimpleFlame : public SceneFluid
{
	SceneSimpleFlame() : SceneFluid("Simple Flame") {}

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);

	virtual void initParams();
};

struct SceneSimpleFlameDouble : public SceneSimpleFlame
{
	SceneSimpleFlameDouble() { m_name = "Multi Material"; }

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);

	virtual void initParams();

	NvFlowGridEmitParams m_emitParamsA;
	NvFlowGridEmitParams m_emitParamsB;
	NvFlowGridMaterialHandle m_materialA;
	NvFlowGridMaterialHandle m_materialB;
};

struct SceneSimpleFlameFuelMap : public SceneSimpleFlame
{
	SceneSimpleFlameFuelMap() { m_name = "Fuel Map"; }

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);

	virtual void initParams();
};

struct SceneSimpleFlameParticleSurface : public SceneSimpleFlame
{
	SceneSimpleFlameParticleSurface() { m_name = "Particle Surface"; }

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);
	virtual void imguiFluidRenderExtra();

	virtual void initParams();

	static void emitCustomAllocFunc(void* userdata, const NvFlowGridEmitCustomAllocParams* params);
	static void emitCustomEmitVelocityFunc(void* userdata, NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params);
	static void emitCustomEmitDensityFunc(void* userdata, NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params);

	void doEmitCustomAllocFunc(const NvFlowGridEmitCustomAllocParams* params);
	void doEmitCustomEmitVelocityFunc(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params);
	void doEmitCustomEmitDensityFunc(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params);

	NvFlowParticleSurface* m_particleSurface = nullptr;

	NvFlowParticleSurfaceParams m_particleParams = {};
	NvFlowParticleSurfaceEmitParams m_surfaceEmitParams = {};

	bool m_visualizeSurface = true;
	float m_time = 0.f;

	std::vector<float> m_positions;
};

struct SceneSimpleFlameThrower : public SceneSimpleFlame
{
	float time = 0.f;

	SceneSimpleFlameThrower() { m_name = "Simple Flame Thrower"; }

	virtual void initParams()
	{
		SceneSimpleFlame::initParams();

		m_flowGridActor.m_gridParams.gravity = { 0.f, -0.5f, 0.f };

		m_flowGridActor.m_materialParams.smoke.damping = 0.4f;
		m_flowGridActor.m_materialParams.smoke.fade = 0.4f;
		m_flowGridActor.m_materialParams.vorticityStrength = 20.f;

		m_emitParams.fuel = 2.0f;
		m_emitParams.temperature = 3.f;
		m_emitParams.smoke = 0.5f;

		m_emitParams.bounds.x.x = 0.5f;
		m_emitParams.bounds.y.y = 0.5f;
		m_emitParams.bounds.z.z = 0.5f;

		m_emitParams.allocationScale = { 1.f, 1.f, 1.f };
		m_emitParams.allocationPredict = 0.05f;
	}

	void init(AppGraphCtx* context, int winw, int winh)
	{
		SceneSimpleFlame::init(context, winw, winh);
	}

	void doUpdate(float dt)
	{
		time += dt;

		const float rate = 8.f;
		const float period = 2.f * 3.14159265f / rate;
		if (time > period) time -= period;

		m_emitParams.bounds.w.x = 0.f; // 2.f * cosf(rate * time);
		m_emitParams.bounds.w.z = 0.f; // 2.f * sinf(rate * time);

		float a = 0.25f * cosf(rate * time);

		m_emitParams.velocityLinear.x = +32.f * cosf(a);
		m_emitParams.velocityLinear.y = -2.f + 64.f * (1.f - cosf(a));
		m_emitParams.velocityLinear.z = +32.f * sinf(a);

		SceneSimpleFlame::doUpdate(dt);
	}

	void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
	{
		SceneSimpleFlame::draw(projection, view);
	}

	void release()
	{
		SceneSimpleFlame::release();
	}
};

struct SceneSimpleFlameBall : public SceneSimpleFlame
{
	float time = 0.f;
	float m_radius = 0.6f;

	SceneSimpleFlameBall() { m_name = "Simple Flame Ball"; }

	virtual void initParams();

	void init(AppGraphCtx* context, int winw, int winh);

	void doUpdate(float dt);

	void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);

	void imguiFluidRenderExtra();

	void imguiFluidEmitterExtra();

	void imguiFluidSimExtra();

	void imgui(int xIn, int yIn, int wIn, int hIn);

	void release();
};

struct SceneSimpleFlameAnimated : public SceneSimpleFlame
{
	float time = 0.f;

	SceneSimpleFlameAnimated() { m_name = "Simple Flame Animated"; }

	virtual void initParams()
	{
		SceneSimpleFlame::initParams();

		m_flowGridActor.m_gridParams.gravity = { 0.f, -0.5f, 0.f };

		m_emitParams.fuel = 1.4f;
		m_emitParams.temperature = 3.f;
		m_emitParams.smoke = 0.5f;

		m_emitParams.allocationScale = { 1.33f, 1.33f, 1.33f };
	}

	void init(AppGraphCtx* context, int winw, int winh)
	{
		SceneSimpleFlame::init(context, winw, winh);
	}

	void doUpdate(float dt)
	{
		time += dt;

		const float rate = 0.375f;
		const float period = 2.f * 3.14159265f / rate;
		if (time > period) time -= period;

		m_emitParams.bounds.w.x = 2.f * cosf(rate * time);
		m_emitParams.bounds.w.z = 2.f * sinf(rate * time);

		// testing hack
		//static int offset = 0;
		//offset++;
		//if (offset > 4)
		//{
		//	offset = 0;
		//}
		//m_emitParams.bounds.w.z += float(offset) - 4.f;

		//NvFlowFloat3 gridLocation = { m_emitParams.bounds.w.x, 0.f, m_emitParams.bounds.w.z };
		//NvFlowGridSetTargetLocation(m_flowGridActor.m_grid, gridLocation);

		//bool parity = (time - floorf(time)) > 0.5f;
		//NvFlowFloat3 gridLocation = parity ? NvFlowFloat3{-4.f, 0.f, 0.f} : NvFlowFloat3{+4.f, 0.f, 0.f};
		//NvFlowGridSetTargetLocation(m_flowGridActor.m_grid, gridLocation);

		m_emitParams.velocityLinear.x = +8.f * sinf(rate * time);
		m_emitParams.velocityLinear.y = -2.f;
		m_emitParams.velocityLinear.z = -8.f * cosf(rate * time);

		m_emitParams.predictVelocityWeight = 1.f;
		m_emitParams.predictVelocity.x = -4.f * sinf(rate * time);
		m_emitParams.predictVelocity.y = 0.f;
		m_emitParams.predictVelocity.z = +4.f * cosf(rate * time);

		SceneSimpleFlame::doUpdate(dt);
	}

	void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
	{
		SceneSimpleFlame::draw(projection, view);
	}

	void release()
	{
		SceneSimpleFlame::release();
	}
};

struct SceneDynamicCoupleRate : public SceneSimpleFlame
{
	float theta = 0.f;
	float rate = 0.25f;
	float coupleRateScale = 0.1f;
	float emitterScale = 1.f;

	SceneDynamicCoupleRate() { m_name = "Dynamic Couple Rate"; }

	virtual void initParams();

	void doUpdate(float dt);

	virtual void imguiFluidEmitterExtra();

	static float positionFunc(float theta);
	static float velocityFunc(float theta, float rate);
};

struct SceneSimpleFlameMesh : public SceneSimpleFlame
{
	MeshContext* m_meshContext = nullptr;
	Mesh* m_mesh = nullptr;
	NvFlowSDFGen* m_sdfGen = nullptr;
	NvFlowShapeSDF* m_teapotShape = nullptr;
	bool m_shouldDrawMesh = true;

	NvFlowFloat3 m_sdfScale = { 0.25f, 0.25f, 0.25f };
	NvFlowFloat3 m_emitterScale = { 0.5f, 0.5f, 0.5f };
	NvFlowFloat3 m_emitterOffset = { 0.f, 0.33f, 0.f };

	const char* m_meshPath = "../../data/teapot.ply";

	NvFlowGridEmitParams m_teapotEmitParams;

	bool m_animate = true;
	float m_time = 0.f;

	SceneSimpleFlameMesh() { m_name = "Simple Flame Mesh"; }

	virtual void initParams();

	virtual void init(AppGraphCtx* context, int winw, int winh);

	virtual void doUpdate(float dt);

	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);

	virtual void release();

	virtual void imguiFluidRenderExtra();

	virtual void imguiFluidEmitterExtra();
};

struct SceneSimpleFlameCollision : public SceneSimpleFlameMesh
{
	SceneSimpleFlameCollision() 
	{ 
		m_name = "Simple Flame Collision"; 
		m_meshPath = "../../data/box.ply";
		m_sdfScale = { 0.75f, 0.75f, 0.75f };
		m_emitterScale = { 1.5f, 0.125f, 1.5f };
		m_emitterOffset = { 0.f, 0.6f, 0.f };
	}
};

struct SceneSimpleFlameCulling : public SceneSimpleFlame
{
	SceneSimpleFlameCulling() { m_name = "Simple Flame Culling"; }

	virtual void initParams();

	virtual void doUpdate(float dt);

	virtual void imguiFluidEmitterExtra();

	NvFlowUint m_emitGridR = 8u;
};

struct SceneSimpleFlameConvex : public SceneSimpleFlame
{
	SceneSimpleFlameConvex() { m_name = "Convex"; }

	virtual void initParams();

	virtual void doUpdate(float dt);

	virtual void imguiFluidEmitterExtra();

	float m_size = 0.75f;
	float m_distanceScale = 1.f;
};

struct SceneSimpleFlameCapsule : public SceneSimpleFlame
{
	SceneSimpleFlameCapsule() { m_name = "Capsule"; }

	virtual void initParams();

	virtual void doUpdate(float dt);

	virtual void imguiFluidEmitterExtra();

	float m_capsuleRadius = 0.25f;
	float m_capsuleLength = 0.75f;
	float m_distanceScale = 3.5f;
	bool m_boxMode = false;
	bool m_flameSpread = false;
	bool m_flameSpreadOld = false;
};

struct Scene2DTextureEmitter : public SceneFluid
{
	Scene2DTextureEmitter(bool animEnabled) : SceneFluid(animEnabled ? "Logo Pulsed" : "Logo Steady"), m_animEnabled(animEnabled) {}

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imguiFluidEmitterExtra();

	virtual void initParams();

	Bitmap m_bitmap;

	float m_animTime = 0.f;
	bool m_animEnabled = false;

	void animChanged();
};

struct SceneSDFTest : public SceneFluid
{
	SceneSDFTest() : SceneFluid("SDF Test") {}

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);

	virtual void initParams();

	NvFlowSDFGen* m_sdfGen = nullptr;

	MeshContext* m_meshContext = nullptr;
	Mesh* m_mesh = nullptr;
};

struct ComputeContext;
struct ComputeShader;
struct ComputeConstantBuffer;
struct ComputeResource;
struct ComputeResourceRW;

struct SceneCustomLighting : public SceneSDFTest
{
	SceneCustomLighting() { m_name = "Custom Lighting"; }

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);

	virtual void initParams();

	float m_time = 0.f;

	NvFlowGridImport* m_import = nullptr;

	ComputeContext* m_computeContext = nullptr;
	ComputeShader* m_computeShader = nullptr;
	ComputeConstantBuffer* m_computeConstantBuffer = nullptr;
	ComputeResource* m_exportBlockList = nullptr;
	ComputeResource* m_exportBlockTable = nullptr;
	ComputeResource* m_exportData = nullptr;
	ComputeResource* m_importBlockList = nullptr;
	ComputeResource* m_importBlockTable = nullptr;
	ComputeResourceRW* m_importDataRW = nullptr;
};

struct SceneSimpleSmoke : public SceneFluid
{
	SceneSimpleSmoke() : SceneFluid("Simple Smoke") {}

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);

	virtual void imguiFluidEmitterExtra();
	virtual void imguiFluidRenderExtra();

	virtual void initParams();

	NvFlowVolumeShadow* m_volumeShadow = nullptr;

	bool m_animate = false;
	bool m_animateOld = false;
	float theta = 0.f;
	float rate = 0.125f;
	float coupleRateScale = 0.1f;

	static NvFlowFloat3 positionFunc(float theta);
	static NvFlowFloat3 velocityFunc(float theta, float rate);
};

struct SceneCustomEmit : public SceneFluid
{
	SceneCustomEmit() : SceneFluid("Custom Emit") {}

	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);

	virtual void imguiFluidEmitterExtra();

	virtual void initParams();

	static void emitCustomAllocFunc(void* userdata, const NvFlowGridEmitCustomAllocParams* params);
	static void emitCustomEmitVelocityFunc(void* userdata, NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params);
	static void emitCustomEmitDensityFunc(void* userdata, NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params);

	void doEmitCustomAllocFunc(const NvFlowGridEmitCustomAllocParams* params);
	void doEmitCustomEmitVelocityFunc(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params);
	void doEmitCustomEmitDensityFunc(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params);

	struct CustomEmitParams
	{
		NvFlowUint radius;
		NvFlowFloat4 targetValue;
		NvFlowFloat4 blendRate;
	};

	void doEmitCustomEmit(NvFlowUint* dataFrontIdx, const NvFlowGridEmitCustomEmitParams* params, const CustomEmitParams* customParams);

	ComputeContext* m_customContext = nullptr;
	ComputeShader* m_customEmitAllocCS = nullptr;
	ComputeShader* m_customEmitEmitCS = nullptr;
	ComputeShader* m_customEmitEmit2CS = nullptr;
	ComputeConstantBuffer* m_customConstantBuffer = nullptr;

	ComputeResourceRW* m_allocMask = nullptr;
	ComputeResource* m_blockTable = nullptr;
	ComputeResource* m_blockList = nullptr;
	ComputeResourceRW* m_dataRW[2u] = { nullptr };

	bool m_fullDomain = false;
};

struct SceneEmitSubStep : public SceneFluid
{
	SceneEmitSubStep() : SceneFluid("Emit Sub Step") {}

	virtual void initParams();
	virtual void init(AppGraphCtx* context, int winw, int winh);
	virtual void doFrameUpdate(float dt);
	virtual void doUpdate(float dt);
	virtual void preDraw();
	virtual void draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view);
	virtual void release();
	virtual void imgui(int x, int y, int w, int h);

	virtual void imguiFluidEmitterExtra();

	void emitSubSteps(float t_old, float x_old, float t_new, float x_new, float frame_dt);
	void emitImpulse(float x, float impulse_dt);

	float anim_t = 0.f;
	float anim_x = -1.f;
	float anim_x_old = 0.f;

	TimeStepper m_emitterTimeStepper;
};

Scene* getScene(int index);

void pointsToImage(NvFlowFloat4* image, int imageDim, const CurvePoint* pts, int numPts);

#endif