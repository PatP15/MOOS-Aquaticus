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
  blu("Usage: uFldTagMgr  file.moos [OPTIONS]                    ");
  blu("==========================================================");
  blk("                                                          ");
  showSynopsis();
  blk("                                                          ");
  blk("Options:                                                  ");
  mag("  --alias","=<ProcessName>                                ");
  blk("      Launch uFldTagMgr with the given process name       ");
  blk("      rather than uFldTagMgr.                             ");
  mag("  --example, -e                                           ");
  blk("      Display example MOOS configuration block.           ");
  mag("  --help, -h                                              ");
  blk("      Display this help message.                          ");
  mag("  --interface, -i                                         ");
  blk("      Display MOOS publications and subscriptions.        ");
  mag("  --version,-v                                            ");
  blk("      Display release version of uFldTagMgr.              ");
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
  blu("uFldTagMgr Example MOOS Configuration                           ");
  blu("=============================================================== ");
  blk("                                                                ");
  blk("ProcessConfig = uFldTagMgr                                      ");
  blk("{                                                               ");
  blk("  AppTick   = 4                                                 ");
  blk("  CommsTick = 4                                                 ");
  blk("                                                                ");
  blk("  depth_charge_config         = name=henry,range=35,amt=3       ");
  blk("  depth_charge_range_default  = 25                              ");
  blk("  depth_charge_amount_default = 5                               ");
  blk("  depth_charge_delay_default  = 15                              ");
  blk("  depth_charge_delay_max      = 60                              ");
  blk("                                                                ");
  blk("  replenish_range   = 50                                        ");
  blk("  replenish_time    = 60                                        ");
  blk("  replenish_station = 0,0                                       ");
  blk("                                                                ");
  blk("  visual_hints = drop_color=blue, detination_color=white        ");
  blk("  visual_hints = hit_color=red                                  ");
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
  blu("uFldTagMgr INTERFACE                                            ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("SUBSCRIPTIONS:                                                  ");
  blk("------------------------------------                            ");
  blk("  NODE_REPORT                                                   ");
  blk("  NODE_REPORT = NAME=alpha,TYPE=UUV,TIME=1252348077.59,x=51.7,  ");
  blk("                Y=-35.50,LAT=43.824981,LON=-70.329755,SPD=2.0,  ");
  blk("                HDG=118.8,YAW=118.8,DEPTH=4.6,LENGTH=3.8,       ");
  blk("                MODE=MODE@ACTIVE:LOITERING                      ");
  blk("                                                                ");
  blk("  DEPTH_CHARGE_LAUNCH        = name=archie,delay=30             ");
  blk("  DEPTH_CHARGE_REPLENISH_REQ = name=archie                      ");
  blk("  DEPTH_CHARGE_STATUS_REQ    = name=archie                      ");
  blk("  REPLENISH_CLARIFY          = name=archie                      ");
  blk("                                                                ");
  blk("PUBLICATIONS:                                                   ");
  blk("------------------------------------                            ");
  blk("  DEPTH_CHARGE_HIT    = target=jackal,range =11.9               ");
  blk("  TARGET_HIT_ALL      = jackal                                  ");
  blk("  DEPTH_CHARGE_MISS   = target=jackal,range =92.3               ");
  blk("  VIEW_RANGE_PULSE    = x=-40,y=-150,radius=40,duration=15,     ");
  blk("                        fill=0.25,fill_color=green,label=04,    ");
  blk("                        edge_color=green,time=3892830128.5,     ");
  blk("                        edge_size=1                             ");
  blk("                                                                ");
  blk("  DEPTH_CHARGE_STATUS_VNAME = name=alpha,amt=3,range=25,        ");
  blk("                              launches_ever=2,launches_now=1,   ");
  blk("                              hits=2                            ");
  blk("  REPLENISH_RULES        = station=30:150, range=50, time=60    ");
  blk("  REPLENISH_STATUS_VNAME = name=henry,status=replenishing,      ");
  blk("                           time_remaining=21.3                  ");

  exit(0);
}

//----------------------------------------------------------------
// Procedure: showReleaseInfoAndExit

void showReleaseInfoAndExit()
{
  showReleaseInfo("uFldTagMgr", "gpl");
  exit(0);
}

