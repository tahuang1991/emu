#ifndef EmuDisplayClient_h
#define EmuDisplayClient_h

#include <algorithm>
#include <map>
#include <list>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include <sys/stat.h>
#include <unistd.h>


#include "emu/dqm/common/xdaq.h"
#include "emu/dqm/common/xdata.h"
#include "emu/dqm/common/toolbox.h"
#include "emu/dqm/common/xoap.h"
#include "emu/dqm/common/xgi.h"

#include "emu/dqm/common/cgicc.h"

#include <TROOT.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TSystem.h>
#include <TBuffer.h>
#include <TMessage.h>
#include <TObject.h>
#include <TH1F.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TClass.h>
#include <TImage.h>
#include <THStack.h>
#include <TFile.h>
#include <TKey.h>

#include "emu/base/WebReporter.h"
#include "emu/base/FactFinder.h"

#ifdef UNPACKER2013
#include "DQM/CSCMonitorModule/plugins/CSCDQM_Summary.h"
#else
#include "DQM/CSCMonitorModule/src/CSCDQM_Summary.h"
#endif

#include "emu/dqm/common/ConsumerCanvas.hh"
#include "emu/dqm/common/EmuMonitoringObject.h"
#include "emu/dqm/common/EmuMonitoringCanvas.h"
#include "emu/dqm/common/EmuDQM_Utils.h"
#include "emu/dqm/common/EmuDQM_AppParameters.h"
#include "emu/dqm/common/CSCReadoutMappingFromFile.h"
#include "emu/dqm/common/CSCReport.h"
#include "emu/dqm/common/CSCDqmFact.h"

typedef std::map<std::string, std::set<int> > MapType;
typedef std::map<std::string, std::map<std::string, std::string> >Counters;
typedef Counters NodesStatus;

class FoldersMap: public MapType
{
public:
  FoldersMap(): MapType(), timestamp(0) {};
  time_t getTimeStamp() const
  {
    return timestamp;
  }
  void setTimeStamp(time_t t)
  {
    timestamp=t;
  }
  void clear()
  {
    timestamp=0;
    MapType::clear();
  }
private:
  time_t timestamp;

};

class CSCCounters: public Counters
{
public:
  CSCCounters(): Counters(), timestamp(0) {};
  time_t getTimeStamp() const
  {
    return timestamp;
  }
  void setTimeStamp(time_t t)
  {
    timestamp=t;
  }
  void clear()
  {
    timestamp=0;
    Counters::clear();
  }
private:
  time_t timestamp;
};

class DQMNodesStatus: public NodesStatus
{
public:
  DQMNodesStatus(): NodesStatus(), timestamp(0) {};
  time_t getTimeStamp() const
  {
    return timestamp;
  }
  void setTimeStamp(time_t t)
  {
    timestamp=t;
  }
  void clear()
  {
    timestamp=0;
    NodesStatus::clear();
  }
private:
  time_t timestamp;
};

typedef DQMNodesStatus DQMNodesStatusT;
typedef std::list< std::pair<time_t, DQMNodesStatusT> > DQMNodesHistory; 

typedef CSCCounters CSCCountersT;
typedef std::list< std::pair<time_t, CSCCountersT> > CSCCountersHistory;

using namespace toolbox;

class EmuDisplayClient : 
	public emu::base::WebReporter,
	public emu::base::FactFinder,
	xdata::ActionListener,
	Task
{
public:

  //! define factory method for instantion of EmuLocalRUI application
  XDAQ_INSTANTIATOR();


  EmuDisplayClient(xdaq::ApplicationStub* stub) throw (xdaq::exception::Exception);
  ~EmuDisplayClient();

  // == Callback for requesting current exported parameter values
  void actionPerformed (xdata::Event& e);

  bool onError ( xcept::Exception& ex, void * context );


  // == Web callback functions
  void Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception);

  void getNodesStatus (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getCSCList (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getTestsList (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getRunsList (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);

  void getPlot (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getRefPlot (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getCSCCounters (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getCSCCountersHistory (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getDQMReport (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getROOTFile (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void getNodesHistory (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);

  void controlDQM (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void configureDQM (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void controlDisplay (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);
  void redir (xgi::Input * in, xgi::Output * out)  throw (xgi::exception::Exception);



  std::map<std::string, std::list<std::string> > requestObjectsList(xdaq::ApplicationDescriptor* monitor);
  std::map<std::string, std::list<std::string> > requestCanvasesList(xdaq::ApplicationDescriptor* monitor);
  TMessage* requestObjects(xdata::Integer nodeaddr,  std::string folder, std::string objname);
  TMessage* requestCanvas(xdata::Integer nodeaddr,  std::string folder, std::string objname, int width, int height);
  Counters requestCSCCounters(xdaq::ApplicationDescriptor* monitor);
  DQMReport requestReport(xdaq::ApplicationDescriptor* monitor);
  std::set<std::string> requestFoldersList(xdaq::ApplicationDescriptor* dest);

  emu::base::Fact findFact(const emu::base::Component& component, const std::string& factType);
  emu::base::FactCollection findFacts();


protected:
  

  void book();
  void cleanup();
  void saveNodesResults();
  int loadXMLBookingInfo(std::string xmlFile);
  int loadXMLCanvasesInfo(std::string xmlFile);
  void clearMECollection(std::map<std::string, ME_List > & collection);
  void clearMECollection(ME_List &collection);
  void printMECollection(ME_List &collection);
  void clearCanvasesCollection(std::map<std::string, MECanvases_List > & collection);
  void clearCanvasesCollection(MECanvases_List &collection);
  void printCanvasesCollection(std::map<std::string, MECanvases_List > & collection);
  void printCanvasesCollection(MECanvases_List &collection);
  EmuMonitoringObject* createME(DOMNode* MEInfo);

  TCanvas* getMergedCanvas(std::vector<TObject*>& canvases);

  bool isMEValid(std::map<std::string, ME_List >&  List, std::string Folder, std::string Name, EmuMonitoringObject*& mo);
  bool isMEValid(std::string Folder, std::string Name, EmuMonitoringObject*& mo)
  {
    return isMEValid(MEs, Folder, Name, mo);
  }
  bool bookME(std::map<std::string, ME_List >&  List, std::string Folder, std::string Name, std::string Title, EmuMonitoringObject*& mo);
  bool bookME(std::string Folder, std::string Name, std::string Title, EmuMonitoringObject*& mo)
  {
    return bookME(MEs, Folder, Name, Title, mo);
  }
  bool updateME(std::string Folder, std::string Name, EmuMonitoringObject*& mo, TFile* rootsrc=NULL);
  bool readME(std::string Folder, std::string Name, EmuMonitoringObject*& mo, TFile* rootsrc);


  bool isCanvasValid(std::map<std::string, MECanvases_List >&  List, std::string Folder, std::string Name, EmuMonitoringCanvas*& me);
  bool isCanvasValid(std::string Folder, std::string Name, EmuMonitoringCanvas*& me)
  {
    return isCanvasValid(MECanvases, Folder, Name, me);
  }
  bool bookCanvas(std::map<std::string, MECanvases_List >&  List, std::string Folder, std::string Name, std::string Title, EmuMonitoringCanvas*& me);
  bool bookCanvas(std::string Folder, std::string Name, std::string Title, EmuMonitoringCanvas*& me)
  {
    return bookCanvas(MECanvases, Folder, Name, Title, me);
  }
  MonitorElement* mergeObjects(std::vector<TObject*>& olist);
  void updateEfficiencyHistos(TFile* rootsrc=NULL);

  // Report related
  MonitorElement* findME(std::string tag, std::string name);
  std::string getCSCName(std::string cscID, int& crate, int& slot, int& CSCtype, int& CSCposition );
  std::string getCSCFromMap(int crate, int slot, int& csctype, int& cscposition);
  std::vector<std::string> getListOfFolders(std::string filter);
  int generateSummaryReport(std::string runname, DQMReport& report);
  std::string getReportJSON(std::string runname);
  DQMReport mergeNodesReports(std::map<uint32_t,DQMReport> rep_list);

  void setCSCMapFile(std::string filename);

  std::string getHref(xdaq::ApplicationDescriptor *appDescriptor);

  // EmuPage1 reports
  vector<emu::base::WebReportItem> materialToReportOnPage1();
  std::string getDQMState();
  std::string getDQMQuality();
  std::string getDQMUnpackingRate();
  std::string getDQMEventsRate();


  DQMReport updateNodesReports();
  FoldersMap updateFoldersMap();
  FoldersMap readFoldersMap(std::string runname);
  CSCCounters updateCSCCounters();
  DQMNodesStatus updateNodesStatus();
  std::vector<std::string> readRunsList();
  bool isCSCListFileAvailable(std::string runname);
  bool isDQMReportFileAvailable(std::string runname, std::string ver="");
  bool isCSCCountersFileAvailable(std::string runname);

  int syncMonitorsStates();
  int syncNodesToCurrentRun();
  int prepareReportFacts(std::string runname);
  int updateNodesStatusFacts();
  inline void addFact(const emu::base::Fact &fact) {
                                collectedFacts.push_back(fact);
                        }

  int addToNodesStatusHistory(time_t timestamp, DQMNodesStatus nodes_status);
  int addToCSCCountersHistory(time_t timestamp, CSCCounters counters);
  
  int svc();
  std::string generateLoggerName();

private:

  Logger 	logger_;
 
  // The list of Monitored Elements
  std::map<std::string, ME_List > 		MEs;
  std::map<std::string, MECanvases_List > 	MECanvases;

  std::map<std::string, ME_List > 		MEFactories;
  std::map<std::string, MECanvases_List > 	MECanvasFactories;

  // CSC summary maps
  cscdqm::Summary 	summary;
  ChamberMap 		chamberMap;
  SummaryMap 		summaryMap;
  EventDisplay* 	eventDisplay;

  DQMReport 			dqm_report;
  CSCReadoutMappingFromFile 	cscMapping;
  std::map<std::string, int> 	tmap;
  std::vector<std::string> 	runsList;
  std::map<std::string, std::vector<std::string> > multi_runList; 	// Run lists for multiple folders
  std::map<std::string, int>	nodeSyncFlag;	// Flags for Nodes syncronization

 


  toolbox::exception::HandlerSignature*	 	errorHandler_;

  std::set<xdaq::ApplicationDescriptor*> 	getAppsList(xdata::String className, xdata::String group="dqm");

  // List of all external data servers tids
  std::set<xdaq::ApplicationDescriptor*> 	monitors;


  xdata::String 	xmlHistosBookingCfgFile_;
  xdata::String 	xmlCanvasesCfgFile_;
  xdata::String 	cscMapFile_;


  std::string 		curRunNumber;
  xdata::String 	monitorClass_;
  xdata::String 	imageFormat_;
  xdata::String 	imagePath_;
  xdata::Boolean 	viewOnly_;
  xdata::Boolean 	debug;
  xdata::Boolean 	useExSys; 		// Use Expert System
  xdata::Boolean        enableNodesAutoSync;	// Enable sending of sync to current run commands to EmuMonitor nodes 
  xdata::String 	BaseDir;
  xdata::String 	ResultsDir;
  xdata::String 	refImagePath;     
  xdata::UnsignedInteger saveResultsDelay; 	// Time delay for sending saveResults command to Monitors 
   
  bool 			fSkipUpdateTasks;	// Skip updates tasks for one cycle if nodes were synched 

  FoldersMap 		foldersMap; 		// Associate DDUs and CSCs with Monitoring nodes
  CSCCounters 		cscCounters;	 	// CSC Counters from EmuMonitor nodes
  DQMNodesStatus 	nodesStatus; 		// DQM Monitoring Nodes Statuses
  DQMNodesStatus 	prevNodesStatus;	// Saved copy of previous DQM Monitoring Nodes Statuses
  DQMNodesHistory       nodesStatusHistory; 	// Container for Historical DQM Monitoring Nodes Statuses data
  xdata::UnsignedInteger maxNodesHistorySize;
  CSCCountersHistory    cscCountersHistory;	// Container for Historical DQM CSC Counters data
  xdata::UnsignedInteger maxCSCCountersHistorySize;

  BSem 			appBSem_;
  BSem			utilBSem_;
  struct timeval 	bsem_tout;

  std::list<emu::base::Fact> collectedFacts;

  toolbox::task::ActionSignature *readRunListSignature_;



};
typedef xdaq::ApplicationDescriptor* pApplicationDescriptor;

class Compare_ApplicationDescriptors
{
public:
  int operator() (const pApplicationDescriptor& ad1, const pApplicationDescriptor& ad2) const
  {
    return ad1->getInstance() < ad2->getInstance();
  }
};

#endif


