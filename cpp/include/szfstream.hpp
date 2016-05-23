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

/** \file include/szfstream.hpp
 * \brief Stream wrapper for pipe/text/compressed files.
 * \date 8-8-2012
 * \author Gonzalo Iglesias
 */

#ifndef SZFSTREAM_HPP
#define SZFSTREAM_HPP

#ifdef USE_FDSTREAM_HPP
#include "fdstream.hpp"
#endif

namespace ucam {
namespace util {

/**
 * \brief Wrapper stream class that reads pipes, text files or gzipped files.
 */
class iszfstream {

 private:

  /// C FILE variable
  FILE* sfile_;

#ifndef  USE_FDSTREAM_HPP
  boost::scoped_ptr<std::ifstream> file;
  boost::scoped_ptr<boost::iostreams::filtering_streambuf<boost::iostreams::input> >
  in;
#endif
  /// Actual special stream.
  std::istream *filestream_;
  std::string auxfilename;

 public:

  ///Empty constructor
  iszfstream () :
    sfile_ ( NULL ),
    filestream_ ( NULL ) {
  };
  ///Constructor with a file name. Opens the file
  iszfstream ( const std::string& filename ) :
    sfile_ ( NULL ),
    filestream_ ( NULL ) {
    open ( filename );
  };

  iszfstream ( const std::stringstream& ss ) :
    sfile_ ( NULL ),
    filestream_ ( NULL ) {
    open ( ss );
  };

  ///Returns internal stream.
  inline std::istream *getStream() {
    return filestream_;
  };

  inline void open ( const std::stringstream& ss ) {
    close();
    filestream_ = new std::stringstream ( ss.str() );
  };

  /**
   * \brief Opens a [file] (pipe, or a text/compressed file).
   * using boost:
   * using fdstream: All three cases are handled with zcat -f,, which is piped (i.e. handled by another processor).
   * \param filename File name to be opened.
   */
  inline void open ( const std::string& filename ) {
    close();
#ifdef USE_FDSTREAM_HPP
    if ( filename != "-" ) {
      LDEBUG ( "Test file=[" << filename << "]" );
      sfile_ = fopen ( filename.c_str(), "r" );
      USER_CHECK ( sfile_ != NULL, "Error while opening file" );
      fclose ( sfile_ );
      //Now we can assume more or less safely that file really exists (sigh)
    }
    //lets open this with the pipe.
    std::string command = "zcat -f ";
    command += filename;
    LINFO ( "Opening (fd)" << command );
    sfile_ = popen ( command.c_str(), "r" );
    USER_CHECK ( sfile_ != NULL, "Error while opening pipe" );
    filestream_ = new boost::fdistream ( fileno ( sfile_ ) );
    USER_CHECK (filestream_, "File Stream allocation failed!");
#else
    LINFO ( "Opening " << filename );
    std::string auxfilename = filename;
    if (auxfilename == "-" ) auxfilename = "/dev/stdin";
    file.reset (new std::ifstream (auxfilename.c_str(),
                                   std::ios_base::in | std::ios_base::binary) );
    if (!USER_CHECK (file->is_open(),
                     "Error while opening file:") ) exit (EXIT_FAILURE);
    in.reset (new boost::iostreams::filtering_streambuf<boost::iostreams::input>);
    if (auxfilename.substr (0, 5) != "/dev/" ) {
      if (ends_with (auxfilename, ".gz") )
        in->push (boost::iostreams::gzip_decompressor() );
    }
    in->push (*file);
    filestream_ = new std::istream (&*in);
    if (!USER_CHECK (filestream_,
                     "File Stream allocation failed!") ) exit (EXIT_FAILURE);
    //Try to detect whether it actually _is_ a compressed file (sigh) .
    if (ends_with (auxfilename, ".gz") ) {
      filestream_->peek();
      if (!USER_CHECK (filestream_->good(),
                       "File not open/doesn't exist... Or possibly not compressed but ends with .gz? " ) )
        exit (EXIT_FAILURE);
    }
#endif
  };

  ///Checks if the file/pipe is open.
  inline bool is_open() {
    return ( filestream_ != NULL );
  };

  ///Destructor. Closes the file.
  ~iszfstream () {
    close();
  };

  ///Checks for end-of-file
  virtual inline int eof() {
    return filestream_->eof();
  };

  ///Closes file
  inline void close() {
    if ( !is_open() ) return;
#ifndef USE_FDSTREAM_HPP
    in.reset();
    file.reset();
#endif
    if ( sfile_ ) {
      fclose ( sfile_ );
      sfile_ = NULL;
    }
    if ( filestream_ ) {
      delete filestream_;
      filestream_ = NULL;
    }
  };

  /// Returns a pointer to itself or NULL if end-of-file reached.
  virtual operator void *() {
    return eof() ? NULL : this;
  };

  ///Read a line.
  virtual inline iszfstream& getline ( std::string& line );

  friend iszfstream& getline ( iszfstream&, std::string&);

  template <typename T>
  friend iszfstream& operator>> ( iszfstream&, T&);

};

inline iszfstream& getline ( iszfstream&  izs, std::string& line ) {
  std::getline ( *izs.filestream_, line );
  return izs;
};

inline iszfstream& iszfstream::getline ( std::string& line ) {
  ::ucam::util::getline ( *this, line );
  return *this;
};

///Templated operator >> for streaming out of iszfstream
template <typename T>
inline iszfstream& operator>> ( iszfstream& iszf, T& stff ) {
  *iszf.filestream_ >> stff;
  return iszf;
};

/**
 * \brief Wrapper stream class that writes to pipes, text files or gzipped files.
 * \remark Note that this class can be used in practice as any stream class.
 */

class oszfstream {
 private:
  bool append_;

  /// C FILE variable
  FILE* sfile_;

#ifndef USE_FDSTREAM_HPP
  boost::scoped_ptr<std::ofstream> file;
  boost::scoped_ptr<boost::iostreams::filtering_streambuf<boost::iostreams::output> >
  out;
#endif
  /// Actual special stream.
  std::ostream *filestream_;
  std::string myfilename;

 public:

  /**
   * \brief Constructor
   * \remark Opens a pipe or a (compressed) file.
   */

  oszfstream ( const std::string& filename , bool append = false) :
    sfile_ ( NULL ),
    filestream_ ( NULL ),
    append_ (append) {
    open ( filename );
  }

  /**
   * \brief Constructor
   * \remark Opens a stringstream.
   */

  oszfstream ( const std::stringstream& ss ) :
    sfile_ ( NULL ), filestream_ ( NULL ), append_ (false) {
    open ( ss );
  }

  ///Returns internal stream.
  inline std::ostream *getStream() {
    return filestream_;
  };

  void open ( const std::stringstream& ss ) {
    close();
    filestream_ = new std::stringstream ( ss.str() );
  }

  /**
   * \brief Opens a [file]
   * \param filename: [file], which could be - for a pipe. If it ends in .gz then it will compress.
   */

  void open ( const std::string& filename ) {
    close();
    if (filename == "") {
      LWARN ("Empty file name?");
      return;
    }
    std::string cmd;
    DirName ( cmd, filename );
    if ( cmd != "" && cmd != "./" && cmd != "/" ) {
      cmd = "mkdir -p " + cmd;
      int a = system ( cmd.c_str() );
    }
#ifdef USE_FDSTREAM_HPP
    std::string command = "cat - >";
    if ( filename.size() > 3 )
      if ( filename.substr ( filename.size() - 3 ) == ".gz" ) command = "gzip > ";
    if (append_) command += ">";
    if ( filename == "-" ) command += "/dev/stdout";
    else command += filename;
    if ( ( sfile_ = popen ( command.c_str(), "w" ) ) == NULL ) {
      std::cerr << "Error while opening file via: " << command << std::endl;
      exit ( EXIT_FAILURE );
    }
    LINFO ( "Opening (fd)" << command );
    filestream_ = new boost::fdostream ( fileno ( sfile_ ) );
#else
    if ( filename == "-" ) myfilename = "/dev/stdout";
    else myfilename = filename;
    if (!append_)
      file.reset (new std::ofstream (myfilename.c_str(),
                                     std::ios_base::out | std::ios_base::binary ) );
    else
      file.reset (new std::ofstream (myfilename.c_str(),
                                     std::ios_base::out | std::ios_base::binary | std::ios_base::app ) );
    if (!file->is_open() ) {
      std::cerr << "Error while opening " << filename << std::endl;
      exit (EXIT_FAILURE);
    }
    out.reset (new boost::iostreams::filtering_streambuf<boost::iostreams::output>);
    if (filename.substr (0, 5) != "/dev/" ) {
      if (ends_with (filename, ".gz") ) {
        out->push (boost::iostreams::gzip_compressor() );
      }
    }
    out->push (*file);
    LINFO ( "Opening " << filename  );
    filestream_ = new std::ostream (&*out);
    if (filestream_ == NULL) {
      std::cerr << "Error while opening " << filename << std::endl;
      exit (EXIT_FAILURE);
    }
#endif
  };

  /**
   * \brief Destructor. Closes the file.
   */

  ~oszfstream () {
    close();
  };

  ///Checks whether the file is open
  bool is_open() {
    return ( filestream_ != NULL );
  };

  ///Closes the file
  inline void close() {
    if ( !is_open() ) return;
#ifndef USE_FDSTREAM_HPP
    out.reset();
    file.reset();
#endif
    if ( filestream_ ) {
      delete filestream_;
      filestream_ = NULL;
    }
    if ( sfile_ ) {
      fclose ( sfile_ );
      sfile_ = NULL;
    }
  };

  typedef std::basic_ostream<char, std::char_traits<char> > CoutType;
  typedef CoutType& ( *StandardEndLine ) ( CoutType&);

  ///Makes endl understandable when streaming towards this class.
  oszfstream& operator<< ( StandardEndLine manip ) {
    *this << "\n";
    return *this;
  };

  ///Templated operator << for streaming into filestream_.
  template <typename T>
  inline oszfstream& operator<< ( const T& stff ) {
    *filestream_ << stff;
    return *this;
  };

};

/// Function that reads from a file. Templated on any external class with a parse method
template <typename FM>
inline void readtextfile ( const std::string& filename, FM& fm ) {
  iszfstream iszf ( filename );
  std::string line;
  while ( getline ( iszf, line ) ) {
    fm.parse ( line );
  }
  iszf.close();
};

/// Function that writes to file. Templated on any external class with a toLine method
template <typename FM>
inline void writetextfile ( const std::string& filename, FM& fm ) {
  oszfstream oszf ( filename );
  std::string line;
  while ( fm.toLine ( line ) ) {
    oszf << line << std::endl;
  }
  oszf.close();
};

///Convenience class that reads "quickly" until a queried line.
template <class StreamT = std::istream >
class FastForwardRead {
 private:
  boost::scoped_ptr<StreamT> af_;
  uint idx_;
 public:
  ///Constructor
  FastForwardRead ( StreamT *af ) :
    af_ ( af ),
    idx_ ( 0 ) {
    LDEBUG ( "Ready!" );
  };

  ///Reads lines until id line is reached
  inline bool operator() ( uint id, std::string *line ) {
    bool finished = false;
    std::string aux;
    *line = "";
    USER_CHECK ( idx_ <= id, "Will not read backwards!" );
    while ( ++idx_ < id ) {
      USER_CHECK ( getline ( *af_, aux ), "source text file out of range!" );
    }
    if ( !getline ( *af_, *line ) ) {
      finished = true;
    }
    return finished;
  };

};

}
} // end namespaces

#endif
