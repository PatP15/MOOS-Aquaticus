#!/bin/bash -e
#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
TIME_WARP=1
JUST_MAKE="no"
AMT=1
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
	printf "%s [SWITCHES] [time_warp]   \n" $0
	printf "  --just_make, -j    \n" 
	printf "  --help, -h         \n" 
	exit 0;
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then 
        TIME_WARP=$ARGI
    elif [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
	JUST_MAKE="yes"
    elif [ "${ARGI:0:6}" = "--amt=" ] ; then
        AMT="${ARGI#--amt=*}"
    else 
	printf "Bad Argument: %s \n" $ARGI
	exit 0
    fi
done

#-------------------------------------------------------
#  Part 2: Create the .moos and .bhv files. 
#-------------------------------------------------------
VNAME="henry"   
START_POS="0,0"         
LOITER_POS="x=0,y=-75"
SHORE_LISTEN="9300"

VNAMES=( apia baku cary doha elko fahy galt hays iola juba kiev lima mesa 
	 nuuk oslo pace quay rome sako troy ubly vimy waco xian york zahl )

STARTXS=( 0 20 40 60 80 100 120 140 5 25 45 65 85 105 125 145 10 50 90 130
	  15 55 95 135 140 145 150 155 160 165 170 175 180 185 190 195 200 )

# Cap the AMT to be 26 (just our limit of unique names)
if [ $AMT -ge 1 ] ; then
    AMT=`expr $AMT - 1`
fi

nsplug meta_vehicle.moos targ_henry.moos -f WARP=$TIME_WARP \
    VNAME=$VNAME           SHARE_LISTEN="9301"              \
    VPORT="9001"           SHORE_LISTEN=$SHORE_LISTEN       \
    START_POS=$START_POS 

nsplug meta_shoreside.moos targ_shoreside.moos -f WARP=$TIME_WARP \
    SNAME="shoreside"  SHARE_LISTEN=$SHORE_LISTEN  SPORT="9000"       

nsplug meta_vehicle.bhv targ_henry.bhv -f VNAME=$VNAME     \
    START_POS=$START_POS LOITER_POS=$LOITER_POS       

if [ ! -e targ_henry.moos ]; then echo "no targ_henry.moos"; exit; fi
if [ ! -e targ_henry.bhv  ]; then echo "no targ_henry.bhv "; exit; fi
if [ ! -e targ_shoreside.moos ]; then echo "no targ_shoreside.moos";  exit; fi

for INDEX in `seq 0 $AMT`;
do 
    VNAME=${VNAMES[$INDEX]}
    POSX=${STARTXS[$INDEX]}

    INDEX=`expr $INDEX + 1`
    VPORT=`expr $INDEX + 9400`
    LPORT=`expr $INDEX + 9500`

    START_POS=$POSX",0,180"

    echo "Vehicle:" $VNAME
    echo "Index:" $INDEX  "Port: " $VPORT "POS:" $START_POS 

    nsplug meta_chaser.moos targ_$VNAME.moos -f WARP=$TIME_WARP  \
	   VNAME=$VNAME          SHARE_LISTEN=$LPORT             \
	   VPORT=$VPORT          SHORE_LISTEN=$SHORE_LISTEN      \
	   START_POS=$START_POS  
    
    nsplug meta_chaser.bhv targ_$VNAME.bhv -f VNAME=$VNAME     \
	START_POS=$START_POS  

    if [ ! -e targ_$VNAME.moos ]; then echo "no targ_$VNAME.moos"; exit; fi
    if [ ! -e targ_$VNAME.bhv  ]; then echo "no targ_$VNAME.bhv "; exit; fi

done

if [ ${JUST_MAKE} = "yes" ] ; then
    exit 0
fi


#-------------------------------------------------------
#  Part 3: Launch the processes
#-------------------------------------------------------
printf "Launching $SNAME MOOS Community (WARP=%s) \n"  $TIME_WARP
pAntler targ_shoreside.moos >& /dev/null &
printf "Launching $VNAME1 MOOS Community (WARP=%s) \n" $TIME_WARP
pAntler targ_henry.moos >& /dev/null &
printf "Done \n"

uMAC targ_shoreside.moos

printf "Killing all processes ... \n"
mykill
printf "Done killing processes.   \n"
