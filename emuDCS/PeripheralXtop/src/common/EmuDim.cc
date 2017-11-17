// $Id: EmuDim.cc,v 1.49 2012/06/11 03:30:45 liu Exp $

#include "emu/x2p/EmuDim.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <time.h>
#include <iomanip>
#include <sstream>

namespace emu {
  namespace x2p {

EmuDim::EmuDim(xdaq::ApplicationStub * s): xdaq::WebApplication(s)
{
  HomeDir_     = getenv("HOME");
  ConfigDir_   = HomeDir_+"/config/";
  //
  PeripheralCrateDimFile_  = "";
  BadChamberFile_ = "";
  XmasDcsUrl_ = "";
  BlueDcsUrl_ = "";
  FedcDcsUrl_ = "";
  TestPrefix_ = "";
  OpMode_ = 0;
  EndCap_ = 0;
  heartbeat = 0;
  readin_ = 0;
  read_timeout = 0;
  lastread_ch = -1;

  xgi::bind(this,&EmuDim::Default, "Default");
  xgi::bind(this,&EmuDim::MainPage, "MainPage");
  //
  current_state_ = 0;
  xmas_state_ = 0;
  blue_state_ = 0;
  yp_state_ = 0;
  old_x2p_state = 0;
  old_xmas_state = 0;
  old_blue_state = 0;
  old_yp_state = 0;
  getApplicationInfoSpace()->fireItemAvailable("xmlFileName", &PeripheralCrateDimFile_);
  getApplicationInfoSpace()->fireItemAvailable("BadChamberFileName", &BadChamberFile_);
  getApplicationInfoSpace()->fireItemAvailable("XmasDcsUrl", &XmasDcsUrl_ );
  getApplicationInfoSpace()->fireItemAvailable("BlueDcsUrl", &BlueDcsUrl_ );
  getApplicationInfoSpace()->fireItemAvailable("FedcDcsUrl", &FedcDcsUrl_ );
  getApplicationInfoSpace()->fireItemAvailable("TestPrefix", &TestPrefix_ );
  getApplicationInfoSpace()->fireItemAvailable("OperationMode", &OpMode_ );
  getApplicationInfoSpace()->fireItemAvailable("EndCap", &EndCap_ );

  // everything below for Monitoring
  timer_ = toolbox::task::getTimerFactory()->createTimer("EmuMonitorTimer");
  timer_->stop();
  inited=false;
  Monitor_On_ = false;
  Monitor_Ready_ = false;
  In_Monitor_ = false;
  Suspended_ = false;
  fastloop=0;
  slowloop=0;
  extraloop=0;
  getApplicationInfoSpace()->fireItemAvailable("FastLoop", &fastloop);
  getApplicationInfoSpace()->fireItemAvailable("SlowLoop", &slowloop);
  getApplicationInfoSpace()->fireItemAvailable("ExtraLoop", &extraloop);
  xoap::bind(this, &EmuDim::SoapStart, "SoapStart", XDAQ_NS_URI);
  xoap::bind(this, &EmuDim::SoapStop, "SoapStop", XDAQ_NS_URI);
  xoap::bind(this, &EmuDim::SoapInfo, "SoapInfo", XDAQ_NS_URI);
  xgi::bind(this,&EmuDim::ButtonStart      ,"ButtonStart");
  xgi::bind(this,&EmuDim::ButtonStop      ,"ButtonStop");
  xgi::bind(this,&EmuDim::SwitchBoard      ,"SwitchBoard");

  for(int i=0; i<TOTAL_CRATES; i++) 
  {  crate_state[i]=0; 
     crate_name[i]=""; 
  }
}  

xoap::MessageReference EmuDim::SoapStart (xoap::MessageReference message) 
  throw (xoap::exception::Exception) 
{
     Start();
     return createReply(message);
}

xoap::MessageReference EmuDim::SoapStop (xoap::MessageReference message) 
  throw (xoap::exception::Exception) 
{
     Stop();
     return createReply(message);
}

xoap::MessageReference EmuDim::SoapInfo (xoap::MessageReference message) 
  throw (xoap::exception::Exception) 
{
   if(inited)
   {
     std::string xmasst=getAttrFromSOAP(message, "MonitorState");
     if(xmasst!="")
     {
        xmas_state_ = 1;
        if(xmasst=="ON" || xmasst=="on")   xmas_state_ = 2;
        if(xmasst=="OFF" || xmasst=="off")  xmas_state_ = 4;   
        std::cout << getLocalDateTime() << " Xmas Report: state changed to " << xmasst << std::endl;
        std::string confirm = "XMAS_" + xmasst;
        strcpy(pvssrespond.command, confirm.c_str());
        Confirmation_Service->updateService();
     }
     else
     {
        std::string cratename=getAttrFromSOAP(message, "CrateUp");
        int i=CrateToNumber( cratename.c_str() );
        if(i>=0 && i<TOTAL_CRATES)
        {
            crate_state[i] = 0;
            std::string confirm = "INIT_IS_DONE;" + crate_name[i];
            std::cout << getLocalDateTime() << " BP Return: " << confirm << std::endl;
            XmasLoader->reload(xmas_info+"?CRATEON="+crate_name[i]);
            strcpy(pvssrespond.command, confirm.c_str());
            Confirmation_Service->updateService();
        }
     }
   }
   return createReply(message);
}

void EmuDim::timeExpired (toolbox::task::TimerEvent& e)
{
     if(! Monitor_On_ ) return;

     std::string name = e.getTimerTask()->name;
     // std::cout << "timeExpired: " << name << std::endl;
     if(strncmp(name.c_str(),"EmuDimRead",13)==0) heartbeat++;

     if( In_Monitor_ ) return;
     In_Monitor_ = true;

     // always check DimCommand
     CheckCommand();

     // update states
     if(xmas_state_ != old_xmas_state)
     {
         EmuDim_xmas.XMAS_info = xmas_state_;
         EmuDim_xmas.update_time = (int) time(NULL);
         XMAS_1_Service->updateService();
         old_xmas_state=xmas_state_;
     }

     // check reading timeout
     if(strncmp(name.c_str(),"EmuDimCmnd",13)==0) 
     {  if( readin_==1 || readin_==3 )
        {
           time_t nowtime=time(NULL);
           if((nowtime-readtime_)>60) 
           {   
              if(read_timeout != readin_)
                 std::cout << "Timeout " << readin_ << " on " << heartbeat << std::endl;
              read_timeout = readin_;
           }
           else
           {
              read_timeout = 0;
           }
        }
     }

     // if( Suspended_ ) return;
     if(strncmp(name.c_str(),"EmuDimRead",13)==0) 
     {  if( !Suspended_ ) ReadFromXmas();
        UpdateAllDim();
     }
     In_Monitor_ = false;
}
//
void EmuDim::Start () 
{
     if(!Monitor_Ready_)
     {
         toolbox::TimeInterval interval1, interval2, interval3;
         toolbox::TimeVal startTime;
         startTime = toolbox::TimeVal::gettimeofday();

         if(!inited) Setup();
         timer_->start(); // must activate timer before submission, abort otherwise!!!
         if(fastloop) 
         {   interval1.sec((time_t)fastloop);
             timer_->scheduleAtFixedRate(startTime,this, interval1, 0, "EmuDimCmnd" );
             std::cout << "Dim Command scheduled" << std::endl;
             ::sleep(2);
         }
         if(slowloop) 
         {   interval2.sec((time_t)slowloop);
             timer_->scheduleAtFixedRate(startTime,this, interval2, 0, "EmuDimRead" );
             std::cout << "XMAS Read scheduled" << std::endl;
         }
         if(extraloop) 
         {   interval3.sec((time_t)extraloop);
             timer_->scheduleAtFixedRate(startTime,this, interval3, 0, "EmuDimExtra" );
             std::cout << "extra scheduled" << std::endl;
         }
         Monitor_Ready_=true;
     }
     Monitor_On_=true;
     Suspended_ = false;
     std::cout<< getLocalDateTime() << " EmuDim Started " << std::endl;
}

void EmuDim::Stop () 
{
     if(Monitor_On_)
     {
         Monitor_On_=false;
         std::cout << getLocalDateTime() << " EmuDim stopped " << std::endl;
     }
}

void EmuDim::ButtonStart(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
     Start();
     this->Default(in,out);
}

void EmuDim::ButtonStop(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
     Stop();
     this->Default(in,out);
}

void EmuDim::SwitchBoard(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
    cgicc::CgiEnvironment cgiEnvi(in);
    std::string Page=cgiEnvi.getPathInfo()+"?"+cgiEnvi.getQueryString();
    Page=cgiEnvi.getQueryString();
    std::string command_name=Page.substr(0,Page.find("=", 0) );
    std::string command_argu=Page.substr(Page.find("=", 0)+1);

    if (command_name=="STATUS")
    {
       if(Monitor_On_) *out << "ON ";
       else *out << "OFF ";

       *out << heartbeat;

       if(Suspended_) *out << " SUSPENDED" << std::endl;
       else *out << " READING" << std::endl;
    }
    else if (command_name=="READING")
    {
       if(command_argu=="ON" || command_argu=="on")
       {   
           XmasLoader->reload(xmas_start);
           Suspended_ = false;
           std::cout << getLocalDateTime() << " SwitchBoard: Resume Reading" << std::endl;
       }
       else if (command_argu=="OFF" || command_argu=="off")
       { 
           XmasLoader->reload(xmas_stop);
           Suspended_ = true;
           std::cout << getLocalDateTime() << " SwitchBoard: Stop Reading" << std::endl;
       }
    }
    else
    {
       std::cout << getLocalDateTime() << " Unknown command: " << command_name << " " << command_argu << std::endl;
    }
}

void EmuDim::Default(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception)
{
  *out << "<meta HTTP-EQUIV=\"Refresh\" CONTENT=\"0; URL=/"
       <<getApplicationDescriptor()->getURN()<<"/"<<"MainPage"<<"\">" <<std::endl;
}
//
/////////////////////////////////////////////////////////////////////////////////
// Main page description
/////////////////////////////////////////////////////////////////////////////////
void EmuDim::MainPage(xgi::Input * in, xgi::Output * out ) throw (xgi::exception::Exception) {
  //
   int endcap = EndCap_;
   if(endcap > 0)
   {
      MyHeader(in,out,"EmuDim -- Plus Endcap");
   }
   else if(endcap < 0)
   {
      MyHeader(in,out,"EmuDim -- Minus Endcap");
   }
   else
      MyHeader(in,out,"EmuDim");
  //
  if(Monitor_On_)
  {
     *out << cgicc::span().set("style","color:green");
     *out << cgicc::b(cgicc::i("X2P Status: On")) << cgicc::span() << std::endl ;
     std::string TurnOffMonitor = toolbox::toString("/%s/ButtonStop",getApplicationDescriptor()->getURN().c_str());
     *out << cgicc::form().set("method","GET").set("action",TurnOffMonitor) << std::endl ;
     *out << cgicc::input().set("type","submit").set("value","Turn Off X2P") << std::endl ;
     *out << cgicc::form() << std::endl ;
  } else
  {
     *out << cgicc::span().set("style","color:red");
     *out << cgicc::b(cgicc::i("X2P Status: Off")) << cgicc::span() << std::endl ;
     std::string TurnOnMonitor = toolbox::toString("/%s/ButtonStart",getApplicationDescriptor()->getURN().c_str());
     *out << cgicc::form().set("method","GET").set("action",TurnOnMonitor) << std::endl ;
     *out << cgicc::input().set("type","submit").set("value","Turn On X2P") << std::endl ;
     *out << cgicc::form() << std::endl ;
  }
  if(Suspended_) 
  {  *out << cgicc::span().set("style","color:yellow")
          << cgicc::b("XMAS suspended") << cgicc::span() << std::endl;
  }
  //
}
//
void EmuDim::MyHeader(xgi::Input * in, xgi::Output * out, std::string title ) 
  throw (xgi::exception::Exception) {
  //
  *out << "<h1 style=\"text-align: center\"> " << title << "</h1>" << std::endl;
  *out << "<h5 style=\" font-weight: regular; text-align: center\"> " << "( time stamp: " << getLocalDateTime()  << " ) </h5>" << std::endl;
  //
}

void EmuDim::Setup()
{
   XmasLoader = new LOAD();
   BlueLoader = new LOAD();
   FedcLoader = new LOAD();

   xmas_root=XmasDcsUrl_;
   xmas_load=xmas_root + "/DCSOutput";
   xmas_load2=xmas_root + "/DCSOutput2";
   xmas_start=xmas_root + "/MonitorStart";
   xmas_stop=xmas_root + "/MonitorStop";
   xmas_info=xmas_root + "/SwitchBoard";
   XmasLoader->init(xmas_load.c_str());

   blue_root=BlueDcsUrl_;
   blue_info=blue_root + "/SwitchBoard";
   BlueLoader->init(blue_info.c_str());

   fedc_root=FedcDcsUrl_;
   fedc_load=fedc_root + "/DCSOutput";
   FedcLoader->init(fedc_load.c_str());
   FedcLoader->settimeout(60);

   std::string fn=PeripheralCrateDimFile_;
   int ch=ReadFromFile(fn.c_str());
   if(ch<=0) 
      std::cout << "ERROR in read file " << fn << std::endl;

   ch=ReadFromXmas();
   // try one more time for the first time
   if(ch<=0) 
   {
      ::sleep(60);
      heartbeat++;
      ch=ReadFromXmas();
   }
   if(ch<=0) std::cout << "ERROR in connecting monitor process." << std::endl;
   StartDim();
   inited=true;
}

int EmuDim::ReadFromFile(const char *filename)
{
   FILE * fl;
   char *buffer;
   int readsize, ch=0, chd=0;

   if(strlen(filename)==0) return 0;
   fl=fopen(filename, "r");
   if(fl==NULL) return 0;
   buffer=(char *)malloc(100000);
   if(buffer==NULL) return 0;
   // then fill the structure
   readsize=fread(buffer, 1, 100000, fl);
   if(readsize>40) ch=ParseTXT(buffer, readsize, 1, 0);
   std::cout << ch << " Chambers read from file " << filename << std::endl;
   fseek(fl,0,SEEK_SET);
   readsize=fread(buffer, 1, 100000, fl);
   if(readsize>40) chd=ParseDDU(buffer, readsize, 1);
   std::cout << chd << " DDUs read from file " << filename << std::endl;
   free(buffer);
   fclose(fl);
   return ch;
}

int EmuDim::ReadFromXmas()
{
   int ch1=0, ch2=0, ch=0, du=0;

   readin_=1;
   readtime_=time(NULL);
   readin_=2; 

   // read
   XmasLoader->reload(xmas_load);
   // then fill the structure
   ch1=ParseTXT(XmasLoader->Content(), XmasLoader->Content_Size(), 0, 1);
   XmasLoader->reload(xmas_load2);
   ch2=ParseTXT(XmasLoader->Content(), XmasLoader->Content_Size(), 0, 2);
   ch=ch1+ch2;
   std::cout << "Cycle " << heartbeat << " read " << ch << " Chambers and ";
   
   // if no chamber info read back, report Xmas state as error
   if(ch>0 && (xmas_state_ & 0x100)) xmas_state_ &= 0xfffffeff;
   if(ch==0 && lastread_ch==0) xmas_state_ |= 0x100;
   lastread_ch=ch;
   // 
   readin_=3;
   readtime_=time(NULL);
   // read
   FedcLoader->reload(fedc_load);

   readin_=4;
   // then fill the structure
   du=ParseDDU(FedcLoader->Content(), FedcLoader->Content_Size(), inited?0:1);
   std::cout << du << " DDUs at " << getLocalDateTime() << std::endl;
   // 
   readin_=0;
   return ch;
}

int EmuDim::ParseTXT(char *buff, int buffsize, int source, int type)
{
//
// source==0  from Xmas
//       ==1  from file
//
   int chmbs=0;
   bool more_line = true;
   char * start = buff;
   char *endstr;
   unsigned char *bbb=(unsigned char *)buff;

   if(source==0 && buffsize<100) return 0;
   for(int i=0; i<buffsize; i++)
   {  if((bbb[i]< 0x20 || bbb[i]>0x7e) && bbb[i]!=0x0a)
      { std::cout << "ERROR at " << i << " " << std::hex << (int)(bbb[i]) << std::dec << std::endl;
        bbb[i]=0x0a;
      }
   }
   do
   {
       endstr=strchr(start, '\n');
       if(endstr==NULL || ((endstr-buff)>buffsize)) more_line=false;
       else
       {
           *endstr=0;
           chmbs += FillChamber(start, source, type);
       }
       start = endstr+1;
   } while(more_line);
   return chmbs;
}

int EmuDim::FillChamber(char *buff, int source, int type)
{
   char *content;
   char * endstr;
   std::string label;

   if(source==0 && strlen(buff) < 100) return 0;
   endstr=strchr(buff, ' ');
   if(endstr==NULL) return 0;
   *endstr=0;
   int chnumb=ChnameToNumber(buff);
   label=buff;
//   std::cout << "Found chamber " << label << " with number " << chnumb << std::endl; 
   content = endstr+1;
   if((type<=1 && strlen(content)>800) || (type==2 && strlen(content)>4000)) std::cout<< label << " WARNING type: " << type << " content: " << content << std::endl;
   if(chnumb>=0 && chnumb <TOTAL_CHAMBERS)
   {   chamb[chnumb].SetLabel(label);
       chamb[chnumb].Fill(content, source);
       return 1;
   }
   else
   {   if(source==0) std::cout << "WRONG tag " << label << std::endl;
       return 0;
   }
}

int EmuDim::ChnameToNumber(const char *chname)
{
   int endcap = EndCap_;

   if(strlen(chname)<7) return -1;
   if(endcap > 0)
   {
      if(strncmp(chname,"ME+",3)) return -2;
   }
   else if(endcap < 0)
   {
      if(strncmp(chname,"ME-",3)) return -2;
   }
   else
      if(strncmp(chname,"ME",2)) return -2;
   int station = std::atoi(chname+3);
   int ring = std::atoi(chname+5);
   int chnumb = std::atoi(chname+7);
   if(ring<1 || ring>3) return -3;
   switch(station)
   {   case 1:
         return     (ring-1)*36 + chnumb-1;
       case 2:
         return 108+(ring-1)*18 + chnumb-1;
       case 3:
         return 162+(ring-1)*18 + chnumb-1;
       case 4:
         return 216+(ring-1)*18 + chnumb-1;
       default:
         return -3;
   }
}

int EmuDim::CrateToNumber(const char *chname)
{
   int endcap = EndCap_;

   if(strlen(chname)<7) return -1;
   if(endcap > 0)
   {
      if(strncmp(chname,"VMEp",4)) return -2;
   }
   else if(endcap < 0)
   {
      if(strncmp(chname,"VMEm",4)) return -2;
   }
   else
      if(strncmp(chname,"VME",3)) return -2;
     
   int station = std::atoi(chname+4);
   int chnumb = std::atoi(chname+6);
   if(chnumb>12 || chnumb<1) return -3;

   if (station >4 || station<1) return -4;
   else
   {  int cr = chnumb-1;
      if (station>1) cr += station*6;
      if(crate_name[cr]=="") crate_name[cr] = chname;
      return cr;
   }
}

int EmuDim::ParseDDU(char *buff, int buffsize, int source)
{
//
// source==0  from Xmas
//       ==1  from file
//
   int chmbs=0;
   bool more_line = true;
   char * start = buff;
   char *endstr;
   unsigned char *bbb=(unsigned char *)buff;

   if(source==0 && buffsize < 50) return 0;
   for(int i=0; i<buffsize; i++)
   {  if((bbb[i]< 0x20 || bbb[i]>0x7e) && bbb[i]!=0x0a)
      { std::cout << "ERROR at " << i << " " << std::hex << (int)(bbb[i]) << std::dec << std::endl;
        bbb[i]=0x0a;
      }
   }
   do
   {
       endstr=strchr(start, '\n');
       if(endstr==NULL || ((endstr-buff)>buffsize)) more_line=false;
       else
       {
           *endstr=0;
           chmbs += FillDDU(start, source);
       }
       start = endstr+1;
   } while(more_line);
   return chmbs;
}

int EmuDim::FillDDU(char *buff, int source)
{
   char *content;
   char * endstr;
   std::string label;

   if(source==0 && strlen(buff) < 50) return 0;
   endstr=strchr(buff, ' ');
   if(endstr==NULL) return 0;
   *endstr=0;
   int chnumb=atoi(buff+3);
   // names with DDU8XX
   if(chnumb>860) chnumb=(chnumb-842);
   else if(chnumb>850) chnumb=(chnumb-823);
   else if(chnumb>840) chnumb=(chnumb-840);
   else if(chnumb>830) chnumb=(chnumb-821);
   else
   {  // names with DDUxx
     if(chnumb>100) chnumb=(chnumb%10)+(chnumb%100)/10; // for non-standard DDU #, create one in the range.
   }
   if(chnumb>18) chnumb -= 18;  // now chnumb is used as the array index (not DDU number) must 0-17
   label=buff;
   // std::cout << "Found DDU " << label << " with number " << chnumb << std::endl; 
   content = endstr+1;
   if(strlen(content)>100) std::cout<< label << " WARNING " << content << std::endl;
   if(strncmp(buff, "DDU", 3)==0 && chnumb>0 && chnumb <TOTAL_DDUS)
   {   ddumb[chnumb].SetLabel(label);
       ddumb[chnumb].Fill(content, source);
       return 1;
   }
   else
   {   if(source==0) std::cout << "WRONG tag " << label << std::endl;
       return 0;
   }
}

void EmuDim::StartDim()
{
   int total=0, total_d=0, i=0;
   std::string dim_lv_name, dim_temp_name, dim_ddu_name, dim_command, dim_server, pref;
   std::string dim_lv2_name, dim_temp2_name;
   
   pref=TestPrefix_;

   std::string xmas_server = pref + "XMAS_X2P";
   EmuDim_xmas.XMAS_info=0;
   EmuDim_xmas.X2P_info=0;
   EmuDim_xmas.update_time= (int) time(NULL);
   XMAS_1_Service= new DimService(xmas_server.c_str(),"I:3",
           &EmuDim_xmas, sizeof(XMAS_1_DimBroker));
   
   while(i < TOTAL_CHAMBERS)
   {
      if(chamb[i].Ready())
      {
        if(chamb[i].GetType()<=1)
        {
         chamb[i].GetDimLV(0, &(EmuDim_lv[i]));
         chamb[i].GetDimTEMP(0, &(EmuDim_temp[i]));
         dim_lv_name = pref + "LV_1_" + chamb[i].GetLabel(); 
         dim_temp_name = pref + "TEMP_1_" + chamb[i].GetLabel(); 

         LV_1_Service[i]= new DimService(dim_lv_name.c_str(),"F:5;F:5;F:5;F:5;F:5;F:5;F:8;F:14;F:2;I:4",
           &(EmuDim_lv[i]), sizeof(LV_1_DimBroker));
         TEMP_1_Service[i]= new DimService(dim_temp_name.c_str(),"F:7;I:2",
           &(EmuDim_temp[i]), sizeof(TEMP_1_DimBroker));
        }
        else
        {
         chamb[i].GetDimLV2(0, &(EmuDim_lv2[i]));
         chamb[i].GetDimTEMP2(0, &(EmuDim_temp2[i]));
         dim_lv2_name = pref + "LV_2_" + chamb[i].GetLabel(); 
         dim_temp2_name = pref + "TEMP_2_" + chamb[i].GetLabel(); 

         LV_1_Service[i]= new DimService(dim_lv2_name.c_str(),"F:7;F:7;F:7;F:7;F:7;F:7;F:8;F:14;F:7;F:7;F:2;F:7;I:3;I:7;I:7;I:7;I:7;I:4",
           &(EmuDim_lv2[i]), sizeof(LV_2_DimBroker));
         TEMP_1_Service[i]= new DimService(dim_temp2_name.c_str(),"F:4;F:7;F:7;F:7;I:2",
           &(EmuDim_temp2[i]), sizeof(TEMP_2_DimBroker));
        }
         total++;
      }
      i++;
   }

   for(i=1;  i<TOTAL_DDUS; i++)
   {
      if(ddumb[i].Ready())
      {
         ddumb[i].GetDimDDU(0, &(EmuDim_ddu[i]));
         dim_ddu_name = pref + "FED_1_" + ddumb[i].GetLabel(); 

         DDU_1_Service[i]= new DimService(dim_ddu_name.c_str(),"F:8;I:2",
           &(EmuDim_ddu[i]), sizeof(DDU_1_DimBroker));

         total_d++;
      }
   }
   std::string confirm_cmd = pref + "LV_CONFIRMATION_SERVICE";
   Confirmation_Service = new DimService(confirm_cmd.c_str(),"C:80", (void *)&pvssrespond, sizeof(pvssrespond));

   dim_command = pref + "LV_1_COMMAND";
   LV_1_Command = new DimCommand( dim_command.c_str(),"C");
   dim_server = pref + "Emu-Dcs Dim Server";
   DimServer::start(dim_server.c_str());

   strcpy(pvssrespond.command, "SOFT_START");
   Confirmation_Service->updateService();
 
   std::cout << total << " Chamber serives and " << total_d << " DDU services";
   if(pref!="") std::cout << " ( with prefix " << pref << " )";
   std::cout << " started at " << getLocalDateTime() << std::endl;
}

int EmuDim::UpdateChamber(int ch)
{
   int mode = OpMode_;
   bool lv_ok=false, temp_ok=false;

   if(mode<=0) mode = 2;
   if(ch>=0 && ch < TOTAL_CHAMBERS && chamb[ch].Ready())
   {
     if(chamb[ch].GetType()<=1)
     {
         lv_ok=chamb[ch].GetDimLV(mode, &(EmuDim_lv[ch]));
         temp_ok=chamb[ch].GetDimTEMP(mode, &(EmuDim_temp[ch]));
     }
     else
     {
         lv_ok=chamb[ch].GetDimLV2(mode, &(EmuDim_lv2[ch]));
         temp_ok=chamb[ch].GetDimTEMP2(mode, &(EmuDim_temp2[ch]));
     }
     try 
     {
         if(lv_ok && LV_1_Service[ch]) LV_1_Service[ch]->updateService();
         if(temp_ok && TEMP_1_Service[ch]) TEMP_1_Service[ch]->updateService();
     } catch (...) {}
         return 1;
   }
   else return 0;
}

int EmuDim::UpdateDDU(int ch)
{
   int mode = OpMode_;
   if(mode<=0) mode = 2;
   if(ch>0 && ch < TOTAL_DDUS && ddumb[ch].Ready())
   {
         ddumb[ch].GetDimDDU(mode, &(EmuDim_ddu[ch]));
     try 
     {
         if(DDU_1_Service[ch]) DDU_1_Service[ch]->updateService();
     } catch (...) {}
         return 1;
   }
   else return 0;
}

void EmuDim::UpdateAllDim()
{
   if(!inited)
   {
      StartDim();
      inited=true;
      return;
   }
   int j=0;

   for(int i=0; i < TOTAL_CHAMBERS; i++)
   {
      j += UpdateChamber(i);
   }
   for(int i=1; i < TOTAL_DDUS; i++)
   {
      j += UpdateDDU(i);
   }
   // std::cout << j << " Dim services updated" << std::endl;
}

void EmuDim::CheckCommand()
{
   if(LV_1_Command==NULL) return;
   bool start_powerup=false;
   int more_command=0;
   std::string cmnd="";
   try 
     {
        more_command = LV_1_Command->getNext();
        if (more_command>0) cmnd = LV_1_Command->getString();
     } catch (...) 
     {   
        std::cout << getLocalDateTime() << " Get Dim Command error" << std::endl;
        // more_command = 0; 
     }
   while(more_command>0)
   {
      std::cout << getLocalDateTime() << " Dim Command:" << cmnd << std::endl;
      if(cmnd.substr(0,10)=="STOP_SLOW_")
      {
         XmasLoader->reload(xmas_stop);
         Suspended_ = true;
         start_powerup=false;
         for(int i=0; i<TOTAL_CRATES; i++)
         {  
            crate_state[i]=0;
         }
      }
      else if(cmnd.substr(0,12)=="RESUME_SLOW_")
      {
         XmasLoader->reload(xmas_start);
         if(Suspended_)
         {  
            ::sleep(3);
            XmasLoader->reload(xmas_info+"?NEWDATA");
            Suspended_ = false;
         }
         start_powerup=false;
         for(int i=0; i<TOTAL_CRATES; i++)
         {
            crate_state[i]=0;
         }
      }
      else if(cmnd.substr(cmnd.length()-8,8)=="get_data")
      {
/*
         if(strncmp(cmnd.c_str(), "DDU", 3)==0)
         {
            int ch=atoi(cmnd.c_str()+3);
            if(ch>18) ch -= 18;
            if(ch>0 && ch<TOTAL_DDUS) UpdateDDU(ch);
         }
         else
         {
            int ch=ChnameToNumber(cmnd.c_str());
            if(ch>=0 && ch<TOTAL_CHAMBERS) UpdateChamber(ch);
         }
*/
      }
      else if(cmnd.substr(0,15)=="TURN_CHAMBER_OFF")
      {
         int cr=CrateToNumber(cmnd.substr(16).c_str());
         if(cr>=0 && cr<TOTAL_CRATES) crate_state[cr] = -1;
         BlueLoader->reload(blue_info+"?CHAMBEROFF="+crate_name[cr]);
      }
      else if(cmnd.substr(0,15)=="CRATE_POWER_OFF")
      {
         int cr=CrateToNumber(cmnd.substr(16).c_str());
         if(cr>=0 && cr<TOTAL_CRATES) crate_state[cr] = -1;
         XmasLoader->reload(xmas_info+"?CRATEOFF="+crate_name[cr]);
      }
      else if(cmnd.substr(0,13)=="PREPARE_POWER")
      {
         int cr=CrateToNumber(cmnd.substr(17).c_str());
         if(cr>=0 && cr<TOTAL_CRATES) crate_state[cr] = 1;
         if(!Suspended_)
         {
            XmasLoader->reload(xmas_stop);
         }
      }
      else if(cmnd.substr(0,13)=="EXECUTE_POWER")
      {
         BlueLoader->reload(blue_info+"?RELOAD");
         start_powerup=true;
      }
      // all other commands are ignored
      more_command=0;
      try 
        {
           more_command = LV_1_Command->getNext();
           if (more_command>0) cmnd = LV_1_Command->getString();
        } catch (...) 
        {   
           std::cout << getLocalDateTime() << " Get Dim Command error" << std::endl;
           // more_command = 0; 
        }
   }
   if(start_powerup) PowerUp();
}

int EmuDim::PowerUp()
{
   std::string pref = TestPrefix_;
   std::string confirm_cmd = pref + "LV_1_COMMAND_CONFIRMATION";
   int upcrates=0;
   for(int i=0; i<TOTAL_CRATES; i++)
   {
       if(crate_state[i]==1) upcrates++;
   }
   if(upcrates==0) return 0;
   std::cout << getLocalDateTime() << " Start Power Up " << upcrates << " crate(s)." << std::endl;
   // make sure Xmas stopped before power-up
   XmasLoader->reload(xmas_stop);
   for(int i=0; i<TOTAL_CRATES; i++)
   {  
      if(crate_state[i]==1) 
      {  
         // start initializing
         std::string confirm = "INITIALIZING;" + crate_name[i];              
         strcpy(pvssrespond.command, confirm.c_str());
         Confirmation_Service->updateService();
         std::cout << getLocalDateTime() << " Start Power-up Init crate " << crate_name[i] << std::endl;
         BlueLoader->reload(blue_info+"?POWERUP="+crate_name[i]);
         // check return message
         if(BlueLoader->Content_Size() > 27)
         {
           if(strncmp(BlueLoader->Content(), "Power Up Successful",19)==0)
           {
              crate_state[i] = 0;
              upcrates--;
              confirm = "INIT_IS_DONE;" + crate_name[i];              
              std::cout << getLocalDateTime() << " Return: " << confirm << std::endl;
              XmasLoader->reload(xmas_info+"?CRATEON="+crate_name[i]);
           }
           else
           {
              confirm = "INIT_FAILED;" + crate_name[i];              
              std::cout << getLocalDateTime() << " Init failed: " << crate_name[i] << std::endl;
           }
         }
         else
         {
            std::cout << "Blue Page returns bad message with total length: " << BlueLoader->Content_Size() << std::endl;
            confirm = "INIT_FAILED;" + crate_name[i];              
            std::cout << getLocalDateTime() << " Init failed: " << crate_name[i] << std::endl;
         }
         strcpy(pvssrespond.command, confirm.c_str());
         Confirmation_Service->updateService();
      }
   }
   // if Xmas was on before power-up, then resume it after all crates finished
   if(upcrates==0 && Suspended_==false) XmasLoader->reload(xmas_start);
   return 0;
}

xoap::MessageReference EmuDim::createReply(xoap::MessageReference message)
                throw (xoap::exception::Exception)
{
        std::string command = "";

        DOMNodeList *elements =
                        message->getSOAPPart().getEnvelope().getBody()
                        .getDOMNode()->getChildNodes();

        for (unsigned int i = 0; i < elements->getLength(); i++) {
                DOMNode *e = elements->item(i);
                if (e->getNodeType() == DOMNode::ELEMENT_NODE) {
                        command = xoap::XMLCh2String(e->getLocalName());
                        break;
                }
        }

        xoap::MessageReference reply = xoap::createMessage();
        xoap::SOAPEnvelope envelope = reply->getSOAPPart().getEnvelope();
        xoap::SOAPName responseName = envelope.createName(
                        command + "Response", "xdaq", XDAQ_NS_URI);
        envelope.getBody().addBodyElement(responseName);

        return reply;
}

std::string EmuDim::getAttrFromSOAP(xoap::MessageReference message, std::string tag)
{
        xoap::SOAPEnvelope envelope = message->getSOAPPart().getEnvelope();
        xoap::SOAPName name = envelope.createName(tag);
        std::string value = envelope.getBody().getAttributeValue(name);
        return value;
}

std::string EmuDim::getLocalDateTime(){
  time_t t;
  struct tm *tm;

  time( &t );
  tm = localtime( &t );

  std::stringstream ss;
  ss << std::setfill('0') << std::setw(4) << tm->tm_year+1900 << "-"
     << std::setfill('0') << std::setw(2) << tm->tm_mon+1     << "-"
     << std::setfill('0') << std::setw(2) << tm->tm_mday      << " "
     << std::setfill('0') << std::setw(2) << tm->tm_hour      << ":"
     << std::setfill('0') << std::setw(2) << tm->tm_min       << ":"
     << std::setfill('0') << std::setw(2) << tm->tm_sec;

  return ss.str();
}

  }  // namespace emu::x2p
}  // namespace emu

//
// provides factory method for instantion of XDAQ application
//
XDAQ_INSTANTIATOR_IMPL(emu::x2p::EmuDim)
