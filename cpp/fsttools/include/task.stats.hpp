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

#ifndef STATSOUTPUTTASK_HPP
#define STATSOUTPUTTASK_HPP

/** \file
 *  \brief Task that dumps statistics stored by any previous task in the pipeline.
 * \date 20-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

/// Task that reads stats from data object and writes them to a [file]
template <class Data>
class SpeedStatsTask: public ucam::util::TaskInterface<Data> {
 private:
  typedef ucam::util::oszfstream oszfstream;

  ///[file] name in which stats will be printed
  ucam::util::IntegerPatternAddress statsoutput_;

  ///Pointer to data object
  Data *d_;

 public:

  ///Constructor with RegistryPO object.
  SpeedStatsTask ( const ucam::util::RegistryPO& rg ) :
    d_ ( NULL ),
    statsoutput_ ( rg.get<std::string> ( HifstConstants::kStatsWrite ) ) {
  };

  /**
   * \brief General run method from TaskInterface. Dumps all stats to a file
   * \param &d: general Data structure containing a pointer to StatsData structure
   */
  bool run ( Data& d ) {
    if ( statsoutput_() == "" ) return false;
    d_ = &d;
    FORCELINFO ( "Writing stats to "  << statsoutput_ ( d.sidx ) );
    oszfstream o ( statsoutput_ ( d.sidx ) );
    o << "=================================================================" <<
      std::endl;
    o << "Sentence " << d.sidx << ": Time (ms):" << std::endl;
    writeSpeedStats ( o );
    o << "-----------------------------------------------------------------" <<
      std::endl;
    o << "Other:" << std::endl;
    o << d_->stats->message << std::endl;
    o << "=================================================================" <<
      std::endl;
    o.close();
    return false;
  };

 private:

  /**
   * \brief Write speed measurements associated to a key. Each line will contain:
   * key:value
   * ... where key is a string defined by the programmer of any other task value is reported in ms.
   * Keys should be unique, i.e. to measure different components we need different keys.
   * \param &o [file] stream
   */
  void writeSpeedStats ( oszfstream& o ) {
    d_->stats->write ( o );
  }
};

}
} // end namespaces
#endif
