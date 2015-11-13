/*
 * moosJoy.cpp
 *
 *  Created on: Sep 24, 2015
 *      Author: Alon Yaari
 */

#include "MBUtils.h"
#include "moosJoy.h"

using namespace std;

moosJoy::moosJoy()
{
    m_prefix                = "JOY_";
    m_joystickID            = 0;
    m_joystickCount         = 0;
    m_joy                   = 0;
    m_joyName               = "";
    m_goodJoy               = false;
    m_countAxes             = 0;
    m_countButtons          = 0;
}

moosJoy::~moosJoy()
{
    SDL_Quit();
}

bool moosJoy::OnNewMail(MOOSMSG_LIST &NewMail)
{
    AppCastingMOOSApp::OnNewMail(NewMail);

    MOOSMSG_LIST::iterator p;
    for (p=NewMail.begin(); p!=NewMail.end(); ++p) {
        CMOOSMsg & rMsg = *p;
        // if (MOOSStrCmp(rMsg.GetKey(), "SOME_MOOS_MESSAGE"))
        //     DoSomeWork(sVal);
        }
    return true;
}

bool moosJoy::Iterate()
{
    AppCastingMOOSApp::Iterate();

    GetandPublishMostRecentJoystickValues();

    AppCastingMOOSApp::PostReport();
    return true;
}

void moosJoy::GetandPublishMostRecentJoystickValues()
{
    // Grab and store most recent joystick values
    SDL_JoystickUpdate();
    for (int i = 0; i < m_countAxes; i++)
        m_joyAxisVal[i] = (int) SDL_JoystickGetAxis(m_joy, i);

    // Once all the values are stored, then publish
    for (int i = 0; i < m_countAxes; i++)
        PublishJoystickAxisValue(i);

    // Deal with buttons
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_QUIT:
            cerr << "\nQuitting..." << endl;
            RequestQuit();
            break;
        case SDL_JOYAXISMOTION:
            break;
        case SDL_JOYBUTTONDOWN: {
            int i = event.jbutton.button;
            if (m_joyButtons[i] != BUTTON_DOWN_STR) {
                m_joyButtons[i] = BUTTON_DOWN_STR;
                PublishJoystickButtonValue(i); } }
            break;
        case SDL_JOYBUTTONUP: {
            int i = event.jbutton.button;
            if (m_joyButtons[i] != BUTTON_UP_STR) {
                m_joyButtons[i] = BUTTON_UP_STR;
                PublishJoystickButtonValue(i); } }
            break;
        default:
            break; }
}

void moosJoy::PublishJoystickAxisValue(int axis)
{
    // Publish the axis value as double
    stringstream ss;
    ss << m_prefix << "AXIS_" << axis;
    m_Comms.Notify(ss.str(), m_joyAxisVal[axis]);

    // For dependent axes, also publish a string with two values
    if (m_axisDep[axis] != NO_DEPENDENCY) {
        ss << "_DEP";
        stringstream outStr;
        outStr << m_joyAxisVal[axis];
        outStr << ",";
        outStr << m_joyAxisVal[m_axisDep[axis]];
        m_Comms.Notify(ss.str(), outStr.str()); }
}

void moosJoy::PublishJoystickButtonValue(int button)
{
    stringstream ss;
    ss << m_prefix;
    ss << "BUTTON_";
    ss << button;
    m_Comms.Notify(ss.str(), m_joyButtons[button]);
}

bool moosJoy::OnConnectToServer()
{
    return true;
}

bool moosJoy::OnStartUp()
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
        if (param == "JOYSTICK_ID" || param == "JOYSTICKID")
            bHandled = SetParam_JOYSTICK_ID(value);
        else if (param == "OUTPUT_PREFIX")
            bHandled = SetParam_OUTPUT_PREFIX(value);
        else if (param == "DEPENDENT")
            m_depDefs.push_back(value);
        else
            reportUnhandledConfigWarning(orig); }
    RegisterForMOOSMessages();
    RegisterVariables();

    JoystickSetup();

    // Configure dependent axes (after setting up the joystick)
    vector<string>::iterator it = m_depDefs.begin();
    for (;it !=m_depDefs.end(); ++it)
        SetParam_DEPENDENT(*it);

    // OnStartup() must always return true
    //    - Or else it will quit during launch and appCast info will be unavailable
    return true;
}

bool moosJoy::RegisterForMOOSMessages()
{
    AppCastingMOOSApp::RegisterVariables();
    //m_Comms.Register("SOME_MOOS_MESSAGE", 0);
    return RegisterMOOSVariables();
}

bool moosJoy::JoystickSetup()
{
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
        reportRunWarning("Could not connect with SDL to read joystick.");
        return false; }
    m_joystickCount = SDL_NumJoysticks();
    stringstream ss;
    ss << m_joystickCount;
    if (!m_joystickCount) {
        reportRunWarning("No joysticks connected to the system " + ss.str());
        return false; }
    if (m_joystickID > m_joystickCount) {
        stringstream ss;
        ss << "JOYSTICK_ID defined as ";
        ss << m_joystickID;
        ss << " but there are only ";
        ss << m_joystickCount;
        ss << " joysticks connected with a maximum JOYSTICK_ID of ";
        ss << m_joystickCount - 1;
        ss << ".";
        reportRunWarning(ss.str());
        return false; }

    // Open SDL connection with the joystick
    m_joy = SDL_JoystickOpen(m_joystickID);

    if (!JoystickConnected()) {
        reportRunWarning("Could not connect with joystick.");
        return false; }

    m_joyName               = SDL_JoystickName(m_joy);          // Get registration name for the joystick
    m_countAxes             = SDL_JoystickNumAxes(m_joy);       // Get number of axes
    m_countButtons          = SDL_JoystickNumButtons(m_joy);    // Get number of buttons
    // int numberOfBalls    = SDL_JoystickNumBalls(m_joy);
    // int numberOfPOVHats  = SDL_JoystickNumHats(m_joy);

    // Set up polled reading for axes
    int polling = SDL_JoystickEventState(SDL_ENABLE);
    if (polling) {
        string msgName = m_prefix + "AXIS_COUNT";
        m_Comms.Notify(msgName, (double) m_countAxes);
        msgName = m_prefix + "BUTTON_COUNT";
        m_Comms.Notify(msgName, (double) m_countButtons);

        // Load axes into the map and clear out the dependency vector
        for (int i = 0; i < m_countAxes; i++) {
            m_joyAxisVal[i] = 0;
            m_axisDep[i] = NO_DEPENDENCY; }

        // Load buttons into the map and publish all buttons as being up
        for (int i = 0; i < m_countButtons; i++) {
            m_joyButtons[i] = BUTTON_UP_STR;
            PublishJoystickButtonValue(i); }

        return true; }
    return false;
}

bool moosJoy::JoystickConnected()
{
    m_goodJoy = (SDL_JoystickGetAttached(m_joy) == SDL_TRUE);
    return m_goodJoy;
}

bool moosJoy::SetParam_JOYSTICK_ID(string sVal)
{
    if (sVal.empty())
        reportConfigWarning("JOYSTICK_ID must not be blank. Using default value.");
    else {
        int joyID = strtol(sVal.c_str(), 0, 10);
        if (joyID < 0 || joyID > 63)
            reportConfigWarning("JOYSTICK_ID must be 0 to 63, inclusive. Using default value.");
        else
            m_joystickID = joyID; }
    return true;
}

bool moosJoy::SetParam_OUTPUT_PREFIX(string sVal)
{
    int len = sVal.length();
    if (!len)
        reportConfigWarning("OUTPUT_PREFIX must not be blank. Using default value.");
    else {
        if (len > 32)
            reportConfigWarning("Prefix must be fewer than 32 characters. Using default value.");
        else {
            if (sVal.at(len - 1) != '_')
                sVal.append("_");
            m_prefix = sVal; } }
    return true;
}

bool moosJoy::SetParam_DEPENDENT(string sVal)
{
    if (sVal.empty()) {
        reportConfigWarning("DEPENDENT should not be blank. No dependency defined.");
        return true; }
    vector<string> sVec = parseString(sVal, ',');
    if (sVec.size() != 2) {
        reportConfigWarning("DEPENDENT should be two integers describing dependent axes. No dependency defined.");
        return true; }
    int depA = strtol(sVec[0].c_str(), 0, 10);
    int depB = strtol(sVec[1].c_str(), 0, 10);
    if (depA > m_countAxes - 1 || depA < 0 || depB > m_countAxes -1 || depB < 0) {
        string strVR = "[0, ";
        strVR.append(intToString(m_countAxes - 1));
        strVR.append("]");
        reportConfigWarning("DEPENDENT is attempting to describe an axis outside the valid range of " + strVR);
        return true; }
    m_axisDep[depA] = depB;
    m_axisDep[depB] = depA;
    stringstream ss;
    ss << "Axes " << depA << " and " << depB << " are dependent";
    m_depAppCast.push_back(ss.str());
    return true;
}

int moosJoy::appCastBlanks(int val)
{
    int    num      = val + 32768;              // num is positive int in range [0, 65535]
    double dNorm    = (double) num / 65535.0;   // dNorm in range [0.0, 1.0]
    return BLANK_LEN + (int) (30.0 * dNorm);           // return value is in range [10, 40]
}

bool moosJoy::buildReport()
{
    m_msgs <<                " Settings" << endl;
    m_msgs <<                " --------" << endl;
    m_msgs <<                "    Joystick_ID:               " << m_joystickID << endl;
    m_msgs <<                "    Output_Prefix:             " << m_prefix << endl;
    vector<string>::iterator it = m_depAppCast.begin();
    for (; it != m_depAppCast.end(); ++it)
        m_msgs <<            "    Dependency:                " << *it << endl;
    m_msgs << endl;
    m_msgs <<                " Joystick" << endl;
    m_msgs <<                " --------" << endl;
    m_msgs <<                "    Controller description:    " << m_joyName << endl;
    m_msgs <<                "    Good connection:           ";
    m_msgs << (m_goodJoy ? "connected" : "not connected");
    m_msgs << endl;
    m_msgs <<                "    Number of axes:            " << m_countAxes << endl;
    m_msgs <<                "    Number of buttons:         " << m_countButtons << endl;
    m_msgs << endl;
    m_msgs <<                " Readings" << endl;
    m_msgs <<                " --------" << endl;
    for (int i = 0; i < m_countAxes; i++) {
        int axisVal = m_joyAxisVal[i];
        m_msgs <<            "    Axis  ";
        m_msgs.width(3);
        m_msgs << i << "\t";
        if (m_axisDep[i] != NO_DEPENDENCY)
            m_msgs << "dep-" << m_axisDep[i];
        else
            m_msgs << "     ";
        m_msgs.width(appCastBlanks(axisVal));
        m_msgs << axisVal;
        m_msgs << endl; }
    for (int i = 0; i < m_countButtons; i++) {
        m_msgs <<            "    Button";
        m_msgs.width(3);
        m_msgs << i << "   ";
        m_msgs << m_joyButtons[i] << endl; }
    return true;
}

















//

