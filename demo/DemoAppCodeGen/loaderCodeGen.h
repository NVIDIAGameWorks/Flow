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

enum LoaderType
{
	eLoaderTypeDynamicLink = 0,
	eLoaderTypeStaticLink = 1
};

const unsigned int maxFunctionParams = 8u;

struct FunctionParams
{
	const char* typeName;
	const char* instName;
};

struct Function
{
	const char* retType;
	const char* method;
	unsigned int numParams;
	FunctionParams params[maxFunctionParams];
};

struct GenerateCodeParams
{
	LoaderType loaderType;
	FILE* file;
	Function* functions;
	unsigned int numFunctions;

	const char* filenameTmp;
	const char* filenameDst;
	const char* moduleNameUpperCase;
	const char* moduleNameLowerCase;
	const char* instName;
	const char* apiMarker;
};

void typedef_function_ptrs(const GenerateCodeParams* params)
{
	for (unsigned int functionIdx = 0u; functionIdx < params->numFunctions; functionIdx++)
	{
		auto& function = params->functions[functionIdx];

		fprintf(params->file, "typedef %s (*%s_ptr_t)(", function.retType, function.method);

		for (unsigned int paramIdx = 0u; paramIdx < function.numParams; paramIdx++)
		{
			fprintf(params->file, "%s %s", function.params[paramIdx].typeName, function.params[paramIdx].instName);

			if (paramIdx < function.numParams - 1) fprintf(params->file, ", ");
		}

		fprintf(params->file, ");\n");
	}

	fprintf(params->file, "\n");
}

void declare_function_ptrs(const GenerateCodeParams* params)
{
	for (unsigned int functionIdx = 0u; functionIdx < params->numFunctions; functionIdx++)
	{
		auto& function = params->functions[functionIdx];

		fprintf(params->file, "\t%s_ptr_t %s_ptr;\n", function.method, function.method);
	}
}

void implement_pointer_wrappers(const GenerateCodeParams* params)
{
	for (unsigned int functionIdx = 0u; functionIdx < params->numFunctions; functionIdx++)
	{
		auto& function = params->functions[functionIdx];

		fprintf(params->file, "%s %s(", function.retType, function.method);

		for (unsigned int paramIdx = 0u; paramIdx < function.numParams; paramIdx++)
		{
			fprintf(params->file, "%s %s", function.params[paramIdx].typeName, function.params[paramIdx].instName);

			if (paramIdx < function.numParams - 1) fprintf(params->file, ", ");
		}

		fprintf(params->file, ")\n{\n");

		fprintf(params->file, "\treturn %s.%s_ptr(", params->instName, function.method);

		for (unsigned int paramIdx = 0u; paramIdx < function.numParams; paramIdx++)
		{
			fprintf(params->file, "%s", function.params[paramIdx].instName);

			if (paramIdx < function.numParams - 1) fprintf(params->file, ", ");
		}

		fprintf(params->file, ");\n");

		fprintf(params->file, "}\n\n");
	}
}

void load_function_ptrs(const GenerateCodeParams* params)
{
	for (unsigned int functionIdx = 0u; functionIdx < params->numFunctions; functionIdx++)
	{
		auto& function = params->functions[functionIdx];

		fprintf(params->file, "\t%s.%s_ptr = ", params->instName, function.method);

		fprintf(params->file, "(%s_ptr_t", function.method);

		fprintf(params->file, ")(%sLoaderLoadFunction(&%s, \"%s\")", params->moduleNameLowerCase, params->instName, function.method);

		fprintf(params->file, ");\n");
	}
}

void load_function_ptrs_static(const GenerateCodeParams* params, const char* suffix)
{
	fprintf(params->file, "\tif (type == APP_CONTEXT_%s)\n\t{\n", suffix);

	for (unsigned int functionIdx = 0u; functionIdx < params->numFunctions; functionIdx++)
	{
		auto& function = params->functions[functionIdx];

		fprintf(params->file, "\t\t%s.%s_ptr = ", params->instName, function.method);

		fprintf(params->file, "%s%s;\n", function.method, suffix);
	}

	fprintf(params->file, "\t}\n\n");
}

void unload_function_ptrs(const GenerateCodeParams* params)
{
	for (unsigned int functionIdx = 0u; functionIdx < params->numFunctions; functionIdx++)
	{
		auto& function = params->functions[functionIdx];

		fprintf(params->file, "\t%s.%s_ptr = nullptr;\n", params->instName, function.method);
	}
}

void declare_backend_functions(const GenerateCodeParams* params, const char* suffix)
{
	for (unsigned int functionIdx = 0u; functionIdx < params->numFunctions; functionIdx++)
	{
		auto& function = params->functions[functionIdx];

		fprintf(params->file, "%s %s %s%s(", params->apiMarker, function.retType, function.method, suffix);

		for (unsigned int paramIdx = 0u; paramIdx < function.numParams; paramIdx++)
		{
			fprintf(params->file, "%s %s", function.params[paramIdx].typeName, function.params[paramIdx].instName);

			if (paramIdx < function.numParams - 1) fprintf(params->file, ", ");
		}

		fprintf(params->file, ");\n");
	}

	fprintf(params->file, "\n");
}

const char* fileHeader =
"/*\n"
"* Copyright (c) 2014-2017, NVIDIA CORPORATION.  All rights reserved.\n"
"*\n"
"* NVIDIA CORPORATION and its licensors retain all intellectual property\n"
"* and proprietary rights in and to this software, related documentation\n"
"* and any modifications thereto.  Any use, reproduction, disclosure or\n"
"* distribution of this software and related documentation without an express\n"
"* license agreement from NVIDIA CORPORATION is strictly prohibited.\n"
"*/\n\n";

void generateCode(const GenerateCodeParams* params)
{
	fprintf(params->file, fileHeader);

	typedef_function_ptrs(params);

	const char* loaderBegin =
		"struct %sLoader \n"
		"{ \n"
		"	void* module = nullptr; \n"
		"	const char* suffix = \"\"; \n"
		"	char buf[1024u]; \n\n";
	fprintf(params->file, loaderBegin, params->moduleNameUpperCase);

	declare_function_ptrs(params);

	const char* loaderEnd =
		"\n"
		"}%s; \n\n";
	fprintf(params->file, loaderEnd, params->instName);

	implement_pointer_wrappers(params);

	if (params->loaderType == eLoaderTypeDynamicLink)
	{
		const char* loaderFunction =
			"void* %sLoaderLoadFunction(%sLoader* inst, const char* name)\n"
			"{\n"
			"	snprintf(inst->buf, 1024u, \"%s%s\", name, inst->suffix);\n"
			"\n"
			"	return SDL_LoadFunction(inst->module, inst->buf);\n"
			"}\n\n";
		fprintf(params->file, loaderFunction, params->moduleNameLowerCase, params->moduleNameUpperCase, "%s", "%s");

		const char* loadModule =
			"void load%s(AppGraphCtxType type)\n"
			"{\n"
			"	const char* moduleName = demoAppDLLName(type);\n"
			"\n"
			"	%s.suffix = demoAppBackendSuffix(type);\n"
			"\n"
			"	%s.module = SDL_LoadObject(moduleName);\n\n";
		fprintf(params->file, loadModule, params->moduleNameUpperCase, params->instName, params->instName);

		load_function_ptrs(params);

		const char* unloadModule =
			"}\n"
			"\n"
			"void unload%s()\n"
			"{\n";
		fprintf(params->file, unloadModule, params->moduleNameUpperCase);

		unload_function_ptrs(params);

		const char* unloadModuleEnd =
			"\n"
			"	SDL_UnloadObject(%s.module);\n"
			"}\n";
		fprintf(params->file, unloadModuleEnd, params->instName);
	}
	else if (params->loaderType == eLoaderTypeStaticLink)
	{
		const unsigned int suffixCount = 2u;
		const char* suffix[suffixCount] = { "D3D11", "D3D12" };

		for (unsigned int suffixIdx = 0u; suffixIdx < 2u; suffixIdx++)
		{
			declare_backend_functions(params, suffix[suffixIdx]);
		}

		const char* loadModule =
			"void load%s(AppGraphCtxType type)\n"
			"{\n"
			"	%s.suffix = demoAppBackendSuffix(type);\n"
			"\n";
		fprintf(params->file, loadModule, params->moduleNameUpperCase, params->instName);

		for (unsigned int suffixIdx = 0u; suffixIdx < 2u; suffixIdx++)
		{
			load_function_ptrs_static(params, suffix[suffixIdx]);
		}

		const char* unloadModule =
			"}\n"
			"\n"
			"void unload%s()\n"
			"{\n";
		fprintf(params->file, unloadModule, params->moduleNameUpperCase);

		unload_function_ptrs(params);

		const char* unloadModuleEnd =
			"\n"
			"}\n";
		fprintf(params->file, unloadModuleEnd, params->instName);
	}
}

void fileDiffAndWriteIfModified(const GenerateCodeParams* params)
{
	FILE* fileTmp = nullptr;
	FILE* fileDst = nullptr;
	bool match = true;

	fopen_s(&fileDst, params->filenameDst, "r");
	if (fileDst)
	{
		fopen_s(&fileTmp, params->filenameTmp, "r");
		if (fileTmp)
		{
			while (1)
			{
				int a = fgetc(fileTmp);
				int b = fgetc(fileDst);

				if (a == EOF && b == EOF)
				{
					break;
				}
				else if (a != b)
				{
					match = false;
					break;
				}
			}

			fclose(fileTmp);
		}
		else
		{
			match = false;
		}
		fclose(fileDst);
	}
	else
	{
		match = false;
	}

	if (!match)
	{
		remove(params->filenameDst);
		rename(params->filenameTmp, params->filenameDst);
	}

	// always cleanup temp file
	remove(params->filenameTmp);
}