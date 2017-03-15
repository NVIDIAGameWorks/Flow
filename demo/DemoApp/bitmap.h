/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

struct Bitmap
{
	// Header Elements
	char headerField0, headerField1;
	unsigned int size;
	unsigned short reserved1;
	unsigned short reserved2;
	unsigned int offset;
	unsigned int headerSize;
	unsigned int width;
	unsigned int height;
	unsigned short colorPlanes;
	unsigned short bitsPerPixel;
	unsigned int compressionMethod;
	unsigned int imageSize;
	unsigned int hRes;
	unsigned int vRes;
	unsigned int numColors;
	unsigned int numImportantColors;
	// Internal
	unsigned char* data;

	Bitmap();
	~Bitmap();
	int create(int w, int h, int bpp);
	int write(FILE* stream);
	int read(FILE* stream);

};
