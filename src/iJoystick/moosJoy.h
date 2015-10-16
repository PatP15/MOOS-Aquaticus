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

#define BAD_AXIS_VAL   		-99999
#define BAD_BUTTON_VAL 		254
#define	DEFAULT_DEAD_END	200
#define DEFAULT_DEAD_ZERO	500
#define DEFAULT_MAX_AXIS	1000
#define JOY_FULL_RIGHT      32767
#define JOY_FULL_LEFT       -32768

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
    bool    SetParam_MAX_AXIS(std::string sVal);
    bool    SetParam_DEAD_END(std::string sVal);
    bool    SetParam_DEAD_ZERO(std::string sVal);
    void	GetandPublishMostRecentJoystickValues();
    void	PublishJoystickAxisValue(int axis, int value);
    void	PublishJoystickButtonValue(int button, int value);
    bool	JoystickConnected();
    int		NormalizeAxis(int val);

    std::string 	m_prefix;
    std::string		m_joyName;
    int				m_joystickID;
    int				m_joystickCount;
    SDL_Joystick*	m_joy;
    bool			m_goodJoy;
    int				m_countAxes;
    int				m_countButtons;
    int				m_maxAxis;
    int				m_deadEnd;
    int				m_deadZero;

    std::map<int, int> m_joyRaw;
    std::map<int, int> m_joyAxes;
    std::map<int, int> m_joyButtons;
};

#endif







//
