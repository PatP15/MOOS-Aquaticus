/*
 * moosJoyInfo.cpp
 *
 *  Created on: Sep 24 2015
 *      Author: Alon Yaari
 */

#include <cstdlib>
#include <iostream>
#include "ColorParse.h"
#include "ReleaseInfo.h"
#include "moosJoyInfo.h"

using namespace std;

void showSynopsis()
{
  blk("SYNOPSIS:");
  blk("------------------------------------");
  blk(" Reads a joystick and publishes axis and button info as MOOS messages.");
  blk(" Designed to be cross-platform and joystick-agnostic.");
}

void showHelpAndExit()
{
  blk("");
  blu("============================================================================");
  blu("Usage: iJoystick file.moos [OPTIONS]");
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
  blu("iJoystick Example MOOS Configuration");
  blu("============================================================================");
  blk("");
  blk("ProcessConfig = iJoystick");
  blk("{");
  blk("  AppTick    = 10");
  blk("  CommsTick  = 10");
  blk("");
  blk("  JoystickID = 0         // If more than one joystick is connected, this is the");
  blk("                         //   0-based index ID of the joystick to report on.");
  blk("                         // Default is 0.");
  blk("  Output_Prefix = JOY0_  // All output for this joystick will be published");
  blk("                         //   with this prefix on the message names.");
  blk("                         //   Default is JOY_");
  blk("");
  blk("  //  raw values -32768            0            32767");
  blk("  //                  |---.-----.--|--.-----.---|");
  blk("  //  translated        a |  b  |  c  |  d  | e");
  blk("  //                      W     X     Y     Z");
  blk("  //     W = -32768 + DEAD_END     a: Always reports -MAX_AXIS");
  blk("  //     X = 0 - DEAD_ZERO         b: Between 0 and -MAX_AXIS");
  blk("  //     Y = DEAD_ZERO             c: Always 0");
  blk("  //     Z = 32767 - DEAD_END      d: Between 0 and MAX_AXIS");
  blk("  //                               e: Always MAX_AXIS");
  blk("");
  blk("  Max_Axis  = 1000      // Axes are normalized to +/- this value");
  blk("  Dead_End  = 200       // Taken off the raw value at the +/- ends");
  blk("  Dead_Zero = 500       // Between +/- this value reports 0 (centered)");
  blk("");
  blk("}");
  blk("");
  exit(0);
}

void showInterfaceAndExit()
{
	  blk("");
  blu("============================================================================");
  blu("iJoystick INTERFACE                                                           ");
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
