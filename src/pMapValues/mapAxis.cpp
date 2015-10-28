/*
 * mapAxis.cpp
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#include "math.h"       // fabs(), sqrt()
#include "MBUtils.h"
#include "mapAxis.h"

using namespace std;

// Input range [inMin, inMax]
// Output range [outMin, outMax]
// Input value dIn
// Output mapped value dOut
//
//                  (dIn - inMin) x (maxOut - minOut)
// dOut = outMin +  ---------------------------------
//                         (maxIn - minIn)
//

mapAxis::mapAxis()
{
    m_bValidSetup       = false;
    m_bValidValues      = false;
    m_HasDep            = false;
    m_inDef             = "";
    m_curInValue        = BAD_DOUBLE;
    m_curNorm           = BAD_DOUBLE;
    m_curDepValue       = BAD_DOUBLE;
    m_curDepNorm        = BAD_DOUBLE;
    m_curOutValue       = BAD_DOUBLE;
    m_count             = 0;
    m_appcastSetup      = "Undefined mapping.";
    m_errorStr          = "";

    // Will be set by the define string
    m_inName            = "";
    m_inMin             = BAD_DOUBLE;
    m_inMax             = BAD_DOUBLE;
    m_inConstrainMin    = BAD_DOUBLE;
    m_inConstrainMax    = BAD_DOUBLE;
    m_dead              = 0.0;
    m_sat               = 0.0;
    m_outName           = "";
    m_outMin            = BAD_DOUBLE;
    m_outMax            = BAD_DOUBLE;
    m_normMin           = -1.0;
    m_normMax           = 1.0;
}

// Sample string to parse for definition:
//      in_msg=JOY_AXIS_2, in_min=-32768, in_max=32768, dead=5, sat=5,
//          out_msg=DESIRED_RUDDER, out_min=-40, out_max=40

mapAxis::mapAxis(const std::string sDef)
{
    m_normMin           = -1.0;
    m_normMax           = 1.0;
    if (sDef.empty()) {
        m_errorStr = "Mapping definition cannot be blank.";
        return; }
    m_inDef = toupper(sDef);
    bool bGood = true;
    bGood &= SetRequiredDef("IN_MSG",  m_inName);
    bGood &= SetRequiredDef("IN_MIN",  m_inMin);
    bGood &= SetRequiredDef("IN_MAX",  m_inMax);
    bGood &= SetRequiredDef("OUT_MSG", m_outName);
    bGood &= SetRequiredDef("OUT_MIN", m_outMin);
    bGood &= SetRequiredDef("OUT_MAX", m_outMax);
    if (!bGood) return;

    SetOptionalDef("DEAD", m_dead);
    if (m_dead < 0.0 || m_dead >= 100.0) {
        m_errorStr = "DEAD (dead zone) must be 0 or a positive number < 100.";
        return; }
    m_dead /= 100.0;
    SetOptionalDef("SAT", m_sat);
    if (m_sat < 0.0 || m_sat >= 100.0) {
        m_errorStr = "SAT (saturation) must be 0 or a positive number < 100.";
        return; }
    m_sat /= 100.0;
    SetOptionalDef("NORM_MIN",  m_normMin);
    if (m_normMin < -1.0 || m_normMin > 1.0) {
        m_errorStr = "NORM_MIN must be in range [-1.0, 1.0].";
        return; }
    SetOptionalDef("NORM_MAX",  m_normMax);
    if (m_normMax < -1.0 || m_normMax > 1.0) {
        m_errorStr = "NORM_MAX must be in range [-1.0, 1.0].";
        return; }
    SetOptionalDef("DEP",     m_depName);
    m_HasDep = !m_depName.empty();

    // Set constraints (in case min and max are reversed)
    //m_inConstrainMin = (m_inMin < m_inMax) ? m_inMin : m_inMax;
    //m_inConstrainMax = (m_inMin < m_inMax) ? m_inMax : m_inMin;
    m_inConstrainMin = m_inMin;
    m_inConstrainMax = m_inMax;
    // Create the appcast string detaling setup
    stringstream ss;
    ss << "    IN_NAME:       " << m_inName  << endl;
    ss << "    IN_MIN,MAX:    " << m_inMin   << ", " << m_inMax  << endl;
    ss << "    OUT_NAME:      " << m_outName << endl;
    ss << "    NORM_MIN, MAX: " << m_normMin << ", " << m_normMax << endl;
    ss << "    OUT_MIN, MAX:  " << m_outMin  << ", " << m_outMax << endl;
    ss << "    DEAD, SAT:     " << m_dead * 100.0 << ", " << m_sat * 100.0 << endl;
    m_appcastSetup = ss.str();
    m_bValidSetup = true;
}

void mapAxis::SetOptionalDef(const string key, string& storeHere)
{
    tokParse(m_inDef, key, ',', '=', storeHere);
}

void mapAxis::SetOptionalDef(const string key, double& storeHere)
{
    string sVal;
    bool bGood = tokParse(m_inDef, key, ',', '=', sVal);
    if (bGood)
        storeHere = strtod(sVal.c_str(), 0);
}

bool mapAxis::SetRequiredDef(const string key, double& storeHere)
{
    string sVal;
    bool bGood = SetRequiredDef(key, sVal);
    if (bGood)
        storeHere = strtod(sVal.c_str(), 0);
    return bGood;
}

bool mapAxis::SetRequiredDef(const string key, string& storeHere)
{
    bool bGood = tokParse(m_inDef, key, ',', '=', storeHere);
    if (!bGood) {
        m_errorStr = "Missing definition for required element ";
        m_errorStr.append(key);
        m_errorStr.append(".");
        return false; }
    return true;
}

string mapAxis::GetAppCastStatusString()
{
    //    in_msg name     in_value    norm   out_msg name  out_value  count
    stringstream ss;
    ss << "    MSG_IN name:  " << m_inName      << "    count : " << m_count << endl;
    ss << "    In value:     " << m_curInValue  << endl;
    ss << "    Normalized:   " << m_curNorm     << endl;
    ss << "    MSG_OUT name: " << m_outName     << endl;
    ss << "    Out value:    " << m_curOutValue << endl;
    return ss.str();
}

double mapAxis::ConstrainDouble(const double in)
{
    if (in < m_inConstrainMin)      return m_inConstrainMin;
    else if (in > m_inConstrainMax) return m_inConstrainMax;
    return in;
}

void mapAxis::SetInputValue(double dIn)
{
    SetInputValues(dIn, BAD_DOUBLE);
}

void mapAxis::SetInputValues(string sIn)
{
    if (sIn.empty()) return;
    vector<string> sVec = parseString(sIn, ',');
    if (sVec.size() != 2) return;
    double inVal = strtod(sVec[0].c_str(), 0);
    double inDep = strtod(sVec[1].c_str(), 0);
    SetInputValues(inVal, inDep);
}

void mapAxis::SetInputValues(double rawIn, double depRawIn)
{
    // Normalize input axis
    m_curInValue    = ConstrainDouble(rawIn);
    m_curNorm       = MapToNorm(m_curInValue);

    // Deal with dependent axis
    if (depRawIn != BAD_DOUBLE) {

        // Normalize dependent axis
        m_curDepValue   = ConstrainDouble(depRawIn);
        m_curDepNorm    = MapToNorm(m_curDepValue);

        // Relate input axis to dependent value
        double d        = sqrt((m_curNorm * m_curNorm) + (m_curDepNorm * m_curDepNorm));
        m_curNorm       = (m_curNorm < 0.0) ? d * -1 : d; }

    // Adjust norm: Dead zone and saturation
    if (fabs(m_curNorm) < m_dead)       m_curNorm =  0.0;
    else if (m_curNorm > 1.0 - m_sat)   m_curNorm =  1.0;
    else if (m_curNorm < -1.0 + m_sat)  m_curNorm = -1.0;

    //                  (norm - normMin) x (maxOut - minOut)
    // dOut = outMin +  ---------------------------------
    //                         (normMax - normMin)

    double interim = m_curNorm;
    if (m_normMin < m_normMax) {
        if (interim < m_normMin) interim = m_normMin;
        if (interim > m_normMax) interim = m_normMax; }
    else {
        if (interim < m_normMax) interim = m_normMax;
        if (interim > m_normMin) interim = m_normMin; }

    //                  (dIn - inMin) x (maxOut - minOut)
    // dOut = outMin +  ---------------------------------
    //                         (maxIn - minIn)

    // Map norm to output
    //m_curOutValue = m_outMin + ((interim + m_normMin) * (m_outMax - m_outMin) / (m_normMax - m_normMin));
    m_curOutValue = m_outMin + ((interim - m_normMin) * (m_outMax - m_outMin) / (m_normMax - m_normMin));
    m_count++;
}

//               (d - inMin) x (2.0)
// norm = -1.0 + ----------------------
//                   (maxIn - minIn)
double mapAxis::MapToNorm(double d)
{
    return -1.0 + (d - m_inMin) * (2.0) / (m_inMax - m_inMin);
}












//
