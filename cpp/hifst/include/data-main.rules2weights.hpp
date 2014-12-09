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

#pragma once

/** \file
 *\brief Data object for alilats to sparse weight lats binary
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

/**
 *\brief Data class containing relevant variables. To be used as template for task classes using it.
 *
 */
template <class KenLMModelT = void >
class RuleIdsToSparseWeightLatsData {
 public:
  RuleIdsToSparseWeightLatsData()
      : sidx ( 0 )
      , weights(NULL)
  {};

  /// Sentence index
  unsigned sidx;

  // Weights:
  typedef unordered_map<unsigned,TupleArc32::Weight> WeightsTable;
  typedef WeightsTable::const_iterator WeightsTableIt;
  WeightsTable *weights;
  // pointers to lattices
  unordered_map<std::string, void * > fsts;
};

}} // end namespaces


