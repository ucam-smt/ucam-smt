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

#ifndef TASK_LOADWORDMAP_HPP
#define TASK_LOADWORDMAP_HPP

/**
 * \file
 * \brief Wrapper around WordMapper loader
 * \date 12-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

/// Loads wordmap in constructor and delivers pointer to data object during run time
template <class Data >
class LoadWordMapTask: public ucam::util::TaskInterface<Data> {
 private:
  ///Key to store pointer to wordmap
  const std::string key_;

  ///Wordmap itself
  boost::scoped_ptr< ucam::util::WordMapper > wm_;

 public:

  ///Static constructor, will return NULL if there is no need for word-mapping
  static LoadWordMapTask* init ( const ucam::util::RegistryPO& rg ,
                                 const std::string& key ,
                                 bool reverse = false ) {
    if ( rg.exists ( key ) )
      if ( rg.get<std::string> ( key ) != "" ) return new LoadWordMapTask ( rg, key,
            reverse );
    return NULL;
  }
  ///Constructor with ucam::util::RegistryPO object
  LoadWordMapTask ( const ucam::util::RegistryPO& rg ,
                    const std::string& key ,
                    bool reverse = false ) :
    key_ ( key ),
    wm_ ( new ucam::util::WordMapper ( rg.get<std::string> ( key ), reverse ) ) {
    LINFO ( "LoadWordMapTask key=" << key << ", Done!" );
  };

  ///Delivers pointer to wordmap
  bool run ( Data& d ) {
    if (wm_ != NULL ) {
      LINFO ( "wordmap available in data object under key=" << key_ );
      d.wm[key_] = wm_.get();
    } else {
      LINFO ( "Wordmap available");
    }
    return false;
  };

 private:
  ZDISALLOW_COPY_AND_ASSIGN ( LoadWordMapTask );

};

}
}   // End namespaces
#endif
