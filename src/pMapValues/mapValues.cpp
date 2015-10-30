/*
 * mapValues.cpp
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#include "MBUtils.h"

#include "mapValues.h"

using namespace std;

mapValues::mapValues()
{
    m_debugMode = false;
}

bool mapValues::OnNewMail(MOOSMSG_LIST &NewMail)
{
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p=NewMail.begin(); p!=NewMail.end(); ++p) {
        CMOOSMsg & rMsg = *p;
        string msgKey = rMsg.GetKey();

        // Check if incoming message exists in the axis map
        //      - Double value means it's a single value
        //      - String value means it's a value and a dependent value
		if (m_axes.count(msgKey)) {
            if (rMsg.IsDouble())    m_axes[msgKey].SetInputValue(rMsg.GetDouble());
            else                    m_axes[msgKey].SetInputValues(rMsg.GetString()); }

		// Check if incoming message exists in the button map
		//      - Handle double or string messages
		else if (m_switches.count(msgKey)) {
			if (rMsg.IsDouble())    m_switches[msgKey].CheckValueThenPublish(rMsg.GetDouble());
			else                    m_switches[msgKey].CheckValueThenPublish(rMsg.GetString()); } }

    return UpdateMOOSVariables(NewMail);
}

bool mapValues::Iterate()
{
    AppCastingMOOSApp::Iterate();

    PublishOutput();

    AppCastingMOOSApp::PostReport();
    return true;
}

void mapValues::PublishOutput()
{
    map<string, mapAxis>::iterator it = m_axes.begin();
    for (; it != m_axes.end(); ++it)
        m_Comms.Notify(it->second.GetPublishName(), it->second.GetOutputMappedValue());

    if (m_debugMode) {
        stringstream strCircle;
        strCircle << "x=0,y=0,radius=100.0,active=true,label=box,vertex_size=0,edge_color=white,edge_size=2";
        m_Comms.Notify("VIEW_CIRCLE", strCircle.str());

        stringstream strFullBox;
        strFullBox << "pts={100,100:100,-100:-100,-100:-100,100},active=true,label=fullBox,vertex_size=0,edge_color=white,edge_size=2";
        m_Comms.Notify("VIEW_POLYGON", strFullBox.str());

        { stringstream strDeadBox;
        double tb = m_axes[m_strDebug0].GetDeadZone() * 100.0;
        double lr = m_axes[m_strDebug1].GetDeadZone() * 100.0;
        strDeadBox << "pts={";
        strDeadBox <<        lr << ","  << tb << ":";
        strDeadBox <<        lr << ",-" << tb << ":";
        strDeadBox << "-" << lr << ",-" << tb << ":";
        strDeadBox << "-" << lr << "," << tb;
        strDeadBox << "},active=true,label=deadZone,vertex_size=0,edge_color=gray,edge_size=2";
        m_Comms.Notify("VIEW_POLYGON", strDeadBox.str()); }

        { stringstream strSatBox;
        double tb = 100.0 - m_axes[m_strDebug0].GetSaturation() * 100.0;
        double lr = 100.0 - m_axes[m_strDebug1].GetSaturation() * 100.0;
        strSatBox << "pts={";
        strSatBox <<        lr << ","  << tb << ":";
        strSatBox <<        lr << ",-" << tb << ":";
        strSatBox << "-" << lr << ",-" << tb << ":";
        strSatBox << "-" << lr << "," << tb;
        strSatBox << "},active=true,label=saturation,vertex_size=0,edge_color=gray,edge_size=2";
        m_Comms.Notify("VIEW_POLYGON", strSatBox.str()); }

        { stringstream strIn;
        strIn << "x="   << m_axes[m_strDebug0].GetInputValue() * 100.0 / m_axes[m_strDebug0].GetInMax();
        strIn << ",y="  << m_axes[m_strDebug1].GetInputValue() * 100.0 / m_axes[m_strDebug1].GetInMax();
        strIn << ",active=true,label=joyIn,label_color=yellow,vertex_color=yellow,vertex_size=4";
        m_Comms.Notify("VIEW_POINT", strIn.str()); }

        { stringstream strNorm;
        strNorm << "x="   << m_axes[m_strDebug0].GetNormalizedValue() * 100.0;
        strNorm << ",y="  << m_axes[m_strDebug1].GetNormalizedValue() * 100.0;
        strNorm << ",active=true,label=joyNorm,label_color=red,vertex_color=red,vertex_size=4";
        m_Comms.Notify("VIEW_POINT", strNorm.str()); }

        { stringstream strOut;
        strOut << "x=" << m_axes[m_strDebug0].GetOutputMappedValue() * 100.0 / m_axes[m_strDebug0].GetOutMax();
        strOut << ",y=" << m_axes[m_strDebug1].GetOutputMappedValue() * 100.0 / m_axes[m_strDebug1].GetOutMax();
        strOut << ",active=true,label=joyOut,label_color=green,vertex_color=green,vertex_size=4";
        m_Comms.Notify("VIEW_POINT", strOut.str()); } }

}

bool mapValues::OnConnectToServer()
{
    return true;
}

bool mapValues::RegisterForMOOSMessages()
{
    AppCastingMOOSApp::RegisterVariables();

    map<string, mapAxis>::iterator it = m_axes.begin();

    // Register for axis input messages
    for (; it != m_axes.end(); ++it)
        m_Comms.Register(it->second.GetSubscribeName(), 0.0);

    // Button input messages are registered in the mapButton class

    return RegisterMOOSVariables();
}

bool mapValues::OnStartUp()
{
    AppCastingMOOSApp::OnStartUp();

    STRING_LIST sParams;
    if (!m_MissionReader.GetConfiguration(GetAppName(), sParams))
        reportConfigWarning("No config block found for " + GetAppName());

    bool bHandled = true;
    STRING_LIST::iterator p;
    for (p = sParams.begin(); p != sParams.end(); p++) {
        string orig     = *p;
        string line     = *p;
        string param    = toupper(biteStringX(line, '='));
        string value    = line;

        if (param == "RANGE")
            bHandled = SetParam_RANGE(value);
        else if (param == "SWITCH")
            bHandled = SetParam_SWITCH(value);
        else if (param == "DEBUG_MODE") {
            m_debugMode =  (toupper(value) == "TRUE");
            bHandled = true; }
        else if (param == "DEBUG_AXIS0")
            bHandled = SetParam_DEBUG_AXIS0(value);
        else if (param == "DEBUG_AXIS1")
            bHandled = SetParam_DEBUG_AXIS1(value);
        else
            reportUnhandledConfigWarning(orig); }

    RegisterForMOOSMessages();
    RegisterVariables();

    // OnStartup() must always return true
    //    - Or else it will quit during launch and appCast info will be unavailable
    return true;
}

// RANGE = in_msg=x, in_min=0.0, in_max=0.0, out_msg=y, out_min=0.0, out_max=0.0
                // in_msg     Message name for incoming range values
                // in_min     Minimum value on the input range
                // in_max     Maximum value on the input range
                // out_msg    Mapped value published to this message
                // out_min    Minimum value input range is mapped to
                // out_max    Maximum value input range is mapped to
// Example:
// RANGE = in_msg=JOY_AXIS_2, in_min=-1000, in_max=1000, out_msg=DESIRED_RUDDER, out_min=-40, out_max=40
bool mapValues::SetParam_RANGE(string sVal)
{
	if (sVal.empty()) {
		reportConfigWarning("RANGE cannot not be blank.");
		return true; }
	mapAxis ma = mapAxis(sVal);
	if (ma.HasValidSetup())
		m_axes[ma.GetSubscribeName()] = ma;
	else
		reportConfigWarning("Error: " + ma.GetErrorString());
    return true;
}

// SWITCH = in_msg=w, trigger=x, out_msg=y, out_val=z
                  // in_msg     Message name for incoming switch value
                  // trigger    When in_msg contents change to match this trigger,
                  //              the out_msg will be published.
                  //              String/numeric agnostic.
                  // out_msg    Message name for resulting publication.
                  // out_val    Resulting publication posts this value.
                  //              If value is a numeric (within '+-.01234567889'),
                  //              published message is a double. Otherwise, a
                  //              string is published. To publish a numeric as a
                  //              string, put the number in quotes.");
   // Examples:
   //    SWITCH    = JOY_BUTTON_4, 1, ALL_STOP=true
   //    SWITCH    = JOY_BUTTON_7, off, VEHICLE_NUMBER=\"3\"
bool mapValues::SetParam_SWITCH(string sVal)
{
	if (sVal.empty()) {
		reportConfigWarning("SWITCH cannot not be blank.");
		return true; }

	mapButton mb = mapButton(&m_Comms, sVal);
	if (mb.IsValid())
		m_switches[mb.GetKey()] = mb;
	else
		reportConfigWarning("Error: " + mb.GetError());
	return true;
}

bool mapValues::SetParam_DEBUG_AXIS0(string sVal)
{
    if (sVal.empty()) {
        reportConfigWarning("DEBUG_AXIS0 cannot not be blank.");
        return true; }
    m_strDebug0 = sVal;
    return true;
}

bool mapValues::SetParam_DEBUG_AXIS1(string sVal)
{
    if (sVal.empty()) {
        reportConfigWarning("DEBUG_AXIS1 cannot not be blank.");
        return true; }
    m_strDebug1 = sVal;
    return true;
}

bool mapValues::buildReport()
{
	int numAxes = m_axes.size();
    m_msgs <<                " Axis definitions:   " << numAxes << endl;
    m_msgs << setw(20) << "IN_MSG"  << setw(7) << "IN_MIN"  << setw(7)  << "IN_MAX";
    m_msgs << setw(20) << "OUT_MSG" << setw(7) << "OUT_MIN" << setw(7)  << "OUT_MAX";
    m_msgs << setw(7)  << "DEAD"    << setw(7) << "SAT."    << setw(20) << "DEP_NAME";
    m_msgs << endl;
    map<std::string, mapAxis>::iterator itAxes = m_axes.begin();
    for (; itAxes != m_axes.end(); ++itAxes) {
    	mapAxis ma = itAxes->second;
    	m_msgs <<            " " << ma.GetAppCastSetupString() << endl; }
	m_msgs << endl;

	int numButtons = m_switches.size();
    m_msgs <<                " Button definitions: " << numButtons << endl;
    map<std::string, mapButton>::iterator itButtons = m_switches.begin();
    for (; itButtons != m_switches.end(); ++itButtons) {
    	mapButton mb = itButtons->second;
    	m_msgs <<            " " << mb.GetAppCastMsg() << endl; }
	m_msgs << endl << endl;

	m_msgs << " Axis Data:" << endl;
	itAxes = m_axes.begin();
	for (; itAxes != m_axes.end(); ++itAxes) {
	        mapAxis ma = itAxes->second;
	        m_msgs << ma.GetAppCastStatusString() << endl; }
	    m_msgs << endl;

    return true;
}





















//
