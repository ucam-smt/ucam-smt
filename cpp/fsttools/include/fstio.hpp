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
 * \brief Contains convenience functions to write and read fsts.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef FSTIO_HPP
#define FSTIO_HPP

#include <fst/script/print.h>

namespace fst {

/**
 * \brief Templated method that writes an fst in openfst text format to a stream.
 * \param fst: fst we want to write
 * \param os: Generic output stream.
 */
template< class Arc>
inline void PrintFst ( const Fst<Arc>& fst, std::ostream *os ) {
  script::PrintFst<Arc>(fst, *os);

  // FstPrinter<Arc> printer ( fst, 0, 0, 0, false, false );
  // ///What is this string for?
  // std::string foo;
  // printer.Print ( os, foo );
};

/**
 * \brief Detect trivially by extension whether it is an fst or not
 */
inline bool DetectFstFile(std::string const &filename
                   , std::string const &extname ="fst") {
  using ucam::util::ends_with;

  return (ends_with ( filename, "." + extname + ".gz" )
          || ends_with ( filename, "." + extname )
          )
      ? true: false;
};

/**
 * \brief Templated method that reads an fst
 * \param filename: binary [file] to read from.
 * \returns A generic pointer to an fst. This pointer must be deleted.
 */

template < class Arc >
inline Fst<Arc> *FstRead ( const std::string& filename ) {
  ucam::util::iszfstream file ( filename );
  FstReadOptions fro;
  Fst<Arc> *h = Fst<Arc>::Read ( *file.getStream(), fro );
  USER_CHECK ( h, "Error while reading an FST" );
  return h;
};

/**
 * \brief Templated method that reads VectorFst
 * \param filename: binary [file] to read from.
 * \returns pointer VectorFst<StdArc>.
 */
template < class Arc >
inline VectorFst<Arc> *VectorFstRead ( const std::string& filename ) {
  ucam::util::iszfstream file ( filename );
  FstReadOptions fro;
  VectorFst<Arc> *h = VectorFst<Arc>::Read ( *file.getStream(), fro );
  USER_CHECK ( h,
               "Error while reading an FST (is it a vector fst, is the semiring correct?" );
  return h;
};

/**
 * \brief Templated method that reads ConstFst
 * \param filename: binary [file] to read from.
 * \returns pointer ConstFst<StdArc>.
 */

template < class Arc >
inline ConstFst<Arc> *ConstFstRead ( const std::string& filename ) {
  ucam::util::iszfstream file ( filename );
  FstReadOptions fro;
  ConstFst<Arc> *h = ConstFst<Arc>::Read ( *file.getStream(), fro );
  USER_CHECK ( h,
               "Error while reading an FST (is it a const fst, is the semiring correct?" );
  return h;
};

/**
 * \brief Templated method that writes an fst either in binary or text format.
 * \param fst          Generic fst
 * \param filename     [file] to write to.
 * \param fstname      fst extension, defaults to "fst"
 */

template <class Arc>
inline void FstWrite ( const Fst<Arc>& fst
                       , const std::string& filename
                       , const std::string& txtname = "txt" ) {
  if ( filename == "/dev/null" ) return;
  LDEBUG ("Started..." << filename);
  //boost::scoped_ptr<oszfstream> file ( new oszfstream ( filename ) );
  ucam::util::oszfstream file (filename);
  if (!file.is_open() ) {
    LERROR ("Error opening " << filename);
    exit (EXIT_FAILURE);
  }
  if ( filename != "-"
       && filename != "/dev/stdout"
       && filename != "/dev/stderr"
       && (ucam::util::ends_with ( filename, "." + txtname + ".gz" )
           || ucam::util::ends_with ( filename, "." + txtname )
          )
     ) {
    PrintFst<Arc> ( fst, file.getStream() );
    file.close();
    return;
  }
  fst.Write ( *file.getStream(), FstWriteOptions (filename) );
  file.close();
  LDEBUG ("Finished...");
};

/**
 * \brief Struct template that represents a hypothesis in a lattice.
 */
template<class Arc>
struct Hyp {
  std::basic_string<unsigned> hyp, ohyp;
  typename Arc::Weight cost;

  Hyp (std::basic_string<unsigned> const& h
       , std::basic_string<unsigned> const &oh
       , typename Arc::Weight const& c)
    : hyp (h)
    , ohyp(oh)
    , cost (c) {
  }

  Hyp (Hyp<Arc> const& h)
    : hyp (h.hyp)
    , ohyp(h.ohyp)
    , cost (h.cost) {
  }
};

/**
 * \brief Traverses an fst and stores the hypotheses
 * in a vector. Typically the fst is the result of ShortestPath.
 * The fst should not have cycles.
 * \param fst The input fst, typically a lattice resulting from ShortestPath,
 * without cycle.
 * \param hyps The resulting hypotheses.
 */
template <class Arc, class HypT>
void printStrings (const VectorFst<Arc>& fst, std::vector<HypT>* hyps) {

  typename Arc::StateId start = fst.Start();
  for (ArcIterator<VectorFst<Arc> > ai (fst, start); !ai.Done(); ai.Next() ) {
    std::basic_string<unsigned> hyp, ohyp;
    Arc a = ai.Value();
    hyp.push_back (a.ilabel);
    ohyp.push_back (a.olabel);
    typename Arc::Weight w = a.weight;
    typename Arc::StateId nextState = a.nextstate;
    ArcIterator<VectorFst<Arc> >* ai2 =
      new ArcIterator<VectorFst<Arc> > (fst, nextState);
    while (!ai2->Done() ) {
      Arc a2 = ai2->Value();
      hyp.push_back (a2.ilabel);
      ohyp.push_back (a2.olabel);
      w = Times (w, a2.weight);
      nextState = a2.nextstate;
      delete ai2;
      ai2 = new ArcIterator<VectorFst<Arc> > (fst, nextState);
    }
    delete ai2;
    hyps->push_back (HypT (hyp, ohyp, w) );
  }
}

} // end namespace

#endif // FSTIO_HPP
