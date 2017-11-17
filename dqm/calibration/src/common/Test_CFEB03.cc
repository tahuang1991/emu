#include "emu/dqm/calibration/Test_CFEB03.h"

using namespace XERCES_CPP_NAMESPACE;

using namespace emu::dqm::utils;

Test_CFEB03::Test_CFEB03(std::string dfile): Test_Generic(dfile)
{
  testID = "CFEB03";
  nExpectedEvents = 4000;
  dduID=0;
  binCheckMask=0x1FFB7BF6;
  ltc_bug=2;

  logger = Logger::getInstance(testID);

  for (int i=0; i<64; i++)
    It[i] = conv.elec(i*6.25, 50.);

}


void Test_CFEB03::initCSC(std::string cscID)
{
  nCSCEvents[cscID]=0;
  XtalkData xtalkdata;
  xtalkdata.Nbins = getNumStrips(cscID, theFormatVersion);
  xtalkdata.Nlayers = 6;

  memset(xtalkdata.content, 0, sizeof (xtalkdata.content));

  xdata[cscID] = xtalkdata;

  TestData cscdata;
  TestData2D cfebdata;
  cfebdata.Nbins = getNumStrips(cscID, theFormatVersion);
  cfebdata.Nlayers = 6;
  memset(cfebdata.content, 0, sizeof (cfebdata.content));
  memset(cfebdata.cnts, 0, sizeof (cfebdata.cnts));

  test_step tstep;
  memset(&tstep, 0 , sizeof (tstep));
  tstep.active_strip=0;
  tstep.dac_step=1;
  tstep.evt_cnt=0;
  test_steps[cscID]=tstep;

  htree[dduID][cscID]=tstep;


  // Channels mask
  if (tmasks.find(cscID) != tmasks.end())
    {
      cscdata["_MASK"]=tmasks[cscID];
    }
  else
    {
      cscdata["_MASK"]=cfebdata;
    }

  for (int i=0; i<TEST_DATA2D_NLAYERS; i++)
    for (int j=0; j<TEST_DATA2D_NBINS; j++) cfebdata.content[i][j]=BAD_VALUE;

  // R01 - Pulse Maximum Amplitude
  cscdata["R01"]=cfebdata;

  // R02 - Pulse Peak position in Time
  cscdata["R02"]=cfebdata;

  // R03 - Pulse FHWM
  cscdata["R03"]=cfebdata;

  // R04 - Left Crosstalk
  cscdata["R04"]=cfebdata;

  // R05 - Right Crosstalk
  cscdata["R05"]=cfebdata;

  // R06 - Rleft(t) slope
  cscdata["R06"]=cfebdata;

  // R07 - Rleft(t) intercept
  cscdata["R07"]=cfebdata;

  // R08 - Rleft(t) RMS
  cscdata["R08"]=cfebdata;

  // R09 - Rright(t) slope
  cscdata["R09"]=cfebdata;

  // R10 - Rright(t) intercept
  cscdata["R10"]=cfebdata;

  // R11 - Rright(t) RMS
  cscdata["R11"]=cfebdata;

  tdata[cscID] = cscdata;;

  bookTestsForCSC(cscID);
}

void Test_CFEB03::analyze(const char * data, int32_t dataSize, uint32_t errorStat, int32_t nodeNumber)
{
  nTotalEvents++;

  const uint16_t *tmp = reinterpret_cast<const uint16_t *>(data);
  bin_checker.setMask( binCheckMask);
  if ( bin_checker.check(tmp,dataSize/sizeof(short)) < 0 )
    {
      //   No ddu trailer found - force checker to summarize errors by adding artificial trailer
      const uint16_t dduTrailer[4] = { 0x8000, 0x8000, 0xFFFF, 0x8000 };
      tmp = dduTrailer;
      bin_checker.check(tmp,uint32_t(4));
    }

  if (dduID != (bin_checker.dduSourceID()&0xFF))
    {

      LOG4CPLUS_DEBUG(logger, "DDUEvt#" << std::dec << nTotalEvents << ": DDU#" << (bin_checker.dduSourceID()&0xFF) << " First event");
      dduID = bin_checker.dduSourceID()&0xFF;
      dduL1A[dduID]=0;
      DDUstats[dduID].evt_cntr=0;
      DDUstats[dduID].first_l1a=-1;
      DDUstats[dduID].dac=0;
      DDUstats[dduID].strip=1;
    }

  dduID = bin_checker.dduSourceID()&0xFF;
  DDUstats[dduID].evt_cntr++;

  if ((bin_checker.errors() & binCheckMask)!= 0)
    {
      // std::cout << "Evt#" << std::dec << nTotalEvents << ": Nonzero Binary Errors Status is observed: 0x"<< std::hex << bin_checker.errors() << std::endl;
      doBinCheck();

    }

  CSCDDUEventData dduData((uint16_t *) data, &bin_checker);

  currL1A=(int)(dduData.header().lvl1num());
  if (DDUstats[dduID].evt_cntr ==1)
    {
      DDUstats[dduID].first_l1a = currL1A;
      LOG4CPLUS_DEBUG(logger, "DDUEvt#" << std::dec << nTotalEvents << ": DDU#" << dduID
                      << " First L1A:" << DDUstats[dduID].first_l1a);
    }
  else if (DDUstats[dduID].first_l1a==-1)
    {
      DDUstats[dduID].first_l1a = currL1A-DDUstats[dduID].evt_cntr+1;
      LOG4CPLUS_DEBUG(logger, "DDUEvt#" << std::dec << nTotalEvents << ": DDU#" << dduID
                      << " First L1A :" << DDUstats[dduID].first_l1a << " after "
                      << currL1A-DDUstats[dduID].evt_cntr << " bad events");
    }

  DDUstats[dduID].l1a_cntr=currL1A;

  if ((DDUstats[dduID].l1a_cntr-DDUstats[dduID].first_l1a) != (DDUstats[dduID].evt_cntr-1))
    {
      LOG4CPLUS_WARN(logger, "DDUEvt#" << std::dec << nTotalEvents << ": DDU#" << dduID
                     << " Desynched L1A: " << ((DDUstats[dduID].l1a_cntr-DDUstats[dduID].first_l1a) - (DDUstats[dduID].evt_cntr-1)));
    }

  std::vector<CSCEventData> chamberDatas;
  chamberDatas = dduData.cscData();

  fSwitch=false;

  if (chamberDatas.size() >0)
    {
      DDUstats[dduID].csc_evt_cntr++;
    }
  else
    {
      DDUstats[dduID].empty_evt_cntr++;
    }

  // === set ltc_bug=2 in case of LTC double L1A bug
  // TODO: automatic detection of LTC L1A bug
  //  int ltc_bug=1;
  if (DDUstats[dduID].evt_cntr == 8)
    {
      if  (DDUstats[dduID].empty_evt_cntr==0)
        {
          LOG4CPLUS_INFO(logger, "No LTC/TTC double L1A bug in data");
          ltc_bug=1;
        }
      else
        {
          LOG4CPLUS_WARN(logger, "Found LTC/TTC double L1A bug in data. Expected events x2: " << (nExpectedEvents * ltc_bug) );
        }
    }

  int dacSwitch=25*ltc_bug;
  int stripSwitch=250*ltc_bug;

  if (currL1A % stripSwitch==1)
    {
      DDUstats[dduID].strip=currL1A/ stripSwitch + 1;
    }
  if (currL1A% dacSwitch ==1)
    {
      DDUstats[dduID].dac=(currL1A / dacSwitch) % 10;
      DDUstats[dduID].strip=currL1A / stripSwitch + 1;
      DDUstats[dduID].empty_evt_cntr=0;

      fSwitch=true;
      std::map<std::string, test_step> & cscs = htree[dduID];
      for (std::map<std::string, test_step>::iterator itr = cscs.begin(); itr != cscs.end(); ++itr)
        {
          itr->second.evt_cnt = 0;
        }
    }
  /*
  if (chamberDatas.size())
    std::cout << "DDUEvt#" << std::dec << nTotalEvents << ": DDU#" << dduID <<  " strip: " << DDUstats[dduID].strip << " dac: "  << DDUstats[dduID].dac << std::endl;
  */

  for (std::vector<CSCEventData>::iterator chamberDataItr = chamberDatas.begin();
       chamberDataItr != chamberDatas.end(); ++chamberDataItr)
    {
      analyzeCSC(*chamberDataItr);
    }

  DDUstats[dduID].last_empty=chamberDatas.size();

}


void Test_CFEB03::analyzeCSC(const CSCEventData& data)
{

  const CSCDMBHeader* dmbHeader = data.dmbHeader();
  const CSCDMBTrailer* dmbTrailer = data.dmbTrailer();
  if (!dmbHeader && !dmbTrailer)
    {
      return;
    }

  theFormatVersion = data.getFormatVersion();

  int csctype=0, cscposition=0;
  std::string cscID = getCSCFromMap(dmbHeader->crateID(), dmbHeader->dmbID(), csctype, cscposition);

  // == Do not process unmapped CSCs
  if (cscID == "") return;

  cscTestData::iterator td_itr = tdata.find(cscID);
  if ( (td_itr == tdata.end()) || (tdata.size() == 0) )
    {
      LOG4CPLUS_INFO(logger, "Found " << cscID);
      initCSC(cscID);
      addCSCtoMap(cscID, dmbHeader->crateID(), dmbHeader->dmbID());
    }
  nCSCEvents[cscID]++;

  // == Define aliases to access chamber specific data

  // TestData& cscdata = tdata[cscID];

  //  TestData2D& r04 = cscdata["R04"];

  MonHistos& cschistos = mhistos[cscID];

  test_step& tstep = htree[dduID][cscID];// test_steps[cscID];

  XtalkData& xtalkdata = xdata[cscID];

  int curr_dac = DDUstats[dduID].dac;
  int curr_strip =  DDUstats[dduID].strip;
  if ( (curr_strip>16)
       || (curr_dac > TIME_STEPS)
       || (nCSCEvents[cscID] > nExpectedEvents))
    {
      LOG4CPLUS_WARN(logger, "Evt#" << std::dec << nCSCEvents[cscID] << ": " << cscID << " Went beyond pre-defined events counter for this test");
      return;
    }


  if (fSwitch)
    {
      //  std::cout << "DDUEvt#" << std::dec << nTotalEvents << " " << nCSCEvents[cscID] << " " << cscID << " "
      //               << " ("<< tstep.evt_cnt << ") "<< nCSCBadEvents[cscID] << std::endl;
      //  tstep.evt_cnt=0;
    }
  tstep.evt_cnt++;


  TH2F* v02 = reinterpret_cast<TH2F*>(cschistos["V02"]);
  TH2F* v03 = reinterpret_cast<TH2F*>(cschistos["V03"]);
  TH1F* v04 = reinterpret_cast<TH1F*>(cschistos["V04"]);


  unsigned int l1a_cnt = dmbHeader->l1a();
  if (l1a_cnt < l1a_cntrs[cscID]) l1a_cnt+=256;

  if (v04) v04->Fill(l1a_cnt-l1a_cntrs[cscID]);

  l1a_cntrs[cscID]=dmbHeader->l1a();


  // == Check if CFEB Data Available
  if (dmbHeader->cfebAvailable())
    {

      int nCFEBs = getNumStrips(cscID, theFormatVersion)/16;
      for (int icfeb=0; icfeb<nCFEBs; icfeb++)   // loop over cfebs in a given chamber
        {
          CSCCFEBData * cfebData =  data.cfebData(icfeb);

          // Check if CFEB has data and skip it if it doesn't
          if (!cfebData || !cfebData->check())
            {
              LOG4CPLUS_WARN(logger, "Evt#" << std::dec << nCSCEvents[cscID] << ": " << cscID << " No CFEB" << icfeb+1 << " Data");
              continue;
            }

          bool fFirstStrip = (curr_strip ==1 && icfeb==0)? true: false;
          bool fLastStrip = (curr_strip ==16 && icfeb==(nCFEBs-1))? true: false;
          if (((cscID.find("ME+1.1") == 0) || (cscID.find("ME-1.1") ==0 )) && ((curr_strip ==1 && icfeb==4)))
            fFirstStrip = true;

          for (unsigned int layer = 1; layer <= 6; layer++)   // loop over layers in a given chamber
            {

              int nTimeSamples= cfebData->nTimeSamples(); // Get number of time samples
              //  double Qmax=xtalkdata.content[curr_dac][layer-1][icfeb*16+curr_strip-1][NSAMPLES].max;

              // Do CRC check of first two timesamples for pedestal calculation
              if (!cfebData->timeSlice(0)->checkCRC() || !cfebData->timeSlice(1)->checkCRC())
                {
                  LOG4CPLUS_WARN(logger, cscID << " CRC check failed for central strip time sample 1 and 2");
                  continue;
                }

              // Calculate Central strip Pedestals
              double Q12=((cfebData->timeSlice(0))->timeSample(layer,curr_strip,cfebData->isDCFEB())->adcCounts
                          + (cfebData->timeSlice(1))->timeSample(layer,curr_strip,cfebData->isDCFEB())->adcCounts)/2.;

              // Loop over Central strip timesamples
              for (int itime=0; itime<nTimeSamples; itime++)
                {

                  if (cfebData->timeSlice(itime)->checkCRC())
                    {

                      CSCCFEBDataWord* timeSample=(cfebData->timeSlice(itime))->timeSample(layer,curr_strip,cfebData->isDCFEB());
                      int Qi = (int) ((timeSample->adcCounts)&0xFFF);
                      xtalkdata.content[curr_dac][layer-1][icfeb*16+curr_strip-1][itime].cnt++;
                      xtalkdata.content[curr_dac][layer-1][icfeb*16+curr_strip-1][itime].mv += Qi-Q12;

                      /*
                      if (Qi-Q12>Qmax) {
                      Qmax=Qi-Q12;
                      if (curr_dac==TIME_STEPS-1) r04.content[layer-1][icfeb*16+curr_strip-1] = Qi;
                      xtalkdata.content[curr_dac][layer-1][icfeb*16+curr_strip-1][NSAMPLES].max=Qmax;
                      }
                      */

                      if (v02)
                        {
                          v02->Fill(itime*50+6.25*(TIME_STEPS-curr_dac), Qi-Q12);
                        }

                    }
                  else
                    {
                      LOG4CPLUS_WARN(logger,"Evt#" << std::dec << nCSCEvents[cscID] << ": " << cscID << " CRC failed strip " << icfeb << ":" << layer << ":" << curr_strip
                                     << ", time step" << curr_dac << ", time sample " << itime);
                    }

                }

              // Fill left Strip

              double Q12_left=0;
              int Qi_left = 0;

              if (!fFirstStrip)   // Not the first strip
                {
                  int cfeb = icfeb;
                  int strip = curr_strip -1;
                  if (strip == 0)
                    {
                      cfeb -= 1;
                      strip=16;
                    }

                  CSCCFEBData *  cfebData = data.cfebData(cfeb);

                  if (cfebData && cfebData->check())
                    {

                      if (cfebData->timeSlice(0)->checkCRC() &&  cfebData->timeSlice(1)->checkCRC())
                        {
                          // Calculate Left Strip Pedestals
                          Q12_left=((cfebData->timeSlice(0))->timeSample(layer,strip,cfebData->isDCFEB())->adcCounts
                                    + (cfebData->timeSlice(1))->timeSample(layer,strip,cfebData->isDCFEB())->adcCounts)/2.;

                          // Loop over Left Strip timesamples
                          for (int itime=0; itime<nTimeSamples; itime++)
                            {
                              if (cfebData->timeSlice(itime)->checkCRC())
                                {

                                  CSCCFEBDataWord* timeSample=(cfebData->timeSlice(itime))->timeSample(layer,strip,cfebData->isDCFEB());

                                  Qi_left = (int) ((timeSample->adcCounts)&0xFFF);

                                  if (v03)
                                    {
                                      v03->Fill(itime*50+6.25*(TIME_STEPS-curr_dac), Qi_left-Q12_left);
                                    }

                                  xtalkdata.content[curr_dac][layer-1][icfeb*16+curr_strip-1][itime].left += Qi_left-Q12_left;
                                  xtalkdata.content[curr_dac][layer-1][icfeb*16+curr_strip-1][itime].left_cnt++;

                                }
                              else
                                {
                                  LOG4CPLUS_WARN(logger,"Evt#" << std::dec << nCSCEvents[cscID] << ": " << cscID
                                                 << " CRC failed, left strip " << icfeb << ":" << layer << ":" << curr_strip
                                                 << ", time step" << curr_dac << ", time sample " << itime);
                                }

                            }

                        }
                      else
                        {
                          LOG4CPLUS_WARN(logger, cscID << " CRC check failed for left strip time sample 1 and 2");
                        }

                    }
                  else
                    {
                      LOG4CPLUS_WARN(logger,"Evt#" << std::dec << nCSCEvents[cscID] << ": " << cscID << " strip " << icfeb+1 << ":" << layer << ":" << curr_strip
                                     << " - No data for left strip " <<  cfeb+1 << ":" << strip);
                    }

                } // Left Strip


              // Fill Right Strip

              double Q12_right=0;
              int Qi_right = 0;

              if (!fLastStrip)   // Not the last strip
                {
                  int cfeb = icfeb;
                  int strip = curr_strip+1;
                  if (strip == 17)
                    {
                      cfeb += 1;
                      strip =1;
                    }
                  CSCCFEBData * cfebData =  data.cfebData(cfeb);
                  if (cfebData && cfebData->check())
                    {

                      if (cfebData->timeSlice(0)->checkCRC() && cfebData->timeSlice(1)->checkCRC())
                        {

                          // Calculate Right Strip Pedestals
                          Q12_right=((cfebData->timeSlice(0))->timeSample(layer,strip,cfebData->isDCFEB())->adcCounts
                                     + (cfebData->timeSlice(1))->timeSample(layer,strip,cfebData->isDCFEB())->adcCounts)/2.;

                          // Loop over Right strip timesamples
                          for (int itime=0; itime<nTimeSamples; itime++)
                            {

                              if (cfebData->timeSlice(itime)->checkCRC())
                                {

                                  CSCCFEBDataWord* timeSample=(cfebData->timeSlice(itime))->timeSample(layer,strip,cfebData->isDCFEB());

                                  Qi_right = (int) ((timeSample->adcCounts)&0xFFF);

                                  if (v03)
                                    {
                                      v03->Fill(itime*50+6.25*(TIME_STEPS-curr_dac), Qi_right-Q12_right);
                                    }

                                  xtalkdata.content[curr_dac][layer-1][icfeb*16+curr_strip-1][itime].right += Qi_right-Q12_right;

                                  xtalkdata.content[curr_dac][layer-1][icfeb*16+curr_strip-1][itime].right_cnt++;
                                }
                              else
                                {
                                  LOG4CPLUS_WARN(logger,"Evt#" << std::dec << nCSCEvents[cscID] << ": " << cscID
                                                 << " CRC failed, right strip " << icfeb << ":" << layer << ":" << curr_strip
                                                 << ", time step" << curr_dac << ", time sample " << itime);
                                }

                            }
                        }
                      else
                        {
                          LOG4CPLUS_WARN(logger, cscID << " CRC check failed for time right strip sample 1 and 2");
                        }

                    }
                  else
                    {
                      LOG4CPLUS_WARN(logger, "Evt#" << std::dec << nCSCEvents[cscID] << ": " << cscID << " strip " << icfeb+1 << ":" << layer << ":" << curr_strip
                                     << " - No data for right strip " <<  cfeb+1 << ":" << strip);
                    }
                } // Right Strip


            } // Layers

        } // CFEBs

    } // CFEB data available

}


void Test_CFEB03::finishCSC(std::string cscID)
{
  /*
     if (nCSCEvents[cscID] < nExpectedEvents/2) {
     std::cout << Form("%s: Not enough events for test analysis (%d events)", cscID.c_str(), nCSCEvents[cscID] ) << std::endl;
     // = Set error
     return;
     }
  */
  cscTestData::iterator td_itr =  tdata.find(cscID);
  if (td_itr != tdata.end())
    {


      TestData& cscdata= td_itr->second;

      TestData2D& mask = cscdata["_MASK"];

      TestData2D& r01 = cscdata["R01"];
      TestData2D& r02 = cscdata["R02"];
      TestData2D& r03 = cscdata["R03"];
      TestData2D& r04 = cscdata["R04"];
      TestData2D& r05 = cscdata["R05"];
      TestData2D& r06 = cscdata["R06"];
      TestData2D& r07 = cscdata["R07"];
      TestData2D& r08 = cscdata["R08"];
      TestData2D& r09 = cscdata["R09"];
      TestData2D& r10 = cscdata["R10"];
      TestData2D& r11 = cscdata["R11"];


      XtalkData& xtalkdata = xdata[cscID];
      MonHistos& cschistos = mhistos[cscID];
      TH2F* v01 = reinterpret_cast<TH2F*>(cschistos["V01"]);
      TH2F* v05 = reinterpret_cast<TH2F*>(cschistos["V05"]);
      TH2F* v06 = reinterpret_cast<TH2F*>(cschistos["V06"]);
      TH2F* v07 = reinterpret_cast<TH2F*>(cschistos["V07"]);
      TH2F* v08 = reinterpret_cast<TH2F*>(cschistos["V08"]);
      TH2F* v09 = reinterpret_cast<TH2F*>(cschistos["V09"]);
      TH2F* v10 = reinterpret_cast<TH2F*>(cschistos["V10"]);

      ResultsCodes& rcodes = rescodes[cscID];

      /*
        std::ofstream res_out;

        res_out.open((cscID+"_dump.dat").c_str(),std::ios::out);
        res_out << "It: ";
        for (int i=0; i<64; i++) {
        res_out << It[i] << " ";
        }
        res_out << std::endl;
        res_out.close();
      */
      CSCtoHWmap::iterator itr = cscmap.find(cscID);

      if (itr != cscmap.end())
        {

          int dmbID = itr->second.second;
          if (dmbID >= 6) --dmbID;
          int id = 10*itr->second.first+dmbID;

          CSCMapItem::MapItem mapitem = cratemap->item(id);
          int first_strip_index=mapitem.stripIndex;
          // int strips_per_layer=mapitem.strips;
          int strips_per_layer = getNumStrips(cscID, theFormatVersion);


          //      bool fValid=true;

          for (unsigned int layer = 1; layer <= 6; layer++)
            {

              int nCFEBs = getNumStrips(cscID, theFormatVersion)/16;
              for (int icfeb=0; icfeb<nCFEBs; icfeb++)   // loop over cfebs in a given chamber
                {

                  for (int strip = 1; strip <=16  ; ++strip)   // loop over cfeb strip
                    {

                      double max=0;
                      double peak_time=0;
                      double max_left=0;
                      double max_right=0;
                      //      time_step& val= xtalkdata.content[0][layer-1][icfeb*16+strip-1][NSAMPLES];
                      int cnt=0;
                      bool fValid=true;

                      for (int dac=0; dac<TIME_STEPS; dac++)
                        {
                          for (int itime=0; itime < NSAMPLES; itime++)
                            {

                              time_step& cval = xtalkdata.content[dac][layer-1][icfeb*16+strip-1][itime];
                              cnt = cval.cnt;

                              if (cval.cnt<13)
                                {
                                  LOG4CPLUS_DEBUG(logger, cscID << ":" << layer << ":" << (icfeb*16+strip)
                                                  << " Error> time step=" << dac << ", sample=" << itime << ", cnt="<< cval.cnt);
                                  fValid=false;

                                }
                              else
                                {

                                  cval.mv /=cval.cnt;
                                  if (cval.left_cnt) cval.left /=cval.left_cnt;
                                  if (cval.right_cnt) cval.right /=cval.right_cnt;

                                  if ((cval.mv > max) && (dac<7))
                                    {
                                      peak_time = itime*50. + (TIME_STEPS-dac)*6.25;
                                      max=cval.mv;
                                      cnt=cval.cnt;
                                    }

                                  if ((cval.left > max_left) )
                                    {
                                      max_left=cval.left;
                                    }

                                  if ((cval.right > max_right) )
                                    {
                                      max_right=cval.right;
                                    }

                                }

                            }

                          if (v01)
                            {
                              v01->Fill(dac,cnt);
                            }
                          if (!fValid)
                            {
                              rcodes["V01"] = 4;
                            }

                        } // DAC steps loop

                      if (fValid)
                        {

                          r01.content[layer-1][icfeb*16+strip-1] = max;
                          r02.content[layer-1][icfeb*16+strip-1] = peak_time;
                          r04.content[layer-1][icfeb*16+strip-1] = max_left/max;
                          r05.content[layer-1][icfeb*16+strip-1] = max_right/max;


                          // == Find FWHM
                          int half_max = (int)max/2;
                          int fwhm_left= half_max;
                          int fwhm_right=half_max;
                          double fwhm_left_time=0;
                          double fwhm_right_time=400;;
                          double fwhm=0;
                          for (int itime=0; itime < NSAMPLES; itime++)
                            {
                              for (int step=0; step<8; step++)
                                {
                                  time_step& cval = xtalkdata.content[step][layer-1][icfeb*16+strip-1][itime];
                                  double fwhm_time = itime*50. + (7-step)*6.25;
                                  int diff = abs(half_max-(int)cval.mv);
                                  if (fwhm_time < peak_time)
                                    {
                                      if (diff < fwhm_left)
                                        {
                                          fwhm_left = diff;
                                          fwhm_left_time=fwhm_time;
                                        }
                                    }
                                  else
                                    {
                                      if (diff < fwhm_right)
                                        {
                                          fwhm_right = diff;
                                          fwhm_right_time=fwhm_time;
                                        }
                                    }
                                }
                            }
                          if (fwhm_right_time > fwhm_left_time) fwhm = fwhm_right_time-fwhm_left_time;
                          r03.content[layer-1][icfeb*16+strip-1] = fwhm;
                          /*
                          std::cout << cscID << ":" << std::dec << layer << ":" << (icfeb*16+strip)
                          << " peak_amp=" << max << ", peak_time=" <<  peak_time << ", cnt=" << cnt
                          << ", fwhm=" << fwhm << " l:" << fwhm_left_time << ":" << fwhm_left
                          << " r:" << fwhm_right_time <<":" << fwhm_right
                          << ", lXtalk=" << max_left/max << ", rXtalk=" << max_right/max << std::endl;
                          */
                          std::ofstream res_out;
                          /*
                                   res_out.open((cscID+"_chan_dump.txt").c_str(),std::ios::out | std::ios::app);
                                   res_out << std::dec << layer << ":" << (icfeb*16+strip) << " ";
                                   for (int itime=0; itime < NSAMPLES; itime++) {
                                   for (int dac=0; dac<8; dac++) {
                                   time_step& cval = xtalkdata.content[7-dac][layer-1][icfeb*16+strip-1][itime];
                                   res_out << std::fixed << std::setprecision(1) << cval.mv << " ";
                                   }
                                   }
                                   res_out << std::endl;
                                   res_out.close();
                          */


                          int Qc[64];
                          int Ql[64];
                          int Qr[64];

                          double Qcc[64];
                          double Qlc[64];
                          double Qrc[64];
                          double Rl[64];
                          double Rr[64];

                          double Qcc_max=0;
                          double Qcc_peak_time=0;

                          bool fFirstStrip = (strip==1 && icfeb==0)? true: false;
                          // Exclude Last strip for all chambers and last strip for ME11s DCFEB4
                          bool fLastStrip = (strip==16 && (icfeb==(nCFEBs-1) || (((cscID.find("ME+1.1")==0) || (cscID.find("ME-1.1")==0)) && (icfeb==3)) ))? true: false;
                          if (((cscID.find("ME+1.1")==0) || (cscID.find("ME-1.1")==0 )) && ((strip==1 && icfeb==4)))
                            fFirstStrip = true;

                          // std::ofstream res_out;
                          //        res_out.open((cscID+"_dump.dat").c_str(),std::ios::out | std::ios::app);
                          for (int i=0; i<64; i++)
                            {
                              Qc[i]=0;
                              int itime = i/8;
                              int step = 7-i%8;
                              time_step& cval = xtalkdata.content[step][layer-1][icfeb*16+strip-1][itime];
                              Qc[i]=(int)cval.mv;
                              Ql[i]=(int)cval.left;
                              Qr[i]=(int)cval.right;
                              Qcc[i]=0;
                              Qlc[i]=0;
                              Qrc[i]=0;
                              for (int j=0; j<=i; j++)
                                {
                                  Qcc[i] += It[j]*Qc[i-j];
                                  Qlc[i] += It[j]*Ql[i-j];
                                  Qrc[i] += It[j]*Qr[i-j];
                                }
                              if (Qcc[i]>Qcc_max)
                                {
                                  Qcc_max = Qcc[i];
                                  Qcc_peak_time = i;
                                }
                              if (v05) v05->Fill(i*6.25, Qcc[i]);
                              if (Qcc[i]!=0) Rl[i] = Qlc[i]/Qcc[i];
                              if (v06 && !fFirstStrip) v06->Fill(i*6.25, Qlc[i]);
                              if (Qcc[i]!=0) Rr[i] = Qrc[i]/Qcc[i];
                              if (v07 && !fLastStrip) v07->Fill(i*6.25, Qrc[i]);
                            }

                          // r06.content[layer-1][icfeb*16+strip-1]=Qcc_peak_time*6.25;

                          for (int i=0; i<64; i++)
                            {
                              if (v08 && (Qcc[i]>0.25*Qcc_max) && !fFirstStrip) v08->Fill(i*6.25, Rl[i]);
                              if (v09 && (Qcc[i]>0.25*Qcc_max) && !fLastStrip) v09->Fill(i*6.25, Rr[i]);
                              if (v10 && !fFirstStrip && !fLastStrip) v10->Fill(i*6.25, (Qrc[i]-Qlc[i])/2.);
                            }

                          TVectorD params;
                          TVectorD errors;
                          std::vector< std::pair<double, double> > arrL;
                          std::vector< std::pair<double, double> > arrR;
                          for (int i=0; i<64; i++)
                            {
                              if ((Qcc[i]>0.25*Qcc_max) && !fFirstStrip)
                                {
                                  arrL.push_back(std::make_pair(i*6.25, Rl[i]));
                                }
                              if ((Qcc[i]>0.25*Qcc_max) && !fLastStrip)
                                {
                                  arrR.push_back(std::make_pair(i*6.25, Rr[i]));
                                }

                            }


                          if (!fFirstStrip)
                            {
                              TLinearFitter* lf = new TLinearFitter(1);
                              lf->SetFormula("1 ++ x");

                              for (unsigned i=0; i< arrL.size(); i++)
                                {
                                  lf->AddPoint(&(arrL[i].first), arrL[i].second, 1.);
                                }

                              lf->Eval();


                              lf->GetParameters(params);
                              lf->GetErrors(errors);
                              /*
                                for (Int_t i=0; i<2; i++) {
                                printf("left par[%d]=%f+-%f\n", i, params(i), errors(i));
                                }
                              */
                              r06.content[layer-1][icfeb*16+strip-1]=(params(1)!=0)?params(1):BAD_VALUE;
                              // r07.content[layer-1][icfeb*16+strip-1]=params(1)*r02.content[layer-1][icfeb*16+strip-1]+params(0);
                              r07.content[layer-1][icfeb*16+strip-1]=params(1)*Qcc_peak_time*6.25+params(0);
                              double chisquare=lf->GetChisquare();
                              r08.content[layer-1][icfeb*16+strip-1]=chisquare;
                              // printf("chisquare=%f\n", chisquare);

                              //        lf->Clear();
                              delete lf;
                            }
                          else   // Set Left crosstalk values to 0 for first strip in layer
                            {
                              r06.content[layer-1][icfeb*16+strip-1]=0;
                              r07.content[layer-1][icfeb*16+strip-1]=0;
                              r08.content[layer-1][icfeb*16+strip-1]=0;
                            }

                          if (!fLastStrip)
                            {
                              TLinearFitter* lf = new TLinearFitter(1);
                              lf->SetFormula("1 ++ x");

                              for (unsigned i=0; i< arrR.size(); i++)
                                {
                                  lf->AddPoint(&(arrR[i].first), arrR[i].second, 1.);
                                }

                              lf->Eval();
                              params.Clear();
                              errors.Clear();
                              lf->GetParameters(params);
                              lf->GetErrors(errors);
                              /*
                                for (Int_t i=0; i<2; i++) {
                                printf("right par[%d]=%f+-%f\n", i, params(i), errors(i));
                                }
                              */

                              r09.content[layer-1][icfeb*16+strip-1]=(params(1)!=0)?params(1):BAD_VALUE;
                              // r10.content[layer-1][icfeb*16+strip-1]=params(1)*r02.content[layer-1][icfeb*16+strip-1]+params(0);
                              r10.content[layer-1][icfeb*16+strip-1]=params(1)*Qcc_peak_time*6.25+params(0);
                              double chisquare=lf->GetChisquare();
                              r11.content[layer-1][icfeb*16+strip-1]=chisquare;
                              // printf("chisquare=%f\n", chisquare);

                              delete lf;
                            }
                          else    // Set Right crosstalk values to 0 for last strip in layer
                            {
                              r09.content[layer-1][icfeb*16+strip-1]=0;
                              r10.content[layer-1][icfeb*16+strip-1]=0;
                              r11.content[layer-1][icfeb*16+strip-1]=0;
                            }


                          /*
                          res_out << "c: "<< std::dec << layer << ":" << (icfeb*16+strip) << " " ;
                          for (int i=0; i<64; i++) {
                          res_out << Qc[i] << " ";
                          }
                          res_out << std::endl;

                          res_out << "l: "<< std::dec << layer << ":" << (icfeb*16+strip) << " " ;
                          for (int i=0; i<64; i++) {
                          res_out << Ql[i] << " ";
                          }

                          res_out << std::endl;

                          res_out << "r: "<< std::dec << layer << ":" << (icfeb*16+strip) << " " ;
                          for (int i=0; i<64; i++) {
                          res_out << Qr[i] << " ";
                          }
                          res_out << std::endl;

                          res_out.close();
                          */
                        }
                      else     // bad strip data
                        {
                          // setBadStripData(layer-1,icfeb*16+strip-1);
                        }

                    }
                }
            }

          // == Save results to text files
          std::string rpath = "Test_"+testID+"/"+outDir;
          std::string path = rpath+"/"+cscID+"/";

          //      if (checkResults(cscID)) { // Check if 20% of channels with left and right crosstalks are bad
          // == Save results for database transfer
          std::ofstream res_out((path+cscID+"_"+testID+"_DB.dat").c_str());

          std::vector<std::string> tests;
          tests.push_back("R01");
          tests.push_back("R02");
          tests.push_back("R03");
          tests.push_back("R04");
          tests.push_back("R05");


          for (int layer=0; layer<NLAYERS; layer++)
            {

              if ( emu::dqm::utils::isME11(cscID) && (theFormatVersion >= 2013)) // Handle post-LS1 ME11s with 7 DCFEBs
                {

                  for (int strip=0; strip<64; strip++)
                    {
                      int ch_index = first_strip_index+layer*80+strip;
                      res_out << std::fixed << std::setprecision(6) <<  ch_index << " "
                              << r01.content[layer][strip] << " "
                              << r02.content[layer][strip] << " "
                              << r03.content[layer][strip] << " "
                              << r04.content[layer][strip] << " "
                              << r05.content[layer][strip] << " "
                              << (int)(mask.content[layer][strip]) << " "
                              << checkChannel(cscdata, tests, layer, strip, cscID)
                              << std::endl;
                    }

                  // Zero 64-80 ME11 channels gap
                  for (int strip=64; strip<80; strip++)
                    {
                      int ch_index = first_strip_index+layer*80+strip;
                      res_out << std::fixed << std::setprecision(6) <<  ch_index << " "
                              << 0  << " " << 0 << " " << 0 << " " << 0 << " " << 0
                              << " " << 1 << " " << 1  << std::endl;
                    }

                  // Remap post-LS1 ME11a 48 channels
                  for (int strip=64; strip<strips_per_layer; strip++)
                    {
                      int ch_index = emu::dqm::utils::getME11a_first_strip_index(cscID, theFormatVersion) + layer*48 + (strip-64);
                      res_out << std::fixed << std::setprecision(6) <<  ch_index << " "
                              << r01.content[layer][strip] << " "
                              << r02.content[layer][strip] << " "
                              << r03.content[layer][strip] << " "
                              << r04.content[layer][strip] << " "
                              << r05.content[layer][strip] << " "
                              << (int)(mask.content[layer][strip]) << " "
                              << checkChannel(cscdata, tests, layer, strip, cscID)
                              << std::endl;

                    }

                }
              else
                {

                  for (int strip=0; strip<strips_per_layer; strip++)
                    {
                      int ch_index = first_strip_index+layer*strips_per_layer+strip;
                      res_out << std::fixed << std::setprecision(6) <<  ch_index << " "
                              << r01.content[layer][strip] << " "
                              << r02.content[layer][strip] << " "
                              << r03.content[layer][strip] << " "
                              << r04.content[layer][strip] << " "
                              << r05.content[layer][strip] << " "
                              << (int)(mask.content[layer][strip]) << " "
                              << checkChannel(cscdata, tests, layer, strip, cscID)
                              << std::endl;
                    }

                }
            }
          res_out.close();

          tests.clear();
          tests.push_back("R06");
          tests.push_back("R07");
          tests.push_back("R09");
          tests.push_back("R10");

          res_out.open((path+cscID+"_"+testID+"_DB_Xtalk.dat").c_str());

          for (int layer=0; layer<NLAYERS; layer++)
            {
              if ( emu::dqm::utils::isME11(cscID) && (theFormatVersion >= 2013)) // Handle post-LS1 ME11s with 7 DCFEBs
                {

                  for (int strip=0; strip<64; strip++)
                    {
                      int ch_index = first_strip_index+layer*80+strip;
                      res_out << std::fixed << std::resetiosflags(std::ios::fixed) << std::setprecision(-1)
                              << ch_index << " "
                              << checkChannelConstant("R06",r06.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R07",r07.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R09",r09.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R10",r10.content[layer][strip], CHECK_LIMIT)  << " "
                              << (int)(mask.content[layer][strip]) << " "
                              << checkChannel(cscdata, tests, layer, strip, cscID)
                              << std::endl;

                    }

                  // Zero 64-80 ME11 channels gap
                  for (int strip=64; strip<80; strip++)
                    {
                      int ch_index = first_strip_index+layer*80+strip;
                      res_out << std::fixed << std::setprecision(2) <<  ch_index << " "
                              << 0  << " " << 0 << " " << 0 << " " << 0
                              << " " << 1 << " " << 1  << std::endl;
                    }

                  // Remap post-LS1 ME11a 48 channels
                  for (int strip=64; strip<strips_per_layer; strip++)
                    {
                      int ch_index = emu::dqm::utils::getME11a_first_strip_index(cscID, theFormatVersion) + layer*48 + (strip-64);
                      res_out << std::fixed << std::resetiosflags(std::ios::fixed) << std::setprecision(-1)
                              << ch_index << " "
                              << checkChannelConstant("R06",r06.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R07",r07.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R09",r09.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R10",r10.content[layer][strip], CHECK_LIMIT)  << " "
                              << (int)(mask.content[layer][strip]) << " "
                              << checkChannel(cscdata, tests, layer, strip, cscID)
                              << std::endl;
                    }

                }
              else
                {

                  for (int strip=0; strip<strips_per_layer; strip++)
                    {
                      int ch_index = first_strip_index+layer*strips_per_layer+strip;
                      res_out << std::fixed << std::resetiosflags(std::ios::fixed) << std::setprecision(-1)
                              << ch_index << " "
                              << checkChannelConstant("R06",r06.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R07",r07.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R09",r09.content[layer][strip], CHECK_LIMIT)  << " "
                              << checkChannelConstant("R10",r10.content[layer][strip], CHECK_LIMIT)  << " "
                              << (int)(mask.content[layer][strip]) << " "
                              << checkChannel(cscdata, tests, layer, strip, cscID)
                              << std::endl;
                    }

                }
            }
          res_out.close();

        }
      else
        {
          LOG4CPLUS_WARN(logger, cscID << ": Invalid");
        }
    }
}


bool Test_CFEB03::checkResults(std::string cscID)
{
  bool isValid=true;
  cscTestData::iterator td_itr =  tdata.find(cscID);
  TestData& cscdata= td_itr->second;
  TestData2D& mask = cscdata["_MASK"];

  //   return isValid;

  if (td_itr != tdata.end())
    {
      TestData& cscdata= td_itr->second;
      TestData2D& r04 = cscdata["R04"];
      TestData2D& r05 = cscdata["R05"];

      int badChannels=0;
      // Check Left Xtalk
      for (int i=0; i<r04.Nlayers; i++)
        {
          for (int j=0; j<r04.Nbins; j++)
            {
              if (((r04.content[i][j] > 0.15) || (r04.content[i][j] < 0.03)) && ((int)(mask.content[i][j]) == 0)) badChannels++;
            }
        }
      if (badChannels/(float(r04.Nlayers*r04.Nbins)) >=0.2)
        {
          isValid=false;
          LOG4CPLUS_WARN(logger, cscID << ": 20% of channels have bad Left Crosstalk");
        }

      badChannels=0;
      // Check Right Xtalk
      for (int i=0; i<r05.Nlayers; i++)
        {
          for (int j=0; j<r05.Nbins; j++)
            {
              if ( ((r05.content[i][j] > 0.15 ) || (r05.content[i][j] < 0.03)) && ((int)(mask.content[i][j]) == 0)) badChannels++;
            }
        }
      if (badChannels/(float(r05.Nlayers*r05.Nbins)) >=0.2)
        {
          isValid=false;
          LOG4CPLUS_WARN(logger, cscID << ": 20% of channels have bad Right Crosstalk");
        }
    }

  return isValid;
}



