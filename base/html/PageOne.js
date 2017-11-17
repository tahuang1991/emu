var panels = null;
var tmbPanel = null;
var whoIsInControl = 'global';
var TCDS_system = 'pri'; // TODO: switch automatically between 'pri' and 'sec'

function onLoad() {

    var URLs = {
	PCMan:    "http://csc-sv.cms:20010/urn:xdaq-application:lid=60/ForEmuPage1",
	BlueP:    "http://csc-dcs-pc1.cms:20070/urn:xdaq-application:lid=70/ForEmuPage1",
	BlueM:    "http://csc-dcs-pc2.cms:20070/urn:xdaq-application:lid=70/ForEmuPage1",
	PCMonP:   "http://csc-dcs-pc1.cms:20040/urn:xdaq-application:lid=30/ForEmuPage1",
	PCMonM:   "http://csc-dcs-pc2.cms:20040/urn:xdaq-application:lid=30/ForEmuPage1",
	DAQDisks: "http://kvm-s3562-1-ip151-99.cms:9945/urn:xdaq-application:lid=16/retrieveCollection",
	// CSCTF:    "http://cmslas.cern.ch/emtflas/urn:xdaq-application:lid=16/retrieveCollection",
	CSCTF:    "http://l1ts-xaas.cms:9945/urn:xdaq-application:lid=16/retrieveCollection",
	// TCDS:     "http://cmslas.cern.ch/tcdslas/urn:xdaq-application:lid=16/retrieveCollection",
	TCDS:     "http://tcds-xaas.cms:9945/urn:xdaq-application:lid=16/retrieveCollection",
	FED:      "http://csc-sv.cms:20101/urn:xdaq-application:lid=66/ForEmuPage1",
	DAQ:      "http://csc-daq00.cms:20200/urn:xdaq-application:class=emu::daq::manager::Application,instance=0/ForEmuPage1",
	DQM:      "http://csc-dqm.cms:20550/urn:xdaq-application:lid=1450/ForEmuPage1"
    };
    var tmbURLs = {
	minus:'http://csc-dcs-pc2.cms:20040/urn:xdaq-application:lid=30/XmlOutput', 
	plus: 'http://csc-dcs-pc1.cms:20040/urn:xdaq-application:lid=30/XmlOutput'
    };

    var divs = document.getElementsByTagName('div');
    panels = new Array();
    var period = 10000;
    for ( var i=0; i<divs.length; i++ ){
    	var position = divs[i].id.search("_panel");
		var name = divs[i].id.slice(0, position);
		if ( name == 'TMB' ){
		    new XSLTLoader('TMB_XSL.xml','RUI-to-chamber_mapping.xml',
				   function(){
				       $('#TMB_panel').append( this.result );
				       tmbPanel = new TMBPanel( tmbURLs );
				   	}
				  	);
		}
		else if(position > 0){
		    panels.push( new Panel( name, period, URLs[name] ) );
		}
    }

    document.onmousemove = mouseMove;
    document.onmouseup = onMouseUp;
    
}

function mouseMove(ev){
    ev.preventDefault();
    //alert( panels.length );
    for ( var i=0; i<panels.length; i++ ){
	panels[i].mouseMove(ev);
    }
}

function onMouseUp(e){
    e.preventDefault();
    //alert( panels.length );
    for ( var i=0; i<panels.length; i++ ){
	panels[i].onMouseUp(e);
    }
}

//
// Utility methods
//

function formatNumber( number ){
    var n = Number( number );
    var a = Math.abs( n );
    return n.toFixed( ( a<1 ? 3 : ( a<10 ? 2 : ( a<100 ? 1 : 0 ) ) ) );
}


function printTransforms( transformList ){
    var printedList = '';
    for ( var i=0; i<transformList.numberOfItems; i++ ){
	printedList = printedList + printSvgMatrix( transformList.getItem(i).matrix ) + ' ';
    }
    return printedList;
}

function printSvgMatrix( matrix ){
    if ( matrix != null ){
	return printedList =  
	    '(' + matrix.a +
	    ' ' + matrix.b +
	    ' ' + matrix.c +
	    ' ' + matrix.d +
	    ' ' + matrix.e +
	    ' ' + matrix.f +
	    ')';
    }
    return 'NO MATRIX';
}

function toUnixTime( dateTime ){
    var d = new Date();
    if ( dateTime.length == 19 ){
	 // YYYY-MM-DD hh:mm:ss
	 d.setFullYear( Number( dateTime.substr( 0,4) )     );
	 d.setMonth   ( Number( dateTime.substr( 5,2) ) - 1 );
	 d.setDate    ( Number( dateTime.substr( 8,2) )     );
	 d.setHours   ( Number( dateTime.substr(11,2) )     );
	 d.setMinutes ( Number( dateTime.substr(14,2) )     );
	 d.setSeconds ( Number( dateTime.substr(17,2) )     );
    }
    else if ( dateTime.length == 23 ){
	 // YYYY-MM-DD hh:mm:ss UTC
	 d.setUTCFullYear( Number( dateTime.substr( 0,4) )     );
	 d.setUTCMonth   ( Number( dateTime.substr( 5,2) ) - 1 );
	 d.setUTCDate    ( Number( dateTime.substr( 8,2) )     );
	 d.setUTCHours   ( Number( dateTime.substr(11,2) )     );
	 d.setUTCMinutes ( Number( dateTime.substr(14,2) )     );
	 d.setUTCSeconds ( Number( dateTime.substr(17,2) )     );
    }
    else {
	 var months = { 'Jan':1, 'Feb':2, 'Mar':3, 'Apr':4, 'May':5, 'Jun':6, 'Jul':7, 'Aug':8, 'Sep':9, 'Oct':10, 'Nov':11, 'Dec':12 }
	 // Wed, Jun 10 2009 21:47:35 GMT
	 d.setUTCFullYear( Number( dateTime.substr(12,4) )     );
	 d.setUTCMonth   ( months[ dateTime.substr( 5,3) ] - 1 );
	 d.setUTCDate    ( Number( dateTime.substr( 9,2) )     );
	 d.setUTCHours   ( Number( dateTime.substr(17,2) )     );
	 d.setUTCMinutes ( Number( dateTime.substr(20,2) )     );
	 d.setUTCSeconds ( Number( dateTime.substr(23,2) )     );
    }
    return d.getTime();
}

function zeroPadTo2( number ){
     return ( number < 10 ? '0' : '' )+number.toString();
}

function timeToString( time ){
     var d = new Date();
     d.setTime( time );
     return    zeroPadTo2( d.getFullYear() )
	  +'-'+zeroPadTo2( d.getMonth()+1  )
	  +'-'+zeroPadTo2( d.getDate()     )
	  +' '+zeroPadTo2( d.getHours()    )
	  +':'+zeroPadTo2( d.getMinutes()  )
	  +':'+zeroPadTo2( d.getSeconds()  );
}


//
// XSLTLoader object
//

function XSLTLoader( xslURL, xmlURL, onCompletion ){
    // Load the XSL asynchronously. When that's loaded, load the XML asynchronously. 
    // When that, too, is loaded, perform the transformation and call onCompletion.

    var self = this;
    this.xslURL = xslURL;
    this.xmlURL = xmlURL;
    this.onCompletion = onCompletion; // user's callback
    this.xsltProcessor = new XSLTProcessor();
    this.result = null;

    this.xslRequestStateChanged = function(e){
	if ( this.readyState == 4 ){
	    self.xsltProcessor.importStylesheet( this.responseXML );
	    self.loadXML();
	}
    };
    
    this.loadXSL = function(){
	var request = new XMLHttpRequest();
	request.open("GET", this.xslURL, true);
	request.onreadystatechange=this.xslRequestStateChanged
	request.send(null);
    };
    
    this.xmlRequestStateChanged = function(e){
	if ( this.readyState == 4 ){
	    var xml = this.responseXML;
	    self.result = self.xsltProcessor.transformToFragment(xml, document);
	    // console.log( (new XMLSerializer()).serializeToString( self.result ) );
	    self.onCompletion(); // invoke user's callback
	}
    };
    
    this.loadXML = function(){
	var request = new XMLHttpRequest();
	request.open("GET", xmlURL, true);
	request.onreadystatechange=this.xmlRequestStateChanged
	request.send(null);
    };

    this.loadXSL();
}

//
// Disk object
//

function Disk( host, mount, time, state, usage, free ){
  this.host  = host ;
  this.mount = mount;
  this.time  = time ;
  this.state = state;
  this.usage = usage; // [%]
  this.free  = free ; // [MB]
}

//
// Panel object
//

function Panel( name, refreshPeriod, dataURL ) {
    var self = this; // This will be needed in member methods that work as callback functions.

    //console.log( name+' '+refreshPeriod+' '+dataURL );

    this.name = name;
    this.state = '';
    this.pageRefreshPeriod = refreshPeriod; // ms
    this.svg_element = null;
    this.g_element = null;
    this.title_element = null;
    this.value_element = null;
    this.graph_element = document.getElementById(this.name+"-graph");
    this.pad_element = null;
    this.age_element = null;
    this.coords_element = null;
    this.follow_element = null;
    this.zoom_element = null;
    this.progressTable_fragment = document.createDocumentFragment(); // To cache progress table in non-calib runs.
    this.stopTime_fragment = document.createDocumentFragment(); // To cache stop time on calib runs.
    this.dragging = false;
    this.dragStart = { x:0, y:0 };
    this.cursorOverGraph = false;
    this.following   = true;
    this.autoZooming = true;
    this.oldTransform = null;
    this.trends = new Array();
    
    this.xAxis = null;
    this.yAxis = null;
    
    this.XmlDoc    = null;
    this.Clock     = null;
    this.Subsystem = null;
    this.DataURL   = dataURL;
    
    if ( this.graph_element ){
	this.svg_element = document.getElementById(this.name+"-svgRoot");
	this.g_element = document.getElementById(this.name+"-transformer");
	if ( !this.g_element ) g_element = document.getElementById(this.name+"-scaler");
	this.title_element = document.getElementById(this.name+"-title");
	this.value_element = document.getElementById(this.name+"-lastValue");
	this.pad_element = document.getElementById(this.name+"-pad");
	this.age_element = document.getElementById(this.name+"-ageOfGraph");
	this.follow_element = document.getElementById(this.name+"-follow");
	this.zoom_element = document.getElementById(this.name+"-zoom");
    }
	
    //
    // Member methods
    //

    this.mouseMove = function (ev){
	if ( this.cursorOverGraph ) this.coords_element.firstChild.nodeValue = '('+this.xAxis.pixelToValue(ev.pageX,true)+', '+this.yAxis.pixelToValue(ev.pageY,true)+')';
	if ( this.dragging ){
	    var newTransform = " translate(" + (ev.pageX-this.dragStart.x)*this.xAxis.svgPerPixel + " " + (ev.pageY-this.dragStart.y)*this.yAxis.svgPerPixel + ") ";
	    if ( this.oldTransform ) newTransform = this.oldTransform + newTransform;
	    this.graph_element.setAttribute("transform", newTransform );
	    try{
		this.redrawAxes();
	    } catch(ex) {
		//alert( ex );
	    }
	    // var msg = 'mousemove: ';
	    // msg += ' readout: ' + this.graph_element.getAttribute("transform") + ' or ' + printTransforms( this.graph_element.transform.baseVal ) +'   ' + printTransforms( this.graph_element.transform.animVal );
	    // msg += this.xAxis.print() + '   ' + this.yAxis.print();
	    // if (this.span2_element) this.span2_element.innerHTML = msg; 
	}
    };
    
    this.onMouseUp = function (e){
	if ( document.body ) document.body.style.cursor = 'default';
	this.dragging = false;
	//var msg = 'mouseup: ';
	if ( this.graph_element ){
	    this.graph_element.transform.baseVal.consolidate();
	}
	//msg += ' readout: ' + this.graph_element.getAttribute("transform") + ' or ' + printTransforms( this.graph_element.transform.baseVal ) +'   ' + printTransforms( this.graph_element.transform.animVal );
	// if (this.span2_element) this.span2_element.innerHTML = this.xAxis.print() + '   ' + this.yAxis.print(); 
    };
    
    this.attachListeners = function (){
    	this.coords_element = document.getElementById(this.name+"-pointerCoords");
    	this.pad_element = document.getElementById(this.name+"-pad");
    	this.pad_element.onmousedown = this.onMouseDownOnPad;
    	this.pad_element.onmouseup = this.onMouseUpOnPad;
    	this.graph_element.onmouseover = this.onMouseOverGraph;
    	this.graph_element.onmouseout = this.onMouseOffGraph;
    	this.pad_element.addEventListener('DOMMouseScroll', this.onMouseWheelOnPad, false); // Firefox
    	this.pad_element.addEventListener('mousewheel', this.onMouseWheelOnPad, false); // Chrome
    	this.follow_element.onmousedown = this.onMouseDownOnFollowButton;
    	this.zoom_element.onmousedown = this.onMouseDownOnZoomButton;
    	$("#TCDS-td_value_LPMState").on("mouseover", {id: "TCDS-a_value_LPMState_tooltip", elementId: "TCDS-td_value_LPMState"}, this.onMouseOverTableData);
    	$("#TCDS-td_value_LPMState").on("mouseout", {id: "TCDS-a_value_LPMState_tooltip", elementId: "TCDS-td_value_LPMState"}, this.onMouseOutTableData);
    	$("#TCDS-td_value_CPMState").on("mouseover", {id: "TCDS-a_value_CPMState_tooltip", elementId: "TCDS-td_value_CPMState"}, this.onMouseOverTableData);
    	$("#TCDS-td_value_CPMState").on("mouseout", {id: "TCDS-a_value_CPMState_tooltip", elementId: "TCDS-a_value_CPMState"}, this.onMouseOutTableData);
    	$("#TCDS-td_value_State").on("mouseover", {id: "TCDS-a_value_State_tooltip", elementId: "TCDS-td_value_State"}, this.onMouseOverTableData);
    	$("#TCDS-td_value_State").on("mouseout", {id: "TCDS-a_value_State_tooltip", elementId: "TCDS-td_value_State"}, this.onMouseOutTableData);
    	$("#TCDS-td_value_MUTFUPState").on("mouseover", {id: "TCDS-a_value_MUTFUPState_tooltip", elementId: "TCDS-td_value_MUTFUPState"}, this.onMouseOverTableData);
    	$("#TCDS-td_value_MUTFUPState").on("mouseout", {id: "TCDS-a_value_MUTFUPState_tooltip", elementId: "TCDS-td_value_MUTFUPState"}, this.onMouseOutTableData);
    	$("#CSCTF-a_value_total_in").on("mouseover", {id: "CSCTF-a_value_total_in_tooltip", elementId: "CSCTF-a_value_total_in"}, this.onMouseOverTableData);
    	$("#CSCTF-a_value_total_in").on("mouseout", {id: "CSCTF-a_value_total_in_tooltip", elementId: "CSCTF-a_value_total_in"}, this.onMouseOutTableData);
    	$("#CSCTF-a_value_total_out").on("mouseover", {id: "CSCTF-a_value_total_out_tooltip", elementId: "CSCTF-a_value_total_out"}, this.onMouseOverTableData);
    	$("#CSCTF-a_value_total_out").on("mouseout", {id: "CSCTF-a_value_total_out_tooltip", elementId: "CSCTF-a_value_total_out"}, this.onMouseOutTableData);
    };

    this.onMouseOverTableData = function(e){
    	var dataElement = document.getElementById(e.data.elementId);
    	var dataElementTooltip = document.getElementById(e.data.id);
    	if (dataElement && dataElementTooltip){
    		tooltip(e, e.data.id);
    	}
    } 

    this.onMouseOutTableData = function(e){
    	var dataElement = document.getElementById(e.data.elementId);
    	var dataElementTooltip = document.getElementById(e.data.id);
    	if (dataElement && dataElementTooltip){
    		tooltip(e, e.data.id);
    	}
    }

    this.onMouseDownOnPad = function (e){
	//alert(self.graph_element + ' ' + self.dragStart.x );
	self.dragStart = { x:e.pageX, y:e.pageY };
	if ( document.body ) document.body.style.cursor = 'move';
	self.dragging = true;
	self.graph_element.transform.baseVal.consolidate();
	self.oldTransform = self.graph_element.getAttribute( "transform" );
	// self.printSvgSvg();
	event.preventDefault();
    };

    this.onMouseUpOnPad = function (e){
	if ( document.body ) document.body.style.cursor = 'default';
	self.dragging = false;
	event.preventDefault();
    };

    this.onMouseDownOnFollowButton = function (){
	//alert( self.following );
	var follow_rect  = document.getElementById(self.name+'-follow_rect');
	var follow_text  = document.getElementById(self.name+'-follow_text');
	var follow_edge1 = document.getElementById(self.name+'-follow_edge1');
	var follow_edge2 = document.getElementById(self.name+'-follow_edge2');
	if ( !self.following ){
	    // Start following
	    follow_rect.setAttribute( 'fill', '#333333' );
	    follow_edge1.setAttribute( 'fill', '#111111' );
	    follow_edge2.setAttribute( 'fill', '#888888' );
	    follow_text.setAttribute( 'fill', '#eeeeee' );
	    follow_text.setAttribute( 'title', 'click to stop following last point' );
	    follow_rect.setAttribute( 'title', 'click to stop following last point' );
	    follow_text.firstChild.nodeValue = 'freeze'
	    self.following = true;
	} else {
	    // Stop following
	    follow_rect.setAttribute( 'fill', '#ffffaa' );
	    follow_edge1.setAttribute( 'fill', '#996644' );
	    follow_edge2.setAttribute( 'fill', '#ffffee' );
	    follow_text.setAttribute( 'fill', '#ffaa55' );
	    follow_text.setAttribute( 'title', 'click to keep last point in view' );
	    follow_rect.setAttribute( 'title', 'click to keep last point in view' );
	    follow_text.firstChild.nodeValue = 'follow'
	    self.following = false;
	}
    };

    this.onMouseDownOnZoomButton = function (){
	//alert( self.autoZooming );
	var zoom_rect = document.getElementById(self.name+'-zoom_rect');
	var zoom_text = document.getElementById(self.name+'-zoom_text');
	if ( self.autoZooming ){
	    // Disable automatic zoom
	    zoom_text.setAttribute( 'title', 'click to let vertical range adjust automatically' );
	    zoom_rect.setAttribute( 'title', 'click to let vertical range adjust automatically' );
	    zoom_text.firstChild.nodeValue = 'auto range'
	    self.autoZooming = false;
	} else {
	    // Enable automatic zoom
	    zoom_text.setAttribute( 'title', 'click to fix vertical range' );
	    zoom_rect.setAttribute( 'title', 'click to fix vertical range' );
	    zoom_text.firstChild.nodeValue = 'fix range'
	    self.autoZooming = true;
	}
    };
    
    this.onMouseOffGraph = function (e){
	if ( document.body ) document.body.style.cursor = 'default';
	self.coords_element.style.visibility = 'hidden';
	self.cursorOverGraph = false;
    };
    
    this.onMouseOverGraph = function (e){
	if ( document.body ) document.body.style.cursor = 'crosshair';
	self.coords_element.style.visibility = 'visible';
	self.cursorOverGraph = true;
    };

    this.onMouseWheelOnPad = function (event){
	// console.log( 'Alt: ' + event.altKey + '  detail: ' + event.detail + '  deltaX: ' + event.deltaX  + '  deltaY: ' + event.deltaY  + '  deltaZ: ' + event.deltaZ + '  wheelDelta: ' + event.wheelDelta );
	var delta;
	if ( typeof event.wheelDelta === "undefined" ) delta = -event.detail;     // Firefox
	else                                           delta =  event.wheelDelta; // Chrome
	if ( event.altKey ){
	    if ( delta > 0 ){
		// Scale up in Y
		if ( document.body ) document.body.style.cursor = 'n-resize';
		self.scale( 1., 1.25 );
	    }
	    else if ( delta < 0 ){
		// Scale down in Y
		if ( document.body ) document.body.style.cursor = 's-resize';
		self.scale( 1., 0.8 );
	    }
	}
	else{
	    if ( delta > 0 ){
		// Scale up in X
		if ( document.body ) document.body.style.cursor = 'e-resize';
 		self.scale( 1.25, 1. );
	    }
	    else if ( delta < 0 ){
		// Scale down in X
		if ( document.body ) document.body.style.cursor = 'w-resize';
 		self.scale( 0.8, 1. );
	    }
	}
	// Prevent default actions caused by mouse wheel.
	event.preventDefault();
	//event.returnValue = false; // what does this do?
    };

    this.translate = function ( xDistSVG, yDistSVG ){
	var newTransformation = ' translate('+xDistSVG+','+yDistSVG+') ';
	var oldTransformation = this.graph_element.getAttribute("transform");
	if ( oldTransformation ) newTransformation = oldTransformation + newTransformation;
	this.graph_element.setAttribute("transform", newTransformation );
	this.redrawAxes();
    };
    
    this.scale = function ( xFactor, yFactor ){
	var xToOrigin = 0;
	var yToOrigin = 0;
	if ( this.graph_element.transform.animVal.numberOfItems ){
	    this.graph_element.transform.baseVal.consolidate();
	    try{ 
		var matrix = this.graph_element.transform.animVal.getItem(0).matrix;
		xToOrigin = matrix.e/matrix.a;
		yToOrigin = matrix.f/matrix.d;
	    } catch(ex){}// { alert(ex); }
	}
	// Go to the origin, scale, and get back
	var newTransformation = ' translate('+(-xToOrigin)+','+(-yToOrigin)+') scale( '+xFactor+','+yFactor+' ) translate('+xToOrigin+','+yToOrigin+') ';
	var oldTransformation = this.graph_element.getAttribute("transform");
	if ( oldTransformation ) newTransformation = oldTransformation + newTransformation;
	this.graph_element.setAttribute("transform", newTransformation );
	this.redrawAxes();    
    };
    
    this.relabelAxes = function (){
	
	var labelElements = document.getElementById(this.name+"-xAxis").getElementsByTagNameNS('http://www.w3.org/2000/svg','text');
	for ( var i=0; i<labelElements.length / 2 ; i++ ){
	    labelElements[i                           ].firstChild.nodeValue = this.xAxis.labels[i].stringValue1;
	    labelElements[i + labelElements.length / 2].firstChild.nodeValue = this.xAxis.labels[i].stringValue2;
	}
	
	labelElements = document.getElementById(this.name+"-yAxis").getElementsByTagNameNS('http://www.w3.org/2000/svg','text');
	for ( var i=0; i<labelElements.length; i++ ) labelElements[i].firstChild.nodeValue = this.yAxis.labels[i].stringValue1;
	
    };
    
    this.redrawAxes = function (){
	if ( this.graph_element ){
	    this.graph_element.transform.baseVal.consolidate();
	    if ( this.graph_element.transform.animVal.numberOfItems ){
		var matrix = this.graph_element.transform.animVal.getItem(0).matrix
		this.xAxis.transform( matrix );
		this.yAxis.transform( matrix );
	    } else {
		this.xAxis.transform( null ); // just for updating everything (will do identity transformation)
		this.yAxis.transform( null ); // just for updating everything (will do identity transformation)
	    }
	    this.relabelAxes();
	}
    };
    
    this.printSvgSvg = function (){
	var msg = 
	    '<table>' +
	    '  <tr><td>currentScale </td><td>' + this.svg_element.currentScale  + '</td></tr>' +
	    // 	'  <tr><td>pixelUnitToMillimeterX </td><td>' + this.svg_element.pixelUnitToMillimeterX  + '</td></tr>' +
	    // 	'  <tr><td>pixelUnitToMillimeterY </td><td>' + this.svg_element.pixelUnitToMillimeterY  + '</td></tr>' +
	    // 	'  <tr><td>screenPixelToMillimeterX </td><td>' + this.svg_element.screenPixelToMillimeterX  + '</td></tr>' +
	    // 	'  <tr><td>screenPixelToMillimeterY </td><td>' + this.svg_element.screenPixelToMillimeterY  + '</td></tr>' +
	    '  <tr><td>width </td><td>' + this.svg_element.width.animVal.value  + '</td></tr>' +
	    '  <tr><td>height </td><td>' + this.svg_element.height.animVal.value  + '</td></tr>' +
	    '  <tr><td>CTM </td><td>' + printSvgMatrix( this.svg_element.getCTM() ) + '</td></tr>' +
	    '  <tr><td>ScreenCTM </td><td>' + printSvgMatrix( this.svg_element.getScreenCTM() ) + '</td></tr>' +
	    '</table>';
	if (this.span3_element) this.span3_element.innerHTML = msg;
    };

    this.followLastPoint = function ( xSVGLastPoint ){
	if ( xSVGLastPoint > this.xAxis.hiSVG ) this.translate( this.xAxis.hiSVG-xSVGLastPoint, 0 );
    };

    this.centerXAxisOnValue = function ( xSVG ){
	this.translate( 0.5*(this.xAxis.loSVG+this.xAxis.hiSVG) - xSVG, 0 );
    };
    
    this.transformYToFit = function (){
	// Zoom and pan in y so that each point in range can be seen.
	var minYSVG =  0.1*Number.MAX_VALUE;
	var maxYSVG = -0.1*Number.MAX_VALUE;
	var noPointInRange = true;
	for ( var i=0; i<this.graph_element.points.numberOfItems; i++ ){
	    if ( this.xAxis.loSVG <= this.graph_element.points.getItem(i).x && this.graph_element.points.getItem(i).x <= this.xAxis.hiSVG ){
		if ( this.graph_element.points.getItem(i).y < minYSVG ) minYSVG = this.graph_element.points.getItem(i).y;
		if ( this.graph_element.points.getItem(i).y > maxYSVG ) maxYSVG = this.graph_element.points.getItem(i).y;
		noPointInRange = false;
	    }
	}
	if ( noPointInRange ) return;
	var yShiftSVG = 0.5* ( ( this.yAxis.loSVG + this.yAxis.hiSVG ) - ( minYSVG + maxYSVG ) ); // midpoint shift
	var yFactor   = ( this.yAxis.loSVG - this.yAxis.hiSVG ) / Math.max( 300., 1.1 * ( maxYSVG -  minYSVG ) );
	this.translate( 0., yShiftSVG );
	this.scale( 1., yFactor );
    };

    this.ageOfPageClock = function (ageOfPage){
    	hours=Math.floor(ageOfPage/3600);
    	minutes=Math.floor(ageOfPage/60)%60;
    	// graph
    	var age="";
    	if (hours) age+=hours+"h ";
    	if (minutes) age+=minutes+"m ";
    	age+=ageOfPage%60+"s ";
	//alert(age);
    	if ( this.age_element ){
    	    this.value_element.firstChild.nodeValue = this.value_element.firstChild.nodeValue; // Heh?! *This* prevents age_element from starting inheriting background color from the containing td element when it is updated!?
    	    this.age_element.firstChild.nodeValue = age+'ago';
    	    //age_element.setAttribute( 'style', 'background-color: #333333;' ); // Apparently, this has no effect. It keeps on inheriting the bkg color of the td element. 
    	    this.value_element.firstChild.nodeValue = this.value_element.firstChild.nodeValue; // Heh?! *This* prevents age_element from starting inheriting background color from the containing td element when it is updated!?
    	}
    	// table
    	try{
    	    age="";
    	    if (hours) age+=hours+"h&nbsp;";
    	    if (minutes) age+=minutes+"m&nbsp;";
    	    age+=ageOfPage%60+"s&nbsp;";
    	    document.getElementById(this.name+'-td_ageOfPage').innerHTML='Loaded&nbsp;'+age+'ago';
    	    var mainTableElem = document.getElementById(this.name+'-fadingTable');
    	    if      ( ageOfPage < 0.003 * this.pageRefreshPeriod ) mainTableElem.className = 'fresh';
    	    else if ( ageOfPage < 0.010 * this.pageRefreshPeriod ) mainTableElem.className = 'aging';
    	    else                                                   mainTableElem.className = 'stale';
    	} catch(ex){}// { alert(ex); }
    	ageOfPage=ageOfPage+1;
	// Two methods to pass the reference to this object:
	// * Method 1: Save this in another variable that we can use inside setTimeout.
	// var self = this;
    	this.Clock = setTimeout( function(){ self.ageOfPageClock( ageOfPage ); }, 1000 );
	// * Method 2: Pass this as the third argument. This is said not to work in IE...
    	//this.Clock = setTimeout( function( thisObj ){ thisObj.ageOfPageClock( ageOfPage ); }, 1000, this );
    };

    this.appendPoint = function ( p ){
	if ( !this.graph_element ) return;

	var xSVG = this.xAxis.toSVG( p.time );
	var ySVG;
	ySVG = this.yAxis.toSVG( p.value );
	this.title_element.firstChild.nodeValue = p.name;
	this.value_element.firstChild.nodeValue = formatNumber( p.value );

	var oldPoints = this.graph_element.getAttribute("points");
	if ( oldPoints ) this.graph_element.setAttribute("points", oldPoints+' '+xSVG+','+ySVG );
	else             this.graph_element.setAttribute("points", xSVG+','+ySVG );
	//var newPoints = graph_element.getAttribute("points");
	// Jump to the first point (in case the client's clock is not set correctly or is in another time zone):
	if ( oldPoints.length == 0 ) this.centerXAxisOnValue( xSVG );
	if ( this.following ) this.followLastPoint( xSVG );
	if ( this.autoZooming ) this.transformYToFit();
    };

    this.TrackFinderFromJson = function(){
    	$.getJSON(self.DataURL + "?fmt=json&flash=urn:xdaq-flashlist:l1ts_cell", function(json){
    		$.each(json.table.rows, function(i,row){
    			if (row.name == "EMTF SWATCH Cell"){
    				$('#'+self.name+'-td_value_state').attr( 'class', row.Operations.rows[0].state );
		    		$('#'+self.name+'-a_value_state').text( row.Operations.rows[0].state );
		    		$('#'+self.name+'-a_value_state').attr( 'title', 'The EMTF (Endcap Muon Track Finder) Controller application is '+row.Operations.rows[0].state);
    			}
    		});
    	}).success(function(){
    	    var graphPoint = null;
    	    $.getJSON(self.DataURL + "?fmt=json&flash=urn:xdaq-flashlist:emtf_cell", function(json){
		// Input (LCT) rates
		$("#CSCTF-a_value_total_in_tooltip").empty();
		$("#CSCTF-a_value_total_in_tooltip").append("<table><tbody><tr><td colspan='2'>Input LCT rates:</td></tr></tbody></table>");
		var minInputLCTRate = Number.MAX_VALUE;
		var totalInputLCTRate = 0;
		var minInputTriggerSector = undefined;
		var minInputTriggerSectorsArray = new Array();
		var sortedInputTriggerSectors = new Array();
		var nameOfMinInputTriggerSectors = "";
		// Output (track) rates
		$("#CSCTF-a_value_total_out_tooltip").empty();
		$("#CSCTF-a_value_total_out_tooltip").append("<table><tbody><tr><td colspan='2'>Output track rates:</td></tr></tbody></table>");
		var minOutputTrackRate = Number.MAX_VALUE;
		var totalOutputTrackRate = 0;
		var minOutputTriggerSector = undefined;
		var minOutputTriggerSectorsArray = new Array();
		var sortedOutputTriggerSectors = new Array();
		var nameOfMinOutputTriggerSectors = "";
		var time = toUnixTime( json.table.properties.LastUpdate );
		$('#'+self.name+'-td_localDateTime').text( timeToString( time ) );
    		$.each(json.table.rows[0].processor_EmtfProcessor.rows, function(i,row){

		    var inputLCTRate = 0;
		    $.each(row, function(name,value){
			if ( name.substr(0,8) == 'rateLct_' ) inputLCTRate += value;
			// console.log( name+":"+value+"  " );
		    });
		    if (inputLCTRate < minInputLCTRate){
			minInputLCTRate = inputLCTRate;
    			minInputTriggerSector = row.id;
    			minInputTriggerSectorsArray = [];
    			minInputTriggerSectorsArray.push(minInputTriggerSector);
		    }
    		    else if (inputLCTRate == minInputLCTRate){
    			minInputTriggerSector = row.id;
    			minInputTriggerSectorsArray.push(minInputTriggerSector);
    		    }
    		    sortedInputTriggerSectors.push({name: row.id, value: inputLCTRate});
		    totalInputLCTRate += inputLCTRate;
			
    		    var outputTrackRate = row.outputTrack0Rate + row.outputTrack1Rate + row.outputTrack2Rate;
    		    if (outputTrackRate < minOutputTrackRate){
    			minOutputTrackRate = outputTrackRate;
    			minOutputTriggerSector = row.id;
    			minOutputTriggerSectorsArray = [];
    			minOutputTriggerSectorsArray.push(minOutputTriggerSector);
    		    } 
    		    else if (outputTrackRate == minOutputTrackRate){
    			minOutputTriggerSector = row.id;
    			minOutputTriggerSectorsArray.push(minOutputTriggerSector);
    		    }
    		    sortedOutputTriggerSectors.push({name: row.id, value: outputTrackRate});
    		    totalOutputTrackRate += outputTrackRate;
    		});
    		sortedOutputTriggerSectors.sort(function(a,b){
    		    if(a.name < b.name) return -1;
    		    if(a.name > b.name) return 1;
    		    return 0;
    		});

    		sortedInputTriggerSectors.forEach(function(item,index){
		    $("#CSCTF-a_value_total_in_tooltip table tbody").append("<tr><td>" + item.name + ": </td><td>" + item.value + "</td></tr>");
    		});
    		minInputTriggerSectorsArray.forEach(function(item, index){
    		    nameOfMinInputTriggerSectors += (minInputTriggerSectorsArray[index] + ", ");
    		});
    		$("#CSCTF-a_value_min_in").text(minInputLCTRate);
    		$("#CSCTF-a_value_min_in").attr("title", "Name(s) of the minimum trigger sector(s): " + nameOfMinInputTriggerSectors);
    		$("#CSCTF-a_value_total_in").text(totalInputLCTRate);
		
    		sortedOutputTriggerSectors.forEach(function(item,index){
		    $("#CSCTF-a_value_total_out_tooltip table tbody").append("<tr><td>" + item.name + ": </td><td>" + item.value + "</td></tr>");
    		});
    		minOutputTriggerSectorsArray.forEach(function(item, index){
    		    nameOfMinOutputTriggerSectors += (minOutputTriggerSectorsArray[index] + ", ");
    		});
    		$("#CSCTF-a_value_min_out").text(minOutputTrackRate);
    		$("#CSCTF-a_value_min_out").attr("title", "Name(s) of the minimum trigger sector(s): " + nameOfMinOutputTriggerSectors);
    		$("#CSCTF-a_value_total_out").text(totalOutputTrackRate);
    		graphPoint = { name:'Total track rate [kHz]', time: time, value:totalOutputTrackRate*0.001 };
    		self.appendPoint(graphPoint);
		
    	    });
    	    clearTimeout(self.Clock);
	    self.ageOfPageClock(0);
    	});
    }

    this.TCDSFromJson = function (){
	// Get info on TCDS
	var isFirstCall = false;
	if ( self.trends.length == 0 ){
	    self.trends[0] = new Trend(2); // container to keep the last two HardReset counts in
	    self.trends[1] = new Trend(2); // container to keep the last two changed HardReset counts in
	    self.trends[2] = new Trend(2); // container to keep the LPM L1A counts in
	    isFirstCall = true; // So that we know that this is the first call after this page was loaded, and percieve a HardReset count of 0 on the second call as a change from the previous one.
	}
	$.getJSON( self.DataURL+'?fmt=json&flash=urn:xdaq-flashlist:tcds_common', function(json){
	    var combinedState = null;
	    $("#TCDS-a_value_LPMState_tooltip").empty();
	    $("#TCDS-a_value_CPMState_tooltip").empty();
	    $("#TCDS-a_value_State_tooltip").empty();
	    $("#TCDS-a_value_State_tooltip").append("<table><tbody></tbody></table>");
	    $.each( json.table.rows, function(i,row){
		if ( row.service.search('i-csc[^ ]*-'+TCDS_system) >= 0 ){
		    if ( combinedState && combinedState != row.state_name ) combinedState = 'INDEFINITE';
		    else                                                    combinedState = row.state_name;
		    // console.log( row.service+' '+row.state_name+' '+combinedState );
		    //console.log($("#TCDS-a_value_State_tooltip") + "valami");
		    if (row.problem_description == "-"){
		    	$("#TCDS-a_value_State_tooltip table tbody").append("<tr><td>" + row.service + ": </td><td class='" + row.state_name + "'>" + row.state_name + "</td></tr>");
		    }
		    else {
		    	$("#TCDS-a_value_State_tooltip table tbody").append("<tr><td>" + row.service + ": </td><td class='" + row.state_name + "'>" + row.state_name + "</td><td>Problem description: </td><td class='ERROR'>" + row.problem_description + "</td></tr>");
		    }		    
		}
		else if ( row.service == 'lpm-csc-'+TCDS_system ){
		    $('#'+self.name+'-td_value_LPMState').attr( 'class', row.state_name );
		    $('#'+self.name+'-a_value_LPMState').text( row.state_name );
		    $('#'+self.name+'-a_value_LPMState').attr( 'title', 'The LPM (Local Partition Manager) Controller application is '+row.state_name);
		    if (row.problem_description != "-"){
		    	$("#TCDS-a_value_LPMState_tooltip").append("<p>Problem description: " + row.problem_description + "</p>");
		    }
		}
		else if ( row.service == 'cpm-'+TCDS_system ){
		    $('#'+self.name+'-td_value_CPMState').attr( 'class', row.state_name );
		    $('#'+self.name+'-a_value_CPMState').text( row.state_name );
		    $('#'+self.name+'-a_value_CPMState').attr( 'title', 'The CPM (Central Partition Manager) Controller application is '+row.state_name);
		    if (row.problem_description != "-"){
		    	$("#TCDS-a_value_CPMState_tooltip").append("<p>Problem description: " + row.problem_description + "</p>");
		    }
		}
	    });
	    $('#'+self.name+'-td_value_State').attr( 'class', combinedState );
	    $('#'+self.name+'-a_value_State').text( combinedState );
	    $('#'+self.name+'-a_value_State').attr( 'title', (combinedState == 'INDEFINITE' ? 'Not all TCDS CI and PI Controller applications are in the same FSM state.' : 'All TCDS CI and PI Controller applications are '+combinedState ) );
	    
	}).success( function(){
	    if ( whoIsInControl == 'global' ){
	      $.getJSON( self.DataURL+'?fmt=json&flash=urn:xdaq-flashlist:tcds_cpm_rates', function(json){
 	      	  var time = toUnixTime( json.table.properties.LastUpdate );
	      	  $('#'+self.name+'-td_localDateTime').text( timeToString( time ) );
	      	  var totalTriggerRate = 0;
	      	  $.each( json.table.rows, function(i,row){
	      	      if ( row.service == 'cpm-'+TCDS_system ) totalTriggerRate = row.trg_rate_total;
	      	    });
	    	  var graphPoint = { name:'Total '+(TCDS_system=='pri'?'primary':'secondary')+' CPM trigger [kHz]', time:time, value:totalTriggerRate*0.001 };
	    	  self.appendPoint( graphPoint );
	    	  // }).success( function(){
	    	  clearTimeout(self.Clock);
	    	  self.ageOfPageClock(0);
	    	});
	    }
	    else{
		$.getJSON('http://tcds-control-csc-'+TCDS_system+'.cms:2104/urn:xdaq-application:service=lpm-csc-'+TCDS_system+'/update', function(json){
		    var time = toUnixTime( json["Application state"]["Latest monitoring update time"] );
		    $('#'+self.name+'-td_localDateTime').text( timeToString( time ) );
		    var L1As = json["itemset-trigger-counter"]["# L1As"];
		    self.trends[2].add( time, Number(L1As) );
		    var graphPoint = { name:'CSC LPM L1A rate [Hz]', time:time, value:self.trends[2].rate( 2 ) };
		    self.appendPoint( graphPoint );
		    clearTimeout(self.Clock);
		    self.ageOfPageClock(0);
		});
	    }
			  
	});

	//$.getJSON("http://cmslas.cern.ch/emtflas/urn:xdaq-application:lid=16/retrieveCollection?fmt=json&flash=urn:xdaq-flashlist:l1ts_cell", function(json){
	$.getJSON("http://l1ts-xaas.cms:9945/urn:xdaq-application:lid=16/retrieveCollection?fmt=json&flash=urn:xdaq-flashlist:l1ts_cell", function(json){
		var combinedState = null;
		$("#TCDS-a_value_MUTFUPState_tooltip").empty();
		$("#TCDS-a_value_MUTFUPState_tooltip").append("<table><tbody></tbody></table>");
		$.each(json.table.rows, function(i,row){
			if(row.name == "MUTFUP TCDS ICI Cell" || row.name == "MUTFUP TCDS PI Cell"){
				if(combinedState && combinedState != row.Operations.rows[0].state) combinedState = "INDEFINITE";
				else 															   combinedState = row.Operations.rows[0].state;
				$("#TCDS-a_value_MUTFUPState_tooltip table tbody").append("<tr><td>" + row.name + ": </td><td class='" + row.Operations.rows[0].state + "'>" + row.Operations.rows[0].state + "</td></tr>");
			}
		});
		$("#" + self.name + "-td_value_MUTFUPState").attr("class", combinedState);
		$("#" + self.name + "-a_value_MUTFUPState").text( combinedState );
		$("#" + self.name + "-a_value_MUTFUPState").attr("title", (combinedState == "INDEFINITE" ? 'MUTFUP TCDS CI and PI Controller applications are not in the same FSM state.' : 'MUTFUP TCDS CI and PI Controller applications are '+combinedState ));
	});

 	// Get TCDS PI spy log
	$.getJSON('http://tcds-control-csc-'+TCDS_system+'.cms:2104/urn:xdaq-application:service=pi-cscm-'+TCDS_system+'/update', function(json){
	    // var msg='';
	    var time = toUnixTime( json["Application state"]["Latest monitoring update time"] );
	    var nHardResets=0;
	    $.each( json["itemset-ttcspylog"], function(k,v){
	    	// msg+=' '+k+'\n';
	    	$.each( v, function(l,u){
		    if ( u["Data"] == 0x10 ){
			nHardResets++;
	    		// msg+='  '+ nHardResets +' '+l+' '+u["Delta"].split(' ')[0]+' '+u["Data"]+'\n';
		    }
	    	});
	    });
	    // Add this value to the container of counts:
	    self.trends[0].add( time, nHardResets );
	    if ( self.trends[0].difference( 2 ) > 0 || isFirstCall ){
		// The count changed w.r.t. the previous reading, or this is the first call. Add the new value to the container of changed values:
		self.trends[1].add( time, nHardResets );
	    }
	    //console.log( 'self.trends[1].points.length ' + self.trends[1].points.length );
	    if ( isFirstCall ){
		// First call. Wait for the next one.
		$('#TCDS-a_value_LastHardReset').text( 'reading...' ).attr('title','Please, wait for the next automatic refresh.');
	    }
	    else{
		if ( nHardResets == 0 ){
		    $('#TCDS-a_value_LastHardReset').text( 'none' + String.fromCharCode(160) + 'yet' ).attr('title','No HardReset yet, at least since the PI Spy was last configured. Click to visit the plus-side PI.');
		}
		else{
		    var timeOfLastHardReset = self.trends[1].points[self.trends[1].points.length-1].time;
		    if ( self.trends[1].points.length > 1 ){
			// We have seen more than one change. (One change may just mean that the first reading was non-zero, in which case we don't know when it incremented last.)
			$('#TCDS-a_value_LastHardReset').text( timeToString( timeOfLastHardReset ) + String.fromCharCode(160,177) + '15s' ).attr('title','This time is accurate to wothin ~15s. '+nHardResets+' HardReset(s) issued since the last time the PI Spy was configured. Click to visit the plus-side PI.');
		    }
		    else{
			// We don't know when the first change actually happened, so we just give an upper limit.
			$('#TCDS-a_value_LastHardReset').text( 'before' + String.fromCharCode(160) + timeToString( timeOfLastHardReset ) ).attr('title','Not sure when the last HardReset was issued as it may have been before this page was loaded. '+nHardResets+' HardReset(s) issued since the last time the PI Spy was configured. Click to visit the plus-side PI.');
		    }
		}
	    }
	    // msg = json["Application state"]["Latest monitoring update time"] + ' ' + time + ' ' + msg; 
            // console.log( msg );
	});
   }


    this.diskUsageFromJson = function(){
	// Get data disk info
	$.getJSON( this.DataURL+'?fmt=json&flash=urn:xdaq-flashlist:diskInfo', function(json){
	    var time = toUnixTime( json.table.properties.LastUpdate );
	    $('#'+self.name+'-td_localDateTime').text( timeToString( time ) );
	    var disks  = new Array();
	    $.each( json.table.rows, function(i,contextRow){
		if ( contextRow.context.lastIndexOf(':9999') > 0 ){
		    $.each( contextRow.diskUsage.rows, function(j,fsRow){ 
			if ( fsRow.fileSystem == '/data' ){
			    try{
				// Select local DAQ farm machines that read out DDUs
				var matches = contextRow.context.match('^http://([sS][rR][vV]-[cC]2[dD]08-(0[7-9]|1[0-9])-01(.cms)?):[0-9]+');
				if ( matches.length > 1 ){
				    disks.push( new Disk( matches[1], '/data', fsRow.sampleTime, fsRow.state, 
							  fsRow.usePercent, (1.-0.01*fsRow.usePercent)*fsRow.totalMB ) );
				}
			    }
			    catch(e){}
			}
		    });
		}
	    });
	    // Display disk of highest usage
	    if ( disks.length > 0 ){
		var d = disks.sort( function( a, b ){ return b.usage - a.usage; } )[0]; // Sort function must return a pos/neg number. With boolean, e.g. a>b, it doesn't work in WebKit (e.g., in rekonq).
		var klass = 'ON';
		if      ( d.usage > 80 ) klass = 'WARN';
		else if ( d.usage > 95 ) klass = 'OFF';
		$('#'+self.name+'-td_value_0').attr('class',klass);
		$('#'+self.name+'-a_value_0').attr('title',d.host+':'+d.mount+' has '+formatNumber(d.free)+' MB free left at '+d.time+'.').text(d.usage.toFixed(0)+' %');
	    }
	})
	    .success( function(){
		clearTimeout(self.Clock);
		self.ageOfPageClock(0);
	    });

    }


    this.getXML = function(){
	var msg = 'getXML ';
	
	// $.ajax({ url:     self.DataURL, 
	// 	 success: function(xml){
	// 	     $('monitorable',xml).each( 
	// 		 function(m){
	// 		     msg = msg + $(this).attr('name') + ', ';
	// 		 }
	// 	     );
	// 	 }
	//        })
	//     .complete( function(){ alert( msg ); } );

	//msg = msg + ' ' + self.name+'-td_localDateTime ' + document.getElementById( 'blabla' );
	//alert( msg );

	$.get( self.DataURL, {},
	       function(xml) {
		   var time;
		   $('ForEmuPage1',xml).each( 
		       function(){
			   $('#'+self.name+'-td_localDateTime').text( $(this).attr('localDateTime') );
			   time = $(this).attr('localUnixTime')* 1000; // ms in JavaScript!
			   if ( isNaN( time ) ) time = toUnixTime( $(this).attr('localDateTime') ); // PC Manager doesn't report localUnixTime...
		       }
		   );
		   $('monitorable',xml).each( 
		       function(){
			   if ( self.name == 'FED' ){
			       if ( self.trends.length == 0 ){
				   self.trends[0] = new Trend(6); // for + side errors
				   self.trends[1] = new Trend(6); // for - side errors
				   self.trends[2] = new Trend(6); // for TF     errors
			       }
			       if      ( $(this).attr('name') == 'title' ){
				   $('#FED-a_value_Title').attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'State' ){
				   $('#FED-a_name_State'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			           $('#FED-a_value_State' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			           $('#FED-td_value_State').attr('class',$(this).attr('value'   ));
				   self.state = $(this).attr('value');
			       }
			       else if ( $(this).attr('name') == 'ME+ Errors' ){
				   self.trends[0].add( time, Number($(this).attr('value')) );
				   $('#FED-td_value_ErrorsPlus').attr('class',( self.trends[0].rate(6) <= 0 ? '' : 'WARN' ));
				   $('#FED-a_name_ErrorsPlus'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('ME+');
			           $('#FED-a_value_ErrorsPlus' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'ME- Errors' ){
				   self.trends[1].add( time, Number($(this).attr('value')) );
				   $('#FED-td_value_ErrorsMinus').attr('class',( self.trends[1].rate(6) <= 0 ? '' : 'WARN' ));
				   $('#FED-a_name_ErrorsMinus'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('ME−');
			           $('#FED-a_value_ErrorsMinus' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'TF Errors' ){
				   self.trends[2].add( time, Number($(this).attr('value')) );
				   $('#FED-td_value_ErrorsTF').attr('class',( self.trends[2].rate(6) <= 0 ? '' : 'WARN' ));
				   $('#FED-a_name_ErrorsTF'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('TF');
			           $('#FED-a_value_ErrorsTF' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'ME+ Configuration' ){
				   $('#FED-a_name_ConfigPlus'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('ME+');
			           $('#FED-a_value_ConfigPlus' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'ME- Configuration' ){
				   $('#FED-a_name_ConfigMinus'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('ME−');
			           $('#FED-a_value_ConfigMinus' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'TF Configuration' ){
				   $('#FED-a_name_ConfigTF'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('TF');
			           $('#FED-a_value_ConfigTF' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'DCC Total Output Rate' ){
			           $('#FED-td_name_DCC_out' ).attr('class', ( $(this).attr('value') == 0 && self.state == 'Enabled' ? 'WARN' : '' ));
				   $('#FED-a_name_DCC_out'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('Total Out');
			           $('#FED-a_value_DCC_out' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text( formatNumber(Number($(this).attr('value'))*0.001)+' kB/s' );
			       }
			       else if ( $(this).attr('name') == 'DCC Total Input Rate' ){
			           $('#FED-td_name_DCC_in' ).attr('class', ( $(this).attr('value') == 0 && self.state == 'Enabled' ? 'WARN' : '' ));
				   $('#FED-a_name_DCC_in'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('Total In');
			           $('#FED-a_value_DCC_in' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text( formatNumber(Number($(this).attr('value'))*0.001)+' kB/s' );
				   if ( time > 0 ){
				       var graphPoint = { name: 'DCC input [kB/s]', 
							  time:  time, 
							  value: Number($(this).attr('value'))*0.001 // B --> kB
							};
				       self.appendPoint( graphPoint );
				   }
				   
			       }
			   }
			   else if ( self.name == 'DAQ' ){
			       if ( self.trends.length == 0 ){
				   self.trends[0] = new Trend(2); // for min events
				   self.trends[1] = new Trend(2); // for max events
			       }
			       if      ( $(this).attr('name') == 'title' ){
				   $('#DAQ-a_value_Title').attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'state' ){
				   $('#DAQ-a_name_State'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			           $('#DAQ-a_value_State' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			           $('#DAQ-td_value_State').attr('class',$(this).attr('value'   ));
				   self.state = $(this).attr('value');
			       }
			       else if ( $(this).attr('name') == '#' ){
				   $('#DAQ-a_name_RunNum'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			           $('#DAQ-a_value_RunNum' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'type' ){
				   $('#DAQ-a_name_RunType'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			           $('#DAQ-a_value_RunType' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'ctrl' ){
				   whoIsInControl = $(this).attr('value');
				   $('#DAQ-a_name_RunCtrl'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			           $('#DAQ-a_value_RunCtrl' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'start' ){
				   $('#DAQ-a_name_Start'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			           $('#DAQ-a_value_Start' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'stop' ){
				   var runType = $('monitorable[name="type"]',xml).attr('value');
				   // if ( Math.random() > 0.5 ){
				   if ( runType.indexOf('Calib') == 0 ){
				       // Save the stop-time anchor and remove it:
			               if ( $('#DAQ-a_value_Stop' ).length > 0 ){
					   self.stopTime_fragment.appendChild( document.getElementById('DAQ-a_value_Stop') );
					   $('#DAQ-a_value_Stop' ).remove();
				       }
				       // Reinsert the calib progress table from the doc fragment cache:
				       if ( $('#DAQ-table_Progress' ).length == 0 ){
					   $('#DAQ-td_value_Stop').append( self.progressTable_fragment );
				       }
				       $('#DAQ-a_name_Stop'  ).attr('href' ,'').attr('title','Progress of the calibration run sequence.').text('progress');
				       var iR = $('monitorable[name="calib runIndex"]' ,xml).attr('value');
				       var nR = $('monitorable[name="calib nRuns"]'    ,xml).attr('value');
				       var iS = $('monitorable[name="calib stepIndex"]',xml).attr('value');
				       var nS = $('monitorable[name="calib nSteps"]'   ,xml).attr('value');
				       $('#DAQ-progress_Runs' ).attr('value',iR).attr('max',nR)
					   .attr('title','Finished '+iR+' runs in a sequence of '+nR+' calibration runs.');
				       $('#DAQ-progress_Steps').attr('value',iS).attr('max',nS)
					   .attr('title','Done step '+iS+' of '+nS+'.');
				   }
				   else{
				       // Save the calib progress table and remove it:
			               if ( $('#DAQ-table_Progress' ).length > 0 ){
					   self.progressTable_fragment.appendChild( document.getElementById('DAQ-table_Progress') );
					   $('#DAQ-table_Progress' ).remove();
				       }
				       // Reinsert the stop-time anchor from the doc fragment cache:
				       if ( $('#DAQ-a_value_Stop' ).length == 0 ){
					   $('#DAQ-td_value_Stop').append( self.stopTime_fragment );
				       }
				       $('#DAQ-a_name_Stop'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			               $('#DAQ-a_value_Stop' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
				   }

			       }
			       else if ( $(this).attr('name') == 'min events' ){
				   var klass = '';
				   if ( time > 0 ){
				       self.trends[0].add( time, Number($(this).attr('value')) );
				       if ( self.trends[0].rate(2) == 0 && self.state == 'Enabled' ) klass = 'WARN';
				   }
				   $('#DAQ-td_value_Min').attr('class',klass);
				   $('#DAQ-a_name_Min'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('min');
			           $('#DAQ-a_value_Min' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'max events' ){
				   var klass = '';
				   if ( time > 0 ){
				       self.trends[1].add( time, Number($(this).attr('value')) );
				       if ( self.trends[1].rate(2) == 0 && self.state == 'Enabled' ) klass = 'WARN';
				       var graphPoint = { name: 'max rate [event/s]', 
							  time:  time, 
							  value: self.trends[1].rate(2)
							};
				       self.appendPoint( graphPoint );
				   }
				   $('#DAQ-td_value_Max').attr('class',klass);
				   $('#DAQ-a_name_Max'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('max');
			           $('#DAQ-a_value_Max' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			   }
			   else if ( self.name == 'DQM' ){
			       if      ( $(this).attr('name') == 'title' ){
				   $('#DQM-a_value_Title').attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'state' ){
				   $('#DQM-a_name_State'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			           $('#DQM-a_value_State' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			           $('#DQM-td_value_State').attr('class',$(this).attr('value'   ));
				   self.state = $(this).attr('value');
			       }
			       else if ( $(this).attr('name') == 'cscrate' ){
				   $('#DQM-a_name_CSC'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('CSC');
			           $('#DQM-a_value_CSC' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
				   $('#DQM-td_value_CSC').attr('class',( $(this).attr('value') == 0 && self.state == 'Enabled' ? 'WARN' : '' ));
			       }
			       else if ( $(this).attr('name') == 'evtrate' ){
				   $('#DQM-a_name_Event'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text('event');
			           $('#DQM-a_value_Event' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
				   $('#DQM-td_value_Event').attr('class',( $(this).attr('value') == 0 && self.state == 'Enabled' ? 'WARN' : '' ));
			       }
			       else if ( $(this).attr('name') == 'quality' ){
				   $('#DQM-a_name_Quality'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ));
			           $('#DQM-a_value_Quality' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
				   if ( time > 0 ){
				       var graphPoint = { name: 'quality', 
							  time:  time, 
							  value: Number($(this).attr('value'))
							};
				       self.appendPoint( graphPoint );
				   }
			       }
			   }
			   else if ( self.name == 'PCMan' ){
			       if      ( $(this).attr('name') == 'title' ){
				   $('#PCMan-a_value_Title').attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value').replace(' ',' '));
			       }
			       else if ( $(this).attr('name') == 'State' ){
				   $('#PCMan-a_name_State'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#PCMan-a_value_State' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			           $('#PCMan-td_value_State').attr('class',$(this).attr('value'   ));
			       }
			       else if ( $(this).attr('name') == 'Key+' ){
				   $('#PCMan-a_name_KeyP'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#PCMan-a_value_KeyP' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			       else if ( $(this).attr('name') == 'Key-' ){
				   $('#PCMan-a_name_KeyM'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#PCMan-a_value_KeyM' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			       }
			   }
			   else if ( self.name == 'PCMonP' ){
			       if ( self.trends.length == 0 ) self.trends[0] = new Trend(3); // for heartbeat
			       if      ( $(this).attr('name') == 'title' ){
				   $('#PCMonP-a_value_Title').attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value').replace(' ',' '));
			       }
			       else if ( $(this).attr('name') == 'VME Access' ){
				   $('#PCMonP-a_name_Access'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#PCMonP-a_value_Access' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			           $('#PCMonP-td_value_Access').attr('class',$(this).attr('value'   ));
			       }
			       else if ( $(this).attr('name') == 'Heartbeat' ){
				   self.trends[0].add( time, Number($(this).attr('value')) );
				   $('#PCMonP-td_value_Heart').attr('class',( self.trends[0].rate(3) == 0 ? 'WARN' : '' ));
				   $('#PCMonP-a_name_Heart'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#PCMonP-a_value_Heart' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text( formatNumber( self.trends[0].rate(3) )+' Hz');
				   // alert( self.trends[0].print() );
			       }
			   }
			   else if ( self.name == 'PCMonM' ){
			       if ( self.trends.length == 0 ) self.trends[0] = new Trend(3); // for heartbeat
			       if      ( $(this).attr('name') == 'title' ){
				   $('#PCMonM-a_value_Title').attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value').replace(' ',' '));
			       }
			       else if ( $(this).attr('name') == 'VME Access' ){
				   $('#PCMonM-a_name_Access'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#PCMonM-a_value_Access' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			           $('#PCMonM-td_value_Access').attr('class',$(this).attr('value'   ));
			       }
			       else if ( $(this).attr('name') == 'Heartbeat' ){
				   self.trends[0].add( time, Number($(this).attr('value')) );
				   $('#PCMonM-td_value_Heart').attr('class',( self.trends[0].rate(3) == 0 ? 'WARN' : '' ));
				   $('#PCMonM-a_name_Heart'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#PCMonM-a_value_Heart' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text( formatNumber( self.trends[0].rate(3) )+' Hz');
			       }
			   }
			   else if ( self.name == 'BlueP' ){
			       if      ( $(this).attr('name') == 'title' ){
				   $('#BlueP-a_value_Title').attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value').replace(' ',' '));
			       }
			       else if ( $(this).attr('name') == 'VME Access' ){
				   $('#BlueP-a_name_Access'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#BlueP-a_value_Access' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			           $('#BlueP-td_value_Access').attr('class',$(this).attr('value'   ));
			       }
			   }
			   else if ( self.name == 'BlueM' ){
			       if      ( $(this).attr('name') == 'title' ){
				   $('#BlueM-a_value_Title').attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value').replace(' ',' '));
			       }
			       else if ( $(this).attr('name') == 'VME Access' ){
				   $('#BlueM-a_name_Access'  ).attr('href' ,$(this).attr('nameURL' )).attr('title',$(this).attr('nameDescription' )).text($(this).attr('name' ).replace(' ',' '));
			           $('#BlueM-a_value_Access' ).attr('href' ,$(this).attr('valueURL')).attr('title',$(this).attr('valueDescription')).text($(this).attr('value'));
			           $('#BlueM-td_value_Access').attr('class',$(this).attr('value'   ));
			       }
			   }
		       }
		   );
	       }
	     )
	    .success( function(){
		clearTimeout(self.Clock);
		self.ageOfPageClock(0);
	    });
	    //.complete( function(){ alert( msg ); } );

    }

    this.autoReloadData = function (){
	if      ( self.name == 'DAQDisks' ) this.diskUsageFromJson();
	else if ( self.name == 'CSCTF'    ) this.TrackFinderFromJson();
	else if ( self.name == 'TCDS'     ) this.TCDSFromJson();
	else                                this.getXML();

	setTimeout( function(){ self.autoReloadData(); }, this.pageRefreshPeriod );
    };

    //
    // End of member methods' definitions
    //

    // Time development graph, if any
    if ( this.graph_element ){
	try{	
	    var d = new Date();
	    var t = d.getTime();
	    // Times in millisec!
	    this.xAxis = new Axis( this, t-300000, t+300000, 5, true,
				   Number(this.pad_element.getAttribute('x')),
				   Number(this.pad_element.getAttribute('x'))+Number(this.pad_element.getAttribute('width')),
				   1./this.svg_element.getScreenCTM().a );
	    this.yAxis = new Axis( this, 0., 1., 5, false,
				   Number(this.pad_element.getAttribute('y'))+Number(this.pad_element.getAttribute('height')),
				   Number(this.pad_element.getAttribute('y')),
				   1./this.svg_element.getScreenCTM().d );
	    this.redrawAxes();
	} catch(ex) {
	    //alert( ex );
	}

	this.attachListeners();
    }


    this.ageOfPageClock(0);
    
    // setTimeout( function(){ self.autoReloadData(); }, refreshOffset );

    this.autoReloadData();
}



//
// Axis object
//

function Axis(panel,loValue,hiValue,nLabels,isX,loSVG,hiSVG,svgPerPixel){
    this.panel = panel;
    this.loValueRef = loValue; // initial value to be preserved for reference
    this.hiValueRef = hiValue; // initial value to be preserved for reference
    this.loValue = loValue; // current value of lower end
    this.hiValue = hiValue; // current value of higher end

    this.nLabels = nLabels;
    this.isX = isX;

    this.loSVGRef = loSVG; // initial SVG coord to be preserved for reference
    this.hiSVGRef = hiSVG; // initial SVG coord  to be preserved for reference
    this.loSVG = loSVG; // current SVG coord of lower end
    this.hiSVG = hiSVG; // current SVG coord of higher end

    this.svgPerPixelRef   = svgPerPixel;
    this.lengthSVGRef     = this.hiSVGRef - this.loSVGRef;
    this.valuePerPixelRef = ( this.hiValueRef - this.loValueRef ) / this.lengthSVGRef * this.svgPerPixelRef;
    this.svgPerPixel   = this.svgPerPixelRef;
    this.lengthSVG     = this.lengthSVGRef;
    this.valuePerPixel = this.valuePerPixelRef;

    this.labels = new Array(this.nLabels);

    this.updateLabels = function(){
	var increment = ( this.hiValue - this.loValue ) / ( this.nLabels - 1 );
    	for ( var i=0; i<this.labels.length; i++ ){
	    if ( this.labels[i] ) this.labels[i].setValue( this.loValue + i * increment );
	    else this.labels[i] = new Label( this.loValue + i * increment, this.isX );
	}
    }

    this.updateScaling = function(){
	this.lengthSVG     = this.hiSVG - this.loSVG;
	this.svgPerPixel   = this.svgPerPixelRef * this.lengthSVG / this.lengthSVGRef;
	this.valuePerPixel = ( this.hiValue - this.loValue ) / this.lengthSVG * this.svgPerPixel;
    }

    this.toSVG = function( value ){
	return this.loSVGRef + ( value - this.loValueRef ) * ( this.hiSVGRef - this.loSVGRef ) / ( this.hiValueRef - this.loValueRef );
    }

    this.toValue = function( SVG ){
	return this.loValueRef + ( SVG - this.loSVGRef ) / ( this.hiSVGRef - this.loSVGRef ) * ( this.hiValueRef - this.loValueRef );
    }

    this.transform = function( svgMatrix ){
	if ( svgMatrix ){
	    if ( this.isX ){
		this.loSVG = ( this.loSVGRef - svgMatrix.e ) / svgMatrix.a;
		this.hiSVG = ( this.hiSVGRef - svgMatrix.e ) / svgMatrix.a;
	    } else {
		this.loSVG = ( this.loSVGRef - svgMatrix.f ) / svgMatrix.d;
		this.hiSVG = ( this.hiSVGRef - svgMatrix.f ) / svgMatrix.d;
	    }
	}
	this.loValue = this.toValue( this.loSVG );
	this.hiValue = this.toValue( this.hiSVG );
	this.updateScaling();
	this.updateLabels();
    }

    this.formatValue = function( value ){
	if ( this.isX ){
	    var d = new Date();
	    d.setTime( value );
	    return d.getFullYear()+'-'+(d.getMonth()+1)+'-'+d.getDate()+' '+d.getHours()+':'+d.getMinutes()+':'+d.getSeconds();
	}
	else { return formatNumber( value ); }
    }

    this.pixelToValue = function( pixel, formatted ){
	var svgMatrix = this.panel.svg_element.getScreenCTM();
	this.panel.g_element.transform.baseVal.consolidate();
	var gMatrix = this.panel.g_element.transform.baseVal.getItem(0).matrix;
	var graphMatrix = null;
	this.panel.graph_element.transform.baseVal.consolidate();
	if ( this.panel.graph_element.transform.animVal.numberOfItems){
	    graphMatrix = this.panel.graph_element.transform.animVal.getItem(0).matrix
	}
	var SVG;
	if ( this.isX ){
	    // Transform from screen to svg:svg...
	    SVG = ( pixel - svgMatrix.e ) / svgMatrix.a;
	    // ...then from svg:svg to svg:g...
	    SVG = ( SVG - gMatrix.e ) / gMatrix.a;
	    // ...then from svg:g to svg:polyline (the graph)...
	    if ( graphMatrix ) SVG = ( SVG - graphMatrix.e ) / graphMatrix.a;
	} else {
	    // Transform from screen to svg:svg...
	    SVG = ( pixel - svgMatrix.f ) / svgMatrix.d;
	    // ...then from svg:svg to svg:g...
	    SVG = ( SVG - gMatrix.f ) / gMatrix.d;
	    // ...then from svg:g to svg:polyline (the graph)...
	    if ( graphMatrix ) SVG = ( SVG - graphMatrix.f ) / graphMatrix.d;
	}
	// ... and finally to value:
	if ( formatted ) return this.formatValue( this.toValue( SVG ) );
	return this.toValue( SVG );
    }

    this.print = function(){
	msg = 
	    '<table>' +
	    '  <tr><td>isX           </td><td>' + this.isX            + '</td></tr>' +
	    '  <tr><td>loValue       </td><td>' + this.loValue        + '</td></tr>' +
	    '  <tr><td>hiValue       </td><td>' + this.hiValue        + '</td></tr>' +
	    '  <tr><td>loValueRef    </td><td>' + this.loValueRef     + '</td></tr>' +
	    '  <tr><td>hiValueRef    </td><td>' + this.hiValueRef     + '</td></tr>' +
	    '  <tr><td>loSVG         </td><td>' + this.loSVG          + '</td></tr>' +
	    '  <tr><td>hiSVG         </td><td>' + this.hiSVG          + '</td></tr>' +
	    '  <tr><td>loSVGRef      </td><td>' + this.loSVGRef       + '</td></tr>' +
	    '  <tr><td>hiSVGRef      </td><td>' + this.hiSVGRef       + '</td></tr>' +
	    '  <tr><td>lengthSVG     </td><td>' + this.lengthSVG      + '</td></tr>' +
	    '  <tr><td>svgPerPixel   </td><td>' + this.svgPerPixel    + '</td></tr>' +
	    '  <tr><td>valuePerPixel </td><td>' + this.valuePerPixel  + '</td></tr>' +
	    '</table>';
	return msg;
    }

}


function Label(value,isTime){
    this.value = 0;
    this.isTime = isTime;
    this.stringValue1 = '';
    this.stringValue2 = ''; // YYYY-MM-DD if isTime; null otherwise

    this.zeroPadTo2 = function( number ){
	if ( number <= 9 ) return '0'+number.toString();
	return number.toString();
    }

    this.setValue = function(v){
	this.value = v;
  	if ( isTime ){
//  	if ( false ){
	    var d = new Date();
	    d.setTime(this.value);
	    this.stringValue1 = this.zeroPadTo2(d.getHours())+':'+this.zeroPadTo2(d.getMinutes())+':'+this.zeroPadTo2(d.getSeconds());
	    this.stringValue2 = d.getFullYear().toString().substr(2,2)+'-'+this.zeroPadTo2(d.getMonth()+1)+'-'+this.zeroPadTo2(d.getDate());
	} else {
	    this.stringValue1 = formatNumber( this.value );
	}
    }

    this.setValue( value );
}


function Trend( maxSize ){
    this.maxSize = Math.max( 2, maxSize ); // at least 2
    this.points = new Array();
    this.add = function( t, v ){
	if ( this.points.push( { time:t, value:v } ) > this.maxSize ) while( this.points.length > this.maxSize ) this.points.shift(); // keep size at maxSize at most
    };
    this.rate = function( sampleSize ){
	if ( this.points.length < 2 ) return 0;
	var iLast = Math.min( this.points.length, Math.max( 2, sampleSize ) ) - 1;
	if ( this.points[iLast].time <= this.points[0].time ) return 0;
	return Math.max( 0, ( this.points[iLast].value - this.points[0].value ) * 1000. / ( this.points[iLast].time - this.points[0].time ) ); // ms --> s
    };
    this.difference = function( sampleSize ){
	if ( this.points.length < 2 ) return 0;
	var iLast = Math.min( this.points.length, Math.max( 2, sampleSize ) ) - 1;
	return ( this.points[iLast].value - this.points[0].value );
    };
    this.print = function(){
	var str = 'maxSize '+this.maxSize+' length '+this.points.length+' ';
	for ( var i=0; i<this.points.length; i++ ) str = str+'( '+this.points[i].time+','+this.points[i].value+') ';
	return str;
    }
}


function appendPoint( p ){
    if ( !graph_element ) return;

    var xSVG = xAxis.toSVG( p.time );
    var ySVG;
    ySVG = yAxis.toSVG( p.value );
    title_element.firstChild.nodeValue = p.name;
    value_element.firstChild.nodeValue = formatNumber( p.value );

    var oldPoints = graph_element.getAttribute("points");
    if ( oldPoints ) graph_element.setAttribute("points", oldPoints+' '+xSVG+','+ySVG );
    else             graph_element.setAttribute("points", xSVG+','+ySVG );
    //var newPoints = graph_element.getAttribute("points");
    previousPoint.time  = p.time;
    previousPoint.value = p.value;
    // Jump to the first point (in case the client's clock is not set correctly or is in another time zone):
    if ( oldPoints.length == 0 ) centerXAxisOnValue( xSVG );
    if ( following ) followLastPoint( xSVG );
    if ( autoZooming ) transformYToFit();
}

function Monitorable( time, name, value, nameDescr, valueDescr, nameURL, valueURL ){
  this.time       = time;
  this.name       = name;
  this.value      = value;
  this.nameDescr  = nameDescr;
  this.valueDescr = valueDescr;
  this.nameURL    = nameURL;
  this.valueURL   = valueURL;
  
  this.previousTime  = this.time;
  this.previousValue = this.value;
  this.prevPrevTime  = this.time;
  this.prevPrevValue = this.value;
  
  this.set = function( time, name, value, nameDescr, valueDescr, nameURL, valueURL ){
    this.prevPrevTime  = this.previousTime;
    this.prevPrevValue = this.previousValue;
    this.previousTime  = this.time;
    this.previousValue = this.value;

    this.time       = time;
    this.value      = value;
    this.nameDescr  = nameDescr;
    this.valueDescr = valueDescr;
    this.nameURL    = nameURL;
    this.valueURL   = valueURL;
  };

  this.rate = function(){
    if ( this.time == this.previousTime ) return 0;
    return Math.max( 0, (this.value-this.previousValue) * 1000 / (this.time-this.previousTime) ); // ms --> s
  };

  this.rate2 = function(){ // rate calculated over two samplings
    if ( this.time == this.prevPrevTime ) return 0;
    return Math.max( 0, (this.value-this.prevPrevValue) * 1000 / (this.time-this.prevPrevTime) ); // ms --> s
  };
}
