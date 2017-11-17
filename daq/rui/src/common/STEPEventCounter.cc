#include "emu/daq/rui/STEPEventCounter.h"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <limits>

emu::daq::rui::STEPEventCounter::STEPEventCounter(){
  for ( int i=0; i<maxDDUInputs_; ++i ) isMaskedInput_[i] = false;
  reset();
}

void emu::daq::rui::STEPEventCounter::initialize( const uint64_t requestedEvents, char* const DDUHeader ){
  // Sets number of requested events, live DDU inputs and zeros all counters.
  requestedEvents_ = requestedEvents;
  neededEvents_ = 0;
  short* liveDDUInputs = (short*)( DDUHeader + offsetLiveDDUInputsField_ );
  // std::cout << "LLLL: " << std::hex << *liveDDUInputs << " " << std::dec << *liveDDUInputs << std::endl; 
  short bitMask = 0x0001;
  for ( int i=0; i<maxDDUInputs_; ++i ){
    isLiveInput_[i] = bool( bitMask & *liveDDUInputs );
    bitMask <<= 1;
    count_[i] = 0;
    countRead_[i] = 0;
  }
  isInitialized_ = true;
}

void emu::daq::rui::STEPEventCounter::reset(){
  // Zeros number of requested events and all counters, and unsets live DDU inputs.
  // Does NOT unmask inputs.
  requestedEvents_ = 0;
  neededEvents_ = 0;
  for ( int i=0; i<maxDDUInputs_; ++i ){
    isLiveInput_[i] = false;
    count_[i] = 0;
    countRead_[i] = 0;
  }
  isInitialized_ = false;
}

bool emu::daq::rui::STEPEventCounter::isNeededEvent( char* const DDUHeader ){
  // This event is needed if it contains data from a DDU input that has not yet produced
  // data in the requested number of events.
  // If this event is needed, increment counters for DDU inputs with data in this event.

  // Need this event?
  bool isNeeded = false;
  short* nonEmptyDDUInputs = (short*)( DDUHeader + offsetNonEmptyDDUInputsField_ );
  short bitMask = 0x0001;
  for ( int i=0; i<maxDDUInputs_; ++i ){
    if ( bool(bitMask & *nonEmptyDDUInputs) && 
	 ! isMaskedInput_[i]                &&
	 count_[i] < requestedEvents_          ){
      isNeeded = true;
      break;
    }
    bitMask <<= 1;
  }

  // If so, increment counters.
  if ( isNeeded ){
    bitMask = 0x0001;
    for ( int i=0; i<maxDDUInputs_; ++i ){
      if ( bitMask & *nonEmptyDDUInputs && 
	   ! isMaskedInput_[i]             ) ++count_[i];
      bitMask <<= 1;
    }
    ++neededEvents_;
  }

  // Irrespective of whether this event is needed, increment the counters of read events with data from the given input.
  bitMask = 0x0001;
  for ( int i=0; i<maxDDUInputs_; ++i ){
    if ( bitMask & *nonEmptyDDUInputs && 
	 ! isMaskedInput_[i]             ) ++countRead_[i];
    bitMask <<= 1;
  }

  return isNeeded;
}

uint64_t emu::daq::rui::STEPEventCounter::getLowestCount() const {
  if ( isInitialized_ ){
    uint64_t lowestCount = std::numeric_limits<int64_t>::max(); // When cast to signed int, this should still be positive.
    bool allExcluded = true;
    for ( int i=0; i<maxDDUInputs_; ++i ){
      allExcluded &= ( !isLiveInput_[i] || isMaskedInput_[i]);
      if ( isLiveInput_[i] && ! isMaskedInput_[i] )
	if ( count_[i] < lowestCount ) lowestCount = count_[i];
    }
    if ( allExcluded ) return std::numeric_limits<int64_t>::max(); // When cast to signed int, this should still be positive.
    return lowestCount;
  }
  return 0;
}

uint64_t emu::daq::rui::STEPEventCounter::getCount( const int dduInputIndex ) const {
  if ( 0 <= dduInputIndex && dduInputIndex < maxDDUInputs_ ) return count_[dduInputIndex];
  return 0;
}

uint64_t emu::daq::rui::STEPEventCounter::getCountRead( const int dduInputIndex ) const {
  if ( 0 <= dduInputIndex && dduInputIndex < maxDDUInputs_ ) return countRead_[dduInputIndex];
  return 0;
}

bool emu::daq::rui::STEPEventCounter::isLiveInput( const int dduInputIndex ) const {
  if ( 0 <= dduInputIndex && dduInputIndex < maxDDUInputs_ ) return isLiveInput_[dduInputIndex];
  return false;
}

void emu::daq::rui::STEPEventCounter::maskInput( const int dduInputIndex ){
  if ( 0 <= dduInputIndex && dduInputIndex < maxDDUInputs_ ) isMaskedInput_[dduInputIndex] = true;
}

void emu::daq::rui::STEPEventCounter::unmaskInput( const int dduInputIndex ){
  if ( 0 <= dduInputIndex && dduInputIndex < maxDDUInputs_ ) isMaskedInput_[dduInputIndex] = false;
}

bool emu::daq::rui::STEPEventCounter::isMaskedInput( const int dduInputIndex ) const {
  if ( 0 <= dduInputIndex && dduInputIndex < maxDDUInputs_ ) return isMaskedInput_[dduInputIndex];
  return false;
}

std::string emu::daq::rui::STEPEventCounter::print() const {
  // Prints comma-separated list of counts. 
  // Prints '-' for inputs that are not alive.
  // Prints '_' for inputs that are masked.
  // Prints '=' for inputs that are masked and not alive.
  std::stringstream counters;
  for ( int i=0; i<maxDDUInputs_; ++i ){
    if      (   isLiveInput_[i] && ! isMaskedInput_[i] ) counters << count_[i];
    else if ( ! isLiveInput_[i] && ! isMaskedInput_[i] ) counters << "-";
    else if (   isLiveInput_[i] &&   isMaskedInput_[i] ) counters << "_";
    else if ( ! isLiveInput_[i] &&   isMaskedInput_[i] ) counters << "=";
    if ( i+1 != maxDDUInputs_ ) counters << ",";
  }
  return counters.str();
}
