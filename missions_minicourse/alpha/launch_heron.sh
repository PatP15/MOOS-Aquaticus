#!/bin/bash
#----------------------------------------------------------
#  Script: launch_heron.sh
#  Author: Michael Benjamin
#  LastEd: Nov 13th, 2020
#----------------------------------------------------------
#  Part 1: Set global var defaults
#----------------------------------------------------------
WARP=1

SHORE_IP="localhost"
SHORE_LISTEN="9300"

TRAIL_RANGE="3"
TRAIL_ANGLE="330"
VNAME="evan"
VTEAM="red"
LCLOCK="false"
JUST_BUILD="no"

VERBOSE="false"
START_POS=""
RETURN_POS=""
LOITER_POS="x=100,y=-180"
GRAB_POS=""
UNTAG_POS=""
DEFEND_POS=""

echo "Initiate launch_heron.sh script"

#-------------------------------------------------------
#  Part 2: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	echo "launch_heron.sh [SWITCHES]                                "
	echo "Switches:                                                 "
	echo "  --vname=NAME      Set vehicle name {evan,felix,gus,hal} "
	echo "  --vteam=TEAM      Set team name {red,blue}              "
	echo "  --warp=WARP       Set the time warp                     "
	echo "  --just_build, -j  Just build files                      "
	echo "  --help, -h                                              "
	exit 0;
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$WARP" = 1 ]; then
        WARP=$ARGI
    elif [ "${ARGI:0:7}" = "--warp=" ] ; then
        WARP="${ARGI#--warp=*}"
    elif [ "${ARGI:0:8}" = "--vname=" ] ; then
        VNAME="${ARGI#--vname=*}"
    elif [ "${ARGI:0:8}" = "--vteam=" ] ; then
        VTEAM="${ARGI#--vteam=*}"
    elif [ "${ARGI:0:11}" = "--startpos=" ] ; then
        START_POS="${ARGI#--startpos=*}"
    elif [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
        JUST_BUILD="yes"
    elif [ "${ARGI}" = "--lclock" -o "${ARGI}" = "-l" ] ; then
        LCLOCK="true"
    else
	echo "launch_heron.sh Bad Arg:" $ARGI
	exit 1
    fi
done

#-------------------------------------------------------
# Part 2: Set Vehicle specific parameters
#-------------------------------------------------------
if [ "${VNAME}" = "evan" ] ; then
    VPORT="9005"
    SHARE_LISTEN="9305"
elif [ "${VNAME}" = "felix" ] ; then
    VPORT="9006"
    SHARE_LISTEN="9306"
elif [ "${VNAME}" = "gus" ] ; then
    VPORT="9007"
    SHARE_LISTEN="9307"
elif [ "${VNAME}" = "hal" ] ; then
    VPORT="9008"
    SHARE_LISTEN="9308"
else
    echo "Unrecognized or no vehicle name selected: $VNAME";
    exit 1
fi

echo "$VNAME vehicle selected."
    
#-------------------------------------------------------
# Part 3: Set Team specific parameters
#-------------------------------------------------------
if [ "${VTEAM}" = "red" ] ; then
    GRAB_POS="20,40"
    RETURN_POS="140,40"
    UNTAG_POS="140,40"     
    DEFEND_POS="100,40"     
elif [ "${VTEAM}" = "blue" ] ; then
    GRAB_POS="140,40"
    RETURN_POS="20,40"
    UNTAG_POS="20,40"    
    DEFEND_POS="60,40"    
else
    echo "Unrecognized or no vehicle team selected: $VTEAM";
    exit 1
fi

echo "$VTEAM vehicle team selected."
    
#-------------------------------------------------------
#  Part 4: Create the .moos and .bhv files.
#-------------------------------------------------------
echo "Assembling MOOS file targ_${VNAME}.moos "

nsplug meta_heron.moos targ_${VNAME}.moos -f --strict     \
       VNAME=$VNAME          SHARE_LISTEN=$SHARE_LISTEN  \
       VPORT=$VPORT          SHORE_LISTEN=$SHORE_LISTEN  \
       WARP=$WARP            SHORE_IP=$SHORE_IP          \
       VTEAM=$VTEAM          HOSTIP_FORCE="localhost"    \
       VTYPE="kayak"         START_POS=$START_POS  
    
echo "Assembling BHV file targ_${VNAME}.bhv"
nsplug meta_heron.bhv targ_${VNAME}.bhv -f --strict      \
       RETURN_POS=${RETURN_POS}     VTEAM=$VTEAM         \
       VNAME=$VNAME                 GRAB_POS=$GRAB_POS   \
       DEFEND_POS=$DEFEND_POS       UNTAG_POS=$UNTAG_POS \
       LCLOCK=$LCLOCK

if [ ! -e targ_${VNAME}.moos ]; then echo "no targ_${VNAME}.moos"; exit; fi
if [ ! -e targ_${VNAME}.bhv ];  then echo "no targ_${VNAME}.bhv"; exit; fi

if [ ${JUST_BUILD} = "yes" ] ; then
    echo "Files assembled; vehicle not launched; exiting per request."
    exit 0
fi

#-------------------------------------------------------
#  Part 4: Launch the processes
#-------------------------------------------------------

echo "Launching $VNAME MOOS Community "
pAntler targ_${VNAME}.moos >& /dev/null &

#uMAC targ_${VNAME}.moos
# "Killing all processes ...  "
#kill -- -$$
#printf "Done killing processes.    "
