#ifndef __emu_step_PeriodicDumper_h__
#define __emu_step_PeriodicDumper_h__

#include "emu/step/Dumper.h"

#include "emu/step/TimingOptions.h"
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
    class PeriodicDumper : public emu::step::Dumper, public toolbox::task::TimerListener{
      ///
      /// Reset the counters in the given TMBs, wait \c interval seconds, and read them out. Repeat.
      ///

    public:
      PeriodicDumper( emu::step::TimingOptions* options );
      ~PeriodicDumper();
    private:
      string dumpHeader();
      void timeExpired( toolbox::task::TimerEvent& event );
      toolbox::task::Timer  *timer_;
    };
  }
}

#endif
