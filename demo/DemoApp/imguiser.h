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

#include "imgui.h"

struct ImserNode;

/// ******************* imguiser public interface ********************

void imguiserInit();

void imguiserUpdate();

void imguiserDestroy();

void imguiserBeginFrame();

void imguiserEndFrame();

void imguiserBeginGroup(const char* name, int* numItems);

void imguiserEndGroup();

bool imguiserCheck(const char* text, bool checked, bool enabled = true);

bool imguiserSlider(const char* text, float* val, float vmin, float vmax, float vinc, bool enabled = true);

bool imguiserOffscreenUpdate();

void imguiserValue1f(const char* text, float* val);

void imguiserValueBool(const char* text, bool* val);

void imguiserSave(const char* filename);

void imguiserLoad(const char* filename);

void imguiserLoadC(const ImserNode* nodes, unsigned int sizeInBytes);

/// **************** imguiser serialization protocol *********************

enum ImserType
{
	IMSER_TYPE_GROUP_BEGIN = 0,
	IMSER_TYPE_GROUP_END = 1,
	IMSER_TYPE_FLOAT = 2,
	IMSER_TYPE_BOOL = 3,
};

struct ImserNode
{
	const char* name;
	ImserType typeID;
	union
	{
		float valFloat;
		bool valBool;
	};
};

ImserNode imserNodeGroupBegin(const char* name);

ImserNode imserNodeGroupEnd();

ImserNode imserNodeValue1f(const char* name, float value);

ImserNode imserNodeValueBool(const char* name, bool value);