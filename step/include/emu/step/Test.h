#ifndef __emu_step_Test_h__
#define __emu_step_Test_h__

#include "emu/step/TestParameters.h"

#ifdef DIP
#include "Dip.h"
#include "DipSubscription.h"
#include "emu/step/TimingOptions.h"
#endif

#include "emu/utils/Progress.h"

#include "emu/pc/XMLParser.h"
#include "emu/pc/XMLParser.h"
#include "emu/pc/EmuEndcap.h"
#include "emu/pc/DAQMB.h"

#include "toolbox/exception/Listener.h"

#include <vector>
#include <map>
#include <string>

using namespace std;

namespace emu{
  namespace step{

    enum ODMBMode_t { ODMBNormalMode, ODMBPedestalMode, ODMBCalibrationMode };
    enum ODMBInputKill_t { kill_None   = 0x0000,
			   kill_DCFEB1 = 0x0001, 
			   kill_DCFEB2 = 0x0002, 
			   kill_DCFEB3 = 0x0004, 
			   kill_DCFEB4 = 0x0008, 
			   kill_DCFEB5 = 0x0010, 
			   kill_DCFEB6 = 0x0020, 
			   kill_DCFEB7 = 0x0040, 
			   kill_TMB    = 0x0080,
			   kill_ALCT   = 0x0100, 
                           kill_DCFEBs = 0x007f,
			   kill_All    = 0x01ff };


    class SPSSynchronizer;

    class Test : public TestParameters, public toolbox::exception::Listener {

      friend class SPSListener;
      friend class TimingOptions;

    public:

      /// Ctor.
      ///
      /// @param id Test id.
      /// @param group VME group that this instance of emu::step::Test will handle.
      /// @param testParametersXML XML of the parameters of the test.
      /// @param generalSettingsXML XML of the PCrate and on-chamber electronics settings.
      /// @param specialSettingsXML XML of the PCrate and on-chamber electronics settings specific to this test.
      /// @param isFake If \em true, the test will just be simulated without VME communication.
      /// @param pLogger Pointer to the logger.
      ///
      /// @return The Test object.
      ///
      Test( const string& id, 
	    const string& group, 
	    const string& testParametersXML, 
	    const string& generalSettingsXML,
	    const string& specialSettingsXML,
	    const bool    isFake,
	    Logger*       pLogger              );

      ~Test();


      /// Setter of run number
      ///
      /// @param runStartTime Run number as obtained from the local DAQ.
      ///
      /// @return Reference to this Test object.
      ///
      Test& setRunNumber( const uint32_t runNumber ){ runNumber_ = runNumber; return *this; }

      /// Setter of run start time
      ///
      /// @param runStartTime Time of start of run as obtained from the local DAQ (e.g., 130529_154434_UTC).
      ///
      /// @return Reference to this Test object.
      ///
      Test& setRunStartTime( const string& runStartTime ){ runStartTime_ = runStartTime; return *this; }

      /// Setter of data directory names
      ///
      /// @param dataDirNames Data directory names (hostname:directory) of all RUIs
      ///
      /// @return Reference to this Test object.
      ///
      Test& setDataDirNames( const vector<string>& dataDirNames ){ dataDirNames_ = dataDirNames; return *this; }

      /// Configure the test.
      ///
      ///
      void configure();

      /// Execute the test.
      ///
      ///
      void enable();

      /// Get the progress fo this test.
      ///
      ///
      /// @return Progress in percent.
      ///
      double getProgress();

      /// Interrupt the running test.
      ///
      ///
      void stop(){ isToStop_ = true; }

      /// Diagnostic method for histograming and printing out (among other things) the time between the DMB's receiving CLCT pretrigger and L1A.
      ///
      /// @param tmb Pointer to the TMB associated with this DMB.
      /// @param dmb Pointer to the DMB.
      /// @param ccb Pointer to the CCB in the same crate.
      /// @param mpc Pointer to the MPC in the same crate.
      ///
      void PrintDmbValuesAndScopes( emu::pc::TMB* tmb, emu::pc::DAQMB* dmb, emu::pc::CCB* ccb, emu::pc::MPC* mpc );

      bool isFake() const { return isFake_; }
      

      enum { pipelineDepthFromXML = -99 };

    private:
      enum State_t        { forConfigure_, forEnable_ };
      string              group_; ///< Name of the VME crate group that this emu::step::Test instance handles.
      bool                isFake_; ///< If \em true, the test will just be simulated without VME communication.
      bool                isToStop_; ///< Set this to \em true to interrupt the test.
      string              VME_XML_; ///< The VME configuration XML for this test.
      uint32_t            runNumber_; ///< run number as obtained from the local DAQ
      string              runStartTime_; ///< time of start of run as obtained from the local DAQ (e.g., 130529_154434_UTC)
      vector<string>      dataDirNames_; // all RUIs' data directory names

      emu::pc::XMLParser   parser_;
      emu::utils::Progress progress_; ///< Progress counter.

#ifdef DIP
    public:
      /// The SPS cycle whose timing signals we're interested in
      const string SPSCycleName_;
      /// DIP publication indices (each corresponds to a published SPS timing signal)
      enum SPSTimingSignal_t { SPSSuperCycleStart ,  SPSCycleStart ,  SPSExtractionStart ,  SPSExtractionEnd, nDIPPublications };
      /// Names of SPS timing signals that we'll subscribe to by DIP: 
      static const char* SPSTimingSignalNames_[nDIPPublications];
    private:
      /// Array of DIP publication names
      static const char* DIPPublications_[nDIPPublications];
      /// DIP publication name --> index map (the inverse of the DIPPublications_ array)
      map<string,int> DIPPublicationIndices_; 
      /// The "DIP message received" word (its bits are indexed by the SPSTimingSignal_t enum)
      unsigned int DIPMessageReceivedFlags_;
      /// DIPFactory singleton. Call Dip::create() only once to create it, all subsequent calls would only return the same object. Do not try to destroy dip_.
      DipFactory *dip_;
      /// Nested DIP listener class
      class SPSListener : public DipSubscriptionListener{
      private:
	Test *client_;
      public:
	SPSListener( Test *c ) : client_( c ){};
	void handleMessage( DipSubscription *subscription, DipData &message);
	void connected( DipSubscription *subscription );
	void disconnected( DipSubscription *subscription, char *reason );
	void handleException( DipSubscription* subscription, DipException& ex );
      } *spsListener_; /// SPS DIP listener
      DipSubscription **subscriptions_;/// The array of our subscriptions
      void subscribeToDIP();
      void unsubscribeFromDIP();
      void resetDIPMessageReceivedFlags( unsigned int signals );
      /// See if we received any of the indices we're interested in, and if so, reset its corresponding receive flag, and return its index.
      ///
      /// @param signals The world whose bits corresponds to the indices of the signals (see SPSTimingSignal_t) that we're interested in.
      ///
      /// @return The index of the received signal (see SPSTimingSignal_t) we're interested in.
      ///
      int receivedSPSSignal( unsigned int signals );
      int waitForSPSTimingSignals( unsigned int signals );
#endif

    private:
      void ( emu::step::Test::* getProcedure( const string& testId, State_t state ) )(); 
      void createEndcap( const string& generalSettingsXML,
			 const string& specialSettingsXML  );
      int getDDUInputFiberMask( int crateId, int dduSlot );
      void setUpDDU(emu::pc::Crate*);
      void configureCrates();
      string withoutChars( const string& chars, const string& str );
      void disableTrigger();
      void setUpDMB( emu::pc::DAQMB *dmb );
      void setUpODMBPulsing( emu::pc::DAQMB *dmb, ODMBMode_t mode, ODMBInputKill_t killInput );
      void setAllDCFEBsPipelineDepth( emu::pc::DAQMB* dmb, const short int depth = pipelineDepthFromXML );
      void turnONlvDCFEBandALCT( emu::pc::Crate* crate );
      void configureODMB( emu::pc::Crate* crate );
      void resyncDCFEBs(emu::pc::Crate* crate); ///< Temporary method until ODMB firmware takes care of it.
      void hardResetOTMBs(emu::pc::Crate* crate); ///< hard-reset all OTMBs *only* (to clean the hot channel masks)
      void printDCFEBUserCodes( emu::pc::DAQMB* dmb );
      string getDataDirName() const;

      void configure_11();
      void configure_11c();
      void configure_12();
      void configure_13();
      void configure_14();
      void configure_15();
      void configure_16();
      void configure_17();
      void configure_17b();
      void configure_18();
      void configure_19();
      void configure_21();
      void configure_25();
      void configure_27();
      void configure_30();
      void configure_40();
      void configure_fake();
      void enable_11();
      void enable_11c();
      void enable_12();
      void enable_13();
      void enable_14();
      void enable_15();
      void enable_16();
      void enable_17();
      void enable_17b();
      void enable_18();
      void enable_19();
      void enable_21();
      void enable_25();
      void enable_27();
      void enable_30();
      void enable_40();
      void enable_fake();

      virtual void onException( xcept::Exception& e ); // callback for toolbox::exception::Listener

    };
  }
}

#endif
