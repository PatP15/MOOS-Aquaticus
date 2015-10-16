/*
 * moosJoy.cpp
 *
 *  Created on: Sep 24, 2015
 *      Author: yar
 */

#include "MBUtils.h"
#include "moosJoy.h"


using namespace std;

moosJoy::moosJoy()
{
    m_prefix                = "JOY0_";
    m_joystickID            = 0;
    m_joystickCount         = 0;
    m_joy                   = 0;
    m_joyName				= "";
    m_goodJoy				= false;
    m_countAxes				= 0;
    m_countButtons			= 0;
    m_maxAxis				= DEFAULT_MAX_AXIS;
    m_deadEnd				= DEFAULT_DEAD_END;
    m_deadZero				= DEFAULT_DEAD_ZERO;
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
    return UpdateMOOSVariables(NewMail);
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
	SDL_JoystickUpdate();		// Grab most recent joystick values
	int i = 0;
	for (; i < m_countAxes; i++) {
		Sint16 jAxisVal = SDL_JoystickGetAxis(m_joy, i);
		if (m_joyRaw[i] != jAxisVal) {
			m_joyRaw[i] = jAxisVal;
			m_joyAxes[i] = NormalizeAxis(jAxisVal);
			PublishJoystickAxisValue(i, m_joyAxes[i]); } }

	SDL_Event event;
	SDL_PollEvent(&event);
	switch (event.type) {
	case SDL_QUIT:
		cerr << "\nQuitting..." << endl;
		RequestQuit();
		break;
	case SDL_JOYAXISMOTION:
		break;
	case SDL_JOYBUTTONDOWN:
		i = event.jbutton.button;
		m_joyButtons[i] = 1;
		PublishJoystickButtonValue(i, 1);
		break;
	case SDL_JOYBUTTONUP:
		i = event.jbutton.button;
		m_joyButtons[i] = 0;
		PublishJoystickButtonValue(i, 0);
		break;
	default:
	  //		cerr << "------------------------ " << (unsigned int) event.type << endl;
		break; }

//	for (int i = 0; i < m_countButtons; i++) {
//		int buttonStat = 0;
//		if (SDL_JoystickGetButton(m_joy, i)) {
//			cout << "------------------------------------ " << i << "   " << endl;
//			buttonStat = 1; }
//		if (m_joyButtons[i] != buttonStat) {
//			m_joyButtons[i] = buttonStat;
//			PublishJoystickButtonValue(i, buttonStat); } }
}

void moosJoy::PublishJoystickAxisValue(int axis, int value)
{
	stringstream ss;
	ss << m_prefix;
	ss << "AXIS_";
	ss << axis;
	m_Comms.Notify(ss.str(), value);
}

void moosJoy::PublishJoystickButtonValue(int button, int value)
{
	stringstream ss;
	ss << m_prefix;
	ss << "button_";
	ss << button;
	m_Comms.Notify(ss.str(), value);
}


//  raw values -32768            0            32767");
//                  |---.-----.--|--.-----.---|");
//                    a |  b  |  c  |  d  | e");
//                      W     X     Y     Z");
//
//     W = -32768 + DEAD_END          a: Always reports -MAX_AXIS");
//     X = 0 - DEAD_ZERO              b: Between 0 and -MAX_AXIS");
//     Y = DEAD_ZERO                  c: Always 0");
//     Z = 32767 - DEAD_END           d: Between 0 and MAX_AXIS");
//                                    e: Always MAX_AXIS");
int moosJoy::NormalizeAxis(int val)
{
	int W = JOY_FULL_LEFT + m_deadEnd;
	int X = 0 - m_deadZero;
	int Y = m_deadZero;
	int Z = JOY_FULL_RIGHT - m_deadEnd;
	if (val < W)            return 0 - m_maxAxis;  // a: dead end on left
	if (val > X && val < Y) return 0;              // c: center dead zone
	if (val > Z)            return m_maxAxis;      // e: dead end on right

	// Normalize range b and d

	//    0            32767
	//    |--.-----.---|
	//       |  d  |
	//       Y     Z
	//    val is somewhere in the range d: (Y, Z)
	//    Find what % into the the range it is
	//         Y is 0%, Z is 100%

	int negORpos = (val < 0 ? -1 : 1);  // Retain whether this is left or right

	// Remove the center dead zone from all numbers involved
	//    Y doesn't need dead zone removed because the result is always 0
	int norm = abs(val) - m_deadZero;   // Remove the deadzone from the number
	int highEnd = Z - m_deadZero;		// Remove the deadzone from Z
	double pct = (double) norm / (double) highEnd;

	return negORpos * (int) (pct * m_maxAxis);
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
        else if (param == "MAX_AXIS")
            bHandled = SetParam_MAX_AXIS(value);
        else if (param == "DEAD_ZERO")
            bHandled = SetParam_DEAD_ZERO(value);
        else if (param == "DEAD_END")
            bHandled = SetParam_DEAD_END(value);
        else
            reportUnhandledConfigWarning(orig); }

    RegisterForMOOSMessages();
    RegisterVariables();

    JoystickSetup();

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
//	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
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

	m_joyName 		= SDL_JoystickName(m_joy);			// Get registration name for the joystick
	m_countAxes 	= SDL_JoystickNumAxes(m_joy);		// Get number of axes
	m_countButtons	= SDL_JoystickNumButtons(m_joy);	// Get number of buttons
	// int numberOfBalls = SDL_JoystickNumBalls(m_joy);
	// int numberOfPOVHats = SDL_JoystickNumHats(m_joy);

	// Seed bad values so that everything will publish on first reading
	for (int i = 0; i < m_countAxes; i++) {
		m_joyAxes[i] = BAD_AXIS_VAL;
		m_joyRaw[i]  = BAD_AXIS_VAL; }
	for (int i = 0; i < m_countButtons; i++)
		m_joyButtons[i] = BAD_BUTTON_VAL;

	// Set up polled reading
	int polling = SDL_JoystickEventState(SDL_ENABLE);
	if (polling) {
		string msgName = m_prefix + "AXIS_COUNT";
		m_Comms.Notify(msgName, (double) m_countAxes);
		msgName = m_prefix + "BUTTON_COUNT";
		m_Comms.Notify(msgName, (double) m_countButtons);
		return true;}
	return false;
}

bool moosJoy::JoystickConnected()
{
	m_goodJoy = (SDL_JoystickGetAttached(m_joy) == SDL_TRUE);
	return m_goodJoy;
}


bool moosJoy::SetParam_MAX_AXIS(std::string sVal)
{
	if (sVal.empty())
		reportConfigWarning("MAX_AXIS must not be blank. Using default value.");
	else {
		int ma = strtol(sVal.c_str(), 0, 10);
		if (ma < 0 || ma > 32767)
			reportConfigWarning("MAX_AXIS must be between 1 and 32767, inclusive. Using default value.");
		else
			m_maxAxis = ma; }
	return true;
}

bool moosJoy::SetParam_DEAD_END(std::string sVal)
{
	if (sVal.empty())
		reportConfigWarning("DEAD_END must not be blank. Using default value.");
	else {
		int de = strtol(sVal.c_str(), 0, 10);
		if (de < 0 || de > 32767)
			reportConfigWarning("DEAD_END must be between 1 and 32767, inclusive. Using default value.");
		else
			m_deadEnd = de; }
	return true;
}

bool moosJoy::SetParam_DEAD_ZERO(std::string sVal)
{
	if (sVal.empty())
		reportConfigWarning("DEAD_ZERO must not be blank. Using default value.");
	else {
		int dz = strtol(sVal.c_str(), 0, 10);
		if (dz < 0 || dz > 32767)
			reportConfigWarning("DEAD_ZERO must be between 1 and 32767, inclusive. Using default value.");
		else
			m_deadEnd = dz; }
	return true;
}

bool moosJoy::SetParam_JOYSTICK_ID(std::string sVal)
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

bool moosJoy::SetParam_OUTPUT_PREFIX(std::string sVal)
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

bool moosJoy::buildReport()
{
    m_msgs <<                " Settings" << endl;
    m_msgs <<                " --------" << endl;
    m_msgs <<                "    Joystick_ID:               " << m_joystickID << endl;
    m_msgs <<                 "    Output_Prefix:             " << m_prefix << endl;
    m_msgs << endl;
    m_msgs <<                " Joystick" << endl;
    m_msgs <<                " --------" << endl;
    m_msgs <<                "    Joystick description:      " << m_joyName << endl;
    m_msgs <<                "    Good connection:           ";
    m_msgs << (m_goodJoy ? "connected" : "not connected");
    m_msgs << endl;
    m_msgs <<                "    Number of axes:            " << m_countAxes << endl;
    m_msgs <<                "    Number of buttons:         " << m_countButtons << endl;
    m_msgs << endl;
    m_msgs <<                " Readings" << endl;
    m_msgs <<                " --------" << endl;
    for (int i = 0; i < m_countAxes; i++) {
    	m_msgs <<            "    Axis   ";
    	m_msgs.width(3);
    	m_msgs << i << "   ";
    	m_msgs.width(6);
    	m_msgs << m_joyAxes[i] << "   " << m_joyRaw[i] << endl; }
    for (int i = 0; i < m_countButtons; i++) {
    	m_msgs <<            "    Button ";
    	m_msgs.width(3);
    	m_msgs << i << "   ";
    	m_msgs.width(6);
    	m_msgs << m_joyButtons[i] << endl; }
    return true;
}

















//

