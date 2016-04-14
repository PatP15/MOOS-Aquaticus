#!/bin/bash

SHORE_IP=192.168.1.155
# SHORE_IP=multicast_7

WARP=1
HELP="no"
JUST_BUILD="no"
BAD_ARGS=""
MOOS_FILE=""

LOITER_POSA="x=50,y=-125"
DOT_MOOS="meta_mokai.moos"

printf "Initiate launch vehicle script\n"

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
        DOT_MOOS="meta_mokai.moos"
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

printf "Assembling MOOS file ${MOOS_FILE}\n"

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

printf "Launching $VNAME MOOS Community \n"
pAntler $MOOS_FILE >& /dev/null &
uMAC $MOOS_FILE

printf "Killing all processes ... \n "
kill -- -$$
printf "Done killing processes.   \n "
