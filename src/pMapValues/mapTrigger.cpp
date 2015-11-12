/*
 * mapTrigger.cpp
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#include "MBUtils.h"
#include "mapTrigger.h"

using namespace std;

mapTrigger::mapTrigger()
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

mapTrigger::mapTrigger(MOOS::MOOSAsyncCommClient* pComms, string sDef)
{
    m_error = "";
    m_pComms = pComms;
    string line = toupper(sDef);
    string sVal = "";
    bool bGood = tokParse(line, "IN_MSG", ',', '=', m_inName);
    if (bGood)
        bGood = ayTokParse(line, "TRIGGER", ',', '=', m_triggerValue);
    if (bGood)
        bGood = tokParse(line, "OUT_MSG", ',', '=', m_outName);
    if (bGood)
        bGood = ayTokParse(line, "OUT_VAL", ',', '=', m_outString);
    if (bGood) {
        m_outDouble = strtod(m_outString.c_str(), 0);    // Always convert, even if string val will simply be 0.0
        // Is the outVal a string or a double?
        //      - If it has any quote characters, it's a string
        //      - If it registers as a pure number, it's a double
        if (m_outString.find('\"') != string::npos)
            m_bOutIsDouble = false;
        else
            m_bOutIsDouble = isNumber(m_outString, true);

        PrepAppCastMsg();
        m_pComms->Register(m_inName, 0.0);
        PrepAppCastMsg();
        return; }
    m_error = "Bad trigger definition: " + line;
}

bool mapTrigger::StoreValueThenPublish(double dVal)
{
	return StoreValueThenPublish(doubleToString(dVal, 6));
}

bool mapTrigger::StoreValueThenPublish(std::string sVal)
{
	// Ignore when new input is the same as last time;
	if (sVal == m_lastVal) return true;
	m_lastVal = sVal;

	// Ignore when new input is not the trigger value
	if (sVal != m_triggerValue) {
		return true; }

	// Reaching here means input value changed into the trigger
	m_countNotified++;

	if (m_bOutIsDouble)
		return m_pComms->Notify(m_outName, m_outDouble);
	return m_pComms->Notify(m_outName, m_outString);
}

void mapTrigger::PrepAppCastMsg()
{
	stringstream ss;
	ss << "      In:         " << m_inName << endl;
	ss << "       Trigger on: " << m_triggerValue << endl;
	ss << "       Out:        " << m_outName << endl;
	ss << "       Value:      " << m_outString;
    if (m_bOutIsDouble)
    	ss << " (as double).";
    else
    	ss << " (as string).";
    ss << endl;
	m_appCastPrep = ss.str();
}


string mapTrigger::GetAppCastMsg()
{
    return m_appCastPrep;
}

string mapTrigger::GetAppCastStatusString()
{
    stringstream ss;
    ss << "   " << m_inName << endl;
    ss << "      Last in value: " << m_lastVal << endl;
    ss << "   Published: " << m_countNotified << "" << endl;
    return ss.str();
}

bool mapTrigger::ayTokParse(const std::string& str, const std::string& left, char gsep, char lsep, std::string& rstr)
{
    rstr = "error";
    vector<string> svector1 = parseStringQ(str, gsep);
    for (vector<string>::size_type i = 0; i < svector1.size(); i++) {
        vector<string> svector2 = parseStringQ(svector1[i], lsep);
        if (svector2.size() != 2) return false;
        svector2[0] = stripBlankEnds(svector2[0]);
        if (svector2[0] == left) {
            rstr = svector2[1];
            return true; } }
    return false ;
}














//
