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

#ifndef STATS_HPP
#define STATS_HPP
/** \file
 * \brief Relative to Stats across the pipeline
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace fsttools {

/**
 * \struct StatsData
 * \brief Contains data for statistics, i.e. allows timing actions and methods called during execution.
 * \remark Timing in ms. Each measurement is assigned a specific key. The key is expected to be meaningful to the user.
 * Finally, methods provided to dump a list of pairs:
 * key1:time1
 * key2:time2
 * ...
 *
 */

struct StatsData {

  StatsData() :
    lpcount ( 0 ),
    numcats ( 0 ) {
  }

  ///local pruning
  unsigned lpcount;
  ///number of syntactic categories.
  unsigned numcats;

  ///Stores absolute time for a key prior to executing function
  unordered_map<std::string, std::vector<timeb> > time1;
  ///Stores absolute time for a key after executing function
  unordered_map<std::string, std::vector<timeb> > time2;

  /// cyk rule counts
  unordered_map<unsigned, unsigned> rulecounts;

  ///number of states for full and pruned lattices through the cyk grid
  unordered_map<unsigned, unsigned> numstates;
  unordered_map<unsigned, unsigned> numprunedstates;

  /// Any other general stuff appended here -- to be printed in stats file.
  std::string message;

  ///Store absolute timing value last thing, just before executing
  inline void setTimeStart ( const std::string& key ) {
    timeb t;
    time1[key].push_back ( t );
    ftime ( &time1[key][time1[key].size() - 1] );
  };
  ///Store absolute timing value right after an execution
  inline void setTimeEnd ( const std::string& key ) {
    timeb t;
    ftime ( &t );
    time2[key].push_back ( t );
  };

  /**
   *
   * \brief Dumps time measurements as a list of pairs
   * key1:time1
   * key2:time2
   * ...
   * Each key is expected to be semantically related to the function(s). Time in ms.
   * \param o File or pipe to dump timings
   */
  void write ( ucam::util::oszfstream& o ) {
    for ( unordered_map<std::string, std::vector<timeb> >::iterator itx =
            time2.begin(); itx != time2.end(); itx++ ) {
      USER_CHECK ( time1[itx->first].size() == itx->second.size(),
                   "Mismatch with SpeedStats (each setTimeStart needs a setTimeEnd" );
      o << std::setw ( 30 ) << setiosflags ( std::ios::right ) << itx->first << ":";
      int64 fst_time = 0;
      for ( unsigned k = 0; k < itx->second.size() - 1 ; ++k ) {
        fst_time += ( itx->second[k].time - time1[itx->first][k].time ) * 1000;
        fst_time += itx->second[k].millitm - time1[itx->first][k].millitm;
      }
      int64 last_time = ( itx->second[itx->second.size() - 1].time -
                          time1[itx->first][itx->second.size() - 1].time ) * 1000
                        + itx->second[itx->second.size() - 1].millitm -
                        time1[itx->first][itx->second.size() - 1].millitm;
      fst_time += last_time;
      o << std::setw ( 10 ) << last_time;
      o << std::setw ( 10 ) << fst_time ;
      o << " (" << itx->second.size() << " times )" << endl;
    }
  }
};

}
}  // end namespaces

#endif

