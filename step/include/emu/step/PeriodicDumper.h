#ifndef __emu_step_PeriodicDumper_h__
#define __emu_step_PeriodicDumper_h__

#include "emu/step/Test.h"
#include "emu/pc/TMB.h"

#include "toolbox/task/Timer.h"
#include "toolbox/task/TimerListener.h"
#include "toolbox/TimeInterval.h"
#include "log4cplus/logger.h"

#include <string>
#include <vector>

using namespace std;

namespace emu{
  namespace step{
    class PeriodicDumper : public toolbox::task::TimerListener{
      ///
      /// Reset the counters in the given TMBs, wait \c interval seconds, and read them out. Repeat.
      ///

    public:
      PeriodicDumper( emu::step::Test* parent, 
		      const string&          name, 
		      const string&          directory, 
		      const double           interval, 
		      vector<emu::pc::TMB*>& tmbs,
		      Logger*                pLogger );
      ~PeriodicDumper();
    private:
      void dumpCounters();
      void resetCounters();
      void timeExpired( toolbox::task::TimerEvent& event );
      string                 name_;
      string                 directory_;
      toolbox::TimeInterval  interval_;
      toolbox::task::Timer  *timer_;
      vector<emu::pc::TMB*>  tmbs_;
    Logger                  *pLogger_;

    };
  }
}

#endif
