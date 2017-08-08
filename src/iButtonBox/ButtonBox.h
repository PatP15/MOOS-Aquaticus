/************************************************************/
/*    NAME: Carter Fendley                                  */
/*    ORGN: MIT                                             */
/*    FILE: ButtonBox.h                                     */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef ButtonBox_HEADER
#define ButtonBox_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "SerialComms.h"

class ButtonBox : public AppCastingMOOSApp
{
 public:
   ButtonBox();
   ~ButtonBox();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();
   void parseSerialString(std::string data);
   bool serialSetup();

  std::string getName(int button_index);

 private: // Configuration variables
  std::string m_serial_port;
  int m_baudrate;

  SerialComms* m_serial;
  bool m_valid_serial_connection;

  std::map <std::string, std::string> m_button_names;
  std::vector<bool> m_button_values;
};

#endif 
