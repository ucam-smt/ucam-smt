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

/** \file include/registrypo.hpp
 * \brief Contains wrapper class RegistryPO, which uses boost::program_options to parse parameters, and provides methods to access them.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef REGISTRYPO_HPP
#define REGISTRYPO_HPP

namespace HifstConstants {
// General. Some of them are extended with short versions of the program option
const std::string kHelp = "help";
const std::string kHelpExtended = kHelp + ",h";
//  const string kLoggerVerbose="logger.verbose"; //defined in logger.hpp
const std::string kLoggerVerboseExtended = kLoggerVerbose + ",v";
const std::string kConfig = "config";
const std::string kConfigExtended = kConfig + ",c"; // short options

}

namespace ucam {
namespace util {

using boost::any_cast;
namespace bpo = boost::program_options;

inline void init_param_options ( int argc, const char* argv[],
                                 bpo::variables_map *vm ); //forward definition
/**
 * \brief class wrapping around the boost program_options variable with parsed values.
 */

inline void initGlobalOptions (bpo::options_description& generic
                               , std::string& configFile) {
  generic.add_options()
  ( HifstConstants::kHelpExtended.c_str(), "produce help message" )
  ( HifstConstants::kLoggerVerboseExtended.c_str(),
    "log with more info messages " )
  ( HifstConstants::kConfigExtended.c_str(),
    bpo::value<std::string> ( &configFile )->default_value ( "" ),
    "name of a configuration file" );
}

inline void parseOptionsGeneric ( bpo::options_description& desc
                                  , bpo::variables_map *vm
                                  , int argc, const char* argv[] ) {
  std::string config_file;
  bpo::options_description generic ( "Command-line options" );
  initGlobalOptions (generic, config_file);
  bpo::options_description cmdline_options;
  cmdline_options.add ( generic ).add ( desc );
  bpo::options_description config_file_options;
  config_file_options.add ( desc );
  bpo::positional_options_description p;
  p.add ( HifstConstants::kConfig.c_str(), -1 );
  bpo::store ( bpo::command_line_parser ( argc,
                                          argv ).options ( cmdline_options ).positional ( p ).run(), *vm );
  bpo::notify ( *vm );
  if ( vm->count ( HifstConstants::kHelp.c_str() ) ) {
    cout << cmdline_options << "\n";
    exit ( EXIT_SUCCESS );
  }
  if ( config_file != "" ) {
    std::vector<std::string> configFiles;
    boost::algorithm::split (configFiles, config_file,
                             boost::algorithm::is_any_of (",") );
    for (int k = configFiles.size() - 1 ; k >= 0; --k) {
      LINFO ("Reading config file" << configFiles[k] );
      std::ifstream ifs ( configFiles[k].c_str() );
      if ( !ifs ) {
        LERROR ( "can not open config file: " << configFiles[k] );
        exit ( EXIT_FAILURE );
      } else {
        bpo::store ( parse_config_file ( ifs, config_file_options ), *vm );
        bpo::notify ( *vm );
      }
    }
  }
}

class RegistryPO {
 private:
  /// Holds all parsed variables
  bpo::variables_map vm_;

 public:

  /**
   * \brief Dumps all configuration parameters into a string with a reasonably pretty format
   * \param decorator_start : A user-defined string prepended as a head to the parameter list
   * \param decorator_end   :  user-defined string appended as a foot to the parameter list
   * \remark Not an efficient implementation, but there seems to be no other choice with boost::any but checking individually each type.
   */
  std::string dump ( const std::string& decorator_start = "",
                     const std::string& decorator_end = "" )  {
    std::stringstream ss;
    ss << decorator_start << endl;
    for ( bpo::variables_map::iterator itx = vm_.begin(); itx != vm_.end();
          ++itx ) {
      if ( !print<std::string> ( ss, itx ) )
        if ( !print<uint> ( ss, itx ) )
          if ( !print<int> ( ss, itx ) )
            if ( !print<short> ( ss, itx ) )
              if ( !print<long> ( ss, itx ) )
                if ( !print<std::size_t> ( ss, itx ) )
                  if ( !print<float> ( ss, itx ) )
                    if ( !print<double> ( ss, itx ) )
                      if ( !print<char> ( ss, itx ) )
                        if ( !print<int64> ( ss, itx ) ) {
                          LERROR ( "Error dumping configuration -- type not recognized for:" <<
                                   itx->first );
                          exit ( EXIT_FAILURE );
                          return ss.str() + decorator_end;
                        }
    }
    return ss.str() + decorator_end + "\n";
  };

  ///Trivial Constructor
  RegistryPO ( const bpo::variables_map& vm ) : vm_ ( vm ) {};
  /**
   * \brief Constructor
   * Runs init_param_options to parse and store in a bpo::variables_map all the values.
   * Once this object is loaded, all command-line/config parameters have been parsed and we know it is safe to get values.
   * \param argc  parameter counter (identical to that of main function)
   * \param argv  holds command-line parameter values (identical to that of main function)
   * \remark init_param_options has a forward definition. It must be implemented by the user of this class.
   */
  RegistryPO ( int argc, const char* argv[] ) {
    init_param_options ( argc, argv, &vm_ );
  };

  /**
   * \brief Constructor
   * Forces a set of values using program_options.
   * The hashmap can contain either bools, (u)ints, chars or strings, floats.
   * Possibly there is a better way to fill in manually program_options::variables_map variable.
   * This constructor is only recommended for testing purposes.
   */

  RegistryPO ( const std::tr1::unordered_map<std::string, boost::any>& v ) {
    bpo::options_description desc ( "" );
    for ( std::tr1::unordered_map<std::string, boost::any>::const_iterator itx =
            v.begin(); itx != v.end(); ++itx ) {
      if ( any_cast<std::string> ( & ( itx->second ) ) ) {
        desc.add_options() ( itx->first.c_str(),
                             bpo::value<std::string>()->default_value ( any_cast<std::string>
                                 ( itx->second ) ), "" );
      } else if ( itx->second.type() == typeid ( int ) ) {
        desc.add_options() ( itx->first.c_str(),
                             bpo::value<int>()->default_value ( any_cast<int> ( itx->second ) ), "" );
      } else if ( itx->second.type() == typeid ( uint ) ) {
        desc.add_options() ( itx->first.c_str(),
                             bpo::value<uint>()->default_value ( any_cast<uint> ( itx->second ) ), "" );
      } else if ( itx->second.type() == typeid ( bool ) ) {
        desc.add_options() ( itx->first.c_str(),
                             bpo::value<bool>()->default_value ( any_cast<bool> ( itx->second ) ), "" );
      } else if ( itx->second.type() == typeid ( char ) ) {
        desc.add_options() ( itx->first.c_str(),
                             bpo::value<char>()->default_value ( any_cast<char> ( itx->second ) ), "" );
      } else if ( itx->second.type() == typeid ( float ) ) {
        desc.add_options() ( itx->first.c_str(),
                             bpo::value<float>()->default_value ( any_cast<float> ( itx->second ) ), "" );
      } else if ( itx->second.type() == typeid ( double ) ) {
        desc.add_options() ( itx->first.c_str(),
                             bpo::value<double>()->default_value ( any_cast<double> ( itx->second ) ), "" );
      } else {
        LERROR ( "Undetected type for " << itx->first );
        exit ( EXIT_FAILURE );
      }
      uint ac = 0;
      char **av = NULL;
      bpo::store ( bpo::parse_command_line ( ac, av, desc ), vm_ );
      bpo::notify ( vm_ );
    }
  };

  ///Returns parsed value associated to key.
  template<typename T>
  inline T get ( const std::string& key ) const {
    LDEBUG ( "Quering key=" << key );
    //    if (!USER_CHECK ( exists ( key ), "Key is not defined!" )){
    if ( !exists ( key ) ) {
      LERROR ( "Failed to query undefined program option key=" << key );
      exit ( EXIT_FAILURE );
    }
    return vm_[key].as< T >();
  };

  ///Performs get<string> and checks whether the real value is to be loaded from file (--param=file://.....)
  inline std::string getString ( const std::string& key ) const {
    std::string value = get<std::string> ( key );
    if ( value.size() > 7 )
      if ( value.substr ( 0, 7 ) == "file://" ) {
        LINFO ( "Loading from " << value.substr ( 7 ) );
        iszfstream aux ( value.substr ( 7 ) );
        std::string line;
        value = "";
        while ( !aux.eof() ) {
          getline ( aux, line );
          boost::algorithm::trim ( line );
          if ( line != "" ) value += " " + line;
        }
        aux.close();
        boost::algorithm::trim ( value );
      }
    return value;
  };

  ///To handle yes|no program option values
  inline bool getBool ( const std::string& key ) const {
    std::string value = get<std::string> ( key );
    boost::algorithm::trim ( value );
    if (value == "yes") return true;
    else if (value == "no") return false;
    LERROR ("Expected yes|no value for program option --" << key);
    exit (EXIT_FAILURE);
  };

  ///Determines whether a program option (key) has been defined by the user.
  bool exists ( const std::string& key ) const {
    return vm_.count ( key ) > 0;
  };

  /**
   * \brief Convenience method that returns a vector of strings taking "," as the separator character.
   * \param key: key to query.
   * \returns value splitted by commas or spaces into a vector of strings.
   */

  inline std::vector<std::string> getVectorString ( const std::string& key )
  const {
    std::vector<std::string> vaux;
    std::string aux = get<std::string> ( key );
    if ( aux != "" ) boost::algorithm::split ( vaux, aux,
          boost::algorithm::is_any_of ( ", " ) );
    return vaux;
  };

  /**
   * \brief Convenience method that returns a vector of strings taking "," as the separator character.
   * \param key       key to query.
   * \param index     Points to index-th element in the vector
   * \returns value splitted by commas into a vector of strings.
   */

  inline std::string getVectorString ( const std::string& key ,
                                       uint index ) const {
    std::vector<std::string> vaux = getVectorString ( key );
    if ( !vaux.size() && !index ) return std::string ( "" );
    LDEBUG ( "Accessing " << key << ",with index=" << index );
    if ( index > vaux.size() - 1 ) {
      LERROR ( "Wrong index at program option key=" << key );
      exit ( EXIT_FAILURE );
    }
    return vaux[index];
  };

  /**
   * \brief Convenience method that returns a set of strings taking "," as the separator character.
   * \param key: key to query.
   * \returns value splitted by commas into a set of strings.
   */

  inline unordered_set<std::string> getSetString ( const std::string& key ) const {
    std::vector<std::string> vaux = getVectorString ( key );
    unordered_set<std::string> saux;
    for ( uint k = 0; k < vaux.size(); ++k )
      saux.insert ( vaux[k] );
    return saux;
  };

  /**
   * \brief Templated method that returns a set of numbers taking "," as the separator character.
   * \param key: key to query.
   * \returns value splitted by commas into a set of numbers (i.e. int, float,...)
   */

  template<typename NumberT>
  inline unordered_set<NumberT> getSetNumber ( const std::string& key ) const {
    std::vector<std::string> vaux = getVectorString ( key );
    unordered_set<NumberT> saux;
    for ( uint k = 0; k < vaux.size(); ++k )
      saux.insert ( toNumber<NumberT> (vaux[k]) );
    return saux;
  };

  /**
   * \brief Convenience method that returns a hash of strings indexed by position and taking "," as the separator character.
   * \param key: key to query.
   * \returns unordered set indexed by position (starting at 1).
   */

  unordered_map<uint, std::string> getMappedIndexString ( const std::string& key )
  const {
    std::vector <std::string> aux = getVectorString ( key );
    unordered_map<uint, std::string> mraux;
    for ( uint k = 0; k < aux.size(); ++k )
      mraux[k + 1] = aux[k]; //Note that mapped indices always start from 1
    return mraux;
  };

  /**
   * \brief Convenience method that returns a hash of indices indexed by string and taking "," as the separator character.
   * \param key: key to query.
   * \returns unordered_map indexed by string returning position (starting at 1).
   */

  inline unordered_map<std::string, uint> getMappedStringIndex (
    const std::string& key ) const {
    std::vector <std::string> aux = getVectorString ( key );
    unordered_map<std::string, uint> miraux;
    for ( uint k = 0; k < aux.size(); ++k ) {
      miraux[aux[k]] = k + 1; //Note that mapped inverse indices always start from 1
    }
    return miraux;
  };

  /**
   * \brief Convenience method that builds a hash with pairs taking "," as the separator character. Pair elements assumed to be unsigned integers.
   * For instance, --param=X,3,T,5 =>  hash["X"]=3 and hash["T"]=5.
   * \param key: key to query.
   * \returns unordered_map indexed by string returning position (starting at 1).
   */

  unordered_map<std::string, uint> getPairMappedStringUInt (
    const std::string& key ) const {
    std::vector <std::string> aux = getVectorString ( key );
    unordered_map<std::string, uint> mraux;
    if ( !USER_CHECK ( ! ( aux.size() % 2 ),
                       "Values are expected to come by pairs string,uint,string,uint,..." ) )
      return mraux;
    for ( uint k = 0; k < aux.size(); k += 2 )
      mraux[aux[k]] = toNumber<uint> ( aux[k + 1] );
    return mraux;
  };

 private:

  /**
   * \brief If variable any is correctly casted to type T, then proceed converting, push into stream and return true; otherwise return false.
   * \param ss    : Stream containing the details of the active parameter
   * \param itx   : Iterator to the map variable containing all registered parameters
   * \return bool : true  if success, false otherwise.
   */
  template <class T>
  inline bool print ( std::stringstream& ss, bpo::variables_map::iterator itx ) {
    if ( any_cast<T> ( & ( itx->second.value() ) ) ) {
      std::stringstream ssaux;
      ssaux << any_cast<T> ( itx->second.value() );
      if ( ssaux.str() != "" )
        ss << "\t\t+" << itx->first << "=" << any_cast<T> ( itx->second.value() ) <<
           endl;
      else
        ss << "\t\t+" << itx->first << endl;
      return true;
    }
    return false;
  };

  DISALLOW_COPY_AND_ASSIGN ( RegistryPO );

};

}
}  // end of namespaces

#endif
