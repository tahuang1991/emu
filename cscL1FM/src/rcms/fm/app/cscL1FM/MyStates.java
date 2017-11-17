package rcms.fm.app.cscL1FM;

import rcms.statemachine.definition.State;

/**
 * Defined Level 1 Function Manager states.
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini, Hannes Sakulin
 */
public final class MyStates {

	// Defined steady and transitional states for Level 1 Function Manager
	public static final State INITIALIZING = new State("Initializing");

	public static final State INITIAL = new State("Initial");

	public static final State HALTING = new State("Halting");

	public static final State HALTED = new State("Halted");

	public static final State CONFIGURING = new State("Configuring");

	public static final State CONFIGURED = new State("Configured");

	public static final State STARTING = new State("Starting");

	public static final State RESUMING = new State("Resuming");

	public static final State RUNNING = new State("Running");
	
	public static final State RUNNINGDEGRADED = new State("RunningDegraded");

	public static final State STOPPING = new State("Stopping");

	public static final State PAUSING = new State("Pausing");

	public static final State PAUSED = new State("Paused");

	public static final State RECOVERING = new State("Recovering");

	public static final State RESETTING = new State("Resetting");

	public static final State ERROR = new State("Error");
	
	public static final State PREPARING_TTSTEST_MODE = new State("PreparingTTSTestMode");
	
	public static final State TTSTEST_MODE = new State("TTSTestMode");

	public static final State TESTING_TTS = new State("TestingTTS");
	
	public static final State COLDRESETTING = new State("ColdResetting");

	public static final State RUNNINGSOFTERRORDETECTED = new State("RunningSoftErrorDetected");

	public static final State FIXINGSOFTERROR = new State("FixingSoftError");
	
	// Defined XDAQ EVB States
	//public static final State XDAQHALTED = new State("Halted");

	//public static final State XDAQREADY = new State("Ready");

	//public static final State XDAQENABLED = new State("Enabled");

	//public static final State UNDEFINED = new State("Undefined");
}