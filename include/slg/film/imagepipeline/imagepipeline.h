/***************************************************************************
 * Copyright 1998-2020 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxCoreRender.                                   *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#ifndef _SLG_IMAGEPIPELINE_H
#define	_SLG_IMAGEPIPELINE_H

#include <vector>
#include <memory>
#include <typeinfo> 

#include <bcd/core/SamplesAccumulator.h>

#include "luxrays/luxrays.h"
#include "luxrays/core/color/color.h"
#include "luxrays/devices/oclintersectiondevice.h"
#include "luxrays/utils/serializationutils.h"
#include "slg/film/imagepipeline/radiancechannelscale.h"

namespace slg {

class Film;

//------------------------------------------------------------------------------
// ImagePipelinePlugin
//------------------------------------------------------------------------------

class ImagePipelinePlugin {
public:
	ImagePipelinePlugin() { }
	virtual ~ImagePipelinePlugin() { }

	void SetRadianceChannelScale(const u_int index, const RadianceChannelScale &scale);

	virtual bool CanUseOpenCL() const { return false; }
	virtual ImagePipelinePlugin *Copy() const = 0;

	virtual void Apply(Film &film, const u_int index) = 0;
	virtual void ApplyOCL(Film &film, const u_int index) {
		throw std::runtime_error("Internal error in ImagePipelinePlugin::ApplyOCL()");
	};

#if !defined(LUXRAYS_DISABLE_OPENCL)
	static cl::Program *CompileProgram(Film &film, const std::string &kernelsParameters,
		const std::string &kernelSource, const std::string &name);
#endif

	static float GetGammaCorrectionValue(const Film &film, const u_int index);
	static u_int GetBCDPipelineIndex(const Film &film);
	static float GetBCDWarmUpSPP(const Film &film);
	static const bcd::HistogramParameters &GetBCDHistogramParameters(const Film &film);

	friend class boost::serialization::access;

private:
	template<class Archive> void serialize(Archive &ar, const u_int version);
};

//------------------------------------------------------------------------------
// ImagePipeline
//------------------------------------------------------------------------------

class ImagePipeline {
public:
	ImagePipeline();
	virtual ~ImagePipeline();

	void SetRadianceGroupCount(const u_int radianceGroupCount);
	void SetRadianceChannelScale(const u_int index, const RadianceChannelScale &scale);
	
	bool CanUseOpenCL() const { return canUseOpenCL; }

	const std::vector<ImagePipelinePlugin *> &GetPlugins() const { return pipeline; }
	// An utility method
	const ImagePipelinePlugin *GetPlugin(const std::type_info &type) const;

	ImagePipeline *Copy() const;

	void AddPlugin(ImagePipelinePlugin *plugin);
	void Apply(Film &film, const u_int index);

	friend class boost::serialization::access;

	std::vector<RadianceChannelScale> radianceChannelScales;

private:
	template<class Archive> void serialize(Archive &ar, const u_int version);

	std::vector<ImagePipelinePlugin *> pipeline;

	bool canUseOpenCL;
};

}

BOOST_SERIALIZATION_ASSUME_ABSTRACT(slg::ImagePipelinePlugin)

BOOST_CLASS_VERSION(slg::ImagePipeline, 2)
		
BOOST_CLASS_EXPORT_KEY(slg::ImagePipeline)

#endif	/*  _SLG_IMAGEPIPELINE_H */
