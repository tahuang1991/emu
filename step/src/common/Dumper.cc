#include "emu/step/Dumper.h"

#include "toolbox/task/TimerFactory.h"

#include "emu/pc/Crate.h"
#include "emu/pc/Chamber.h"
#include "emu/utils/System.h"

#include <sys/time.h>
#include <iomanip>
#include <fstream>

using namespace std;

emu::step::Dumper::Dumper( emu::step::TimingOptions* options )
  : options_( options ){
  LOG4CPLUS_INFO( *options_->test()->getLogger(), "In Dumper:  options_->TMBs()[0] is in " << options_->TMBs()[0]->slot() );
}

string emu::step::Dumper::dumpHeader(){
  ostringstream header;
#ifdef DIP
  header << "#################################################################################\n"
	 << "Time:          "    << emu::utils::getDateTime( false )
	 << "\nSynchronized to " << options_->printTimingSignals( options_->spsTimingSignalsForDumps() )
	 << "\n";
#endif
  return header.str();
}

unsigned long long emu::step::Dumper::timeNowInMicrosec(){
  struct timeval now;
  gettimeofday( &now, 0 );
  return (unsigned long long)now.tv_usec + (unsigned long long)now.tv_sec * 1000000;
}

void emu::step::Dumper::dumpCounters( const char* signalName ){
  ostringstream counterDump;
  counterDump << dumpHeader();
  counterDump << "Start at " << timeOfReset_;        
  if ( signalName ) counterDump << " us on " << resetOnSignal_;
  counterDump << "\n";
  counterDump << "End   at " << timeNowInMicrosec();
  if ( signalName ) counterDump << " us on " << signalName;
  counterDump << "\n";
  vector<emu::pc::TMB*> tmbs( options_->TMBs() );
  for ( vector<emu::pc::TMB*>::iterator tmb = tmbs.begin(); tmb != tmbs.end(); ++tmb ){
    counterDump << "=================================================================================\n"
		<< "Crate:   "   << (*tmb)->getCrate()->CrateID()
		<< "\nSlot:    " << (*tmb)->slot()
		<< "\nChamber: " << (*tmb)->getChamber()->GetLabel()
		<< "\n";
    if ( ! options_->test()->isFake() ){
      (*tmb)->RedirectOutput( &counterDump );
      (*tmb)->GetCounters();
      (*tmb)->PrintCounters();
      (*tmb)->RedirectOutput( &cout );
    }
  }
  string fileName( options_->dumpDir() + "/TMBCounters_" + options_->dumperName() + ".txt" );
  ofstream outFile( fileName.c_str(), ofstream::out | ofstream::app );
  outFile << counterDump.str();
  outFile.close();
  LOG4CPLUS_INFO( *options_->test()->getLogger(), "TMB dumper " << options_->dumperName() << " wrote counters of " << tmbs.size() << " TMBs to " << fileName );
}

void emu::step::Dumper::resetCounters( const char* signalName ){
  if ( signalName ) resetOnSignal_ = signalName;
  timeOfReset_ = timeNowInMicrosec();
  vector<emu::pc::TMB*> tmbs( options_->TMBs() );
  for ( vector<emu::pc::TMB*>::iterator tmb = tmbs.begin(); tmb != tmbs.end(); ++tmb ){
    if ( ! options_->test()->isFake() ) (*tmb)->ResetCounters();
  }
}
