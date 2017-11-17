#include "emu/step/PeriodicDumper.h"

#include "toolbox/task/TimerFactory.h"

#include "emu/pc/Crate.h"
#include "emu/pc/Chamber.h"
#include "emu/utils/System.h"

#include <iomanip>
#include <fstream>
#include <sstream>

using namespace std;

emu::step::PeriodicDumper::PeriodicDumper( emu::step::TimingOptions* options )
  : emu::step::Dumper( options )
  , timer_( toolbox::task::getTimerFactory()->createTimer( options_->dumperName() ) ){
  if ( ! options_->doPeriodicDumps() ) return; // Do nothing if not periodic dump is requested.
  timer_->addExceptionListener( dynamic_cast<toolbox::exception::Listener*>( options_->test() ) );
  toolbox::TimeInterval interval( options_->dumpPeriodInSec() );
  toolbox::TimeVal start = toolbox::TimeVal::gettimeofday(); // Start right away.
  resetCounters( NULL );
  timer_->scheduleAtFixedRate( start, this, interval, NULL, "" );
}

emu::step::PeriodicDumper::~PeriodicDumper(){
  toolbox::task::getTimerFactory()->removeTimer( options_->dumperName() );
}

string emu::step::PeriodicDumper::dumpHeader(){
  ostringstream header;
  header << "#################################################################################\n"
	 << "Time:       " << emu::utils::getDateTime( false )
	 << "\nPeriod [s]: " << options_->dumpPeriodInSec()
	 << "\n";
  return header.str();
}

void emu::step::PeriodicDumper::timeExpired( toolbox::task::TimerEvent& event ){
  dumpCounters( NULL );
  resetCounters( NULL );
}
