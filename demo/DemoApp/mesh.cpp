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

#include "mesh.h"

#include <vector>

struct Mesh
{
	std::vector<MeshVertex> m_vertices;
	std::vector<MeshUint> m_indices;

	float m_bounds[6] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };

	MeshContext* m_context = nullptr;
	MeshIndexBuffer* m_indexBuffer = nullptr;
	MeshVertexBuffer* m_vertexBuffer = nullptr;

	Mesh() {}

	void loadFromPLY(const char* filename);

	void computeBounds();

	void normalize();
};

Mesh* MeshCreate(MeshContext* context)
{
	Mesh* mesh = new Mesh;

	mesh->m_context = context;

	return mesh;
}

void MeshLoadFromFile(Mesh* mesh, const char* filename)
{
	MeshIndexBufferRelease(mesh->m_indexBuffer);
	MeshVertexBufferRelease(mesh->m_vertexBuffer);

	mesh->loadFromPLY(filename);

	mesh->computeBounds();

	mesh->normalize();

	mesh->m_vertexBuffer = MeshVertexBufferCreate(mesh->m_context, &mesh->m_vertices[0], (MeshUint)mesh->m_vertices.size());
	mesh->m_indexBuffer = MeshIndexBufferCreate(mesh->m_context, &mesh->m_indices[0], (MeshUint)mesh->m_indices.size());
}

void MeshGetData(Mesh* mesh, MeshData* data)
{
	data->numVertices = (MeshUint) mesh->m_vertices.size();
	data->positions = &mesh->m_vertices[0].x;
	data->positionStride = sizeof(MeshVertex);
	data->normals = &mesh->m_vertices[0].nx;
	data->normalStride = sizeof(MeshVertex);

	data->numIndices = (MeshUint) mesh->m_indices.size();
	data->indices = &mesh->m_indices[0];

	data->boundsMin[0] = mesh->m_bounds[0];
	data->boundsMin[1] = mesh->m_bounds[1];
	data->boundsMin[2] = mesh->m_bounds[2];
	data->boundsMax[0] = mesh->m_bounds[3];
	data->boundsMax[1] = mesh->m_bounds[4];
	data->boundsMax[2] = mesh->m_bounds[5];
}

void MeshDraw(Mesh* mesh, const MeshDrawParams* params)
{
	MeshContextDrawParams drawParams;
	drawParams.params = params;
	drawParams.indexBuffer = mesh->m_indexBuffer;
	drawParams.vertexBuffer = mesh->m_vertexBuffer;

	MeshContextDraw(mesh->m_context, &drawParams);
}

void MeshRelease(Mesh* mesh)
{
	if (mesh == nullptr) return;

	MeshIndexBufferRelease(mesh->m_indexBuffer);
	MeshVertexBufferRelease(mesh->m_vertexBuffer);

	delete mesh;
}

/// **************** Private functions *******************************

void Mesh::computeBounds()
{
	size_t imax = m_vertices.size();
	if (imax >= 1)
	{
		m_bounds[0] = m_bounds[3] = m_vertices[0].x;
		m_bounds[1] = m_bounds[4] = m_vertices[0].y;
		m_bounds[2] = m_bounds[5] = m_vertices[0].z;

		for (size_t i = 1; i < imax; i++)
		{
			m_bounds[0] = fminf(m_bounds[0], m_vertices[i].x);
			m_bounds[3] = fmaxf(m_bounds[3], m_vertices[i].x);
			m_bounds[1] = fminf(m_bounds[1], m_vertices[i].y);
			m_bounds[4] = fmaxf(m_bounds[4], m_vertices[i].y);
			m_bounds[2] = fminf(m_bounds[2], m_vertices[i].z);
			m_bounds[5] = fmaxf(m_bounds[5], m_vertices[i].z);
		}
	}
}

void Mesh::normalize()
{
	size_t imax = m_vertices.size();
	for (size_t i = 0; i < imax; i++)
	{
		float x, y, z, w;
		x = m_vertices[i].nx;
		y = m_vertices[i].ny;
		z = m_vertices[i].nz;
		w = sqrtf(x*x + y*y + z*z);
		if (w > 0.f)
		{
			x /= w;
			y /= w;
			z /= w;
		}
		m_vertices[i].nx = x;
		m_vertices[i].ny = y;
		m_vertices[i].nz = z;
	}
}

/// ****************** PLY mesh support *******************************

namespace
{
	struct PLYLoader
	{
		enum ElementType
		{
			ELEM_VERTEX = 0,
			ELEM_FACE,
			ELEM_EDGE,
			ELEM_INVALID
		};

		enum Format
		{
			FORMAT_ASCII = 0,
			FORMAT_BINARY_LITTLE_ENDIAN,
			FORMAT_BINARY_BIG_ENDIAN,
			FORMAT_INVALID
		};

		// temporary variables
		FILE* file = nullptr;

		static const int bufSize = 1024u;
		char buf[bufSize];

		ElementType elementType = ELEM_INVALID;
		Format format = FORMAT_INVALID;

		int numElements[ELEM_INVALID] = { 0 };
		int numProperties[ELEM_INVALID] = { 0 };

		int elementList[ELEM_INVALID] = { 0 };
		int elementListIdx = 0;

		// captured variables
		std::vector<MeshVertex>& m_vertices;
		std::vector<uint32_t>& m_indices;

		// capture in constructor
		PLYLoader(Mesh& mesh) :
			m_vertices(mesh.m_vertices),
			m_indices(mesh.m_indices)
		{
		}

		// utility functions
		void getline()
		{
			fgets(buf, bufSize, file);
		};
		void getstr()
		{
			fscanf_s(file, "%s", buf, bufSize);
		};
		int getint()
		{
			int i = 0;
			fscanf_s(file, "%d", &i);
			return i;
		};
		bool match(const char* key)
		{
			return strncmp(buf, key, bufSize) == 0;
		};

		// temporary state
		int elemType = ELEM_INVALID;
		int elemNum = 0;
		int elemProperties = 0;

		template<class getFloatType, class advanceFuncType>
		void getVertices(getFloatType getFloat, advanceFuncType advanceFunc)
		{
			for (int i = 0; i < elemNum; i++)
			{
				float data[6];
				for (int j = 0; j < elemProperties; j++)
				{
					float val = getFloat();
					if (j < 6) data[j] = val;
				}
				m_vertices.push_back(MeshVertex{
					data[0], data[1], data[2],
					data[3], data[4], data[5]
				});
				advanceFunc();
			}
		}

		template<class getCountType, class getIndexType, class advanceFuncType>
		void getIndices(getCountType getCount, getIndexType getIndex, advanceFuncType advanceFunc)
		{
			for (int i = 0; i < elemNum; i++)
			{
				int count = getCount();
				int indices[4];
				if (count > 0) indices[0] = getIndex();
				if (count > 1) indices[1] = getIndex();
				if (count > 2) indices[2] = getIndex();
				if (count > 3) indices[3] = getIndex();

				if (count >= 3)
				{
					m_indices.push_back(indices[0]);
					m_indices.push_back(indices[1]);
					m_indices.push_back(indices[2]);
				}
				if (count == 4)
				{
					m_indices.push_back(indices[2]);
					m_indices.push_back(indices[3]);
					m_indices.push_back(indices[0]);
				}
				advanceFunc();
			}
		}

		// main phases
		bool parseHeader()
		{
			// verify file format
			getstr();
			if (match("ply"))
			{
				// extra header information
				while (feof(file) == 0)
				{
					getstr();
					if (match("element"))
					{
						getstr();
						if (match("vertex")) {
							elementType = ELEM_VERTEX;
						}
						else if (match("face")) {
							elementType = ELEM_FACE;
						}
						else if (match("edge")) {
							elementType = ELEM_EDGE;
						}
						unsigned int idx = (unsigned int)elementType;
						if (idx < ELEM_INVALID)
						{
							numElements[idx] = getint();
						}
						if (elementListIdx < ELEM_INVALID)
						{
							elementList[elementListIdx++] = elementType;
						}
					}
					else if (match("format"))
					{
						getstr();
						if (match("ascii")) {
							format = FORMAT_ASCII;
						}
						else if (match("binary_big_endian")) {
							format = FORMAT_BINARY_BIG_ENDIAN;
						}
						else if (match("binary_little_endian")) {
							format = FORMAT_BINARY_LITTLE_ENDIAN;
						}
					}
					else if (match("property"))
					{
						unsigned int idx = (unsigned int)elementType;
						if (idx < ELEM_INVALID)
						{
							numProperties[idx]++;
						}
					}
					else if (match("end_header"))
					{
						break;
					}
				} // end read header

				  // advance past newline
				getline();
				return true;
			}
			return false;
		}

		void loadData()
		{
			// read in each element type
			for (int eidx = 0; eidx < elementListIdx; eidx++)
			{
				elemType = elementList[eidx];
				elemNum = numElements[elemType];
				elemProperties = numProperties[elemType];

				if (elemType == ELEM_VERTEX)
				{
					// size vertex buffers
					m_vertices.reserve(elemNum);

					if (format == FORMAT_ASCII)
					{
						getVertices(
							[&]()
						{
							float val = 0.f;
							fscanf_s(file, "%f", &val);
							return val;
						},
							[&]()
						{
							getline();
						}
						);
					}
					else if (format == FORMAT_BINARY_BIG_ENDIAN)
					{
						getVertices(
							[&]()
						{
							char data0[4];
							fread(data0, sizeof(float), 1, file);
							union
							{
								char data1[4];
								float val;
							};
							data1[0] = data0[3];
							data1[1] = data0[2];
							data1[2] = data0[1];
							data1[3] = data0[0];
							return val;
						},
							[&]()
						{
						}
						);
					}
					else if (format == FORMAT_BINARY_LITTLE_ENDIAN)
					{
						getVertices(
							[&]()
						{
							float val = 0.f;
							fread(&val, sizeof(float), 1, file);
							return val;
						},
							[&]()
						{
						}
						);
					}
				}
				else if (elemType == ELEM_FACE)
				{
					// size vertex buffers
					m_indices.reserve(3 * elemNum);

					if (format == FORMAT_ASCII)
					{
						getIndices(
							[&]()
						{
							return getint();
						},
							[&]()
						{
							return getint();
						},
							[&]()
						{
							getline();
						}
						);
					}
					else if (format == FORMAT_BINARY_BIG_ENDIAN)
					{
						getIndices(
							[&]()
						{
							int val = 0;
							fread(&val, 1, 1, file);
							return val;
						},
							[&]()
						{
							char data0[4];
							fread(data0, sizeof(float), 1, file);
							union
							{
								char data1[4];
								int val;
							};
							data1[0] = data0[3];
							data1[1] = data0[2];
							data1[2] = data0[1];
							data1[3] = data0[0];
							return val;
						},
							[&]()
						{
						}
						);
					}
					else if (format == FORMAT_BINARY_LITTLE_ENDIAN)
					{
						getIndices(
							[&]()
						{
							int val = 0;
							fread(&val, 1, 1, file);
							return val;
						},
							[&]()
						{
							int val;
							fread(&val, sizeof(int), 1, file);
							return val;
						},
							[&]()
						{
						}
						);
					}
				}
				else if (elemType == ELEM_EDGE)
				{

				}
			}
		}

		// main entry point
		void operator()(const char* filename)
		{
			fopen_s(&file, filename, "rb");
			if (file)
			{
				if (parseHeader())
				{
					loadData();
				}
				fclose(file);
			}
		}
	};
}

void Mesh::loadFromPLY(const char* filename)
{
	PLYLoader loader(*this);
	loader(filename);
}