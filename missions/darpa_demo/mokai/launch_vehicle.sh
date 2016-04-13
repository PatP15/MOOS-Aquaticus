#!/bin/bash

# M200_IP
#  Emulator running on same machine as vehicle:     localhost
#  Emulator running on different machine:           IP address of that machine (often 192.168.2.1)
#  Actual evan vehicle:                             192.168.5.1
#  Actual felix vehile:                             192.168.6.1
#M200_IP="localhost"
#M200_IP=192.168.5.1 #evan
#M200_IP=192.168.6.1 #felix

# SHORE_IP
#  Emulation, shoreside running on same machine as vehicle: localhost
#  Emulation, shoreside running on a different machine:     IP address of that machine (often 192.168.2.1)
#  Actual vehicle:                                          IP address of the shoreside computer
#SHORE_IP="localhost"
SHORE_IP=192.168.1.155


WARP=1
HELP="no"
JUST_BUILD="no"
BAD_ARGS=""
MOOS_FILE=""

LOITER_POSA="x=50,y=-125"
DOT_MOOS="meta_vehicle_fld.moos"

printf "Initiate launch vehicle script\n"

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
      HELP="yes"
      UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--mokai" -o "${ARGI}" = "-m" ] ; then
        M200_IP=192.168.1.206 #mokai
        UNDEFINED_ARG=""
        VNAME="mokai"
        VPORT="9013"
        SHARE_LISTEN="9313"
        MOOS_FILE="targ_mokai.moos"
        DOT_MOOS="meta_vehicle_mokai.moos"
        printf "MOKAI vehicle selected.\n"
        LOITER_POSV=$LOITER_POSA
    fi
    if [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
        JUST_BUILD="yes"
        UNDEFINED_ARG=""
        printf "Just building files; no vehicle launch.\n"
    fi
    if [ "${UNDEFINED_ARG}" != "" ] ; then
        BAD_ARGS=$UNDEFINED_ARG
    fi
done

#-------------------------------------------------------
#  Part 2: Handle Ill-formed command-line arguments
#-------------------------------------------------------

if [ "${BAD_ARGS}" != "" ] ; then
    printf "Bad Argument: %s \n" $BAD_ARGS
    exit 0
fi

if [ "${HELP}" = "yes" ]; then
    printf "%s [SWITCHES]            \n" $0
    printf "Switches:                \n"
    printf "  --evan, -e             evan vehicle only                     \n"
    printf "  --felix, -f            felix vehicle only                    \n"
    printf "  --just_build, -j       \n"
    printf "  --help, -h             \n"
    exit 0;
fi

#-------------------------------------------------------
#  Part 3: Create the .moos and .bhv files.
#-------------------------------------------------------

printf "Assembling MOOS file ${MOOS_FILE}\n"

#CRUISESPEED="1.5"
SHORE_LISTEN="9300"

nsplug ${DOT_MOOS} ${MOOS_FILE} -f  \
       VNAME=$VNAME                 \
       VPORT=$VPORT                 \
       WARP=$WARP                   \
       SHARE_LISTEN=$SHARE_LISTEN   \
       SHORE_LISTEN=$SHORE_LISTEN   \
       SHORE_IP=$SHORE_IP           \
       M200_IP=$M200_IP             \
       HOSTIP_FORCE="localhost"     \
       LOITER_POS=$LOITER_POSV      \
       VARIATION=$VARIATION         \
       VTYPE="kayak"                \
       VTEAM="red"

if [ ${JUST_BUILD} = "yes" ] ; then
    printf "Files assembled; vehicle not launched; exiting per request.\n"
    exit 0
fi

#-------------------------------------------------------
#  Part 4: Launch the processes
#-------------------------------------------------------

printf "Launching $VNAME MOOS Community \n"
pAntler $MOOS_FILE >& /dev/null &
uMAC $MOOS_FILE

printf "Killing all processes ... \n "
kill -- -$$
printf "Done killing processes.   \n "
