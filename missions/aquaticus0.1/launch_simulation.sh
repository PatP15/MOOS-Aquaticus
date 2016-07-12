#!/bin/bash

TIME_WARP=1
JUST_MAKE=""
#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	printf "%s [WARP]             \n" $0
	exit 0;
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then
        TIME_WARP=$ARGI
    elif [ "${ARGI:0:7}" = "--warp=" ] ; then
	TIME_WARP="${ARGI#--warp=*}"
    elif [ "${ARGI}" = "--just_make" -o "${ARGI}" = "-j" ] ; then
        JUST_MAKE="-j"
    else
	printf "Bad Argument: %s \n" $ARGI ". Use --help"
	exit 0
    fi
done


#-----------------------------------------------
# Launch Evan and check for results
#-----------------------------------------------
./launch_m200.sh --vname=evan                  \
		 --vteam=blue                  \
		 --startpos=-38,-68,70         \
		 --sim $TIME_WARP              \
		 $JUST_MAKE 

if [ $? -ne 0 ]; then
    echo launch of evan failed. Exiting.
    exit 1
fi

#-----------------------------------------------
# Launch Felix and check for results
#-----------------------------------------------
./launch_m200.sh --vname=felix                 \
		 --vteam=red                   \
		 --startpos=35,-32,230         \
		 --sim $TIME_WARP              \
		 $JUST_MAKE 

if [ $? -ne 0 ]; then
    echo launch of felix failed. Exiting.
    exit 1
fi


#-----------------------------------------------
# Launch the Shoreside
#-----------------------------------------------
./launch_shoreside.sh $TIME_WARP $JUST_MAKE
