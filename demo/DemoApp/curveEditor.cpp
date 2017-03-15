/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "imgui.h"

#include "curveEditor.h"

namespace
{
	unsigned char colorInByte(float v)
	{
		const float cScale = 255.f;
		v *= cScale;
		if (v < 0.f) v = 0.f;
		if (v > 255.f) v = 255.f;
		return (unsigned char) v;
	}

	unsigned int colorInBytes(float r, float g, float b, float a)
	{
		return imguiRGBA(colorInByte(r), colorInByte(g), colorInByte(b), colorInByte(a));
	}
};

bool curveEditor(CurveEditState* editState, const CurveEditParams* params)
{
	const float sliderWidth = 256u;

	const float border = 20;
	const float ptSize0 = 12.f;
	const float ptOffset0 = -6.f;
	const float ptSize1 = 9.f;
	const float ptOffset1 = -4.5f;

	float graphx = params->editorBounds.x + border;
	float graphy = params->editorBounds.y + border;
	float graphw = float(params->editorBounds.w - 2 * border) - sliderWidth - border;
	float graphh = float(params->editorBounds.h - 2 * border);

	float xscale = graphw / (params->rangeMax.x - params->rangeMin.x);
	float ascale = graphh / (params->rangeMax.a - params->rangeMin.a);
	float xoffset = -xscale * params->rangeMin.x + graphx;
	float aoffset = -ascale * params->rangeMin.a + graphy;

	bool isMouseOverPoint = false;
	int mousePointIdx = 0;
	int mx = params->mouseState.x;
	int my = params->mouseState.y;

	bool modified = false;
	{
		// make a copy of the active point, in case it is modified
		editState->point = params->points[editState->activePointIndex];
	}

	// draw graph backgroud
	{
		imguiDrawRoundedRect(
			(float)params->editorBounds.x,
			(float)params->editorBounds.y,
			(float)params->editorBounds.w - sliderWidth - border,
			(float)params->editorBounds.h,
			6.f,
			imguiRGBA(0, 0, 0, 192)
			);
	}

	// draw border
	{
		float pts[4][2] = {
			{ graphx, graphy },
			{ graphx + graphw, graphy },
			{ graphx + graphw, graphy + graphh },
			{ graphx, graphy + graphh }
		};
		imguiDrawLine(pts[0][0], pts[0][1], pts[1][0], pts[1][1], 2.f, 0xCFCFCFCF);
		imguiDrawLine(pts[1][0], pts[1][1], pts[2][0], pts[2][1], 2.f, 0xCFCFCFCF);
		imguiDrawLine(pts[2][0], pts[2][1], pts[3][0], pts[3][1], 2.f, 0xCFCFCFCF);
		imguiDrawLine(pts[3][0], pts[3][1], pts[0][0], pts[0][1], 2.f, 0xCFCFCFCF);
	}

	// process mouse input
	{
		// check if mouse is still active
		if ((params->mouseState.but & IMGUI_MBUT_LEFT) == 0)
		{
			editState->pointMoveActive = false;
		}
		// check for overlap
		if (!editState->pointMoveActive)
		{
			for (unsigned int i = 0; i < params->numPoints; i++)
			{
				CurvePoint& pt = params->points[i];

				float x = xscale * pt.x + xoffset;
				float a = ascale * pt.a + aoffset;

				// check if mouse overlaps point
				if (float(mx) >= x + ptOffset0 && float(mx) <= x - ptOffset0 &&
					float(my) >= a + ptOffset0 && float(my) <= a - ptOffset0)
				{
					isMouseOverPoint = true;
					mousePointIdx = i;
				}
			}
		}
		// support dragging points
		if (isMouseOverPoint)
		{
			if (params->mouseState.but & IMGUI_MBUT_LEFT)
			{
				if (editState->activePointIndex != mousePointIdx)
				{
					editState->activePointIndex = mousePointIdx;
					editState->point = params->points[editState->activePointIndex];
				}
				if (!editState->pointMoveActive)
				{
					editState->pointMoveActive = true;
					editState->pointMoveX = mx;
					editState->pointMoveY = my;
				}
			}
		}
		if (editState->pointMoveActive)
		{
			CurvePoint& pt = params->points[editState->activePointIndex];

			int dmx = mx - editState->pointMoveX;
			int dmy = my - editState->pointMoveY;
			editState->pointMoveX = mx;
			editState->pointMoveY = my;

			// inverse transform
			float dx = dmx / xscale;
			float da = dmy / ascale;

			auto& point = editState->point;
			point = pt;
			point.x += dx;
			point.a += da;

			modified = true;
		}
	}

	// draw lines and points
	{
		float x0, a0, x1, a1;
		{
			CurvePoint& pt = params->points[0];

			x0 = xscale * pt.x + xoffset;
			a0 = ascale * pt.a + aoffset;
		}
		for (unsigned int i = 1; i < params->numPoints; i++)
		{
			CurvePoint& pt = params->points[i];

			// scale based on min max
			x1 = xscale * pt.x + xoffset;
			a1 = ascale * pt.a + aoffset;

			imguiDrawLine(x0, a0, x1, a1, 2.f, 0xFFFFFFFF);

			x0 = x1;
			a0 = a1;
		}

		int activeIdx = editState->activePointIndex;
		unsigned int activeColor = imguiRGBA(255, 196, 0, 255);
		for (unsigned int i = 0; i < params->numPoints; i++)
		{
			CurvePoint& pt = params->points[i];

			float x = xscale * pt.x + xoffset;
			float a = ascale * pt.a + aoffset;

			imguiDrawRect(x + ptOffset0, a + ptOffset0, ptSize0, ptSize0, (i == activeIdx) ? activeColor : 0xFFFFFFFF);
			imguiDrawRect(x + ptOffset1, a + ptOffset1, ptSize1, ptSize1, colorInBytes(pt.r, pt.g, pt.b, 1.f));
		}
		// highlight mouse point as needed
		if (isMouseOverPoint)
		{
			unsigned int mouseColor = imguiRGBA(255, 196, 0, 128);

			CurvePoint& pt = params->points[mousePointIdx];

			float x = xscale * pt.x + xoffset;
			float a = ascale * pt.a + aoffset;

			imguiDrawRect(x + ptOffset0, a + ptOffset0, ptSize0, ptSize0, mouseColor);
		}
	}

	// sliders
	{
		static int scroll = 0;

		imguiBeginScrollArea("Curve Editor",
			int(graphx + graphw + 2 * border), int(params->editorBounds.y),
			int(params->editorBounds.w - graphw - 3*border), int(params->editorBounds.h),
			&scroll);

		// keep active index valid
		if (editState->activePointIndex >= (int)params->numPoints)
		{
			editState->activePointIndex = params->numPoints - 1;
			if (editState->activePointIndex < 0) editState->activePointIndex = 0;
		}

		float val = float(editState->activePointIndex);
		if(imguiSlider("Active Point ID", &val, 0.f, float(params->numPoints) - 1.f, 1.f, true))
		{
			editState->activePointIndex = int(val);
			editState->point = params->points[editState->activePointIndex];
		}

		imguiSeparatorLine();

		// get active point data
		auto& point = editState->point;

		if (imguiSlider("x", &point.x, params->rangeMin.x, params->rangeMax.x, 0.01f, true)) modified = true;
		if (imguiSlider("r", &point.r, params->rangeMin.r, params->rangeMax.r, 0.01f, true)) modified = true;
		if (imguiSlider("g", &point.g, params->rangeMin.g, params->rangeMax.g, 0.01f, true)) modified = true;
		if (imguiSlider("b", &point.b, params->rangeMin.b, params->rangeMax.b, 0.01f, true)) modified = true;
		if (imguiSlider("a", &point.a, params->rangeMin.a, params->rangeMax.a, 0.01f, true)) modified = true;

		// enforce constraints
		{
			int activeIdx = editState->activePointIndex;

			// do not allow points to cross with their neighbors
			if (activeIdx > 0)
			{
				float xmin = params->points[activeIdx - 1].x;
				if (point.x < xmin)
				{
					point.x = xmin;
					modified = true;
				}
			}
			if (activeIdx + 1 < (int)params->numPoints)
			{
				float xmax = params->points[activeIdx + 1].x;
				if (point.x > xmax)
				{
					point.x = xmax;
					modified = true;
				}
			}

			// enforce that curve must span x range
			{
				if (activeIdx == 0)
				{
					point.x = params->rangeMin.x;
					modified = true;
				}
				if (activeIdx == params->numPoints - 1)
				{
					point.x = params->rangeMax.x;
					modified = true;
				}
			}
		}

		editState->action = modified ? CURVE_POINT_MODIFY : CURVE_POINT_NO_ACTION;

		if (imguiButton("Add Point", true))
		{
			editState->action = CURVE_POINT_INSERT;
			// generate interpolated new point
			int bidx = editState->activePointIndex + 1;
			if (bidx >= (int)params->numPoints)
			{
				bidx = editState->activePointIndex;
			}
			else
			{
				editState->activePointIndex++;
			}

			const auto ptb = params->points[bidx];
			point.x = 0.5f * point.x + 0.5f * ptb.x;
			point.r = 0.5f * point.r + 0.5f * ptb.r;
			point.g = 0.5f * point.g + 0.5f * ptb.g;
			point.b = 0.5f * point.b + 0.5f * ptb.b;
			point.a = 0.5f * point.a + 0.5f * ptb.a;
		}

		if (imguiButton("Remove Point", true))
		{
			// ignore if 2 or less
			if (params->numPoints > 2)
			{
				editState->action = CURVE_POINT_REMOVE;
			}
		}

		imguiEndScrollArea();
	}

	return (editState->action != CURVE_POINT_NO_ACTION);
}