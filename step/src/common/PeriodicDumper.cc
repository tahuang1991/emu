#include "emu/step/PeriodicDumper.h"

#include "toolbox/task/TimerFactory.h"

#include "emu/pc/Crate.h"
#include "emu/pc/Chamber.h"
#include "emu/utils/System.h"

#include <iomanip>
#include <fstream>

using namespace std;

emu::step::PeriodicDumper::PeriodicDumper(  emu::step::Test* parent,
					    const string&          name, 
					    const string&          directory, 
					    const double           interval, 
					    vector<emu::pc::TMB*>& tmbs,
					    Logger*                pLogger )
  : name_( name )
  , directory_( directory )
  , interval_( interval )
  , timer_( toolbox::task::getTimerFactory()->createTimer( name_ ) )
  , tmbs_( tmbs )
  , pLogger_( pLogger ){
  if ( interval <= 0. ) return; // Do nothing if the interval makes no sense.
  timer_->addExceptionListener( dynamic_cast<toolbox::exception::Listener*>( parent ) );
  toolbox::TimeVal start = toolbox::TimeVal::gettimeofday(); // Start right away.
  resetCounters();
  timer_->scheduleAtFixedRate( start, this, interval_, NULL, "" );
}

emu::step::PeriodicDumper::~PeriodicDumper(){
  toolbox::task::getTimerFactory()->removeTimer( name_ );
}

void emu::step::PeriodicDumper::timeExpired( toolbox::task::TimerEvent& event ){
  dumpCounters();
  resetCounters();
}

void emu::step::PeriodicDumper::dumpCounters(){
  ostringstream counterDump;
  counterDump << "#################################################################################\n"
	      << "Time:       " << emu::utils::getDateTime( false )
	      << "\nPeriod [s]: " << interval_ 
	      << "\n";
  for ( vector<emu::pc::TMB*>::iterator tmb = tmbs_.begin(); tmb != tmbs_.end(); ++tmb ){
    counterDump << "=================================================================================\n"
		<< "Crate:   "   << (*tmb)->getCrate()->CrateID()
		<< "\nSlot:    " << (*tmb)->slot()
		<< "\nChamber: " << (*tmb)->getChamber()->GetLabel()
		<< "\n";
    (*tmb)->RedirectOutput( &counterDump );
    (*tmb)->GetCounters();
    (*tmb)->PrintCounters();
    (*tmb)->RedirectOutput( &cout );
  }
  string fileName( directory_ + "/TMBCounters_" + name_ + ".txt" );
  ofstream outFile( fileName.c_str(), ofstream::out | ofstream::app );
  outFile << counterDump.str();
  outFile.close();
  if ( pLogger_ ){ 
    LOG4CPLUS_INFO( *pLogger_, "TMB dumper " << name_ << " wrote counters of " << tmbs_.size() << " TMBs to " << fileName ); 
  }
}

void emu::step::PeriodicDumper::resetCounters(){
  for ( vector<emu::pc::TMB*>::iterator tmb = tmbs_.begin(); tmb != tmbs_.end(); ++tmb ){
    (*tmb)->ResetCounters();
  }
}
