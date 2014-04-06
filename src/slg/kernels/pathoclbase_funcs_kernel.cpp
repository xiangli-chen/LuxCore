#include <string>
namespace slg { namespace ocl {
std::string KernelSource_pathoclbase_funcs = 
"#line 2 \"patchoclbase_kernels.cl\"\n"
"\n"
"/***************************************************************************\n"
" * Copyright 1998-2013 by authors (see AUTHORS.txt)                        *\n"
" *                                                                         *\n"
" *   This file is part of LuxRender.                                       *\n"
" *                                                                         *\n"
" * Licensed under the Apache License, Version 2.0 (the \"License\");         *\n"
" * you may not use this file except in compliance with the License.        *\n"
" * You may obtain a copy of the License at                                 *\n"
" *                                                                         *\n"
" *     http://www.apache.org/licenses/LICENSE-2.0                          *\n"
" *                                                                         *\n"
" * Unless required by applicable law or agreed to in writing, software     *\n"
" * distributed under the License is distributed on an \"AS IS\" BASIS,       *\n"
" * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n"
" * See the License for the specific language governing permissions and     *\n"
" * limitations under the License.                                          *\n"
" ***************************************************************************/\n"
"\n"
"// List of symbols defined at compile time:\n"
"//  PARAM_TASK_COUNT\n"
"//  PARAM_RAY_EPSILON_MIN\n"
"//  PARAM_RAY_EPSILON_MAX\n"
"//  PARAM_HAS_IMAGEMAPS\n"
"//  PARAM_HAS_PASSTHROUGH\n"
"//  PARAM_USE_PIXEL_ATOMICS\n"
"//  PARAM_HAS_BUMPMAPS\n"
"//  PARAM_ACCEL_BVH or PARAM_ACCEL_MBVH or PARAM_ACCEL_QBVH or PARAM_ACCEL_MQBVH\n"
"//  PARAM_DEVICE_INDEX\n"
"//  PARAM_DEVICE_COUNT\n"
"//  PARAM_LIGHT_WORLD_RADIUS_SCALE\n"
"//  PARAM_TRIANGLE_LIGHT_COUNT\n"
"//  PARAM_LIGHT_COUNT\n"
"\n"
"// To enable single material support\n"
"//  PARAM_ENABLE_MAT_MATTE\n"
"//  PARAM_ENABLE_MAT_MIRROR\n"
"//  PARAM_ENABLE_MAT_GLASS\n"
"//  PARAM_ENABLE_MAT_METAL\n"
"//  PARAM_ENABLE_MAT_ARCHGLASS\n"
"//  PARAM_ENABLE_MAT_MIX\n"
"//  PARAM_ENABLE_MAT_NULL\n"
"//  PARAM_ENABLE_MAT_MATTETRANSLUCENT\n"
"\n"
"// To enable single texture support\n"
"//  PARAM_ENABLE_TEX_CONST_FLOAT\n"
"//  PARAM_ENABLE_TEX_CONST_FLOAT3\n"
"//  PARAM_ENABLE_TEX_CONST_FLOAT4\n"
"//  PARAM_ENABLE_TEX_IMAGEMAP\n"
"//  PARAM_ENABLE_TEX_SCALE\n"
"//  etc.\n"
"\n"
"// Film related parameters:\n"
"//  PARAM_FILM_RADIANCE_GROUP_COUNT\n"
"//  PARAM_FILM_CHANNELS_HAS_ALPHA\n"
"//  PARAM_FILM_CHANNELS_HAS_DEPTH\n"
"//  PARAM_FILM_CHANNELS_HAS_POSITION\n"
"//  PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL\n"
"//  PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL\n"
"//  PARAM_FILM_CHANNELS_HAS_MATERIAL_ID\n"
"//  PARAM_FILM_CHANNELS_HAS_DIRECT_DIFFUSE\n"
"//  PARAM_FILM_CHANNELS_HAS_DIRECT_GLOSSY\n"
"//  PARAM_FILM_CHANNELS_HAS_EMISSION\n"
"//  PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE\n"
"//  PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY\n"
"//  PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR\n"
"//  PARAM_FILM_CHANNELS_HAS_MATERIAL_ID_MASK (and PARAM_FILM_MASK_MATERIAL_ID)\n"
"//  PARAM_FILM_CHANNELS_HAS_BY_MATERIAL_ID (and PARAM_FILM_BY_MATERIAL_ID)\n"
"//  PARAM_FILM_CHANNELS_HAS_DIRECT_SHADOW_MASK\n"
"//  PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK\n"
"//  PARAM_FILM_CHANNELS_HAS_UV\n"
"//  PARAM_FILM_CHANNELS_HAS_RAYCOUNT\n"
"\n"
"// (optional)\n"
"//  PARAM_CAMERA_HAS_DOF\n"
"\n"
"// (optional)\n"
"//  PARAM_HAS_INFINITELIGHT\n"
"//  PARAM_HAS_SUNLIGHT\n"
"//  PARAM_HAS_SKYLIGHT\n"
"//  PARAM_HAS_SKYLIGHT2\n"
"//  PARAM_HAS_POINTLIGHT\n"
"//  PARAM_HAS_MAPPOINTLIGHT\n"
"//  PARAM_HAS_SPOTLIGHT\n"
"//  PARAM_HAS_PROJECTIONLIGHT\n"
"//  PARAM_HAS_CONSTANTINFINITELIGHT\n"
"//  PARAM_HAS_SHARPDISTANTLIGHT\n"
"//  PARAM_HAS_DISTANTLIGHT\n"
"//  PARAM_HAS_INFINITELIGHTS (if it has any infinite light)\n"
"//  PARAM_HAS_ENVLIGHTS (if it has any env. light)\n"
"\n"
"// (optional)\n"
"//  PARAM_HAS_NORMALS_BUFFER\n"
"//  PARAM_HAS_UVS_BUFFER\n"
"//  PARAM_HAS_COLS_BUFFER\n"
"//  PARAM_HAS_ALPHAS_BUFFER\n"
"\n"
"void AddEmission(const bool firstPathVertex, const BSDFEvent pathBSDFEvent, const uint lightID,\n"
"		__global SampleResult *sampleResult, const float3 emission) {\n"
"	VADD3F(sampleResult->radiancePerPixelNormalized[lightID].c, emission);\n"
"\n"
"	if (firstPathVertex) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_EMISSION)\n"
"		VADD3F(sampleResult->emission.c, emission);\n"
"#endif\n"
"	} else {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SHADOW_MASK)\n"
"		sampleResult->indirectShadowMask = 0.f;\n"
"#endif\n"
"		if (pathBSDFEvent & DIFFUSE) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_DIFFUSE)\n"
"			VADD3F(sampleResult->indirectDiffuse.c, emission);\n"
"#endif\n"
"		} else if (pathBSDFEvent & GLOSSY) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_GLOSSY)\n"
"			VADD3F(sampleResult->indirectGlossy.c, emission);\n"
"#endif\n"
"		} else if (pathBSDFEvent & SPECULAR) {\n"
"#if defined(PARAM_FILM_CHANNELS_HAS_INDIRECT_SPECULAR)\n"
"			VADD3F(sampleResult->indirectSpecular.c, emission);\n"
"#endif\n"
"		}\n"
"	}\n"
"}\n"
"\n"
"void MangleMemory(__global unsigned char *ptr, const size_t size) {\n"
"	Seed seed;\n"
"	Rnd_Init(7 + get_global_id(0), &seed);\n"
"\n"
"	for (uint i = 0; i < size; ++i)\n"
"		*ptr++ = (unsigned char)(Rnd_UintValue(&seed) & 0xff);\n"
"}\n"
; } }
