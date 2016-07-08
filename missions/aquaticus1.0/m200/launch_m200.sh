#!/bin/bash
WARP=1

SHORE_IP=192.168.1.155
SHORE_LISTEN="9300"

TRAIL_RANGE="3"
TRAIL_ANGLE="330"
HELP="no"
JUST_BUILD="no"
VTEAM="red"

START_POS="56,16,240"
RETURN_POS=""
LOITER_POS="x=100,y=-180"
GRAB_POS=""
UNTAG_POS=""

printf "Initiate launch vehicle script\n"

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP="yes"
    elif [ "${ARGI}" = "--evan" -o "${ARGI}" = "-e" ] ; then
        M200_IP=192.168.5.1 #evan
        VNAME="evan"
        VPORT="9005"
        SHARE_LISTEN="9305"
        printf "EVAN vehicle selected as HUNTER.\n"
    elif [ "${ARGI}" = "--felix" -o "${ARGI}" = "-f" ] ; then
        M200_IP=192.168.6.1 #felix
        VNAME="felix"
        VPORT="9006"
        SHARE_LISTEN="9306"
        printf "FELIX vehicle selected.\n"
    elif [ "${ARGI}" = "--gus" -o "${ARGI}" = "-g" ] ; then
        M200_IP=192.168.7.1 #gus
        VNAME="gus"
        VPORT="9007"
        SHARE_LISTEN="9307"
        printf "GUS vehicle selected as HUNTER.\n"
    elif [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
        JUST_BUILD="yes"
        printf "Just building files; no vehicle launch.\n"
    elif [ "${ARGI}" = "--sim" -o "${ARGI}" = "-s" ] ; then
        SIM="SIM"
        printf "Simulation mode ON.\n"
    elif [ "${ARGI}" = "--red" -o "${ARGI}" = "-r" ] ; then
        VTEAM="red"
        GRAB_POS="-57,-71"
        UNTAG_POS="50,-26"
        RETURN_POS="50,-26"
        START_POS="56,16,240"
        printf "Red team selected.\n"
    elif [ "${ARGI}" = "--blue" -o "${ARGI}" = "-b" ] ; then
        VTEAM="blue"
        GRAB_POS="50,-26"
        UNTAG_POS="-57,-71"
        RETURN_POS="-57,-71"
        START_POS="-53,-114,60"
        printf "Blue team selected.\n"
    elif [ "${ARGI:0:10}" = "--start-x=" ] ; then
        START_POS_X="${ARGI#--start-x=*}"
    elif [ "${ARGI:0:10}" = "--start-y=" ] ; then
        START_POS_Y="${ARGI#--start-y=*}"
    elif [ "${ARGI:0:10}" = "--start-a=" ] ; then
        START_POS_A="${ARGI#--start-a=*}"
    else
        echo "Undefined argument:" $ARGI
        echo "Please use -h for help."
        exit 1
    fi
done

#-------------------------------------------------------
#  Part 2: Handle Ill-formed command-line arguments
#-------------------------------------------------------

if [ "${HELP}" = "yes" ]; then
    printf "%s [SWITCHES]            \n" $0
    printf "Switches:                \n"
    printf "  --evan, -e      : Evan vehicle only.\n"
    printf "  --felix, -f     : Felix vehicle only.\n"
    printf "  --gus, -g       : Gus vehicle only.\n"
    printf "  --blue, -b      : Blue team.\n"
    printf "  --red, -r       : Red team.\n"
    printf "  --sim, -s       : Simulation mode.\n"
    printf "  --start-x       : Start from x position (requires x y a).\n"
    printf "  --start-y       : Start from y position (requires x y a).\n"
    printf "  --start-a       : Start from angle (requires x y a).\n"
    printf "  --just_build, -j       \n"
    printf "  --help, -h             \n"
    exit 0;
fi

#-------------------------------------------------------
#  Part 3: Create the .moos and .bhv files.
#-------------------------------------------------------

if [[ -n $START_POS_X && (-n $START_POS_Y && -n $START_POS_A)]]; then
  START_POS="$START_POS_X,$START_POS_Y,$START_POS_A"
  echo "Starting from " $START_POS
elif [[ -z $START_POS_X && (-z $START_POS_Y && -z $START_POS_A) ]]; then
  echo "Starting from default postion: " $START_POS
else [[ -z $START_POS_X || (-z $START_POS_Y || -z $START_POS_A) ]]
  echo "When specifing a strating coordinate, all 3 should be specified (x,y,a)."
  echo "See help (-h)."
  exit 1
fi

printf "Assembling MOOS file targ_${VNAME}.moos\n"


nsplug meta_m200.moos targ_${VNAME}.moos -f \
    VNAME=$VNAME                 \
    VPORT=$VPORT                 \
    WARP=$WARP                   \
    SHARE_LISTEN=$SHARE_LISTEN   \
    SHORE_LISTEN=$SHORE_LISTEN   \
    SHORE_IP=$SHORE_IP           \
    M200_IP=$M200_IP             \
    HOSTIP_FORCE="localhost"     \
    LOITER_POS=$LOITER_POS       \
    VARIATION=$VARIATION         \
    VTYPE="kayak"                \
    VTEAM=$VTEAM                 \
    START_POS=$START_POS         \
    $SIM

printf "Assembling BHV file targ_${VNAME}.bhv\n"
nsplug meta_m200.bhv targ_${VNAME}.bhv -f  \
        RETURN_POS=${RETURN_POS}    \
        TRAIL_RANGE=$TRAIL_RANGE    \
        TRAIL_ANGLE=$TRAIL_ANGLE    \
        VTEAM=$VTEAM                \
        VNAME=$VNAME                \
        GRAB_POS=$GRAB_POS          \
        UNTAG_POS=$UNTAG_POS


if [ ${JUST_BUILD} = "yes" ] ; then
    printf "Files assembled; vehicle not launched; exiting per request.\n"
    exit 0
fi

#-------------------------------------------------------
#  Part 4: Launch the processes
#-------------------------------------------------------

printf "Launching $VNAME MOOS Community \n"
pAntler targ_${VNAME}.moos >& /dev/null &
uMAC targ_${VNAME}.moos

printf "Killing all processes ... \n "
kill -- -$$
printf "Done killing processes.   \n "
