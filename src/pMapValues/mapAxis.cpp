/*
 * mapAxis.cpp
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#include "MBUtils.h"
#include "mapAxis.h"

using namespace std;


mapAxis::mapAxis()
{
	m_error  = "No definition for this mapping.";
    m_inName = "";
    m_inMin  = BAD_DOUBLE;
    m_inMax  = BAD_DOUBLE;
    m_inConstrainMin = BAD_DOUBLE;
    m_inConstrainMax = BAD_DOUBLE;
    m_outName = "";
    m_outMin = BAD_DOUBLE;
    m_outMax = BAD_DOUBLE;
    m_pComms = 0;
    m_countNotified = 0;
}

mapAxis::mapAxis(MOOS::MOOSAsyncCommClient* pComms, std::string sDef)
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
    	if (key == "IN_MIN") {
    		m_inMin = dVal;
    		iGood += 10; }
    	if (key == "IN_MAX") {
    		m_inMax = dVal;
    		iGood += 100; }
    	if (key == "OUT_MSG") {
    		m_outName = sVal;
    		iGood += 1000; }
    	if (key == "OUT_MIN") {
    		m_outMin = dVal;
    		iGood += 10000; }
    	if (key == "OUT_MAX") {
    		m_outMax = dVal;
    		iGood += 100000; } }
    if (iGood != 111111) {
    	m_error = "Invalid RANGE definition.";
    	return; }

    SetConstraints();
    PrepAppCastMsg();
    m_pComms->Register(m_inName, 0.0);
}

void mapAxis::SetConstraints()
{
    if (m_inMin < m_inMax) {
    	m_inConstrainMin = m_inMin;
    	m_inConstrainMax = m_inMax; }
    else {
    	m_inConstrainMin = m_inMax;
    	m_inConstrainMax = m_inMin; }
}

bool mapAxis::MapAndNotify(double dIn)
{
	return AxisNotify(MapInToOut(dIn));
}

// Input range [inMin, inMax]
// Output range [outMin, outMax]
// Input value dIn
// Output mapped value dOut
//
//                  (dIn - inMin) x (maxOut - minOut)
// dOut = outMin +  ---------------------------------
//                         (maxIn - minIn)
//
double mapAxis::MapInToOut(double dIn)
{
	if (dIn < m_inConstrainMin) dIn = m_inConstrainMin;
	if (dIn > m_inConstrainMax) dIn = m_inConstrainMax;
	return m_outMin + (dIn - m_inMin) * (m_outMax - m_outMin) / (m_inMax - m_inMin);
}

bool mapAxis::AxisNotify(double dMappedAlready)
{
	m_countNotified++;
	return m_pComms->Notify(m_outName, dMappedAlready);
}

void mapAxis::PrepAppCastMsg()
{
	stringstream ss;
	ss << "Map " << m_inName << " in range [" << m_inMin << ", " << m_inMax << "]";
	ss << " to " << m_outName << " in range [" << m_outMin << ", " << m_outMax << "].";
	m_appCastPrep = ss.str();
}


string mapAxis::GetAppCastMsg()
{
	stringstream ss;
	ss << m_appCastPrep << " |";
	ss << m_countNotified << "|";
	return ss.str();
}






















//
