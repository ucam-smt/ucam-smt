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

#ifndef TASK_WRITEFST_HPP
#define TASK_WRITEFST_HPP

/**
 * \file
 * \brief Implementation of a Fst writer taking the fst from data object
 * \date 10-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

/**
 * \brief Convenience class that inherits Taskinterface behaviour and writes an fst to [file] using a key defined
 * in the constructor. The key is used to access the registry object (i.e. actual program option telling where to write the fst)
 * and a pointer in the data object, telling where to read the fst from.
 */
template <class Data, class Arc = fst::StdArc >
class WriteFstTask: public ucam::util::TaskInterface<Data> {
  typedef typename Arc::Label Label;
  typedef typename Arc::Weight Weight;

 private:
  ///key to access fst in the data object
  std::string fstkey_;
  std::string readfstkey_;
  ///Fst filename
  ucam::util::IntegerPatternAddress fstfile_;

 public:
  ///Constructor with RegistryPO object
  WriteFstTask ( const ucam::util::RegistryPO& rg
                 , const std::string& fstkey
                 , const std::string& readfstkey = ""
               )
    : fstkey_ ( fstkey )
    , readfstkey_ (readfstkey != "" ? readfstkey : fstkey)
    , fstfile_ ( rg.get<std::string> ( fstkey ) ) {
  };

  inline static WriteFstTask * init ( const ucam::util::RegistryPO& rg
                                      , const std::string& fstkey
                                      , const std::string& readfstkey = ""
                                    ) {
    if ( rg.exists ( fstkey ) ) return new WriteFstTask ( rg, fstkey, readfstkey );
    return NULL;
  };

  /**
   * \brief Method inherited from TaskInterface. Stores fst to [file].
   * The fst is accessed via data object using access key fstkey_. If parentheses exist, then the will be dumped too, with extra extension .parens
   * \param &d: data object
   * \returns false (does not break in any case the chain of tasks)
   */
  inline bool run ( Data& d ) {
    if ( fstfile_ ( d.sidx ) == "" ) return false;
    if ( d.fsts.find ( readfstkey_ ) == d.fsts.end() ) {
      LERROR ( "fst with key="  << readfstkey_ << " does not exist!" );
      exit ( EXIT_FAILURE );
    }

    LINFO ( "Fst with key=" << readfstkey_);
    FORCELINFO ("Writing lattice " << d.sidx << " to ... " << fstfile_( d.sidx ) );
    fst::FstWrite<Arc> ( * ( static_cast< fst::Fst<Arc> *>
                             ( d.fsts[readfstkey_] ) ), fstfile_ ( d.sidx ) );
    std::string parenskey = readfstkey_ + ".parens";
    if ( d.fsts.find ( parenskey ) != d.fsts.end() )
      fst::WriteLabelPairs (fstfile_ ( d.sidx )  + ".parens",
                            * (static_cast< std::vector<pair<Label, Label> > * > (  d.fsts[parenskey] ) ) );
    return false;
  };

 private:

  ZDISALLOW_COPY_AND_ASSIGN ( WriteFstTask );

};

}
}  // end namespaces

#endif
