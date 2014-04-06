#include <string>
namespace slg { namespace ocl {
std::string KernelSource_light_funcs = 
"#line 2 \"light_funcs.cl\"\n"
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
"//------------------------------------------------------------------------------\n"
"// ConstantInfiniteLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_CONSTANTINFINITELIGHT)\n"
"\n"
"float3 ConstantInfiniteLight_GetRadiance(__global LightSource *constantInfiniteLight,\n"
"		const float3 dir, float *directPdfA) {\n"
"	*directPdfA = 1.f / (4.f * M_PI_F);\n"
"\n"
"	return VLOAD3F(constantInfiniteLight->notIntersectable.gain.c) *\n"
"			VLOAD3F(constantInfiniteLight->notIntersectable.constantInfinite.color.c);\n"
"}\n"
"\n"
"float3 ConstantInfiniteLight_Illuminate(__global LightSource *constantInfiniteLight,\n"
"		const float worldCenterX, const float worldCenterY, const float worldCenterZ,\n"
"		const float sceneRadius,\n"
"		const float u0, const float u1, const float3 p,\n"
"		float3 *dir, float *distance, float *directPdfW) {\n"
"	const float phi = u0 * 2.f * M_PI_F;\n"
"	const float theta = u1 * M_PI_F;\n"
"	*dir = SphericalDirection(sin(theta), cos(theta), phi);\n"
"\n"
"	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);\n"
"	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;\n"
"\n"
"	const float3 toCenter = worldCenter - p;\n"
"	const float centerDistance = dot(toCenter, toCenter);\n"
"	const float approach = dot(toCenter, *dir);\n"
"	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -\n"
"		centerDistance + approach * approach));\n"
"\n"
"	const float3 emisPoint = p + (*distance) * (*dir);\n"
"	const float3 emisNormal = normalize(worldCenter - emisPoint);\n"
"\n"
"	const float cosAtLight = dot(emisNormal, -(*dir));\n"
"	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	*directPdfW = 1.f / (4.f * M_PI_F);\n"
"\n"
"	return VLOAD3F(constantInfiniteLight->notIntersectable.gain.c) *\n"
"			VLOAD3F(constantInfiniteLight->notIntersectable.constantInfinite.color.c);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// InfiniteLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"\n"
"float3 InfiniteLight_GetRadiance(__global LightSource *infiniteLight,\n"
"		__global float *infiniteLightDistirbution,\n"
"		const float3 dir, float *directPdfA\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	__global ImageMap *imageMap = &imageMapDescs[infiniteLight->notIntersectable.infinite.imageMapIndex];\n"
"	__global float *pixels = ImageMap_GetPixelsAddress(\n"
"			imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"\n"
"	const float3 localDir = normalize(Transform_InvApplyVector(&infiniteLight->notIntersectable.light2World, -dir));\n"
"	const float2 uv = (float2)(\n"
"		SphericalPhi(localDir) * (1.f / (2.f * M_PI_F)),\n"
"		SphericalTheta(localDir) * M_1_PI_F);\n"
"\n"
"	// TextureMapping2D_Map() is expended here\n"
"	const float2 scale = VLOAD2F(&infiniteLight->notIntersectable.infinite.mapping.uvMapping2D.uScale);\n"
"	const float2 delta = VLOAD2F(&infiniteLight->notIntersectable.infinite.mapping.uvMapping2D.uDelta);\n"
"	const float2 mapUV = uv * scale + delta;\n"
"\n"
"	const float distPdf = Distribution2D_Pdf(infiniteLightDistirbution, mapUV.s0, mapUV.s1);\n"
"	*directPdfA = distPdf / (4.f * M_PI_F);\n"
"\n"
"	return VLOAD3F(infiniteLight->notIntersectable.gain.c) * ImageMap_GetSpectrum(\n"
"			pixels,\n"
"			imageMap->width, imageMap->height, imageMap->channelCount,\n"
"			mapUV.s0, mapUV.s1);\n"
"}\n"
"\n"
"float3 InfiniteLight_Illuminate(__global LightSource *infiniteLight,\n"
"		__global float *infiniteLightDistirbution,\n"
"		const float worldCenterX, const float worldCenterY, const float worldCenterZ,\n"
"		const float sceneRadius,\n"
"		const float u0, const float u1, const float3 p,\n"
"		float3 *dir, float *distance, float *directPdfW\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	float2 sampleUV;\n"
"	float distPdf;\n"
"	Distribution2D_SampleContinuous(infiniteLightDistirbution, u0, u1, &sampleUV, &distPdf);\n"
"\n"
"	const float phi = sampleUV.s0 * 2.f * M_PI_F;\n"
"	const float theta = sampleUV.s1 * M_PI_F;\n"
"	*dir = normalize(Transform_ApplyVector(&infiniteLight->notIntersectable.light2World,\n"
"			SphericalDirection(sin(theta), cos(theta), phi)));\n"
"\n"
"	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);\n"
"	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;\n"
"\n"
"	const float3 toCenter = worldCenter - p;\n"
"	const float centerDistance = dot(toCenter, toCenter);\n"
"	const float approach = dot(toCenter, *dir);\n"
"	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -\n"
"		centerDistance + approach * approach));\n"
"\n"
"	const float3 emisPoint = p + (*distance) * (*dir);\n"
"	const float3 emisNormal = normalize(worldCenter - emisPoint);\n"
"\n"
"	const float cosAtLight = dot(emisNormal, -(*dir));\n"
"	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	*directPdfW = distPdf / (4.f * M_PI_F);\n"
"\n"
"	// InfiniteLight_GetRadiance is expended here\n"
"	__global ImageMap *imageMap = &imageMapDescs[infiniteLight->notIntersectable.infinite.imageMapIndex];\n"
"	__global float *pixels = ImageMap_GetPixelsAddress(\n"
"			imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"\n"
"	const float2 uv = (float2)(sampleUV.s0, sampleUV.s1);\n"
"\n"
"	// TextureMapping2D_Map() is expended here\n"
"	const float2 scale = VLOAD2F(&infiniteLight->notIntersectable.infinite.mapping.uvMapping2D.uScale);\n"
"	const float2 delta = VLOAD2F(&infiniteLight->notIntersectable.infinite.mapping.uvMapping2D.uDelta);\n"
"	const float2 mapUV = uv * scale + delta;\n"
"	\n"
"	return VLOAD3F(infiniteLight->notIntersectable.gain.c) * ImageMap_GetSpectrum(\n"
"			pixels,\n"
"			imageMap->width, imageMap->height, imageMap->channelCount,\n"
"			mapUV.s0, mapUV.s1);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// SkyLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_SKYLIGHT)\n"
"\n"
"float SkyLight_PerezBase(__global float *lam, const float theta, const float gamma) {\n"
"	return (1.f + lam[1] * exp(lam[2] / cos(theta))) *\n"
"		(1.f + lam[3] * exp(lam[4] * gamma)  + lam[5] * cos(gamma) * cos(gamma));\n"
"}\n"
"\n"
"float SkyLight_RiAngleBetween(const float thetav, const float phiv, const float theta, const float phi) {\n"
"	const float cospsi = sin(thetav) * sin(theta) * cos(phi - phiv) + cos(thetav) * cos(theta);\n"
"	if (cospsi >= 1.f)\n"
"		return 0.f;\n"
"	if (cospsi <= -1.f)\n"
"		return M_PI_F;\n"
"	return acos(cospsi);\n"
"}\n"
"\n"
"float3 SkyLight_ChromaticityToSpectrum(float Y, float x, float y) {\n"
"	float X, Z;\n"
"	\n"
"	if (y != 0.f)\n"
"		X = (x / y) * Y;\n"
"	else\n"
"		X = 0.f;\n"
"	\n"
"	if (y != 0.f && Y != 0.f)\n"
"		Z = (1.f - x - y) / y * Y;\n"
"	else\n"
"		Z = 0.f;\n"
"\n"
"	// Assuming sRGB (D65 illuminant)\n"
"	return (float3)(3.2410f * X - 1.5374f * Y - 0.4986f * Z,\n"
"			-0.9692f * X + 1.8760f * Y + 0.0416f * Z,\n"
"			0.0556f * X - 0.2040f * Y + 1.0570f * Z);\n"
"}\n"
"\n"
"float3 SkyLight_GetSkySpectralRadiance(__global LightSource *skyLight,\n"
"		const float theta, const float phi) {\n"
"	// Add bottom half of hemisphere with horizon colour\n"
"	const float theta_fin = fmin(theta, (M_PI_F * .5f) - .001f);\n"
"	const float gamma = SkyLight_RiAngleBetween(theta, phi, \n"
"			skyLight->notIntersectable.sky.absoluteTheta, skyLight->notIntersectable.sky.absolutePhi);\n"
"\n"
"	// Compute xyY values\n"
"	const float x = skyLight->notIntersectable.sky.zenith_x * SkyLight_PerezBase(\n"
"			skyLight->notIntersectable.sky.perez_x, theta_fin, gamma);\n"
"	const float y = skyLight->notIntersectable.sky.zenith_y * SkyLight_PerezBase(\n"
"			skyLight->notIntersectable.sky.perez_y, theta_fin, gamma);\n"
"	const float Y = skyLight->notIntersectable.sky.zenith_Y * SkyLight_PerezBase(\n"
"			skyLight->notIntersectable.sky.perez_Y, theta_fin, gamma);\n"
"\n"
"	return SkyLight_ChromaticityToSpectrum(Y, x, y);\n"
"}\n"
"\n"
"float3 SkyLight_GetRadiance(__global LightSource *skyLight, const float3 dir,\n"
"		float *directPdfA) {\n"
"	*directPdfA = 1.f / (4.f * M_PI_F);\n"
"\n"
"	const float theta = SphericalTheta(-dir);\n"
"	const float phi = SphericalPhi(-dir);\n"
"	const float3 s = SkyLight_GetSkySpectralRadiance(skyLight, theta, phi);\n"
"\n"
"	return VLOAD3F(skyLight->notIntersectable.gain.c) * s;\n"
"}\n"
"\n"
"float3 SkyLight_Illuminate(__global LightSource *skyLight,\n"
"		const float worldCenterX, const float worldCenterY, const float worldCenterZ,\n"
"		const float sceneRadius,\n"
"		const float u0, const float u1, const float3 p,\n"
"		float3 *dir, float *distance, float *directPdfW) {\n"
"	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);\n"
"	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;\n"
"\n"
"	const float3 localDir = normalize(Transform_ApplyVector(&skyLight->notIntersectable.light2World, -(*dir)));\n"
"	*dir = normalize(Transform_ApplyVector(&skyLight->notIntersectable.light2World,  UniformSampleSphere(u0, u1)));\n"
"\n"
"	const float3 toCenter = worldCenter - p;\n"
"	const float centerDistance = dot(toCenter, toCenter);\n"
"	const float approach = dot(toCenter, *dir);\n"
"	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -\n"
"		centerDistance + approach * approach));\n"
"\n"
"	const float3 emisPoint = p + (*distance) * (*dir);\n"
"	const float3 emisNormal = normalize(worldCenter - emisPoint);\n"
"\n"
"	const float cosAtLight = dot(emisNormal, -(*dir));\n"
"	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	return SkyLight_GetRadiance(skyLight, -(*dir), directPdfW);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Sky2Light\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_SKYLIGHT2)\n"
"\n"
"float RiCosBetween(const float3 w1, const float3 w2) {\n"
"	return clamp(dot(w1, w2), -1.f, 1.f);\n"
"}\n"
"\n"
"float3 SkyLight2_ComputeRadiance(__global LightSource *skyLight2, const float3 w) {\n"
"	const float3 absoluteSunDir = VLOAD3F(&skyLight2->notIntersectable.sky2.absoluteSunDir.x);\n"
"	const float cosG = RiCosBetween(w, absoluteSunDir);\n"
"	const float cosG2 = cosG * cosG;\n"
"	const float gamma = acos(cosG);\n"
"	const float cosT = fmax(0.f, CosTheta(w));\n"
"\n"
"	const float3 aTerm = VLOAD3F(skyLight2->notIntersectable.sky2.aTerm.c);\n"
"	const float3 bTerm = VLOAD3F(skyLight2->notIntersectable.sky2.bTerm.c);\n"
"	const float3 cTerm = VLOAD3F(skyLight2->notIntersectable.sky2.cTerm.c);\n"
"	const float3 dTerm = VLOAD3F(skyLight2->notIntersectable.sky2.dTerm.c);\n"
"	const float3 eTerm = VLOAD3F(skyLight2->notIntersectable.sky2.eTerm.c);\n"
"	const float3 fTerm = VLOAD3F(skyLight2->notIntersectable.sky2.fTerm.c);\n"
"	const float3 gTerm = VLOAD3F(skyLight2->notIntersectable.sky2.gTerm.c);\n"
"	const float3 hTerm = VLOAD3F(skyLight2->notIntersectable.sky2.hTerm.c);\n"
"	const float3 iTerm = VLOAD3F(skyLight2->notIntersectable.sky2.iTerm.c);\n"
"	const float3 radianceTerm = VLOAD3F(skyLight2->notIntersectable.sky2.radianceTerm.c);\n"
"	\n"
"	const float3 expTerm = dTerm * Spectrum_Exp(eTerm * gamma);\n"
"	const float3 rayleighTerm = fTerm * cosG2;\n"
"	const float3 mieTerm = gTerm * (1.f + cosG2) /\n"
"		Spectrum_Pow(1.f + iTerm * (iTerm - (2.f * cosG)), 1.5f);\n"
"	const float3 zenithTerm = hTerm * sqrt(cosT);\n"
"\n"
"	// 0.00001f is an arbitrary scale factor to match LuxRender intensity output\n"
"	return 0.00001f * (1.f + aTerm * Spectrum_Exp(bTerm / (cosT + .01f))) *\n"
"		(cTerm + expTerm + rayleighTerm + mieTerm + zenithTerm) * radianceTerm;\n"
"}\n"
"\n"
"float3 SkyLight2_GetRadiance(__global LightSource *skyLight2, const float3 dir,\n"
"		float *directPdfA) {\n"
"	*directPdfA = 1.f / (4.f * M_PI_F);\n"
"	const float3 s = SkyLight2_ComputeRadiance(skyLight2, -dir);\n"
"\n"
"	return VLOAD3F(skyLight2->notIntersectable.gain.c) * s;\n"
"}\n"
"\n"
"float3 SkyLight2_Illuminate(__global LightSource *skyLight2,\n"
"		const float worldCenterX, const float worldCenterY, const float worldCenterZ,\n"
"		const float sceneRadius,\n"
"		const float u0, const float u1, const float3 p,\n"
"		float3 *dir, float *distance, float *directPdfW) {\n"
"	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);\n"
"	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;\n"
"\n"
"	const float3 localDir = normalize(Transform_ApplyVector(&skyLight2->notIntersectable.light2World, -(*dir)));\n"
"	*dir = normalize(Transform_ApplyVector(&skyLight2->notIntersectable.light2World,  UniformSampleSphere(u0, u1)));\n"
"\n"
"	const float3 toCenter = worldCenter - p;\n"
"	const float centerDistance = dot(toCenter, toCenter);\n"
"	const float approach = dot(toCenter, *dir);\n"
"	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -\n"
"		centerDistance + approach * approach));\n"
"\n"
"	const float3 emisPoint = p + (*distance) * (*dir);\n"
"	const float3 emisNormal = normalize(worldCenter - emisPoint);\n"
"\n"
"	const float cosAtLight = dot(emisNormal, -(*dir));\n"
"	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	return SkyLight2_GetRadiance(skyLight2, -(*dir), directPdfW);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// SunLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_SUNLIGHT)\n"
"\n"
"float3 SunLight_Illuminate(__global LightSource *sunLight,\n"
"		const float worldCenterX, const float worldCenterY, const float worldCenterZ,\n"
"		const float sceneRadius,\n"
"		const float3 p, const float u0, const float u1,\n"
"		float3 *dir, float *distance, float *directPdfW) {\n"
"	const float cosThetaMax = sunLight->notIntersectable.sun.cosThetaMax;\n"
"	const float3 sunDir = VLOAD3F(&sunLight->notIntersectable.sun.absoluteDir.x);\n"
"	*dir = UniformSampleCone(u0, u1, cosThetaMax, VLOAD3F(&sunLight->notIntersectable.sun.x.x), VLOAD3F(&sunLight->notIntersectable.sun.y.x), sunDir);\n"
"\n"
"	// Check if the point can be inside the sun cone of light\n"
"	const float cosAtLight = dot(sunDir, *dir);\n"
"	if (cosAtLight <= cosThetaMax)\n"
"		return BLACK;\n"
"\n"
"	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);\n"
"	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;\n"
"	const float3 toCenter = worldCenter - p;\n"
"	const float centerDistance = dot(toCenter, toCenter);\n"
"	const float approach = dot(toCenter, *dir);\n"
"	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -\n"
"		centerDistance + approach * approach));\n"
"\n"
"	*directPdfW = UniformConePdf(cosThetaMax);\n"
"\n"
"	return VLOAD3F(sunLight->notIntersectable.sun.color.c);\n"
"}\n"
"\n"
"float3 SunLight_GetRadiance(__global LightSource *sunLight, const float3 dir, float *directPdfA) {\n"
"	const float cosThetaMax = sunLight->notIntersectable.sun.cosThetaMax;\n"
"	const float sin2ThetaMax = sunLight->notIntersectable.sun.sin2ThetaMax;\n"
"	const float3 x = VLOAD3F(&sunLight->notIntersectable.sun.x.x);\n"
"	const float3 y = VLOAD3F(&sunLight->notIntersectable.sun.y.x);\n"
"	const float3 absoluteSunDir = VLOAD3F(&sunLight->notIntersectable.sun.absoluteDir.x);\n"
"\n"
"	const float xD = dot(-dir, x);\n"
"	const float yD = dot(-dir, y);\n"
"	const float zD = dot(-dir, absoluteSunDir);\n"
"	if ((cosThetaMax == 1.f) || (zD < 0.f) || ((xD * xD + yD * yD) > sin2ThetaMax))\n"
"		return BLACK;\n"
"\n"
"	if (directPdfA)\n"
"		*directPdfA = UniformConePdf(cosThetaMax);\n"
"\n"
"	return VLOAD3F(sunLight->notIntersectable.sun.color.c);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// TriangleLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"\n"
"float3 TriangleLight_Illuminate(__global LightSource *triLight,\n"
"		__global HitPoint *tmpHitPoint,\n"
"		const float3 p, const float u0, const float u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float passThroughEvent,\n"
"#endif\n"
"		float3 *dir, float *distance, float *directPdfW\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float3 p0 = VLOAD3F(&triLight->triangle.v0.x);\n"
"	const float3 p1 = VLOAD3F(&triLight->triangle.v1.x);\n"
"	const float3 p2 = VLOAD3F(&triLight->triangle.v2.x);\n"
"	float b0, b1, b2;\n"
"	const float3 samplePoint = Triangle_Sample(\n"
"			p0, p1, p2,\n"
"			u0, u1,\n"
"			&b0, &b1, &b2);\n"
"\n"
"	const float3 sampleN = Triangle_GetGeometryNormal(p0, p1, p2); // Light sources are supposed to be flat\n"
"\n"
"	*dir = samplePoint - p;\n"
"	const float distanceSquared = dot(*dir, *dir);;\n"
"	*distance = sqrt(distanceSquared);\n"
"	*dir /= (*distance);\n"
"\n"
"	const float cosAtLight = dot(sampleN, -(*dir));\n"
"	if (cosAtLight < DEFAULT_COS_EPSILON_STATIC)\n"
"		return BLACK;\n"
"\n"
"	float3 emissionColor = WHITE;\n"
"#if defined(PARAM_HAS_IMAGEMAPS)\n"
"	if (triLight->triangle.imageMapIndex != NULL_INDEX) {\n"
"		// Build the local frame\n"
"		float3 X, Y;\n"
"		CoordinateSystem(sampleN, &X, &Y);\n"
"\n"
"		const float3 localFromLight = ToLocal(X, Y, sampleN, -(*dir));\n"
"\n"
"		// Retrieve the image map information\n"
"		__global ImageMap *imageMap = &imageMapDescs[triLight->triangle.imageMapIndex];\n"
"		__global float *pixels = ImageMap_GetPixelsAddress(\n"
"				imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"		const float2 uv = (float2)(SphericalPhi(localFromLight) * (1.f / (2.f * M_PI_F)), SphericalTheta(localFromLight) * M_1_PI_F);\n"
"		emissionColor = ImageMap_GetSpectrum(\n"
"			pixels,\n"
"			imageMap->width, imageMap->height, imageMap->channelCount,\n"
"			uv.s0, uv.s1) / triLight->triangle.avarage;\n"
"\n"
"		*directPdfW = triLight->triangle.invArea * distanceSquared ;\n"
"	} else\n"
"#endif\n"
"		*directPdfW = triLight->triangle.invArea * distanceSquared / cosAtLight;\n"
"\n"
"	const float2 uv0 = VLOAD2F(&triLight->triangle.uv0.u);\n"
"	const float2 uv1 = VLOAD2F(&triLight->triangle.uv1.u);\n"
"	const float2 uv2 = VLOAD2F(&triLight->triangle.uv2.u);\n"
"	const float2 triUV = Triangle_InterpolateUV(uv0, uv1, uv2, b0, b1, b2);\n"
"\n"
"	VSTORE3F(-sampleN, &tmpHitPoint->fixedDir.x);\n"
"	VSTORE3F(samplePoint, &tmpHitPoint->p.x);\n"
"	VSTORE2F(triUV, &tmpHitPoint->uv.u);\n"
"	VSTORE3F(sampleN, &tmpHitPoint->geometryN.x);\n"
"	VSTORE3F(sampleN, &tmpHitPoint->shadeN.x);\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"	tmpHitPoint->passThroughEvent = passThroughEvent;\n"
"#endif\n"
"\n"
"	return Material_GetEmittedRadiance(&mats[triLight->triangle.materialIndex],\n"
"			tmpHitPoint, triLight->triangle.invArea\n"
"			MATERIALS_PARAM) * emissionColor;\n"
"}\n"
"\n"
"float3 TriangleLight_GetRadiance(__global LightSource *triLight,\n"
"		 __global HitPoint *hitPoint, float *directPdfA\n"
"		MATERIALS_PARAM_DECL) {\n"
"	const float3 dir = VLOAD3F(&hitPoint->fixedDir.x);\n"
"	const float3 hitPointNormal = VLOAD3F(&hitPoint->geometryN.x);\n"
"	const float cosOutLight = dot(hitPointNormal, dir);\n"
"	if (cosOutLight <= 0.f)\n"
"		return BLACK;\n"
"\n"
"	if (directPdfA)\n"
"		*directPdfA = triLight->triangle.invArea;\n"
"\n"
"	float3 emissionColor = WHITE;\n"
"#if defined(PARAM_HAS_IMAGEMAPS)\n"
"	if (triLight->triangle.imageMapIndex != NULL_INDEX) {\n"
"		// Build the local frame\n"
"		float3 X, Y;\n"
"		CoordinateSystem(hitPointNormal, &X, &Y);\n"
"\n"
"		const float3 localFromLight = ToLocal(X, Y, hitPointNormal, dir);\n"
"\n"
"		// Retrieve the image map information\n"
"		__global ImageMap *imageMap = &imageMapDescs[triLight->triangle.imageMapIndex];\n"
"		__global float *pixels = ImageMap_GetPixelsAddress(\n"
"				imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"		const float2 uv = (float2)(SphericalPhi(localFromLight) * (1.f / (2.f * M_PI_F)), SphericalTheta(localFromLight) * M_1_PI_F);\n"
"		emissionColor = ImageMap_GetSpectrum(\n"
"			pixels,\n"
"			imageMap->width, imageMap->height, imageMap->channelCount,\n"
"			uv.s0, uv.s1) / triLight->triangle.avarage;\n"
"	}\n"
"#endif\n"
"\n"
"	return Material_GetEmittedRadiance(&mats[triLight->triangle.materialIndex],\n"
"			hitPoint, triLight->triangle.invArea\n"
"			MATERIALS_PARAM) * emissionColor;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// PointLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_POINTLIGHT)\n"
"\n"
"float3 PointLight_Illuminate(__global LightSource *pointLight,\n"
"		const float3 p,	float3 *dir, float *distance, float *directPdfW) {\n"
"	const float3 toLight = VLOAD3F(&pointLight->notIntersectable.point.absolutePos.x) - p;\n"
"	const float distanceSquared = dot(toLight, toLight);\n"
"	*distance = sqrt(distanceSquared);\n"
"	*dir = toLight / *distance;\n"
"\n"
"	*directPdfW = distanceSquared;\n"
"\n"
"	return VLOAD3F(pointLight->notIntersectable.point.emittedFactor.c);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// MapPointLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_MAPPOINTLIGHT)\n"
"\n"
"float3 MapPointLight_Illuminate(__global LightSource *mapPointLight,\n"
"		const float3 p,	float3 *dir, float *distance, float *directPdfW\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	const float3 toLight = VLOAD3F(&mapPointLight->notIntersectable.mapPoint.absolutePos.x) - p;\n"
"	const float distanceSquared = dot(toLight, toLight);\n"
"	*distance = sqrt(distanceSquared);\n"
"	*dir = toLight / *distance;\n"
"\n"
"	*directPdfW = distanceSquared;\n"
"\n"
"	// Retrieve the image map information\n"
"	__global ImageMap *imageMap = &imageMapDescs[mapPointLight->notIntersectable.mapPoint.imageMapIndex];\n"
"	__global float *pixels = ImageMap_GetPixelsAddress(\n"
"			imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"\n"
"	const float3 localFromLight = normalize(Transform_InvApplyVector(&mapPointLight->notIntersectable.light2World, p) - \n"
"		VLOAD3F(&mapPointLight->notIntersectable.mapPoint.localPos.x));\n"
"	const float2 uv = (float2)(SphericalPhi(localFromLight) * (1.f / (2.f * M_PI_F)), SphericalTheta(localFromLight) * M_1_PI_F);\n"
"	const float3 emissionColor = ImageMap_GetSpectrum(\n"
"			pixels,\n"
"			imageMap->width, imageMap->height, imageMap->channelCount,\n"
"			uv.s0, uv.s1) / mapPointLight->notIntersectable.mapPoint.avarage;\n"
"\n"
"	return VLOAD3F(mapPointLight->notIntersectable.mapPoint.emittedFactor.c) * emissionColor;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// SpotLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_SPOTLIGHT)\n"
"\n"
"float SpotLight_LocalFalloff(const float3 w, const float cosTotalWidth, const float cosFalloffStart) {\n"
"	if (CosTheta(w) < cosTotalWidth)\n"
"		return 0.f;\n"
" 	if (CosTheta(w) > cosFalloffStart)\n"
"		return 1.f;\n"
"\n"
"	// Compute falloff inside spotlight cone\n"
"	const float delta = (CosTheta(w) - cosTotalWidth) / (cosFalloffStart - cosTotalWidth);\n"
"	return pow(delta, 4);\n"
"}\n"
"\n"
"float3 SpotLight_Illuminate(__global LightSource *spotLight,\n"
"		const float3 p,	float3 *dir, float *distance, float *directPdfW) {\n"
"	const float3 toLight = VLOAD3F(&spotLight->notIntersectable.spot.absolutePos.x) - p;\n"
"	const float distanceSquared = dot(toLight, toLight);\n"
"	*distance = sqrt(distanceSquared);\n"
"	*dir = toLight / *distance;\n"
"\n"
"	const float3 localFromLight = normalize(Transform_InvApplyVector(\n"
"			&spotLight->notIntersectable.light2World, -(*dir)));\n"
"	const float falloff = SpotLight_LocalFalloff(localFromLight,\n"
"			spotLight->notIntersectable.spot.cosTotalWidth,\n"
"			spotLight->notIntersectable.spot.cosFalloffStart);\n"
"	if (falloff == 0.f)\n"
"		return BLACK;\n"
"\n"
"	*directPdfW = distanceSquared;\n"
"\n"
"	return VLOAD3F(spotLight->notIntersectable.spot.emittedFactor.c) *\n"
"			(falloff / fabs(CosTheta(localFromLight)));\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// ProjectionLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_PROJECTIONLIGHT)\n"
"\n"
"float3 ProjectionLight_Illuminate(__global LightSource *projectionLight,\n"
"		const float3 p,	float3 *dir, float *distance, float *directPdfW\n"
"		IMAGEMAPS_PARAM_DECL) {\n"
"	const float3 toLight = VLOAD3F(&projectionLight->notIntersectable.projection.absolutePos.x) - p;\n"
"	const float distanceSquared = dot(toLight, toLight);\n"
"	*distance = sqrt(distanceSquared);\n"
"	*dir = toLight / *distance;\n"
"\n"
"	// Check the side\n"
"	if (dot(-(*dir), VLOAD3F(&projectionLight->notIntersectable.projection.lightNormal.x)) < 0.f)\n"
"		return BLACK;\n"
"\n"
"	// Check if the point is inside the image plane\n"
"	const float3 localFromLight = normalize(Transform_InvApplyVector(\n"
"			&projectionLight->notIntersectable.light2World, -(*dir)));\n"
"	const float3 p0 = Matrix4x4_ApplyPoint(\n"
"			&projectionLight->notIntersectable.projection.lightProjection, localFromLight);\n"
"\n"
"	const float screenX0 = projectionLight->notIntersectable.projection.screenX0;\n"
"	const float screenX1 = projectionLight->notIntersectable.projection.screenX1;\n"
"	const float screenY0 = projectionLight->notIntersectable.projection.screenY0;\n"
"	const float screenY1 = projectionLight->notIntersectable.projection.screenY1;\n"
"	if ((p0.x < screenX0) || (p0.x >= screenX1) || (p0.y < screenY0) || (p0.y >= screenY1))\n"
"		return BLACK;\n"
"\n"
"	*directPdfW = distanceSquared;\n"
"\n"
"	float3 c = VLOAD3F(projectionLight->notIntersectable.projection.emittedFactor.c);\n"
"	const uint imageMapIndex = projectionLight->notIntersectable.projection.imageMapIndex;\n"
"	if (imageMapIndex != NULL_INDEX) {\n"
"		const float u = (p0.x - screenX0) / (screenX1 - screenX0);\n"
"		const float v = (p0.y - screenY0) / (screenY1 - screenY0);\n"
"		\n"
"		// Retrieve the image map information\n"
"		__global ImageMap *imageMap = &imageMapDescs[imageMapIndex];\n"
"		__global float *pixels = ImageMap_GetPixelsAddress(\n"
"				imageMapBuff, imageMap->pageIndex, imageMap->pixelsIndex);\n"
"\n"
"		c *= ImageMap_GetSpectrum(\n"
"				pixels,\n"
"				imageMap->width, imageMap->height, imageMap->channelCount,\n"
"				u, v);\n"
"	}\n"
"\n"
"	return c;\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// SharpDistantLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_SHARPDISTANTLIGHT)\n"
"\n"
"float3 SharpDistantLight_Illuminate(__global LightSource *sharpDistantLight,\n"
"		const float worldCenterX, const float worldCenterY, const float worldCenterZ,\n"
"		const float sceneRadius,\n"
"		const float3 p,	float3 *dir, float *distance, float *directPdfW) {\n"
"	*dir = -VLOAD3F(&sharpDistantLight->notIntersectable.sharpDistant.absoluteLightDir.x);\n"
"\n"
"	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);\n"
"	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;\n"
"	const float3 toCenter = worldCenter - p;\n"
"	const float centerDistance = dot(toCenter, toCenter);\n"
"	const float approach = dot(toCenter, *dir);\n"
"	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -\n"
"		centerDistance + approach * approach));\n"
"\n"
"	*directPdfW = 1.f;\n"
"\n"
"	return VLOAD3F(sharpDistantLight->notIntersectable.gain.c) *\n"
"			VLOAD3F(sharpDistantLight->notIntersectable.sharpDistant.color.c);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// DistantLight\n"
"//------------------------------------------------------------------------------\n"
"\n"
"#if defined(PARAM_HAS_DISTANTLIGHT)\n"
"\n"
"float3 DistantLight_Illuminate(__global LightSource *distantLight,\n"
"		const float worldCenterX, const float worldCenterY, const float worldCenterZ,\n"
"		const float sceneRadius,\n"
"		const float3 p,	const float u0, const float u1,\n"
"		float3 *dir, float *distance, float *directPdfW) {\n"
"	const float3 absoluteLightDir = VLOAD3F(&distantLight->notIntersectable.distant.absoluteLightDir.x);\n"
"	const float3 x = VLOAD3F(&distantLight->notIntersectable.distant.x.x);\n"
"	const float3 y = VLOAD3F(&distantLight->notIntersectable.distant.y.x);\n"
"	const float cosThetaMax = distantLight->notIntersectable.distant.cosThetaMax;\n"
"	*dir = -UniformSampleCone(u0, u1, cosThetaMax, x, y, absoluteLightDir);\n"
"\n"
"	const float3 worldCenter = (float3)(worldCenterX, worldCenterY, worldCenterZ);\n"
"	const float worldRadius = PARAM_LIGHT_WORLD_RADIUS_SCALE * sceneRadius * 1.01f;\n"
"	const float3 toCenter = worldCenter - p;\n"
"	const float centerDistance = dot(toCenter, toCenter);\n"
"	const float approach = dot(toCenter, *dir);\n"
"	*distance = approach + sqrt(max(0.f, worldRadius * worldRadius -\n"
"		centerDistance + approach * approach));\n"
"\n"
"	const float uniformConePdf = UniformConePdf(cosThetaMax);\n"
"	*directPdfW = uniformConePdf;\n"
"\n"
"	return VLOAD3F(distantLight->notIntersectable.gain.c) *\n"
"			VLOAD3F(distantLight->notIntersectable.sharpDistant.color.c);\n"
"}\n"
"\n"
"#endif\n"
"\n"
"//------------------------------------------------------------------------------\n"
"// Generic light functions\n"
"//------------------------------------------------------------------------------\n"
"\n"
"float3 EnvLight_GetRadiance(__global LightSource *light, const float3 dir, float *directPdfA\n"
"				LIGHTS_PARAM_DECL) {\n"
"	switch (light->type) {\n"
"#if defined(PARAM_HAS_CONSTANTINFINITELIGHT)\n"
"		case TYPE_IL_CONSTANT:\n"
"			return ConstantInfiniteLight_GetRadiance(light,\n"
"					dir, directPdfA);\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"		case TYPE_IL:\n"
"			return InfiniteLight_GetRadiance(light,\n"
"					&infiniteLightDistribution[light->notIntersectable.infinite.distributionOffset],\n"
"					dir, directPdfA\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined(PARAM_HAS_SKYLIGHT)\n"
"		case TYPE_IL_SKY:\n"
"			return SkyLight_GetRadiance(light,\n"
"					dir, directPdfA);\n"
"#endif\n"
"#if defined(PARAM_HAS_SKYLIGHT2)\n"
"		case TYPE_IL_SKY2:\n"
"			return SkyLight2_GetRadiance(light,\n"
"					dir, directPdfA);\n"
"#endif\n"
"#if defined(PARAM_HAS_SUNLIGHT)\n"
"		case TYPE_SUN:\n"
"			return SunLight_GetRadiance(light,\n"
"					dir, directPdfA);\n"
"#endif\n"
"#if defined(PARAM_HAS_SHARPDISTANTLIGHT)\n"
"		case TYPE_SHARPDISTANT:\n"
"			// Just return Black\n"
"#endif\n"
"#if defined(PARAM_HAS_DISTANTLIGHT)\n"
"		case TYPE_DISTANT:\n"
"			// Just return Black\n"
"#endif\n"
"\n"
"		default:\n"
"			return BLACK;\n"
"	}\n"
"}\n"
"\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"float3 IntersectableLight_GetRadiance(__global LightSource *light,\n"
"		 __global HitPoint *hitPoint, float *directPdfA\n"
"		LIGHTS_PARAM_DECL) {\n"
"	return TriangleLight_GetRadiance(light, hitPoint, directPdfA\n"
"			MATERIALS_PARAM);\n"
"}\n"
"#endif\n"
"\n"
"float3 Light_Illuminate(\n"
"		__global LightSource *light,\n"
"		const float3 point,\n"
"		const float u0, const float u1, const float u2,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"		const float u3,\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHTS)\n"
"		const float worldCenterX,\n"
"		const float worldCenterY,\n"
"		const float worldCenterZ,\n"
"		const float worldRadius,\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		__global HitPoint *tmpHitPoint,\n"
"#endif\n"
"		float3 *lightRayDir, float *distance, float *directPdfW\n"
"		LIGHTS_PARAM_DECL) {\n"
"	switch (light->type) {\n"
"#if defined(PARAM_HAS_CONSTANTINFINITELIGHT)\n"
"		case TYPE_IL_CONSTANT:\n"
"			return ConstantInfiniteLight_Illuminate(\n"
"				light,\n"
"				worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"				u0, u1,\n"
"				point,\n"
"				lightRayDir, distance, directPdfW);\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"		case TYPE_IL:\n"
"			return InfiniteLight_Illuminate(\n"
"				light,\n"
"				&infiniteLightDistribution[light->notIntersectable.infinite.distributionOffset],\n"
"				worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"				u0, u1,\n"
"				point,\n"
"				lightRayDir, distance, directPdfW\n"
"				IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined(PARAM_HAS_SKYLIGHT)\n"
"		case TYPE_IL_SKY:\n"
"			return SkyLight_Illuminate(\n"
"				light,\n"
"				worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"				u0, u1,\n"
"				point,\n"
"				lightRayDir, distance, directPdfW);\n"
"#endif\n"
"#if defined(PARAM_HAS_SKYLIGHT2)\n"
"		case TYPE_IL_SKY2:\n"
"			return SkyLight2_Illuminate(\n"
"				light,\n"
"				worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"				u0, u1,\n"
"				point,\n"
"				lightRayDir, distance, directPdfW);\n"
"#endif\n"
"#if defined(PARAM_HAS_SUNLIGHT)\n"
"		case TYPE_SUN:\n"
"			return SunLight_Illuminate(\n"
"				light,\n"
"				worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"				point, u0, u1, lightRayDir, distance, directPdfW);\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		case TYPE_TRIANGLE:\n"
"			return TriangleLight_Illuminate(\n"
"					light,\n"
"					tmpHitPoint,\n"
"					point,\n"
"					u0, u1,\n"
"#if defined(PARAM_HAS_PASSTHROUGH)\n"
"					u3,\n"
"#endif\n"
"					lightRayDir, distance, directPdfW\n"
"					MATERIALS_PARAM);\n"
"#endif\n"
"#if defined(PARAM_HAS_POINTLIGHT)\n"
"		case TYPE_POINT:\n"
"			return PointLight_Illuminate(\n"
"					light, point,\n"
"					lightRayDir, distance, directPdfW);\n"
"#endif\n"
"#if defined(PARAM_HAS_MAPPOINTLIGHT)\n"
"		case TYPE_MAPPOINT:\n"
"			return MapPointLight_Illuminate(\n"
"					light, point,\n"
"					lightRayDir, distance, directPdfW\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined(PARAM_HAS_SPOTLIGHT)\n"
"		case TYPE_SPOT:\n"
"			return SpotLight_Illuminate(\n"
"					light, point,\n"
"					lightRayDir, distance, directPdfW);\n"
"#endif\n"
"#if defined(PARAM_HAS_PROJECTIONLIGHT)\n"
"		case TYPE_PROJECTION:\n"
"			return ProjectionLight_Illuminate(\n"
"					light, point,\n"
"					lightRayDir, distance, directPdfW\n"
"					IMAGEMAPS_PARAM);\n"
"#endif\n"
"#if defined(PARAM_HAS_SHARPDISTANTLIGHT)\n"
"		case TYPE_SHARPDISTANT:\n"
"			return SharpDistantLight_Illuminate(\n"
"				light,\n"
"				worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"				point, lightRayDir, distance, directPdfW);\n"
"#endif\n"
"#if defined(PARAM_HAS_DISTANTLIGHT)\n"
"		case TYPE_DISTANT:\n"
"			return DistantLight_Illuminate(\n"
"				light,\n"
"				worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n"
"				point, u0, u1, lightRayDir, distance, directPdfW);\n"
"#endif\n"
"		default:\n"
"			return BLACK;\n"
"	}\n"
"}\n"
"\n"
"bool Light_IsEnvOrIntersectable(__global LightSource *light) {\n"
"	switch (light->type) {\n"
"#if defined(PARAM_HAS_CONSTANTINFINITELIGHT)\n"
"		case TYPE_IL_CONSTANT:\n"
"#endif\n"
"#if defined(PARAM_HAS_INFINITELIGHT)\n"
"		case TYPE_IL:\n"
"#endif\n"
"#if defined(PARAM_HAS_SKYLIGHT)\n"
"		case TYPE_IL_SKY:\n"
"#endif\n"
"#if defined(PARAM_HAS_SKYLIGHT2)\n"
"		case TYPE_IL_SKY2:\n"
"#endif\n"
"#if defined(PARAM_HAS_SUNLIGHT)\n"
"		case TYPE_SUN:\n"
"#endif\n"
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"		case TYPE_TRIANGLE:\n"
"#endif\n"
"#if defined(PARAM_HAS_SHARPDISTANTLIGHT)\n"
"		case TYPE_SHARPDISTANT:\n"
"#endif\n"
"#if defined(PARAM_HAS_DISTANTLIGHT)\n"
"		case TYPE_DISTANT:\n"
"#endif\n"
"#if defined(PARAM_HAS_CONSTANTINFINITELIGHT) || defined(PARAM_HAS_INFINITELIGHT) || defined(PARAM_HAS_SKYLIGHT) || defined(PARAM_HAS_SKYLIGHT2) || defined(PARAM_HAS_SUNLIGHT) || (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n"
"			return true;\n"
"#endif\n"
"\n"
"#if defined(PARAM_HAS_POINTLIGHT)\n"
"		case TYPE_POINT:\n"
"#endif\n"
"#if defined(PARAM_HAS_MAPPOINTLIGHT)\n"
"		case TYPE_MAPPOINT:\n"
"#endif\n"
"#if defined(PARAM_HAS_SPOTLIGHT)\n"
"		case TYPE_SPOT:\n"
"#endif\n"
"#if defined(PARAM_HAS_PROJECTIONLIGHT)\n"
"		case TYPE_PROJECTION:\n"
"#endif\n"
"#if defined(PARAM_HAS_POINTLIGHT) || defined(PARAM_HAS_MAPPOINTLIGHT) || defined(PARAM_HAS_SPOTLIGHT) || defined(PARAM_HAS_PROJECTIONLIGHT) || defined(PARAM_HAS_SHARPDISTANTLIGHT) || defined(PARAM_HAS_DISTANTLIGHT)\n"
"			return false;\n"
"#endif\n"
"\n"
"		default:\n"
"			return false;\n"
"	}\n"
"}\n"
; } }
