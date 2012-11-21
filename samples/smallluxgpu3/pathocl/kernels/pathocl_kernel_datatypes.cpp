#include "pathocl/kernels/kernels.h"
std::string luxrays::KernelSource_PathOCL_kernel_datatypes = 
"#line 2 \"patchocl_kernel_datatypes.cl\"\n"
"\n"
"/***************************************************************************\n"
" *   Copyright (C) 1998-2011 by authors (see AUTHORS.txt )                 *\n"
" *                                                                         *\n"
" *   This file is part of LuxRays.                                         *\n"
" *                                                                         *\n"
" *   LuxRays is free software; you can redistribute it and/or modify       *\n"
" *   it under the terms of the GNU General Public License as published by  *\n"
" *   the Free Software Foundation; either version 3 of the License, or     *\n"
" *   (at your option) any later version.                                   *\n"
" *                                                                         *\n"
" *   LuxRays is distributed in the hope that it will be useful,            *\n"
" *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *\n"
" *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *\n"
" *   GNU General Public License for more details.                          *\n"
" *                                                                         *\n"
" *   You should have received a copy of the GNU General Public License     *\n"
" *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *\n"
" *                                                                         *\n"
" *   LuxRays website: http://www.luxrender.net                             *\n"
" ***************************************************************************/\n"
"\n"
"// List of symbols defined at compile time:\n"
"//  PARAM_TASK_COUNT\n"
"//  PARAM_IMAGE_WIDTH\n"
"//  PARAM_IMAGE_HEIGHT\n"
"//  PARAM_RAY_EPSILON\n"
"//  PARAM_SEED\n"
"//  PARAM_MAX_PATH_DEPTH\n"
"//  PARAM_MAX_DIFFUSE_PATH_VERTEX_COUNT\n"
"//  PARAM_MAX_RR_DEPTH\n"
"//  PARAM_MAX_RR_CAP\n"
"//  PARAM_HAS_TEXTUREMAPS\n"
"//  PARAM_HAS_ALPHA_TEXTUREMAPS\n"
"//  PARAM_USE_PIXEL_ATOMICS\n"
"//  PARAM_HAS_BUMPMAPS\n"
"//  PARAM_ACCEL_BVH or PARAM_ACCEL_QBVH or PARAM_ACCEL_MQBVH\n"
"\n"
"// To enable single material suopport (work around for ATI compiler problems)\n"
"//  PARAM_ENABLE_MAT_MATTE\n"
"//  PARAM_ENABLE_MAT_AREALIGHT\n"
"//  PARAM_ENABLE_MAT_MIRROR\n"
"//  PARAM_ENABLE_MAT_GLASS\n"
"//  PARAM_ENABLE_MAT_MATTEMIRROR\n"
"//  PARAM_ENABLE_MAT_METAL\n"
"//  PARAM_ENABLE_MAT_MATTEMETAL\n"
"//  PARAM_ENABLE_MAT_ALLOY\n"
"//  PARAM_ENABLE_MAT_ARCHGLASS\n"
"\n"
"// (optional)\n"
"//  PARAM_DIRECT_LIGHT_SAMPLING\n"
"//  PARAM_DL_LIGHT_COUNT\n"
"\n"
"// (optional)\n"
"//  PARAM_CAMERA_HAS_DOF\n"
"\n"
"// (optional)\n"
"//  PARAM_HAS_INFINITELIGHT\n"
"\n"
"// (optional, requires PARAM_DIRECT_LIGHT_SAMPLING)\n"
"//  PARAM_HAS_SUNLIGHT\n"
"\n"
"// (optional)\n"
"//  PARAM_HAS_SKYLIGHT\n"
"\n"
"// (optional)\n"
"//  PARAM_IMAGE_FILTER_TYPE (0 = No filter, 1 = Box, 2 = Gaussian, 3 = Mitchell, 4 = MitchellSS)\n"
"//  PARAM_IMAGE_FILTER_WIDTH_X\n"
"//  PARAM_IMAGE_FILTER_WIDTH_Y\n"
"// (Box filter)\n"
"// (Gaussian filter)\n"
"//  PARAM_IMAGE_FILTER_GAUSSIAN_ALPHA\n"
"// (Mitchell filter) & (MitchellSS filter)\n"
"//  PARAM_IMAGE_FILTER_MITCHELL_B\n"
"//  PARAM_IMAGE_FILTER_MITCHELL_C\n"
"\n"
"// (optional)\n"
"//  PARAM_SAMPLER_TYPE (0 = Inlined Random, 1 = Random, 2 = Metropolis, 3 = Stratified)\n"
"// (Metropolis)\n"
"//  PARAM_SAMPLER_METROPOLIS_LARGE_STEP_RATE\n"
"//  PARAM_SAMPLER_METROPOLIS_MAX_CONSECUTIVE_REJECT\n"
"//  PARAM_SAMPLER_METROPOLIS_IMAGE_MUTATION_RANGE\n"
"//  PARAM_SAMPLER_METROPOLIS_DEBUG_SHOW_SAMPLE_DENSITY\n"
"// (Stratified)\n"
"//  PARAM_SAMPLER_STRATIFIED_X_SAMPLES\n"
"//  PARAM_SAMPLER_STRATIFIED_Y_SAMPLES\n"
"\n"
"// TODO: IDX_BSDF_Z used only if needed\n"
"\n"
"//#define PARAM_SAMPLER_METROPOLIS_DEBUG_SHOW_SAMPLE_DENSITY 1\n"
"\n"
"// (optional)\n"
"//  PARAM_ENABLE_ALPHA_CHANNEL\n"
"\n"
"//#pragma OPENCL EXTENSION cl_amd_printf : enable\n"
"#if defined(PARAM_USE_PIXEL_ATOMICS)\n"
"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_SUNLIGHT) & !defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"Error: PARAM_HAS_SUNLIGHT requires PARAM_DIRECT_LIGHT_SAMPLING !\n"
"#endif\n"
"\n"
"#ifndef M_PI\n"
"#define M_PI 3.14159265358979323846f\n"
"#endif\n"
"\n"
"#ifndef INV_PI\n"
"#define INV_PI  0.31830988618379067154f\n"
"#endif\n"
"\n"
"#ifndef INV_TWOPI\n"
"#define INV_TWOPI  0.15915494309189533577f\n"
"#endif\n"
"\n"
"#ifndef TRUE\n"
"#define TRUE 1\n"
"#endif\n"
"\n"
"#ifndef FALSE\n"
"#define FALSE 0\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Types\n"
"//------------------------------------------------------------------------------\n"
"\n"
"typedef struct {\n"
"	unsigned int s1, s2, s3;\n"
"} Seed;\n"
"\n"
"// Indices of Sample.u[] array\n"
"#define IDX_SCREEN_X 0\n"
"#define IDX_SCREEN_Y 1\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"#define IDX_DOF_X 2\n"
"#define IDX_DOF_Y 3\n"
"#define IDX_BSDF_OFFSET 4\n"
"#else\n"
"#define IDX_BSDF_OFFSET 2\n"
"#endif\n"
"\n"
"// Relative to IDX_BSDF_OFFSET + PathDepth * SAMPLE_SIZE\n"
"#if defined(PARAM_HAS_ALPHA_TEXTUREMAPS) && defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"\n"
"#define IDX_TEX_ALPHA 0\n"
"#define IDX_BSDF_X 1\n"
"#define IDX_BSDF_Y 2\n"
"#define IDX_BSDF_Z 3\n"
"#define IDX_DIRECTLIGHT_X 4\n"
"#define IDX_DIRECTLIGHT_Y 5\n"
"#define IDX_DIRECTLIGHT_Z 6\n"
"#define IDX_RR 7\n"
"\n"
"#define SAMPLE_SIZE 8\n"
"\n"
"#elif defined(PARAM_HAS_ALPHA_TEXTUREMAPS)\n"
"\n"
"#define IDX_TEX_ALPHA 0\n"
"#define IDX_BSDF_X 1\n"
"#define IDX_BSDF_Y 2\n"
"#define IDX_BSDF_Z 3\n"
"#define IDX_RR 4\n"
"\n"
"#define SAMPLE_SIZE 5\n"
"\n"
"#elif defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"\n"
"#define IDX_BSDF_X 0\n"
"#define IDX_BSDF_Y 1\n"
"#define IDX_BSDF_Z 2\n"
"#define IDX_DIRECTLIGHT_X 3\n"
"#define IDX_DIRECTLIGHT_Y 4\n"
"#define IDX_DIRECTLIGHT_Z 5\n"
"#define IDX_RR 6\n"
"\n"
"#define SAMPLE_SIZE 7\n"
"\n"
"#else\n"
"\n"
"#define IDX_BSDF_X 0\n"
"#define IDX_BSDF_Y 1\n"
"#define IDX_BSDF_Z 2\n"
"#define IDX_RR 3\n"
"\n"
"#define SAMPLE_SIZE 4\n"
"\n"
"#endif\n"
"\n"
"#define TOTAL_U_SIZE (IDX_BSDF_OFFSET + PARAM_MAX_PATH_DEPTH * SAMPLE_SIZE)\n"
"\n"
"typedef struct {\n"
"	Spectrum radiance;\n"
"\n"
"#if (PARAM_SAMPLER_TYPE == 0)\n"
"	uint pixelIndex;\n"
"\n"
"	// Only IDX_SCREEN_X and IDX_SCREEN_Y need to be saved\n"
"	float u[2];\n"
"#elif (PARAM_SAMPLER_TYPE == 1)\n"
"	uint pixelIndex;\n"
"\n"
"	float u[TOTAL_U_SIZE];\n"
"#elif (PARAM_SAMPLER_TYPE == 2)\n"
"	float totalI;\n"
"\n"
"	// Using ushort here totally freeze the ATI driver\n"
"	uint largeMutationCount, smallMutationCount;\n"
"	uint current, proposed, consecutiveRejects;\n"
"\n"
"	float weight;\n"
"	Spectrum currentRadiance;\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"	float currentAlpha;\n"
"#endif\n"
"\n"
"	float u[2][TOTAL_U_SIZE];\n"
"#elif (PARAM_SAMPLER_TYPE == 3)\n"
"	uint pixelIndex;\n"
"\n"
"	float stratifiedScreen2D[PARAM_SAMPLER_STRATIFIED_X_SAMPLES * PARAM_SAMPLER_STRATIFIED_Y_SAMPLES * 2];\n"
"#if defined(PARAM_CAMERA_HAS_DOF)\n"
"	float stratifiedDof2D[PARAM_SAMPLER_STRATIFIED_X_SAMPLES * PARAM_SAMPLER_STRATIFIED_Y_SAMPLES * 2];\n"
"#endif\n"
"#if defined(PARAM_HAS_ALPHA_TEXTUREMAPS)\n"
"	float stratifiedAlpha1D[PARAM_SAMPLER_STRATIFIED_X_SAMPLES];\n"
"#endif\n"
"\n"
"	float stratifiedBSDF2D[PARAM_SAMPLER_STRATIFIED_X_SAMPLES * PARAM_SAMPLER_STRATIFIED_Y_SAMPLES * 2];\n"
"	float stratifiedBSDF1D[PARAM_SAMPLER_STRATIFIED_X_SAMPLES];\n"
"\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"	float stratifiedLight2D[PARAM_SAMPLER_STRATIFIED_X_SAMPLES * PARAM_SAMPLER_STRATIFIED_Y_SAMPLES * 2];\n"
"	float stratifiedLight1D[PARAM_SAMPLER_STRATIFIED_X_SAMPLES];\n"
"#endif\n"
"\n"
"	float u[TOTAL_U_SIZE];\n"
"#endif\n"
"} Sample;\n"
"\n"
"#define PATH_STATE_GENERATE_EYE_RAY 0\n"
"#define PATH_STATE_DONE 1\n"
"#define PATH_STATE_NEXT_VERTEX 2\n"
"#define PATH_STATE_SAMPLE_LIGHT 3\n"
"\n"
"typedef struct {\n"
"	uint state;\n"
"	uint depth;\n"
" #if (PARAM_MAX_DIFFUSE_PATH_VERTEX_COUNT < PARAM_MAX_PATH_DEPTH)\n"
"	uint diffuseVertexCount;\n"
"#endif\n"
"	Spectrum throughput;\n"
"\n"
"#if defined(PARAM_DIRECT_LIGHT_SAMPLING)\n"
"	float bouncePdf;\n"
"	int specularBounce;\n"
"\n"
"	Ray nextPathRay;\n"
"	Spectrum nextThroughput;\n"
"	// Radiance to add to the result if light source is visible\n"
"	Spectrum lightRadiance;\n"
"#endif\n"
"\n"
"#if defined(PARAM_ENABLE_ALPHA_CHANNEL)\n"
"	uint vertexCount;\n"
"	float alpha;\n"
"#endif\n"
"} PathState;\n"
"\n"
"// The memory layout of this structure is highly UNoptimized for GPUs !\n"
"typedef struct {\n"
"	// The task seed\n"
"	Seed seed;\n"
"\n"
"	// The set of Samples assigned to this task\n"
"	Sample sample;\n"
"\n"
"	// The state used to keep track of the rendered path\n"
"	PathState pathState;\n"
"} GPUTask;\n"
"\n"
"typedef struct {\n"
"	uint sampleCount;\n"
"} GPUTaskStats;\n"
"\n"
"typedef struct {\n"
"	Spectrum c;\n"
"	float count;\n"
"} Pixel;\n"
"\n"
"typedef struct {\n"
"	float alpha;\n"
"} AlphaPixel;\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#define MAT_MATTE 0\n"
"#define MAT_AREALIGHT 1\n"
"#define MAT_MIRROR 2\n"
"#define MAT_GLASS 3\n"
"#define MAT_MATTEMIRROR 4\n"
"#define MAT_METAL 5\n"
"#define MAT_MATTEMETAL 6\n"
"#define MAT_ALLOY 7\n"
"#define MAT_ARCHGLASS 8\n"
"#define MAT_NULL 9\n"
"\n"
"typedef struct {\n"
"    float r, g, b;\n"
"} MatteParam;\n"
"\n"
"typedef struct {\n"
"    float gain_r, gain_g, gain_b;\n"
"} AreaLightParam;\n"
"\n"
"typedef struct {\n"
"    float r, g, b;\n"
"    int specularBounce;\n"
"} MirrorParam;\n"
"\n"
"typedef struct {\n"
"    float refl_r, refl_g, refl_b;\n"
"    float refrct_r, refrct_g, refrct_b;\n"
"    float ousideIor, ior;\n"
"    float R0;\n"
"    int reflectionSpecularBounce, transmitionSpecularBounce;\n"
"} GlassParam;\n"
"\n"
"typedef struct {\n"
"	MatteParam matte;\n"
"	MirrorParam mirror;\n"
"	float matteFilter, totFilter, mattePdf, mirrorPdf;\n"
"} MatteMirrorParam;\n"
"\n"
"typedef struct {\n"
"    float r, g, b;\n"
"    float exponent;\n"
"    int specularBounce;\n"
"} MetalParam;\n"
"\n"
"typedef struct {\n"
"	MatteParam matte;\n"
"	MetalParam metal;\n"
"	float matteFilter, totFilter, mattePdf, metalPdf;\n"
"} MatteMetalParam;\n"
"\n"
"typedef struct {\n"
"    float diff_r, diff_g, diff_b;\n"
"    float refl_r, refl_g, refl_b;\n"
"    float exponent;\n"
"    float R0;\n"
"    int specularBounce;\n"
"} AlloyParam;\n"
"\n"
"typedef struct {\n"
"    float refl_r, refl_g, refl_b;\n"
"    float refrct_r, refrct_g, refrct_b;\n"
"	float transFilter, totFilter, reflPdf, transPdf;\n"
"	bool reflectionSpecularBounce, transmitionSpecularBounce;\n"
"} ArchGlassParam;\n"
"\n"
"typedef struct {\n"
"	unsigned int type;\n"
"	union {\n"
"		MatteParam matte;\n"
"        AreaLightParam areaLight;\n"
"		MirrorParam mirror;\n"
"        GlassParam glass;\n"
"		MatteMirrorParam matteMirror;\n"
"        MetalParam metal;\n"
"        MatteMetalParam matteMetal;\n"
"        AlloyParam alloy;\n"
"        ArchGlassParam archGlass;\n"
"	} param;\n"
"} Material;\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"typedef struct {\n"
"	Point v0, v1, v2;\n"
"	Vector normal;\n"
"	float area;\n"
"	float gain_r, gain_g, gain_b;\n"
"} TriangleLight;\n"
"\n"
"typedef struct {\n"
"	float shiftU, shiftV;\n"
"	Spectrum gain;\n"
"	uint width, height;\n"
"} InfiniteLight;\n"
"\n"
"typedef struct {\n"
"	Vector sundir;\n"
"	Spectrum gain;\n"
"	float turbidity;\n"
"	float relSize;\n"
"	// XY Vectors for cone sampling\n"
"	Vector x, y;\n"
"	float cosThetaMax;\n"
"	Spectrum suncolor;\n"
"} SunLight;\n"
"\n"
"typedef struct {\n"
"	Spectrum gain;\n"
"	float thetaS;\n"
"	float phiS;\n"
"	float zenith_Y, zenith_x, zenith_y;\n"
"	float perez_Y[6], perez_x[6], perez_y[6];\n"
"} SkyLight;\n"
"\n"
"//------------------------------------------------------------------------------\n"
"\n"
"typedef struct {\n"
"	unsigned int rgbPage, rgbPageOffset;\n"
"	unsigned int alphaPage, alphaPageOffset;\n"
"	unsigned int width, height;\n"
"} TexMap;\n"
"\n"
"typedef struct {\n"
"	uint vertsOffset;\n"
"	uint trisOffset;\n"
"\n"
"	float trans[4][4];\n"
"	float invTrans[4][4];\n"
"} Mesh;\n"
"\n"
"typedef struct {\n"
"	float lensRadius;\n"
"	float focalDistance;\n"
"	float yon, hither;\n"
"\n"
"	float rasterToCameraMatrix[4][4];\n"
"	float cameraToWorldMatrix[4][4];\n"
"} Camera;\n"
;
