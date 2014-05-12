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

#ifndef MULTIEPSILONCOMPOSE_HPP
#define MULTIEPSILONCOMPOSE_HPP

/** \file
 * \brief Multiepsilon composition
 * \date 12-10-2012
 * \author Gonzalo Iglesias
 */

namespace fst {

/**
 * \brief Convenience function that performs composition with multiple epsilons.
 * \param fstlhs : left FST to compose
 * \param fstrhs : right FST to compose
 * \param multiepsilons: a vector of labels different from 0 which are to be considered as epsilons in the left FST.
 * \returns Composed Fst.
 */

template <class Arc>
ComposeFst<Arc> MultiEpsilonCompose ( const Fst<Arc>& fstlhs
                                      , const Fst<Arc>& fstrhs
                                      , const std::vector<typename Arc::Label>& multiepsilons ) {
  typedef MultiEpsMatcher< Matcher<Fst<Arc> > > MultiEpsilonMatcher;
  typedef ComposeFstOptions<Arc, MultiEpsilonMatcher> COptions;
  MultiEpsilonMatcher *mem1 = new MultiEpsilonMatcher ( fstlhs, MATCH_OUTPUT,
      kMultiEpsList );
  MultiEpsilonMatcher *mem2 = new MultiEpsilonMatcher ( fstrhs, MATCH_NONE,
      kNoLabel );
  for ( std::size_t i = 0; i < multiepsilons.size(); ++i ) {
    mem1->AddMultiEpsLabel ( multiepsilons[i] );
    //    mem2->AddMultiEpsLabel(multiepsilons[i]);
  }
  //  MultiEpsilonFilter *filter = new MultiEpsilonFilter(fstlhs, fstrhs, mem1, mem2, true);
  COptions copts (
    CacheOptions(),
    mem1,
    mem2
  );
  return ComposeFst<Arc> ( fstlhs, fstrhs, copts );
}

}; //namespaces

#endif
