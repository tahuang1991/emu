#ifndef _emu_daq_writer_RawDataFile_h_
#define _emu_daq_writer_RawDataFile_h_

#include <stdint.h>

#include <string>
#include <sstream>
#include <fstream>
#include <ostream>
#include <iomanip>
#include "log4cplus/logger.h"

#include "emu/daq/writer/TransferJSON.h"

namespace emu { namespace daq { namespace writer {

  /// Class for writing binary data files.

  /// \par
  /// <b>1)</b> Name file as @c csc_RRRRRRRR_AAAAAAII_TTTTTT_FFF.raw
  /// where
  /// \li @c RRRRRRRR 8-digit decimal run number
  /// \li @c AAAAAA name of application producing the data
  /// \li @c II 2-digit instance number of application producing the data
  /// \li @c TTTTTT run type
  /// \li @c NNN 3-digit file counter in this run
  /// \par
  /// Example: \c csc_00012915_EmuRUI00_Monitor_000.raw
  /// \par
  /// If the run type is \c Debug, or the run number was not booked with the run info database, 
  /// the UTC date and time of the start of run are also included 
  /// as \c csc_RRRRRRRR_AAAAAAII_TTTTTT_FFF_YYMMDD_hhmmss_UTC.raw
  /// where
  /// \li @c YY last 2 digits of year
  /// \li @c MM month [1-12]
  /// \li @c DD day [1-31]
  /// \li @c hh hour
  /// \li @c mm minute
  /// \li @c ss second
  /// \par
  /// Example: \c  csc_00057005_EmuRUI01_Debug_000_070710_103403_UTC.raw
  /// \par 
  /// <b>2)</b> Write data to file as long as its size is below a given maximum.
  /// \par 
  /// <b>3)</b> Close file that has exceeded the maximum size, and write an empty marker file with
  /// the same file name but with the extension \c raw replaced with \c is_closed
  /// \par 
  /// <b>4)</b> Repeat from <b>1)</b>.
  using namespace std;

  class RawDataFile{

  private:
    uint64_t          maxFileSize_;	///< when exceeding this size [bytes], the file will be closed, and a new one opened
    string            pathToFile_;	///< direcory where the file is to be written
    string            host_;          ///< host name
    string            appName_;	///< name of application producing the data
    uint32_t          appInstance_;	///< instance of application producing the data
    string            appVersion_;    ///< version of application producing the data
    log4cplus::Logger logger_;	///< logger
    string            runStartTime_;	///< date and time of start of run
    string            runStopTime_;	///< date and time of end of run
    string            runType_;	///< run type
    uint32_t          runNumber_;	///< run number
    bool              isBookedRunNumber_; ///< whether or not this run number was booked with the database
    uint64_t          bytesInFileCounter_; ///< number of bytes written into this file so far
    uint64_t          eventsInFileCounter_; ///< number of events written into this file so far
    uint64_t          filesInRunCounter_; ///< number of files written in this run so far
    uint64_t          bytesInRunCounter_; ///< number of bytes written in this run so far
    uint64_t          eventsInRunCounter_; ///< number of events written in this run so far
    string            fileName_;	///< file name
    string            metaFileName_; ///< name of metadata file [ <em>file_name_base</em>.<tt>meta</tt> ]
    vector<string>    fileNames_;	///< names of files opened so far
    fstream          *fs_;		///< output file stream
    TransferJSON      tjson_;	///< builds checksum and writes it (and other metadata) to a JSON file

    /// Names the file to be written.
    void nameFile();

    /// Opens a binary file for output.
    void open();

    /// Closes file, and writes an empty <em>file_name_base</em>.<tt>is_closed</tt> marker file.
    void close();

    /// Writes an empty <em>file_name_base</em>.<tt>is_closed</tt> marker file.
    void writeMarkerFile();

    /// Writes a <em>file_name_base</em>.<tt>meta</tt> metadata file to make CASTOR happy.
    void writeMetaFile();

    /// Converts time given as string to Unix time

    ///
    /// @param YYMMDD_hhmmss_UTC Time string. Must be in this format.
    ///
    /// @return Unix time. 0 if conversion fails.
    ///
    time_t toUnixTime( const std::string YYMMDD_hhmmss_UTC ) const;

    ///
    /// @param YYMMDD_hhmmss_UTC Time string. Must be in this format.
    ///
    /// @return Integer time that reads YYMMDDhhmmss in decimal. 0 if conversion fails.
    ///
    unsigned long int toDecimalTime( const std::string& YYMMDD_hhmmss_UTC ) const;

    /** 
     * Picks the symlink, which determines the destination dir on EOS:
     * CSCUXCLOCEOS -> /eos/cms/store/group/dpg_csc/comm_csc/uxc/ldaq/local
     * CSCUXCCALEOS -> /eos/cms/store/group/dpg_csc/comm_csc/uxc/ldaq/calib
     * CSCUXCDQMEOS -> /eos/cms/store/group/dpg_csc/comm_csc/uxc/ldqm
     * @return The name of the EOS symlink (one of the above).
     */
    std::string chooseEOSSymLink() const;
    
  public:

    /// constructor

    ///
    /// @param maxFileSize maximum file size [bytes]
    /// @param pathToFile direcory where the file is to be written
    /// @param app name of application producing the data
    /// @param logger logger
    ///
    RawDataFile(const uint64_t maxFileSize, 
		const string pathToFile, 
		const string host, 
		const string appName, 
		const uint32_t appInstance, 
		const string appVersion, 
		const log4cplus::Logger* logger);

    /// destructor
    ~RawDataFile();

    /// To be called when a new run starts.

    /// Resets counters and opens a file.
    ///
    /// @param runNumber run number
    /// @param isBookedRunNumber whether or not this run number was booked with the database
    /// @param runStartTime UTC date and time of the start of run
    /// @param runType run type
    ///
    void startNewRun( const int runNumber, 
		      const bool isBookedRunNumber, 
		      const string runStartTime, 
		      const string runType );

    /// Writes data to file.

    /// @param buf data buffer
    /// @param nBytes number of bytes to be written
    ///
    void writeData( const char* buf, const int nBytes );

    /// To be called when a new event starts.
    void startNewEvent();

    /// To be called when the run ends.
    void endRun();

    /// To be called when the run ends.

    ///
    /// @param runStopTime UTC date and time of the end of run
    void endRun( const string runStopTime );

    /// Gets file size in bytes.

    ///
    /// @return file size in bytes, or -1 if file couldn't be obtained
    ///
    int getFileSize();

    /// accessor of run number

    /// @return run number
    ///
    uint32_t getRunNumber() const { return runNumber_; }

    /// accessor of number of events written in this run

    /// @return number of events written in this run
    ///
    uint64_t getNumberOfEventsWritten() const { return eventsInRunCounter_; }

    vector<string> getFileNames() const { return fileNames_; }
  };

}}} // namespace emu::daq::writer

#endif //#ifndef _emu_daq_writer_RawDataFile_h_
