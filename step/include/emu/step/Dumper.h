#ifndef __emu_step_Dumper_h__
#define __emu_step_Dumper_h__

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
    class Dumper{
      ///
      /// Reset the counters in the given TMBs, and read them out.
      ///

    public:
      Dumper( emu::step::TimingOptions* options );
      virtual ~Dumper(){}
      void   dumpCounters( const char* signalName );
      void   resetCounters( const char* signalName );
    protected:
      virtual string dumpHeader();
      unsigned long long timeNowInMicrosec();
      unsigned long long timeOfReset_;
      string resetOnSignal_;
      emu::step::TimingOptions* options_;
    };
  }
}

#endif
