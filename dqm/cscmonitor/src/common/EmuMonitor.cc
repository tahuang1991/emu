/*************************************************************************
 * XDAQ Components for Distributed Data Acquisition                      *
 * Copyright (C) 2000-2004, CERN.                      *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see LICENSE.                            *
 * For the list of contributors see CREDITS.                 *
 *************************************************************************/

#include <time.h>

#include "emu/dqm/cscmonitor/EmuMonitor.h"
#include "emu/dqm/cscmonitor/version.h"

XDAQ_INSTANTIATOR_IMPL(EmuMonitor)

// == EmuMonitor Constructor == //
EmuMonitor::EmuMonitor(xdaq::ApplicationStub* c)
throw(xdaq::exception::Exception)
  :xdaq::WebApplication(c),
   Task("EmuMonitor"),
   logger_(Logger::getInstance(generateLoggerName())),
   appBSem_(toolbox::BSem::FULL),
   isReadoutActive(false)
{

  LOG4CPLUS_INFO(logger_,"Constructor called");

  /*
      getApplicationDescriptor()->setAttribute("icon",
      "/emu/emuDQM/EmuMonitor/images/dqm64x64.gif");
  */

  appBSem_.take();
  plotter_  = NULL;
  deviceReader_ = NULL;
  pool_   = NULL;
  fileWriter_   = NULL;

  initProperties();
  setMemoryPool();

  timer_  = new EmuMonitorTimerTask();
  pmeter_   = new toolbox::PerformanceMeter();
  pmeterCSC_  = new toolbox::PerformanceMeter();
  rateMeter   = new RateMeter<xdata::UnsignedInteger>();

  rateMeter->addSampler ( "averageRate",&sessionEvents_);
  rateMeter->addSampler ( "cscRate",  &cscUnpacked_);

  errorHandler_ = toolbox::exception::bind (this, &EmuMonitor::onError, "onError");

  defineFSM();
  defineWebSM();

  stateName_     = wsm_.getStateName(wsm_.getCurrentState());
  stateChangeTime_ = emu::dqm::utils::now();
  lastEventTime_   = "---";

  bindI2Ocallbacks();
  bindSOAPcallbacks();
  bindCGIcallbacks();

  appDescriptor_   = getApplicationDescriptor();
  appContext_      = getApplicationContext();
  appTid_      = appDescriptor_->getInstance();
  zone_        = appContext_->getDefaultZone();

  // Initialize ROOT engine
  if (!gApplication)
    TApplication::CreateApplication();

  appBSem_.give();
}

std::string EmuMonitor::generateLoggerName()
{
  xdaq::ApplicationDescriptor *appDescriptor = getApplicationDescriptor();
  string                      appClass       = appDescriptor->getClassName();
  unsigned long               appInstance    = appDescriptor->getInstance();
  stringstream                oss;
  string                      loggerName;


  oss << appClass << "." << setfill('0') << std::setw(2) << appInstance;
  loggerName = oss.str();

  return loggerName;
}


// == Application Error Handler == //
bool EmuMonitor::onError ( xcept::Exception& ex, void * context )
{
  LOG4CPLUS_ERROR(logger_, "onError: " << ex.what());
  return false;
}


// == Init Application Properties == //
void EmuMonitor::initProperties()
{

  LOG4CPLUS_DEBUG(logger_, "initProperties called");

  sTimeout    			= 3;
  plotterSaveTimer_   		= 120;
  outputROOTFile_   		= "";
  outputImagesPath_   		= "";
  xmlHistosBookingCfgFile_ 	= "";
  xmlCanvasesCfgFile_   	= "";
  cscMapFile_     		= "";
  dduCheckMask_   		= 0xFFFFFFFF;
  binCheckMask_   		= 0xFFFFFFFF;

  dataRate_     		= "0.0";
  dataLatency_    		= "0.0";
  dataBw_     			= "0.0";
  cscRate_    			= 0;
  cscUnpacked_    		= 0;
  cscDetected_    		= 0;
  runNumber_    		= 0;
  runStartUTC_  		= 0;

  creditMsgsSent_   		= 0;
  creditsHeld_    		= 0;
  eventsRequested_  		= 0;
  eventsReceived_   		= 0;
  nEventCredits_  		= defEventCredits_;

  bsem_tout.tv_sec  		= 10;
  bsem_tout.tv_usec 		= 0;

  totalEvents_    		= 0;
  sessionEvents_  		= 0;
  prev_sessionEvents_		= 0;
  readoutMode_   	 	= "internal";
  nDAQEvents_   		= 0;

  inputDataFormat_      	= "DDU";
  inputDeviceType_  		= "file";
  inputDeviceName_      	= "";

  serversClassName_   		= "EmuRUI";
  serverTIDs_.clear();
  daqGroup_   			= "default";

  collectorsClassName_  	= "EmuDisplayServer";
  collectorID_    		= 0;
  transport_            	= "i2o";
  committedPoolSize_  		= 0x500000;
  nEventCredits_  		= 50;
  defEventCredits_      	= nEventCredits_;
  prescalingFactor_ 		= 1;
  averageRate_          	= 0;


  useAltFileReader_ 		= false;
  stateName_    		= "";
  fSaveROOTFile_  		= false;
  fCheckMapping_		= true;
  keepRunning           	= false;
  loopFileReadout_  		= false;
  setPlotterDebug_		= false;

  fileSizeInMegaBytes_  	= 200;
  outputDataFile_ 		= "/tmp";
  enableDataWrite_  		= false;
  maxEvents_    		= -1;
  nSavedEvents_   		= 0;
  ableToWriteToDisk_  		= true;

  getApplicationInfoSpace()->fireItemAvailable("totalEvents",   &totalEvents_);
  getApplicationInfoSpace()->fireItemAvailable("sessionEvents",   &sessionEvents_);
  getApplicationInfoSpace()->fireItemAvailable("prev_sessionEvents",   &prev_sessionEvents_);
  getApplicationInfoSpace()->fireItemAvailable("dataBw",    &dataBw_);
  getApplicationInfoSpace()->fireItemAvailable("dataLatency",   &dataLatency_);
  getApplicationInfoSpace()->fireItemAvailable("dataRate",    &dataRate_);
  getApplicationInfoSpace()->fireItemAvailable("averageRate",   &averageRate_);
  getApplicationInfoSpace()->fireItemAvailable("cscRate",   &cscRate_);
  getApplicationInfoSpace()->fireItemAvailable("cscUnpacked",   &cscUnpacked_);
  getApplicationInfoSpace()->fireItemAvailable("cscDetected",   &cscDetected_);
  getApplicationInfoSpace()->fireItemAvailable("runNumber",   &runNumber_);
  getApplicationInfoSpace()->fireItemAvailable("runStartUTC",   &runStartUTC_);

  getApplicationInfoSpace()->fireItemAvailable("readoutMode",   &readoutMode_);
  getApplicationInfoSpace()->fireItemAvailable("transport",   &transport_);
  getApplicationInfoSpace()->fireItemAvailable("collectorsClassName", &collectorsClassName_);
  getApplicationInfoSpace()->fireItemAvailable("collectorID",   &collectorID_);

  getApplicationInfoSpace()->fireItemAvailable("committedPoolSize", &committedPoolSize_);
  getApplicationInfoSpace()->fireItemAvailable("serversClassName",  &serversClassName_);
  getApplicationInfoSpace()->fireItemAvailable("serverTIDs",    &serverTIDs_);
  getApplicationInfoSpace()->fireItemAvailable("nEventCredits",   &nEventCredits_);
  getApplicationInfoSpace()->fireItemAvailable("prescalingFactor",  &prescalingFactor_);

  getApplicationInfoSpace()->fireItemAvailable("inputDataFormat",   &inputDataFormat_);
  getApplicationInfoSpace()->fireItemAvailable("inputDeviceType",   &inputDeviceType_);
  getApplicationInfoSpace()->fireItemAvailable("inputDeviceName",   &inputDeviceName_);

  getApplicationInfoSpace()->fireItemAvailable("xmlCfgFile",    &xmlHistosBookingCfgFile_);
  getApplicationInfoSpace()->fireItemAvailable("xmlCanvasesCfgFile",  &xmlCanvasesCfgFile_);
  getApplicationInfoSpace()->fireItemAvailable("plotterSaveTimer",  &plotterSaveTimer_);
  getApplicationInfoSpace()->fireItemAvailable("outputROOTFile",  &outputROOTFile_);
  getApplicationInfoSpace()->fireItemAvailable("fSaveROOTFile",   &fSaveROOTFile_);
  getApplicationInfoSpace()->fireItemAvailable("fCheckMapping",   &fCheckMapping_);
  getApplicationInfoSpace()->fireItemAvailable("outputImagesPath",  &outputImagesPath_);
  getApplicationInfoSpace()->fireItemAvailable("dduCheckMask",    &dduCheckMask_);
  getApplicationInfoSpace()->fireItemAvailable("binCheckMask",    &binCheckMask_);
  getApplicationInfoSpace()->fireItemAvailable("cscMapFile",    &cscMapFile_);

  getApplicationInfoSpace()->fireItemAvailable("useAltFileReader",  &useAltFileReader_);
  getApplicationInfoSpace()->fireItemAvailable("loopFileReadout", &loopFileReadout_);
  getApplicationInfoSpace()->fireItemAvailable("setPlotterDebug", &setPlotterDebug_);
  getApplicationInfoSpace()->fireItemAvailable("stateName",   &stateName_);
  getApplicationInfoSpace()->fireItemAvailable("stateChangeTime", &stateChangeTime_);
  getApplicationInfoSpace()->fireItemAvailable("lastEventTime",   &lastEventTime_);
  getApplicationInfoSpace()->fireItemAvailable("nDAQEvents",    &nDAQEvents_);
  getApplicationInfoSpace()->fireItemAvailable("daqGroup",    &daqGroup_);

  getApplicationInfoSpace()->fireItemAvailable("enableDataWrite", &enableDataWrite_);
  getApplicationInfoSpace()->fireItemAvailable("maxSavedEvents",  &maxEvents_);
  getApplicationInfoSpace()->fireItemAvailable("nSavedEvents",    &nSavedEvents_);
  getApplicationInfoSpace()->fireItemAvailable("outputDataFile",  &outputDataFile_);
  getApplicationInfoSpace()->fireItemAvailable("fileSizeInMegaBytes", &fileSizeInMegaBytes_);


  getApplicationInfoSpace()->addItemChangedListener ("cscMapFile",  this);
  getApplicationInfoSpace()->addItemChangedListener ("readoutMode",   this);
  getApplicationInfoSpace()->addItemChangedListener ("transport",   this);
  getApplicationInfoSpace()->addItemChangedListener ("collectorsClassName", this);
  getApplicationInfoSpace()->addItemChangedListener ("serversClassName", this);
  getApplicationInfoSpace()->addItemChangedListener ("serverTIDs", this);
  getApplicationInfoSpace()->addItemChangedListener ("nEventCredits",   this);
  getApplicationInfoSpace()->addItemChangedListener ("prescalingFactor",this);

  getApplicationInfoSpace()->addItemChangedListener ("inputDataFormat", this);
  getApplicationInfoSpace()->addItemChangedListener ("inputDeviceType", this);
  getApplicationInfoSpace()->addItemChangedListener ("inputDeviceName", this);

  getApplicationInfoSpace()->addItemChangedListener ("xmlCfgFile",  this);
  getApplicationInfoSpace()->addItemChangedListener ("xmlCanvasesCfgFile", this);
  getApplicationInfoSpace()->addItemChangedListener ("outputROOTFile",  this);
  getApplicationInfoSpace()->addItemChangedListener ("fSaveROOTFile",   this);
  getApplicationInfoSpace()->addItemChangedListener ("fCheckMapping",   this);
  getApplicationInfoSpace()->addItemChangedListener ("plotterSaveTimer", this);
  getApplicationInfoSpace()->addItemChangedListener ("dduCheckMask",  this);
  getApplicationInfoSpace()->addItemChangedListener ("binCheckMask",  this);
  getApplicationInfoSpace()->addItemChangedListener ("daqGroup",  this);

  getApplicationInfoSpace()->addItemChangedListener ("useAltFileReader", this);
  getApplicationInfoSpace()->addItemChangedListener ("loopFileReadout", this);
  getApplicationInfoSpace()->addItemChangedListener ("setPlotterDebug", this);

  getApplicationInfoSpace()->addItemChangedListener ("fileSizeInMegaBytes", this);
  getApplicationInfoSpace()->addItemChangedListener ("enableDataWrite", this);
  getApplicationInfoSpace()->addItemChangedListener ("maxSavedEvents", this);
  getApplicationInfoSpace()->addItemChangedListener ("outputDataFile", this);
  getApplicationInfoSpace()->addItemChangedListener ("runNumber", this);


  getApplicationInfoSpace()->addItemRetrieveListener ("totalEvents",  this);
  getApplicationInfoSpace()->addItemRetrieveListener ("sessionEvents",  this);
  getApplicationInfoSpace()->addItemRetrieveListener ("prev_sessionEvents",  this);
  getApplicationInfoSpace()->addItemRetrieveListener ("dataBw",   this);
  getApplicationInfoSpace()->addItemRetrieveListener ("dataLatency",  this);
  getApplicationInfoSpace()->addItemRetrieveListener ("dataRate",   this);
  getApplicationInfoSpace()->addItemRetrieveListener ("averageRate",  this);
  getApplicationInfoSpace()->addItemRetrieveListener ("stateName",  this);
  getApplicationInfoSpace()->addItemRetrieveListener ("cscRate",  this);
  getApplicationInfoSpace()->addItemRetrieveListener ("nDAQEvents",   this);
  getApplicationInfoSpace()->addItemRetrieveListener ("serversClassName", this);
  getApplicationInfoSpace()->addItemRetrieveListener ("stateChangeTime", this);
  getApplicationInfoSpace()->addItemRetrieveListener ("lastEventTime",  this);
  getApplicationInfoSpace()->addItemRetrieveListener ("readoutMode",  this);

  getApplicationInfoSpace()->addItemRetrieveListener ("nSavedEvents", this);

};


// == Define Application FSM == //
void EmuMonitor::defineFSM()
{
  fsm_.addState ('H', "Halted");
  fsm_.addState ('R', "Ready");
  fsm_.addState ('E', "Enabled");
  fsm_.addStateTransition ('H','R', "Configure", this, &EmuMonitor::ConfigureAction);
  fsm_.addStateTransition ('H','E', "Enable", this, &EmuMonitor::EnableAction);
  fsm_.addStateTransition ('R','R', "Configure", this, &EmuMonitor::noAction);
  fsm_.addStateTransition ('R','E', "Enable", this, &EmuMonitor::EnableAction);
  fsm_.addStateTransition ('E','E', "Enable", this, &EmuMonitor::noAction);
  fsm_.addStateTransition ('E','R', "Configure", this, &EmuMonitor::ConfigureAction);
  fsm_.addStateTransition ('R','H', "Halt", this, &EmuMonitor::HaltAction);
  fsm_.addStateTransition ('E','H', "Halt", this, &EmuMonitor::HaltAction);
  fsm_.addStateTransition ('H','H', "Halt", this, &EmuMonitor::HaltAction);
  fsm_.setInitialState('H');
  fsm_.reset();
};

// == Define Web state machine == //
void EmuMonitor::defineWebSM()
{
  wsm_.addState('H', "Halted",    this, &EmuMonitor::stateMachinePage);
  wsm_.addState('R', "Ready",     this, &EmuMonitor::stateMachinePage);
  wsm_.addState('E', "Enabled",   this, &EmuMonitor::stateMachinePage);
  wsm_.addStateTransition('H','R', "Configure", this, &EmuMonitor::Configure, &EmuMonitor::failurePage);
  wsm_.addStateTransition('R','E', "Enable",    this, &EmuMonitor::Enable,    &EmuMonitor::failurePage);
  wsm_.addStateTransition('R','H', "Halt",      this, &EmuMonitor::Halt,      &EmuMonitor::failurePage);
  wsm_.addStateTransition('E','H', "Halt",      this, &EmuMonitor::Halt,      &EmuMonitor::failurePage);
  wsm_.addStateTransition('H','H', "Halt",      this, &EmuMonitor::Halt, &EmuMonitor::failurePage);
  wsm_.setInitialState('H');
};

void EmuMonitor::setMemoryPool()
{
  if (pool_ != NULL)
    {
      delete pool_;
      pool_ = NULL;
    }
  try
    {
      LOG4CPLUS_INFO (logger_, "Committed pool size is " << (uint32_t) committedPoolSize_);
      toolbox::mem::CommittedHeapAllocator* a = new toolbox::mem::CommittedHeapAllocator(committedPoolSize_);
      toolbox::net::URN urn ("toolbox-mem-pool", "EmuMonitor_EMU_MsgPool");
      pool_ = toolbox::mem::getMemoryPoolFactory()->createPool(urn, a);
      //            LOG4CPLUS_INFO (logger_, "Set high watermark to 90% and low watermark to 70%");
      LOG4CPLUS_INFO (logger_, "Set event credit message buffer's high watermark to 70%");
      pool_->setHighThreshold ( (uint32_t) (committedPoolSize_ * 0.7));
      //            pool_->setLowThreshold ((uint32_t) (committedPoolSize_ * 0.7));
      //  hasSet_committedPoolSize_ = true;
    }
  catch (toolbox::mem::exception::Exception & e)
    {
      LOG4CPLUS_FATAL (logger_, toolbox::toString("Could not set up memory pool, %s (exiting thread)", e.what()));
      return;
    }


}

void EmuMonitor::setupPlotter()
{
  if (plotter_ != NULL)
    {

      if (timer_ != NULL && timer_->isActive())
        {
          int timeout=0;
          while (timer_->isActive() && timeout < 10)
            {
              usleep(1000000);
              timeout++;
              LOG4CPLUS_WARN (logger_, "Waiting to finish saving of results... " << timeout);
            }
          timer_->stop();
        }


      delete plotter_;
      plotter_ = NULL;
    }

  plotter_ = new EmuPlotter(Logger::getInstance(Form("EmuPlotter.%d", appTid_)));

  if (plotter_ != NULL)
    {
      plotter_->setLogLevel(WARN_LOG_LEVEL);
      plotter_->setUnpackingLogLevel(OFF_LOG_LEVEL);
      if ( fCheckMapping_ == xdata::Boolean(true) ) plotter_->setCheckMapping(true);
      else plotter_->setCheckMapping(false);
      if ( setPlotterDebug_ == xdata::Boolean(true) ) plotter_->setDebug(true);
      else plotter_->setDebug(false);
      if (xmlHistosBookingCfgFile_ != "") plotter_->setXMLHistosBookingCfgFile(xmlHistosBookingCfgFile_.toString());
      if (xmlCanvasesCfgFile_ != "") plotter_->setXMLCanvasesCfgFile(xmlCanvasesCfgFile_.toString());
      if (cscMapFile_ != "") plotter_->setCSCMapFile(cscMapFile_.toString());
    }

}

// == Bind I2O Callbacks ==//
void EmuMonitor::bindI2Ocallbacks()
{
  i2o::bind(this, &EmuMonitor::emuDataMsg, I2O_EMUMONITOR_CODE, XDAQ_ORGANIZATION_ID );
}


// == Bind SOAP Callbacks == //
void EmuMonitor::bindSOAPcallbacks()
{

  // == Bind SOAP callbacks for control messages
  xoap::bind (this, &EmuMonitor::fireEvent, "Configure",XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::fireEvent, "Enable",   XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::fireEvent, "Halt",   XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::onReset,   "Reset",  XDAQ_NS_URI);

  xoap::bind (this, &EmuMonitor::requestObjectsList,  "requestObjectsList",   XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::requestObjects,  "requestObjects",   XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::requestCanvasesList,   "requestCanvasesList",  XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::requestCanvas,   "requestCanvas",  XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::requestFoldersList,  "requestFoldersList",   XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::requestCSCCounters,  "requestCSCCounters",   XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::requestReport,   "requestReport",  XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::saveResults,     "saveResults",    XDAQ_NS_URI);
  xoap::bind (this, &EmuMonitor::syncToCurrentRun,     "syncToCurrentRun",    XDAQ_NS_URI);

}

// == Bind CGI Callbacks ==//
void EmuMonitor::bindCGIcallbacks()
{
  xgi::bind(this, &EmuMonitor::Default,  "Default");
  xgi::bind(this, &EmuMonitor::dispatch,  "dispatch");
  xgi::bind(this, &EmuMonitor::showStatus,  "showStatus");
  xgi::bind(this, &EmuMonitor::showControl,   "showControl");

  maxFrameSize_ = sizeof(I2O_EMUMONITOR_CREDIT_MESSAGE_FRAME);

}


// == SOAP Callback trigger state change == //
xoap::MessageReference EmuMonitor::fireEvent (xoap::MessageReference msg) throw (xoap::exception::Exception)
{

  xoap::SOAPPart part = msg->getSOAPPart();
  xoap::SOAPEnvelope env = part.getEnvelope();
  xoap::SOAPBody body = env.getBody();
  DOMNode* node = body.getDOMNode();
  DOMNodeList* bodyList = node->getChildNodes();

  for (unsigned int i = 0; i < bodyList->getLength(); i++)
    {
      DOMNode* command = bodyList->item(i);

      if (command->getNodeType() == DOMNode::ELEMENT_NODE)
        {
          std::string commandName = xoap::XMLCh2String (command->getLocalName());

          try
            {
              LOG4CPLUS_INFO(logger_, "Received FSM Command : " << commandName);
              toolbox::Event::Reference e(new toolbox::Event(commandName, this));
              fsm_.fireEvent(e);
              // Synchronize Web state machine
              wsm_.setInitialState(fsm_.getCurrentState());

              LOG4CPLUS_INFO(logger_, "Current FSM State : " << fsm_.getStateName(fsm_.getCurrentState()));
              stateChangeTime_ = emu::dqm::utils::now();
            }
          catch (toolbox::fsm::exception::Exception & e)
            {
              XCEPT_RETHROW(xcept::Exception, "invalid command", e);
            }

          xoap::MessageReference reply = xoap::createMessage();
          xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
          xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
          envelope.getBody().addBodyElement ( responseName );
          return reply;
        }
    }

  XCEPT_RAISE(xoap::exception::Exception,"command not found");
}

xoap::MessageReference EmuMonitor::onReset(xoap::MessageReference msg)
throw (xoap::exception::Exception)
{
  std::string commandName = "Reset";

  try
    {
      fsm_.reset();
    }
  catch ( toolbox::fsm::exception::Exception e )
    {
      XCEPT_RETHROW(xoap::exception::Exception,
                    "Failed to reset FSM: ", e);
    }

  toolbox::Event::Reference evtRef(new toolbox::Event("Halt", this));
  fsm_.fireEvent( evtRef );

  xoap::MessageReference reply = xoap::createMessage();
  xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
  xoap::SOAPName responseName = envelope.createName( commandName +"Response", "xdaq", XDAQ_NS_URI);
  envelope.getBody().addBodyElement ( responseName );
  return reply;
}





// == Run Control requests current parameter values == //
void EmuMonitor::actionPerformed (xdata::Event& e)
{
  appBSem_.take();
  // update measurements monitors
  if (e.type() == "ItemRetrieveEvent")
    {
      std::string item = dynamic_cast<xdata::ItemRetrieveEvent&>(e).itemName();
      if ( item == "totalEvents")
        {
          LOG4CPLUS_DEBUG(logger_, "Total Events : " << totalEvents_.toString());
        }
      else if ( item == "dataBw")
        {
          dataBw_ = toolbox::toString("%.2f",pmeter_->bandwidth());
          LOG4CPLUS_DEBUG(logger_, "Data Bandwidth: " << dataBw_.toString());
        }
      else if ( item == "dataLatency")
        {
          dataLatency_ =  toolbox::toString("%f",pmeter_->latency());
          LOG4CPLUS_DEBUG(logger_, "Data Latency: " << dataLatency_.toString());
        }
      else if ( item == "dataRate")
        {

          dataRate_ = toolbox::toString("%.2f",pmeter_->rate());
          LOG4CPLUS_DEBUG(logger_, "Data Rate: " << dataRate_.toString());
        }
      else if ( item == "averageRate")
        {
          averageRate_ = rateMeter->getRate("averageRate");
          LOG4CPLUS_DEBUG(logger_, "Average Data Rate: " << averageRate_.toString());
        }
      else if ( item == "cscRate")
        {

          cscRate_ = rateMeter->getRate("cscRate");
          //        cscRate_ = toolbox::toString("%.2f",pmeterCSC_->rate());
          LOG4CPLUS_DEBUG(logger_, "Data Rate: " << cscRate_.toString());
        }
      else if ( item == "stateName")
        {
          stateName_ = wsm_.getStateName(wsm_.getCurrentState());
          LOG4CPLUS_DEBUG(logger_, "State: " << stateName_.toString());
        }
      else if ( item == "nDAQEvents")
        {
          nDAQEvents_ = 0;
          /*
            std::set<xdaq::ApplicationDescriptor*>::iterator pos;
            for (pos=dataservers_.begin(); pos!=dataservers_.end(); ++pos) {
            xdata::UnsignedLong count;
            try
            {
            count.fromString(emu::dqm::getScalarParam(getApplicationContext(), getApplicationDescriptor(), (*pos),"nEventsRead","unsignedLong"));

            }
            catch(xcept::Exception e)
            {
            count    = 0;
            LOG4CPLUS_WARN(logger_, "Failed to get event count from EmuRUI" << (*pos)->getInstance()
            << " : " << xcept::stdformat_exception_history(e));
            }
            nDAQEvents_ = nDAQEvents_ + count;
            }
          */
        }


    };
  if (e.type() == "ItemChangedEvent")
    {
      std::string item = dynamic_cast<xdata::ItemChangedEvent&>(e).itemName();

      if ( item == "readoutMode")
        {
          LOG4CPLUS_INFO(logger_, "Readout Mode : " << readoutMode_.toString());
          disableReadout();
          configureReadout();
        }
      else if ( item == "committedPoolSize")
        {
          LOG4CPLUS_INFO(logger_,
                         toolbox::toString("EmuMonitor's Tid: %d",
                                           i2o::utils::getAddressMap()->getTid(this->getApplicationDescriptor())) );
          setMemoryPool();
        }
      else if ( item == "serversClassName")
        {
          LOG4CPLUS_INFO(logger_, "Data Servers Class Name : " << serversClassName_.toString());
          getDataServers(serversClassName_);
        }
      else if ( item == "serverTIDs" || item == "daqGroup")
        {
          LOG4CPLUS_INFO(logger_, "List of Servers TIDs changed : ");
          getDataServers(serversClassName_);
        }

      else if ( item == "collectorsClassName")
        {
          LOG4CPLUS_INFO(logger_, "Collectors Class Name : " << collectorsClassName_.toString());
          getCollectors(collectorsClassName_);
        }
      else if ( item == "inputDeviceType")
        {
          LOG4CPLUS_INFO(logger_, "Input Device Type : " << inputDeviceType_.toString());
          // destroyDeviceReader();
          // createDeviceReader();
        }
      else if ( item == "nEventCredits")
        {
          LOG4CPLUS_INFO(logger_, "Events Credits : " << nEventCredits_.toString());
          defEventCredits_ =  nEventCredits_;
        }
      else if ( item == "prescalingFactor")
        {
          LOG4CPLUS_INFO(logger_, "Prescaling Factor : " << prescalingFactor_.toString());
          // destroyDeviceReader();
          // createDeviceReader();
        }

      else if ( item == "inputDeviceName")
        {
          LOG4CPLUS_INFO(logger_, "Input Device Name : " << inputDeviceName_.toString());
          // destroyDeviceReader();
          // createDeviceReader();
        }
      else if ( item == "inputDataFormat")
        {
          LOG4CPLUS_INFO(logger_, "Input Data Format : " << inputDataFormat_.toString());
          // destroyDeviceReader();
          // createDeviceReader();
        }
      else if ( item == "useAltFileReader")
        {
          LOG4CPLUS_DEBUG(logger_, "Use Alt File Reader : " << useAltFileReader_.toString());
          // destroyDeviceReader();
          // createDeviceReader();
        }
      else if ( item == "enableDataWrite")
        {
          LOG4CPLUS_INFO(logger_,
                         "Event Data Writing : " << enableDataWrite_.toString());
        }
      else if ( item == "maxSavedEvents")
        {
          LOG4CPLUS_INFO(logger_,
                         "Max number of events to write : " << maxEvents_.toString());
        }
      else if ( item == "outputDataFile")
        {
          LOG4CPLUS_INFO(logger_,
                         "Output path for data writing : " << outputDataFile_.toString());
        }
      else if ( item == "fileSizeInMegaBytes")
        {
          LOG4CPLUS_INFO(logger_,
                         "Data file maximum size in MBytes : " << fileSizeInMegaBytes_.toString());
        }
      else if ( item == "plotterSaveTimer")
        {
          LOG4CPLUS_INFO(logger_, "Plotter Save Timer : " << plotterSaveTimer_.toString());
          if ((plotterSaveTimer_>xdata::Integer(0))
              && (timer_ != NULL))
            {
              timer_->setTimer(plotterSaveTimer_);
            }

        }
      else if ( item == "binCheckMask")
        {
          LOG4CPLUS_INFO(logger_, "Binary Checks Mask : 0x" << std::hex << std::uppercase << binCheckMask_ << std::dec);
          if (binCheckMask_ >= xdata::UnsignedInteger(0)
              && (plotter_ != NULL))
            {
              plotter_->setBinCheckMask(binCheckMask_);
            }

        }
      else if ( item == "dduCheckMask")
        {
          LOG4CPLUS_INFO(logger_, "DDU Checks Mask : 0x" << std::hex << std::uppercase << dduCheckMask_ << std::dec);
          if (dduCheckMask_ >= xdata::UnsignedInteger(0)
              && (plotter_ != NULL))
            {
              plotter_->setDDUCheckMask(dduCheckMask_);
            }

        }
      else if ( item == "outputROOTFile")
        {
          LOG4CPLUS_INFO(logger_, "Output ROOT File : " << outputROOTFile_.toString());
          if (outputROOTFile_.toString().length()
              && (plotter_ != NULL))
            {
              plotter_->setHistoFile(outputROOTFile_);
            }
        }
      else if ( item == "xmlCfgFile")
        {

          LOG4CPLUS_INFO(logger_,
                         "Histograms Booking XML Config File for plotter : " << xmlHistosBookingCfgFile_.toString());
          if (plotter_ != NULL)
            {
              plotter_->setXMLHistosBookingCfgFile(xmlHistosBookingCfgFile_.toString());
            }

        }
      else if ( item == "xmlCanvasesCfgFile")
        {
          LOG4CPLUS_INFO(logger_,
                         "Canvases XML Config File for plotter : " << xmlCanvasesCfgFile_.toString());
          if (plotter_ != NULL)
            {
              plotter_->setXMLCanvasesCfgFile(xmlCanvasesCfgFile_.toString());
            }

        }
      else if ( item == "cscMapFile")
        {
          LOG4CPLUS_INFO(logger_,
                         "CSC Mapping File for plotter : " << cscMapFile_.toString());
          if (plotter_ != NULL)
            {
              plotter_->setCSCMapFile(cscMapFile_.toString());
            }

        }
      else if ( item == "fCheckMapping")
        {
          LOG4CPLUS_INFO(logger_,
                         "DDU-CSC Mapping check flag : " << fCheckMapping_.toString());
          if (plotter_ != NULL)
            {
              if ( fCheckMapping_ == xdata::Boolean(true) ) plotter_->setCheckMapping(true);
              else plotter_->setCheckMapping(false);
            }

        }
      else if ( item == "setPlotterDebug")
        {
          LOG4CPLUS_INFO(logger_,
                         "Set plotter debug flag : " << setPlotterDebug_.toString());
          if (plotter_ != NULL)
            {

              if ( setPlotterDebug_ == xdata::Boolean(true) ) plotter_->setDebug(true);
              else plotter_->setDebug(false);
            }
        }
      else if ( item == "runNumber")
        {
          LOG4CPLUS_INFO(logger_,
                         "Set plotter Run Number : " << runNumber_.toString());
          if (plotter_ != NULL) plotter_->setRunNumber(runNumber_.toString());
        }


      else if ( item == "fSaveROOTFile")
        {
          LOG4CPLUS_INFO(logger_,
                         "Save DQM histograms to ROOT file flag : " << fSaveROOTFile_.toString());

        }

    }
  appBSem_.give();
}

std::string EmuMonitor::getROOTFileName(std::string tstamp)
{
  std::string histofile="dqm_results";
  std::string path=outputROOTFile_;
  if (path.rfind("/") != std::string::npos)
    {
      path = path.substr(0, path.rfind("/")+1);
    }
  else path="";

  if (readoutMode_.toString() == "internal")
    {
      if (inputDeviceName_.toString().rfind(".") != std::string::npos)
        {
          histofile = inputDeviceName_.toString();
          if (histofile.rfind("/") != std::string::npos)
            histofile.erase(0, histofile.rfind("/")+1);
          if (histofile.rfind(".") != std::string::npos)
            histofile.erase(histofile.rfind("."), histofile.size());
        }
    }
  else if (readoutMode_.toString() == "external")
    {
      if (serversClassName_.toString() != "")
        {
          // Timstamp for saved file name
          std::string ts = tstamp;
          if (ts == "") ts = emu::dqm::utils::getDateTime();
          std::ostringstream st;
          st.clear();
          st << "online_" << std::setw(8) << std::setfill('0') << runNumber_ << "_" << "EmuRUI";
          st << std::setw(2) << std::setfill('0') << appTid_ << "_" << ts;
          histofile = st.str();
        }
    }



  return (path+histofile+".root");
}

// == Get Application Descriptors for specified Data Server class name == //
void EmuMonitor::getDataServers(xdata::String className)
{
  try
    {
      dataservers_.clear();
      if (serverTIDs_.size() > 0)
        {
          std::set<xdaq::ApplicationDescriptor*> tmpdataservers_;
          xdaq::ApplicationGroup *g = getApplicationContext()->getDefaultZone()->getApplicationGroup(daqGroup_);
          tmpdataservers_  = g->getApplicationDescriptors(className.toString().c_str());
          std::set<xdaq::ApplicationDescriptor*>::iterator pos;
          for (pos=tmpdataservers_.begin(); pos!=tmpdataservers_.end(); ++pos)
            {
              for (uint32_t j = 0; j<serverTIDs_.size(); j++)
                {
                  if ((serverTIDs_.at(j) != xdata::UnsignedInteger(0)) && (serverTIDs_.at(j) == xdata::UnsignedInteger(i2o::utils::getAddressMap()->getTid(*pos))))
                    dataservers_.insert((*pos));
                }
            }

        }
      else
        {
          xdaq::ApplicationGroup *g = getApplicationContext()->getDefaultZone()->getApplicationGroup(daqGroup_);
          dataservers_  = g->getApplicationDescriptors(className.toString().c_str());

        }
      if (dataservers_.size() == 0)
        {
          LOG4CPLUS_ERROR (logger_,
                           "No " << className.toString() << " Data Servers matching serverTIDs list found.");
        }
    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_ERROR (logger_,
                       "No Data Servers with class name " << className.toString() <<
                       "found. EmuMonitor cannot be configured." <<
                       xcept::stdformat_exception_history(e));
      //            XCEPT_RETHROW (xdaq::exception::Exception,
      //                           toolbox::toString("No %s application instance found. EmuMonitor cannot be configured.",
      //                                             serversClassName_.toString().c_str()),
      //                           e);
    }
}

// == Get Application Descriptors for specified Data Server class name == //
void EmuMonitor::getCollectors(xdata::String className)
{
  try
    {
      collectors_.clear();
      xdaq::ApplicationGroup *g = getApplicationContext()->getDefaultZone()->getApplicationGroup("dqm");
      collectors_  =    g->getApplicationDescriptors(className.toString().c_str());

    }
  catch (xdaq::exception::Exception& e)
    {
      LOG4CPLUS_ERROR (logger_,
                       "No Collectors with class name " << className.toString() <<
                       "found. EmuMonitor cannot be configured." <<
                       xcept::stdformat_exception_history(e));
      //            XCEPT_RETHROW (xdaq::exception::Exception,
      //                           toolbox::toString("No %s application instance found. EmuMonitor cannot be configured.",
      //                                             serversClassName_.toString().c_str()),
      //                           e);
    }
}


void EmuMonitor::ConfigureAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception)
{
  if (fsm_.getCurrentState() == 'E') doStop();

  //  startATCP();
  doConfigure();
  LOG4CPLUS_INFO(logger_, e->type() << " from "
                 << fsm_.getStateName(fsm_.getCurrentState()));
}

void EmuMonitor::startATCP()
throw (emu::dqm::monitor::exception::Exception)
{
  // configure and enable all pt::atcp::PeerTransportATCP

  // std::cout << "In emu::dqm::csc::monitor::Application::startATCP()" << std::endl;

  vector < xdaq::ApplicationDescriptor* > atcpDescriptors;
  try
    {
      atcpDescriptors = emu::dqm::getAppDescriptors(zone_, "pt::atcp::PeerTransportATCP");
    }
  catch (emu::dqm::monitor::exception::Exception e)
    {
      LOG4CPLUS_WARN(logger_, "Failed to get atcp descriptors : "
                     + xcept::stdformat_exception_history(e) );
      atcpDescriptors.clear();
    }

  // std::cout << atcpDescriptors.size() << " atcpDescriptors" << std::endl;

  vector < xdaq::ApplicationDescriptor* >::iterator atcpd;
  for ( atcpd = atcpDescriptors.begin(); atcpd != atcpDescriptors.end(); ++atcpd )
    {

      // std::cout << appDescriptor_->getContextDescriptor()->getURL() << "         "
      //          << (*atcpd)->getContextDescriptor()->getURL() << std::endl;

      // Handle only the pt::atcp::PeerTransportATCP that's in the same Context
      if ( (*atcpd)->getContextDescriptor() == appDescriptor_->getContextDescriptor() )
        {

          // ATCP may already have been started. Check its state.
          std::string atcpState="";
          try
            {
              atcpState = emu::dqm::getScalarParam( appContext_, appDescriptor_, (*atcpd), "stateName", "string" );
            }
          catch (emu::dqm::exception::Exception e)
            {
              stringstream oss;
              oss << "Failed to get state of " << (*atcpd)->getClassName() << (*atcpd)->getInstance();
              XCEPT_RETHROW(emu::dqm::exception::Exception, oss.str(), e);
            }

          if ( atcpState != "Halted" ) continue;
          // Configure ATCP
          try
            {
              emu::dqm::sendFSMEventToApp("Configure", appContext_, appDescriptor_, *atcpd);
            }
          catch (xcept::Exception e)
            {
              stringstream oss;
              oss << "Failed to configure " << (*atcpd)->getClassName() << (*atcpd)->getInstance();
              XCEPT_RETHROW(emu::dqm::monitor::exception::Exception, oss.str(), e);
            }

          // Enable ATCP
          try
            {
              emu::dqm::sendFSMEventToApp("Enable", appContext_, appDescriptor_ , *atcpd);
            }
          catch (xcept::Exception e)
            {
              stringstream oss;
              oss << "Failed to enable " << (*atcpd)->getClassName() << (*atcpd)->getInstance();
              XCEPT_RETHROW(emu::dqm::monitor::exception::Exception, oss.str(), e);
            }

        }

    }

}


void EmuMonitor::doStop()
{
  appBSem_.take();
  if (plotter_ != NULL && isReadoutActive)
    {
      /*
          if (rateMeter != NULL && rateMeter->isActive())
            {
              rateMeter->kill();
            }
      */
      disableReadout();

      if (fSaveROOTFile_ == xdata::Boolean(true) && (sessionEvents_ > xdata::UnsignedInteger(0)))
        {
          plotter_->updateFractionHistos();
          plotter_->updateCSCFractionHistos();

          if ( (timer_ != NULL) && (!timer_->isActive()) )
            {
              timer_->setPlotter(plotter_);
              timer_->setROOTFileName(getROOTFileName("final"));
              timer_->start();
            }
        }
      destroyFileWriter();
    }
  pmeter_->init(200);
  pmeterCSC_->init(200);
  appBSem_.give();

}

void EmuMonitor::doConfigure()
{
//  appBSem_.take();
  disableReadout();
  pmeter_->init(200);
  pmeterCSC_->init(200);

  // Wait for finishing of results saving process before resetting plotter
  if (timer_ != NULL && timer_->isActive())
    {
      int timeout=0;
      while (timer_->isActive() && timeout < 10)
        {
          usleep(1000000);
          timeout++;
          LOG4CPLUS_WARN (logger_,
                          "Waiting to finish saving of results... " << timeout);
        }
      if (timer_->isActive())
        {
          LOG4CPLUS_ERROR (logger_,
                           "Timeout waiting for saving of results. Killing save process." << timeout);
          timer_->stop();
        }
    }


  appBSem_.take();
  setupPlotter();

  if (plotter_ != NULL)
    {
      if (outputROOTFile_.toString().length())
        {
          LOG4CPLUS_INFO (logger_,
                          "plotter::outputROOTFile: " << outputROOTFile_.toString());
          plotter_->setHistoFile(outputROOTFile_);
        }
      if (dduCheckMask_ >= xdata::UnsignedInteger(0))
        {
          LOG4CPLUS_INFO (logger_,
                          "plotter::dduCheckMask: 0x" << std::hex << std::uppercase << strtoul((dduCheckMask_.toString()).c_str(),NULL,0));
          plotter_->setDDUCheckMask(dduCheckMask_);
        }
      if (binCheckMask_ >= xdata::UnsignedInteger(0))
        {
          LOG4CPLUS_INFO (logger_,
                          "plotter::binCheckMask: 0x" << std::hex << std::uppercase << strtoul((binCheckMask_.toString()).c_str(),NULL,0));
          plotter_->setBinCheckMask(binCheckMask_);
        }
      plotter_->reset();
      plotter_->book();
 
    }

  configureReadout();

  /// Try to start ATCP only if readoutMode is set to External
  if (readoutMode_.toString() == "external")
    {
      try
        {
          startATCP();
        }
      catch (xcept::Exception e)
        {
          XCEPT_RETHROW(emu::dqm::monitor::exception::Exception, "Failed to start ATCP ", e);
        }
    }
  appBSem_.give();

}

void  EmuMonitor::doStart()
{
  appBSem_.take();
  sessionEvents_ 	= 0;
  prev_sessionEvents_	= 0;
  creditMsgsSent_ 	= 0;
  creditsHeld_ 		= 0;
  eventsRequested_ 	= 0;
  eventsReceived_ 	= 0;
  cscUnpacked_ 		= 0;
  cscDetected_ 		= 0;
  // runNumber_ 		= 0;
  nEventCredits_ 	= defEventCredits_;

  enableReadout();
  pmeter_->init(200);
  pmeterCSC_->init(200);
  if (rateMeter != NULL)
    {
      rateMeter->init();
      rateMeter->activate();
    }
  appBSem_.give();
}

void EmuMonitor::HaltAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception )
{
  doStop();
  LOG4CPLUS_INFO(logger_, e->type() << " from "
                 << fsm_.getStateName(fsm_.getCurrentState()));
}

void EmuMonitor::EnableAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception )
{
  if (fsm_.getCurrentState() == 'E') return;
  if (fsm_.getCurrentState() == 'H') doConfigure();
  doStart();
  LOG4CPLUS_INFO(logger_, e->type() << " from "
                 << fsm_.getStateName(fsm_.getCurrentState()));
}

void EmuMonitor::noAction(toolbox::Event::Reference e) throw (toolbox::fsm::exception::Exception )
{
  LOG4CPLUS_WARN(logger_, "Attempt of invalid FSM state change to " <<  e->type()
                 << " from "
                 << fsm_.getStateName(fsm_.getCurrentState()) << ". Ignored.");
  fsm_.setInitialState(fsm_.getCurrentState());
}




// == Web Events that trigger state changes (result of wsm::fireEvent) == //
void EmuMonitor::Configure(xgi::Input * in ) throw (xgi::exception::Exception)
{
  try
    {
      toolbox::Event::Reference e(new toolbox::Event("Configure", this));
      fsm_.fireEvent(e);
    }
  catch (toolbox::fsm::exception::Exception & e)
    {
      XCEPT_RETHROW(xgi::exception::Exception, "invalid command", e);
    }
}

void EmuMonitor::Enable(xgi::Input * in ) throw (xgi::exception::Exception)
{
  try
    {
      toolbox::Event::Reference e(new toolbox::Event("Enable", this));
      fsm_.fireEvent(e);
    }
  catch (toolbox::fsm::exception::Exception & e)
    {
      XCEPT_RETHROW(xgi::exception::Exception, "invalid command", e);
    }
}

void EmuMonitor::Halt(xgi::Input * in ) throw (xgi::exception::Exception)
{
  try
    {
      toolbox::Event::Reference e(new toolbox::Event("Halt", this));
      fsm_.fireEvent(e);
    }
  catch (toolbox::fsm::exception::Exception & e)
    {
      XCEPT_RETHROW(xgi::exception::Exception, "invalid command", e);
    }
}



void EmuMonitor::emuDataMsg(toolbox::mem::Reference *bufRef)
{
  // Emu-specific stuff
  creditsHeld_ = ((I2O_EMU_DATA_MESSAGE_FRAME*)bufRef->getDataLocation())->nEventCreditsHeld;
  if (fsm_.getCurrentState() != 'E' || !isReadoutActive)
    {
      // LOG4CPLUS_WARN(logger_,"Dropping received Data. Not in Enabled state.");
      // LOG4CPLUS_WARN(logger_,"EmuRUI holding " << creditsHeld_ << " event credits");
      bufRef->release();
      return;
    }

  appBSem_.take();
  dataMessages_.push_back( bufRef );
  eventsReceived_++;

  //   // Process the oldest message, i.e., the one at the front of the queue
  toolbox::mem::Reference *oldestMessage = dataMessages_.front();

  I2O_EMU_DATA_MESSAGE_FRAME *msg =
    (I2O_EMU_DATA_MESSAGE_FRAME*)oldestMessage->getDataLocation();

  char *startOfPayload = (char*) oldestMessage->getDataLocation()
                         + sizeof(I2O_EMU_DATA_MESSAGE_FRAME);

  uint32_t sizeOfPayload = oldestMessage->getDataSize()-sizeof(I2O_EMU_DATA_MESSAGE_FRAME);

  uint32_t errorFlag = msg->errorFlag;
  uint32_t serverTID = msg->PvtMessageFrame.StdMessageFrame.InitiatorAddress;
  uint32_t status = 0;

  if ((runNumber_ != msg->runNumber) || (runStartUTC_ != msg->runStartUTC))
    {
      LOG4CPLUS_INFO(logger_,"Detected Run Number switch to " << msg->runNumber << "( Run Start Time:  " << emu::dqm::utils::now(msg->runStartUTC) << " ). Resetting Monitor...");
      resetMonitor();
      runNumber_ = msg->runNumber;
      runStartUTC_ = msg->runStartUTC;
      if (plotter_ != NULL) plotter_->setRunNumber(runNumber_.toString()); 
      if (enableDataWrite_ == xdata::Boolean(true)) createFileWriter();
    }

  int32_t nBlocks = (errorFlag >> 12)&0xF; /// Number of reconstructed event data blocks

  // DDU Spy mode readout errors

  if ( (errorFlag & emu::daq::reader::Spy::EndOfEventMissing)   > 0 )   status |= 0x0001;
  if ( (errorFlag & emu::daq::reader::Spy::Timeout)     > 0 ) status |= 0x0002;
  if ( (errorFlag & emu::daq::reader::Spy::PacketsMissing)  > 0 ) status |= 0x0004;
  if ( (errorFlag & emu::daq::reader::Spy::LoopOverwrite)   > 0 ) status |= 0x0008;
  if ( (errorFlag & emu::daq::reader::Spy::BufferOverwrite)   > 0 ) status |= 0x0010;
  if ( (errorFlag & emu::daq::reader::Spy::Oversized)     > 0 ) status |= 0x0020;
  if ( (errorFlag & emu::daq::reader::Spy::HeaderMissing)       > 0 )   status |= 0x0040;
  if ( (errorFlag & emu::daq::reader::Spy::TrailerMissing)      > 0 )   status |= 0x0080;

  if (eventsReceived_%1000 == 0)
    {
      LOG4CPLUS_DEBUG(logger_,
                      // "Received " << bufRef->getDataSize() <<
                      "Received evt#" << eventsReceived_ << " (req: " << eventsRequested_ << ")" <<
                      sizeOfPayload << " bytes, run " << msg->runNumber << ", start time " << msg->runStartUTC <<
                      ", errorFlag 0x"  << std::hex << status << std::dec <<
                      " from " << serversClassName_.toString() <<
                      ":" << serverTID <<
                      ", still holding " << creditsHeld_ << " event credits, " <<
                      "pool size " << dataMessages_.size());
      /*
        LOG4CPLUS_INFO(logger_,
        "==> Average rate: " << averageRate_ << " evt/sec");
        LOG4CPLUS_INFO(logger_,
        "==> Current rate: " << pmeter_->rate() << " evt/sec");
      */
    }

  processEvent(reinterpret_cast<const char *>(startOfPayload), sizeOfPayload, status, appTid_, nBlocks);

  // Free the Emu data message
  bufRef->release();
  dataMessages_.erase( dataMessages_.begin() );
  appBSem_.give();
}

// == Send an I2O token message to all servers == //
int EmuMonitor::sendDataRequest(uint32_t last)
{
  //  creditMsgsSent += 1;
  std::set<xdaq::ApplicationDescriptor*>::iterator pos;
  for (pos=dataservers_.begin(); pos!=dataservers_.end(); ++pos)
    {
      if (eventsReceived_ > xdata::UnsignedInteger(5000))
        {
          uint32_t newRate = averageRate_;
          if (newRate>10) newRate=(newRate/10)*10;
          if ((newRate>=defEventCredits_) && (newRate <=500))
            {
              LOG4CPLUS_DEBUG (logger_, "Adjusting nEventCredits to " << newRate);
              nEventCredits_ = newRate;
            }
        }

      creditMsgsSent_ ++;
      eventsRequested_ = eventsRequested_ +  nEventCredits_;
      toolbox::mem::Reference * ref = 0;
      try
        {
          ref = toolbox::mem::getMemoryPoolFactory()->getFrame(pool_, maxFrameSize_);

          PI2O_EMUMONITOR_CREDIT_MESSAGE_FRAME frame = (PI2O_EMUMONITOR_CREDIT_MESSAGE_FRAME) ref->getDataLocation();


          frame->PvtMessageFrame.StdMessageFrame.MsgFlags         = 0;
          frame->PvtMessageFrame.StdMessageFrame.VersionOffset    = 0;
          frame->PvtMessageFrame.StdMessageFrame.TargetAddress    = i2o::utils::getAddressMap()->getTid((*pos));
          frame->PvtMessageFrame.StdMessageFrame.InitiatorAddress = i2o::utils::getAddressMap()->getTid(this->getApplicationDescriptor());
          frame->PvtMessageFrame.StdMessageFrame.MessageSize      = (sizeof(I2O_EMUMONITOR_CREDIT_MESSAGE_FRAME)) >> 2;

          frame->PvtMessageFrame.StdMessageFrame.Function = I2O_PRIVATE_MESSAGE;
          frame->PvtMessageFrame.XFunctionCode            = I2O_EMUMONITOR_CODE;
          frame->PvtMessageFrame.OrganizationID           = XDAQ_ORGANIZATION_ID;

          frame->nEventCredits    = nEventCredits_;
          frame->prescalingFactor = prescalingFactor_;

          ref->setDataSize(frame->PvtMessageFrame.StdMessageFrame.MessageSize << 2);
          LOG4CPLUS_DEBUG(logger_,
                          "Sending credit #" << creditMsgsSent_ << " to tid: " << frame->PvtMessageFrame.StdMessageFrame.TargetAddress
                          //                                   << ". maxFrameSize=" << maxFrameSize_
                         );
          getApplicationContext()->postFrame(ref, this->getApplicationDescriptor(), (*pos));
        }
      catch (toolbox::mem::exception::Exception & me)
        {
          LOG4CPLUS_FATAL (logger_, xcept::stdformat_exception_history(me));
          return 1; // error
        }
      catch (xdaq::exception::Exception & e)
        //      catch (pt::tcp::exception::Exception & e)
        {
          // Retry 3 times
          bool retryOK = false;
          for (int k = 0; k < 3; k++)
            {
              try
                {
                  getApplicationContext()->postFrame(ref,  this->getApplicationDescriptor(), (*pos));
                  retryOK = true;
                  break; // if send was successfull, continue to send other messages
                }
              catch (xdaq::exception::Exception & re)
                {
                  LOG4CPLUS_WARN (logger_, xcept::stdformat_exception_history(re));
                }
            }

          if (!retryOK)
            {
              LOG4CPLUS_FATAL (logger_, "Frame send failed after 3 times.");
              LOG4CPLUS_FATAL (logger_, xcept::stdformat_exception_history(e));
              ref->release();
              return 1; // error
            }
        }
    }
  return 0; // o.k.
}

// == Configure Readout == //
void EmuMonitor::configureReadout()
{
  sessionEvents_=0;
//  destroyFileWriter();

  if (readoutMode_.toString() == "internal")
    {
      LOG4CPLUS_DEBUG(logger_,
                      "Configure Readout for internal mode");
      destroyDeviceReader();
      createDeviceReader();
      // createFileWriter();
    }
  else if (readoutMode_.toString() == "external")
    {
      LOG4CPLUS_DEBUG(logger_,
                      "Configure Readout for external mode");
      getDataServers(serversClassName_);
    }
  else
    {
      LOG4CPLUS_ERROR(logger_,
                      "Unknown Readout Mode: " << readoutMode_.toString()
                      << "Use \"internal\" or \"external\"");
    }
}

// == Enable Readout == //
void EmuMonitor::enableReadout()
{
  if (!isReadoutActive)
    {
      keepRunning = true;
      this->activate();
      isReadoutActive = true;
    }
}

// == Disable Readout == //
void EmuMonitor::disableReadout()
{
  if (isReadoutActive)
    {
      keepRunning = false;
      // this->kill();
      isReadoutActive = false;
    }
}

// == Create Internal input Device Reader == //
void EmuMonitor::createDeviceReader()
{

  if ( (inputDeviceName_.toString() != "")
       && (readoutMode_.toString() == "internal") )
    {

      // Create reader
      int inputDataFormatInt_ = -1;
      if      ( inputDataFormat_ == "DDU" ) inputDataFormatInt_ = emu::daq::reader::Base::DDU;
      else if ( inputDataFormat_ == "DCC" ) inputDataFormatInt_ = emu::daq::reader::Base::DCC;
      else     LOG4CPLUS_ERROR(logger_,
                                 "No such data format: " << inputDataFormat_.toString() <<
                                 "Use \"DDU\" or \"DCC\"");
      LOG4CPLUS_INFO(logger_,
                     "Creating " << inputDeviceType_.toString() <<
                     " reader for " << inputDeviceName_.toString());
      deviceReader_ = NULL;
      try
        {
          // == Check if file exists
          struct stat stats;
          if (stat((inputDeviceName_.toString()).c_str(), &stats)<0)
            {
              LOG4CPLUS_FATAL(logger_, inputDeviceType_.toString() << ": " <<
                              strerror(errno));
            }
          else
            {
              if      ( inputDeviceType_ == "spy"  )
                deviceReader_ = new emu::daq::reader::Spy(  inputDeviceName_.toString(), inputDataFormatInt_ );
              else if ( inputDeviceType_ == "file" )
		{
                    deviceReader_ = new emu::daq::reader::RawDataFile( inputDeviceName_.toString(), inputDataFormatInt_ );
		    LOG4CPLUS_INFO(logger_,"Setting Run Number to " << emu::dqm::utils::getRunNumberFromFilename (inputDeviceName_.toString()));
		    runNumber_ = emu::dqm::utils::getRunNumberFromFilename (inputDeviceName_.toString());
		    if (plotter_ != NULL) plotter_->setRunNumber(runNumber_.toString());
		}
              // TODO: slink
              else     LOG4CPLUS_ERROR(logger_,
                                         "Bad device type: " << inputDeviceType_.toString() <<
                                         "Use \"file\", \"spy\", or \"slink\"");

              runStartUTC_ = time(NULL);


            }
        }
      catch (char* e)
        {

          std::stringstream oss;
          oss << "Failed to create " << inputDeviceType_.toString()
              << " reader for "      << inputDeviceName_.toString()
              << ": "                << e;
          LOG4CPLUS_ERROR(logger_, oss.str());

          // Don't raise exception as it would be interpreted as FSM transition error
          //      XCEPT_RAISE(toolbox::fsm::exception::Exception, oss.str());
        }
    }
}

// == Destroy Internal input Device Reader == //
void EmuMonitor::destroyDeviceReader()
{
  if (deviceReader_ != NULL)
    {
      LOG4CPLUS_INFO(logger_,
                     "Destroying reader for " << deviceReader_->getName() );
      delete deviceReader_;
      deviceReader_ = NULL;
    }
}


void EmuMonitor::createFileWriter()
{

  // Just in case there's a writer, terminate it in an orderly fashion
  destroyFileWriter();

  nSavedEvents_ = 0;

  // create new writers if path is not empty
  if ( outputDataFile_ != std::string("") &&
       (xdata::UnsignedLongT) fileSizeInMegaBytes_ > (long unsigned int) 0 )
    {
      toolbox::net::URL u( appContext_->getContextDescriptor()->getURL() );
      fileWriter_ = new emu::daq::writer::RawDataFile( 1000000*fileSizeInMegaBytes_,
          outputDataFile_.toString(),
          u.getHost(), "EmuMonitor", appTid_, emudqmcscmonitor::versions, &logger_ );
    }



  // inform the file writer about the new run
  try
    {
      if ( fileWriter_ ) fileWriter_->startNewRun( runNumber_.value_,
            true,
            emu::dqm::utils::getDateTime(runStartUTC_),
            "DQM" );
      LOG4CPLUS_INFO(logger_, "File writer created." );
    }
  catch (std::string e)
    {
      LOG4CPLUS_FATAL( logger_, e );
    }
}

void EmuMonitor::destroyFileWriter()
{
  // Just in case there's a writer, terminate it in an orderly fashion
  if ( fileWriter_ )
    {
      LOG4CPLUS_WARN(logger_, "Terminating leftover file writer." );
      fileWriter_->endRun();
      delete fileWriter_;
      fileWriter_ = NULL;
    }
}

void EmuMonitor::writeDataToFile( const char* const data, const int dataLength )
{
  if ( fileWriter_ )
    {
      try
        {
          fileWriter_->startNewEvent();
          fileWriter_->writeData( data, dataLength );
          ableToWriteToDisk_ = true;
        }
      catch (string e)
        {
          // Cry only once, the first time:
          if ( ableToWriteToDisk_ )
            {
              LOG4CPLUS_ERROR( logger_, e );
              ableToWriteToDisk_ = false;
            }
        }
    }
}


int EmuMonitor::svc()
{
  LOG4CPLUS_INFO (logger_, "Starting Readout loop...");
  //  return 0;

  keepRunning = true;
  bool readValid = true;
  startmark=time(NULL);
  uint32_t ecnt = 0;
  sleep(1);

  while (keepRunning && readValid)
    {
      if ( (readoutMode_.toString() == "internal") &&
           ( (deviceReader_ != NULL)))
        {
          readValid = deviceReader_->readNextEvent();
          if ( !keepRunning || !readValid)
            {
              LOG4CPLUS_ERROR(logger_,
                              " " << inputDataFormat_.toString() << " " << inputDeviceType_.toString() <<
                              " " << inputDeviceName_.toString() << " read error.");
              if (plotter_ != NULL)
                {
                  isReadoutActive = false;
                  if (fSaveROOTFile_== xdata::Boolean(true) && (sessionEvents_ > xdata::UnsignedInteger(0)) )
                    {
                      appBSem_.take();
                      plotter_->updateFractionHistos();
                      plotter_->updateCSCFractionHistos();
                      plotter_->saveToROOTFile(getROOTFileName());
                      appBSem_.give();
                    }


                  if (keepRunning && loopFileReadout_)
                    {
                      LOG4CPLUS_INFO(logger_,
                                     "Restarting readout from " << inputDataFormat_.toString() << " " << inputDeviceType_.toString() <<
                                     " " << inputDeviceName_.toString());
                      appBSem_.take();
                      destroyDeviceReader();
                      createDeviceReader();
                      readValid=true;
                      isReadoutActive = true;
                      appBSem_.give();

                    }

                }
            }
          else
            {
              uint32_t errorFlag = deviceReader_->getErrorFlag();
              uint32_t status=0;
              if ( errorFlag == emu::daq::reader::RawDataFile::Type2)   status |= 0x8000;
              if ( errorFlag == emu::daq::reader::RawDataFile::Type3)   status |= 0x4000;
              if ( errorFlag == emu::daq::reader::RawDataFile::Type4)   status |= 0x2000;
              if ( errorFlag == emu::daq::reader::RawDataFile::Type5)   status |= 0x1000;
              if ( errorFlag == emu::daq::reader::RawDataFile::Type6)   status |= 0x0800;
              appBSem_.take();

              ///** error status test code
              ecnt++;
              // status = ecnt % 256;

              processEvent(deviceReader_->data(), deviceReader_->dataLength(), status, appTid_);
              appBSem_.give();
              /*
              if ((sessionEvents_ % 100) == 1) {
                time_t nowmark=time(NULL);
                averageRate_ = sessionEvents_/(nowmark-startmark);
              }
              */
            }

        }


      uint32_t timeout = 3*1000000; // 5sec
      uint32_t wait = 20000; // 10ms
      uint32_t waittime = 0;
      if (readoutMode_.toString() == "external")
        {
          if (creditsHeld_ > nEventCredits_)
            {
              LOG4CPLUS_DEBUG (logger_, "Waiting to clear " << creditsHeld_ << " event credits from EmuRUI");
            }
          while ((creditsHeld_ > nEventCredits_) && ( waittime <= timeout ))
            {
              usleep(wait);
              waittime += wait;
            }
          if (waittime >= timeout)
            {
              LOG4CPLUS_DEBUG (logger_, toolbox::toString("Timeout waiting for clearing credits."));
              creditsHeld_ = nEventCredits_;
            }
          waittime=0;
          if (eventsReceived_ > eventsRequested_) eventsRequested_=eventsReceived_;
          while ((eventsReceived_ < eventsRequested_) && ( waittime <= timeout ))
            {
              usleep(wait);
              waittime += wait;
            }
          if (waittime >= timeout)
            {
              LOG4CPLUS_DEBUG (logger_, toolbox::toString("Timeout waiting for events from server."));
              LOG4CPLUS_DEBUG (logger_, "Missed " << (eventsRequested_ - eventsReceived_) << " events");

              /*
                time_t nowmark=time(NULL);
                averageRate_ = sessionEvents_/(nowmark-startmark);
              */
              /*
                if ((eventsRequested_ - eventsReceived_) >= nEventCredits_) {
                    // nEventCredits_ = ((nEventCredits_-10)>1)? nEventCredits_-10: 1;
                    continue;
                }
              */
              eventsRequested_ = eventsReceived_;

            }
          waittime = 0;
          //usleep(200000);

          if (!pool_->isHighThresholdExceeded())
            {
              try
                {
                  // Stop if there is an error in sending
                  if (this->sendDataRequest(0) == 1)
                    {
                      LOG4CPLUS_FATAL (logger_, toolbox::toString("Error in frameSend. Stopping client."));
                      // return 1;
                    };
                }
              catch (pt::tcp::exception::Exception & e)
                {
                  LOG4CPLUS_FATAL (logger_, xcept::stdformat_exception_history(e));
                }

            }
          else
            {
              LOG4CPLUS_DEBUG (logger_, "high threshold is exceeded");
              while (pool_->isLowThresholdExceeded())
                {
                  LOG4CPLUS_INFO (logger_, "yield till low threshold reached");
                  this->yield(1);
                }
            }
        }
    }

  return 0;
}


///* == Process Event data == *///
void EmuMonitor::processEvent(const char * data, int dataSize, uint32_t errorFlag, int node, int32_t nBlocks)
{

  if (plotter_ != NULL)
    {
      totalEvents_++;
      sessionEvents_++;
      if (sessionEvents_ % 5000 == 0)
        {
          LOG4CPLUS_DEBUG(logger_,
                          "Evt# " << sessionEvents_.toString() << ": Readout rate: "
                          << rateMeter->getRate("averageRate") << " Evts/sec;  Unpack rate: "
                          << rateMeter->getRate("cscRate") << " CSCs/sec" ) ;
          //       << " (Total processed: " << totalEvents_.toString() << ")");
        }
      plotter_->processEvent(data, dataSize, errorFlag, node, nBlocks);

      // -- Write Event Data to file
      if ( (enableDataWrite_ == xdata::Boolean(true))
           && ((nSavedEvents_ < maxEvents_) || (maxEvents_ == -1) )
           && plotter_->isEventToSave() )
        {
          LOG4CPLUS_DEBUG(logger_, "Writing to file event# " << sessionEvents_.toString());
          writeDataToFile( data, dataSize );
          nSavedEvents_++;
        }

      pmeter_->addSample(dataSize);
      cscUnpacked_ = cscUnpacked_ + plotter_->getUnpackedDMBCount();
      cscDetected_ = plotter_->getDetectedCSCsCount();
      for (unsigned i=0; i< plotter_->getUnpackedDMBCount(); i++)
        {
          pmeterCSC_->addSample(1);
        }
      // lastEventTime_ = emu::dqm::utils::now();
    }
}

///* Reset monitor and plotter *///
void EmuMonitor::resetMonitor()
{
  if (plotter_ != NULL)
    {

      if (timer_ != NULL && timer_->isActive())
        {
          int timeout=0;
          while (timer_->isActive() && timeout < 10)
            {
              usleep(1000000);
              timeout++;
              LOG4CPLUS_WARN (logger_,
                              "Waiting to finish saving of results... " << timeout);
            }
          if (timer_->isActive())
            {
              LOG4CPLUS_ERROR (logger_,
                               "Timeout waiting for saving of results. Killing save process." << timeout);
              timer_->stop();
            }
        }

      if (fSaveROOTFile_== xdata::Boolean(true) && (sessionEvents_ > xdata::UnsignedInteger(0)) )
        {
          plotter_->updateFractionHistos();
          plotter_->updateCSCFractionHistos();
          plotter_->saveToROOTFile(getROOTFileName("final"));
        }
      sessionEvents_	= 0;
      prev_sessionEvents_= 0;
      cscUnpacked_ 	= 0;
      cscDetected_ 	= 0;
      plotter_->reset();
    }

  destroyFileWriter();
}

