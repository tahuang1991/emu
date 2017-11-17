#ifndef __emu_daq_rui_STEPEventCounter_h__
#define __emu_daq_rui_STEPEventCounter_h__

#include <stdint.h>

#include "xdata/Vector.h"
#include "xdata/Boolean.h"
#include <string>

namespace emu { namespace daq { namespace rui {

  class STEPEventCounter{
    // Counts events on each input of the DDU. 
    // Info on live inputs and inputs with data are obtained from DDU header (3).
  public:
    enum { maxDDUInputs_ = 15 };
    STEPEventCounter();
    void         initialize( const uint64_t requestedEvents, char* const DDUHeader );
    bool         isInitialized() const { return isInitialized_; }
    void         reset();
    bool         isNeededEvent( char* const DDUHeader );
    uint64_t     getLowestCount() const;
    uint64_t     getCount( const int dduInputIndex ) const;
    uint64_t     getCountRead( const int dduInputIndex ) const;
    uint64_t     getNEvents() const{ return neededEvents_; }
    bool         isLiveInput( const int dduInputIndex ) const;
    bool         isMaskedInput( const int dduInputIndex ) const;
    void         maskInput( const int dduInputIndex );
    void         unmaskInput( const int dduInputIndex );
    std::string  print() const;

  private:
    enum { offsetNonEmptyDDUInputsField_  = 18, offsetLiveDDUInputsField_ = 22 }; // from the start of DDU header1 [bytes]
    uint64_t     requestedEvents_;
    uint64_t     neededEvents_; // the total number of events needed so far
    bool         isLiveInput_[maxDDUInputs_]; // this is obtained from the DDU header
    bool         isMaskedInput_[maxDDUInputs_]; // this the user can set
    uint64_t     count_[maxDDUInputs_]; // count_[i] is the number of _accepted_ events that have data from input i
    uint64_t     countRead_[maxDDUInputs_]; // countRead_[i] is the number of _read_ (i.e. both accepted and rejected) events that have data from input i
    bool         isInitialized_;
  };

}}} // namespace emu::daq::rui

#endif
