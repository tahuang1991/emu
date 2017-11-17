//-----------------------------------------------------------------------
// $Id: MPC.cc,v 3.25 2012/09/30 21:19:42 liu Exp $
// $Log: MPC.cc,v $
// Revision 3.25  2012/09/30 21:19:42  liu
// update for ME11 new electronics
//
// Revision 3.24  2012/05/09 20:59:00  liu
// fix missing standard header files
//
// Revision 3.23  2012/02/20 13:05:07  liu
// include Board ID in Configuration Check
//
// Revision 3.22  2011/10/14 17:42:14  liu
// fix warnings for GCC4
//
// Revision 3.21  2011/08/25 21:08:57  liu
// add functions for new (Virtex 5) MPC
//
// Revision 3.20  2011/07/28 16:28:38  liu
// update comments
//
// Revision 3.19  2011/06/30 21:26:36  liu
// add setDelay function
//
// Revision 3.18  2010/05/21 12:15:20  liu
// add MPC mask
//
// Revision 3.17  2010/05/05 11:46:58  liu
// make some stdout prints optional
//
// Revision 3.16  2009/03/25 10:19:41  liu
// move header files to include/emu/pc
//
// Revision 3.15  2008/08/13 11:30:54  geurts
// introduce emu::pc:: namespaces
// remove any occurences of "using namespace" and make std:: references explicit
//
// Revision 3.14  2008/08/08 11:01:24  rakness
// centralize logging
//
// Revision 3.13  2008/06/12 21:08:55  rakness
// add firmware tags for DMB, CFEB, MPC, CCB into xml file; add check firmware button
//
// Revision 3.12  2008/05/15 09:55:10  liu
// error messages for Check_Config
//
// Revision 3.11  2008/04/25 12:21:59  liu
// fix bug in CheckConfig
//
// Revision 3.10  2008/04/11 14:48:48  liu
// add CheckConfig() function
//
// Revision 3.9  2008/02/13 16:50:42  liu
// fix board ID setting
//
// Revision 3.8  2008/01/08 10:59:32  liu
// remove exit() in functions
//
// Revision 3.7  2007/08/28 18:06:00  liu
// remove unused & outdated functions
//
// Revision 3.6  2007/08/27 22:51:16  liu
// update
//
// Revision 3.5  2007/06/14 14:46:39  rakness
// remove output containing no information
//
// Revision 3.4  2006/10/30 10:02:10  rakness
// Update
//
// Revision 3.3  2006/09/27 16:50:43  mey
// Update
//
// Revision 3.2  2006/09/13 14:13:32  mey
// Update
//
// Revision 3.1  2006/08/07 14:14:10  mey
// Added BoardId
//
// Revision 3.0  2006/07/20 21:15:48  geurts
// *** empty log message ***
//
// Revision 2.31  2006/07/18 15:23:14  mey
// UPdate
//
// Revision 2.30  2006/07/13 15:46:37  mey
// New Parser strurture
//
// Revision 2.29  2006/07/12 07:58:18  mey
// Update
//
// Revision 2.28  2006/07/11 14:49:29  mey
// New Parser sturcture ready to go
//
// Revision 2.27  2006/07/11 09:31:12  mey
// Update
//
// Revision 2.26  2006/07/06 08:24:17  mey
// Bug fix
//
// Revision 2.25  2006/07/06 07:31:48  mey
// MPC firmware loading added
//
// Revision 2.24  2006/07/05 09:29:18  mey
// Update
//
// Revision 2.23  2006/06/16 13:05:24  mey
// Got rid of Compiler switches
//
// Revision 2.22  2006/05/12 08:03:06  mey
// Update
//
// Revision 2.21  2006/02/16 09:41:47  mey
// Fixed byte swap
//
// Revision 2.20  2006/02/15 22:39:57  mey
// UPdate
//
// Revision 2.19  2006/02/15 10:48:43  mey
// Fixed byte swapping
//
// Revision 2.18  2006/02/13 19:51:01  mey
// Fix bugs
//
// Revision 2.17  2006/01/14 22:24:50  mey
// UPdate
//
// Revision 2.16  2006/01/13 10:11:51  mey
// Update
//
// Revision 2.15  2006/01/12 23:44:43  mey
// Update
//
// Revision 2.14  2006/01/12 22:36:13  mey
// UPdate
//
// Revision 2.13  2006/01/12 12:28:50  mey
// UPdate
//
// Revision 2.12  2006/01/11 16:58:25  mey
// Update
//
// Revision 2.11  2006/01/11 13:47:47  mey
// Update
//
// Revision 2.10  2005/12/08 12:00:07  mey
// Update
//
// Revision 2.9  2005/11/25 23:42:46  mey
// Update
//
// Revision 2.8  2005/11/22 15:15:01  mey
// Update
//
// Revision 2.7  2005/11/21 17:38:28  mey
// Update
//
// Revision 2.6  2005/11/02 16:16:14  mey
// Update for new controller
//
// Revision 2.5  2005/08/23 19:27:17  mey
// Update MPC injector
//
// Revision 2.4  2005/08/22 07:55:45  mey
// New TMB MPC injector routines and improved ALCTTiming
//
// Revision 2.3  2005/08/17 12:27:22  mey
// Updated FindWinner routine. Using FIFOs now
//
// Revision 2.2  2005/08/11 08:13:04  mey
// Update
//
// Revision 2.1  2005/06/06 19:22:35  geurts
// additions for MPC/SP connectivity tests (Dan Holmes)
//
// Revision 2.0  2005/04/12 08:07:05  geurts
// *** empty log message ***
//
//
//-----------------------------------------------------------------------
#include <iostream>
#include <iomanip> 
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "emu/pc/MPC.h"
#include "emu/pc/VMEController.h"
#include "emu/pc/EmuLogger.h"

namespace emu {
  namespace pc {

MPC::MPC(Crate * theCrate, int slot) : VMEModule(theCrate, slot),EmuLogger(),
				       BoardId_(0), TLK2501TxMode_(0), TransparentModeSources_(0), TMBDelayPattern_(0)
{
  std::cout << "MPC: in crate=" << this->crate() 
	    << " slot=" << this->slot() << std::endl;
  hardware_version_=0;
  mpc_tmb_mask_=0;
  //
  read_firmware_day_   = 9999;
  read_firmware_month_ = 9999;
  read_firmware_year_  = 9999;
  //
  expected_firmware_day_   = 999;
  expected_firmware_month_ = 999;
  expected_firmware_year_  = 999;
  //
  MyOutput_ = &std::cout ;
}


MPC::~MPC(){
  (*MyOutput_) << "MPC: module removed from crate=" << this->crate() 
       << " slot=" << this->slot() << std::endl;
}


void MPC::init() {
}

bool MPC::SelfTest() {
  //
  return 0;
  //
}

std::ostream & operator<<(std::ostream & os, MPC & mpc) {
  os << std::dec << "TLK2501TxMode_ " << mpc.TLK2501TxMode_ << std::endl
     << "TransparentModeSources_ " << mpc.TransparentModeSources_ << std::endl
     << "TMBDelayPattern_ " << mpc.TMBDelayPattern_ << std::endl
     << "BoardID_ " << mpc.BoardId_ << std::endl;
  return os;
    }

void MPC::configure() {
  char data[2];
  char addr;
  
  (*MyOutput_) << "MPC: initialize" << std::endl;

  SendOutput("MPC : configure()","INFO");

  ReadRegister(CSR0);

  addr = CSR0;
  data[1]=0x00|((BoardId_&0xf)<<1);
  data[0]=0x42|((BoardId_&0x30)>>2);
  do_vme(VME_WRITE, addr, data, NULL, NOW);

  ReadRegister(CSR0);

  // set GTP Control Register to enable TX of new links
  WriteRegister(CSR5, 0x30);
  
  (*MyOutput_) << "MPC: set default serializer TX mode ..." << std::endl;
  setTLK2501TxMode(TLK2501TxMode_);

  // Sorter Mode is the default power-up mode of the MPC.
  (*MyOutput_) << "MPC: set default MPC operation Mode ..." << std::endl;
  if (TransparentModeSources_)
    setTransparentMode(TransparentModeSources_);
  else
    setSorterMode();

  WriteMask(mpc_tmb_mask_);
  // report firmware version
  firmwareVersion();
  printFirmwareVersion();
}

int MPC::CheckConfig()
{
   std::cout << "MPC: configuration check..." << std::endl;
   int rx;
   rx=ReadRegister(CSR0);
   //
   int read_value = rx & 0x8201;
   int expected_value = 0x0200;
   //
   bool config_ok = true;
   //
   config_ok &= compareValues("MPC Register CSR0",read_value,expected_value); 

   // check Board ID ( same as Crate ID)
   int read_boardid = ((rx>>1)&0xF) | ((rx>>6)&0x30);
   config_ok &= compareValues("MPC Board ID", read_boardid, BoardId_);
   //
   //   if((rx & 0x8201) != 0x0200) 
   //   {  std::cout << "MPC_Check_Config: Register CSR0 wrong " 
   //                << std::hex << (rx&0xffff) << std::dec << std::endl;
   //      return 0;
   //   }

   // check TMB mask
   int tmb_mask = ReadMask();
   config_ok &= compareValues("MPC TMB Mask", tmb_mask & 0x1FF, mpc_tmb_mask_ & 0x1FF);

   return (int) config_ok;
}

void MPC::read_fifo(unsigned address, char * data) {
  // each item in the FIFO occupies two words
  read_later(address); 
  read_now(address, data);
}

void MPC::read_fifosA() {
  //
  char data[100];
  //
  std::cout.fill('0');
  //
  read_now(FIFO_A1a, data);  
  (*MyOutput_) << "MPC: FIFO-A1a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A1b, data);
  (*MyOutput_) << "MPC: FIFO-A1b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A2a, data);
  (*MyOutput_) << "MPC: FIFO-A2a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A2b, data);
  (*MyOutput_) << "MPC: FIFO-A2b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A3a, data);
  (*MyOutput_) << "MPC: FIFO-A3a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A3b, data);
  (*MyOutput_) << "MPC: FIFO-A3b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A4a, data);
  (*MyOutput_) << "MPC: FIFO-A4a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A4b, data);
  (*MyOutput_) << "MPC: FIFO-A4b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A5a, data);
  (*MyOutput_) << "MPC: FIFO-A5a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A5b, data);
  (*MyOutput_) << "MPC: FIFO-A5b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A6a, data);
  (*MyOutput_) << "MPC: FIFO-A6a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A6b, data);
  (*MyOutput_) << "MPC: FIFO-A6b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A7a, data);
  (*MyOutput_) << "MPC: FIFO-A7a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A7b, data);
  (*MyOutput_) << "MPC: FIFO-A7b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A8a, data);
  (*MyOutput_) << "MPC: FIFO-A8a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A8b, data);
  (*MyOutput_) << "MPC: FIFO-A8b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A9a, data);
  (*MyOutput_) << "MPC: FIFO-A9a = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
  read_now(FIFO_A9b, data);
  (*MyOutput_) << "MPC: FIFO-A9b = 0x" << std::setw(2) << (data[0]&0x00ff) << std::setw(2) << (data[1]&0x00ff) << std::endl;
  //
}

void MPC::read_fifos() {
  //
  // Read FIFO-B until empty
  //
  (*MyOutput_) << "MPC:  Read FIFO-B" << std::endl;
  char data[100];
  //read_fifo(STATUS, data);
  read_one(CSR3, data);
  std::cout.fill('0');
  (*MyOutput_) << "begin FIFO status " << std::hex << std::setw(2) << (data[1]&0xFF) << std::setw(2)<< (data[0]&0xFF) << std::endl;
  std::cout.fill(' ');
  //bool full_fifoa=(data[1]&0x0001);
  //bool empty_fifoa=(data[1]&0x0002)>>1;
  //bool full_fifob=(data[1]&0x0004)>>2;
  bool empty_fifob=(data[0]&0x0008)>>3;
  //
  unsigned long Lct0,Lct1, Lct2;
  // Lct0=0;Lct1=0;Lct2=0;
  int items=0;
  //
  if(empty_fifob) 
  {
    (*MyOutput_) << "MPC: FIFO-B is empty!" << std::endl;
    return;
  } 
  while(items<=256 && !empty_fifob)
  {
    (*MyOutput_) << "MPC: 1st Best Muon FIFO" << std::endl;
    read_fifo(FIFO_B1, data);
    Lct0 = ((data[1]&0x00ff) << 8) | (data[0]&0x00ff) ;
    Lct0 = (Lct0<<16) | ((data[3]&0x00ff) << 8) | (data[2]&0x00ff) ;
    FIFOBLct0.push_back(Lct0);
    (*MyOutput_) << std::hex;
    std::cout.fill('0');
    (*MyOutput_) << "MPC: FIFO-B1a = 0x" << std::setw(2) << (data[1]&0x00ff) << std::setw(2) << (data[0]&0x00ff) << std::endl;
    (*MyOutput_) << "MPC: FIFO-B1b = 0x" << std::setw(2) << (data[3]&0x00ff) << std::setw(2) << (data[2]&0x00ff)  << std::endl;
    (*MyOutput_) << "MPC: LCT0     = 0x" << std::setw(8) << Lct0 << std::endl ;
    //
    (*MyOutput_) << "MPC: 2nd Best Muon FIFO" << std::endl;
    read_fifo(FIFO_B2, data);
    Lct1 = ((data[1]&0x00ff) << 8) | (data[0]&0x00ff) ;
    Lct1 = (Lct1<<16) | ((data[3]&0x00ff) << 8) | (data[2]&0x00ff) ;
    FIFOBLct1.push_back(Lct1);
    (*MyOutput_) << "MPC: FIFO-B2a = 0x" << std::setw(2) << (data[1]&0x00ff) << std::setw(2) << (data[0]&0x00ff) << std::endl;
    (*MyOutput_) << "MPC: FIFO-B2b = 0x" << std::setw(2) << (data[3]&0x00ff) << std::setw(2) << (data[2]&0x00ff) << std::endl;
    (*MyOutput_) << "MPC: LCT1     = 0x" << std::setw(8) << Lct1 << std::endl ;
    //
    (*MyOutput_) << "MPC: 3nd Best Muon FIFO" << std::endl;
    read_fifo(FIFO_B3, data);
    Lct2 = ((data[1]&0x00ff) << 8) | (data[0]&0x00ff) ;
    Lct2 = (Lct2<<16) | ((data[3]&0x00ff) << 8) | (data[2]&0x00ff) ;
    FIFOBLct2.push_back(Lct2);
    (*MyOutput_) << "MPC: FIFO-B3a = 0x" << std::setw(2) << (data[1]&0x00ff) << std::setw(2) << (data[0]&0x00ff) << std::endl;
    (*MyOutput_) << "MPC: FIFO-B3b = 0x" << std::setw(2) << (data[3]&0x00ff) << std::setw(2) << (data[2]&0x00ff) << std::endl;
    (*MyOutput_) << "MPC: LCT2     = 0x" << std::setw(8) << Lct2 << std::endl ;
    std::cout.fill(' ');
    (*MyOutput_) << std::dec;    
    //
    read_one(CSR3, data);
    empty_fifob=(data[0]&0x0008)>>3;
    items++;
  }
  //
  read_one(CSR3, data);
  std::cout.fill('0');
  (*MyOutput_) << "end FIFO status " << std::hex << std::setw(2) << (data[1]&0xFF) << std::setw(2)<< (data[0]&0xFF) << std::endl;
  std::cout.fill(' ');
  //
}

void MPC::read_csr0() {
  char data[100];
  read_one(CSR0,data);
  (*MyOutput_).fill('0');
  (*MyOutput_) << "MPC: data read from CSR0: 0x" << std::hex 
       << std::setw(2) << (data[1]&0x00ff) << std::setw(2) << (data[0]&0x00ff)
       << std::dec << std::endl;
  (*MyOutput_).fill(' ');
}

void MPC::SoftReset() {
  //
  char data[2]={0, 1};
  //
  char addr =  0x4;
  //
  // reset FPGA logic----write to address 0x4 for firmware 2006 and newer
  //
  do_vme(VME_WRITE, addr, data, NULL, NOW);
  udelay(500000);
}

void MPC::read_status() {
  //Check FIFO Status:
  char data[100];
  //read_fifo(STATUS, data);
  read_now(CSR3, data);
  std::cout.fill('0');
  (*MyOutput_) << "MPC: FIFO status = 0x" << std::hex << std::setw(2) << (data[1]&0x00ff)
       << std::setw(2) << (data[0]&0x00ff) << std::dec << std::endl;
  std::cout.fill(' ');
  bool full_fifoa=(data[0]&0x0001);
  bool empty_fifoa=(data[0]&0x0002)>>1;
  bool full_fifob=(data[0]&0x0004)>>2;
  bool empty_fifob=(data[0]&0x0008)>>3;

  if (full_fifoa>0)  (*MyOutput_) << "MPC: FIFO_A is Full"  << std::endl;
  if (empty_fifoa>0) (*MyOutput_) << "MPC: FIFO_A is Empty" << std::endl;
  if (full_fifob>0)  (*MyOutput_) << "MPC: FIFO_B is Full"  << std::endl;
  if (empty_fifob>0) (*MyOutput_) << "MPC: FIFO_B is Empty" << std::endl;;
}


void MPC::executeCommand(std::string command) {
  if(command=="Init")   init();
  if(command=="Read FIFOs") read_fifos();
  if(command=="Read CSR0") read_csr0();
  //if(command=="Read Date") read_date();
  if(command=="Read Date") firmwareVersion();
}

void MPC::enablePRBS(){
  //
  char data[2];
  char addr;

  addr = CSR0;
  do_vme(VME_READ, addr, NULL, data, NOW);

  // set both bits 14 and 15 to "1", bit 14 was probably on already
  data[0] |= 0xC0;
  do_vme(VME_WRITE, addr, data, NULL, NOW);

  (*MyOutput_) << "MPC: PRBS mode enabled" << std::endl;
}

void MPC::disablePRBS(){
  //
  char data[2];
  char addr;

  addr = CSR0;
  do_vme(VME_READ, addr, NULL, data, NOW);

  // set bit 15 to "0", leave bit 14 as "1"
  data[0] &= 0x7F;
  do_vme(VME_WRITE, addr, data, NULL, NOW);

  (*MyOutput_) << "MPC: PRBS mode disabled" << std::endl;
}

int MPC::ReadMask()
{
  // return mask:  bit 0 ---- bit 8
  //               TMB9 ....  TMB1
  // bit=1 if both LCT0 & LCT1 are disabled 
  // bit=0 if either LCT is enabled

  int data, mask=0;
  char *temp=(char *) &data;
  if(hardware_version_<=1)
  {
     read_one(CSR7, temp);
     read_one(CSR8, temp+2);

     mask=0;
     for(int i=0; i<9; i++)
     {
        mask = mask << 1;
        if((data&3)==3) mask |= 1;
        data = data >> 2;
     }
  }
  else if(hardware_version_==2)
  {
     read_one(CSR7, temp);
     mask=0;
     for(int i=0; i<9; i++)
     {
        mask = mask << 1;
        if((data&1)==0) mask |= 1;
        data = data >> 1;
     }
  }

//  (*MyOutput_) << "read MPC mask :" << std::hex << mask << std::dec << std::endl;
  return mask;
}

void MPC::WriteMask(int mask)
{
  // TMB mask value:  bit 0 ---- bit 8
  //                  TMB9 ....  TMB1
  // bit=1 if TMB (both LCTs) is disabled 
  // bit=0 if TMB (either LCT) is enabled
  unsigned short raw;
  unsigned data;

  (*MyOutput_) << "write MPC mask : " << std::hex << mask << std::dec << std::endl;

  if(hardware_version_<=1)
  {
     data=0;
     for(int i=0; i<9; i++)
     {
        data = data << 2;
        if(mask&1) data |= 3;
        mask = mask >> 1;
     }
     raw = data&0xFFFF;
     write_later(CSR7, raw);
     raw = (data>>16)&3;
     write_now(CSR8, raw, rcvbuf);
  }
  else if(hardware_version_==2)
  {
     raw=0;
     for(int i=0; i<9; i++)
     {
        raw = raw << 1;
        if((mask&1)==0) raw |= 1;
        mask = mask >> 1;
     }
     write_now(CSRM, raw, rcvbuf);
  }
}

void MPC::injectSP(){

  //
  char data[2];
  char addr;

  int ITR=255;

 this->read_status();
 
   (*MyOutput_) << "..now filling FIFO-A" <<std::endl;
	   // Fill FIFOA with 255 events
	   for (int EVNT=0; EVNT<255; EVNT++)
           {
             unsigned long mframe1, mframe2;

//             mframe1 = (0x1 << 15) | (0xF << 11) | EVNT;
 	mframe1 =  (0xF << 12) | EVNT;
//	     ITR--;
	ITR=3;
	     mframe2 = (0xF << 12) | ITR;

	     // Load 9x2 LCTs to MPC Input "FIFOA"
	     //9 buffers in FIFO-A corresponding to 9 TMBs
	     addr=FIFO_A1a;
	     for (int fifo_a=1; fifo_a<=9; fifo_a++) 
             {
	       for (int LCT=0; LCT<2; LCT++)  //dammit, no i think this should just be a loop over frames!!
//2 sorts of LCTs
	         {
		     data[1]= mframe1&0x00FF;
		     data[0]=(mframe1&0xFF00)>>8;
		     this->do_vme(VME_WRITE, addr, data, NULL, NOW);

//2 frames?:ie 2x16 bits =tot 32 bits..

		     data[1]= mframe2&0x00FF;
		     data[0]=(mframe2&0xFF00)>>8;
		     this->do_vme(VME_WRITE, addr, data, NULL, NOW);

		     addr+=0x00002;

	         }
	     } 
	   }
        

	// Append 0's to the last one..
	addr=FIFO_A1a;
	for (int fifo_a=1; fifo_a<=9; fifo_a++) {
	    for (int LCT=0; LCT<2; LCT++) {

		     data[0]=0;
		     data[1]=0;
		     this->do_vme(VME_WRITE, addr, data, NULL, NOW);
		     data[0]=0;
		     data[1]=0;
		     this->do_vme(VME_WRITE, addr, data, NULL, NOW);
		     addr+=0x00002;
	    }
    }
 
this->read_status();

}


int MPC::injectSP(char *injectDataFileName){


// member function as above but here you have passed a file 
// containing the LCTs you want to inject.
// data file should have a set of 16 bit (4 hex character) words 
  
  
  //
  char data[2];
  char DataWord[4];//4 character hex string=16 bit word 
  int dataWordInt_fr1;// the above string "DataWord" converted -->int.::frame 1s
  int dataWordInt_fr2;// the above string "DataWord" converted -->int.::frame 2s
  unsigned long mframe1, mframe2;
  char addr;
  int readWord;//number of words read by fscanf
  
  (*MyOutput_)<<"...inject test pattern funct"<<std::endl;
  (*MyOutput_)<<"data file passed is: "<<injectDataFileName<<std::endl;
  //let's try to open the file:: 
  
  
  
  (*MyOutput_)<<"opening file..."<<std::endl;
  FILE* myFile = fopen(injectDataFileName , "r" );
  if (myFile==NULL){
    (*MyOutput_)<<"problem opening data file, exiting.."<<std::endl;
    return -1;
  }
  
  
  
  this->read_status();
  
  (*MyOutput_)<<" will use LCT data from file "<<injectDataFileName<<std::endl;
  
  (*MyOutput_) << "..now filling FIFO-A, with your data, alternating frame 1, 2.." <<std::endl;
  //   int ITR=2;mframe2 = ITR;
  


  // read the first data word and turn it into and integer:
  readWord=fscanf( myFile,"%s",DataWord);
  sscanf(DataWord,"%x",&dataWordInt_fr1);
  (*MyOutput_)<<"first frame 1 :: (first word in file) is "<<std::hex<<dataWordInt_fr1<<std::endl;
  if (readWord<1){
    (*MyOutput_)<<"problem reading first word in file ..exiting..."<<std::endl;
    return -1;
  }
  readWord=fscanf( myFile,"%s",DataWord);
  sscanf(DataWord,"%x",&dataWordInt_fr2);
  (*MyOutput_)<<"first frame 2 :: (second word in file) is "<<std::hex<<dataWordInt_fr2<<std::endl;    

  int EVNT=0;//event counter
  while (dataWordInt_fr1 !=0 && dataWordInt_fr2 !=0)//keep on looping while we have valid words
    {
      
      EVNT++;
      //             mframe1 = (0x1 << 15) | (0xF << 11) | dataWordInt_fr1;
      mframe1 =  dataWordInt_fr1;
      mframe2 =  dataWordInt_fr2;
      
      // Load 2 LCTs to each of 8/9!! buffers in FIFO-A corresponding to 9TMBs
      addr=FIFO_A1a;
      for (int fifo_a=1; fifo_a<=9; fifo_a++) //hmm why do we stop at 8??  ..9TMBs and FIFO-A9..?
	{
	  	  
	  for (int LCT=0; LCT<2; LCT++)
	    //2 sorts of LCTs
	    {
	      data[1]= mframe1&0x00FF;
	      data[0]=(mframe1&0xFF00)>>8;
	      this->do_vme(VME_WRITE, addr, data, NULL, NOW);
	      
	      //(*MyOutput_)<<"event: "<<EVNT<<"...filling fifoA with "<<std::hex<<mframe1<<std::endl;
	      
	      //2 frames?:ie 2x16 bits =tot 32 bits..
	      
	      data[0]= mframe2&0x00FF;
	      data[1]=(mframe2&0xFF00)>>8;
	      this->do_vme(VME_WRITE, addr, data, NULL, NOW);
	      
	      addr+=0x00002;
	      
	    }//end for loop, 2LCT
	}
      
      //try to read next 2 data words from file:
      fscanf(myFile,"%s",DataWord);
      sscanf(DataWord,"%x",&dataWordInt_fr1);
      readWord= fscanf(myFile,"%s",DataWord);
      sscanf(DataWord,"%x",&dataWordInt_fr2);
      
      if (readWord<1){
	(*MyOutput_)<<"reached end of file... .."<<EVNT<<" eventsx2 frames loaded.."<<std::endl;
	dataWordInt_fr1=0;
	dataWordInt_fr2=0;
      }//end if	     
      
    }//end while we have valid words
  
  
  // Append 0's to the last one..
  addr=FIFO_A1a;
  for (int fifo_a=1; fifo_a<=9; fifo_a++) {
    for (int LCT=0; LCT<2; LCT++) {
      
      data[0]=0;
      data[1]=0;
      this->do_vme(VME_WRITE, addr, data, NULL, NOW);
      data[0]=0;
      data[1]=0;
      this->do_vme(VME_WRITE, addr, data, NULL, NOW);
      addr+=0x00002;
    }
  }
  
  this->read_status();
  return 0;
}


void MPC::setTLK2501TxMode(int mode){
  /// set the TLK2501 serializer Tx mode
  if (mode==1)
    (*MyOutput_) << "MPC: serializer in FRAMED mode" << std::endl;
  else if (mode==0)
    (*MyOutput_) << "MPC: serializer in CONTINUOUS mode" << std::endl;
  else
    std::cerr << "MPC: -WARNING- serializer in UNKNOWN mode ("<<mode<<")"<< std::endl;

  //Note: CSR2 has only bit-0 as a relevant bit. As soon as there are more
  //      bits this should be specific to that bit using a mask.

  char addr =  CSR2; //addr=0x000ac;
  //int btd;
  //int xfer_done[2];
  char data[2];
  //fg was this really the right way ... or, maybe, 
  //fg it should have been the other way around -- oh-oh.
  //fg anyways, nobody but Lev uses it ...
  //fg data[0]=mode;
  //fg data[1]=0x00;
  data[0]=0x00;
  data[1]=mode;
  do_vme(VME_WRITE, addr, data, NULL, NOW);
}


void MPC::WriteRegister(int reg, int value){
  //
  sndbuf[0] = (value>>8)&0xff;
  sndbuf[1] = value&0xff;
  //
  do_vme(VME_WRITE,reg,sndbuf,rcvbuf,NOW);
  //
}

int MPC::ReadRegister(int reg){
  //
  // make sure we are in framed mode
  do_vme(VME_READ,reg,sndbuf,rcvbuf,NOW);
  //
  int value = ((rcvbuf[0]&0xff)<<8)|(rcvbuf[1]&0xff);
  //
  // printf(" MPC.reg=%x %x %x %x\n", reg, rcvbuf[0]&0xff, rcvbuf[1]&0xff,value&0xffff);
  //
  return value;
  //
}
//
bool MPC::CheckFirmwareDate() {
  //
  bool check_ok;
  //
  // read the firmware version:
  firmwareVersion();
  //
  if ( read_firmware_day_   == GetExpectedFirmwareDay()   &&
       read_firmware_month_ == GetExpectedFirmwareMonth() &&
       read_firmware_year_  == GetExpectedFirmwareYear()  ) {
    check_ok = true;
  } else {
    check_ok = false;
  }
  //
  return check_ok;
}
//
void MPC::printFirmwareVersion() {
  //
  (*MyOutput_) << "MPC: firmware version (day-month-year): (" 
	       << std::dec << read_firmware_day_ 
	       << "-"      << read_firmware_month_
	       << "-"      << read_firmware_year_
	       << ")"      << std::endl;

  return;
}
//
void MPC::firmwareVersion(){
  /// report the firmware version
  //
  char data[2];
  do_vme(VME_READ,CSR1,NULL, data, NOW);
  
  int versionWord = (data[0]<<8) + (data[1]&0xFF);
  //  std::cout << std::hex << versionWord << std::endl;
  read_firmware_day_   =  versionWord & 0x1F;
  read_firmware_month_ = (versionWord >> 5   ) & 0xF;
  read_firmware_year_  = (versionWord >>(5+4)) + 2000;
  //
  // printFirmwareVersion();
  return;
}


void MPC::setSorterMode(){
  /// Switches the MPC to Sorter Mode while keeping the original sources intact.
  (*MyOutput_) << "MPC: switching to Sorter Mode" << std::endl;
  char addr =  CSR4;
  //
  //
  char data[2];
  do_vme(VME_READ,addr, NULL,data, NOW);  
  //fg data[0]=data[0]&0xfe;
  //fg data[1]=data[1];
  //fg andersom, slimpy.
  data[0]=data[0];
  data[1]=data[1]&0xfe;
  do_vme(VME_WRITE, addr, data, NULL, NOW);
}


void MPC::setTransparentMode(unsigned int pattern){
  /// Sets the Transparent Mode according to the source pattern.

  //fg At this point the routine expects the full CSR4 pattern.
  //fg This should change to the source pattern only followed by a
  //fg bit shift adding the bit-1=1 ... Should change once we feel
  //fg comfortable enough about it.

  (*MyOutput_) << "MPC: switching to Transparent Mode. Source pattern = 0x" 
       << std::hex << pattern << std::dec << std::endl;
  //
  char data[2];
  char addr=CSR4;

  do_vme(VME_READ, addr, NULL, data, NOW);

  // make sure that the last bit is actually 1, otherwise there is no transparentMode
  if ( !(pattern & 0x01)){
    (*MyOutput_) << "MPC: WARNING - last bit in source pattern (" << (unsigned short)pattern 
	 << ") assumes Sorter Mode. Using Transparent instead" << std::endl;
    pattern |=0x01;
  }

  // upload the pattern
  //
  data[0]=(pattern>>8)&0xff;   // MSB
  data[1]=pattern&0xff; // LSB
  do_vme(VME_WRITE, addr, data, NULL, NOW);
  //
  (*MyOutput_) << "Reading back..." << std::endl;
  //
  ReadRegister(CSR1);
  ReadRegister(CSR4);
}


void MPC::setTransparentMode(){
  /// Switches to Transparent Mode using whatever orginal source pattern
  /// was previously loaded.
  (*MyOutput_) << "MPC: switching to Transparent Mode. No new source pattern loaded" << std::endl;
  //
  //
  char data[2];
  char addr = CSR4; 
  do_vme(VME_READ, addr, NULL, data, NOW);

  //fg data[0] |= 0x01;
  //fg data[1] = 0;
  data[0] =0;
  data[1] |= 0x01;
  do_vme(VME_WRITE, addr, data, NULL, NOW);
}

void MPC::setDelay(int delay)
{
      int value = ReadRegister(CSR2);
      udelay(10000);
      value = (value & 0x00ff) | ((delay<<8)&0xff00);
      WriteRegister(CSR2,value);
      udelay(50000);
}

void MPC::start() {
#ifdef debugV
  (*MyOutput_) << "starting to talk to TMB, device " << ucla_ldev << std::endl;
#endif
  // send the first signal
  SetupJtag();
  VMEModule::start();
  theController->initDevice(1);
  theController->goToScanLevel();
}

int MPC::readDSN(void *data)
{
   char *dsn=(char *)data;
   int iloop=0, error, i, j, dsnstate, rd, c;

   WriteRegister(DSNclear,0);
   udelay(100);
   WriteRegister(DSNreset,0);
   udelay(2000);
   dsnstate=ReadRegister(CSR6);
   while((dsnstate&5)!=4 && iloop<5)
   {
       udelay(1000*iloop);
       dsnstate=ReadRegister(CSR6);
       iloop++;
   }
   if((dsnstate&5)!=4) return -1;
   int readcmd=0xF;   
   for(i=0; i<8; i++)
   {
      // WriteRegister(DSNclear,0);
      if(readcmd&1) WriteRegister(DSNwrite1, 0);
      else WriteRegister(DSNwrite0, 0);
      readcmd >>= 1;
      udelay(60);
   }
   error=0;
   for(i=0; i<8; i++)
   {
      rd=0;
      for(j=0; j<8; j++)
      {
         WriteRegister(DSNread, 0);
         udelay(10);
         c=ReadRegister(CSR6);
         if(c&8)
            rd |= (((c>>1)&1)<<j);        
         else
            error++;
         udelay(5);
      }
      dsn[i]=rd;
   }      
   if(error) std::cout << "Errors in reading DSN: " << error << std::endl;
   return error;
}

int MPC::newPRBS(int mode)
{
//  mode 0: standard operation mode, non-PRBS 
//       1: PRBS-7
//       2: PRBS-15
//       3: PRBS-23
//       4: PRBS-31
    if(hardware_version_==2)
    {
       int v=ReadRegister(CSR4);
       // if mode==-1 (empty), return current setting
       if(mode<0)
          return (v&0xF)/2; 
       else
       {
          WriteRegister(CSR5, 0x30); // normal mode
          int prbsmode=((mode&7)<<4 | (mode&7)<<1);
          WriteRegister(CSR4, (v&0xFF00)|prbsmode);
          return mode&7;
       }
    }
    else 
     return -1;
}

void MPC::Fill_FIFO_A()
{
/* Write random patterns to MPC's 18 FIFO_As */
   int ireg, pid, quality;

   for(int i=0; i<18; i++)
   {
        ireg=FIFO_A1a+i*2;
        for(int k=0; k<255; k++)
        {
           pid= (int) (65536.0 * (rand() / (RAND_MAX + 1.0)));
           while(pid==0)
           {
               pid= (int) (65536.0 * (rand() / (RAND_MAX + 1.0)));
           }
           // first frame
           quality = (pid>>11)&0xF;
           if (quality==0) quality=1;
           pid &= 0x7FF;
           pid |= (quality<<11);
           pid |= 0x8000;
           WriteRegister(ireg,   pid);
//           usleep(10);

           pid= (int) (65536.0 * (rand() / (RAND_MAX + 1.0)));
           while(pid==0)
           {
               pid= (int) (65536.0 * (rand() / (RAND_MAX + 1.0)));
           }
           // second frame
           pid &= 0xF5FF;
           pid |= 0x0800; // BC0=1
           WriteRegister(ireg,   pid);
//           usleep(10);
        }
        pid=0;
        WriteRegister(ireg,   pid);
//           usleep(10);
        pid=ReadRegister(CSR0);
        WriteRegister(CSR0, pid|1); // put MPC in test mode (use data from FIFO_A)
   }

}

int MPC::Read_FIFO_B_New(int link, unsigned short *data)
{
// data[] must be large enough to hold 511 words
   if(link<1 || link>8) return -1;
   int ireg=FIFO_B1_New + (link-1)*2;
   for(int i=0; i<511; i++)
   {
      data[i]=(unsigned short)ReadRegister(ireg);
   }
   return 511;
}

int MPC::Read_FIFO_B_Old(int link, unsigned short *data)
{
// data[] must be large enough to hold 511 words
   if(link<1 || link>3) return -1;
   int ireg=FIFO_B1 + (link-1)*2;
   for(int i=0; i<511; i++)
   {
      data[i]=(unsigned short)ReadRegister(ireg);
   }
   return 511;
}

int MPC::read_oldPRBS()
{
   int i=ReadRegister(CSR0);
   return i>>15;
}

int MPC::read_newPRBS()
{
   int i=ReadRegister(CSR4);
   return (i&0x70)>>4;
}

void MPC::inject_PRBSerror()
{
   int i=ReadRegister(CSR4);
   WriteRegister(CSR4, i|0x8000);
   ::usleep(10);
   WriteRegister(CSR4, i);
}

void MPC::resetGTP(int signal)
{
   WriteRegister(CSR11, signal&0xFFF);
   std::cout << "MPC reset GTP with signal(s): " << std::hex << signal << std::dec << std::endl;
   ::usleep(10000);
}

void MPC::mpc_scan(int reg, char *snd,int cnt,char *rcv,int ird, int chip)
{
   // same interface as regular scan() but with chip seletion
   // chip=0   FPGA   
   //      1   PROM 1 
   //      2   PROM 2 
           
   if(chip<0 || chip>2) return;
   int TIR[3]={0, 6, 22}, HIR[3]={32,16,0}, HDR[3]={2,1,0}, TDR[3]={0,1,2};
   unsigned long TDI=5, TMS=6, TCK=7, TDO=8; 
   unsigned long regV=0x6200;
   unsigned long handle=(TDI)+(TMS<<4)+(TCK<<8)+(TDO<<12) + (regV<<16);
   char buff[4200];
   int ncnt=cnt;
   if(cnt>0) memcpy(buff, snd, (cnt+7)/8);
   if(reg==1)
   {
      add_headtail(buff, cnt, TDR[chip], HDR[chip]);
      ncnt += HDR[chip]+TDR[chip];
   }
   else if (reg==0 && cnt>0) 
   { 
      add_headtail(buff, cnt, TIR[chip], HIR[chip]); 
      ncnt += HIR[chip]+TIR[chip];
   }
   Jtag_Norm(handle, reg, buff, ncnt, rcv, ird, NOW);
   if(reg==1) cut_headtail(rcv, ncnt, TDR[chip], HDR[chip]);
}

void MPC::jtag_RestoreIdle()
{
   int tmp=0;
   mpc_scan(0, (char *)&tmp, -1, (char *)&tmp, 0, 0);
}

int MPC::erase_eprom(int chip, int broadcast)
{

    unsigned comd=0, data=0;
    int blank_state=0;
    std::cout << "Erasing EPROM #" << chip << "......" << std::endl;
    if(chip<1 || chip>2) return -1;
    getTheController()->SetUseDelay(true);

       jtag_RestoreIdle();      
       comd=XCF_ISC_ENABLE; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       data=0x03;
       mpc_scan(1, (char *)&data, 8, rcvbuf, 0, chip);
       comd=XCF_XSC_UNLOCK; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       data=0x3F;
       mpc_scan(1, (char *)&data, 24, rcvbuf, 0, chip);
       set_flag(0);
       comd=XCF_ISC_ERASE; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       data=0x3F;
       mpc_scan(1, (char *)&data, 24, rcvbuf, 0, chip);
       clear_flag(0);
       ::sleep(140);
    if(broadcast==0)
    {
       comd=XCF_CLR_STATUS; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       udelay(50);
       comd=XCF_BLANK_CHECK; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       ::sleep(1);
       data=0;
       mpc_scan(1, (char *)&data, 8, rcvbuf, 1, chip);
       blank_state = rcvbuf[0]&0x3F;
       if(blank_state==0) std::cout << "Blank Check successful!" << std::endl;
       comd=XCF_CLR_STATUS; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       udelay(50);
              
    }
       comd=XCF_ISC_DISABLE; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       udelay(200);
    std::cout << "Done." << std::endl;
    return blank_state;
}

int MPC::program_eprom(const char *mcsfile, int chip, int broadcast)
{
    if(chip<1 || chip>2) return -1;
    std::cout << "Programming EPROM #" << chip << "......" << std::endl;
   unsigned comd, data;
   const int PROM_SIZE=4194304; // in bytes

   char *bufin, c;
   bufin=(char *)malloc(16*1024*1024);
   if(bufin==NULL)  return -2;
   FILE *fin=fopen(mcsfile,"r");
   if(fin==NULL ) 
   { 
      free(bufin);  
      std::cout << "ERROR: Unable to open MCS file :" << mcsfile << std::endl;
      return -3; 
   }
   int mcssize=read_mcs(bufin, fin);
   fclose(fin);
   std::cout << "Read MCS size: " << std::dec << mcssize << " bytes" << std::endl;
   if(chip==1 && mcssize<PROM_SIZE)
   {
       std::cout << "ERROR: Wrong MCS file. Quit..." << std::endl;
       free(bufin);
       return -4;
   }
/*
// byte swap
   for(int i=0; i<FIRMWARE_SIZE/2; i++)
   {  c=bufin[i*2];
      bufin[i*2]=bufin[i*2+1];
      bufin[i*2+1]=c;
   }
*/
     int blocks=mcssize/32;  // firmware size must be in units of 256-bit units
     if (mcssize%32)
     {  
         for(int i=0; i<32; i++) bufin[mcssize+i]=0xFF;  // pad the last block with 0xFF
         blocks++;
     }
     int p1pct=blocks/100;
     int j=0, pcnts=0;

//    getTheController()->Debug(2);
     getTheController()->SetUseDelay(true);
  
     jtag_RestoreIdle();      
     comd=XCF_ISC_ENABLE; 
     mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
     data=0x03;
     mpc_scan(1, (char *)&data, 8, rcvbuf, 0, chip);
     comd=XCF_XSC_UNLOCK; 
     mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
     data=0x3F;
     mpc_scan(1, (char *)&data, 24, rcvbuf, 0, chip);
     comd=XCF_DATA_BTC; 
     mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
     data=0xFFFFFFEC;
     mpc_scan(1, (char *)&data, 32, rcvbuf, 0, chip);
     comd=XCF_ISC_PROGRAM; 
     mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
     udelay(200);
     for(int i=0; i<blocks; i++)
     {
        if((i%0x8000)==0)   
        {  /* At beginning of each big block, send (byte) address. */
           comd=XCF_ADD_SHIFT; 
           mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
           data=i*32;
           mpc_scan(1, (char *)&data, 24, rcvbuf, 0, chip);
        }
       comd=XCF_DATA_SHIFT; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       mpc_scan(1, bufin+32*i, 256, rcvbuf, 0, chip);
       comd=XCF_ISC_PROGRAM; 
       mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
       udelay(1000);
       j++;
       if(p1pct>0 && j==p1pct)
       {  pcnts++;
          if(pcnts<100) std::cout << "Sending " << pcnts <<"%..." << std::endl;
          j=0;
       }   
     }
     comd=XCF_DATA_DONE; 
     mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
     data=0xC0;
     mpc_scan(1, (char *)&data, 8, rcvbuf, 0, chip);
     comd=XCF_ISC_PROGRAM; 
     mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
     udelay(200);
     std::cout << "Sending 100%..." << std::endl;
//    getTheController()->Debug(2);
     if(broadcast==0)
     {
         std::cout << "Verify. " << std::endl;
     }
     comd=XCF_ISC_DISABLE; 
     mpc_scan(0, (char *)&comd, 16, rcvbuf, 0, chip);
     udelay(200);
     free(bufin);
     return 0;
}

  } // namespace emu::pc
} // namespace emu
