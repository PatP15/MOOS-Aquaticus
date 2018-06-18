/*****************************************************************/
/*    NAME: Michael "Misha" Novitzky                             */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: main.cpp                                             */
/*    DATE: June 13th, 2018                                      */
/*                                                               */
/*  Borrowed heavily from MOOS-IvP applications:                 */
/*  1) app_aloggrep for parsing .alog files                      */
/*  2) app_zaic_hdg for FLTK integration                         */
/*  Both apps by: Michael R. Benjamin                            */
/*****************************************************************/

#include <iostream>
#include <cstdlib>
#include <string.h>
#include "MBUtils.h"
#include "LogUtils.h"
#include "ReleaseInfo.h"
#include "GrepHandler.h"
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Chart.H>
 
using namespace std;

void showHelpAndExit();

void idleProc(void *);

//----------------------/----------------------------------
// Procedure: idleProc

void idleProc(void *)
{
  Fl::flush();
  millipause(10);
}

void but_cb(Fl_Widget* o, void*) {
  Fl_Button* b=(Fl_Button* ) o;
  b->label("Good job");

  b->resize(10,150,150,30);
  b->redraw();
}

//--------------------------------------------------------
// Procedure: main

int main(int argc, char *argv[])
{
  bool verbose = false;
  int  domain  = 360;
  string alogfile_in = "";
  string alogfile_out = "";

 
  bool handled = true;
  for(int i=1; i<argc; i++) {
    string argi = argv[i];
    if((argi=="-h") || (argi == "--help"))
      showHelpAndExit();
    else if((argi=="-v") || (argi == "--version")) 
      showReleaseInfoAndExit
        ("aq_analysis", "gpl");
       else if(strBegins(argi, "--verbose")) 
      verbose = true;
       else if(strEnds(argi,".alog"))
         alogfile_in = argi;
    else
      handled = false;

    if(!handled) {
      cout << "Exiting due to Unhandled arg: " << argi << endl;
      exit(1);
    }      
  }

  //catch not assigning alog file
  if(alogfile_in == "") {
    cout << "Please specify an .alog file" << endl;
    exit(1);
  }
  
  //some variable names to look for
  vector<string> keys;
  keys.push_back("TEAMSPEAK");
  keys.push_back("SAY_MOOS");

  GrepHandler handler;
  //  handler.setFileOverWrite(file_overwrite);
  //handler..setCommentsRetained(comments_retained);
  //handler.setBadLinesRetained(badlines_retained);
  //handler.setGapLinesRetained(gaplines_retained);
  //handler.setAppCastRetained(appcast_retained);

  int ksize = keys.size(); 
  for(int i=0; i<ksize; i++)
    handler.addKey(keys[i]);

   bool handled_alog = handler.handle(alogfile_in, alogfile_out);
  if(!handled_alog)
    exit(1);

  bool make_end_report = true;
  if(handled_alog && make_end_report)
    handler.printReport();

  //our addition to GrepHandler is a vector of strings keeping the lines
  //that would otherwise be printed to console are now kept in m_kept_lines

  std::vector<double> say_moos_times;

  for(int j = 0;j < handler.m_kept_lines.size();j++) {
    std::string to_parse = handler.m_kept_lines[j];
    //let's search for SAY_MOOS
    std::string curr_var;
    curr_var = getVarName(to_parse);
    if(curr_var == "SAY_MOOS") {
      //save the time for use in plotting window
      cout<<endl<< "Kept SAY_MOOS " << to_parse <<endl;
      std::string curr_time = getTimeStamp(to_parse);
      cout<< "Kept SAY_MOOS time: " << curr_time << endl;
      //let's keep track of the times by converting it to a double for use for plotting later.
      double dbl_curr_time = atof(curr_time.c_str());
      say_moos_times.push_back(dbl_curr_time);

    }
    }
 
 

  //Several ideas on information display
  //1) one participant timeline with different color bars per speaker
  //2) a window per participant with a bar per speaker to that participant

  //participant speaks to
  //1) game 2) robots 3) human players 4) self?

  //incoming to participant includes
  //1) isay: Game + Robots 2) Teamspeak for human players
  
  Fl_Window win(400,400,"Testing");
  win.begin();
  Fl_Chart chart1( 0,0,100,100,"Blue One");
  Fl_Chart chart2( 0,150,200,100,"SAY_MOOS");
  Fl_Button but( 10, 300, 70, 30, "Click me");
  win.end();
  chart1.type(FL_FILL_CHART);
  chart2.type(FL_FILL_CHART);
  chart1.add(10,"1st val",10);
  chart1.add(50, "2nd va", 80);
  chart1.insert(1, 10,"1", 12);
  chart1.insert(2, 10,"2", 77);

  chart1.insert(3, 10,"3", 216);
  chart1.insert(4, 10,"4", 49); //49 is default grey
  chart1.insert(5, 10,"5", 12);

  //let's build a timeline using the vector
  //remember that the insert starts at 1 with the "0" and increments from there
  //let's perform a while loop until the vector runs out -- only coloring vector
  //specified events and the rest grey
  std::vector<double>::iterator it = say_moos_times.begin();
  int time_increment = 0;

  while(it != say_moos_times.end()) {
    std::string str_time = intToString(time_increment);
    int color_choice;
    int say_event_time_int = *it;

    if(say_event_time_int == time_increment) {
      color_choice = 12;
      ++it;
    }
    else {
      color_choice = 49; //49 is default color
    }
    chart2.insert(time_increment+1, 10,str_time.c_str(), color_choice);
    time_increment++;
  }

  but.callback(but_cb);
  win.show();
  Fl::add_idle(idleProc);

  // Enter the GUI event loop.
  return Fl::run();
}

//--------------------------------------------------------
// Procedure: showHelpAndExit()

void showHelpAndExit()
{
  cout << endl;
 cout << "Usage: zaic_hdg [OPTIONS]                           " << endl;
  cout << "Options:                                            " << endl;
  cout << "  --help, -h           Display this help message    " << endl;
  cout << "  --domain=360         Set upper value of domain    " << endl;
  cout << "  --verbose,           Enable verbose output        " << endl;
  cout << "  --version, -v,       Display the release version  " << endl;
  cout << "                                                    " << endl;
  cout << "Example:                                            " << endl;
  cout << " $ zaic_hdg --domain=500 --verbose                  " << endl;
  exit(0);
}








