/*
 * mapAxis.h
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#ifndef PMAPVALUES_MAPAXIS_H_
#define PMAPVALUES_MAPAXIS_H_

#include <iostream>
#include "MOOS/libMOOS/MOOSLib.h"

#ifndef BAD_DOUBLE
#define BAD_DOUBLE   -999999.99
#define NO_DEPENDENT BAD_DOUBLE
#endif

#define DEBUG_MODE

// Input range [inMin, inMax]
// Output range [outMin, outMax]
// Input value dIn
// Output mapped value dOut
//
//                  (dIn - inMin) x (maxOut - minOut)
// dOut = outMin +  ---------------------------------
//                         (maxIn - minIn)
//

class mapAxis {
public:
                mapAxis();
                mapAxis(const std::string sDef);
                ~mapAxis() {}

    bool        HasValidSetup()                 { return m_bValidSetup; }
    std::string GetErrorString()                { return m_errorStr; }
    std::string GetSubscribeName()              { return m_inName; }
    std::string GetPublishName()                { return m_outName; }
    std::string GetDependentName()              { return m_depName; }
    std::string GetAppCastSetupString()         { return m_appcastSetup; }
    double      GetOutputMappedValue()          { return m_curOutValue; }
    double      GetNormalizedValue()            { return m_curNorm; }
    double      GetInputValue()                 { return m_curInValue; }
    std::string GetAppCastStatusString();
    void        SetInputValue(const double dIn);
    void        SetInputValues(const std::string sIn);


private:
    double      ConstrainDouble(const double in);
    void        SetInputValues(const double rawIn, const double depRawIn);
    double      MapToNorm(const double d);
    bool        SetRequiredDef(const std::string key, std::string& storeHere);
    bool        SetRequiredDef(const std::string key, double& storeHere);
    void        SetOptionalDef(const std::string key, std::string& storeHere);
    void        SetOptionalDef(const std::string key, double& storeHere);

    std::string m_inDef;
    bool        m_bValidSetup;          // TRUE when this mapping had a valid definition string
    bool        m_bValidValues;         // TRUE when current value (and dependent value if applicable) are not BAD_DOUBLE
    double      m_curInValue;           // Latest incoming value, BAD_DOUBLE before first input
    double      m_curNorm;              // Latest input value normalized to [-1, 1]. BAD_DOUBLE before first input
    double      m_curDepValue;          // Latest dependent value to the input
    double      m_curDepNorm;           // Latest dependent value normalized to [-1, 1], BAD_DOUBLE before first input and if no dependent
    double      m_curOutValue;          // Latest output value
    bool        m_HasDep;               // True when a dependent map was assigned
    std::string m_depName;              // Name of dependent mapping, empty string if not applicable

    // Definition of the input
    std::string m_inName;               // Name of the input MOOS message
    double      m_inMin;                // Minimum expected value on the input
    double      m_inMax;                // Maximum expected value on the input
    double      m_inConstrainMin;       // Lower constraint (in case min and max are reversed)
    double      m_inConstrainMax;       // Upper constraint (in case min and max are reversed)
    double      m_dead;                 // Percent of dead zone, stored in range [0, 100]. No dead zone = 0.0
    double      m_sat;                  // Percent of end saturation, stored in range [0, 100]. No saturation = 0.0

    // Definition of the output
    double      m_normMin;              // Minimum norm value to map
    double      m_normMax;              // Maximum norm value to map
    std::string m_outName;              // Name to publish mapped value to
    double      m_outMin;               // Minimum value output can be mapped to
    double      m_outMax;               // Maximum value output can be mapped to

    unsigned int m_count;               // Count how many inputs have been processed
    std::string m_errorStr;             // Stores error message to pass when definition is bad
    std::string m_appcastSetup;         // Created once, info about setup for appcasting.
};



#endif










//
