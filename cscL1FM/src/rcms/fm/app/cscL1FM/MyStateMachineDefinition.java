package rcms.fm.app.cscL1FM;


import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.ParameterException;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.parameter.type.IntegerT;
import rcms.fm.fw.parameter.type.LongT;
import rcms.fm.fw.parameter.type.StringT;
import rcms.fm.fw.user.UserStateMachineDefinition;
import rcms.statemachine.definition.State;
import rcms.statemachine.definition.StateMachineDefinitionException;

/**
 * This class defines the Finite State Machine for a level 1 Function Manager.
 * 
 * The actual definition of the State Machine must be put in the "init" method.
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 */
public class MyStateMachineDefinition extends UserStateMachineDefinition {

	public MyStateMachineDefinition() throws StateMachineDefinitionException {
		//
		// Defines the States for this Finite State Machine.
		//

		// steady states
		addState(MyStates.INITIAL);
		addState(MyStates.HALTED);
		addState(MyStates.CONFIGURED);
		addState(MyStates.RUNNING);
		addState(MyStates.RUNNINGDEGRADED);
		addState(MyStates.PAUSED);
		addState(MyStates.ERROR);
		addState(MyStates.TTSTEST_MODE);
		addState(MyStates.RUNNINGSOFTERRORDETECTED);
		
		// transitional states
		addState(MyStates.INITIALIZING);
		addState(MyStates.CONFIGURING);
		addState(MyStates.HALTING);
		addState(MyStates.PAUSING);
		addState(MyStates.RESUMING);
		addState(MyStates.STARTING);
		addState(MyStates.STOPPING);
		addState(MyStates.RECOVERING);
		addState(MyStates.RESETTING);
		addState(MyStates.TESTING_TTS);
		addState(MyStates.PREPARING_TTSTEST_MODE);
		addState(MyStates.COLDRESETTING);
		addState(MyStates.FIXINGSOFTERROR);

		//
		// Defines the Initial state.
		//
		setInitialState(MyStates.INITIAL);

		//
		// Defines the Inputs (Commands) for this Finite State Machine.
		//
		addInput(MyInputs.INITIALIZE);
		addInput(MyInputs.CONFIGURE);
		addInput(MyInputs.START);
		addInput(MyInputs.STOP);
		addInput(MyInputs.PAUSE);
		addInput(MyInputs.RESUME);
		addInput(MyInputs.HALT);
		addInput(MyInputs.RECOVER);
		addInput(MyInputs.RESET);
		addInput(MyInputs.SETERROR);
		addInput(MyInputs.PREPARE_TTSTEST_MODE);
		addInput(MyInputs.TEST_TTS);
		addInput(MyInputs.COLDRESET);
		addInput(MyInputs.FIXSOFTERROR);
		
		// The SETERROR Input moves the FSM in the ERROR State.
		// This command is not allowed from the GUI.
		// It is instead used inside the FSM callbacks.
		MyInputs.SETERROR.setVisualizable(false);

		// invisible commands needed for fully asynchronous behaviour
		addInput(MyInputs.SETCONFIGURED);
		addInput(MyInputs.SETRUNNING);
		addInput(MyInputs.SETRUNNINGDEGRADED);
		addInput(MyInputs.SETRUNNINGSOFTERRORDETECTED);
		addInput(MyInputs.SETPAUSED);
		addInput(MyInputs.SETRESUMED);
		addInput(MyInputs.SETRESUMEDDEGRADED);
		addInput(MyInputs.SETRESUMEDSOFTERRORDETECTED);
		addInput(MyInputs.SETHALTED);
		addInput(MyInputs.SETTTSTEST_MODE);
		
		
		// make these invisible
		MyInputs.SETCONFIGURED.setVisualizable(false);
		
//      left visualizable for testing
//		
//		MyInputs.SETRUNNING.setVisualizable(false);
//		MyInputs.SETRUNNINGDEGRADED.setVisualizable(false);
//		MyInputs.SETRUNNINGSOFTERRORDETECTED.setVisualizable(false);
		
		MyInputs.SETPAUSED.setVisualizable(false);
		MyInputs.SETRESUMED.setVisualizable(false);
		MyInputs.SETRESUMEDDEGRADED.setVisualizable(false);
		MyInputs.SETRESUMEDSOFTERRORDETECTED.setVisualizable(false);
		MyInputs.SETHALTED.setVisualizable(false);
		MyInputs.SETTTSTEST_MODE.setVisualizable(false);
		
		//
		// Define command parameters.
		// These are then visible in the default GUI.
		//
		
		// define parameters for tts testing command
		//
		CommandParameter<IntegerT> ttsTestFedid = new CommandParameter<IntegerT>(MyParameters.TTS_TEST_FED_ID, new IntegerT(-1));
		CommandParameter<StringT> ttsTestMode = new CommandParameter<StringT>(MyParameters.TTS_TEST_MODE, new StringT(""));
		CommandParameter<StringT> ttsTestPattern = new CommandParameter<StringT>(MyParameters.TTS_TEST_PATTERN, new StringT(""));
		CommandParameter<IntegerT> ttsTestSequenceRepeat = new CommandParameter<IntegerT>(MyParameters.TTS_TEST_SEQUENCE_REPEAT, new IntegerT(-1));
		
		// define parameter set
		ParameterSet<CommandParameter> ttsTestParameters = new ParameterSet<CommandParameter>();
		try {
			ttsTestParameters.add(ttsTestFedid);
			ttsTestParameters.add(ttsTestMode);
			ttsTestParameters.add(ttsTestPattern);
			ttsTestParameters.add(ttsTestSequenceRepeat);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to ttsTestParameters. Duplicate Parameter?", nothing );
		}
		
		// set the test parameters
		MyInputs.TEST_TTS.setParameters(ttsTestParameters);
	
		//
		// define parameters for Initialize command
		//
		CommandParameter<IntegerT> initializeSid = new CommandParameter<IntegerT>(MyParameters.SID, new IntegerT(0));
		CommandParameter<StringT> initializeGlobalConfigurationKey = new CommandParameter<StringT>(MyParameters.GLOBAL_CONF_KEY, new StringT(""));

		// define parameter set
		ParameterSet<CommandParameter> initializeParameters = new ParameterSet<CommandParameter>();
		try {
			initializeParameters.add(initializeSid);
			initializeParameters.add(initializeGlobalConfigurationKey);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to initializeParameters. Duplicate Parameter?", nothing );
		}
		
		MyInputs.INITIALIZE.setParameters(initializeParameters);
	
		//
		// define parameters for Configure command
		//
		CommandParameter<StringT> configureFedEnableMask = new CommandParameter<StringT>(MyParameters.FED_ENABLE_MASK, new StringT(""));
		CommandParameter<IntegerT> configureRunNumber = new CommandParameter<IntegerT>(MyParameters.RUN_NUMBER, new IntegerT(-1));
		CommandParameter<StringT> configureRunKey = new CommandParameter<StringT>(MyParameters.RUN_KEY, new StringT(""));

		// define parameter set
		ParameterSet<CommandParameter> configureParameters = new ParameterSet<CommandParameter>();
		try {
			configureParameters.add(configureFedEnableMask);
			configureParameters.add(configureRunNumber);
			configureParameters.add(configureRunKey);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to configureParameters. Duplicate Parameter?", nothing );
		}
		
		MyInputs.CONFIGURE.setParameters(configureParameters);
	
		//
		// define parameters for Start command
		//
		CommandParameter<IntegerT> startRunNumber = new CommandParameter<IntegerT>(MyParameters.RUN_NUMBER, new IntegerT(-1));

		// define parameter set
		ParameterSet<CommandParameter> startParameters = new ParameterSet<CommandParameter>();
		try {
			startParameters.add(startRunNumber);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to startParameters. Duplicate Parameter?", nothing );
		}
		
		MyInputs.START.setParameters(startParameters);
	
	
		//
		// define parameters for FixSoftError command
		//
		CommandParameter<LongT> triggerNumberAtPause = new CommandParameter<LongT>(MyParameters.TRIGGER_NUMBER_AT_PAUSE, new LongT(-1));

		// define parameter set
		ParameterSet<CommandParameter> fixSoftErrorParameters = new ParameterSet<CommandParameter>();
		try {
			fixSoftErrorParameters.add(triggerNumberAtPause);
		} catch (ParameterException nothing) {
			// Throws an exception if a parameter is duplicate
			throw new StateMachineDefinitionException( "Could not add to fixSoftErrorParameters. Duplicate Parameter?", nothing );
		}
		
		MyInputs.FIXSOFTERROR.setParameters(fixSoftErrorParameters);

		//
		// Define the State Transitions
		//

		// INIT Command:
		// The INIT input is allowed only in the INITIAL state, and moves the
		// FSM in the INITIALIZING state.
		//
		addTransition(MyInputs.INITIALIZE, MyStates.INITIAL, MyStates.INITIALIZING);
		
		// TEST_MODE Command:
		// The TEST_MODE input is allowed in the HALTED state and moves 
		// the FSM in the PREPARING_TEST_MODE state.
		//
		addTransition(MyInputs.PREPARE_TTSTEST_MODE, MyStates.HALTED, MyStates.PREPARING_TTSTEST_MODE);

		// Reach the TEST_MODE State
		addTransition(MyInputs.SETTTSTEST_MODE, MyStates.PREPARING_TTSTEST_MODE, MyStates.TTSTEST_MODE);
		addTransition(MyInputs.SETTTSTEST_MODE, MyStates.TESTING_TTS, MyStates.TTSTEST_MODE);

		// TEST_TTS Command:
		// The TEST_TTS input is allowed in the TEST_MODE state and moves
		// the FSM in the TESTING_TTS state.
		addTransition(MyInputs.TEST_TTS, MyStates.TTSTEST_MODE, MyStates.TESTING_TTS);

		// ColdReset  Command:
		addTransition(MyInputs.COLDRESET, MyStates.HALTED, MyStates.COLDRESETTING);

		
		// CONFIGURE Command:
		// The CONFIGURE input is allowed only in the HALTED state, and moves
		// the FSM in the CONFIGURING state.
		//
		addTransition(MyInputs.CONFIGURE, MyStates.HALTED, MyStates.CONFIGURING);

		// START Command:
		// The START input is allowed only in the CONFIGURED state, and moves
		// the FSM in the STARTING state.
		//
		addTransition(MyInputs.START, MyStates.CONFIGURED, MyStates.STARTING);

		// PAUSE Command:
		// The PAUSE input is allowed only in the RUNNING and RUNNINGDEGRADED state, and moves
		// the FSM in the PAUSING state.
		//
		addTransition(MyInputs.PAUSE, MyStates.RUNNING, MyStates.PAUSING);
		addTransition(MyInputs.PAUSE, MyStates.RUNNINGDEGRADED, MyStates.PAUSING);
		addTransition(MyInputs.PAUSE, MyStates.RUNNINGSOFTERRORDETECTED, MyStates.PAUSING);

		// RESUME Command:
		// The RESUME input is allowed only in the PAUSED state, and moves
		// the FSM in the RESUMING state.
		//
		addTransition(MyInputs.RESUME, MyStates.PAUSED, MyStates.RESUMING);
		
		// STOP Command:
		// The STOP input is allowed only in the RUNNING and PAUSED state, and moves
		// the FSM in the CONFIGURED state.
		//
		addTransition(MyInputs.STOP, MyStates.RUNNING, MyStates.STOPPING);
		addTransition(MyInputs.STOP, MyStates.RUNNINGDEGRADED, MyStates.STOPPING);
		addTransition(MyInputs.STOP, MyStates.RUNNINGSOFTERRORDETECTED, MyStates.STOPPING);
		addTransition(MyInputs.STOP, MyStates.PAUSED, MyStates.STOPPING);
		
		// HALT Command:
		// The HALT input is allowed in the RUNNING, CONFIGURED and PAUSED
		// state, and moves the FSM in the HALTING state.
		//
		addTransition(MyInputs.HALT, MyStates.RUNNING, MyStates.HALTING);
		addTransition(MyInputs.HALT, MyStates.RUNNINGDEGRADED, MyStates.HALTING);
		addTransition(MyInputs.HALT, MyStates.RUNNINGSOFTERRORDETECTED, MyStates.HALTING);
		addTransition(MyInputs.HALT, MyStates.CONFIGURED, MyStates.HALTING);
		addTransition(MyInputs.HALT, MyStates.PAUSED, MyStates.HALTING);
		addTransition(MyInputs.HALT, MyStates.TTSTEST_MODE, MyStates.HALTING);

		// RECOVER Command:
		// The RECOVER input is allowed from ERROR and moves the FSM in to
		// RECOVERING state.
		//
		addTransition(MyInputs.RECOVER, MyStates.ERROR, MyStates.RECOVERING);

		// RESET Command:
		// The RESET input is allowed from any steady state and moves the FSM in the
		// RESETTING state.
		//
		addTransition(MyInputs.RESET, MyStates.HALTED , MyStates.RESETTING);
		addTransition(MyInputs.RESET, MyStates.CONFIGURED , MyStates.RESETTING);
		addTransition(MyInputs.RESET, MyStates.RUNNING , MyStates.RESETTING);
		addTransition(MyInputs.RESET, MyStates.RUNNINGDEGRADED , MyStates.RESETTING);
		addTransition(MyInputs.RESET, MyStates.RUNNINGSOFTERRORDETECTED , MyStates.RESETTING);
		addTransition(MyInputs.RESET, MyStates.PAUSED , MyStates.RESETTING);
		addTransition(MyInputs.RESET, MyStates.TTSTEST_MODE , MyStates.RESETTING);
		addTransition(MyInputs.RESET, MyStates.ERROR , MyStates.RESETTING);		

		//
		// The following transitions are not triggered from the GUI.
		//

		// Transition for going to ERROR
		addTransition(MyInputs.SETERROR, State.ANYSTATE, MyStates.ERROR);

		//
		// add transitions for transitional States
		//

		// Reach the HALTED State
		addTransition(MyInputs.SETHALTED, MyStates.INITIALIZING, MyStates.HALTED);
		addTransition(MyInputs.SETHALTED, MyStates.HALTING, MyStates.HALTED);
		addTransition(MyInputs.SETHALTED, MyStates.RECOVERING, MyStates.HALTED);
		addTransition(MyInputs.SETHALTED, MyStates.RESETTING, MyStates.HALTED);
		addTransition(MyInputs.SETHALTED, MyStates.COLDRESETTING, MyStates.HALTED);

		// Reach the CONFIGURED State
		addTransition(MyInputs.SETCONFIGURED, MyStates.RECOVERING, MyStates.CONFIGURED);
		addTransition(MyInputs.SETCONFIGURED, MyStates.CONFIGURING, MyStates.CONFIGURED);
		addTransition(MyInputs.SETCONFIGURED, MyStates.STOPPING, MyStates.CONFIGURED);
		
		// Reach the RUNNING State
//		addTransition(MyInputs.SETRUNNING, MyStates.RECOVERING, MyStates.RUNNING);
		addTransition(MyInputs.SETRUNNING, MyStates.STARTING, MyStates.RUNNING);

		// Reach the RUNNINGDEGRADED State		
		addTransition(MyInputs.SETRUNNINGDEGRADED, MyStates.STARTING, MyStates.RUNNINGDEGRADED);

		// Reach the RUNNINGSOFTERRORDETECTED state
		addTransition(MyInputs.SETRUNNINGSOFTERRORDETECTED, MyStates.STARTING, MyStates.RUNNINGSOFTERRORDETECTED);

		
		// Reach the PAUSED State
		addTransition(MyInputs.SETPAUSED, MyStates.PAUSING, MyStates.PAUSED);
//		addTransition(MyInputs.SETPAUSED, MyStates.RECOVERING, MyStates.PAUSED);

		// Reach the RUNNING and RUNNINGDEGRADED states from RESUMING State
		addTransition(MyInputs.SETRESUMED, MyStates.RESUMING, MyStates.RUNNING);
		addTransition(MyInputs.SETRESUMEDDEGRADED, MyStates.RESUMING, MyStates.RUNNINGDEGRADED);
		addTransition(MyInputs.SETRESUMEDSOFTERRORDETECTED, MyStates.RESUMING, MyStates.RUNNINGSOFTERRORDETECTED);
		
		// Switch between RUNNING and RUNNINGDEGRADED states
		addTransition(MyInputs.SETRUNNING, MyStates.RUNNINGDEGRADED, MyStates.RUNNING);
		addTransition(MyInputs.SETRUNNINGDEGRADED, MyStates.RUNNING, MyStates.RUNNINGDEGRADED);
		
		// Switch to RUNNINGSOFTERRORDETECTED state
		addTransition(MyInputs.SETRUNNINGSOFTERRORDETECTED, MyStates.RUNNINGDEGRADED, MyStates.RUNNINGSOFTERRORDETECTED);
		addTransition(MyInputs.SETRUNNINGSOFTERRORDETECTED, MyStates.RUNNING, MyStates.RUNNINGSOFTERRORDETECTED);
		
		// Reach the FIXINGSOFTERROR state
		addTransition(MyInputs.FIXSOFTERROR, MyStates.RUNNING, MyStates.FIXINGSOFTERROR);
		addTransition(MyInputs.FIXSOFTERROR, MyStates.RUNNINGDEGRADED, MyStates.FIXINGSOFTERROR);
		addTransition(MyInputs.FIXSOFTERROR, MyStates.RUNNINGSOFTERRORDETECTED, MyStates.FIXINGSOFTERROR);
	
		// Reach RUNNING and RUNNINGDEGRADED states from FIXINGSOFTERROR state
		addTransition(MyInputs.SETRUNNING, MyStates.FIXINGSOFTERROR, MyStates.RUNNING);
		addTransition(MyInputs.SETRUNNINGDEGRADED, MyStates.FIXINGSOFTERROR, MyStates.RUNNINGDEGRADED);
		
	}

}