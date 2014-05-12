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

/**
 * \file
 * \brief Convenience functions for tropical sparse vector weight.
 * \date 15-10-2012
 * \author Gonzalo Iglesias
 */

#ifndef TROPICALSPARSETUPLEWEIGHTFUNCS_H_
#define TROPICALSPARSETUPLEWEIGHTFUNCS_H_

namespace fst {

///Traverses a machine and returns the indices actually used for the sparse vector weight tropical semiring.
inline void listSparseFeatureIndices ( VectorFst<TupleArc32>& myfst,
                                       unordered_set<uint>& idx ) {
  typedef TupleArc32::StateId StateId;
  for ( StateIterator< VectorFst<TupleArc32> > si ( myfst ); !si.Done();
        si.Next() ) {
    StateId state_id = si.Value();
    for ( MutableArcIterator< VectorFst<TupleArc32> > ai ( &myfst, si.Value() );
          !ai.Done(); ai.Next() ) {
      const TupleW32 w = ai.Value().weight;
      for ( SparseTupleWeightIterator<FeatureWeight32, int> it ( w ); !it.Done();
            it.Next() ) {
        idx.insert ( it.Value().first );
      }
    }
  }
};

}

#endif /* TROPICALSPARSETUPLEWEIGHTFUNCS_H_ */
