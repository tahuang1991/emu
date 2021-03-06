#ifndef EmuMonitoringCanvas_h
#define EmuMonitoringCanvas_h

#include <iostream>
#include <string>
#include <map>
#include <string>
#include <iomanip>
#include <set>
#include <sstream>

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOMNodeList.hpp>

using namespace XERCES_CPP_NAMESPACE;

// ==  ROOT Section
#include <TROOT.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TString.h>
#include <TFile.h>
#include <TRandom.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TPaveStats.h>
#include <TColor.h>
#include <TPaletteAxis.h>
#include <TPRegexp.h>

#include "ConsumerCanvas.hh"
#include "EmuMonitoringObject.h"

#include "functions/FunctionLauncher.h"

#define DEF_WIDTH 1024
#define DEF_HEIGHT 768

// class MonitorElement: public TH1 {};
// typedef EmuMonitorElement MonitorElement;
typedef ConsumerCanvas MonitoringCanvas;
class EmuMonitoringCanvas;
typedef std::map<std::string, EmuMonitoringCanvas*> MECanvases_List;
typedef MECanvases_List::iterator MECanvases_List_iterator;
typedef MECanvases_List::const_iterator MECanvases_List_const_iterator;

class EmuMonitoringCanvas
{

public:
  EmuMonitoringCanvas();
  // == Copy constructor
  EmuMonitoringCanvas(const EmuMonitoringCanvas&);

//	EmuMonitoringCanvas();
  EmuMonitoringCanvas(DOMNode *info);
  ~EmuMonitoringCanvas();
  EmuMonitoringCanvas& operator=(const EmuMonitoringCanvas&);
  bool operator<( const EmuMonitoringCanvas& s1)
  {
    return (getFullName()<s1.getFullName());
  };
  bool operator>(const EmuMonitoringCanvas& s1)
  {
    return (getFullName()>s1.getFullName());
  };
  bool operator==(const EmuMonitoringCanvas& s1)
  {
    return (getFullName()==s1.getFullName());
  };


  int Book();
  int Book(DOMNode *info);

  void Reset();

  MonitoringCanvas* getCanvasObject()
  {
    return canvas;
  }

  MonitoringCanvas* setCanvasObject(MonitoringCanvas* cnv) 
  {
    if (cnv != NULL) {
	if (canvas != NULL) delete canvas;
	canvas = cnv;
    }
    return canvas;
  }

  void setPrefix(std::string);
  std::string getPrefix() const
  {
    return prefix;
  }
  std::string getName() const
  {
    return name;
  }
  void setName(std::string);
  std::string getTitle() const
  {
    return title;
  }
  void setTitle(std::string);
  std::string getFolder() const
  {
    return folder;
  }
  void setFolder(std::string);
  void setRunNumber(std::string num)
  {
    runNumber=num;
  }

  int setParameter(std::string, std::string);
  std::string getParameter(std::string);
  int setParameters(std::map<std::string, std::string>, bool resetParams = true);
  std::map<std::string, std::string>getParameters() const
  {
    return params;
  }
  std::string getFullName() const
  {
    return type+"_"+prefix+"_"+name;
  }
  int getCanvasWidth() const
  {
    return cnv_width;
  }
  void setCanvasWidth(int width)
  {
    if (width>0) cnv_width = width;
  }
  int getCanvasHeight() const
  {
    return cnv_height;
  }
  void setCanvasHeight(int height)
  {
    if (height>0) cnv_height = height;
  }
  bool getDisplayInWeb()
  {
    return displayInWeb;
  }

  void Write()
  {
    if (canvas!=NULL) canvas->Write();
  }
  void Draw(ME_List& MEs, bool useDrawType=false);
  void Draw(ME_List& MEs, int width, int height, bool useDrawType=false);
  void Print(const char* imgfile)
  {
    if (canvas!=NULL) canvas->Print(imgfile);
  }

  void setChamberMap(ChamberMap *map)
  {
    // chamberMap=map; // new ChamberMap implementation doesn't allow reuse of ChamberMap object
  }
  void setSummaryMap(SummaryMap *map)
  {
    summaryMap=map;
  }
  void setEventDisplay(EventDisplay *ed)
  {
    eventDisplay=ed;
  }

  void reset()
  {
    if (canvas!=NULL) delete canvas;
    canvas=NULL;
  }

  std::map<std::string, std::string> getListOfPads();
  time_t getTimeStamp() const
  {
    return timestamp;
  }
  
  void setTimeStamp()
  {
    timestamp = time(NULL);
  }
  void setTimeStamp(time_t tstamp)
  {
    timestamp = tstamp;
  }

//	DOMNode * getDOMInfo();
// 	void setDOMInfo(DOMNode *info);

private:

  static FunctionLauncher flauncher;
  int parseDOMNode(DOMNode* info);
  MonitoringCanvas* canvas;
  std::map<std::string, std::string>params;
  std::string type;
  std::string prefix;
  std::string folder;
  std::string name;
  std::string title;
  bool displayInWeb;
  int cnv_width;
  int cnv_height;
  std::string runNumber;
  ChamberMap* chamberMap;
  SummaryMap* summaryMap;
  EventDisplay* eventDisplay;
  time_t timestamp;

};

/*
bool operator<(const EmuMonitoringCanvas& s1, const EmuMonitoringCanvas& s2)
		{return (s1.getFullName()<s2.getFullName());};
bool operator>(const EmuMonitoringCanvas& s1, const EmuMonitoringCanvas& s2)
                {return (s1.getFullName()>s2.getFullName());};
bool operator==(const EmuMonitoringCanvas& s1, const EmuMonitoringCanvas& s2)
                {return (s1.getFullName()==s2.getFullName());};
*/


#endif

