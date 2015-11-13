/*
 * moosJoy.h
 *
 *  Created on: Sep 24, 2015
 *      Author: Alon Yaari
 */

#ifndef MOOSJOY_H
#define MOOSJOY_H

#include "SDL.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#define NO_DEPENDENCY       -1
#define BUTTON_UP           false
#define BUTTON_DOWN         true
#define BUTTON_UP_STR       "UP"
#define BUTTON_DOWN_STR     "DOWN"
#define BLANK_LEN           10

class moosJoy : public AppCastingMOOSApp
{
public:
			moosJoy();
            ~moosJoy();
    bool    OnNewMail(MOOSMSG_LIST &NewMail);
    bool    Iterate();
    bool    OnConnectToServer();
    bool    OnStartUp();
    bool    buildReport();

protected:
    bool    RegisterForMOOSMessages();
    bool    JoystickSetup();
    bool    SetParam_JOYSTICK_ID(std::string sVal);
    bool    SetParam_OUTPUT_PREFIX(std::string sVal);
    bool    SetParam_DEPENDENT(std::string sVal);
    void	GetandPublishMostRecentJoystickValues();
    void	PublishJoystickAxisValue(int axis);
    void	PublishJoystickButtonValue(int button);
    bool	JoystickConnected();

    int appCastBlanks(int val);

    std::string 	m_prefix;
    std::string		m_joyName;
    int				m_joystickID;
    int				m_joystickCount;
    SDL_Joystick*	m_joy;
    bool			m_goodJoy;
    int				m_countAxes;
    int				m_countButtons;

    std::vector<std::string> m_depAppCast;
    std::vector<std::string> m_depDefs;
    std::map<int, int> m_joyAxisVal;
    std::map<int, int> m_axisDep;
    std::map<int, std::string> m_joyButtons;
};

#endif







//
