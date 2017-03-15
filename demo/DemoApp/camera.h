/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>
#include <DirectXMath.h>

struct Camera
{
	uint32_t rotationActive;
	uint32_t zoomActive;
	uint32_t translateActive;
	int mouseXprev;
	int mouseYprev;
	
	float defaultZoomVal;
	float zoomVal;

	DirectX::XMMATRIX pan;
	DirectX::XMMATRIX tilt;
	DirectX::XMMATRIX rotation;
	DirectX::XMMATRIX zoom;
	DirectX::XMMATRIX translate;
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX projection;

	bool isProjectionRH = false;

	void getViewMatrix(DirectX::XMMATRIX& viewMatrix)
	{
		view =
			DirectX::XMMatrixMultiply(
				DirectX::XMMatrixMultiply(
					translate,
					rotation
					),
				zoom
				);
		if (isProjectionRH)
		{
			view = DirectX::XMMatrixMultiply(
				DirectX::XMMatrixMultiply(
					DirectX::XMMatrixScaling(1.f, 1.f, -1.f),
					view
				),
				DirectX::XMMatrixScaling(1.f, 1.f, -1.f)
			);
		}
		viewMatrix = view;
	}

	void getProjectionMatrix(DirectX::XMMATRIX& projMatrix, int winw, int winh)
	{
		if (isProjectionRH)
		{
			projection = DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PI / 4.f, float(winw) / float(winh), 0.1f, 1000.f);
			projMatrix = projection;
		}
		else
		{
			projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.f, float(winw) / float(winh), 0.1f, 1000.f);
			projMatrix = projection;
		}
	}

	void rotationStart(int x, int y)
	{
		mouseXprev = x;
		mouseYprev = y;
		rotationActive = true;
	}

	void rotationMove(int x, int y, int winw, int winh)
	{
		if (rotationActive)
		{
			int dx = -(x - mouseXprev);
			int dy = -(y - mouseYprev);

			const float gainX = 2.f * 3.14f / (winw);
			const float gainY = 2.f * 3.14f / (winh);

			float rotx = float(gainY * dy);
			float roty = float(gainX * dx);
			float rotMagn = sqrtf(rotx*rotx + roty*roty);

			// tilt
			{
				DirectX::XMVECTOR rotVec = { 1.f,0.f,0.f,0.f };
				const float angle = rotx;
				DirectX::XMMATRIX dtilt = DirectX::XMMatrixRotationAxis(rotVec, angle);
				tilt = DirectX::XMMatrixMultiply(dtilt, tilt);
			}
			// pan
			{
				DirectX::XMVECTOR rotVec = { 0.f,1.f,0.f,0.f };
				const float angle = roty;
				DirectX::XMMATRIX dpan = DirectX::XMMatrixRotationAxis(rotVec, angle);
				pan = DirectX::XMMatrixMultiply(dpan, pan);
			}
			rotation = DirectX::XMMatrixMultiply(pan, tilt);

			mouseXprev = x;
			mouseYprev = y;
		}
	}

	void rotationEnd(int x, int y)
	{
		mouseXprev = x;
		mouseYprev = y;
		rotationActive = false;
	}

	void zoomStart(int x, int y)
	{
		mouseXprev = x;
		mouseYprev = y;
		zoomActive = true;
	}

	void zoomMove(int x, int y, int winw, int winh)
	{
		if (zoomActive)
		{
			float dx = -float(x - mouseXprev);
			float dy = -float(y - mouseYprev);

			const float gain = 3.14f / float(winh);

			//zoom = DirectX::XMMatrixMultiply(zoom, DirectX::XMMatrixScaling(1.f, 1.f, 1.f + gain*dy));
			zoomVal *= (1.f + gain*dy);
			zoom = DirectX::XMMatrixTranslation(0.f, 0.f, zoomVal);

			mouseXprev = x;
			mouseYprev = y;
		}
	}

	void zoomEnd(int x, int y)
	{
		mouseXprev = x;
		mouseYprev = y;
		zoomActive = false;
	}

	void translateStart(int x, int y)
	{
		mouseXprev = x;
		mouseYprev = y;
		translateActive = true;
	}

	void translateMove(int x, int y, int winw, int winh)
	{
		if (translateActive)
		{
			float dx = float(x - mouseXprev);
			float dy = -float(y - mouseYprev);

			const float gainX = isProjectionRH ? -2.f / (winw) : +2.f / (winw);
			const float gainY = isProjectionRH ? -2.f / (winh) : +2.f / (winh);

			// find rotation center in screen space
			DirectX::XMVECTOR centerScreen = { 0.f,0.f,0.f,1.f };
			centerScreen = DirectX::XMVector4Transform(centerScreen, DirectX::XMMatrixMultiply(zoom, projection));
			DirectX::XMFLOAT4 centerScreen4;
			DirectX::XMStoreFloat4(&centerScreen4, centerScreen);

			// produce transform for inverse project, zoom, rotate
			DirectX::XMVECTOR det;
			DirectX::XMMATRIX viewProjInv =
				DirectX::XMMatrixInverse(
					&det,
					DirectX::XMMatrixMultiply(
						DirectX::XMMatrixMultiply(
							rotation,
							zoom
							),
						projection
						)
					);

			// transform screen offset to world offset
			DirectX::XMVECTOR offsetScreen = { gainX*dx, gainY*dy, centerScreen4.z / centerScreen4.w, 1.f };
			DirectX::XMVECTOR offsetWorld =
				DirectX::XMVector4Transform(
					offsetScreen,
					viewProjInv
					);
			DirectX::XMFLOAT4 offsetWorld4;
			DirectX::XMStoreFloat4(&offsetWorld4, offsetWorld);
			offsetWorld4.x /= offsetWorld4.w;
			offsetWorld4.y /= offsetWorld4.w;
			offsetWorld4.z /= offsetWorld4.w;

			// apply offset to net world translation
			translate =
				DirectX::XMMatrixMultiply(
					DirectX::XMMatrixTranslation(offsetWorld4.x, offsetWorld4.y, offsetWorld4.z),
					translate
					);

			mouseXprev = x;
			mouseYprev = y;
		}
	}

	void translateEnd(int x, int  y)
	{
		mouseXprev = x;
		mouseYprev = y;
		translateActive = false;
	}

	void init(int winw, int winh)
	{
		rotationActive = 0u;
		zoomActive = 0u;
		translateActive = 0u;
		defaultZoomVal = 7.f;
		zoomVal = defaultZoomVal;
		pan = DirectX::XMMatrixIdentity();
		{
			//DirectX::XMVECTOR rotVec = {1.f,0.f,0.f,0.f};
			//const float angle = -DirectX::XM_PI / 4.f;
			//tilt = DirectX::XMMatrixRotationAxis(rotVec,angle);

			DirectX::XMVECTOR rotVec = { 1.f,0.f,0.f,0.f };
			const float angle = 0.f;
			tilt = DirectX::XMMatrixRotationAxis(rotVec, angle);
		}
		rotation = DirectX::XMMatrixMultiply(pan, tilt);
		zoom = DirectX::XMMatrixTranslation(0.f, 0.f, zoomVal);
		translate = DirectX::XMMatrixIdentity();
		view = DirectX::XMMatrixIdentity();
		projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 4.f, float(winw) / float(winh), 0.1f, 1000.f);
	}

	Camera()
	{
		init(1024, 1024);
	}
};

#endif