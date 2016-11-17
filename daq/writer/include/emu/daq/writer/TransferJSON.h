#ifndef __TransferJSON_h__
#define __TransferJSON_h__

#include "CheckSum.h"
#include <ostream>


class TransferJSON{
public:
  friend std::ostream& operator<<( std::ostream& os, const TransferJSON& transferJSON );

  TransferJSON()
  : numberOfEvents_( 0L          )
  , fileSize_      ( 0L          )
  , fileName_      ( ""          )
  , symbolicLink_  ( "CSCUXCEOS" )
  , runNumber_     ( 0           )
  , lsNumber_      ( 0           ){}
  TransferJSON& setNumberOfEvents( const unsigned long numberOfEvents ){ numberOfEvents_ = numberOfEvents; return *this; }
  TransferJSON& setDataFileSize  ( const unsigned long fileSize       ){ fileSize_       = fileSize      ; return *this; }
  TransferJSON& setDataFileName  ( const std::string&  fileName       ){ fileName_       = fileName      ; return *this; }
  TransferJSON& setSymbolicLink  ( const std::string&  symbolicLink   ){ symbolicLink_   = symbolicLink  ; return *this; }
  TransferJSON& addData( const unsigned char* buffer, const unsigned long int length ){ checkSum_.add( buffer, length ); fileSize_ += length; return *this; }
  TransferJSON& resetData(){ checkSum_.reset(); fileSize_ = 0L; return *this; }

  TransferJSON& setRunNumber ( const unsigned long runNumber ){ runNumber_  = runNumber ; return *this; }
  TransferJSON& setLSNumber  ( const unsigned int lsNumber   ){ lsNumber_   = lsNumber  ; return *this; }
  TransferJSON& setStreamName( const std::string& streamName ){ streamName_ = streamName; return *this; }
  TransferJSON& setHostName  ( const std::string& hostName   ){ hostName_   = hostName  ; return *this; }
  
  unsigned long int getNumberOfEvents() const { return numberOfEvents_; }
  unsigned long int getDataFileSize  () const { return fileSize_;       }
  std::string       getDataFileName  () const { return fileName_;       }
  std::string       getSymbolicLink  () const { return symbolicLink_;   }
  unsigned long int getCheckSum      () const { return checkSum_.get(); }

  unsigned int getRunNumber () const { return runNumber_ ; }
  unsigned int getLSNumber  () const { return lsNumber_  ; }
  std::string  getStreamName() const { return streamName_; }
  std::string  getHostName  () const { return hostName_  ; }
  std::string  getJSONFileName() const;
  
private:
  // These go into the contents of the JSON file:
  unsigned long     numberOfEvents_;
  unsigned long     fileSize_;
  std::string       fileName_;
  std::string       symbolicLink_;
  CheckSum<Adler32> checkSum_;
  // These go into the name of the JSON file:
  unsigned long     runNumber_;
  std::string       streamName_;
  std::string       hostName_;  
  unsigned int      lsNumber_;
};

#endif
