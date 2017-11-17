#include "emu/dqm/cscanalyzer/EmuPlotter.h"
#include "TClass.h"

/*
  Generate Online DQM report
*/


int EmuPlotter::generateOnlineReport(std::string runname)
{
  appBSem_.take();
  dqm_report.clearReport();
  TDirectory *sourcedir = NULL;

  std::vector<std::string> EMU_folders;
  std::vector<std::string> DDU_folders=getListOfFolders("DDU", sourcedir);
  std::vector<std::string> CSC_folders=getListOfFolders("CSC", sourcedir);

  ReportEntry entry;

  std::string hname="";

  int CSCtype = 0;
  int CSCposition = 0;


  std::map<std::string, uint32_t>::iterator stats_itr;

  MonitorElement* me = NULL;


  std::map<std::string, uint32_t> csc_stats;
  std::map<std::string, bool> deadALCT;
  std::map<std::string, bool> deadCLCT;
  std::map<std::string, bool> deadCFEBs;

  hname = "DMB_Reporting";
  me = findME("EMU", hname,  sourcedir);


  // == Get number events for CSCs
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j <= int(h->GetYaxis()->GetXmax()); j++)
          {
            uint32_t cnt = uint32_t(h->GetBinContent(i, j));
            if (cnt>0)
              {
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                csc_stats[cscName] = cnt;
              }
          }
    }


  // == Check for chambers with Format Errors
  hname =  "DMB_Format_Errors_Fract";
  me = findME("EMU", hname,  sourcedir);
  MonitorElement* me2 = findME("EMU", "DMB_Format_Errors",  sourcedir);
  if (me && me2)
    {
      TH2D* h1 = reinterpret_cast<TH2D*>(me);
      TH2D* h2 = reinterpret_cast<TH2D*>(me2);
      int csc_cntr=0;
      uint32_t min_events=50;
      for (int i=int(h1->GetXaxis()->GetXmin()); i<= int(h1->GetXaxis()->GetXmax()); i++)
        for (int j=int(h1->GetYaxis()->GetXmin()); j <= int(h1->GetYaxis()->GetXmax()); j++)
          {
            double z = h1->GetBinContent(i, j);

            if (z>0) // If chamber has format errors
              {
                csc_cntr++;
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                uint32_t events =  uint32_t(h2->GetBinContent(i, j));

                float fract=z*100;
                DQM_SEVERITY severity=NONE;
                uint32_t csc_events = csc_stats[cscName];
                if (csc_events>min_events)
                  {
                    if (fract >= 80.) severity=CRITICAL;
                    else if (fract >= 20.) severity=SEVERE;
                    else if (fract > 5.) severity=TOLERABLE;
                    else if (fract > 0.5) severity=MINOR;
                  }
                std::string diag=Form("Format Errors: %d events (%.3f%%)",events, z*100);
                dqm_report.addEntry(cscName,entry.fillEntry(diag,severity,"CSC_WITH_FORMAT_ERRORS"));


                // --- Get Format Errors Details
                MonitorElement* me3 = findME(cscTag, "BinCheck_Errors_Frequency",  sourcedir);
                MonitorElement* me4 = findME(cscTag, "BinCheck_ErrorStat_Table",  sourcedir);
                if (me3 && me4)
                  {

                    TH2D* h3 = reinterpret_cast<TH2D*>(me3);
                    TH2D* h4 = reinterpret_cast<TH2D*>(me4);

                    for (int err=int(h3->GetYaxis()->GetXmin()); err <= int(h3->GetYaxis()->GetXmax()); err++)
                      {
                        double z = h3->GetBinContent(1, err);

                        if (z>0)
                          {
                            uint32_t events = uint32_t(h4->GetBinContent(1, err));
                            float fract=z*100;
                            DQM_SEVERITY severity=NONE;
                            if (csc_events>min_events)
                              {
                                if (fract >= 80.) severity=CRITICAL;
                                else if (fract >= 20.) severity=SEVERE;
                                else if (fract > 5.) severity=TOLERABLE;
                                else if (fract > 0.5) severity=MINOR;
                              }
                            std::string error_type = std::string(h3->GetYaxis()->GetBinLabel(err));
                            std::string diag=std::string(Form("\tFormat Errors: %s %d events (%.3f%%)",error_type.c_str(), events, z*100));
                            // LOG4CPLUS_WARN(logger_, cscTag << ": "<< diag);
                            dqm_report.addEntry(cscName, entry.fillEntry(diag,severity,"CSC_WITH_FORMAT_ERRORS"));
                          }
                      }
                  }
                // ---

              }
          }
    }
  else
    {
      LOG4CPLUS_WARN(logger_,"Can not find " << hname);
    }


  // == Check for chambers with DMB-Input FIFO Full
  me = findME("EMU", "DMB_input_fifo_full_Fract",  sourcedir);
  me2 = findME("EMU", "DMB_input_fifo_full",  sourcedir);
  if (me && me2)
    {
      TH2D* h1 = reinterpret_cast<TH2D*>(me);
      TH2D* h2 = reinterpret_cast<TH2D*>(me2);
      int csc_cntr=0;
      uint32_t min_events=50;
      for (int i=int(h1->GetXaxis()->GetXmin()); i<= int(h1->GetXaxis()->GetXmax()); i++)
        for (int j=int(h1->GetYaxis()->GetXmin()); j <= int(h1->GetYaxis()->GetXmax()); j++)
          {
            double z = h1->GetBinContent(i, j);

            if (z>0) // If chamber has DMB Input FIFO Full
              {
                csc_cntr++;
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                uint32_t events =  uint32_t(h2->GetBinContent(i, j));
                uint32_t csc_events = csc_stats[cscName];

                float fract=round(z*100);
                DQM_SEVERITY severity=MINOR;

                if (csc_events>min_events)
                  {
                    if (fract >= 80.) severity=CRITICAL;
                    else if (fract >= 10.) severity=SEVERE;
                    else if (fract > 1.) severity=TOLERABLE;
                  }
                std::string diag=Form("DMB-Input FIFO Full: %d events (%.3f%%)",events, z*100);
                dqm_report.addEntry(cscName, entry.fillEntry(diag, severity,"CSC_WITH_INPUT_FIFO_FULL"));


                // --- Get Format Errors Details
                MonitorElement* me3 = findME(cscTag, "BinCheck_DataFlow_Problems_Frequency",  sourcedir);
                MonitorElement* me4 = findME(cscTag, "BinCheck_DataFlow_Problems_Table",  sourcedir);
                if (me3 && me4)
                  {

                    TH2D* h3 = reinterpret_cast<TH2D*>(me3);
                    TH2D* h4 = reinterpret_cast<TH2D*>(me4);

                    for (int err=int(h3->GetYaxis()->GetXmin()); err <= 7; err++)
                      {
                        double z = h3->GetBinContent(1, err);

                        if (z>0)
                          {
                            uint32_t events = uint32_t(h4->GetBinContent(1, err));
                            float fract=round(z*100);
                            DQM_SEVERITY severity=MINOR;
                            if (csc_events>min_events)
                              {
                                if (fract >= 80.) severity=CRITICAL;
                                else if (fract >= 10.) severity=SEVERE;
                                else if (fract > 1.) severity=TOLERABLE;
                              }
                            std::string error_type = std::string(h3->GetYaxis()->GetBinLabel(err));
                            std::string diag=std::string(Form("DMB-Input FIFO Full: %s %d events (%.3f%%)",error_type.c_str(), events, z*100));
                            // LOG4CPLUS_WARN(logger_, cscTag << ": "<< diag);
                            dqm_report.addEntry(cscName, entry.fillEntry(diag, severity, "CSC_WITH_INPUT_FIFO_FULL"));
                          }
                      }
                  }
                // ---

              }
          }
    }

  // == Check for chambers with DMB-Input Timeout
  me = findME("EMU", "DMB_input_timeout_Fract",  sourcedir);
  me2 = findME("EMU", "DMB_input_timeout",  sourcedir);
  if (me && me2)
    {
      TH2D* h1 = reinterpret_cast<TH2D*>(me);
      TH2D* h2 = reinterpret_cast<TH2D*>(me2);
      int csc_cntr=0;
      uint32_t min_events=50;
      for (int i=int(h1->GetXaxis()->GetXmin()); i<= int(h1->GetXaxis()->GetXmax()); i++)
        for (int j=int(h1->GetYaxis()->GetXmin()); j <= int(h1->GetYaxis()->GetXmax()); j++)
          {
            double z = h1->GetBinContent(i, j);

            if (z>0) // If chamber has DMB Input Timeout
              {
                csc_cntr++;
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                uint32_t events =  uint32_t(h2->GetBinContent(i, j));
                uint32_t csc_events = csc_stats[cscName];

                float fract=round(z*100);
                DQM_SEVERITY severity=MINOR;
                if (csc_events>min_events)
                  {
                    if (fract >= 80.) severity=CRITICAL;
                    else if (fract >= 10.) severity=SEVERE;
                    else if (fract > 1.) severity=TOLERABLE;
                  }
                std::string diag=Form("DMB-Input Timeout: %d events (%.3f%%)",events, z*100);
                dqm_report.addEntry(cscName, entry.fillEntry(diag,severity, "CSC_WITH_INPUT_TIMEOUT"));


                // --- Get Format Errors Details
                MonitorElement* me3 = findME(cscTag, "BinCheck_DataFlow_Problems_Frequency",  sourcedir);
                MonitorElement* me4 = findME(cscTag, "BinCheck_DataFlow_Problems_Table",  sourcedir);
                if (me3 && me4)
                  {

                    TH2D* h3 = reinterpret_cast<TH2D*>(me3);
                    TH2D* h4 = reinterpret_cast<TH2D*>(me4);

                    for (int err=8; err<int(h3->GetYaxis()->GetXmax())-2; err++)
                      {
                        double z = h3->GetBinContent(1, err);

                        if (z>0)
                          {
                            uint32_t events = uint32_t(h4->GetBinContent(1, err));
                            float fract=round(z*100);
                            DQM_SEVERITY severity=MINOR;
                            if (csc_events>min_events)
                              {
                                if (fract >= 80.) severity=CRITICAL;
                                else if (fract >= 10.) severity=SEVERE;
                                else if (fract > 1.) severity=TOLERABLE;
                              }
                            std::string error_type = std::string(h3->GetYaxis()->GetBinLabel(err));
                            std::string diag=std::string(Form("DMB-Input Timeout: %s %d events (%.3f%%)",error_type.c_str(), events, z*100));
                            // LOG4CPLUS_WARN(logger_, cscTag << ": "<< diag);
                            dqm_report.addEntry(cscName, entry.fillEntry(diag, severity, "CSC_WITH_INPUT_TIMEOUT" ));
                          }
                      }
                  }
                // ---
              }
          }
    }

  // == Check for missing ALCT data blocks
  me = findME("EMU", "DMB_wo_ALCT_Fract",  sourcedir);
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
                // float fract=z*100;
                deadALCT[cscName]=false;
                if (csc_events>min_events)
                  {
                    deadALCT[cscName]=true;
                  }
              }
          }
    }

  // == Check for missing CLCT data blocks
  me = findME("EMU", "DMB_wo_CLCT_Fract",  sourcedir);
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
                // float fract=z*100;
                if (csc_events>min_events)
                  {
                    deadCLCT[cscName]=true;
                  }
              }
          }
    }

  // == Check for missing CFEB data blocks
  me = findME("EMU", "DMB_wo_CFEB_Fract",  sourcedir);
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
                // float fract=z*100;
                if (csc_events>min_events)
                  {
                    deadCFEBs[cscName]=true;
                  }
              }
          }
    }


// == Check for ALCT Timing issues
  me = findME("EMU", "CSC_ALCT0_BXN_rms",  sourcedir);
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      uint32_t min_events=400;
      double rms_limit = 1.81;
      if (theFormatVersion == 2013) rms_limit = 2.01;
      for (int j=int(h->GetYaxis()->GetXmax())-1; j>= int(h->GetYaxis()->GetXmin()); j--)
        for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
          {
            std::string cscName = Form("%s/%02d", (emu::dqm::utils::getCSCTypeName(j)).c_str(), i);
            if (!deadALCT[cscName])   ///* Don't check ALCT Timing if ALCT is dead on this chamber
              {
                double limit = rms_limit;
                if (emu::dqm::utils::isME42(cscName)) limit = rms_limit + 0.4; // Handle ME42 chambers, which have different timing pattern
                double z = h->GetBinContent(i, j+1);
                double avg = round(z*10.)/10.;
                if (avg > limit)
                  {
                    csc_cntr++;
                    uint32_t csc_events = csc_stats[cscName];
                    if (csc_events>min_events)
                      {
                        std::string diag=Form("ALCT Timing problem (ALCT0 BXN - L1A BXN) RMS: %.3f ( >%.2f )",z, limit);
                        dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_ALCT_TIMING"));
                      }
                  }
              }
          }
    }

// == Check for CLCT Timing issues
  me = findME("EMU", "CSC_CLCT0_BXN_rms",  sourcedir);
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      int csc_cntr=0;
      uint32_t min_events=400;
      double rms_limit = 2.56;
      // if (theFormatVersion == 2013) rms_limit = 1.91;
      for (int j=int(h->GetYaxis()->GetXmax())-1; j>= int(h->GetYaxis()->GetXmin()); j--)
        for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
          {
            std::string cscName = Form("%s/%02d", (emu::dqm::utils::getCSCTypeName(j)).c_str(), i);
            if (!deadCLCT[cscName])   ///* Don't check CLCT Timing if CLCT is dead on this chamber
              {
                double limit = rms_limit;
                if ((theFormatVersion == 2013) && emu::dqm::utils::isME42(cscName)) limit = rms_limit + 1.0; // Handle ME42 chambers, which have different timing pattern
                double z = h->GetBinContent(i, j+1);
                double avg = round(z*10.)/10.;
                if (avg > limit)
                  {
                    csc_cntr++;
                    uint32_t csc_events = csc_stats[cscName];
                    if (csc_events>min_events)
                      {
                        std::string diag=Form("CLCT Timing problem (CLCT0 BXN - L1A BXN) RMS: %.3f ( >%.2f )",z, limit);
                        dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_CLCT_TIMING"));
                      }
                  }
              }
          }
    }

  // == Check for chambers with L1A out of sync
  me = findME("EMU", "DMB_L1A_out_of_sync_Fract",  sourcedir);
  me2 = findME("EMU", "DMB_L1A_out_of_sync",  sourcedir);
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      TH2D* h2 = reinterpret_cast<TH2D*>(me2);
      int csc_cntr=0;
      uint32_t min_events=50;
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j <= int(h->GetYaxis()->GetXmax()); j++)
          {
            double z = h->GetBinContent(i, j);
            if (z>0)
              {
                csc_cntr++;
                uint32_t events = uint32_t(h2->GetBinContent(i, j));
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                uint32_t csc_events = csc_stats[cscName];
                float fract=round(z*100);
                DQM_SEVERITY severity=MINOR;
                if (csc_events>min_events)
                  {
                    if (fract >= 80.) severity=CRITICAL;
                    else if (fract >= 10.) severity=SEVERE;
                    else if (fract > 1.) severity=TOLERABLE;
                  }

                std::string diag=Form("L1A out of sync: %d events (%.3f%%)",events, z*100);

                dqm_report.addEntry(cscName, entry.fillEntry(diag,severity, "CSC_WITH_L1A_OUT_OF_SYNC"));

                MonitorElement* me3 = 0;

                // Prepare list of L1A histos
                std::vector<std::pair<std::string, std::string> > l1a_histos;
                for (int icfeb=0; icfeb<7; icfeb++)
                  {
                    l1a_histos.push_back(make_pair( Form("CFEB%d", icfeb+1 ), Form("CFEB%d_DMB_L1A_diff", icfeb) ));
                  }
                l1a_histos.push_back(make_pair( "ALCT","ALCT_DMB_L1A_diff") );
                l1a_histos.push_back(make_pair( "CLCT","CLCT_DMB_L1A_diff") );
                l1a_histos.push_back(make_pair( "DDU","DMB_DDU_L1A_diff") );

                // -- Find which board has L1A out of sync
                for (uint32_t k = 0; k < l1a_histos.size(); k++)
                  {
                    me3 = findME(cscTag, l1a_histos[k].second,  sourcedir);
                    if (me3)
                      {
                        TH1D* h3 = reinterpret_cast<TH1D*>(me3);
                        if (h3->GetMean() != 0)
                          {
                            diag=Form("L1A out of sync: %s",(l1a_histos[k].first).c_str());

                            dqm_report.addEntry(cscName, entry.fillEntry(diag, severity, "CSC_WITH_L1A_OUT_OF_SYNC"));
                          }
                      }

                  }

              }

          }
    }


  /// !!! TODO: Format version detection
  /// For now force 2013
  // short theFormatVersion = 2013;

  // == Perform per Chamber Checks
  for (uint32_t i=0; i<CSC_folders.size(); i++)
    {
      int crate=0, slot =0;
      uint32_t min_events = 200;
      double cfeb_hot_thresh = 60.;
      //    std::cout << getCSCName(CSC_folders[i], crate, slot, CSCtype, CSCposition) << std::endl;
      std::string cscName = getCSCName(CSC_folders[i], crate, slot, CSCtype, CSCposition);
      int nCFEBs = emu::dqm::utils::getNumCFEBs(cscName, theFormatVersion);
      bool ME11 = emu::dqm::utils::isME11(cscName);
      bool isBeam = false; // Assume that this is Cosmic run and not Beam
      bool hasHotCFEB = false; // Check if one of the CFEBs is hot (DAV > cfeb_hot_thresh)

      // int nStrips = getNumStrips(cscName);
      int nWireGroups = emu::dqm::utils::getNumWireGroups(cscName);
      std::vector<int> deadCFEBs(nCFEBs,0);
      std::vector<int> lowEffCFEBs(nCFEBs,0);
      std::vector<int> badCFEBs(nCFEBs,0);
      int nbadCFEBs = 0;

      // int deadALCT=0;
      if (csc_stats[cscName] < min_events)
        {
          std::string diag=Form("Not enough events for per-chamber checks (%d events, needs > %d)", csc_stats[cscName], min_events);
          dqm_report.addEntry(cscName, entry.fillEntry(diag,NONE));
          continue;
        }
      // -- Anode Occupancies and HV Segments Checks
      std::vector< std::pair<int,int> > hvSegMap = emu::dqm::utils::getHVSegmentsMap(cscName);
      double low_hvseg_thresh = 0.15;
      double low_anode_thresh = 0.10;
      double high_anode_thresh = 10.;
      std::map<int, vector<int> > no_hv_segments;
      std::map<int, vector<int> > noisy_hv_segments;
      std::map<int, vector<double> > hvsegs;
      std::map<int, vector<double> > afebs;
      if (!deadALCT[cscName])
        {
          for (int ilayer=1; ilayer<=6; ilayer++)
            {
              std::string name = Form("ALCT_Ly%d_Efficiency",ilayer);

              me = findME(CSC_folders[i], name , sourcedir);
              if (me)
                {
                  TH1D* h = reinterpret_cast<TH1D*>(me);
                  if (h == NULL) continue;
                  double ent = h->GetEntries();

                  // For ME11 if occupancy histo is empty then HV segment is OFF
                  if ( ME11 && ((int)csc_stats[cscName] > 5*nWireGroups) && (ent==0) && !deadALCT[cscName])
                    {
                      std::string diag=Form("No HV at Segment%d Layer%d", 1, ilayer );
                      DQM_SEVERITY severity = TOLERABLE;
                      if (ME11) severity = SEVERE; // Raise severity for ME11 chamber
                      dqm_report.addEntry(cscName, entry.fillEntry(diag,severity, "CSC_NO_HV_SEGMENT"));

                    }

                  if ( ent > 20*nWireGroups)
                    {

                      double avg_anode_occup = 0;
                      double anode_max = 0;
                      for (uint32_t hvseg=0; hvseg < hvSegMap.size(); hvseg++)
                        {
                          double val = (h->Integral(hvSegMap[hvseg].first, hvSegMap[hvseg].second)/((hvSegMap[hvseg].second-hvSegMap[hvseg].first+1)*ent))*100.;
                          hvsegs[ilayer].push_back(val);
                          if (val>anode_max) anode_max=val;
                          // std::cout << cscName << " ly" << ilayer << " ent:" <<   ent << " hvseg" << hvseg << " " << val << " sum " << h->Integral() << std::endl;
                        }

                      TH1D* h_tmp = new TH1D("temp", "temp", 1000, 0, anode_max+1);
                      for (uint32_t hvseg=0; hvseg < hvSegMap.size(); hvseg++)
                        {
                          h_tmp->Fill(hvsegs[ilayer][hvseg]);
                        }
                      avg_anode_occup = h_tmp->GetMean();
                      delete h_tmp;
                      for (uint32_t hvseg=0; hvseg < hvSegMap.size(); hvseg++)
                        {
                          double z=hvsegs[ilayer][hvseg];
                          double avg = round(z*100.)/100.;
                          if (avg < low_hvseg_thresh )
                            {
                              std::string diag=Form("No HV at Segment%d Layer%d (occupancy %% %.3f < %.2f threshold)", hvseg+1, ilayer, z, low_hvseg_thresh );
                              DQM_SEVERITY severity = TOLERABLE;
                              if (ME11) severity = SEVERE; // Raise severity for ME11 chamber
                              dqm_report.addEntry(cscName, entry.fillEntry(diag,severity, "CSC_NO_HV_SEGMENT"));
                              no_hv_segments[ilayer].push_back(hvseg);
                            }
                        }
                      anode_max = 0;

                      for (int32_t iseg=0; iseg < nWireGroups/8; iseg++)
                        {
                          double val = (h->Integral(iseg*8+1, (iseg+1)*8)/(8*ent))*100;
                          afebs[ilayer].push_back(val);
                          if (val>anode_max) anode_max=val;
                          // std::cout << cscName << " ly" << ilayer << " ent:" <<   ent << " seg" << iseg << " " << val << std::endl;
                        }

                      h_tmp = new TH1D("temp", "temp", 1000, 0, anode_max+1);
                      for (int32_t iseg=0; iseg < nWireGroups/8; iseg++)
                        {
                          h_tmp->Fill(afebs[ilayer][iseg]);
                        }
                      avg_anode_occup = h_tmp->GetMean();
                      delete h_tmp;
                      for (int32_t iseg=0; iseg < nWireGroups/8; iseg++)
                        {
                          double z=afebs[ilayer][iseg];
                          int hvseg = emu::dqm::utils::getHVSegmentNumber(cscName, iseg);
                          int afeb = iseg*3+(ilayer+1)/2;

                          if ( (z < low_anode_thresh)
                               && (find(no_hv_segments[ilayer].begin(), no_hv_segments[ilayer].end(), hvseg ) == no_hv_segments[ilayer].end())
                               && (find(noisy_hv_segments[ilayer].begin(), noisy_hv_segments[ilayer].end(), hvseg ) == noisy_hv_segments[ilayer].end()))
                            {

                              if (z==0)
                                {
                                  std::string diag=Form("ALCT No Anode Data: AFEB%d Layer%d", afeb, ilayer);
                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_ALCT_NO_ANODE_DATA"));
                                }
                              else
                                {
                                  std::string diag=Form("ALCT Low Anode Efficiency: AFEB%d Layer%d (occupancy %% %.3f < %.2f threshold) ", afeb, ilayer, z, low_anode_thresh);
                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_ALCT_NO_ANODE_DATA"));
                                }

                            }
                          else if (z > high_anode_thresh)
                            {
                              std::string diag=Form("Noisy Anodes Segment: AFEB%d Layer%d (occupancy %% %.3f > %.2f threshold)", afeb, ilayer, z, high_anode_thresh );
                              // !!!! Change to different test ID !!!!
                              dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_ALCT_AFEB_NOISY"));
                            }

                        }

                    }
                }
            }
        }

      me = findME(CSC_folders[i], "ALCT0_KeyWG",  sourcedir);
      if (me && (nWireGroups>0))
        {
          float hot_thresh = 7.0;
          TH1D* h = reinterpret_cast<TH1D*>(me);
          int entries = h->GetEntries();
          double avg_alct_occupancy = (h->Integral(0, nWireGroups)/nWireGroups);
          if ((avg_alct_occupancy > 0) && (entries > nWireGroups*100))
            {
              for (int wg=0; wg<nWireGroups; wg++)
                {
                  double val = h->GetBinContent(wg);
                  if (val >= hot_thresh*avg_alct_occupancy)
                    {
                      std::string diag=Form("Hot ALCT Key WireGroup %d (occupancy %.1f > %.1f average). Possible ALCT SEU.", wg+1, val/avg_alct_occupancy, hot_thresh );
                      dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_ALCT_HOT_KEYWG"));
                    }
                }
            }
        }




      // -- CFEBs DAV checks
      me = findME(CSC_folders[i], "Actual_DMB_CFEB_DAV_Frequency",  sourcedir);
      me2 = findME(CSC_folders[i], "Actual_DMB_CFEB_DAV_Rate",  sourcedir);
      if (me && me2)
        {
          TH1D* h = reinterpret_cast<TH1D*>(me);
          unsigned int entries = h->GetEntries();

          vector<double> cfebs;
          if  ( entries > min_events)
            {

              double avg_cfeb_occup = 0;
              TH1D* h_tmp = new TH1D("temp", "temp", 100, h->GetMinimum(), h->GetMaximum()+1);
              double val=0;


              for (int icfeb=0; icfeb<nCFEBs; icfeb++)
                {
                  val = h->GetBinContent(icfeb+1);
                  cfebs.push_back(val);
                  if (val>cfeb_hot_thresh)
                    {
                      hasHotCFEB = true;
                      continue; // dont count hot CFEBs
                    }
                  if (!ME11 || (ME11 && icfeb<4) ) h_tmp->Fill(val);

                }
              avg_cfeb_occup = round(h_tmp->GetMean());

              delete h_tmp;


              if (theFormatVersion != 2013)
                {
                  // Try to detect Beam run using ME11 CFEB DAV pattern (CFEB5 should ~ 2.5 times higher than average)
                  if (ME11 && ((avg_cfeb_occup*2.5)<cfebs[4])) isBeam = true;
                }
              else
                {
                  // Try to detect Beam run using ME11 DCFEB DAV pattern
                  if (ME11)
                    {
                      int tcnt=0;
                      double tsum=0.;
                      for (int i=4; i<7; i++)
                        {
                          if ((cfebs[i]>0) && (cfebs[i] < cfeb_hot_thresh))
                            {
                              tsum+=cfebs[i];
                              tcnt++;
                            }
                        }
                      if (tcnt && (((tsum/tcnt)/avg_cfeb_occup)>0.75) && (avg_cfeb_occup > 6.0))
                        {
                          // if (!isBeam) LOG4CPLUS_INFO(logger, cscName << " --> Run2 beam collisions data detected");
                          isBeam = true;
                        }
                    }
                }

              for (int icfeb=0; icfeb<nCFEBs; icfeb++)
                {

                  double z=cfebs[icfeb];

                  if (z > cfeb_hot_thresh) /// Check for hot CFEB
                    {
                      std::string diag=Form("CFEB Hot: CFEB%d DAV %.3f%%", icfeb+1, z);
                      deadCFEBs[icfeb]=1;    // Mark this CFEB as dead
                      dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_HOT_CFEB_DAV_EFF"));
                      badCFEBs[icfeb]=1;
                      nbadCFEBs ++;
                    }

                  if (hasHotCFEB) continue; // Dont check if have Hot CFEB

                  if (z==0)
                    {
                      std::string diag=Form("CFEB Dead: CFEB%d DAV %.3f%%", icfeb+1, z);
                      deadCFEBs[icfeb]=1;    // Mark this CFEB as dead
                      dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_WITH_LOW_CFEB_DAV_EFF"));
                      badCFEBs[icfeb]=1;
                      nbadCFEBs ++;
                    }

                  else
                    {
                      if (theFormatVersion != 2013)
                        {
                          if (!isBeam || (isBeam && !ME11 && icfeb!=4)) // Usual occupancy check algorithm for cosmic runs
                            {
                              if ((round(z) < 5.) || (round(z) < 0.4*avg_cfeb_occup))
                                {
                                  std::string diag=Form("CFEB Low efficiency: CFEB%d DAV %.3f%%", icfeb+1, z);
                                  lowEffCFEBs[icfeb]=1;
                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_WITH_LOW_CFEB_DAV_EFF"));
                                  badCFEBs[icfeb]=1;
                                  nbadCFEBs ++;
                                }
                            }
                          else if (ME11 && icfeb == 4)
                            {
                              if (round(z) < avg_cfeb_occup*2)
                                {
                                  std::string diag=Form("CFEB Low efficiency: CFEB%d DAV %.3f%%", icfeb+1, z);
                                  lowEffCFEBs[icfeb]=1;
                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_WITH_LOW_CFEB_DAV_EFF"));
                                  badCFEBs[icfeb]=1;
                                  nbadCFEBs ++;
                                }
                            }
                        }
                      else     // New DCFEBs
                        {
                          if (!ME11 || (isBeam && ME11))  // Usual occupancy check algorithm for
                            {
                              if ((round(z) < 5.) || (round(z) < 0.4*avg_cfeb_occup))
                                {
                                  std::string diag=Form("CFEB Low efficiency: CFEB%d DAV %.3f%%", icfeb+1, z);
                                  lowEffCFEBs[icfeb]=1;
                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_WITH_LOW_CFEB_DAV_EFF"));
                                  badCFEBs[icfeb]=1;
                                  nbadCFEBs ++;
                                }
                            }
                          else if (!isBeam && ME11 && (icfeb <4))
                            {
                              if ((round(z) < 3.) || (round(z) < 0.4*avg_cfeb_occup))
                                {
                                  std::string diag=Form("CFEB Low efficiency: CFEB%d DAV %.3f%%", icfeb+1, z);
                                  lowEffCFEBs[icfeb]=1;
                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_WITH_LOW_CFEB_DAV_EFF"));
                                  badCFEBs[icfeb]=1;
                                  nbadCFEBs ++;
                                }
                            }
                          else if (!isBeam && ME11 && icfeb >= 4)
                            {
                              if ((z < 0.91) || round(z) < 0.05*avg_cfeb_occup)
                                {
                                  std::string diag=Form("CFEB Low efficiency: CFEB%d DAV %.3f%%", icfeb+1, z);
                                  lowEffCFEBs[icfeb]=1;
                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_WITH_LOW_CFEB_DAV_EFF"));
                                  badCFEBs[icfeb]=1;
                                  nbadCFEBs ++;
                                }
                            }
                        }

                    }

                }
            }
        }

      int nActiveCFEBs = nCFEBs-nbadCFEBs;

      // List of layers with lowered HV for ME11
      std::vector<bool> loweredHVsegment(6,false);

      if  ((nActiveCFEBs > 0) && !hasHotCFEB)
        {
          // Expecting active CFEBs

          double avgSCAlayer=0;
          std::vector<double> layerSCAsums;

          // -- CFEB SCA Occupancies Checks
          for (int ilayer=1; ilayer<=6; ilayer++)
            {
              std::string name = Form("CFEB_ActiveStrips_Ly%d",ilayer);

              me = findME(CSC_folders[i], name , sourcedir);
              if (me)
                {
                  TH1D* h = reinterpret_cast<TH1D*>(me);
                  int nentries = (int)h->GetEntries();
                  double allSCAsum = h->Integral();
                  layerSCAsums.push_back(allSCAsum);
                  avgSCAlayer += allSCAsum;
                  std::vector<double> SCAsums;
                  SCAsums.clear();
                  int noSCAs = 0;
                  double low_sca_thresh = 0.2; // !!! Rasing default theshold from 0.2 to 0.55 to be able to detect cable swap problems;
                  double high_sca_thresh = 3.0;

                  if ( nentries >= (10*16*nActiveCFEBs) )
                    {
                      // -- Check for dead SCAs CFEBs
                      for (int icfeb=0; icfeb<nCFEBs; icfeb++)
                        {

                          if (badCFEBs[icfeb] == 1)
                            {
                              SCAsums.push_back(0.0);
                              continue; // Skip already known bad CFEBs
                            }

                          double cfeb_sca_sum = h->Integral(icfeb*16+1, (icfeb+1)*16);
                          SCAsums.push_back(cfeb_sca_sum);

                          if (cfeb_sca_sum == 0)
                            {
                              std::string diag=Form("CFEB No SCA Data: CFEB%d Layer%d", icfeb+1, ilayer);
                              dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE,"CSC_CFEB_NO_SCA_DATA"));
                              noSCAs++;
                            }
                        }
                    }
                  else
                    {
                      if ( (nentries == 0) && ((int)csc_stats[cscName] >= (5*16*nActiveCFEBs)))
                        for (int icfeb=0; icfeb<nCFEBs; icfeb++)
                          {
                            noSCAs++;
                            if (ME11 && (no_hv_segments[ilayer].size() > 0)) continue; // Skip reporting for ME11 with no HV segment
                            std::string diag=Form("CFEB No SCA Data: CFEB%d Layer%d", icfeb+1, ilayer);
                            dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE,"CSC_CFEB_NO_SCA_DATA"));
                          }
                    }


                  if ( nentries >= (25*16*nActiveCFEBs) )
                    {
                      if (nActiveCFEBs - noSCAs > 0)
                        {
                          // Check that still active CFEBs present
                          double avg_sca_occupancy = allSCAsum/(nActiveCFEBs-noSCAs);
                          double avg_sca_ch_occupancy = (avg_sca_occupancy/(16*nentries))*100;
                          for (int icfeb=0; icfeb < nCFEBs; icfeb++)
                            {
                              // Avg. strip SCA occupancy > 5.
                              bool isLowEff = false;
                              double cfeb_sca_sum = SCAsums[icfeb];

                              if (theFormatVersion != 2013)
                                {
                                  if ( (icfeb == 4) && ME11) cfeb_sca_sum/=2;
                                }
                              else
                                {
                                  if (ME11 && (icfeb>=4) && !isBeam)
                                    {
                                      cfeb_sca_sum*=3.5; // Correction for additional DCFEBs for cosmics
                                    }
                                }

                              if (cfeb_sca_sum)
                                {
                                  if (theFormatVersion != 2013)
                                    {

                                      if ( (cfeb_sca_sum < low_sca_thresh*avg_sca_occupancy) && (lowEffCFEBs[icfeb] != 1))
                                        {
                                          if (cfeb_sca_sum < 0.01*avg_sca_occupancy)
                                            {
                                              std::string diag=Form("CFEB No SCA Data: CFEB%d Layer%d", icfeb+1, ilayer);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE,"CSC_CFEB_SCA_LOW_EFF"));
                                            }
                                          else
                                            {
                                              std::string diag=Form("CFEB Low SCA Efficiency: CFEB%d Layer%d (%.3f%% < %.1f%% from average)", icfeb+1, ilayer,
                                                                    (cfeb_sca_sum/avg_sca_occupancy)*100., low_sca_thresh*100 );
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag, TOLERABLE, "CSC_CFEB_SCA_LOW_EFF"));
                                            }
                                          isLowEff = true;
                                        }
                                    }
                                  else // FormatVersion2013
                                    {
                                      if ( (cfeb_sca_sum < low_sca_thresh*avg_sca_occupancy) && (lowEffCFEBs[icfeb] != 1))
                                        {
                                          if (cfeb_sca_sum < 0.01*avg_sca_occupancy)
                                            {
                                              std::string diag=Form("CFEB No SCA Data: CFEB%d Layer%d", icfeb+1, ilayer);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE,"CSC_CFEB_SCA_LOW_EFF"));
                                            }
                                          else
                                            {
                                              std::string diag=Form("CFEB Low SCA Efficiency: CFEB%d Layer%d (%.3f%% < %.1f%% from average)", icfeb+1, ilayer,
                                                                    (cfeb_sca_sum/avg_sca_occupancy)*100., low_sca_thresh*100 );
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag, TOLERABLE, "CSC_CFEB_SCA_LOW_EFF"));
                                            }
                                          isLowEff = true;
                                        }

                                    }

                                  if ( cfeb_sca_sum >= high_sca_thresh*avg_sca_occupancy
                                       && (isBeam || !ME11 || (ME11 && (icfeb<4))) )
                                    {
                                      std::string diag=Form("CFEB Noisy/Hot CFEB SCAs: CFEB%d Layer%d (%.1f > %.1f times from average)", icfeb+1, ilayer,
                                                            cfeb_sca_sum/avg_sca_occupancy, high_sca_thresh);
                                      dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_CFEB_SCA_NOISY"));
                                      // std::cout << cscName << " " << diag << std::endl;
                                    }

                                  double avg_strip_occupancy = cfeb_sca_sum/16;
                                  for (int ch=1; ch <=16; ch++)
                                    {
                                      double ch_occup = h->GetBinContent(ch+icfeb*16);
                                      double ch_val = 100*ch_occup/nentries;
                                      double ch_ratio = ch_occup / avg_strip_occupancy;

                                      if (theFormatVersion != 2013)
                                        {
                                          if ( (icfeb == 4) && ME11) ch_val/=2;
                                        }
                                      else
                                        {
                                          // Adjust for Post-LS1 conditions if necessary
                                        }


                                      if (ch_val > high_sca_thresh*avg_sca_ch_occupancy)
                                        {
                                          std::string diag = Form("CFEB Hot/Noisy SCA channel: CFEB%d Layer%d Ch#%d (occupancy %.1f times > average)",
                                                                  icfeb+1, ilayer, ch, ch_val/avg_sca_ch_occupancy);
                                          dqm_report.addEntry(cscName, entry.fillEntry(diag,MINOR, "CSC_CFEB_SCA_NOISY_CHANNEL"));
                                        }

                                      if (avg_strip_occupancy > 40)
                                        {
                                          double ch_thresh = 0.02;
                                          if ( ((ch == 1) && (icfeb == 0))
                                               || ((ch == 16) && (icfeb == nCFEBs-1)) ) ch_thresh = 0.0; /// First and Last strips have lower occupancya

                                          if (theFormatVersion == 2013)
                                            {
                                              /// ME11 DCFEB3 ch16 and DCFEB4 ch1 have lower occupancy
                                              if ( ME11 && ( ((ch==16) && (icfeb == 3)) || ((ch==1) && (icfeb == 4)) ) ) ch_thresh = 0.0;
                                            }

                                          if ((ch_ratio < ch_thresh) && !isLowEff && (lowEffCFEBs[icfeb] != 1))
                                            {
                                              std::string diag = Form("CFEB Dead SCA channel: CFEB%d Layer%d Ch#%d (occupancy %.3f < %.2f )", icfeb+1, ilayer, ch, ch_ratio, ch_thresh);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,MINOR, "CSC_CFEB_SCA_DEAD_CHANNEL"));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

          //* Try to detect lowered ME11 HV segment
          if (ME11 && isBeam && (csc_stats[cscName] > 50*16*7))
            {
              avgSCAlayer/=6;
              for (unsigned int i=0; i < layerSCAsums.size(); i++)
                {
                  double fract = layerSCAsums[i]/avgSCAlayer;
                  if ((fract > 0.) && fract < 0.8 )
                    {
                      std::string diag = Form("Lowered HV Segment: Layer%d (SCA efficiency %.2f of average)", i+1, fract);
                      dqm_report.addEntry(cscName, entry.fillEntry(diag,MINOR, "CSC_LOWERED_HV_SEGMENT"));
                      loweredHVsegment[i] = true;
                    }
                  if ((fract > 0.0) && fract < 0.1 )
                    {
                      std::string diag = Form("No HV Segment: Layer%d (SCA efficiency %.2f of average)", i+1, fract);
                      dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_NO_HV_SEGMENT"));
                      loweredHVsegment[i] = true;
                    }
                }
            }
        }

      if  ((nActiveCFEBs > 0) && !hasHotCFEB)
        {
          // Expecting active CFEBs
          // -- CFEB Comparators Occupancies Checks
          for (int ilayer=1; ilayer<=6; ilayer++)
            {
              std::string name = Form("CLCT_Ly%d_Rate",ilayer);
              me = findME(CSC_folders[i], name , sourcedir);
              if (me)
                {
                  TH1D* h = reinterpret_cast<TH1D*>(me);
                  int nentries = (int)h->GetEntries();
                  double allCompsum = 0; // h->Integral();
                  std::vector<double> Compsums;
                  Compsums.clear();
                  int noComps = 0;
                  double low_comp_thresh = 0.1;
                  double high_comp_thresh = 3.0;

                  if ( nentries >= (5*32*nActiveCFEBs) )
                    {
                      for (int icfeb=0; icfeb<nCFEBs; icfeb++)
                        {

                          if (badCFEBs[icfeb] ==1 )
                            {
                              Compsums.push_back(0.0);
                              continue;
                            }// Skip dead CFEBs

                          double cfeb_comp_sum = h->Integral(icfeb*32+1, (icfeb+1)*32);

                          if (theFormatVersion != 2013)
                            {
                              // Don't count ME11 CFEB5 occupancy during Beam
                              if (!isBeam || (isBeam && ME11 && (icfeb!=4) )) allCompsum += cfeb_comp_sum;
                            }
                          else
                            {
                              allCompsum += cfeb_comp_sum;
                            }


                          Compsums.push_back(cfeb_comp_sum);

                          if (cfeb_comp_sum == 0 && (lowEffCFEBs[icfeb] != 1))
                            {
                              std::string diag=Form("CFEB No Comparators Data: CFEB%d Layer%d", icfeb+1, ilayer);
                              dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_CFEB_NO_COMPARATORS_DATA"));
                              noComps++;
                            }
                          else
                            {
                              if (theFormatVersion != 2013)
                                {
                                  if (isBeam && ME11 && (icfeb==4)) noComps++; // Don't count ME11 CFEB5 occupancy during Beam
                                }
                            }

                        }
                    }


                  if ( nentries >= (25*32*nActiveCFEBs) )
                    {
                      if (nActiveCFEBs - noComps > 0)
                        {
                          // Check that still active CFEBs present
                          double avg_comp_occupancy = allCompsum/(nActiveCFEBs-noComps);
                          double avg_comp_ch_occupancy = avg_comp_occupancy/32;
                          for (int icfeb=0; icfeb < nCFEBs; icfeb++)
                            {
                              double avg_eff = (100*Compsums[icfeb])/(18.*csc_stats[cscName]);
                              double avg = round(avg_eff*100.)/100.;

                              if (Compsums[icfeb])
                                {
                                  if (theFormatVersion != 2013)
                                    {
                                      if ( !isBeam || (isBeam && ME11 && icfeb!=4) )
                                        {
                                          // Standard occupancy check logic for Cosmic run

                                          bool isDeadLowComps = false;
                                          if ( (avg < low_comp_thresh) && (lowEffCFEBs[icfeb] != 1) && (!loweredHVsegment[ilayer-1]) )
                                            {
                                              if (avg < 0.01)
                                                {
                                                  std::string diag=Form("CFEB No Comparators Data: CFEB%d Layer%d", icfeb+1, ilayer);
                                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_CFEB_COMPARATORS_LOW_EFF"));
                                                }
                                              else
                                                {
                                                  std::string diag=Form("CFEB Low Comparators Efficiency: CFEB%d Layer%d (%.3f%% < %.1f%% threshold)", icfeb+1, ilayer,
                                                                        avg, low_comp_thresh);
                                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_CFEB_COMPARATORS_LOW_EFF"));
                                                }
                                              isDeadLowComps = true;
                                            }

                                          bool isNoisyComps = false;
                                          if (avg >= high_comp_thresh)
                                            {
                                              std::string diag=Form("CFEB Hot/Noisy CFEB Comparators: CFEB%d Layer%d (%.1f > %.1f threshold)", icfeb+1, ilayer,
                                                                    avg_eff, high_comp_thresh);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_CFEB_COMPARATORS_NOISY"));
                                              isNoisyComps = true;
                                            }

                                          bool isNoisyCompsChan = false;
                                          for (int ch=1; ch <=32; ch++)
                                            {
                                              double ch_val = h->GetBinContent(ch+icfeb*32);
                                              if ( (ch_val > 8*avg_comp_ch_occupancy) && !isNoisyComps)
                                                {
                                                  std::string diag = Form("CFEB Hot/Noisy Comparator channel: CFEB%d Layer%d HStrip%d (occupancy %.1f times > average)",
                                                                          icfeb+1, ilayer, ch+icfeb*32,
                                                                          ch_val/avg_comp_ch_occupancy);
                                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_CFEB_COMPARATORS_NOISY_CHANNEL"));
                                                  isNoisyCompsChan = true;
                                                }
                                            }
                                          if (fCheckDeadComparatorsChannels)
                                            {
                                              bool isDeadCompsChan = false;
                                              for (int ch=1; ch <=32; ch++)
                                                {
                                                  double ch_val = h->GetBinContent(ch+icfeb*32);
                                                  if (( ch<=3 && icfeb==0 ) || ( ch>=30  && (icfeb==(nCFEBs-1)) )
                                                      || ( ME11 && ch<=3 && icfeb==4 ) || ( ME11 && ch>=30 && icfeb==3 )
                                                      || ( ch<=3 && icfeb>0 && (badCFEBs[icfeb-1]==1))
                                                      || ( ch>=30 && icfeb<(nCFEBs-1) && (badCFEBs[icfeb+1]==1)) ) continue;
                                                  if ( (nentries >= (40*32*nActiveCFEBs)) && (ch_val <= 0.05*avg_comp_ch_occupancy)
                                                       && !isDeadLowComps && !isNoisyComps && !isNoisyCompsChan )
                                                    {
                                                      std::string diag = Form("CFEB Dead Comparator channel: CFEB%d Layer%d Ch#%d HStrip%d",
                                                                              icfeb+1, ilayer, ch, ch+icfeb*32);
                                                      dqm_report.addEntry(cscName, entry.fillEntry(diag,MINOR, "CSC_CFEB_COMPARATORS_DEAD_CHANNEL"));
                                                      isDeadCompsChan = true;
                                                    }
                                                }
                                            }

                                        }
                                      else if (ME11 && icfeb==4)  // Occupancy check logic for ME11 CFEB5 with Beam run
                                        {
                                          double me11_cfeb5_low_comp_thresh = 1.0;
                                          double me11_cfeb5_high_comp_thresh = 5.;
                                          if ( (avg < me11_cfeb5_low_comp_thresh) && (lowEffCFEBs[icfeb] != 1) && (!loweredHVsegment[ilayer-1]) )
                                            {
                                              std::string diag=Form("CFEB Low Comparators Efficiency: CFEB%d Layer%d (%.3f%% < %.1f%% threshold)", icfeb+1, ilayer,
                                                                    avg, me11_cfeb5_low_comp_thresh);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_CFEB_COMPARATORS_LOW_EFF"));
                                            }

                                          if ( avg >= me11_cfeb5_high_comp_thresh )
                                            {
                                              std::string diag=Form("CFEB Hot/Noisy CFEB Comparators: CFEB%d Layer%d (%.1f%% > %.1f%% threshold)", icfeb+1, ilayer,
                                                                    avg, me11_cfeb5_high_comp_thresh);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_CFEB_COMPARATORS_NOISY"));
                                            }
                                        }
                                    }
                                  else // == Post-LS1
                                    {
                                      bool isDeadLowComps = false;
                                      if ( (avg < low_comp_thresh) && (lowEffCFEBs[icfeb] != 1) && (!loweredHVsegment[ilayer-1]))
                                        {
                                          if (avg < 0.01)
                                            {
                                              std::string diag=Form("CFEB No Comparators Data: CFEB%d Layer%d", icfeb+1, ilayer);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_CFEB_COMPARATORS_LOW_EFF"));
                                            }
                                          else
                                            {
                                              std::string diag=Form("CFEB Low Comparators Efficiency: CFEB%d Layer%d (%.3f%% < %.1f%% threshold)", icfeb+1, ilayer,
                                                                    avg, low_comp_thresh);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_CFEB_COMPARATORS_LOW_EFF"));
                                            }
                                          isDeadLowComps = true;
                                        }

                                      bool isNoisyComps = false;
                                      if ( avg >= high_comp_thresh)
                                        {
                                          std::string diag=Form("CFEB Hot/Noisy CFEB Comparators: CFEB%d Layer%d (%.1f > %.1f threshold)", icfeb+1, ilayer,
                                                                avg, high_comp_thresh);
                                          dqm_report.addEntry(cscName, entry.fillEntry(diag,SEVERE, "CSC_CFEB_COMPARATORS_NOISY"));
                                          isNoisyComps = true;
                                        }

                                      bool isNoisyCompsChan = false;
                                      for (int ch=1; ch <=32; ch++)
                                        {
                                          double ch_val = h->GetBinContent(ch+icfeb*32);
                                          if ( (ch_val > 8*avg_comp_ch_occupancy) && !isNoisyComps)
                                            {
                                              std::string diag = Form("CFEB Hot/Noisy Comparator channel: CFEB%d Layer%d Ch#%d HStrip%d (occupancy %.1f times > average)",
                                                                      icfeb+1, ilayer, ch, ch+icfeb*32,
                                                                      ch_val/avg_comp_ch_occupancy);
                                              dqm_report.addEntry(cscName, entry.fillEntry(diag,TOLERABLE, "CSC_CFEB_COMPARATORS_NOISY_CHANNEL"));
                                              isNoisyCompsChan = true;
                                            }
                                        }
                                      if (fCheckDeadComparatorsChannels)
                                        {
                                          bool isDeadCompsChan = false;
                                          for (int ch=1; ch <=32; ch++)
                                            {
                                              double ch_val = h->GetBinContent(ch+icfeb*32);
                                              if ( ( ch<=3 && icfeb==0 ) || ( ch>=30  && icfeb==(nCFEBs-1) )
                                                   || ( ME11 && ch<=3 && icfeb==4 ) || ( ME11 && ch>=30 && icfeb==3 )
                                                   || ( ch<=3 && icfeb>0 && (badCFEBs[icfeb-1]==1))
                                                   || ( ch>=30 && icfeb<(nCFEBs-1) && (badCFEBs[icfeb+1]==1)) ) continue;
                                              if ( (nentries >= (40*32*nActiveCFEBs)) && (ch_val <= 0.05*avg_comp_ch_occupancy)
                                                   && !isDeadLowComps && !isNoisyComps && !isNoisyCompsChan)
                                                {
                                                  std::string diag = Form("CFEB Dead Comparator channel: CFEB%d Layer%d Ch#%d HStrip%d",
                                                                          icfeb+1, ilayer, ch, ch+icfeb*32);
                                                  dqm_report.addEntry(cscName, entry.fillEntry(diag,MINOR, "CSC_CFEB_COMPARATORS_DEAD_CHANNEL"));
                                                  isDeadCompsChan = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        } // expecting active CFEBs
    }

// == Check for chambers with format warnings (CFEB B-Words)
  me = findME("EMU", "DMB_Format_Warnings_Fract",  sourcedir);
  me2 = findME("EMU", "DMB_Format_Warnings",  sourcedir);
  if (me)
    {
      TH2D* h = reinterpret_cast<TH2D*>(me);
      TH2D* h2 = reinterpret_cast<TH2D*>(me2);
      int csc_cntr=0;
      for (int i=int(h->GetXaxis()->GetXmin()); i<= int(h->GetXaxis()->GetXmax()); i++)
        for (int j=int(h->GetYaxis()->GetXmin()); j <= int(h->GetYaxis()->GetXmax()); j++)
          {
            double z = h->GetBinContent(i, j);
            if (z>0)
              {
                csc_cntr++;
                uint32_t events = uint32_t(h2->GetBinContent(i, j));
                std::string cscTag(Form("CSC_%03d_%02d", i, j));
                std::string cscName=getCSCFromMap(i,j, CSCtype, CSCposition );
                float fract=z*100;
                DQM_SEVERITY severity=NONE;
                if (events > 25)
                  {
                    if (fract >= 80.) severity=CRITICAL;
                    else if (fract >= 20.) severity=SEVERE;
                    else if (fract > 5.) severity=TOLERABLE;
                    else if (fract > 0.5) severity=MINOR;
                  }

                std::string diag=Form("CFEB B-Words: %d events (%.3f%%)",events, z*100);
                dqm_report.addEntry(cscName, entry.fillEntry(diag, severity, "CSC_WITH_BWORDS"));
              }

          }
    }

  int csc_w_problems=0;
  T_DQMReport& report = dqm_report.getReport();
  T_DQMReport::iterator itr;
  for (itr = report.begin(); itr != report.end(); ++itr)
    {
      if (itr->first.find("ME") == 0)
        {
          csc_w_problems++;
        }
    }

//  showReport();
  appBSem_.give();
  return 0;
}










