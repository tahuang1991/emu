#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <iomanip>
#include "log4cplus/logger.h"
#include "log4cplus/loggingmacros.h"

#include "emu/utils/System.h"
#include "emu/daq/writer/RawDataFile.h"

using namespace std;

void emu::daq::writer::RawDataFile::nameFile(){
  /// Names the file to be written.
  ostringstream fileNameStream;
  fileNameStream << pathToFile_        << "/csc_";
  fileNameStream << setfill('0') << setw(8);
  fileNameStream << runNumber_         << "_";
  fileNameStream << appName_ ;
  fileNameStream << setfill('0') << setw(2);
  fileNameStream << appInstance_       << "_";
  fileNameStream << runType_           << "_";
  fileNameStream.width(3);
  fileNameStream << filesInRunCounter_;
  // Insert start time to make sure the name will be unique if it's not a booked run number or it's a debug run:
  if ( !isBookedRunNumber_ || runType_ == "Debug" ) fileNameStream << "_" << runStartTime_;
  fileName_       = fileNameStream.str() + ".raw";
  metaFileName_   = fileNameStream.str() + ".meta";
}

void emu::daq::writer::RawDataFile::open(){
  /// Opens a binary file for output.
  nameFile();
  fs_->open(fileName_.c_str(), ios::out | ios::binary);
  if ( fs_->is_open() ){
    fileNames_.push_back( fileName_ );
    bytesInFileCounter_  = 0;
    eventsInFileCounter_ = 0;
    filesInRunCounter_++;
    tjson_.resetData();
    LOG4CPLUS_INFO( logger_, "Opened " << fileName_ );
  }
  else{
    stringstream oss;
    oss << "Could not open " << fileName_ << " for writing.";
    if ( ( fs_->rdstate() & ifstream::failbit ) != 0 ) oss << " : failbit is set (The last input operation failed because of an error related to the internal logic of the operation itself.)";
    if ( ( fs_->rdstate() & ifstream::eofbit  ) != 0 ) oss << " : eofbit is set (End-Of-File reached while performing an extracting operation on an input stream.)";
    if ( ( fs_->rdstate() & ifstream::badbit  ) != 0 ) oss << " : badbit is set (Error due to the failure of an input/output operation on the stream buffer.)";
    throw oss.str();
  }
}

void emu::daq::writer::RawDataFile::close(){ 
  /// Closes file, and writes a metafile.
  if ( fs_->is_open() ){
    fs_->close();
    LOG4CPLUS_INFO( logger_, "Wrote "                           << 
		    eventsInFileCounter_ << " events ("         << 
		    bytesInFileCounter_  << " bytes) to "       << 
		    fileName_ 	       << "; so far "         << 
		    eventsInRunCounter_  << " events ("         << 
		    bytesInRunCounter_   << " bytes) in run "   << 
		    runNumber_ );
  }
  // Let the world know it's closed (if it indeed is...)
  if ( fs_->is_open() ){
    LOG4CPLUS_ERROR( logger_, fileName_ << " could not be closed.");
  }
  else{
    writeMetaFile();
  }
}

void emu::daq::writer::RawDataFile::writeMetaFile(){
  /// Writes a .jsn file that's needed for the transfer to EOS.
  /// Also writes a <em>file_name_base</em>.<tt>meta</tt> metadata file that will
  /// contain the name of the .jsn file.
  if ( fs_->is_open() ) fs_->close(); // just in case...

  // First prepare the JSON file.
  // Take the timestamp as run number if it's not booked.
  if ( isBookedRunNumber_ ) tjson_.setRunNumber(                runNumber_      );
  else                      tjson_.setRunNumber( toDecimalTime( runStartTime_ ) );
  // For stream name, take the app name + instance number. 
  ostringstream stream;
  stream << appName_ << setfill('0') << setw(2) << appInstance_;
  if (  runType_ == "BadEvents" ) stream << "Bad";
  // Use the hostname shell command instead of the HOSTNAME environment variable, which may not be set.
  string hostname( "" );
  vector<string> hostname_reply = emu::utils::execShellCommand( "hostname -s" );
  if ( hostname_reply.size() > 0 ) hostname = hostname_reply.at(0);
  // Set the parameters for the JSON metafile.
  tjson_
    .setLSNumber      ( filesInRunCounter_ >= 1 ? filesInRunCounter_-1 : 0 )
    .setNumberOfEvents( eventsInFileCounter_ )
    .setDataFileName  ( fileName_.substr( pathToFile_.length()+1 ) ) // Remove the path to file, only the file name is needed.
    .setStreamName    ( stream.str() )
    .setHostName      ( hostname )
    .setSymbolicLink  ( chooseEOSSymLink() );
  // Write the JSON file.
  fs_->open( ( pathToFile_ + "/" + tjson_.getJSONFileName() ).c_str() , ios::out );
  *fs_ << tjson_;
  fs_->close();
    
  // Write the name of the JSON file into the .meta file.
  fs_->open( metaFileName_.c_str(), ios::out );
  *fs_ << tjson_.getJSONFileName();
  fs_->close();
}

time_t emu::daq::writer::RawDataFile::toUnixTime( const std::string YYMMDD_hhmmss_UTC ) const {
  /// Converts time given as string to Unix time
  if ( YYMMDD_hhmmss_UTC.size() < 17 ) return time_t(0);

  struct tm stm;
  std::stringstream ss;

  ss << YYMMDD_hhmmss_UTC.substr( 0,2); ss >> stm.tm_year; ss.clear(); stm.tm_year += 100;
  ss << YYMMDD_hhmmss_UTC.substr( 2,2); ss >> stm.tm_mon;  ss.clear(); stm.tm_mon  -= 1;
  ss << YYMMDD_hhmmss_UTC.substr( 4,2); ss >> stm.tm_mday; ss.clear();
  ss << YYMMDD_hhmmss_UTC.substr( 7,2); ss >> stm.tm_hour; ss.clear();
  ss << YYMMDD_hhmmss_UTC.substr( 9,2); ss >> stm.tm_min;  ss.clear();
  ss << YYMMDD_hhmmss_UTC.substr(11,2); ss >> stm.tm_sec;  ss.clear();

  time_t unixTime = mktime( &stm );

  return ( unixTime < 0 ? time_t(0) : unixTime );
}

#include <iostream>
unsigned long int emu::daq::writer::RawDataFile::toDecimalTime( const std::string& YYMMDD_hhmmss_UTC ) const {
  /// Converts time given as string to a decimal number that reads YYMMDDhhmmss, or 0 if conversion fails.
  unsigned long int dt( 0 );
  std::cout << "YYMMDD_hhmmss_UTC " << YYMMDD_hhmmss_UTC << "\n";
  if ( YYMMDD_hhmmss_UTC.size() < 17 ) return dt;
  std::string dtstring( YYMMDD_hhmmss_UTC );
  std::istringstream iss( dtstring.erase( 13 ).erase( 6, 1 ) );
  std::cout << "iss " << iss.str() << "\n";
  iss >> dt;
  std::cout << "dt " << dt << "\n";
  if ( dt < 100000000000 || dt >= 1000000000000 ) return 0; // It must be a twelve-digit decimal number.
  return dt;
}

std::string emu::daq::writer::RawDataFile::chooseEOSSymLink() const {
  /// Picks the symlink, which determines the destination dir on EOS.
  /// Be sure these symlinks have been created by the Storage Manager Group.
  string symlink;
  if      ( runType_.substr(0,5) == "Calib"  ) symlink = "CSCUXCCALEOS";
  else if ( appName_.substr(0,6) == "EmuRUI" ) symlink = "CSCUXCLOCEOS";
  else if ( appName_.substr(0,5) == "EmuFU"  ) symlink = "CSCUXCLOCEOS";
  return symlink;
}

emu::daq::writer::RawDataFile::RawDataFile(const uint64_t maxFileSize, 
					   const string pathToFile, 
					   const string host, 
					   const string appName, 
					   const uint32_t appInstance, 
					   const string appVersion, 
					   const log4cplus::Logger* logger)
  :maxFileSize_         (maxFileSize)
  ,pathToFile_          (pathToFile)
  ,host_                (host)
  ,appName_             (appName)
  ,appInstance_         (appInstance)
  ,appVersion_          (appVersion)
  ,logger_              (*logger)
  ,runStartTime_        ("")
  ,runStopTime_         ("")
  ,runType_             ("")
  ,runNumber_           (1)
  ,isBookedRunNumber_   (false)
  ,bytesInFileCounter_  (0)
  ,eventsInFileCounter_ (0)
  ,filesInRunCounter_   (0)
  ,bytesInRunCounter_   (0)
  ,eventsInRunCounter_  (0)
{
  fs_ = new std::fstream();
  LOG4CPLUS_DEBUG( logger_, "New file writer has been created." );
}

emu::daq::writer::RawDataFile::~RawDataFile(){ delete fs_; }

void emu::daq::writer::RawDataFile::startNewRun( const int runNumber, 
						 const bool isBookedRunNumber, 
						 const string runStartTime, 
						 const string runType ){
  /// To be called when a new run starts.
  runNumber_          = runNumber;
  isBookedRunNumber_  = isBookedRunNumber;
  // Run number 0 or 1 cannot possibly have been booked:
  if ( runNumber_ == 0 || runNumber_ == 1 ) isBookedRunNumber_ = false;
  runStartTime_       = runStartTime;
  runType_            = runType;
  bytesInRunCounter_  = 0;
  eventsInRunCounter_ = 0;
  open();
}

void emu::daq::writer::RawDataFile::writeData( const char* buf, const int nBytes ){ 
  /// Writes data to file.
  fs_->write(buf, nBytes);
  if ( fs_->fail() ){
    //       LOG4CPLUS_ERROR( logger_, "Error writing to " << fileName_ );
    //       fs_->clear();
    stringstream oss;
    oss << "Error writing to " << fileName_;
    if ( ( fs_->rdstate() & ifstream::failbit ) != 0 ) oss << " : failbit is set (The last input operation failed because of an error related to the internal logic of the operation itself.)";
    if ( ( fs_->rdstate() & ifstream::eofbit  ) != 0 ) oss << " : eofbit is set (End-Of-File reached while performing an extracting operation on an input stream.)";
    if ( ( fs_->rdstate() & ifstream::badbit  ) != 0 ) oss << " : badbit is set (Error due to the failure of an input/output operation on the stream buffer.)";
    throw oss.str();
  }
  else{
    bytesInFileCounter_ += nBytes;
    bytesInRunCounter_  += nBytes;
    tjson_.addData( (const unsigned char*) buf, nBytes );
  }
}

void emu::daq::writer::RawDataFile::startNewEvent(){
  /// To be called when a new event starts.
  eventsInFileCounter_++;
  eventsInRunCounter_++;
  if ( bytesInFileCounter_ > maxFileSize_ ){
    close();
    open();
  }
}

void emu::daq::writer::RawDataFile::endRun(){ 
  /// To be called when the run ends.
  close();
  LOG4CPLUS_INFO( logger_, 
		  "End of run "       <<
		  runNumber_          << ". Wrote "  <<
		  eventsInRunCounter_ << " events (" <<
		  bytesInRunCounter_  <<" bytes) in "<<
		  filesInRunCounter_  <<" file"      <<
		  (filesInRunCounter_==1?".":"s.")      );
}


void emu::daq::writer::RawDataFile::endRun( const string runStopTime ){ 
  /// To be called when the run ends.
  runStopTime_ = runStopTime;
  endRun();
}

int emu::daq::writer::RawDataFile::getFileSize(){
  /// Gets file size in bytes.
  // Returns file size in bytes, or -1 if file couldn't be obtained.
  if ( fs_->is_open() ){
    // If it's still open, synchronize buffer, then get size
    int size = (int) ( fs_->flush().seekp(0,ios::end).tellp() );
    if ( fs_->fail() ) return int(-1);
    else               return size;
  }
  else{
    fs_->open( fileName_.c_str(), ios::in | ios::binary );
    if ( fs_->fail() ){ 
      return int(-1);
    }
    else{
      int size = (int) ( fs_->seekg(0,ios::end).tellg() );
      fs_->close();
      return size;
    }
  }
  return int(-1);
}
