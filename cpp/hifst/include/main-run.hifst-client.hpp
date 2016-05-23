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

#ifndef MAIN_RUN_HIFST_CLIENT_HPP
#define MAIN_RUN_HIFST_CLIENT_HPP

/**
 * \file
 * \brief Contains hifst client core implementation, sends request to hifst in server mode
 * \date 1-10-2012
 * \author Gonzalo Iglesias
 */

namespace ucam {
namespace hifst {

using boost::asio::ip::tcp;
const int max_length = 1024;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

/**
 * \brief Full single-threaded Translation system
 */

template <class Data = HifstClientTaskData>
class SingleThreadedHifstClientTask: public ucam::util::TaskInterface<Data> {
 private:
  typedef ucam::util::iszfstream iszfstream;
  typedef ucam::util::oszfstream oszfstream;

  ///Object reading appropriately file according to range specificed by user
  ucam::util::FastForwardRead<iszfstream> fastforwardread_;
  ///Contains file in which to store translations
  std::string textoutput_;

  ///Registry object with command-line parameters
  const ucam::util::RegistryPO& rg_;

  ///Host to connect to
  const std::string host_;
  ///Server is listening at port port_...
  const std::string port_;

 public:
  /**
   *\brief Constructor
   *\param rg: pointer to ucam::util::RegistryPO object with all parsed parameters.
   */
  SingleThreadedHifstClientTask ( const ucam::util::RegistryPO& rg ) :
    fastforwardread_ ( new iszfstream ( rg.get<std::string>
                                        ( HifstConstants::kSourceLoad ) ) ),
    textoutput_ ( rg.get<std::string> ( HifstConstants::kTargetStore ) ),
    host_ ( rg.get<std::string> ( HifstConstants::kHifstHost ) ),
    port_ ( rg.get<std::string> ( HifstConstants::kHifstPort ) ),
    rg_ ( rg ) {
  };

  ///Sends request, retrieves server reply, prints translation to file
  inline bool operator() () {
    Data d;
    bool finished = false;
    oszfstream *fileoutput = NULL;
    if ( textoutput_ != "" ) {
      fileoutput = new oszfstream ( textoutput_ );
    }
    for ( ucam::util::IntRangePtr ir (ucam::util::IntRangeFactory ( rg_ ) );
          !ir->done ();
          ir->next () ) {
      d.sidx = ir->get ();
      boost::scoped_ptr<std::string> aux ( new std::string ( "" ) );
      d.translation = aux.get();
      finished = fastforwardread_ ( d.sidx ,
                                    &d.sentence ); //Move to whichever next sentence and read
      boost::algorithm::trim (d.sentence);
      if (finished && d.sentence == "" ) break;
      FORCELINFO ( "Translating sentence " << d.sidx << ":" << d.sentence );
      ///Do actual translation!
      run ( d );
      FORCELINFO ( *d.translation );
      if ( fileoutput != NULL )
        *fileoutput << *d.translation << std::endl;
      if ( finished ) break;
    }
    delete fileoutput;
    return false;
  }

 private:

  /**
   * \brief Uses boost asio library to connect to a server. Sends size of source text and source text itself.
   * Receives size of translation and translation itself.
   * \param d     Data object, translation will be added to it once received the server reply
   */
  bool run ( Data& d ) {
    try {
      LINFO ( "Attempting connection to host=" << host_  << ",port=" << port_ );
      boost::asio::io_service io_service;
      tcp::resolver resolver ( io_service );
      tcp::resolver::query query ( tcp::v4(), host_.c_str(), port_.c_str() );
      tcp::resolver::iterator iterator = resolver.resolve ( query );
      tcp::socket s ( io_service );
      boost::asio::connect ( s, iterator );
      {
        char request[max_length + 1];
        strcpy ( request, ( char * ) d.sentence.c_str() );
        std::size_t request_length = strlen ( request );
        boost::asio::write ( s, boost::asio::buffer ( &request_length,
                             sizeof ( std::size_t ) ) );
        boost::asio::write ( s, boost::asio::buffer ( request, request_length ) );
      }
      char *reply = new char[max_length + 1];
      std::size_t reply_length = 0;
      std::size_t reply_length1 = boost::asio::read ( s,
                                  boost::asio::buffer ( &reply_length, sizeof ( std::size_t ) ) );
      LDEBUG ("Expected reply Length=" << reply_length);
      std::size_t reply_length2 = boost::asio::read ( s, boost::asio::buffer ( reply,
                                  reply_length ) );
      LINFO ("Reply Length=" << reply_length2);
      reply[reply_length2] = 0;
      *d.translation = reply;
      delete reply;
      s.close();
    } catch ( std::exception& e ) {
      std::cerr << "Exception: " << e.what() << "\n";
    }
  };

  DISALLOW_COPY_AND_ASSIGN ( SingleThreadedHifstClientTask );

};

}
}  // end namespaces

#endif
