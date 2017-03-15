/*
 * Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "imguiser.h"

#include <stdio.h>

#include <vector>

namespace
{
	std::vector<ImserNode> g_nodes;

	bool g_shouldSave = false;
	bool g_shouldLoad = false;
	bool g_shouldLoadC = false;
	bool g_saveProcessActive = false;
	bool g_loadProcessActive = false;

	char g_saveFilename[256u];
	char g_loadFilename[256u];
	const ImserNode* g_loadC_nodes = nullptr;
	unsigned int g_loadC_sizeInBytes = 0;

	const int TEXT_POOL_CAP = 16u * 1024u;
	char g_textPool[TEXT_POOL_CAP];
	int g_textPoolIdx = 0;

	char* allocText(const char* text)
	{
		if (text == nullptr)
		{
			return nullptr;
		}
		int len = (int)(strlen(text) + 1);
		if (g_textPoolIdx + len < TEXT_POOL_CAP)
		{
			char* dst = &g_textPool[g_textPoolIdx];
			memcpy(dst, text, len);
			g_textPoolIdx += len;
			return dst;
		}
		return nullptr;
	}

	struct TreeNode
	{
		int node;
		int parent;
		int childStart;
		int childEnd;
		int next;
		int childCount;
	};

	std::vector<TreeNode> g_treeNodes;
	int g_treeNodeIdxScope = 0;
	int g_treeNodeIdxSearch = 0;
	int g_treeNoMatchGroupRef = 0;

	bool treeSearch(const char* name)
	{
		if (g_treeNoMatchGroupRef != 0)
		{
			return false;
		}
		if (name != nullptr)
		{
			if (g_treeNodes[g_treeNodeIdxSearch].parent != g_treeNodeIdxScope)
			{
				g_treeNodeIdxSearch = g_treeNodes[g_treeNodeIdxScope].childStart;
			}
			int startIdx = g_treeNodeIdxSearch;
			do
			{
				int nodeIdx = g_treeNodes[g_treeNodeIdxSearch].node;
				if (nodeIdx != -1 && strcmp(g_nodes[nodeIdx].name, name) == 0)
				{
					return true;
				}
				// increment seach with wrap
				g_treeNodeIdxSearch = g_treeNodes[g_treeNodeIdxSearch].next;
				if (g_treeNodeIdxSearch == -1)
				{
					g_treeNodeIdxSearch = g_treeNodes[g_treeNodeIdxScope].childStart;
				}
			} while (startIdx != g_treeNodeIdxSearch);
		}
		else
		{
			g_treeNodeIdxSearch = g_treeNodes[g_treeNodeIdxSearch].next;
			if (g_treeNodeIdxSearch == -1)
			{
				g_treeNodeIdxSearch = g_treeNodes[g_treeNodeIdxScope].childStart;
			}
		}
		return false;
	}
}

static void imguiserDoSave(const char* filename);
static void imguiserDoLoad(const char* filename);
static void imguiserDoLoadC(const ImserNode* nodes, unsigned int sizeInBytes);

void imguiserInit()
{
	g_nodes.reserve(4096u);
	g_treeNodes.reserve(4096u);

	g_shouldSave = false;
	g_shouldLoad = false;
	g_shouldLoadC = false;
	g_saveProcessActive = false;
	g_loadProcessActive = false;
	g_saveFilename[0] = '\0';
	g_loadFilename[0] = '\0';
	g_loadC_nodes = nullptr;
	g_loadC_sizeInBytes = 0;
}

void imguiserUpdate()
{
	if (g_saveProcessActive)
	{
		imguiserDoSave(g_saveFilename);
		g_saveProcessActive = false;
	}
	if (g_loadProcessActive)
	{
		g_loadProcessActive = false;
	}
	if (g_shouldSave)
	{		
		g_saveProcessActive = true;
		g_shouldSave = false;
	}
	if (g_shouldLoad)
	{		
		imguiserDoLoad(g_loadFilename);
		if(g_shouldLoad) g_loadProcessActive = true;
		g_shouldLoad = false;
	}
	if (g_shouldLoadC)
	{
		imguiserDoLoadC(g_loadC_nodes, g_loadC_sizeInBytes);
		if (g_shouldLoadC) g_loadProcessActive = true;
		g_shouldLoadC = false;
	}
}

void imguiserDestroy()
{

}

void imguiserBeginFrame()
{
	if (!g_loadProcessActive)
	{
		g_nodes.clear();
		// reset text pool
		g_textPoolIdx = 0;
	}
	g_treeNodeIdxScope = 0;
	g_treeNodeIdxSearch = 0;
	g_treeNoMatchGroupRef = 0;
}

void imguiserEndFrame()
{

}

void imguiserBeginGroup(const char* name, int* numItems)
{
	if (g_saveProcessActive)
	{
		ImserNode group = imserNodeGroupBegin(name);

		g_nodes.push_back(group);
	}
	if (g_loadProcessActive)
	{
		// search for group with matching name
		if (treeSearch(name))
		{
			g_treeNodeIdxScope = g_treeNodeIdxSearch;
			if (g_treeNodeIdxScope == -1) g_treeNodeIdxScope = 0;
			g_treeNodeIdxSearch = g_treeNodes[g_treeNodeIdxScope].childStart;

			if (numItems)
			{
				*numItems = g_treeNodes[g_treeNodeIdxScope].childCount;
			}
		}
		else
		{
			g_treeNoMatchGroupRef++;
			g_treeNodeIdxScope++;
		}
	}
}

void imguiserEndGroup()
{
	if (g_saveProcessActive)
	{
		ImserNode group = imserNodeGroupEnd();

		g_nodes.push_back(group);
	}
	if (g_loadProcessActive)
	{
		if (g_treeNoMatchGroupRef == 0)
		{
			g_treeNodeIdxScope = g_treeNodes[g_treeNodeIdxScope].parent;
			if (g_treeNodeIdxScope == -1) g_treeNodeIdxScope = 0;
			g_treeNodeIdxSearch = 0;
		}
		else
		{
			g_treeNoMatchGroupRef--;
			g_treeNodeIdxScope--;
		}
	}
}

void imguiserValue1f(const char* text, float* val)
{
	if (g_saveProcessActive)
	{
		ImserNode item = imserNodeValue1f(text, *val);

		g_nodes.push_back(item);
	}
	if (g_loadProcessActive)
	{
		if (text != nullptr)
		{
			if (treeSearch(text))
			{
				*val = g_nodes[g_treeNodes[g_treeNodeIdxSearch].node].valFloat;
			}
		}
		else
		{
			*val = g_nodes[g_treeNodes[g_treeNodeIdxSearch].node].valFloat;
			treeSearch(nullptr);
		}
	}
}

void imguiserValueBool(const char* text, bool* val)
{
	if (g_saveProcessActive)
	{
		ImserNode item = imserNodeValueBool(text, *val);

		g_nodes.push_back(item);
	}
	if (g_loadProcessActive)
	{
		if (text != nullptr)
		{
			if (treeSearch(text))
			{
				*val = g_nodes[g_treeNodes[g_treeNodeIdxSearch].node].valBool;
			}
		}
		else
		{
			*val = g_nodes[g_treeNodes[g_treeNodeIdxSearch].node].valBool;
			treeSearch(nullptr);
		}
	}
}

bool imguiserCheck(const char* text, bool checked, bool enabled)
{
	auto ret = imguiCheck(text, checked, enabled);

	bool temp = checked;
	imguiserValueBool(text, &temp);
	if (temp != checked) ret = true;

	return ret;
}

bool imguiserSlider(const char* text, float* val, float vmin, float vmax, float vinc, bool enabled)
{
	auto ret = imguiSlider(text, val, vmin, vmax, vinc, enabled);

	float temp = *val;
	imguiserValue1f(text, val);
	if (temp != *val) ret = true;

	return ret;
}

bool imguiserOffscreenUpdate()
{
	return g_saveProcessActive || g_loadProcessActive;
}

void imguiserSave(const char* filename)
{
	g_shouldSave = true;
	int len = int(strlen(filename) + 1);
	if (len > 256u) len = 256u;
	memcpy(g_saveFilename, filename, len);
}

void imguiserLoad(const char* filename)
{
	g_shouldLoad = true;
	int len = int(strlen(filename) + 1);
	if (len > 256u) len = 256u;
	memcpy(g_loadFilename, filename, len);
}

void imguiserLoadC(const ImserNode* nodes, unsigned int sizeInBytes)
{
	g_shouldLoadC = true;
	g_loadC_nodes = nodes;
	g_loadC_sizeInBytes = sizeInBytes;
}

inline void fprintTabs(FILE* file, unsigned int count)
{
	static const char tabs[11u] = "\t\t\t\t\t\t\t\t\t\t";
	char format[] = "%.Xs";
	format[2] = '0' + (char)((count >= 10) ? 9 : count);
	fprintf(file, format, tabs);
}

static void imguiserDoSave(const char* filename)
{
	FILE* file = nullptr;
	fopen_s(&file, filename, "w");

	unsigned int tabAmount = 0;
	if (file)
	{
		fprintf(file, "const ImserNode g_root[] = {\n");
		tabAmount++;

		for (size_t i = 0u; i < g_nodes.size(); i++)
		{
			ImserNode& node = g_nodes[i];

			switch (node.typeID)
			{
			case IMSER_TYPE_GROUP_BEGIN:
				fprintTabs(file, tabAmount);
				fprintf(file, "imserNodeGroupBegin(\"%s\"),\n", node.name);
				tabAmount++;
				break;
			case IMSER_TYPE_GROUP_END:
				tabAmount--;
				fprintTabs(file, tabAmount);
				fprintf(file, "imserNodeGroupEnd(),\n");
				break;
			case IMSER_TYPE_FLOAT:
				fprintTabs(file, tabAmount);
				if(node.name) fprintf(file, "imserNodeValue1f(\"%s\", %ff),\n", node.name, node.valFloat);
				else fprintf(file, "imserNodeValue1f(nullptr, %ff),\n", node.valFloat);
				break;
			case IMSER_TYPE_BOOL:
				fprintTabs(file, tabAmount);
				if(node.name) fprintf(file, "imserNodeValueBool(\"%s\", %s),\n", node.name, node.valBool ? "true" : "false");
				else fprintf(file, "imserNodeValueBool(nullptr, %s),\n", node.valBool ? "true" : "false");
				break;
			}
		}

		fprintf(file, "};");

		fclose(file);
	}
}

inline void skipWS(FILE* file)
{
	fscanf(file, "%*[ \t\n\v\f\r]");
}

inline const char* extractName(FILE* file, char* buf)
{
	const char* name = nullptr;
	skipWS(file);
	char c = fgetc(file);
	if (c == '\"')
	{
		fscanf(file, "%255[^\"]", buf);

		name = allocText(buf);
	}
	return name;
}

inline float extractValue1f(FILE* file, char* buf)
{
	float value = 0.f;
	fscanf(file, "%f", &value);
	return value;
}

inline bool extractBool(FILE* file, char* buf)
{
	skipWS(file);
	fscanf(file, "%255[^,) \t\n\v\f\r]", buf);
	bool val = false;
	if (strcmp(buf, "true") == 0) val = true;
	return val;
}

static void generateTree()
{
	// generate tree for faster searching
	{
		g_treeNodes.clear();

		// create root node
		{
			TreeNode n;
			n.node = -1;
			n.parent = -1;
			n.childStart = -1;
			n.childEnd = -1;
			n.next = -1;
			g_treeNodes.push_back(n);
		}
		// generate tree by playing node list and tracking state
		int numNodes = (int)g_nodes.size();
		int treeNodeIdx = 0;
		for (int nodeIdx = 0; nodeIdx < numNodes; nodeIdx++)
		{
			ImserNode& node = g_nodes[nodeIdx];

			if (node.typeID == IMSER_TYPE_GROUP_BEGIN)
			{
				// create new node
				TreeNode n;
				n.node = nodeIdx;
				n.parent = treeNodeIdx;
				n.childStart = -1;
				n.childEnd = -1;
				n.next = -1;
				n.childCount = 0;

				if (g_treeNodes[treeNodeIdx].childEnd != -1)
				{
					g_treeNodes[g_treeNodes[treeNodeIdx].childEnd].next = (int)g_treeNodes.size();
				}
				else
				{
					g_treeNodes[treeNodeIdx].childStart = (int)g_treeNodes.size();
				}
				g_treeNodes[treeNodeIdx].childEnd = (int)g_treeNodes.size();
				g_treeNodes[treeNodeIdx].childCount++;

				treeNodeIdx = (int)g_treeNodes.size();

				g_treeNodes.push_back(n);
			}
			else if (node.typeID == IMSER_TYPE_GROUP_END)
			{
				// move active scope back to parent
				if (treeNodeIdx != -1)
				{
					treeNodeIdx = g_treeNodes[treeNodeIdx].parent;
				}
			}
			else if (node.typeID == IMSER_TYPE_FLOAT || node.typeID == IMSER_TYPE_BOOL)
			{
				TreeNode n;
				n.node = nodeIdx;
				n.parent = treeNodeIdx;
				n.childStart = -1;
				n.childEnd = -1;
				n.next = -1;
				n.childCount = 0;

				if (g_treeNodes[treeNodeIdx].childEnd != -1)
				{
					g_treeNodes[g_treeNodes[treeNodeIdx].childEnd].next = (int)g_treeNodes.size();
				}
				else
				{
					g_treeNodes[treeNodeIdx].childStart = (int)g_treeNodes.size();
				}
				g_treeNodes[treeNodeIdx].childEnd = (int)g_treeNodes.size();
				g_treeNodes[treeNodeIdx].childCount++;

				g_treeNodes.push_back(n);
			}
		}
	}
}

static void imguiserDoLoad(const char* filename)
{
	FILE* file = nullptr;
	fopen_s(&file, filename, "r");

	const unsigned int bufSize = 256u;
	char buf[bufSize];

	if (file)
	{
		// enable save process
		g_saveProcessActive = true;
		g_nodes.clear();
		// reset text pool
		g_textPoolIdx = 0;

		// scan to 'ImserNode'
		while (!feof(file))
		{
			fscanf(file, "%255s,", buf);
			if (strcmp("ImserNode", buf) == 0)
			{
				break;
			}
		}
		// scan to '{'
		while (!feof(file))
		{
			if (fgetc(file) == '{')
			{
				break;
			}
		}
		// start reading commands
		while (!feof(file))
		{
			// get command name
			skipWS(file);
			fscanf(file, "%255[^(](", buf);
			if (strcmp(buf, "imserNodeGroupBegin") == 0)
			{
				auto name = extractName(file, buf);
				imguiserBeginGroup(name, nullptr);
			}
			else if (strcmp(buf, "imserNodeGroupEnd") == 0)
			{
				imguiserEndGroup();
			}
			else if (strcmp(buf, "imserNodeValue1f") == 0)
			{
				auto name = extractName(file, buf);
				fscanf(file, "%[^,],", buf);
				float val = extractValue1f(file, buf);
				imguiserValue1f(name, &val);
			}
			else if (strcmp(buf, "imserNodeValueBool") == 0)
			{
				auto name = extractName(file, buf);
				fscanf(file, "%[^,],", buf);
				bool val = extractBool(file, buf);
				imguiserValueBool(name, &val);
			}

			// scan past close of command
			fscanf(file, "%[^,],", buf);
		}

		// disable save process
		g_saveProcessActive = false;

		fclose(file);
	}
	else
	{
		g_shouldLoad = false;
		return;
	}

	generateTree();
}

static void imguiserDoLoadC(const ImserNode* nodes, unsigned int sizeInBytes)
{
	// copy to g_nodes
	unsigned int numNodes = sizeInBytes / sizeof(ImserNode);

	g_nodes.clear();
	// reset text pool
	g_textPoolIdx = 0;

	g_nodes.resize(numNodes);
	for (size_t i = 0; i < g_nodes.size(); i++)
	{
		g_nodes[i] = nodes[i];
	}

	generateTree();
}

ImserNode imserNodeGroupBegin(const char* name)
{
	ImserNode group;
	group.name = name;
	group.typeID = IMSER_TYPE_GROUP_BEGIN;
	group.valBool = false;
	return group;
}

ImserNode imserNodeGroupEnd()
{
	ImserNode group;
	group.name = nullptr;
	group.typeID = IMSER_TYPE_GROUP_END;
	group.valBool = false;
	return group;
}

ImserNode imserNodeValue1f(const char* name, float value)
{
	ImserNode item;
	item.name = name;
	item.typeID = IMSER_TYPE_FLOAT;
	item.valFloat = value;
	return item;
}

ImserNode imserNodeValueBool(const char* name, bool value)
{
	ImserNode item;
	item.name = name;
	item.typeID = IMSER_TYPE_BOOL;
	item.valBool = value;
	return item;
}