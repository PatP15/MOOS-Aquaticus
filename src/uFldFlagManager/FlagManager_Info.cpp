/************************************************************/
/*   NAME: Mike Benjamin                                    */
/*   ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA */
/*   FILE: FlagManager.h                                    */
/*   DATE: August 18th, 2015                                */
/************************************************************/

#include <cstdlib>
#include <iostream>
#include "FlagManager_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("  The uFldFlagManager is a shoreside manager used for marine    ");
  blk("  autonomy competitions where flags are involved. Flags are     ");
  blk("  declared at the outset each with a position and a unique      ");
  blk("  label. Vehicles have the ability to grab a flag by posting a  ");
  blk("  request. The flag may or not be granted, but if granted, then ");
  blk("  the grabbing vehicle then owns the flag and it cannot be      ");
  blk("  grabbed by other vehicles.                                    ");
}

//----------------------------------------------------------------
// Procedure: showHelpAndExit

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: uFldFlagManager file.moos [OPTIONS]                      ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --alias","=<ProcessName>                                      ");
  blk("      Launch uFldFlagManager with the given process name        ");
  blk("      rather than uFldFlagManager.                              ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  mag("  --version,-v                                                  ");
  blk("      Display the release version of uFldFlagManager.           ");
  blk("                                                                ");
  blk("Note: If argv[2] does not otherwise match a known option,       ");
  blk("      then it will be interpreted as a run alias. This is       ");
  blk("      to support pAntler launching conventions.                 ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showExampleConfigAndExit

void showExampleConfigAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uFldFlagManager Example MOOS Configuration                      ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = uFldFlagManager                                 ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  default_flag_width = 5        // Default is 5 meters          ");
  blk("  default_flag_type  = circle   // Default is circle            ");
  blk("  default_flag_range = 10       // Default is 10 meters         ");
  blk("                                                                ");
  blk("  grabbed_color      = white    // Default is white             ");
  blk("  ungrabbed_color    = red      // Default is red               ");
  blk("                                                                ");
  blk("  flag = x=60, y=-30,  label=one, range=20, width=10            ");
  blk("  flag = x=60, y=-170, label=two, color=purple, type=diamond    ");
  blk("}                                                               ");
  blk("                                                                ");
  exit(0);
}


//----------------------------------------------------------------
// Procedure: showInterfaceAndExit

void showInterfaceAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("uFldFlagManager INTERFACE                                       ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("       NODE_REPORT = NAME=alpha,TYPE=UUV,TIME=1252348077.59,    ");
  blk("                      X=51.71,Y=-35.50, LAT=43.824981,          ");
  blk("                      LON=-70.329755,SPD=2.0,HDG=118.8,         ");
  blk("                      YAW=118.8,DEPTH=4.6,LENGTH=3.8,           ");
  blk("                      MODE=MODE@ACTIVE:LOITERING,               ");
  blk("                      THRUST_MODE_REVERSE=true                  ");
  blk("         FLAG_RESET = vname=henry                               ");
  blk("         FLAG_RESET = label=alpha                               ");
  blk("  FLAG_GRAB_REQUEST = vname=henry                               ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  HAS_FLAG_VNAME     = true/false                               ");
  blk("  HAS_FLAG_ALL       = false (upon global reset)                ");
  blk("  VTEAM_FLAG_GRABBED = grabbed=one,grabbed=two                  ");
  blk("  FLAG_GRAB_REPORT   = grabbed=one,grabbed=two                  ");
  blk("  FLAG_GRAB_REPORT   = nothing_grabbed                          ");
  blk("  VIEW_MARKER        = x=60,y=-30,width=2,range=10.00,          ");
  blk("                       primary_color=red,secondary_color=black, ");
  blk("                       type=circle,label=one                    ");
  blk("                                                                ");
  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("uFldFlagManager", "gpl");
  exit(0);
}

