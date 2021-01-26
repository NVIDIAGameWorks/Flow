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

#include "bitmap.h"

Bitmap::Bitmap() : data(NULL)
{

}

Bitmap::~Bitmap()
{
	delete[] data;
	data = NULL;
}

int Bitmap::create(int w, int h, int bpp)
{
	const int imagesize = w*h*(bpp / 8);
	headerField0 = 'B';
	headerField1 = 'M';
	size = 54 + imagesize;
	reserved1 = 0;
	reserved2 = 0;
	offset = 54;
	headerSize = 40;
	width = w;
	height = h;
	colorPlanes = 1;
	bitsPerPixel = bpp;
	compressionMethod = 0;
	imageSize = imagesize;
	hRes = 2000;
	vRes = 2000;
	numColors = 0;
	numImportantColors = 0;
	data = new unsigned char[imagesize];
	return 0;
}

int Bitmap::write(FILE* stream)
{
	if (stream == NULL || data == NULL) return -1;
	fwrite(&headerField0, 1, 1, stream);
	fwrite(&headerField1, 1, 1, stream);
	fwrite(&size, 4, 1, stream);
	fwrite(&reserved1, 2, 1, stream);
	fwrite(&reserved2, 2, 1, stream);
	fwrite(&offset, 4, 1, stream);
	fwrite(&headerSize, 4, 1, stream);
	fwrite(&width, 4, 1, stream);
	fwrite(&height, 4, 1, stream);
	fwrite(&colorPlanes, 2, 1, stream);
	fwrite(&bitsPerPixel, 2, 1, stream);
	fwrite(&compressionMethod, 4, 1, stream);
	fwrite(&imageSize, 4, 1, stream);
	fwrite(&hRes, 4, 1, stream);
	fwrite(&vRes, 4, 1, stream);
	fwrite(&numColors, 4, 1, stream);
	fwrite(&numImportantColors, 4, 1, stream);

	if (compressionMethod == 0)
	{
		fwrite(data, 1, imageSize, stream);
	}
	else
	{
		printf("Write format not supported\n");
		return -1;
	}

	return 0;
}

int Bitmap::read(FILE* stream)
{
	if (stream == NULL) return -1;
	size_t readCount = 0;
	readCount += fread(&headerField0, 1, 1, stream);
	readCount += fread(&headerField1, 1, 1, stream);
	readCount += fread(&size, 4, 1, stream);
	readCount += fread(&reserved1, 2, 1, stream);
	readCount += fread(&reserved2, 2, 1, stream);
	readCount += fread(&offset, 4, 1, stream);
	readCount += fread(&headerSize, 4, 1, stream);
	readCount += fread(&width, 4, 1, stream);
	readCount += fread(&height, 4, 1, stream);
	readCount += fread(&colorPlanes, 2, 1, stream);
	readCount += fread(&bitsPerPixel, 2, 1, stream);
	readCount += fread(&compressionMethod, 4, 1, stream);
	readCount += fread(&imageSize, 4, 1, stream);
	readCount += fread(&hRes, 4, 1, stream);
	readCount += fread(&vRes, 4, 1, stream);
	readCount += fread(&numColors, 4, 1, stream);
	readCount += fread(&numImportantColors, 4, 1, stream);

	if (compressionMethod == 0)
	{
		delete[] data;
		data = new unsigned char[imageSize];
		readCount += fread(data, 1, imageSize, stream);
	}
	else
	{
		printf("Write format not supported\n");
		return -1;
	}

	return 0;
}
