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