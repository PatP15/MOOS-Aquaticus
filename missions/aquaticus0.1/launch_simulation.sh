#!/bin/bash

TIME_WARP=1
JUST_MAKE=""
JUST_TWO="no"
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
    elif [ "${ARGI}" = "-jt" ] ; then
        JUST_TWO="yes"
    else
	printf "Bad Argument: %s \n" $ARGI ". Use --help"
	exit 0
    fi
done


#-----------------------------------------------
# Launch Evan and check for results      BLUE #1
#-----------------------------------------------
./launch_m200.sh --vname=evan   --startpos=-25,-25,70        \
                 --vteam=blue   --sim $TIME_WARP $JUST_MAKE 
#./launch_m200.sh --vname=evan   --startpos=-40,-55,70        \

if [ $? -ne 0 ]; then echo Launch of evan failed; exit 1;  fi

#-----------------------------------------------
# Launch Gus and check for results        RED #1
#-----------------------------------------------
./launch_m200.sh --vname=gus   --startpos=40,-45,230         \
		 --vteam=red   --sim $TIME_WARP $JUST_MAKE 

if [ $? -ne 0 ]; then echo launch of Gus failed; exit 1; fi




if [ ${JUST_TWO} = "no" ] ; then
    #-----------------------------------------------
    # Launch Felix and check for results     BLUE #2
    #-----------------------------------------------
    ./launch_m200.sh --vname=felix --startpos=-35,-75,70         \
		     --vteam=blue  --sim $TIME_WARP $JUST_MAKE 
    if [ $? -ne 0 ]; then echo launch of Felix failed; exit 1; fi
    
    #-----------------------------------------------
    # Launch Hal and check for results        RED #2
    #-----------------------------------------------
    ./launch_m200.sh --vname=hal   --startpos=25,-15,230         \
		     --vteam=red   --sim $TIME_WARP $JUST_MAKE     
    if [ $? -ne 0 ]; then echo launch of Hal failed; exit 1; fi    
fi

#-----------------------------------------------
# Launch the Shoreside
#-----------------------------------------------
./launch_shoreside.sh $TIME_WARP $JUST_MAKE

