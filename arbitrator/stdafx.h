// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <vector>
#include <set>
#include <functional>
#include <unordered_map>
#include <filesystem>
#include <mutex> 

#include <boost/version.hpp>
#include <boost/config.hpp>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/simple_segregated_storage.hpp>
#include <boost/filesystem.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp> // generators

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <boost/numeric/ublas/matrix.hpp>


#if defined __linux__ || defined __APPLE__
#include <tbb/tbb.h>
#include <tbb/parallel_invoke.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_vector.h>
#include <cpuid.h>

#elif _WIN32
#include <execution>
#include <ppl.h>
#include <concurrent_unordered_map.h>
#include <concurrent_unordered_set.h>
#include <concurrent_vector.h>
namespace tbb = Concurrency;
#else
#endif

#include "version.h"
#include "access.h"
#include "access.info.h"
#include "external.h"
#include "urlaribitrator.h"
#include "urlaribitrator.test.h"




