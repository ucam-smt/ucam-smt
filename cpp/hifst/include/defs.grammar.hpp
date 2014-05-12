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

#ifndef DEFS_GRAMMAR_HPP
#define DEFS_GRAMMAR_HPP

/**
 * \file
 * \brief Contains definitions for cykparser data and task
 * \date 16-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {
typedef unordered_map <std::string, uint> grammar_categories_t;
typedef unordered_map < uint, std::string > grammar_inversecategories_t;
}
}  // end namespaces

#endif
