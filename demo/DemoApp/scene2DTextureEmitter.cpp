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

#include "loader.h"
#include "imgui.h"
#include "imguiser.h"

namespace Preset0
{
#include "preset0.h"
}

#include "scene.h"

#include <SDL.h>

void Scene2DTextureEmitter::initParams()
{
	m_flowGridActor.initParams(AppGraphCtxDedicatedVideoMemory(m_appctx));

	// set emitter defaults
	NvFlowGridEmitParamsDefaults(&m_emitParams);

	m_emitParams.bounds.x.x = 1.5f;
	m_emitParams.bounds.y.y = 1.5f;
	m_emitParams.bounds.z.z = 0.125f;
	m_emitParams.velocityLinear.y = 0.f;
	m_emitParams.fuel = 1.4f;
	m_emitParams.smoke = 0.5f;

	FILE* file = nullptr;
	fopen_s(&file, "../../data/GeforceClaw.bmp", "rb");
	if (file)
	{
		m_bitmap.read(file);
		fclose(file);
	}

	// grid parameter overrides
	m_flowGridActor.m_gridParams.gravity = NvFlowFloat3{ 0.f, -1.f, 0.f };
	//m_flowGridActor.m_gridParams.combustion.buoyancyPerTemp = 2.f;
	m_flowGridActor.m_materialParams.smokePerBurn = 4.f;
	m_flowGridActor.m_materialParams.tempPerBurn = 8.f;
	m_flowGridActor.m_materialParams.smoke.fade = 0.4f;
}

void Scene2DTextureEmitter::init(AppGraphCtx* appctx, int winw, int winh)
{
	m_appctx = appctx;

	if (!m_shouldReset || m_isFirstRun)
	{
		initParams();
		m_isFirstRun = false;
	}

	m_flowContext.init(appctx);

	m_flowGridActor.init(&m_flowContext, appctx);

	// more compute resources
	NvFlowShapeSDFDesc shapeDesc;
	//NvFlowShapeSDFDescDefaults(&shapeDesc);
	shapeDesc.resolution.x = m_bitmap.width;
	shapeDesc.resolution.y = m_bitmap.height;
	shapeDesc.resolution.z = 1u;
	m_shape = NvFlowCreateShapeSDF(m_flowContext.m_gridContext, &shapeDesc);

	// generate SDF from bitmap
	auto mappedData = NvFlowShapeSDFMap(m_shape, m_flowContext.m_gridContext);
	if (mappedData.data)
	{
		NvFlowUint bmRowPitch = ((m_bitmap.bitsPerPixel * m_bitmap.width + 31) / 32) * 4;

		for (NvFlowUint k = 0; k < mappedData.dim.z; k++)
			for (NvFlowUint j = 0; j < mappedData.dim.y; j++)
				for (NvFlowUint i = 0; i < mappedData.dim.x; i++)
				{
					float& val = mappedData.data[k * mappedData.depthPitch + j * mappedData.rowPitch + i];

					float v = 0.5f;

					NvFlowUint byteIdx = j * bmRowPitch + (i >> 3);
					NvFlowUint mask = 128 >> (i & 7);

					NvFlowUint inval = mask & m_bitmap.data[byteIdx];
					
					if (inval)
					{
						v = -0.5f;
					}

					val = v;
				}
		NvFlowShapeSDFUnmap(m_shape, m_flowContext.m_gridContext);
	}

	animChanged();

	// create default color map
	{
		const int numPoints = 5;
		const CurvePoint pts[numPoints] = 
		{
			{0.f, 0.f,0.f,0.f,0.f },
			{0.05f,0.f,0.f,0.f,0.5f},
			{0.6f,0.7f * 141.f / 255.f, 0.7f * 199.f / 255.f, 0.7f * 63.f / 255.f,0.8f},
			{0.85f,0.9f * 141.f / 255.f, 0.9f * 199.f / 255.f, 0.9f * 63.f / 255.f,0.8f},
			{1.f,1.5f * 141.f / 255.f, 1.5f * 199.f / 255.f, 1.5f * 63.f / 255.f,0.5f}
		};

		auto& colorMap = m_flowGridActor.m_colorMap;
		colorMap.initColorMap(m_flowContext.m_renderContext, pts, numPoints, (colorMap.m_curvePointsDefault.size() == 0));
	}

	m_projectile.init(m_appctx, m_flowContext.m_gridContext);

	resize(winw, winh);
}

void Scene2DTextureEmitter::doUpdate(float dt)
{
	bool shouldUpdate = m_flowContext.updateBegin(dt);
	if (shouldUpdate)
	{
		AppGraphCtxProfileBegin(m_appctx, "Simulate");

		{
			m_animTime += dt;
			if (m_animTime > 8.f) m_animTime = 0.f;

			if (m_animEnabled)
			{
				if (m_animTime < 2.f)
				{
					m_emitParams.temperature = 0.f;
					m_emitParams.smoke = 0.f;
					m_emitParams.fuel = 1.4f;
					m_emitParams.velocityLinear.y = 1.f;
				}
				else if (m_animTime < 2.25f)
				{
					m_emitParams.temperature = 5.f;
					m_emitParams.smoke = 0.5f;
					m_emitParams.velocityLinear.y = 8.f;
				}
				else if (m_animTime < 2.5f)
				{
					m_emitParams.temperature = 0.f;
					m_emitParams.fuel = 0.f;
					m_emitParams.smoke = 0.f;
					m_emitParams.velocityLinear.y = 0.f;
				}
			}

			m_flowGridActor.updatePreEmit(&m_flowContext, dt);

			NvFlowShapeDesc shapeDesc;
			shapeDesc.sdf.sdfOffset = 0u; // m_shape;

			m_emitParams.localToWorld = m_emitParams.bounds;
			m_emitParams.shapeType = eNvFlowShapeTypeSDF;
			m_emitParams.deltaTime = dt;

			NvFlowGridEmit(m_flowGridActor.m_grid, &shapeDesc, 1u, &m_emitParams, 1u);

			NvFlowShapeSDF* sdfs[] = { m_shape };
			NvFlowGridUpdateEmitSDFs(m_flowGridActor.m_grid, sdfs, 1u);

			m_projectile.update(m_flowContext.m_gridContext, m_flowGridActor.m_grid, dt);

			m_flowGridActor.updatePostEmit(&m_flowContext, dt, shouldUpdate, m_shouldGridReset);

			m_shouldGridReset = false;
		}

		AppGraphCtxProfileEnd(m_appctx, "Simulate");
	}
	m_flowContext.updateEnd();
}

void Scene2DTextureEmitter::preDraw()
{
	m_flowContext.preDrawBegin();

	m_flowGridActor.preDraw(&m_flowContext);

	m_flowContext.preDrawEnd();
}

void Scene2DTextureEmitter::draw(DirectX::CXMMATRIX projection, DirectX::CXMMATRIX view)
{
	m_projectile.draw(projection, view);

	m_flowContext.drawBegin();

	m_flowGridActor.draw(&m_flowContext, projection, view);

	m_flowContext.drawEnd();
}

void Scene2DTextureEmitter::release()
{
	m_projectile.release();

	m_flowGridActor.release();

	NvFlowReleaseShapeSDF(m_shape);

	m_flowContext.release();
}

void Scene2DTextureEmitter::animChanged()
{
	if (!m_animEnabled)
	{
		m_emitParams.temperature = 1.f;
		m_emitParams.smoke = 0.5f;
		m_emitParams.fuel = 1.2f;
		m_emitParams.velocityLinear.y = -1.f;
	}
	else
	{
		m_animTime = 0.f;
	}
}

void Scene2DTextureEmitter::imguiFluidEmitterExtra()
{
	imguiSeparatorLine();

	imguiLabel("Effect");

	if (imguiserCheck("Pulsed", m_animEnabled, true))
	{
		m_animEnabled = !m_animEnabled;
		animChanged();
	}
}