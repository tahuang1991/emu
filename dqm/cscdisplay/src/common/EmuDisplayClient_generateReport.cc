#include "emu/dqm/cscdisplay/EmuDisplayClient.h"
#include "emu/base/TypedFact.h"

#include <TClass.h>

void EmuDisplayClient::setCSCMapFile(std::string filename)
{
  if (filename != "")
    {
      cscMapping  = CSCReadoutMappingFromFile(cscMapFile_.toString());
    }

}

std::string EmuDisplayClient::getCSCName(std::string cscID, int& crate, int& slot, int& CSCtype, int& CSCposition )
{
  std::string cscName="";
  if (sscanf(cscID.c_str(), "CSC_%03d_%02d", &crate , &slot) == 2)
    {
      cscName=getCSCFromMap(crate,slot, CSCtype, CSCposition );
    }
  return cscName;
}

std::string EmuDisplayClient::getCSCFromMap(int crate, int slot, int& csctype, int& cscposition)
{
  //  LOG4CPLUS_INFO(logger_, "========== get CSC from Map crate" << crate << " slot" << slot);
  int iendcap = -1;
  int istation = -1;
  int iring = -1;
  // TODO: Add actual Map conversion
  int id = cscMapping.chamber(iendcap, istation, crate, slot, -1);
  std::string cscname="";
  if (id==0)
    {
      return cscname;
    }
  CSCDetId cid( id );
  iendcap = cid.endcap();
  istation = cid.station();
  iring = cid.ring();
  cscposition = cid.chamber();

  //  std::map<std::string, int> tmap = getCSCTypeToBinMap();
  std::string tlabel = emu::dqm::utils::getCSCTypeLabel(iendcap, istation, iring );
  std::map<std::string,int>::const_iterator it = tmap.find( tlabel );
  if (it != tmap.end())
    {
      csctype = it->second;
      //      LOG4CPLUS_INFO(logger_, "========== get CSC from Map label:" << tlabel << "/" << cscposition);
      cscname=std::string(Form("%s/%02d",tlabel.c_str(),cscposition));
    }
  else
    {
      //      LOG4CPLUS_INFO(logger_, "========== can not find map");
      csctype = 0;
    }

  // return bin number which corresponds for CSC Type (ex. ME+4/2 -> bin 18)
  return cscname;

}



MonitorElement* EmuDisplayClient::findME(std::string tag, std::string name)
{
  MonitorElement* me = 0;
  EmuMonitoringObject* mo=0;

  if (!isMEValid(tag,name, mo))
    {
      bookME(tag,name,tag, mo);
    }
  if (updateME(tag,name,mo))
    {
      if (mo) me = mo->getObject();
    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Unable to update " << tag << " " << name);
    }
  return me;

}


std::vector<std::string> EmuDisplayClient::getListOfFolders(std::string filter)
{
  std::vector<std::string> dirlist;
  std::map<std::string, std::set<int> >::iterator itr;
  for (itr = foldersMap.begin(); itr != foldersMap.end(); ++itr )
    {
      if (itr->first.find(filter) != std::string::npos) dirlist.push_back(itr->first);
    }

  return dirlist;
}


int EmuDisplayClient::generateSummaryReport(std::string runname, DQMReport& dqm_report)
{


  std::string runNumber = runname;


  std::vector<std::string> EMU_folders;
  std::vector<std::string> DDU_folders=getListOfFolders("DDU");
  std::vector<std::string> CSC_folders=getListOfFolders("CSC");


  ReportEntry entry;


  std::string hname="";


  std::map<std::string, uint32_t>::iterator stats_itr;

  // == DDUs checks and stats
  uint32_t ddu_evt_cntr = 0;
  uint32_t ddu_cntr = 0;
  uint32_t ddu_avg_events = 0;

  int CSCtype = 0;
  int CSCposition = 0;


  std::map<std::string, uint32_t> ddu_stats;
  hname = "All_DDUs_in_Readout";
  MonitorElement* me = findME("EMU", hname);

  if (me)
    {
      TH1D* h = reinterpret_cast<TH1D*>(me);

      TH1D* h_tmp = new TH1D("temp", "temp", 1000, h->GetMinimum(), h->GetMaximum()+1);

      // for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
      for (int i=int(h->GetXaxis()->GetXmin()); i<= MAX_DDU; i++) // Check only 36 DDUs
        {
          uint32_t cnt = uint32_t(h->GetBinContent(i));
          if (cnt>0)
            {
              ddu_cntr++;
              std::string dduName = Form("DDU_%02d", i);
              ddu_stats[dduName] = cnt;
              ddu_evt_cntr+=cnt;
              h_tmp->Fill(cnt);
            }
        }
      ddu_avg_events = (uint32_t)h_tmp->GetMean();
      delete h_tmp;

      if (ddu_cntr)
        {
          int hot_ddus = 0;
          int low_ddus = 0;
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d DDUs in Readout", ddu_cntr),NONE,"ALL_DDU_IN_READOUT"));
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("Total number of DDU events: %d ", ddu_evt_cntr),NONE,"ALL_DDU_EVENTS_COUNTER"));
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("Average number of events per DDU: %d ", ddu_avg_events),NONE));
          if (ddu_avg_events >= 500)   // Detect different efficiency DDUs if average number of events is reasonable (>500)
            {
              for (stats_itr=ddu_stats.begin(); stats_itr != ddu_stats.end(); ++stats_itr)
                {
                  double fract=((double)stats_itr->second)/ddu_avg_events;
                  std::string dduName=stats_itr->first;
                  if (fract > 5)
                    {
                      std::string diag=Form("Hot readout DDU: %d events, %.f times more than average DDU events counter (avg events=%d)",
                                            stats_itr->second, fract, ddu_avg_events);
                      dqm_report.addEntry(dduName, entry.fillEntry(diag, MINOR, "ALL_HOT_DDU_IN_READOUT"));
                      hot_ddus++;
                    }
                  else if (fract < 0.2)
                    {
                      std::string diag=Form("Low readout DDU: %d events, %f fraction of average DDU events counter (avg events=%d)",
                                            stats_itr->second, fract, ddu_avg_events);
                      dqm_report.addEntry(dduName, entry.fillEntry(diag, MINOR, "ALL_LOW_DDU_IN_READOUT"));
                      low_ddus++;
                    }
                }
            }
        }
      else
        {
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d DDUs with data", ddu_cntr),CRITICAL,"ALL_NO_DDU_IN_READOUT"));
        }
    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }

  // Check for DDU trailer errors
  uint32_t ddu_with_errs = 0;
  uint32_t ddu_err_cntr = 0;
  std::map<std::string, uint32_t> ddu_err_stats;
  hname = "All_DDUs_Trailer_Errors";
  me = findME("EMU", hname);

  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        {
          std::string dduName = Form("DDU_%02d", i);
          uint32_t events = ddu_stats[dduName];
          // Workaround for DDU "S-Link Not Ready" error in local runs
          // Check for every error bit and ignore "S-Link Not Ready" 100%
          bool fTrailerError = false;
          for (int j=3; j<int(h->GetYaxis()->GetXmax()); j++)
            {
              uint32_t cnt = uint32_t(h->GetBinContent(i,j));
              if (cnt>0)
                {
                  if ((j==23) && (events>0) && (cnt/events==1)) continue; // Ignore 100% S-Link Not Ready error
                  fTrailerError = true;
                  if (cnt > ddu_err_stats[dduName]) ddu_err_stats[dduName]=cnt;
                }
            }
          if (fTrailerError)
            {
              ddu_with_errs++;
              ddu_err_cntr+=ddu_err_stats[dduName];
            }
        }

      if (ddu_with_errs)
        {
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d DDUs with Trailer Error Status", ddu_with_errs),
                              NONE, "ALL_DDU_WITH_TRAILER_ERRORS"));
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("Total number of DDU events with Trailer Error Status: %d ", ddu_err_cntr),
                              NONE, "ALL_DDU_WITH_TRAILER_ERRORS"));
          if (ddu_avg_events >= 500)   // Detect DDUs with Trailer Errors Status if average number of events is reasonable (>500)
            {
              for (stats_itr=ddu_err_stats.begin(); stats_itr != ddu_err_stats.end(); ++stats_itr)
                {
                  std::string dduName=stats_itr->first;
                  uint32_t events = ddu_stats[dduName];
                  double fract=(((double)stats_itr->second)/events)*100;
                  DQM_SEVERITY severity=NONE;
                  if (fract >= 50.) severity=SEVERE;
                  else if (fract >20.) severity=TOLERABLE;
                  else if (fract > 1.) severity=MINOR;
                  std::string diag=Form("DDU Trailer Error Status: %d events, (%f%%)",
                                        stats_itr->second, fract);
                  dqm_report.addEntry(dduName, entry.fillEntry(diag, severity, "DDU_WITH_TRAILER_ERRORS"));
                }
            }
        }
    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }


  // Check for DDU Format errors
  uint32_t ddu_with_fmt_errs = 0;
  uint32_t ddu_fmt_err_cntr = 0;
  std::map<std::string, uint32_t> ddu_fmt_err_stats;
  hname = "All_DDUs_Format_Errors";
  me = findME("EMU", hname);

  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        {
          uint32_t cnt = uint32_t(h->GetBinContent(i,2));
          if (cnt>0)
            {
              ddu_with_fmt_errs++;
              std::string dduName = Form("DDU_%02d", i);
              ddu_fmt_err_stats[dduName] = cnt;
              ddu_fmt_err_cntr+=cnt;
            }
        }
      if (ddu_with_fmt_errs)
        {
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d DDUs with detected Format Errors", ddu_with_fmt_errs), NONE,"ALL_DDU_WITH_FORMAT_ERRORS"));
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("Total number of DDU events with detected Format Errors: %d ", ddu_fmt_err_cntr), NONE,"ALL_DDU_WITH_FORMAT_ERRORS"));
          if (ddu_avg_events >= 500)   // Detect DDUs with Format Errors if average number of events is reasonable (>500)
            {
              for (stats_itr=ddu_fmt_err_stats.begin(); stats_itr != ddu_fmt_err_stats.end(); ++stats_itr)
                {
                  std::string dduName=stats_itr->first;
                  uint32_t events = ddu_stats[dduName];
                  double fract=(((double)stats_itr->second)/events)*100;
                  DQM_SEVERITY severity=NONE;
                  if (fract >= 20.) severity=CRITICAL;
                  else if (fract >= 10.) severity=SEVERE;
                  else if (fract >5.) severity=TOLERABLE;
                  else if (fract >0.5) severity=MINOR;
                  std::string diag=Form("DDU Detected Format Errors: %d events, (%f%%)",
                                        stats_itr->second, fract);
                  dqm_report.addEntry(dduName, entry.fillEntry(diag, severity,"DDU_WITH_FORMAT_ERRORS"));
                }
            }

        }
    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }

  // Check for DDU Live Inputs
  uint32_t ddu_live_inputs = 0;
  std::map<std::string, uint32_t> ddu_live_inp_stats;
  hname =  "All_DDUs_Live_Inputs";
  me = findME("EMU", hname);

  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j<= int(h->GetYaxis()->GetXmax()); j++)
          {
            uint32_t cnt = uint32_t(h->GetBinContent(i,j));
            if (cnt>0)
              {
                ddu_live_inputs++;
                std::string dduName = Form("DDU_%02d", i);
                ddu_live_inp_stats[dduName] |= (1<<(j-1));
              }
          }
      dqm_report.addEntry("EMU Summary", entry.fillEntry(
                            Form("%d DDU Live Inputs detected", ddu_live_inputs),NONE,"ALL_DDU_WITH_LIVE_INPUTS"));
    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }

  // Check for DDU Inputs with Data
  uint32_t ddu_inp_w_data = 0;
  std::map<std::string, uint32_t> ddu_inp_data_stats;
  hname = "All_DDUs_Inputs_with_Data";
  me = findME("EMU", hname);

  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j<= int(h->GetYaxis()->GetXmax()); j++)
          {
            uint32_t cnt = uint32_t(h->GetBinContent(i,j));
            if (cnt>0)
              {
                ddu_inp_w_data++;
                std::string dduName = Form("DDU_%02d", i);
                ddu_inp_data_stats[dduName] |= (1<<(j-1));
              }

          }
      dqm_report.addEntry("EMU Summary", entry.fillEntry(
                            Form("%d DDU Inputs with Data detected", ddu_inp_w_data),NONE,"ALL_DDU_WITH_DATA"));
      if (ddu_avg_events >= 500)
        {
          for (stats_itr= ddu_inp_data_stats.begin(); stats_itr !=  ddu_inp_data_stats.end(); ++stats_itr)
            {
              std::string dduName=stats_itr->first;
              uint32_t live_inputs = ddu_live_inp_stats[dduName];
              uint32_t with_data = ddu_inp_data_stats[dduName];
              for (int i=0; i<16; i++)
                {
                  if ( ((live_inputs>>i) & 0x1) && !((with_data>>i) & 0x01))
                    {
                      std::string diag=Form("DDU Input #%d: No Data",i+1);
                      dqm_report.addEntry(dduName, entry.fillEntry(diag, MINOR, "DDU_NO_INPUT_DATA"));

                    }
                }
            }
        }
    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }

  // Check for DDU Inputs in ERROR state
  uint32_t ddu_inp_w_errors = 0;
  std::map<std::string, uint32_t> ddu_inp_w_errors_stats;
  std::map<std::string, std::vector<std::pair<uint32_t,double> > > ddu_inp_w_errors_ratio;
  hname = "All_DDUs_Inputs_Errors";
  me = findME("EMU", hname);

  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        {
          double events = h->GetBinContent(i,1) + h->GetBinContent(i,2);
          for (int j=3; j<= int(h->GetYaxis()->GetXmax()); j++)
            {
              uint32_t cnt = uint32_t(h->GetBinContent(i,j));
              std::string dduName = Form("DDU_%02d", i);
              if (cnt>0)
                {
                  ddu_inp_w_errors++;
                  ddu_inp_w_errors_stats[dduName] |= (1<<(j-3));
                }
              if (events) ddu_inp_w_errors_ratio[dduName].push_back(std::make_pair(cnt, (cnt/events)*100.) );
            }
        }
      if (ddu_inp_w_errors)
        {
          dqm_report.addEntry("EMU Summary", entry.fillEntry(
                                Form("%d DDU Inputs in ERROR state detected on %d DDUs",
                                     ddu_inp_w_errors, ddu_inp_w_errors_stats.size()),NONE, "ALL_DDU_INPUT_IN_ERROR_STATE"));
          if (ddu_avg_events >= 500)   // Detect DDUs Inputs in ERROR state if average number of events is reasonable (>500)
            {
              for (stats_itr= ddu_inp_w_errors_stats.begin(); stats_itr !=  ddu_inp_w_errors_stats.end(); ++stats_itr)
                {
                  std::string dduName=stats_itr->first;
                  uint32_t err_inputs = ddu_inp_w_errors_stats[dduName];
                  for (int i=0; i<16; i++)
                    {
                      if ( (err_inputs>>i) & 0x1)
                        {
                          DQM_SEVERITY severity=NONE;
                          double fract = ddu_inp_w_errors_ratio[dduName][i].second;
                          if (fract >= 50.) severity=SEVERE;
                          else if (fract >20.) severity=TOLERABLE;
                          else if (fract > 1.) severity=MINOR;
                          std::string diag=Form("DDU Input #%d: detected ERROR state in %d events, (%f%%)",i+1,
                                                ddu_inp_w_errors_ratio[dduName][i].first, fract );
                          dqm_report.addEntry(dduName, entry.fillEntry(diag, severity, "DDU_INPUT_IN_ERROR_STATE" ));
                        }
                    }
                }
            }
        }

    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }

  // Check for DDU Inputs with WARNING state
  uint32_t ddu_inp_w_warn = 0;
  std::map<std::string, uint32_t> ddu_inp_w_warn_stats;
  std::map<std::string, std::vector<std::pair<uint32_t,double> > > ddu_inp_w_warn_ratio;
  hname = "All_DDUs_Inputs_Warnings";
  me = findME("EMU", hname);

  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        {
          double events = h->GetBinContent(i,1) + h->GetBinContent(i,2);
          for (int j=3; j<= int(h->GetYaxis()->GetXmax()); j++)
            {
              uint32_t cnt = uint32_t(h->GetBinContent(i,j));
              std::string dduName = Form("DDU_%02d", i);
              if (cnt>0)
                {
                  ddu_inp_w_warn++;
                  ddu_inp_w_warn_stats[dduName] |= (1<<(j-3));
                }
              if (events) ddu_inp_w_warn_ratio[dduName].push_back(std::make_pair(cnt, (cnt/events)*100.) );
            }
        }

      if (ddu_inp_w_warn)
        {
          dqm_report.addEntry("EMU Summary",entry.fillEntry(
                                Form("%d DDU Inputs in WARNING state detected on %d DDUs",
                                     ddu_inp_w_warn, ddu_inp_w_warn_stats.size()), NONE, "ALL_DDU_INPUT_IN_WARNING_STATE"));
          if (ddu_avg_events >= 500)   // Detect DDUs Inputs in ERROR state if average number of events is reasonable (>500)
            {
              for (stats_itr= ddu_inp_w_warn_stats.begin(); stats_itr !=  ddu_inp_w_warn_stats.end(); ++stats_itr)
                {
                  std::string dduName=stats_itr->first;
                  uint32_t warn_inputs = ddu_inp_w_warn_stats[dduName];
                  for (int i=0; i<16; i++)
                    {
                      if ( (warn_inputs>>i) & 0x1)
                        {
                          std::string diag=Form("DDU Input #%d: detected WARNING state in %d events (%f%%)",i+1,
                                                ddu_inp_w_warn_ratio[dduName][i].first,
                                                ddu_inp_w_warn_ratio[dduName][i].second);
                          dqm_report.addEntry(dduName, entry.fillEntry(diag, MINOR, "DDU_INPUT_IN_WARNING_STATE"));

                        }
                    }
                }
            }
        }
    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }


  // == Chambers checks and stats
  uint32_t csc_evt_cntr = 0;
  uint32_t csc_cntr = 0;
  uint32_t csc_avg_events = 0;
  std::map<std::string, uint32_t> csc_stats;
  std::map<std::string, bool> deadALCT;
  std::map<std::string, bool> deadCLCT;
  std::map<std::string, bool> deadCFEBs;
  std::map<std::string, bool> hotChamber;

  hname = "CSC_Reporting";
  me = findME("EMU", hname);

  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int n_csc_types = int(h->GetYaxis()->GetXmax());
      std::vector<uint32_t> csc_type_stats(n_csc_types);
      std::vector<uint32_t> csc_type_avg_events(n_csc_types);
      TH1D* h_tmp1 = new TH1D("temp1", "temp1", 1000, h->GetMinimum(), h->GetMaximum()+1);
      std::vector<uint32_t> csc_type_cntr(n_csc_types);
      for (int j=int(h->GetYaxis()->GetXmax())-1; j>= int(h->GetYaxis()->GetXmin()); j--)
        {
          TH1D* h_tmp = new TH1D("temp", "temp", 1000, h->GetMinimum(), h->GetMaximum()+1);
          for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
            {
              std::string cscName = Form("%s/%02d", (emu::dqm::utils::getCSCTypeName(j)).c_str(), i);
              uint32_t cnt = uint32_t(h->GetBinContent(i, j+1));

              if (cnt>0)
                {
                  csc_type_stats[j] += cnt;
                  csc_type_cntr[j]++;
                  csc_stats[cscName] = cnt;
                  csc_evt_cntr+=cnt;
                  csc_cntr++;

                  h_tmp->Fill(cnt);
                  h_tmp1->Fill(cnt);

                }
            }
          csc_type_avg_events[j] = (uint32_t)h_tmp->GetMean();
          delete h_tmp;
          std::string diag=Form("%s: Total #Events: %d, Average #Events per CSC: %d, Active CSCs: %d",
                                (emu::dqm::utils::getCSCTypeName(j)).c_str(),
                                csc_type_stats[j],
                                csc_type_avg_events[j],
                                csc_type_cntr[j] );
          dqm_report.addEntry("EMU Summary", entry.fillEntry(diag));

        }

      csc_avg_events = (uint32_t)h_tmp1->GetMean();
      delete h_tmp1;
      if (csc_cntr)
        {
          // csc_avg_events = csc_evt_cntr/csc_cntr;
          int hot_cscs = 0;
          int low_cscs = 0;
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs with data", csc_cntr)));
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("Total number of CSC events: %d ", csc_evt_cntr)));
          dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("Average number of events per CSC: %d ", csc_avg_events)));


          for (int j=int(h->GetYaxis()->GetXmax())-1; j>= int(h->GetYaxis()->GetXmin()); j--)
            {
              bool isHotCSCPresent = false;  // Is there hot CSC present in ring, which could screw up other chamber efficiency

              for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
                {
                  std::string cscName = Form("%s/%02d", (emu::dqm::utils::getCSCTypeName(j)).c_str(), i);
                  if ( (csc_stats[cscName]>0) && (csc_type_avg_events[j]>200) )
                    {
                      double fract=((double)(csc_stats[cscName]))/csc_type_avg_events[j];
                      if (fract >= 10.)
                        {
                          std::string diag=Form("Hot chamber: %d events, %.f times more than %s type average events counter (avg events=%d)",
                                                csc_stats[cscName], fract,
                                                (emu::dqm::utils::getCSCTypeName(j)).c_str(),
                                                csc_type_avg_events[j]);
                          dqm_report.addEntry(cscName, entry.fillEntry(diag, CRITICAL, "CSC_HOT_CHAMBER"));
                          hot_cscs++;
                          hotChamber[cscName] = true;
                          isHotCSCPresent = true;
                        }
                      else if (csc_stats[cscName] >= 20*csc_avg_events)
                        {
                          std::string diag=Form("Hot chamber: %d events, %.f times more than system average events counter (avg events=%d)",
                                                csc_stats[cscName], csc_stats[cscName]/(1.*csc_avg_events),
                                                (int)csc_avg_events);
                          dqm_report.addEntry(cscName, entry.fillEntry(diag, CRITICAL, "CSC_HOT_CHAMBER"));
                          hot_cscs++;
                          hotChamber[cscName] = true;
                          isHotCSCPresent = true;
                        }
                    }
                }

              for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
                {
                  std::string cscName = Form("%s/%02d", (emu::dqm::utils::getCSCTypeName(j)).c_str(), i);
                  if ( (csc_stats[cscName]>0) && (csc_type_avg_events[j]>200) )
                    {
                      double fract=((double)(csc_stats[cscName]))/csc_type_avg_events[j];
                      double avg = round(100.*fract)/100;
                      if ((avg < 0.05) && !isHotCSCPresent)
                        {
                          std::string diag=Form("Low efficiency chamber: %d events, %f fraction of %s type average events counter (avg events=%d)",
                                                csc_stats[cscName], fract,
                                                (emu::dqm::utils::getCSCTypeName(j)).c_str(),
                                                csc_type_avg_events[j]);
                          dqm_report.addEntry(cscName, entry.fillEntry(diag, TOLERABLE, "CSC_LOW_EFF_CHAMBER"));
                          low_cscs++;
                        }
                    }
                }
            }
          if (hot_cscs) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d Hot CSCs", hot_cscs), CRITICAL, "ALL_HOT_CHAMBERS"));
          if (low_cscs) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d Low Efficiency CSCs", low_cscs), TOLERABLE, "ALL_LOW_EFF_CHAMBERS"));
        }

    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }

  // == Check for chambers with Format Errors
  hname =  "DMB_Format_Errors_Fract";
  me = findME("EMU", hname);
  if (me)
    {
      TH2D* h1 = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      for (int i=int(h1->GetXaxis()->GetXmin()); i<= int(h1->GetXaxis()->GetXmax()); i++)
        for (int j=int(h1->GetYaxis()->GetXmin()); j <= int(h1->GetYaxis()->GetXmax()); j++)
          {
            double z = h1->GetBinContent(i, j);

            if (z>0) // If chamber has format errors
              {
                csc_cntr++;
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs with Format Errors", csc_cntr),NONE,"ALL_CHAMBERS_WITH_FORMAT_ERRORS"));
    }
  else
    {
      if (debug) LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }


  // == Check for chambers with DMB-Input FIFO Full
  me = findME("EMU", "DMB_input_fifo_full_Fract");
  if (me)
    {
      TH2D* h1 = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      for (int i=int(h1->GetXaxis()->GetXmin()); i<= int(h1->GetXaxis()->GetXmax()); i++)
        for (int j=int(h1->GetYaxis()->GetXmin()); j <= int(h1->GetYaxis()->GetXmax()); j++)
          {
            double z = h1->GetBinContent(i, j);

            if (z>0) // If chamber has DMB Input FIFO Full
              {
                csc_cntr++;
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs with DMB-Input FIFO Full", csc_cntr),NONE,"ALL_CHAMBERS_WITH_INPUT_FIFO_FULL"));
    }

  // == Check for chambers with DMB-Input Timeout
  me = findME("EMU", "DMB_input_timeout_Fract");
  if (me)
    {
      TH2D* h1 = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      for (int i=int(h1->GetXaxis()->GetXmin()); i<= int(h1->GetXaxis()->GetXmax()); i++)
        for (int j=int(h1->GetYaxis()->GetXmin()); j <= int(h1->GetYaxis()->GetXmax()); j++)
          {
            double z = h1->GetBinContent(i, j);

            if (z>0) // If chamber has DMB Input Timeout
              {
                csc_cntr++;
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs with DMB-Input Timeouts", csc_cntr), NONE, "ALL_CHAMBERS_WITH_INPUT_TIMEOUT"));
    }

  // == Check for missing ALCT data blocks
  me = findME("EMU", "DMB_wo_ALCT_Fract");
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      uint32_t min_events=100;
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j <= int(h->GetYaxis()->GetXmax()); j++)
          {
            double z = h->GetBinContent(i, j);
            if (round(z*100)>95.)
              {
                csc_cntr++;
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                uint32_t csc_events = csc_stats[cscName];
                float fract=z*100;
                deadALCT[cscName]=false;
                if (csc_events>min_events)
                  {
                    deadALCT[cscName] = true;
                    if (!hotChamber[cscName])
                      {
                        std::string diag=Form("No ALCT Data: %.1f%%",fract);
                        dqm_report.addEntry(cscName, entry.fillEntry(diag, CRITICAL, "CSC_WITHOUT_ALCT"));
                      }
                  }
                else
                  {
                    if ((csc_events > 50) && (z == 1.))
                      {
                        if (!hotChamber[cscName])
                          {
                            std::string diag=Form("No ALCT Data (low stats): %.1f%%",fract);
                            dqm_report.addEntry(cscName, entry.fillEntry(diag, TOLERABLE, "CSC_WITHOUT_ALCT"));
                          }
                      }
                  }
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs without ALCT data", csc_cntr), NONE, "ALL_CHAMBERS_WITHOUT_ALCT"));
    }

  // == Check for missing CLCT data blocks
  me = findME("EMU", "DMB_wo_CLCT_Fract");
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      uint32_t min_events=100;
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j <= int(h->GetYaxis()->GetXmax()); j++)
          {
            double z = h->GetBinContent(i, j);
            if (round(z*100)>95.)
              {
                csc_cntr++;
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                uint32_t csc_events = csc_stats[cscName];
                float fract=z*100;
                if (csc_events>min_events)
                  {
                    deadCLCT[cscName] = true;
                    if (!hotChamber[cscName])
                      {
                        std::string diag=Form("No CLCT Data: %.1f%%",fract);
                        dqm_report.addEntry(cscName, entry.fillEntry(diag,CRITICAL, "CSC_WITHOUT_CLCT"));
                      }
                  }
                else
                  {
                    if ((csc_events > 50) && (z == 1.))
                      {
                        if (!hotChamber[cscName])
                          {
                            std::string diag=Form("No CLCT Data (low stats): %.1f%%",fract);
                            dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_WITHOUT_CLCT"));
                          }
                      }
                  }
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs without CLCT data", csc_cntr), NONE, "ALL_CHAMBERS_WITHOUT_CLCT"));
    }

  // == Check for missing CFEB data blocks
  me = findME("EMU", "DMB_wo_CFEB_Fract");
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      uint32_t min_events=100;
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j <= int(h->GetYaxis()->GetXmax()); j++)
          {
            double z = h->GetBinContent(i, j);
            if (round(z*100)>95.)
              {
                csc_cntr++;
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                uint32_t csc_events = csc_stats[cscName];
                float fract=z*100;
                if (csc_events>min_events)
                  {
                    deadCFEBs[cscName] = true;
                    std::string diag=Form("No CFEB Data: %.1f%%",fract);
                    dqm_report.addEntry(cscName, entry.fillEntry(diag,CRITICAL, "CSC_WITHOUT_CFEB"));
                  }
                else
                  {
                    if ((csc_events > 50) && (z == 1.))
                      {
                        std::string diag=Form("No CFEB Data (low stats): %.1f%%",fract);
                        dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_WITHOUT_CFEB"));
                      }
                  }
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs without CFEB data", csc_cntr),NONE,"ALL_CHAMBERS_WITHOUT_CFEB"));
    }

  // == Check for missing ALCT Timing issues
  me = findME("EMU", "CSC_ALCT0_BXN_rms");
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      uint32_t min_events=200;
      // double rms_limit = 1.81;
      double rms_limit = 1.91; // Post-LS1
      for (int j=int(h->GetYaxis()->GetXmax())-1; j>= int(h->GetYaxis()->GetXmin()); j--)
        for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
          {
            std::string cscName = Form("%s/%02d", (emu::dqm::utils::getCSCTypeName(j)).c_str(), i);
            if (!deadALCT[cscName])   ///* Don't check ALCT Timing if ALCT is dead on this chamber
              {
                double limit = rms_limit;
                // if (emu::dqm::utils::isME42(cscName)) limit = rms_limit + 0.2; // Handle ME42 chambers, which have different timing pattern
                double z = h->GetBinContent(i, j+1);
                double avg = round(z*10.)/10.;
                if (avg > limit)
                  {
                    uint32_t csc_events = csc_stats[cscName];
                    if (csc_events>min_events)
                      {
                        csc_cntr++;
                      }
                  }
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs with ALCT Timing Problems", csc_cntr),NONE,"ALL_CHAMBERS_ALCT_TIMING"));
    }


  // == Check for missing CLCT Timing issues
  me = findME("EMU", "CSC_CLCT0_BXN_rms");
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      uint32_t min_events=200;
      // double rms_limit = 2.11;
      double rms_limit = 2.31; // Post-LS1
      for (int j=int(h->GetYaxis()->GetXmax())-1; j>= int(h->GetYaxis()->GetXmin()); j--)
        for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
          {
            std::string cscName = Form("%s/%02d", (emu::dqm::utils::getCSCTypeName(j)).c_str(), i);
            if (!deadCLCT[cscName])   ///* Don't check CLCT Timing if CLCT is dead on this chamber
              {
                double limit = rms_limit;
                double z = h->GetBinContent(i, j+1);
                double avg = round(z*10.)/10.;
                if (avg > limit)
                  {
                    uint32_t csc_events = csc_stats[cscName];
                    if (csc_events>min_events)
                      {
                        csc_cntr++;
                      }
                  }
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs with CLCT Timing Problems", csc_cntr),NONE,"ALL_CHAMBERS_CLCT_TIMING"));
    }


  // == Check for chambers with L1A out of sync
  me = findME("EMU", "DMB_L1A_out_of_sync_Fract");
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j <= int(h->GetYaxis()->GetXmax()); j++)
          {
            double z = h->GetBinContent(i, j);
            if (z>0)
              {
                csc_cntr++;
              }

          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs with L1A out of sync", csc_cntr),NONE,"ALL_CHAMBERS_WITH_L1A_OUT_OF_SYNC"));
    }


  // == Check for chambers with format warnings (CFEB B-Words)
  me = findME("EMU", "DMB_Format_Warnings_Fract");
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j <= int(h->GetYaxis()->GetXmax()); j++)
          {
            double z = h->GetBinContent(i, j);
            if (z>0)
              {
                csc_cntr++;
              }
          }
      if (csc_cntr) dqm_report.addEntry("EMU Summary", entry.fillEntry(Form("%d CSCs with CFEB B-Words", csc_cntr), NONE, "ALL_CHAMBERS_WITH_BWORDS"));
    }
  return 0;
}

// == Save DQM Report to file
std::string EmuDisplayClient::getReportJSON(std::string runname)
{
  T_DQMReport::iterator itr;
  vector<ReportEntry>::iterator err_itr;


  T_DQMReport& report = dqm_report.getReport();

  std::stringstream out;
  out << "var DQM_REPORT = { \"run\": \"" << runname
      << "\", \"genDate\": \"" << emu::dqm::utils::now()
      << "\", \"version\": \"" << dqm_report.getVersion()
      <<  "\", \"report\":\n[" << std::endl;


  for (itr = report.begin(); itr != report.end(); ++itr)
    {
      if (itr->first.find("EMU") == 0)
        {
          std::string scope = "EMU";
          out << "{\"objID\": \"" << scope << "\", \"name\": \"" << itr->first << "\", \"list\": [" << std::endl;
          for (err_itr = itr->second.begin(); err_itr != itr->second.end(); ++err_itr)
            {
              out << "\t{\"testID\": \"" << err_itr->testID
                  << "\", \"scope\": \"" << scope
                  << "\", \"descr\": \"" << err_itr->descr
                  << "\", \"severity\": \"" << err_itr->severity << "\"}";
              if ((err_itr+1) != itr->second.end()) out << ",";
              out << std::endl;
            }
          out << "]}," << std::endl;
        }
    }

  for (itr = report.begin(); itr != report.end(); ++itr)
    {
      if (itr->first.find("DDU") == 0)
        {
          std::string scope = "DDU";
          out << "{\"objID\": \"" << itr->first << "\", \"name\": \"" << itr->first << "\", \"list\": [" << std::endl;
          for (err_itr = itr->second.begin(); err_itr != itr->second.end(); ++err_itr)
            {
              out << "\t{\"testID\": \"" << err_itr->testID
                  << "\", \"scope\": \"" << scope
                  << "\", \"descr\": \"" << err_itr->descr
                  << "\", \"severity\": \"" << err_itr->severity << "\"}";
              if ((err_itr+1) != itr->second.end()) out << ",";
              out << std::endl;
            }
          out << "]}," << std::endl;
        }
    }

  for (itr = report.begin(); itr != report.end(); ++itr)
    {
      if (itr->first.find("ME") == 0)
        {
          std::string scope = "CSC";
          out << "{\"objID\": \"" << itr->first << "\", \"name\": \"" << itr->first << "\", \"list\": [" << std::endl;
          for (err_itr = itr->second.begin(); err_itr != itr->second.end(); ++err_itr)
            {
              out << "\t{\"testID\": \"" << err_itr->testID
                  << "\", \"scope\": \"" << scope
                  << "\", \"descr\": \"" << err_itr->descr
                  << "\", \"severity\": \"" << err_itr->severity << "\"}";
              if ((err_itr+1) != itr->second.end()) out << ",";
              out << std::endl;
            }
          out << "]},";
          out << std::endl;
        }
    }

  out << "]\n};" << std::endl;

  return out.str();


}


DQMReport EmuDisplayClient::mergeNodesReports(std::map<uint32_t, DQMReport> rep_lists)
{
  DQMReport merged_report;
  T_DQMReport& m_report = merged_report.getReport();
  std::map<uint32_t, DQMReport>::iterator list_itr;
  for (list_itr = rep_lists.begin(); list_itr != rep_lists.end(); ++list_itr)
    {

      T_DQMReport& n_report = list_itr->second.getReport();
      T_DQMReport::iterator rep_itr;
      T_DQMReport::iterator new_rep_itr;

      for (rep_itr = n_report.begin(); rep_itr != n_report.end(); ++rep_itr)
        {
          new_rep_itr = m_report.find(rep_itr->first);
          if (new_rep_itr == m_report.end()) // No report for this object
            {

              m_report.insert(make_pair(rep_itr->first, rep_itr->second));

            }
          else // Report for this object is already in merged report
            {
              // !!! No merging for multiple report entries yet
            }
        }


    }

  return merged_report;
}

// == Prepare DQM Report facts to send to Expert System
int EmuDisplayClient::prepareReportFacts(std::string runname)
{

  int nFacts=0;
  std::string severity="INFO";
  T_DQMReport::iterator itr;
  vector<ReportEntry>::iterator err_itr;


  T_DQMReport& report = dqm_report.getReport();



  for (itr = report.begin(); itr != report.end(); ++itr)
    {
      if (itr->first.find("EMU") == 0)
        {
          emu::base::Component comp("ME");
          for (err_itr = itr->second.begin(); err_itr != itr->second.end(); ++err_itr)
            {
              emu::base::TypedFact<DqmReportFact> fact;
              fact
              .setRun( runname )
              .setComponent( comp )
              .setSeverity( err_itr->severity == NONE ? emu::base::Fact::INFO : (emu::base::Fact::Severity_t) err_itr->severity )
              .setDescription( err_itr->descr )
              .setParameter( DqmReportFact::testId, err_itr->testID );
              addFact(fact);
              nFacts++;
            }
        }
    }

  for (itr = report.begin(); itr != report.end(); ++itr)
    {
      if (itr->first.find("DDU") == 0)
        {
          std::string ddu_id = itr->first;
          ///** Replace RUI numbering with Post-LS1 FED/DDU id
          // Not needed anymore
          /*
          ddu_id = emu::dqm::utils::replaceRUIwithDDUId(itr->first);
          if (ddu_id == "")
            {
              LOG4CPLUS_WARN(logger_, "Unable to convert RUI " << itr->first << " to DDU Id");
              continue;
            }
          */
          emu::base::Component comp( std::string(ddu_id).erase( std::string(ddu_id).find('_'),1) );
          // emu::base::Component comp( std::string(itr->first).erase( std::string(itr->first).find('_'),1) );
          for (err_itr = itr->second.begin(); err_itr != itr->second.end(); ++err_itr)
            {
              emu::base::TypedFact<DqmReportFact> fact;
              fact
              .setRun( runname )
              .setComponent( comp )
              .setSeverity( err_itr->severity == NONE ? emu::base::Fact::INFO : (emu::base::Fact::Severity_t) err_itr->severity )
              .setDescription( err_itr->descr )
              .setParameter( DqmReportFact::testId, err_itr->testID );
              addFact(fact);
              nFacts++;
            }
        }
    }

  for (itr = report.begin(); itr != report.end(); ++itr)
    {
      if (itr->first.find("ME") == 0)
        {
          emu::base::Component comp(itr->first);
          for (err_itr = itr->second.begin(); err_itr != itr->second.end(); ++err_itr)
            {
              // Skip not enough statistics entries
              if (err_itr->descr.find("Not enough events for") != std::string::npos) continue;
              emu::base::TypedFact<DqmReportFact> fact;
              fact
              .setRun( runname )
              .setComponent( comp )
              .setSeverity( err_itr->severity == NONE ? emu::base::Fact::INFO : (emu::base::Fact::Severity_t) err_itr->severity )
              .setDescription( err_itr->descr )
              .setParameter( DqmReportFact::testId, err_itr->testID );
              addFact(fact);
              nFacts++;
            }
        }
    }

  return nFacts;

}
