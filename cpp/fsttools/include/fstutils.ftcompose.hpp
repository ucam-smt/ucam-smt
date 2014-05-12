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

#ifndef FTCOMPOSE_HPP
#define FTCOMPOSE_HPP

/**
 * \file
 * \brief Implementation of different type of compositions (i.e. failure transitions)
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace fst {

/**
 * \brief Performs composition with PHI, based on OpenFST matchers
 * PHI transitions are expected on fstrhs
 * \param fstlhs: left FST
 * \param fstrhs: right FST, potentially using kPhiLabel
 * \param kSpecialLabel: special Failure transition label phi (recommend to use PHI, see ../include/global_decls.hpp)
 */
template <class Arc>
ComposeFst<Arc> RPhiCompose ( const Fst<Arc>& fstlhs,
                              const Fst<Arc>& fstrhs,
                              const typename Arc::Label kSpecialLabel ) {
  typedef PhiMatcher< Matcher< Fst<Arc> > > PM;
  ComposeFstOptions<Arc, PM> copts (
    CacheOptions(),
    new PM ( fstlhs, MATCH_NONE,  kNoLabel ),
    new PM ( fstrhs, MATCH_INPUT, kSpecialLabel ) );
  return ComposeFst<Arc> ( fstlhs, fstrhs, copts );
};

/**
 * \brief Performs composition with RHO, based on OpenFST matchers
 * RHO transitions are expected on fstrhs
 * \param fstlhs: left FST
 * \param fstrhs: right FST, potentially using kPhiLabel
 * \param kSpecialLabel: special Failure transition label rho (recommend to use RHO, see ../include/global_decls.hpp)
 */

template<class Arc>
inline ComposeFst<Arc> RRhoCompose ( const VectorFst<Arc>& fstlhs,
                                     const VectorFst<Arc>& fstrhs,
                                     const typename Arc::Label kSpecialLabel = RHO ) {
  typedef RhoMatcher< Matcher< Fst<Arc> > > RM;
  ComposeFstOptions<Arc, RM> copts (
    CacheOptions(),
    new RM ( fstlhs, MATCH_NONE,  kNoLabel ),
    new RM ( fstrhs, MATCH_INPUT, kSpecialLabel, MATCHER_REWRITE_ALWAYS ) );
  //                            new RM(fstrhs, MATCH_INPUT, kSpecialLabel));
  return ComposeFst<Arc> ( fstlhs, fstrhs, copts );
};

}; //namespaces

#endif
