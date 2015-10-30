/*
 * mapValues.h
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#ifndef PMAPVALUES_AYCONTROL_H_
#define PMAPVALUES_AYCONTROL_H_

#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "mapAxis.h"
#include "mapButton.h"

class mapValues : public AppCastingMOOSApp
{
public:
			mapValues();
            ~mapValues() {}
    bool    OnNewMail(MOOSMSG_LIST &NewMail);
    bool    Iterate();
    bool    OnConnectToServer();
    bool    OnStartUp();
    bool    buildReport();

protected:
    void    PublishOutput();
    bool    RegisterForMOOSMessages();
    bool    SetParam_RANGE(std::string sVal);
    bool    SetParam_SWITCH(std::string sVal);
    bool    SetParam_DEBUG_AXIS0(std::string sVal);
    bool    SetParam_DEBUG_AXIS1(std::string sVal);
    std::map<std::string, mapAxis> m_axes;
    std::map<std::string, mapButton> m_switches;

    bool m_debugMode;
    std::string m_strDebug0;
    std::string m_strDebug1;
};

#endif







//
