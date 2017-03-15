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

#include "mesh.h"

#include "appGraphCtx.h"

MESH_API MeshContext* MeshInteropContextCreate(AppGraphCtx* appctx);

MESH_API void MeshInteropContextUpdate(MeshContext* context, AppGraphCtx* appctx);