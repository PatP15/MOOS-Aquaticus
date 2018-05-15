/************************************************************/
/*    NAME: Caileigh Fitzgerald                                              */
/*    ORGN: MIT                                             */
/*    FILE: LEDInfoBar.h                                          */
/*    DATE: May 5th 2018                                                  */
/************************************************************/

#ifndef LEDInfoBar_HEADER
#define LEDInfoBar_HEADER

#include "MBUtils.h"
#include "MOOS/libMOOS/MOOSLib.h"
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "SerialComms.h"
#include "iLEDInfoBar_enums.h"
#include <cstdlib>
#include <string>
#include <vector>
#include <list>
#include <iterator>
#include <sstream>

class LEDInfoBar : public AppCastingMOOSApp
{
 public:
  LEDInfoBar();
  virtual ~LEDInfoBar();

  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool Iterate();
  bool OnConnectToServer();
  bool OnStartUp();
  void RegisterVariables();

protected:
  //------------------------------------------------
  // struct for list of icons to keep track of LED states
  //
  struct Icon 
  { 
    Icon(TYPE_ENUM t, STATE_ENUM s=m_OFF, std::string str="") 
    {
      // type set in constructor
      // optional state 
      // optional string value, maybe fill in register for vars?
      m_type_str = str;
      m_type     = t; 
      m_state    = s;
    }
    
    std::string m_type_str; // to keep track of different naming conventions
    TYPE_ENUM   m_type; 
    STATE_ENUM  m_state; 

    std::string toString()
    {
      std::stringstream ss;
      ss << "ICON_TYPE=";
      switch(m_type)
      {
        case m_HAVE_FLAG:     ss << "HAVE_FLAG";     break;
        case m_FLAG_ZONE:     ss << "FLAG_ZONE";     break;
        case m_TAGGED:        ss << "TAGGED";        break;
        case m_IN_TAG_RANGE:  ss << "IN_TAG_RANGE";  break;
        case m_OUT_OF_BOUNDS: ss << "OUT_OF_BOUNDS"; break;
        default:
          ss << "NaN";
      }
      ss << ",STATE=";
      switch(m_state)
      {
        case m_OFF:      ss << "OFF";     break;
        case m_ACTIVE:   ss << "ACTIVE";  break;
        case m_BLINKING: ss << "BLINKING";break;
        case m_ERROR:    ss << "ERROR";   break;
        default:
          ss << "NaN";
      }

      return(ss.str());
    }
  }; 

    // relevant to comms with arduino
  void parseSerialString(std::string data);
  bool serialSetup();
  // deconflicts when LED change is illogical
  bool deconflictStates(Icon updated_icon);
  // Standard AppCastingMOOSApp function to overload
  //bool buildReport();

  int           m_baudrate;
  bool          m_valid_serial_connection;
  std::string   m_serial_port;
  SerialComms*  m_serial;

  std::string   m_team_color;
  std::vector<Icon> m_icons;

};

#endif 
