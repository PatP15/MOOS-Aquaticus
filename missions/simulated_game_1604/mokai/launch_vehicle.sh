#!/bin/bash

SHORE_IP=192.168.1.155
# SHORE_IP=multicast_7
SHORE_LISTEN="9300"

WARP=1
HELP="no"
JUST_BUILD="no"
BAD_ARGS=""
VTEAM="red"
START_POS="0,0,0"
VNAME="mokai"
VPORT="9013"
SHARE_LISTEN="9313"
BUTTON="5"
JOY_ID="0"

printf "Initiate launch MOKAI script.\n"

for ARGI; do
    UNDEFINED_ARG=$ARGI
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP="yes"
        UNDEFINED_ARG=""
    fi
    if [ "${ARGI}" = "--red" -o "${ARGI}" = "-r" ] ; then
        VTEAM="red"
        UNDEFINED_ARG=""
        START_POS="56,16,240"
        VPORT="9013"
        SHARE_LISTEN="9313"
        printf "Red team selected.\n"
    fi
    if [ "${ARGI}" = "--blue" -o "${ARGI}" = "-b" ] ; then
        VTEAM="blue"
        UNDEFINED_ARG=""
        START_POS="-53,-114,60"
        VPORT="9014"
        SHARE_LISTEN="9314"
        printf "Blue team selected.\n"
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

if [ "${BAD_ARGS}" != "" ] ; then
    printf "Bad Argument: %s \n" $BAD_ARGS
    exit 0
fi

if [ "${HELP}" = "yes" ]; then
    printf "%s [SWITCHES]            \n" $0
    printf "Switches:                \n"
    printf "  --just_build, -j       \n"
    printf "  --help, -h             \n"
    exit 0;
fi

printf "Assembling MOOS file targ_${VNAME}.moos .\n"

nsplug meta_mokai.moos targ_${VNAME}.moos -f  \
       VNAME="${VNAME}_${VTEAM}"                 \
       VPORT=$VPORT                 \
       SHARE_LISTEN=$SHARE_LISTEN   \
       WARP=$WARP                   \
       SHORE_LISTEN=$SHORE_LISTEN   \
       SHORE_IP=$SHORE_IP           \
       VTYPE="mokai"                \
       VTEAM=$VTEAM                 \
       BUTTON=$BUTTON               \
       JOY_ID=$JOY_ID               \
       START_POS=$START_POS

if [ ${JUST_BUILD} = "yes" ] ; then
    printf "Files assembled; vehicle not launched; exiting per request.\n"
    exit 0
fi

if [ ! -e targ_${VNAME}.moos ]; then echo "no targ_${VNAME}.moos!"; exit 1; fi

printf "Launching $VNAME MOOS Community.\n"
pAntler targ_${VNAME}.moos >& /dev/null &
uMAC targ_${VNAME}.moos

printf "Killing all processes ... \n "
kill -- -$$
printf "Done killing processes.   \n "
