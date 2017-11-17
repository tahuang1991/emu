#include "emu/step/TimingOptions.h"

#include "emu/pc/Crate.h"

#include "emu/utils/IO.h"

#include "log4cplus/loggingmacros.h"

emu::step::TimingOptions::TimingOptions( emu::step::Test *test )
 : test_                    ( test )
 , doPeriodicDumps_         ( false )        
 , doSynchronizedDumps_     ( false )
 , synchronizeTestStart_    ( false )    
 , dumpPeriodInSec_         ( 0 )
 , spsTimingSignalsForDumps_( 0 )
 , spsTimingSignalsForStart_( 0 )
 , dumpDir_                 ( "/tmp" )
{
  // Get optional TMB counter dump period
  dumpPeriodInSec_ = ( test_->parameters_.find( "tmb_counter_dump_period_ms" ) == test_->parameters_.end() ? double( 0. ) : double( test_->parameters_[ "tmb_counter_dump_period_ms" ] ) * 0.001 );
  if ( dumpPeriodInSec_ > 0. ) doPeriodicDumps_ = true;

  // Get optional SPS timing signal for TMB counter dump
  spsTimingSignalsForDumps_ = ( test_->parameters_.find( "tmb_counter_dump_synched_to" ) == test_->parameters_.end() ? 0 : test_->parameters_[ "tmb_counter_dump_synched_to" ] );

#ifdef DIP
  if ( spsTimingSignalsForDumps_ > 0 ){
    if ( doPeriodicDumps_ ){
      ostringstream oss;
      oss << "TMB counters can be dumped either periodically or synch'd to an SPS signal, but not both ways. Set the parameter tmb_counter_dump_period_ms to a positive integer (e.g. 10000 for 10s samples) if you want to dump them periodically. Set the parameter tmb_counter_dump_synched_to to a positive integer if you want to synchronize them to an SPS timing signal. (Its bits are:";
      for ( int i = 0 ; i < test_->nDIPPublications; i++ ){
	oss << "\n" << (1<<i) << "=" << test_->SPSTimingSignalNames_[i] << " ";
      }
      XCEPT_RAISE( xcept::Exception, oss.str() );
    }
    doSynchronizedDumps_ = true;
  }

  // Get optional SPS timing signal for test start
  spsTimingSignalsForStart_ = ( test_->parameters_.find( "synch_test_start_to" ) == test_->parameters_.end() ? 0 : test_->parameters_[ "synch_test_start_to" ] );
  if ( spsTimingSignalsForStart_ > 0 ) synchronizeTestStart_ = true;
#endif

  if ( doPeriodicDumps_ || doSynchronizedDumps_ ){
    dumperName_ = test_->withoutChars( " \t:;<>'\",?/~`!@#$%^&*()=[]|\\", test_->group_ ) + "_Test" + test_->id_ + "_" + test_->runStartTime_;
    
    // Collect TMBs whose counters are to be dumped:
    vector<emu::pc::Crate*> crates = test_->parser_.GetEmuEndcap()->crates();
    for ( vector<emu::pc::Crate*>::iterator crate = crates.begin(); crate != crates.end(); ++crate ){
      LOG4CPLUS_INFO( *test_->pLogger_, "In crate " << (*crate)->GetLabel() );
      vector<emu::pc::TMB*> tmbsInCrate = (*crate)->tmbs();
      LOG4CPLUS_INFO( *test_->pLogger_, "Crate " << (*crate)->GetLabel() << " TMB count " << tmbsInCrate.size() );
      LOG4CPLUS_INFO( *test_->pLogger_, "Crate " << (*crate)->GetLabel() << " TMB[0] in slot " << tmbsInCrate[0]->slot() );
      tmbs_.insert( tmbs_.end(), tmbsInCrate.begin(), tmbsInCrate.end() );
      LOG4CPLUS_INFO( *test_->pLogger_, "Total TMB count " << tmbs_.size() );
    }
    LOG4CPLUS_INFO( *test_->pLogger_, "Total TMB[0] in slot" << tmbs_[0]->slot() );

    // Get the data directory name. That's where we want to save the TMB counters' dump file, too.
    dumpDir_ = test_->getDataDirName();
    if ( dumpDir_.size() == 0 ){
      LOG4CPLUS_WARN( *test_->pLogger_, "No data directory found on this host. TMB counter's dump files will be saved in /tmp instead." );
      dumpDir_ = "/tmp";
    }
    if ( doPeriodicDumps_ ){
      LOG4CPLUS_INFO( *test_->pLogger_, "TMB counters will be dumped every " << dumpPeriodInSec_ << " seconds to a file in directory " << dumpDir_ );
    }
#ifdef DIP
    if ( doSynchronizedDumps_ ){
      LOG4CPLUS_INFO( *test_->pLogger_, "TMB dumps will be synchronized to " << printTimingSignals( spsTimingSignalsForDumps_ ) << " and written to a file in directory " << dumpDir_ );
    }
#endif    
  }
  else{
    ostringstream oss;
    oss << "TMB counters will not be read out and saved. Set the parameter tmb_counter_dump_period_ms to a positive integer (e.g. 10000 for 10s samples) if you want to dump them periodically.";
#ifdef DIP
    oss << "Set the parameter tmb_counter_dump_synched_to to a positive integer in if you want to dump them synchronized to an SPS timing signal. (Its bits are:";
    for ( int i = 0 ; i < test_->nDIPPublications; i++ ){
      oss << "\n" << (1<<i) << "=" << test_->SPSTimingSignalNames_[i] << " ";
    }
#endif
    LOG4CPLUS_WARN( *test_->pLogger_, oss.str() );
  }
}

#ifdef DIP
string emu::step::TimingOptions::printTimingSignals( unsigned int signals ){
  ostringstream oss;
  for( int i = 0 ; i < test_->nDIPPublications; i++ ){
    if ( 1<<i & signals ) oss << test_->SPSTimingSignalNames_[i] << ( i+1 < test_->nDIPPublications ? " " : "" );
  }
  return oss.str();
}
#endif
