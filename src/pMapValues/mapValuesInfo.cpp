/*
 * mapValuesInfo.cpp
 *
 *  Created on: Sep 30, 2015
 *      Author: Alon Yaari
 */

#include "mapValuesInfo.h"

#include <cstdlib>
#include <iostream>
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

void showSynopsis()
{
  blk("SYNOPSIS:");
  blk("------------------------------------");
  blk("   Publishes an actuation command based on subscribed values. The main purpose");
  blk(" of this application is to provide a link between a joystick reading MOOS");
  blk(" module and actuating a vessel. However, this workflow can be used to convert");
  blk(" any numerical input range into any numerical output range.");
  blk("");
  blk("   Button presses can also be monitored. A numerical or string publication will");
  blk(" trigger a publication. Triggering occurs when the published message switches to");
  blk(" the specified trigger value.");
  blk("");
  blk(" Examples:");
  blk("");
  blk("    Joystick axis 1 is used to control the rudder angle:");
  blk("         subscribe to: JOY_AXIS_1       range [-1000,1000]");
  blk("         publish:      DESIRED_RUDDER   range[-40,40]");
  blk("");
  blk("    Button 1 is used to signify all-stop:");
  blk("         subscribe to: JOY_BUTTON_1     possible values of 0 or 1");
  blk("         trigger on:   JOY_BUTTON_1 = 1 (0 when not pressed, 1 when pressed)");
  blk("         publish:      ALL_STOP = true when JOY_BUTTON_1 changes from 0 to 1");
  blk("");
}

void showHelpAndExit()
{
  blk("");
  blu("============================================================================");
  blu("Usage: pMapValues file.moos [OPTIONS]");
  blu("============================================================================");
  blk("");
  showSynopsis();
  blk("");
  blk("Options:                                                                    ");
  mag("  --example, -e                                                             ");
  blk("      Display example MOOS configuration block.                             ");
  mag("  --help, -h                                                                ");
  blk("      Display this help message.                                            ");
  mag("  --interface, -i                                                           ");
  blk("      Display MOOS publications and subscriptions.                          ");
  blk("");
  blk("Note: If argv[2] does not otherwise match a known option, then it will be   ");
  blk("      interpreted as a run alias. This is to support pAntler conventions.   ");
  blk("");
  exit(0);
}

void showExampleConfigAndExit()
{
//     0         1         2         3         4         5         6         7
//     01234567890123456789012345678901234567890123456789012345678901234567890123456789
  blk("");
  blu("============================================================================");
  blu("pMapValues Example MOOS Configuration");
  blu("============================================================================");
  blk("");
  blk("ProcessConfig = pMapValues");
  blk("{");
  blk("  AppTick    = 10");
  blk("  CommsTick  = 10");
  blk("");
  blk("  // RANGE = in_msg=, in_min=, in_max=, dead=, sat=, norm_min=, norm_max=, /");
  blk("  //         out_msg=, out_min=, out_max=, dep=, trig_msg=, trig_val=");
  blk("  //      in_msg   (string, required) Message name for incoming range values");
  blk("  //      in_min   (double, required) Minimum expected value on the input message");
  blk("  //      in_max   (double, required) Maximum expected value on the input message");
  blk("  //      norm_min (double, optional) Minimum normalized value to map to output (default = -1.0)");
  blk("  //      norm_max (double, optional) Maximum normalized value to map to output (default = 1.0)");
  blk("  //      out_msg  (string, required) Mapped value published to this message");
  blk("  //      out_min  (double, required) Minimum value input range is mapped to");
  blk("  //      out_max  (double, required) Maximum value input range is mapped to");
  blk("  //      dead     (int,    optional) Percent to each side of 0 that reports 0");
  blk("  //      sat      (int,    optional) Percent at each end that reports extreme value");
  blk("  //                                    Use single-value input for any other");
  blk("  //                                    value or if omitted.");
  blk("  //      dep      (string, optional) Message name of dependent axis");
  blk("  //      trig_msg (string, optional) Publish mapped value only when on this trigger message");
  blk("  //      trig_val (any,    required) Publish only when trig_msg has this value.");
  blk("  RANGE = in_msg=JOY_AXIS_0, in_min=-32768, in_max=32768, dead=5, sat=5, /");
  blk("            out_msg=DESIRED_RUDDER, out_min=-40, out_max=40, trig_msg=JOY_BUTTON_0, trig_val=DOWN");
  blk("  RANGE = in_msg=JOY_AXIS_1_DEP, in_min=-32768, in_max=32768, dead=5, sat=5, /");
  blk("            norm_min = 0.0, norm_max = -1.0, out_msg=DESIRED_THRUST, out_min=0, out_max=100");
  blk("");
  blk("  // TRIGGER = in_msg=w, trigger=x, out_msg=y, out_val=z");
  blk("  //      in_msg  (string, required) Message name for incoming switch value");
  blk("  //      trigger (any,    required) When in_msg contents change to match this trigger,");
  blk("  //                                   the out_msg will be published. String/numeric agnostic.");
  blk("  //      out_msg (string, required) Message name for resulting publication.");
  blk("  //      out_val (any,    required) Resulting publication posts this value.");
  blk("  //                                   If value is a numeric (within '+-.01234567889'),");
  blk("  //                                   published message is a double. Otherwise, a");
  blk("  //                                   string is published. To publish a numeric as a");
  blk("  //                                   string, put the number in quotes.");
  blk("   TRIGGER   = in_msg=JOY_BUTTON_4, trigger=DOWN, out_msg=ALL_STOP, out_val=true");
  blk("}");
  blk("");
  exit(0);
}

void showInterfaceAndExit()
{
	  blk("");
  blu("============================================================================");
  blu("pMapValues INTERFACE                                                           ");
  blu("============================================================================");
  blk("");
  showSynopsis();
  blk("");
  blk("SUBSCRIPTIONS:");
  blk("------------------------------------ ");
  blk("  None.");
  blk("");
  blk("PUBLICATIONS:                                                               ");
  blk("------------------------------------                                        ");
  blk("  [prefix] is defined in the Output_predix mission file parameter");
  blk("  [X]      is the channel number for the current joystick");
  blk("  [prefix]_AXIS_COUNT       Number of valid axes current joystick reports");
  blk("  [prefix]_AXIS_[X]         Axis position value published for each valid joystick axis.");
  blk("                              ONLY PUBLISHED WHEN values change.");
  blk("                              Leftmost:    -32768");
  blk("                              Rightmost:    32767");
  blk("                              Centered:         0");
  blk("                              Example:      JOY0_AXIS_3 = 5162");
  blk("  [prefix]_AXIS_[X]_DEP     If an axis has a dependency, a _DEP version of the");
  blk("                              message is used that contains both values.");
  blk("  [prefix]_BUTTON_COUNT     Number of valid buttons current joystick reports");
  blk("  [prefix]_BUTTON_[X]       Binary button status published for each valid joystick button.");
  blk("                              ONLY PUBLISHED WHEN the button status changes.");
  blk("                              Pressed:      DOWN");
  blk("                              Not pressed:  UP");
  blk("                              Example:      JOY2_BUTTON_6 = ");
  blk("");
  exit(0);
}
























//
