#include "emu/supervisor/CIControl.h"

emu::supervisor::CIControl::CIControl( xdaq::Application *parent, 
				       xdaq::ApplicationDescriptor* tcdsApplicationDescriptor, 
				       xdata::String partition )
  : emu::supervisor::TCDSControl::TCDSControl( parent, tcdsApplicationDescriptor, partition, "CI" ){}

emu::supervisor::CIControl& emu::supervisor::CIControl::setRunType( xdata::String& runType ){
  this->emu::supervisor::TCDSControl::setRunType( runType );
  return *this;
}

emu::supervisor::CIControl& emu::supervisor::CIControl::configureSequence(){
  // First of all, wait for CI to complete 'Configure' transition
  waitForState( "Configured", 20 );

  // SendLongBDATA 196608 = 0x30000
  // Bits 0x3fff00000 are the address of the TTCrx
  // Bit  0x20000 (denoted by 'E' in  http://ttc.web.cern.ch/TTC/TTCrx_manual3.11.pdf):
  //              E=0 accesses TTCrx internal registers
  //              E=1 accesses external sub-addresses; accompanied by DOUTSTR.
  // Bit  0x10000 is 1 for individually addressed commands/data
  // Bits 0x0ff00 are the subaddress
  // Bits 0x000ff are the data
  // In our case sub-address=data=0 as they should to clear the discrete logic decoder.
  // Mike Matveev:
  // "I think this is a long-format command
  // to external (w.r.t. to TTCrx) registers to clear the discrete
  // logic decoder on the CCB board. You can find more details
  // on p.3 of the user's guide (see step.14)
  // http://padley.rice.edu/cms/users_guide_147.pdf
  // but this is described in terms of access to the TTCvi.
  // Years ago we learned that the default state of some
  // TTCrx after power cycling may cause problems for a discrete
  // logic decoder on a CCB. This command (with the subaddess=data=0)
  // allows to clear up the whole path; needs only once at
  // initialization."
  
  xdata::UnsignedInteger data( 0 );
  // xdata::String          type( "long" );
  xdata::String          type( "addressed" );
  xdata::UnsignedInteger addressOfTTCrx( 0 );
  xdata::UnsignedInteger subaddress( 0 );
  xdata::String          bcommandAddressType( "external" );
  xdata::String          BC0( "BC0" );
  xdata::String          TestEnable( "TestEnable" );
  xdata::String          HardReset( "HardReset" );
  xdata::String          Resync( "Resync" );

  
  switch ( runType_ ){
  case global: // fall through
  case local:
    // BEGINSEQUENCE configure
    //   DisableL1A
    //   ResetCounters
    //   SendLongBDATA 196608
    //   mSleep 100
    //   BGO HardReset
    //   mSleep 500
    //   BGO Resynch
    //   mSleep 100
    //   Periodic 1
    //   Periodic Off
    //   EnableL1A
    // ENDSEQUENCE
    sendBCommand( data, type, addressOfTTCrx, subaddress, bcommandAddressType );		  
    mSleep( 100 );
    // A hard reset is needed to make sure the DDU FIFO is cleared
    // (in case some events have been left there) so that the subsequent resync 
    // can zero its L1A register. Otherwise, with nonzero L1A register, 
    // the DDU would fail to be enabled.
    // Do this from the CIs so that they can be sent in both local and global.
    // Not being simultaneous, this resync here could not possibly result
    // in properly aligned SP fibers. They should be aligned properly by the
    // simultaneous resync issued by the {C,L}PM on starting/enabling as part of 
    // the standard TCDS 'Start' Bgo train.
    sendBgo( HardReset );
    mSleep( 500 );
    sendBgo( Resync );
    mSleep( 100 );
    break;
  case AFEBcalibration: // fall through
  case CFEBcalibration:
    // BEGINSEQUENCE configure
    //   DisableL1A
    //   ResetCounters
    //   SendLongBDATA 196608
    //   mSleep 100
    //   BGO HardReset
    //   mSleep 500
    //   BGO Resynch
    //   mSleep 100
    //   Periodic 1
    //   Periodic Off
    //   EnableL1A
    // ENDSEQUENCE
    sendBCommand( data, type, addressOfTTCrx, subaddress, bcommandAddressType );		  
    mSleep( 100 );
    // Hard reset is needed in order to clear the FIFO of the DDUs 
    // (in case some events have been left there) so that the resync can zero its
    // L1A counter. Otherwise, with nonzero L1A counter, the DDU would fail to be
    // enabled.
    // Do this from the CIs so that it can be sent in both local and global.
    sendBgo( HardReset );
    mSleep( 500 );
    // In calibration runs, we disable the standard TCDS Bgo train 'Start' (which
    // would normally emit a resync), so emit a resync now instead:
    sendBgo( Resync );
    mSleep( 100 );
    break;
  default:
    XCEPT_RAISE( xcept::Exception, "Unknown run type." );
  }
  return *this;
}

emu::supervisor::CIControl& emu::supervisor::CIControl::enableSequence(){
  // First of all, wait for CI to complete 'Enable' transition
  waitForState( "Enabled", 20 );

  switch ( runType_ ){
  case global:
  case local:
    break;
  case AFEBcalibration:
    break;
  case CFEBcalibration:
    break;
  default:
    XCEPT_RAISE( xcept::Exception, "Unknown run type." );
  }
  return *this;
}

emu::supervisor::CIControl& emu::supervisor::CIControl::stopSequence(){
  // First of all, wait for CI to complete the 'Stop' transition
  // waitForState( "Halted|Configured", 20 );
  waitForState( "Configured", 20 );

  xdata::String Resync( "Resync" );
  xdata::String HardReset( "HardReset" );

  switch ( runType_ ){
  case global:           // fall through
  case local:            // fall through
  case AFEBcalibration:  // fall through
  case CFEBcalibration:
    // Hard reset is needed in order to clear the FIFO of the DDUs 
    // (in case some events have been left there) so that the resync can zero its
    // L1A counter. Otherwise, with nonzero L1A counter, the DDU would fail to be
    // enabled.
    // This way, the FEDs will be ready to start the next run.
    sendBgo( HardReset );
    mSleep( 500 );
    sendBgo( Resync );
    mSleep( 100 );
    break;
  default:
    XCEPT_RAISE( xcept::Exception, "Unknown run type." );
  }
  return *this;
}
