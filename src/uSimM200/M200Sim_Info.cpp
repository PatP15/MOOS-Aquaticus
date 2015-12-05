

#include <cstdlib>
#include <iostream>
#include "M200Sim_Info.h"
#include "ColorParse.h"
#include "ReleaseInfo.h"

using namespace std;

void showSynopsis()
{
  blk("SYNOPSIS:                                                       ");
  blk("------------------------------------                            ");
  blk("  Simulates the front seat computer of a Clearpath Robotics Kingfisher M200.");
  blk("  This application is meant to run in a MOOS community that is also launches");
  blk("  pHelmIvP, pMarinePID, and uSimMarine. Together this MOOS community runs a ");
  blk("  simulated vehicle. uSimM200 provides the interface that interacts on a TCP");
  blk("  port, appearing the same as the actual M200's NMEA interface.             ");
}

void showHelpAndExit()
{
  blk("                                                                ");
  blu("=============================================================== ");
  blu("Usage: uSimM200 file.moos [OPTIONS]                             ");
  blu("=============================================================== ");
  blk("                                                                ");
  showSynopsis();
  blk("                                                                ");
  blk("Options:                                                        ");
  mag("  --example, -e                                                 ");
  blk("      Display example MOOS configuration block.                 ");
  mag("  --help, -h                                                    ");
  blk("      Display this help message.                                ");
  mag("  --interface, -i                                               ");
  blk("      Display MOOS publications and subscriptions.              ");
  blk("                                                                ");
  blk("Note: If argv[2] does not otherwise match a known option,       ");
  blk("      then it will be interpreted as a run alias. This is       ");
  blk("      to support pAntler launching conventions.                 ");
  blk("                                                                ");
  exit(0);
}

void showExampleConfigAndExit()
{
    // 0        1         2         3         4         5         6         7
    // 1234567890123456789012345678901234567890123456789012345678901234567890123456789
  blk("                                                                              ");
  blu("============================================================================= ");
  blu("uSimM200 Example MOOS Configuration                                           ");
  blu("============================================================================= ");
  blk("                                                                              ");
  blk("ProcessConfig = uSimM200                                                      ");
  blk("{                                                                             ");
  blk("  AppTick           = 10                                                      ");
  blk("  CommsTick         = 10                                                      ");
  blk("  PORT or                                                                     ");
  blk("     PORT_NUMBER    = 29500   // Port number on localhost to publish out to   ");
  blk("                              // Defaults to Cleapath M200 port of 29500      ");
  blk("}                                                                             ");
  blk("                                                                              ");
  exit(0);
}

void showInterfaceAndExit()
{
    // 0        1         2         3         4         5         6         7
    // 1234567890123456789012345678901234567890123456789012345678901234567890123456789
  blk("                                                                              ");
  blu("============================================================================= ");
  blu("iM200 INTERFACE                                                               ");
  blu("============================================================================= ");
  blk("                                                                              ");
  showSynopsis();
  blk("                                                                              ");
  blk("SUBSCRIPTIONS:                                                                ");
  blk("------------------------------------                                          ");
  blk(" NAV_LAT       double     Current latitude of simulated vehicle               ");
  blk(" NAV_LON or                                                                   ");
  blk("    NAV_LONG   double     Current longitude of simulated vehicle              ");
  blk(" NAV_HEADING   double     Current heading relative to true N                  ");
  blk(" NAV_SPEED     double     Current simulated speed in m/s                      ");
  blk("                                                                              ");
  blk("PUBLICATIONS:                                                                 ");
  blk("------------------------------------                                          ");
  blk(" THRUST_MODE_DIFFERENTIAL bool    When TRUE, asks the simulator to respond to ");
  blk("                                  thrust left and right commands.             ");
  blk("                                  When FALSE, asks the simulator to respond to");
  blk("                                  commands for rudder and thrust.             ");
  blk(" DESIRED_THRUST_L         double  Desired thrust to simulate for left motor   ");
  blk(" DESIRED_THRUST_R         double  Desired thrust to simulate for right motor  ");
  blk(" DESIRED_RUDDER           double  Desired rudder angle to simulate            ");
  blk(" DESIRED_THRUST           double  Desired percent thrust to simulate          ");
  blk("                                                                              ");
  exit(0);
}

