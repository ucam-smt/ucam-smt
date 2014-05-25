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

#ifndef TASK_READFST_HPP
#define TASK_READFST_HPP

/**
 * \file
 * \brief Implementation of a Fst reader to data structure
 * \date 10-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

/**
 * \brief Convenience class that loads an fst using a key defined in the constructor
 * and delivers it to the data object
 */
template <class Data, class Arc = fst::StdArc >
class ReadFstTask: public ucam::util::TaskInterface<Data> {

 private:
  ///Fst filename
  ucam::util::IntegerPatternAddress fstfile_;

  ///Previous file, only reload if new file is different
  std::string previousfile_;
  ///key to store fst_
  std::string fstkey_;
  /// Pointer to Fst
  boost::scoped_ptr< fst::VectorFst<Arc> > fst_;

 public:
  ///Constructor with RegistryPO object and fstkey to access the registry object
  ReadFstTask ( const ucam::util::RegistryPO& rg ,
                const std::string& fstkey
              ) :
    fstkey_ ( fstkey ),
    fstfile_ ( rg.get<std::string> ( fstkey ) ) {
  };

  ///Static constructor. May return NULL if relevant parameters in registrypo object tell that there will be no use of this object.
  static ReadFstTask * init ( const ucam::util::RegistryPO& rg ,
                              const std::string& fstkey
                            ) {
    if ( rg.exists ( fstkey ) ) return new ReadFstTask ( rg, fstkey );
    return NULL;
  }

  /**
   * \brief Method inherited from TaskInterface. Loads an fst and stores a pointer into Data structure using a key.
   * Will check that the same file hasn't been loaded before. In this case, it just assumes the fst in memory is already valid.
   * \param &d: data structure in which the null filter is to be stored.
   * \returns false (does not break in any case the chain of tasks)
   */
  bool run ( Data& d ) {
    fst_.reset();
    if ( fstfile_ ( d.sidx ) != "" && fstfile_ ( d.sidx ) != previousfile_ ) {
      LINFO ( "Loading ... " << fstfile_ ( d.sidx ) << " with key=" << fstkey_ );
      fst_.reset ( fst::VectorFstRead<Arc> ( fstfile_ ( d.sidx ) ) );
      d.fsts[fstkey_] = fst_.get();
      previousfile_ = fstfile_ ( d.sidx );
    }
    return false;
  };

  ~ReadFstTask ( ) {
    //fst_->DeleteStates();
    fst_.reset();
    LINFO ("Shutdown!");
  }

 private:

  ZDISALLOW_COPY_AND_ASSIGN ( ReadFstTask );

};

///External Static constructor. May return NULL
///if relevant parameters in registrypo object tell that there will be no use of this object.
///Loads with different arc templating
template <class Data>
static ucam::util::TaskInterface<Data> * ReadFstInit ( const
    ucam::util::RegistryPO& rg ,
    const std::string& fstkey ,
    const std::string& arctype = HifstConstants::kHifstSemiringStdArc
                                                     ) {
  if ( rg.exists ( fstkey ) )
    if (arctype == HifstConstants::kHifstSemiringStdArc) return new
          ReadFstTask<Data, fst::StdArc> ( rg, fstkey );
    else if (arctype == HifstConstants::kHifstSemiringLexStdArc ) return
        static_cast< ucam::util::TaskInterface<Data> * > (new
            ReadFstTask<Data, fst::LexStdArc> ( rg, fstkey ) );
    else {
      LERROR ("Unknown arc type:" << arctype);
      exit (EXIT_FAILURE);
    }
  return NULL;
}

}
}  // End namespaces

#endif
