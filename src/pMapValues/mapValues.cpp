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
}

bool mapValues::OnNewMail(MOOSMSG_LIST &NewMail)
{
    AppCastingMOOSApp::OnNewMail(NewMail);
    MOOSMSG_LIST::iterator p;
    for (p=NewMail.begin(); p!=NewMail.end(); ++p) {
        CMOOSMsg & rMsg = *p;
        string msgKey = rMsg.GetKey();
        double msgVal = rMsg.GetDouble();
		if (m_axes.count(msgKey))
			m_axes[msgKey].MapAndNotify(msgVal);
		else if (m_switches.count(msgKey)) {
			if (rMsg.IsDouble())
				m_switches[msgKey].CheckValueThenPublish(rMsg.GetDouble());
			else
				m_switches[msgKey].CheckValueThenPublish(rMsg.GetString()); } }
    return UpdateMOOSVariables(NewMail);
}

bool mapValues::Iterate()
{
    AppCastingMOOSApp::Iterate();


    AppCastingMOOSApp::PostReport();
    return true;
}

bool mapValues::OnConnectToServer()
{
    return true;
}

bool mapValues::RegisterForMOOSMessages()
{
    AppCastingMOOSApp::RegisterVariables();
    // Axis and button messages registered in constructors of mapAxis and mapButton
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
	mapAxis ma = mapAxis(&m_Comms, sVal);
	if (ma.IsValid())
		m_axes[ma.GetKey()] = ma;
	else
		reportConfigWarning("Error: " + ma.GetError());
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

bool mapValues::buildReport()
{
	int numAxes = m_axes.size();
    m_msgs <<                " Axis definitions:   " << numAxes << endl;
    map<std::string, mapAxis>::iterator itAxes = m_axes.begin();
    for (; itAxes != m_axes.end(); ++itAxes) {
    	mapAxis ma = itAxes->second;
    	m_msgs <<            "          " << ma.GetAppCastMsg() << endl; }
	m_msgs << endl;

	int numButtons = m_switches.size();
    m_msgs <<                " Button definitions: " << numButtons << endl;
    map<std::string, mapButton>::iterator itButtons = m_switches.begin();
    for (; itButtons != m_switches.end(); ++itButtons) {
    	mapButton mb = itButtons->second;
    	m_msgs <<            "          " << mb.GetAppCastMsg() << endl; }
	m_msgs << endl;
    return true;
}





















//
