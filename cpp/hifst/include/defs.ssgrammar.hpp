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

#ifndef SENTENCESPECIFICGRAMMARDEFS_HPP
#define SENTENCESPECIFICGRAMMARDEFS_HPP

/**
 * \file
 * \brief Contains definitions for sentence-specific grammar data and task.
 * \date 16-8-2012
 * \author Gonzalo Iglesias
 * \remark This file has been reviewed/modified by:
 */

namespace ucam {
namespace hifst {

typedef std::basic_string<uint> ssgrammar_listofrules_t;
typedef unordered_map<std::string, ssgrammar_listofrules_t >
ssgrammar_firstelementmap_t;
typedef unordered_map<uint, ssgrammar_firstelementmap_t > ssgrammar_rulesmap_t;
typedef unordered_map<std::string, std::vector< pair <uint, uint> > >
ssgrammar_instancemap_t;

}
} // end namespaces

#endif
