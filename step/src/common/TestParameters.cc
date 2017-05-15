#include "emu/step/TestParameters.h"

#include "emu/utils/IO.h"
#include "emu/utils/DOM.h"
#include "emu/utils/String.h"
#include "emu/utils/System.h"

#include "xcept/tools.h"

using namespace emu::utils;

emu::step::TestParameters::TestParameters( const string& id, 
					   const string& testParametersXML,
					   Logger*       pLogger )
  : bsem_( toolbox::BSem::EMPTY ) // locked
  , id_( id )
  , nEvents_( 0 )
  , pLogger_( pLogger ){
  try{
    extractParameters( testParametersXML );
    bsem_.give();
  }
  catch ( xcept::Exception &e ){
    stringstream ss;
    ss << "Failed to create emu::step::TestParameters for test " << id_ << ".";
    bsem_.give();
    XCEPT_RETHROW( xcept::Exception, ss.str(), e );
  }
}

void emu::step::TestParameters::extractParameters( const string& testParametersXML ){

  stringstream xpath;
  xpath << "//STEP_tests/test_config[translate(test,' ','')='" << id_ << "']/*[name()!='test']";
  if ( pLogger_ ){ LOG4CPLUS_INFO( *pLogger_,  "testParametersXML\n" << testParametersXML << "\nxpath\n" << xpath.str() << "\n"); }
  vector< pair< string, string > > par = utils::getSelectedNodesValues( testParametersXML, xpath.str() );

  for ( vector< pair< string, string > >::const_iterator p = par.begin(); p != par.end(); ++p ){
    parameters_[p->first] = utils::stringTo<int>( p->second );
  }

  calculateNEvents();
}

void emu::step::TestParameters::calculateNEvents(){
  // For passive tests (i.e., those without pulses), get the number of events to be collected.
  if ( id_ == "18"  ||
       id_ == "27"  || 
       id_ == "27s" || 
       id_ == "40"  || 
       id_ == "40s"    ) nEvents_ = parameters_["events_total"];
  // For active (i.e., pulsed) tests, the number of events to be collected will be calculated at the beginning of the test
  // as it depends on the number of chambers to be tested. For the time being, keep nEvents_ = 0 for active (pulsed or timed) tests.
}
