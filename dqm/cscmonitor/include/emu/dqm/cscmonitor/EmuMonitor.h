
/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.			                 *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.		                         *
 * For the list of contributors see CREDITS.   			         *
 *************************************************************************/

#ifndef _EmuMonitor_h_
#define _EmuMonitor_h_

#include <vector>
#include <map>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "emu/dqm/common/xdaq.h"
#include "emu/dqm/common/xdata.h"
#include "emu/dqm/common/toolbox.h"
#include "emu/dqm/common/xoap.h"
#include "emu/dqm/common/xgi.h"

#include "i2o/Method.h"
#include "i2o/utils/AddressMap.h"

#include "pt/tcp/exception/Exception.h"

#include "i2oEmuMonitorMsg.h"

#include "emu/dqm/cscmonitor/exception/Exception.h"
#include "emu/daq/writer/RawDataFile.h"
#include "emu/daq/reader/RawDataFile.h"
#include "emu/daq/reader/Spy.h"

#include "emu/dqm/cscanalyzer/EmuPlotter.h"
#include "emu/dqm/common/EmuDQM_AppParameters.h"
#include "emu/dqm/common/EmuDQM_Utils.h"
#include "emu/dqm/common/RateMeter.h"


using namespace toolbox;

class EmuMonitorTimerTask: public Task
{

public:

  EmuMonitorTimerTask():
    Task("EmuMonitorTimerTask")
  {
    timerDelay	= 60;
    plotter 	= NULL;
    fname 	= "";
    fActive	= false;
  }

  EmuMonitorTimerTask(std::string name):
    Task("EmuMonitorTimerTask"),
    fname(name)
  {
    timerDelay	= 120;
    plotter 	= NULL;
    fActive	= false;
  }

  ~EmuMonitorTimerTask() { }

  void setTimer(int delay)
  {
    timerDelay 	= delay;
  }

  void setPlotter(EmuPlotter* pl)
  {
    plotter 	= pl;
  }

  void setROOTFileName(std::string name)
  {
    fname	= name;
  };
  bool isActive() const
  {
    return 	fActive;
  };

  int svc()
  {
    fActive	= true;

    if (plotter != NULL)
      plotter->saveToROOTFile(fname);

    fActive	= false;

    return 0;
  }

  bool start()
  {
    fActive = true;
    activate();
    return fActive;
  }

  bool stop()
  {
    if (fActive) kill();
    fActive = false;
    return fActive;
  }
  
private:

  bool 		fActive;
  int 		timerDelay;
  std::string 	fname;
  EmuPlotter* 	plotter;

};


class EmuMonitor: public xdaq::WebApplication, xdata::ActionListener, Task
{
public:

  XDAQ_INSTANTIATOR();

  EmuMonitor(xdaq::ApplicationStub* c) throw(xdaq::exception::Exception);

protected:

  // readout service routine
  int 	svc();

  bool 	onError ( xcept::Exception& ex, void * context );

  // == Callback for requesting current exported parameter values
  void 	actionPerformed (xdata::Event& e);

  // == Callback for incoming i2o message
  void 	emuDataMsg(toolbox::mem::Reference *bufRef);

  int 	sendDataRequest(uint32_t last);

  xoap::MessageReference fireEvent (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference onReset (xoap::MessageReference msg) throw (xoap::exception::Exception);

  void 	ConfigureAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception);
  void	EnableAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception );
  void 	HaltAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception );
  void 	noAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception );

  void 	doConfigure();
  void 	doStart();
  void 	doStop();

  // Web callback functions
  void 	Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);
  void 	dispatch (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void 	showStatus (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void 	showControl (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void 	Configure(xgi::Input * in ) throw (xgi::exception::Exception);
  void 	Enable(xgi::Input * in ) throw (xgi::exception::Exception);
  void 	Halt(xgi::Input * in ) throw (xgi::exception::Exception);
  void 	InvalidWebAction(xgi::Input * in ) throw (xgi::exception::Exception);
  void 	stateMachinePage( xgi::Output * out ) throw (xgi::exception::Exception);
  void 	failurePage(xgi::Output * out, xgi::exception::Exception & e)  throw (xgi::exception::Exception);

  // Process Event data
  void 	processEvent(const char * data, int32_t dataSize, uint32_t errorFlag, int32_t node=0, int32_t nBlocks=0);

  void 	updateList(xdata::Integer);
  void 	updateObjects(xdata::Integer);

  xoap::MessageReference requestObjectsList (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference requestObjects (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference requestCanvasesList (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference requestCanvas (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference requestFoldersList (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference requestCSCCounters (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference requestReport (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference saveResults (xoap::MessageReference msg) throw (xoap::exception::Exception);
  xoap::MessageReference syncToCurrentRun (xoap::MessageReference msg) throw (xoap::exception::Exception);

private:

  void initProperties();
  void defineFSM();
  void defineWebSM();
  void bindI2Ocallbacks();
  void bindSOAPcallbacks();
  void bindCGIcallbacks();
  void startATCP()
  throw (emu::dqm::monitor::exception::Exception);

  void getDataServers(xdata::String className);
  void getCollectors(xdata::String className);

  void createDeviceReader();
  void destroyDeviceReader();

  void createFileWriter();
  void destroyFileWriter();
  void writeDataToFile( const char* const data, const int dataLength);

  void configureReadout();
  void enableReadout();
  void disableReadout();

  void setMemoryPool();
  void setupPlotter();
  void resetMonitor();

  void printParametersTable( xgi::Output * out ) throw (xgi::exception::Exception);

  std::string generateLoggerName();
  std::string getROOTFileName(std::string tstamp=""); // Construct ROOT output file name

protected:

  Logger logger_;

  // == Plotter
  EmuPlotter* 			plotter_;
  xdata::String 		xmlHistosBookingCfgFile_;
  xdata::String 		xmlCanvasesCfgFile_;
  xdata::String 		cscMapFile_;
  xdata::String 		outputROOTFile_;
  xdata::String 		outputImagesPath_;
  xdata::Integer 		plotterSaveTimer_;
  xdata::UnsignedInteger 	binCheckMask_;
  xdata::UnsignedInteger 	dduCheckMask_;
  xdata::Boolean 		fSaveROOTFile_;
  xdata::Boolean		fCheckMapping_;  	// Enable or disable DDU-CSC mapping checks
  xdata::String 		daqGroup_;

  // == File Writer
  emu::daq::writer::RawDataFile *fileWriter_;           // File Writer
  xdata::Boolean		enableDataWrite_;	// Enable Data File writing
  xdata::String       		outputDataFile_;    	// The path to the file to write the data into (no file written if "")
  xdata::UnsignedLong 		fileSizeInMegaBytes_;  	// When the file size exceeds this, no more events will be written to it (no file written if <=0)
  xdata::Integer      		maxEvents_;            	// Stop reading from DDU after this many events
  xdata::Integer		nSavedEvents_;		// Counter of saved to file events
  bool				ableToWriteToDisk_;

  EmuMonitorTimerTask * 	timer_;

  xdata::UnsignedInteger   	creditMsgsSent_;
  xdata::UnsignedInteger   	eventsRequested_;
  xdata::UnsignedInteger   	eventsReceived_;
  xdata::UnsignedInteger   	creditsHeld_;

  xdata::String			stateName_;
  xdata::String			stateChangeTime_;
  xdata::String			lastEventTime_;
  xdata::UnsignedInteger   	nDAQEvents_;

  xdata::UnsignedInteger 	totalEvents_;		// Total processed events counter
  xdata::UnsignedInteger 	sessionEvents_;		// Session processed events counter
  xdata::UnsignedInteger	prev_sessionEvents_;

  toolbox::PerformanceMeter * 	pmeter_;
  toolbox::PerformanceMeter * 	pmeterCSC_;

  // == Data processing rate
  xdata::String			dataBw_;
  xdata::String         	dataLatency_;
  xdata::String 		dataRate_;
  xdata::UnsignedInteger   	cscRate_;
  xdata::UnsignedInteger   	cscUnpacked_;
  xdata::UnsignedInteger   	cscDetected_;
  xdata::UnsignedInteger   	runNumber_;
  xdata::UnsignedInteger   	runStartUTC_;

  xdata::Boolean 		useAltFileReader_;
  xdata::Boolean		loopFileReadout_;
  xdata::Boolean		setPlotterDebug_;	// Enable Plotter Debug flag

  emu::daq::reader::Base*  	deviceReader_;  	// Device Reader
  xdata::String         	inputDeviceName_;      	// Input Device Name (file path or board number)
  xdata::String         	inputDeviceType_;      	// Spy, Slink or File
  xdata::String         	inputDataFormat_;      	// "DDU" or "DCC"
  int32_t               	inputDataFormatInt_;   	// EmuReader::DDU or EmuReader::DCC

  xdata::String 		readoutMode_;		// Data Readout Modes (external, internal)
  xdata::String 		transport_;		// Net Transports for external Readout (i20, soap)
  xdata::String 		collectorsClassName_;	// Collectors classname
  xdata::Integer		collectorID_;

  xdata::UnsignedInteger 	committedPoolSize_;	// Total memory for credit messages
  xdata::String       		serversClassName_;	// Servers' class name
  xdata::Vector<xdata::UnsignedInteger> serverTIDs_;	// Server TIDs
  xdata::UnsignedInteger 	nEventCredits_;		// Send this many event credits at a time
  xdata::UnsignedInteger 	prescalingFactor_;	// Prescaling factor for data to be received
  xdata::UnsignedInteger        defEventCredits_;
  xdata::UnsignedInteger        averageRate_;

  xdaq::ApplicationDescriptor 	*appDescriptor_;	// Application's descriptor
  xdaq::ApplicationContext 	*appContext_;		// Application's context
  int32_t       		appTid_;		// Application's TID (instance)
  xdaq::Zone 			*zone_;			// Application's zone

  std::set<xdaq::ApplicationDescriptor*> dataservers_;	// List of all external data servers TIDs
  std::set<xdaq::ApplicationDescriptor*> collectors_;	// List of all collectors TIDs

  xdata::UnsignedInteger maxFrameSize_;			// The maximum frame size to be allocated by the Client

  toolbox::exception::HandlerSignature  * errorHandler_; // Exception handler

  toolbox::mem::Pool* pool_;				// Memory pool for allocating messages for sending

  std::deque<toolbox::mem::Reference*> dataMessages_; 	// The queue of data messages waiting to be processed

  BSem appBSem_;

  toolbox::fsm::FiniteStateMachine fsm_;		// Application state machine
  xgi::WSM wsm_;					// Web dialog state machine

  time_t 	startmark;
  int 		sTimeout; 				// Timeout (in secs) waiting for plotter's busy flag to clear
  bool 		isReadoutActive;
  bool 		keepRunning;
  RateMeter<xdata::UnsignedInteger>* 	rateMeter;
  struct timeval 			bsem_tout;

};

#endif
