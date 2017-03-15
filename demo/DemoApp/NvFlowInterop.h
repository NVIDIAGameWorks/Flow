/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef NV_FLOW_INTEROP_H
#define NV_FLOW_INTEROP_H

#include "appGraphCtx.h"
#include "NvFlow.h"

NV_FLOW_API NvFlowContext* NvFlowInteropCreateContext(AppGraphCtx* appctx);

NV_FLOW_API NvFlowDepthStencilView* NvFlowInteropCreateDepthStencilView(AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API NvFlowRenderTargetView* NvFlowInteropCreateRenderTargetView(AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API void NvFlowInteropUpdateContext(NvFlowContext* context, AppGraphCtx* appctx);

NV_FLOW_API void NvFlowInteropUpdateDepthStencilView(NvFlowDepthStencilView* view, AppGraphCtx* appctx, NvFlowContext* flowctx);

NV_FLOW_API void NvFlowInteropUpdateRenderTargetView(NvFlowRenderTargetView* view, AppGraphCtx* appctx, NvFlowContext* flowctx);

#endif