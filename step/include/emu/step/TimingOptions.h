#ifndef __emu_step_TimingOptions_h__
#define __emu_step_TimingOptions_h__

#include "emu/step/Test.h"

#include "emu/pc/TMB.h"

#include <vector>
#include <map>
#include <string>

namespace emu{
  namespace step{

    class Test;

    class TimingOptions{
    public:
      TimingOptions( emu::step::Test *test );
      emu::step::Test* test          ()       { return test_;                     }
      bool   doPeriodicDumps         () const { return doPeriodicDumps_;          }
      bool   doSynchronizedDumps     () const { return doSynchronizedDumps_;	  }
      bool   synchronizeTestStart    () const { return synchronizeTestStart_;	  }
      double dumpPeriodInSec         () const { return dumpPeriodInSec_;	  }
      int    spsTimingSignalsForDumps() const { return spsTimingSignalsForDumps_; }
      int    spsTimingSignalsForStart() const { return spsTimingSignalsForStart_; }
      vector<emu::pc::TMB*> TMBs     () const { return tmbs_;                     }
      string dumpDir                 () const { return dumpDir_;                  }
      string dumperName              () const { return dumperName_;               }
#ifdef DIP
      string printTimingSignals( unsigned int signals );
#endif
      
    private:
      emu::step::Test *test_;
      bool doPeriodicDumps_; /// TRUE if dumps are to be done at a predefined rate
      bool doSynchronizedDumps_; /// TRUE if dumps are to be synchronized to an SPS timing signal
      bool synchronizeTestStart_; /// TRUE if the start of test is to coincide with an SPS timing signal
      double dumpPeriodInSec_; /// If dumps are to be done at a predefined rate, this should be their period in seconds.
      unsigned int spsTimingSignalsForDumps_; /// SPS timing signals that the dumps should be done at (see emu::step::Test::SPSTimingSignal_t for bit positions)
      int spsTimingSignalsForStart_; /// SPS timing signals that the test should start at (see emu::step::Test::SPSTimingSignal_t for bit positions)
      vector<emu::pc::TMB*> tmbs_; /// TMBs whose counters to dump.
      string dumpDir_; /// Name of directory to dump the TMB counters to.
      string dumperName_; /// Name of the dumper.
    };
  }
}
#endif
