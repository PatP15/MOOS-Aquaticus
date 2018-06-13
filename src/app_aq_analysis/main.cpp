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
#include <string>
#include "MBUtils.h"
#include "ReleaseInfo.h"
#include "GrepHandler.h"
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
  
  bool handled = true;
  for(int i=1; i<argc; i++) {
    string argi = argv[i];
    if((argi=="-h") || (argi == "--help"))
      showHelpAndExit();
    else if((argi=="-v") || (argi == "--version")) 
      showReleaseInfoAndExit
        ("zaic_hdg", "gpl");
    else if(strBegins(argi, "--domain=")) {
      string domain_str = argi.substr(9);
      domain = vclip(atoi(domain_str.c_str()), 100, 1000);
    }
    else if(strBegins(argi, "--verbose")) 
      verbose = true;
    else
      handled = false;

    if(!handled) {
      cout << "Exiting due to Unhandled arg: " << argi << endl;
      exit(1);
    }      
  }

  //some variable names to look for
  vector<string> keys;
  keys.push_back("TEAMSPEAK");
  keys.push_back("SAY_MOOS");

  GrepHandler handler;
  //  handler.setFileOverWrite(file_overwrite);
  //handler.setCommentsRetained(comments_retained);
  //handler.setBadLinesRetained(badlines_retained);
  //handler.setGapLinesRetained(gaplines_retained);
  //handler.setAppCastRetained(appcast_retained);

  int ksize = keys.size(); 
  for(int i=0; i<ksize; i++)
    handler.addKey(keys[i]);

  string alogfile_in = "test.alog";
  string alogfile_out = "";

  bool handled_alog = handler.handle(alogfile_in, alogfile_out);
  if(!handled_alog)
    exit(1);

  bool make_end_report = true;
  if(handled_alog && make_end_report)
    handler.printReport();
 
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
  Fl_Chart chart2( 0,150,100,100,"Blue Two");
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

  chart2.insert(1, 10,"0", 12);
  chart2.insert(2, 10,"1", 216);
  chart2.insert(3, 10,"2", 80);
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








