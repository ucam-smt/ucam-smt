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






#ifndef LEXICOGRAPHICTROPICALTROPICALWEIGHTDECLS_H_
#define LEXICOGRAPHICTROPICALTROPICALWEIGHTDECLS_H_

/** \file
 * \brief Lexicographic stdarc registering
 * \date 12-10-2012
 * \author Gonzalo Iglesias
 */


using namespace fst;
using namespace fst::script;

REGISTER_FST_OPERATIONS ( LexStdArc );
REGISTER_FST ( VectorFst, LexStdArc );
REGISTER_FST ( ConstFst, LexStdArc );
REGISTER_FST_CLASSES ( LexStdArc );




#endif
