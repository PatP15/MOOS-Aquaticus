/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: TagManager_Info.cpp                                  */
/*    DATE: Sept 20th 2015                                       */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/
 
#include <cstdlib>
#include <iostream>
#include "ColorParse.h"
#include "TagManager_Info.h"
#include "ReleaseInfo.h"

using namespace std;

//----------------------------------------------------------------
// Procedure: showSynopsis

void showSynopsis()
{
  blk("SYNOPSIS:                                                    ");
  blk("------------------------------------                         ");
  blk("  Typically run in a shoreside community. Takes reports from ");
  blk("  remote vehicles, notes their position. Takes a tag request ");
  blk("  from a vehicle and (a) notes whether it currently          ");
  blk("  has remaining tags (b) notes the launch point with         ");
  blk("  with requested target. The manager will apply the tag      ");
  blk("  and notify the tagged and tagging vehicles.                ");
}

//----------------------------------------------------------------
// Procedure: showHelp

void showHelpAndExit()
{
  blk("                                                          ");
  blu("==========================================================");
  blu("Usage: uFldTagManager  file.moos [OPTIONS]                ");
  blu("==========================================================");
  blk("                                                          ");
  showSynopsis();
  blk("                                                          ");
  blk("Options:                                                  ");
  mag("  --alias","=<ProcessName>                                ");
  blk("      Launch uFldTagManager with the given process name   ");
  blk("      rather than uFldTagManager.                         ");
  mag("  --example, -e                                           ");
  blk("      Display example MOOS configuration block.           ");
  mag("  --help, -h                                              ");
  blk("      Display this help message.                          ");
  mag("  --interface, -i                                         ");
  blk("      Display MOOS publications and subscriptions.        ");
  mag("  --version,-v                                            ");
  blk("      Display release version of uFldTagManager.          ");
  blk("                                                          ");
  blk("Note: If argv[2] does not otherwise match a known option, ");
  blk("      then it will be interpreted as a run alias. This is ");
  blk("      to support pAntler launching conventions.           ");
  blk("                                                          ");
  exit(0);
}


//----------------------------------------------------------------
// Procedure: showExampleConfigAndExit

void showExampleConfigAndExit()
{
  blu("=============================================================== ");
  blu("uFldTagManager Example MOOS Configuration                       ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = uFldTagManager                                  ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  vtag_range = 50     // default (meters)                       ");
  blk("                                                                ");
  blk("  post_color = white  // default                                ");
  blk("  miss_color = blue   // default                                ");
  blk("  hit_color  = red    // default                                ");
  blk("                                                                ");
  blk("  team_one = red                                                ");
  blk("  team_two = blue                                               ");
  blk("                                                                ");
  blk("  zone_one = pts={0,-20:120,-20:120,-100:0,-100}                ");
  blk("  zone_two = pts={0,-100:120,-100:120,-180:0,-180}              ");
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
  blu("uFldTagManager INTERFACE                                        ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  NODE_REPORT = NAME=alpha,TYPE=UUV,TIME=1252348077.59,x=51.7,  ");
  blk("                Y=-35.50,LAT=43.824981,LON=-70.329755,SPD=2.0,  ");
  blk("                HDG=118.8,YAW=118.8,DEPTH=4.6,LENGTH=3.8,       ");
  blk("                MODE=MODE@ACTIVE:LOITERING                      ");
  blk("                                                                ");
  blk("  TAG_POST    = vname=henry                                     ");
  blk("  APPCAST_REQ                                                   ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  TAG_RESULT         = event=23,source=henry,                   ");
  blk("                       result=rejected_toofreq                  ");
  blk("  TAG_RESULT         = event=23,source=henry,                   ");
  blk("                       tagged=gus                               ");
  blk("  TAG_RESULT_VERBOSE =                                          ");
  blk("  VIEW_RANGE_PULSE   = x=-40,y=-150,radius=40,duration=15,     ");
  blk("                       fill=0.25,fill_color=green,label=04,    ");
  blk("                       edge_color=green,time=3892830128.5,     ");
  blk("                       edge_size=1                             ");

  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("uFldTagManager", "gpl");
  exit(0);
}

