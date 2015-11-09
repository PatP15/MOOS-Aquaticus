/************************************************************/
/*    NAME: Michael "Misha" Novitzky                        */
/*    ORGN: MIT                                             */
/*    FILE: SpeechRec.h                                     */
/*    DATE: August 13th, 2015                               */
/************************************************************/

#ifndef SpeechRec_HEADER
#define SpeechRec_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "MOOS/libMOOS/Utils/MOOSThread.h"
#include <string>
#include <queue>
#include <julius/juliuslib.h>

class SpeechRec : public AppCastingMOOSApp
{
 public:
  SpeechRec();
  ~SpeechRec() {};

 protected: // Standard MOOSApp functions to overload  
  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool Iterate();
  bool OnConnectToServer();
  bool OnStartUp();
  static void statusRecReady(Recog *recog, void *dummy);
  static void statusRecStart(Recog *recog, void *dummy);
  bool handleJuliusConf(std::string fileName);
  static void outputResult(Recog *recog, void *dummy);
  static bool startJRecognize(void* param);
  bool internalStartJRecognize();
 protected: // Standard AppCastingMOOSApp function to overload 
  bool buildReport();

 protected:
  void registerVariables();

 private: // Configuration variables
  CMOOSThread* m_t;
  static CMOOSLock m_message_lock;
  static std::queue<std::string> m_messages;
  static std::queue<std::string> m_scores;
  bool m_julius_debug;
  std::string m_julius_configuration;
  uint m_message_sequence;
  Jconf *m_jconf;
  Recog *m_recog;


 private: // State variables
  enum program_state {RUNNING, ERROR_CONFIG_FILE, ERROR_CREATE_JULIUS, 
		      ERROR_AUDIO_INIT, ERROR_AUDIO_INPUT, ERROR_AUDIO_START, ERROR_RECOG_LOOP};

  program_state m_state;

};

#endif 
