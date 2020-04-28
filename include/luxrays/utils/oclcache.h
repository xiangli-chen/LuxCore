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

#ifndef _LUXRAYS_OPENCLCACHE_H
#define	_LUXRAYS_OPENCLCACHE_H

#include <string>
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

#if defined(LUXRAYS_ENABLE_OPENCL)

#include "luxrays/utils/ocl.h"

namespace luxrays {

class oclKernelCache {
public:
	oclKernelCache() { }
	virtual ~oclKernelCache() { }

	virtual cl::Program *Compile(cl::Context &context, cl::Device &device,
		const std::vector<std::string> &kernelsParameters, const std::string &kernelSource,
		bool *cached, cl::STRING_CLASS *error) = 0;

	static std::string ToOptsString(const std::vector<std::string> &kernelsParameters);
	static cl::Program *ForcedCompile(cl::Context &context, cl::Device &device,
		const std::vector<std::string> &kernelsParameters, const std::string &kernelSource,
		cl::STRING_CLASS *error);
};

class oclKernelDummyCache : public oclKernelCache {
public:
	oclKernelDummyCache() { }
	~oclKernelDummyCache() { }

	cl::Program *Compile(cl::Context &context, cl::Device &device,
		const std::vector<std::string> &kernelsParameters, const std::string &kernelSource,
		bool *cached, cl::STRING_CLASS *error) {
		if (cached)
			*cached = false;

		return ForcedCompile(context, device, kernelsParameters, kernelSource, error);
	}
};

// WARNING: this class is not thread safe !
class oclKernelPersistentCache : public oclKernelCache {
public:
	oclKernelPersistentCache(const std::string &applicationName);
	~oclKernelPersistentCache();

	cl::Program *Compile(cl::Context &context, cl::Device &device,
		const std::vector<std::string> &kernelsParameters, const std::string &kernelSource,
		bool *cached, cl::STRING_CLASS *error);

	static std::string HashString(const std::string &ss);
	static u_int HashBin(const char *s, const size_t size);

	static std::string SanitizeFileName(const std::string &name);
	static boost::filesystem::path GetCacheDir(const std::string &applicationName);

private:
	std::string appName;
};

}

#endif

#endif	/* _LUXRAYS_OPENCLCACHE_H */

