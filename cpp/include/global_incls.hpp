// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use these files except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Copyright 2012 - Gonzalo Iglesias, Adrià de Gispert, William Byrne

/** \file include/global_incls.hpp
 * \brief All included standard headers, boost headers,...
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef GLOBAL_INCLUDE_HPP
#define GLOBAL_INCLUDE_HPP

#include <typeinfo>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <deque>
#include <sstream>
#include <ostream>
#include <set>
#include <map>
#include <queue>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <memory>

#include <unordered_map>
#include <unordered_set>

#ifndef OSR
//#define USE_GOOGLE_SPARSE_HASH
#endif

#ifdef USE_GOOGLE_SPARSE_HASH
#include <google/sparse_hash_set>
#include <google/sparse_hash_map>
#include <google/dense_hash_set>
#include <google/dense_hash_map>
#endif

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <sys/timeb.h>

#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/regex.hpp>
#include <boost/any.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/operations.hpp>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#ifdef USE_BOOSTLOG

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>

#include <boost/log/common.hpp>

#include <boost/log/attributes/timer.hpp>

#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

#endif

#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>

#include <boost/thread.hpp>
#include <boost/asio.hpp>

#endif
