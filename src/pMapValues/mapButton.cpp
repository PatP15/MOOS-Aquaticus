/*
 * mapButton.cpp
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#include "MBUtils.h"
#include "mapButton.h"

using namespace std;

mapButton::mapButton()
{
	m_error = "No definition for this mapping.";
	m_countNotified = 0;
	m_lastVal = "";
    m_inName = "";
    m_triggerValue = "";
    m_outName = "";
    m_outString = "";
    m_outDouble = BAD_DOUBLE;
    m_bOutIsDouble = true;
    m_pComms = 0;
}

mapButton::mapButton(MOOS::MOOSAsyncCommClient* pComms, string sDef)
{
	m_error = "";
	m_countNotified = 0;
    m_pComms = pComms;
    vector<string> items = parseStringQ(sDef, ',');
    vector<string>::iterator it = items.begin();
    int iGood = 0;
    for (; it != items.end(); ++it) {
    	string sVal = *it;
    	string key = toupper(biteStringX(sVal, '='));
    	double dVal = strtod(sVal.c_str(), 0);
    	if (key == "IN_MSG") {
    		m_inName = sVal;
    		iGood += 1; }
    	if (key == "TRIGGER") {
    		m_triggerValue = sVal;
    		iGood += 10; }
    	if (key == "OUT_MSG") {
    		m_outName = sVal;
    		iGood += 100; }
    	if (key == "OUT_VAL") {
    		m_outString = sVal;
    		m_outDouble = strtod(m_outString.c_str(), 0);    // Always convert, even if string val will simply be 0.0
    		// Is the outVal a string or a double?
			//      - If it has any quote characters, it's a string
			//      - If it registers as a pure number, it's a double
			if (m_outString.find('\"') != string::npos)
				m_bOutIsDouble = false;
			else
				m_bOutIsDouble = isNumber(m_outString, true);
    		iGood += 1000; } }
    if (iGood != 1111) {
    	m_error = "Invalid SWITCH definition.";
    	return; }

    PrepAppCastMsg();
    m_pComms->Register(m_inName, 0.0);
    PrepAppCastMsg();
}

bool mapButton::CheckValueThenPublish(double dVal)
{
	return CheckValueThenPublish(doubleToString(dVal, 6));
}

bool mapButton::CheckValueThenPublish(std::string sVal)
{
	// Ignore when new input is the same as last time;
	if (sVal == m_lastVal) return true;

	// Ignore when new input is not the trigger value
	if (sVal != m_triggerValue) {
		m_lastVal = sVal;
		return true; }

	// Reaching here means input value changed into the trigger
	m_lastVal = sVal;
	m_countNotified++;

	if (m_bOutIsDouble)
		return m_pComms->Notify(m_outName, m_outDouble);
	return m_pComms->Notify(m_outName, m_outString);
}

void mapButton::PrepAppCastMsg()
{
	stringstream ss;
	ss << "When " << m_inName << " = " << m_triggerValue << ",";
	ss << " publish " << m_outName << " = " << m_outString;
    if (m_bOutIsDouble)
    	ss << " (as double).";
    else
    	ss << " (as string).";
	m_appCastPrep = ss.str();
}


string mapButton::GetAppCastMsg()
{
	stringstream ss;
	ss << m_appCastPrep << " [";
	ss << m_countNotified << "]";
	return ss.str();
}















//
