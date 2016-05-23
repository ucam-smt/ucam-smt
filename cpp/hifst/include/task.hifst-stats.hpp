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

#ifndef TASK_HIFST_STATS_HPP
#define TASK_HIFST_STATS_HPP

/** \file
 *  \brief Task that dumps statistics related specifically to the tool hifst
 * \date 19-10-2012
 * \author Gonzalo Iglesias
 */

#include "task.stats.hpp"

namespace ucam {
namespace hifst {

/**
 * \brief Reads StatsData and dumps all stats to (sentence-specific) file.
 * Provides a special method for cyk data: dumps a grid in text format with relevant information per cell.
 *
 */

template <class Data>
class HifstStatsTask: public ucam::util::TaskInterface<Data> {
 private:
  typedef ucam::util::oszfstream oszfstream;

  ucam::util::IntegerPatternAddress statsoutput_;

  Data *d_;
  float width_;
  bool writeCYKStats_;

 public:

  ///Constructor with RegistryPO object.
  HifstStatsTask ( const ucam::util::RegistryPO& rg ) :
    d_ ( NULL ),
    writeCYKStats_ ( rg.getBool ( HifstConstants::kStatsHifstCykgridEnable  ) ),
    width_ ( rg.get<unsigned> ( HifstConstants::kStatsHifstCykgridCellwidth ) ),
    statsoutput_ ( rg.get<std::string> ( HifstConstants::kStatsHifstWrite ) ) {
    this->appendTask ( new ucam::fsttools::SpeedStatsTask<Data> ( rg ) );
  };

  /**
   * \brief General run method from TaskInterface. Dumps all stats to a file
   * \param &d: general Data structure containing a pointer to StatsData structure
   */
  bool run ( Data& d ) {
    if ( statsoutput_() == "" ) return false;
    d_ = &d;
    FORCELINFO ( "Writing stats to "  << statsoutput_ ( d.sidx ) );
    ucam::util::oszfstream o ( statsoutput_ ( d.sidx ) );
    if ( writeCYKStats_ )
      writeCYKStats ( o );
    o << "=================================================================" <<
      std::endl;
    o << "Local pruning during lattice construction" << std::endl;
    writePruneStats ( o );
    o.close();
    return false;
  };

 private:

  /**
   * \brief Special method that deals with cyk information, including number of rules applied at each cell,  number of states of the full lattice and,
   * if pruned, number of states after pruning.
   * \param &o: [file] stream to write to.
   */

  void writeCYKStats ( oszfstream& o ) {
    std::vector<std::string> ws;
    boost::algorithm::split ( ws, d_->sentence,
                              boost::algorithm::is_any_of ( " " ) );
    std::stringstream line;
    for ( unsigned z = 0; z < width_; ++z ) line << "-";
    o << "Source sentence:" << d_->sentence << std::endl;
    o << "Word count:" << ws.size() << std::endl;
// Not available right now.
//    o << "Sentence-specific grammar size:" << d_->rules.size() << " rules " << endl;
    o << "Number of rules (R), states (NS) and states after pruning (NSP) for each cell of the CYK grid:"
      << std::endl;
    o << "=================================================================" <<
      std::endl;
    o << std::setw ( width_ + 5 ) << std::setiosflags ( std::ios::left ) << "x\\y";
    for ( unsigned x = 0; x < ws.size(); x++ ) {
      o << std::setw ( width_ ) << std::setiosflags ( std::ios::left ) << x + 1;
    }
    o << std::endl;
    o << std::setw ( 4 ) << std::setiosflags ( std::ios::left ) << "----";
    for ( unsigned x = 0; x < ws.size() + 1; x++ ) {
      o << std::setw ( width_ ) <<  line.str();
    }
    o << std::endl;
    for ( unsigned x = 0; x < ws.size(); x++ ) {
      o << std::setw ( 4 ) << std::setiosflags ( std::ios::left ) <<
        std::resetiosflags ( std::ios::right ) << ( ucam::util::toString<unsigned>
            ( x + 1 ) + "->" );
      o << std::setw ( width_ - 4 ) << std::setiosflags ( std::ios::left ) <<
        std::resetiosflags ( std::ios::right ) << ws[x];
      o << std::setw ( 4 ) << std::setiosflags ( std::ios::right ) << "R:";
      for ( unsigned int y = 0; y < ws.size() - x; y++ ) {
        std::stringstream a1;
        a1 << "|";
        for ( unsigned int cc = 1; cc <= d_->stats->numcats; cc++ ) {
          if ( d_->stats->rulecounts[cc * 1000000 + y * 1000 + x] ) {
            //            a1  << d_->cykdata->vcat[cc] << "=" << d_->stats->rulecounts[cc * 1000000 + y * 1000 + x] << " ";
            a1  << d_->vcat[cc] << "=" << d_->stats->rulecounts[cc * 1000000 + y * 1000 + x]
                << " ";
          }
        }
        o << std::setw ( width_ ) << std::setiosflags ( std::ios::left ) <<
          std::resetiosflags ( std::ios::right ) << a1.str() ;
      }
      o << std::setiosflags ( std::ios::left ) << "|" << std::endl;
      o << std::setw ( width_ ) <<  " ";
      o << std::setw ( 4 ) << std::setiosflags ( std::ios::right ) << "NS:";
      for ( unsigned int y = 0; y < ws.size() - x; y++ ) {
        std::stringstream a2;
        a2 << "|";
        for ( unsigned int cc = 1; cc <= d_->stats->numcats; cc++ ) {
          if ( d_->stats->numstates[cc * 1000000 + y * 1000 + x] ) {
            //            a2  << d_->cykdata->vcat[cc] << "=" << d_->stats->numstates[cc * 1000000 + y * 1000 + x] << " ";
            a2  << d_->vcat[cc] << "=" << d_->stats->numstates[cc * 1000000 + y * 1000 + x]
                << " ";
          }
        }
        o << std::setw ( width_ ) << std::setiosflags ( std::ios::left ) <<
          std::resetiosflags ( std::ios::right ) << a2.str();
      }
      o << std::setiosflags ( std::ios::left ) << "|" << std::endl;
      o << std::setw ( width_ ) << " ";
      o << std::setw ( 4 ) << std::setiosflags ( std::ios::right ) << "NSP:";
      for ( unsigned int y = 0; y < ws.size() - x; y++ ) {
        std::stringstream a3;
        a3 << "|";
        for ( unsigned int cc = 1; cc <= d_->stats->numcats; cc++ ) {
          if ( d_->stats->numprunedstates[cc * 1000000 + y * 1000 + x] ) {
            //            a3  << d_->cykdata->vcat[cc] << "=" << d_->stats->numprunedstates[cc * 1000000 + y * 1000 + x] << " ";
            a3  << d_->vcat[cc] << "=" << d_->stats->numprunedstates[cc * 1000000 + y * 1000
                + x] << " ";
          }
        }
        o << std::setw ( width_ ) << std::setiosflags ( std::ios::left ) <<
          std::resetiosflags ( std::ios::right ) << a3.str();
      }
      o << std::setiosflags ( std::ios::left ) << "|" << std::endl;
      o << std::setw ( 4 ) << "----";
      for ( unsigned int y = 0; y < ws.size() - x + 1; y++ ) {
        o << std::setw ( width_ ) <<  line.str();
      }
      o << std::endl;
    }
    o << std::endl;
  }

  /**
   * \brief Write stats relative to pruning (i.e. number of times local pruning has been done)
   * \param &o [file] stream
   */
  void writePruneStats ( oszfstream& o ) {
    o << "Number of times=" << d_->stats->lpcount << std::endl;
  }

};

}
} // end namespaces

#endif
