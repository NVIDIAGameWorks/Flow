/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#pragma once

struct CurveMouseState
{
	int x;
	int y;
	unsigned char but;
};

struct CurveEditorBounds
{
	int x, y, w, h;
};

struct CurvePoint
{
	float x;
	float r, g, b, a;
};

enum CurvePointAction
{
	CURVE_POINT_NO_ACTION,
	CURVE_POINT_MODIFY,
	CURVE_POINT_INSERT,
	CURVE_POINT_REMOVE,
};

struct CurveEditParams
{
	CurveMouseState mouseState;
	CurveEditorBounds editorBounds;
	CurvePoint rangeMin;
	CurvePoint rangeMax;
	CurvePoint* points;
	unsigned int numPoints;
};

struct CurveEditState
{
	int activePointIndex;
	int action;
	bool pointMoveActive;
	int pointMoveX;
	int pointMoveY;
	CurvePoint point;
};

bool curveEditor(CurveEditState* editState, const CurveEditParams* params);