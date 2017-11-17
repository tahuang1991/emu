#ifndef EmuPlotter_h
#define EmuPlotter_h

#include <iostream>
#include <string>
#include <signal.h>
#include <pthread.h>
#include <libgen.h>
#include <map>
#include <string>
#include <iomanip>
#include <set>
#include <sstream>

#include "emu/dqm/common/xdaq.h"
#include "emu/dqm/common/xdata.h"
#include "emu/dqm/common/toolbox.h"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

#include "version.h"


// == CMSSW Section
/*  actually calls  emuDQM/CMSSWLibs/FWCore/MessageLogger/interface/MessageLogger.h */
#include "FWCore/MessageLogger/interface/MessageLogger.h"

/* Normal calls to CMSSW source tree */
#include "EventFilter/CSCRawToDigi/interface/CSCDCCExaminer.h"
#include "EventFilter/CSCRawToDigi/interface/CSCEventData.h"
#include "EventFilter/CSCRawToDigi/interface/CSCALCTHeader.h"
#include "EventFilter/CSCRawToDigi/interface/CSCALCTTrailer.h"
#include "EventFilter/CSCRawToDigi/interface/CSCAnodeData.h"
#include "EventFilter/CSCRawToDigi/interface/CSCCFEBData.h"
#include "EventFilter/CSCRawToDigi/interface/CSCCFEBTimeSlice.h"
#include "EventFilter/CSCRawToDigi/interface/CSCCLCTData.h"
#include "EventFilter/CSCRawToDigi/interface/CSCDDUEventData.h"
#include "EventFilter/CSCRawToDigi/interface/CSCDDUHeader.h"
#include "EventFilter/CSCRawToDigi/interface/CSCDDUTrailer.h"
#include "EventFilter/CSCRawToDigi/interface/CSCDMBHeader.h"
#include "EventFilter/CSCRawToDigi/interface/CSCDMBTrailer.h"
#include "EventFilter/CSCRawToDigi/interface/CSCEventData.h"
#include "EventFilter/CSCRawToDigi/interface/CSCTMBData.h"
#include "EventFilter/CSCRawToDigi/interface/CSCTMBHeader.h"
#include "EventFilter/CSCRawToDigi/interface/CSCTMBTrailer.h"
#include "EventFilter/CSCRawToDigi/src/bitset_append.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigi.h"

#include "CondFormats/CSCObjects/interface/CSCCrateMap.h"

#include "DataFormats/FEDRawData/interface/FEDNumbering.h"

#ifdef UNPACKER2013
#include "DQM/CSCMonitorModule/plugins/CSCDQM_Summary.h"
#else
#include "DQM/CSCMonitorModule/src/CSCDQM_Summary.h"
#endif


// ==  ROOT Section
#include <TROOT.h>
#include <TApplication.h>
#include <TSystem.h>
#include <TH1.h>
#include <TF1.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TString.h>
#include <TCanvas.h>
#include <TKey.h>

#define DEFAULT_IMAGE_FORMAT "png"
#define DEFAULT_CANVAS_WIDTH  800
#define DEFAULT_CANVAS_HEIGHT 600

#include "emu/dqm/common/EmuMonitoringObject.h"
#include "emu/dqm/common/EmuMonitoringCanvas.h"
#include "emu/dqm/common/EmuDQM_Utils.h"
#include "emu/dqm/common/CSCReadoutMappingFromFile.h"
#include "emu/dqm/common/CSCReport.h"

typedef std::map<std::string, uint32_t> CSCCounters;

///
/// @class EmuPlotter
/// @brief EmuPlotter class
///
///
class EmuPlotter
{
public:

  EmuPlotter();
  EmuPlotter(Logger logger);
  ~EmuPlotter();

  // === Book Histograms
  void book();
  int loadFromROOTFile(std::string rootfile, bool fReset=true);

  // === Fills the data coming in.  unpacks the data into the METBRawFormat
  //     data structures and calls the appropriate fill() routine below.
  void processEvent(const char * data, int32_t dataSize, uint32_t errorStat, int32_t nodeNumber = 0, int32_t nBlocks = 0);
  void processChamber(const CSCEventData& data,int32_t nodeID, int32_t dduID);

  void setHistoFile(std::string hfile)
  {
    HistoFile = hfile;
  };

  void setDDUCheckMask(uint32_t mask)
  {
    dduCheckMask = mask;
  }
  uint32_t getDDUCheckMask() const
  {
    return dduCheckMask;
  }
  void setBinCheckMask(uint32_t mask)
  {
    binCheckMask = mask;
  }
  uint32_t getBinCheckMask() const
  {
    return binCheckMask;
  }
  void setXMLHistosBookingCfgFile(std::string filename)
  {
    xmlHistosBookingCfgFile = filename;
  }
  std::string getXMLHistosBookingCfgFile() const
  {
    return xmlHistosBookingCfgFile;
  }
  void setXMLCanvasesCfgFile(std::string filename)
  {
    xmlCanvasesCfgFile = filename;
  }
  std::string getXMLCanvasesCfgFile() const
  {
    return xmlCanvasesCfgFile;
  }
  void setCSCMapFile(std::string filename);
  std::string getCSCMapFile() const
  {
    return cscMapFile;
  }
  void setRunNumber(std::string num)
  {
    runNumber=num;
  }

  void setCheckMapping(bool flag)
  {
    fCheckMapping = flag;
  }

  void enableDeadComparatorsChannelsCheck(bool flag)
  {
    fCheckDeadComparatorsChannels = flag;
  }

  void saveHistos() {};

  void saveToROOTFile(std::string filename);
  void saveCanvasesToROOTFile(std::string filename);
  void saveImages(std::string path,
                  std::string format=DEFAULT_IMAGE_FORMAT,
                  int width=DEFAULT_CANVAS_WIDTH,
                  int height=DEFAULT_CANVAS_HEIGHT,
                  std::string runname="");
  void saveCanvasImages(std::string path,
                        std::string format=DEFAULT_IMAGE_FORMAT,
                        int width=DEFAULT_CANVAS_WIDTH,
                        int height=DEFAULT_CANVAS_HEIGHT,
                        std::string runname="");
  int  convertROOTToImages(std::string rootfile,
                           std::string path="",
                           std::string format=DEFAULT_IMAGE_FORMAT,
                           int width=DEFAULT_CANVAS_WIDTH,
                           int height=DEFAULT_CANVAS_HEIGHT,
                           std::string runname="",
                           std::string filter="");
  void generateCanvasesListFile(std::string filename="canvases_list.js", std::string imgformat="png");
  void generateLayout(std::string filename, std::string rootfolder);
  int generateReport(std::string rootfile,std::string path,std::string runname="");
  int generateOnlineReport(std::string runname="");
  int save_ALCT_CLCT_Match_Data(std::string rootfile,std::string path,std::string runname="");
  int save_CSCCounters(std::string rootfile,std::string path,std::string runname="");
  void showReport();
  void saveReport(std::string filename, std::string runname="");
  void saveReportJSON(std::string filename, std::string runname="");


  bool isListModified()
  {
    return fListModified;
  }
  void setListModified(bool flag)
  {
    fListModified = flag;
  }

  void setLogLevel(int level)
  {
    logger_.setLogLevel(level);
  }
  void setUnpackingLogLevel(int level)
  {
    Logger::getInstance("CSCRawUnpacking").setLogLevel(level);
  }
  void setUnpackingDebug(bool flag)
  {
    /*CSCDDUEventData::setDebug(flag);*/
  }

  void setDebug(bool flag)
  {
    debug = flag;
  }

  bool isMEvalid(ME_List&, std::string, EmuMonitoringObject* & );
  MonitorElement* findME(std::string tag, std::string name, TDirectory* rootfolder=NULL);
  std::vector<std::string> getListOfFolders(std::string filter, TDirectory* dir=NULL);
  int getFormatVersion(TDirectory* dir=NULL);
  std::map<std::string, ME_List >&  getMEs()
  {
    return MEs;
  };
  std::map<std::string, MECanvases_List >&  getMECanvases()
  {
    return MECanvases;
  };
  std::map<std::string, CSCCounters >& getCSCCounters()
  {
    return cscCounters;
  }
  DQMReport& getDQMReport()
  {
    return dqm_report;
  }

  uint32_t getUnpackedDMBCount() const
  {
    return unpackedDMBcount;
  }
  uint32_t getTotalEvents() const
  {
    return nEvents;
  }
  uint32_t getTotalUnpackedCSCs() const
  {
    return nCSCEvents;
  }
  uint32_t getGoodEventsCount() const
  {
    return nGoodEvents;
  }
  uint32_t getBadEventsCount() const
  {
    return nBadEvents;
  }
  uint32_t getDetectedCSCsCount() const
  {
    return nDetectedCSCs;
  }

  void cleanup();
  void updateFractionHistos();
  void updateEfficiencyHistos();
  void updateCSCHistos();
  void updateCSCFractionHistos (std::string cscTag="");

  bool isEventToSave() const
  {
    return fInterestingEvent;
  }

  void reset();
  void lock()
  {
    appBSem_.take();
  }
  void lock ( struct timeval * timeout )
  {
    appBSem_.take(timeout);
  }
  void unlock()
  {
    appBSem_.give();
  }

protected:

  ME_List bookMEs(std::string factoryID, std::string prefix);
  MECanvases_List bookMECanvases(std::string factoryID, std::string prefix, std::string title="");
  CSCCounters bookCounters(std::string cscName="");

  void init();
  std::string getCSCFromMap(int crate, int slot, int& csctype, int& cscposition);
  std::string getCSCName(std::string cscID, int& crate, int& slot, int& csctype, int& cscposition );
  int loadXMLBookingInfo(std::string xmlFile);
  int loadXMLCanvasesInfo(std::string xmlFile);
  int loadKillBitsConfig(std::string xmlFile);
  void clearMECollection(std::map<std::string, ME_List > & collection);
  void clearMECollection(ME_List &collection);
  void printMECollection(ME_List &collection);
  void clearCanvasesCollection(std::map<std::string, MECanvases_List > & collection);
  void clearCanvasesCollection(MECanvases_List &collection);
  void printCanvasesCollection(std::map<std::string, MECanvases_List > & collection);
  void printCanvasesCollection(MECanvases_List &collection);
  EmuMonitoringObject* createME(DOMNode* MEInfo);

  void fillChamberBinCheck(int32_t nodeNumber, bool isEventDenied);

  void createHTMLNavigation(std::string path);
  void createTreeTemplate(std::string path);
  void createTreeEngine(std::string path);
  void createTreePage(std::string path);

  void calcFractionHisto(ME_List MEs, std::string resultHistoName, std::string setHistoName, std::string subSetHistoName);

  std::map<std::string, int> getCSCTypeToBinMap();
  std::string getCSCTypeLabel(int endcap, int station, int ring );

private:

  bool  debug;

  // == list of Monitored Elements
  std::map<std::string, ME_List >   MEs;
  std::map<std::string, MECanvases_List >   MECanvases;

  std::map<std::string, ME_List >     MEFactories;
  std::map<std::string, MECanvases_List >   MECanvasFactories;

  std::map<std::string,uint32_t>    nDMBEvents;
  std::map<uint32_t,uint32_t>       L1ANumbers;
  std::map<uint32_t,bool>       fNotFirstEvent;

  uint32_t    L1ANumber;
  uint32_t    BXN;
  uint32_t    evtSize;

  uint32_t    unpackedDMBcount;
  uint32_t    nEvents;
  uint32_t    nGoodEvents;
  uint32_t    nBadEvents;
  uint32_t    nCSCEvents;
  uint32_t    nDetectedCSCs;
  uint16_t    theFormatVersion;

  Logger    logger_;

  CSCDCCExaminer  bin_checker;

  uint32_t    dduCheckMask;
  uint32_t    binCheckMask;
  uint32_t    dduBinCheckMask; // Set this as constant
  std::string     runNumber;

  bool      fListModified;
  std::string     HistoFile;
  bool      fSaveHistos;
  int       SaveTimer;
  int       fStopTimer;
  bool      fFirstEvent;
  bool      fCloseL1As; // Close L1A bit from DDU Trailer
  bool      fInterestingEvent;
  bool 	    fCheckMapping;
  bool	    fCheckDeadComparatorsChannels;

  std::string     xmlHistosBookingCfgFile;
  std::string     xmlCanvasesCfgFile;
  std::string     xmlKillBitsCfgFile;
  std::string     cscMapFile;

  CSCReadoutMappingFromFile   cscMapping;
  CSCCrateMap* cratemap;
  std::map<std::string, int>  tmap;
  std::string       eTag;
  std::map<std::string, CSCCounters>  cscCounters;

  /** CSC summary map */
  cscdqm::Summary   summary;
  ChamberMap    chamberMap;
  SummaryMap    summaryMap;
  EventDisplay    eventDisplay;


  std::map<std::string, std::vector<std::string> > report;
  DQMReport     dqm_report;

  toolbox::BSem   appBSem_;

};

#endif
