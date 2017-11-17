package rcms.fm.app.cscL1FM;


import rcms.fm.fw.parameter.CommandParameter;
import rcms.fm.fw.parameter.ParameterSet;
import rcms.fm.fw.user.UserActionException;
import rcms.fm.fw.user.UserFunctionManager;
import rcms.fm.resource.QualifiedResourceContainer;
import rcms.fm.resource.qualifiedresource.XdaqApplicationContainer;
import rcms.statemachine.definition.State;
import rcms.statemachine.definition.StateMachineDefinitionException;
import rcms.util.logger.RCMSLogger;

/**
 * Example of Function Machine for controlling an Level 1 Function Manager.
 * 
 * @author Andrea Petrucci, Alexander Oh, Michele Gulmini
 */
public class MyFunctionManager extends UserFunctionManager {

	/**
	 * <code>RCMSLogger</code>: RCMS log4j Logger
	 */
	static RCMSLogger logger = new RCMSLogger(MyFunctionManager.class);

	/**
	 * define some containers
	 */
	public XdaqApplicationContainer containerXdaqApplication = null;

	/**
	 * define specific application containers
	 */
	public XdaqApplicationContainer cEVM = null;

	/**
	 * <code>containerXdaqExecutive</code>: container of XdaqExecutive in the
	 * running Group.
	 */
	public XdaqApplicationContainer containerXdaqExecutive = null;

	/**
	 * <code>containerFunctionManager</code>: container of FunctionManagers
	 * in the running Group.
	 */
	public QualifiedResourceContainer containerFunctionManager = null;

	/**
	 * <code>containerJobControl</code>: container of JobControl in the
	 * running Group.
	 */
	public QualifiedResourceContainer containerJobControl = null;

	/**
	 * <code>calcState</code>: Calculated State.
	 */
	public State calcState = null;

	// In the template FM we store whether we are degraded in a boolean
	boolean degraded = false;         

	// In the template FM we store whether we have detected a softError in a boolean
	boolean softErrorDetected=false;

	
	/**
	 * Instantiates an MyFunctionManager.
	 */
	public MyFunctionManager() {
		//
		// Any State Machine Implementation must provide the framework
		// with some information about itself.
		//

		// make the parameters available
		addParameters();

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see rcms.statemachine.user.UserStateMachine#createAction()
	 */
	public void createAction(ParameterSet<CommandParameter> pars) throws UserActionException {
		//
		// This method is called by the framework when the Function Manager is
		// created.

		System.out.println("createAction called.");
		logger.debug("createAction called.");

		System.out.println("createAction executed.");
		logger.debug("createAction executed.");

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see rcms.statemachine.user.UserStateMachine#destroyAction()
	 */
	public void destroyAction() throws UserActionException {
		//
		// This method is called by the framework when the Function Manager is
		// destroyed.
		//
	    qualifiedGroup.destroy();
	    
		System.out.println("destroyAction called");
		logger.debug("destroyAction called");

		// do nothing

		System.out.println("destroyAction executed");
		logger.debug("destroyAction executed");
	}

	/**
	 * add parameters to parameterSet. After this they are accessible.
	 */
	private void addParameters() {

		// add parameters to parameter Set so they are visible.
		parameterSet = MyParameters.LVL_ONE_PARAMETER_SET;

	}

	public void init() throws StateMachineDefinitionException,
			rcms.fm.fw.EventHandlerException {

		//
		// Set first of all the State Machine Definition
		//
		setStateMachineDefinition(new MyStateMachineDefinition());

		//
		// Add event handler
		//
		addEventHandler(new MyEventHandler());

		//
		// Add error handler
		//
		addEventHandler(new MyErrorHandler());

	}
	
	public boolean isDegraded() {
		// FM may check whether it is currently degraded if such functionality exists
		return degraded;
	}
	
	public boolean hasSoftError() {
		// FM may check whether the system has a soft error if such functionality exists
		return softErrorDetected;
	}
	
	// only needed if FM cannot check for degradation
	public void setDegraded(boolean degraded) {
		this.degraded = degraded;
	}
	
	// only needed if FM cannot check for softError
	public void setSoftErrorDetected(boolean softErrorDetected) {
		this.softErrorDetected = softErrorDetected;
	}
}