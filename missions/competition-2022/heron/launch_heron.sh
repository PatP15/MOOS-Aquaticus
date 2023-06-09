#!/bin/bash
TIME_WARP=1

#SHORE_IP=192.168.1.150
SHORE_IP=localhost
SHORE_LISTEN="9300"

TRAIL_RANGE="3"
TRAIL_ANGLE="330"
HELP="no"
JUST_BUILD="no"
VTEAM=""
VNAME=""
RNAME=""
VMODEL="M300"

START_POS="140,45,240"
RETURN_POS="5,0"
LOITER_POS="x=100,y=-180"
GRAB_POS=""
GRABR_POS=""
GRABL_POS=""
UNTAG_POS=""

HERON_TEAMMATE=""
HERON_TEAMMATE_VTEAM=""

CID=000 #COMP ID
LOGPATH=./

START_ACTION="PROTECT"

function help(){
    echo ""
    echo "USAGE: $0 <heron_vehicle_name> <vehicle_role> <heron_teammate_vehicle_role> [SWITCHES]"
    
    echo ""
    echo "POSSIBLE HERON VEHICLE NAMES:"
    echo "  evan,         e   : Evan heron."
    echo "  felix,        f   : Felix heron."
    echo "  gus,          g   : Gus heron."
    echo "  hal,          h   : Hal heron."
    echo "  ida,          i   : Ida heron."
    echo "  jing,         j   : Jing heron."
    echo "  kirk,         k   : Kirk heron."
    echo "  luke,         l   : Luke heron."

    echo ""
    echo "POSSIBLE ROLES (and heron teammate_roles):"
    echo "  blue_one,     b1  : Vehicle one on blue team."
    echo "  blue_two,     b2  : Vehicle two on blue team."
    echo "  red_one,      r1  : Vehicle one on red team."
    echo "  red_two,      r2  : Vehicle two on red team."

    echo ""
    echo "POSSIBLE SWITCHES:"
    echo "  --role,           : Autonomy startup roles: ATTACK/DEFEND _E/_MED."
    echo "  --sim,        -s  : Simulation mode."
    echo "  --start-x=        : Start from x position (requires x y a)."
    echo "  --start-y=        : Start from y position (requires x y a)."
    echo "  --start-a=        : Start from angle (requires x y a)."
    echo "  --cid=            : Competition ID (for log file)"
    echo "  --logpath=        : Log path"
    echo "  --just_build, -J  : Just build targ files."
    echo "  --help,       -H  : Display this message."
    echo "  #                 : Moos Time Warp."
    exit 0
}

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
case "$1" in
    e|evan)
        HERON_IP=192.168.5.1
        VNAME="EVAN"
        echo "EVAN heron selected."
        ;;
    f|felix)
        HERON_IP=192.168.6.1
        VNAME="FELIX"
        echo "FELIX heron selected."
        ;;
    g|gus)
        HERON_IP=192.168.7.1
        VNAME="GUS"
        echo "GUS heron selected."
        ;;
    h|hal)
        HERON_IP=192.168.8.1
        VNAME="HAL"
        echo "HAL heron selected."
        ;;
    i|ida)
        HERON_IP=192.168.9.1
        VNAME="IDA"
        echo "IDA heron selected."
        ;;
    j|jing)
        HERON_IP=192.168.10.1
        VNAME="JING"
        echo "JING heron selected."
        ;;
    k|kirk)
        HERON_IP=192.168.11.1
        VNAME="KIRK"
        echo "KIRK heron selected."
        ;;
    l|luke)
	    HERON_IP=192.168.12.1
        VNAME="LUKE"
	    echo "LUKE heron selected."
	    ;;
    *)
        echo "!!! Error invalid positional argument $1 !!!"
        ;;
esac

case "$2" in
    r1|red_one)
        VTEAM="red"
        RNAME="red_one"
        VPORT="9011"
	VR_PORT="9811"
        SHARE_LISTEN="9311"
	START_ACTION="DEFEND_E"
        echo "Vehicle set to red one."
        ;;
    r2|red_two)
        VTEAM="red"
        RNAME="red_two"
        VPORT="9012"
	VR_PORT="9812"
        SHARE_LISTEN="9312"
	START_ACTION="ATTACK_MED"
        echo "Vehicle set to red two."
        ;;
    b1|blue_one)
        VTEAM="blue"
        RNAME="blue_one"
        VPORT="9015"
	VR_PORT="9815"
        SHARE_LISTEN="9315"
	START_ACTION="DEFEND_MED"
        echo "Vehicle set to blue one."
        ;;
    b2|blue_two)
        VTEAM="blue"
        RNAME="blue_two"
        VPORT="9016"
	VR_PORT="9816"
        SHARE_LISTEN="9316"
	PLAYERS="b1,b3,b4"
	START_ACTION="ATTACK_E"
        echo "Vehicle set to blue two."
        ;;
    *)
        echo "!!! Error invalid positional argument $2 !!!"
        help
        ;;
esac

	
case "$3" in
    r1|red_one)
        HERON_TEAMMATE="red_one"
        HERON_TEAMMATE_VTEAM="red"
        echo "Vehicle set to red one."
        ;;
    r2|red_two)
        HERON_TEAMMATE="red_two"
        HERON_TEAMMATE_VTEAM="red"
        echo "Vehicle set to red two."
        ;;
    b1|blue_one)
        HERON_TEAMMATE="blue_one"
        HERON_TEAMMATE_VTEAM="blue"
        echo "Vehicle set to blue one."
        ;;
    b2|blue_two)
        HERON_TEAMMATE="blue_two"
        HERON_TEAMMATE_VTEAM="blue"
        echo "Vehicle set to blue two."
        ;;
    *)
        echo "!!! Error invalid positional argument $3 !!!"
        ;;
esac

if [[ "$HERON_TEAMMATE_VTEAM" != "$VTEAM" ]]; then
    echo "!!! Error teammate team can not be different then vehicle team !!!"
    help
fi

for arg in "${@:4}"; do
    if [ "${arg}" = "--help" -o "${arg}" = "-H" ]; then
        help
    elif [ "${arg//[^0-9]/}" = "$arg" -a "$TIME_WARP" = 1 ]; then
        TIME_WARP=$arg
        echo "Time warp set to: " $arg
    elif [ "${arg}" = "--just_build" -o "${arg}" = "-J" ] ; then
        JUST_BUILD="yes"
        echo "Just building files; no vehicle launch."
    elif [ "${arg}" = "--sim" -o "${arg}" = "-s" ] ; then
        SIM="SIM"
        echo "Simulation mode ON."
    elif [ "${arg:0:10}" = "--start-x=" ] ; then
        START_POS_X="${arg#--start-x=*}"
    elif [ "${arg:0:10}" = "--start-y=" ] ; then
        START_POS_Y="${arg#--start-y=*}"
    elif [ "${arg:0:10}" = "--start-a=" ] ; then
        START_POS_A="${arg#--start-a=*}"
    elif [ "${arg:0:7}" = "--role=" ] ; then
        START_ACTION="${arg#--role=*}"
    elif [ "${arg:0:6}" = "--cid=" ] ; then
        CID="${arg#--cid=*}"
        CID=$(printf "%03d" $CID)
    elif [ "${arg:0:10}" = "--logpath=" ]; then
        LOGPATH="${arg#--logpath=*}"
    else
        echo "Undefined switch:" $arg
        help
    fi
done

#HERON_NAME=`get_vname.sh`
#echo "Heron name:"$HERON_NAME
echo $LOGPATH



if [ "${VTEAM}" = "red" ]; then
    GRAB_POS="20,40"
    GRABR_POS="20,20"
    GRABL_POS="30,30"
    UNTAG_POS="140,40"
    RETURN_POS="5,0"
    START_POS="140,45,240"
    STATION_KEEP_AGGRESSIVE="122,38"
    OPFOR="blue"
    OPFOR_ZONE="80,0,160,80"
    echo "Red team selected."
elif [ "${VTEAM}" = "blue" ]; then
    GRAB_POS="140,40"
    GRABR_POS="130,30"
    GRABL_POS="130,50"
    UNTAG_POS="20,40"
    RETURN_POS="5,0"
    START_POS="20,35,60"
    STATION_KEEP_AGGRESSIVE="40,40"
    OPFOR="red"
    OPFOR_ZONE="1,0,80,80"
    echo "Blue team selected."
fi
   
#-------------------------------------------------------
#  Part 2: Create the .moos and .bhv files.
#-------------------------------------------------------

if [[ -n $START_POS_X && (-n $START_POS_Y && -n $START_POS_A)]]; then
  START_POS="$START_POS_X,$START_POS_Y,$START_POS_A"
  echo "Starting from " $START_POS
elif [[ -z $START_POS_X && (-z $START_POS_Y && -z $START_POS_A) ]]; then
  echo "Starting from default postion: " $START_POS
else [[ -z $START_POS_X || (-z $START_POS_Y || -z $START_POS_A) ]]
  echo "When specifying a starting coordinate, all 3 should be specified (x,y,a)."
  echo "See help (-h)."
  exit 1
fi

echo "Assembling MOOS file targ_${RNAME}.moos"


nsplug meta_heron.moos targ_${RNAME}.moos -f \
    VNAME=$VNAME                 \
    RNAME=$RNAME                 \
    VPORT=$VPORT                 \
    VR_PORT=$VR_PORT             \
    WARP=$TIME_WARP              \
    SHARE_LISTEN=$SHARE_LISTEN   \
    SHORE_LISTEN=$SHORE_LISTEN   \
    SHORE_IP=$SHORE_IP           \
    HERON_IP=$HERON_IP           \
    HOSTIP_FORCE="localhost"     \
    LOITER_POS=$LOITER_POS       \
    VARIATION=$VARIATION         \
    VMODEL=$VMODEL               \
    VTYPE="kayak"                \
    VTEAM=$VTEAM                 \
    START_POS=$START_POS         \
    CID=$CID                     \
    LOGPATH=$LOGPATH             \
    UNTAG_POS=$UNTAG_POS         \
    OPFOR=$OPFOR                 \
    OPFOR_ZONE=$OPFOR_ZONE       \
    $SIM                         

echo "Assembling BHV file targ_${RNAME}.bhv"
nsplug meta_heron.bhv targ_${RNAME}.bhv -f  \
    RETURN_POS=${RETURN_POS}       \
    TRAIL_RANGE=$TRAIL_RANGE       \
    TRAIL_ANGLE=$TRAIL_ANGLE       \
    VTEAM=$VTEAM                   \
    VNAME=$VNAME                   \
    RNAME=$RNAME                   \
    GRAB_POS=$GRAB_POS             \
    GRABR_POS=$GRABR_POS           \
    GRABL_POS=$GRABL_POS           \
    UNTAG_POS=$UNTAG_POS           \
    HERON_TEAMMATE=$HERON_TEAMMATE \
    STATION_KEEP_AGGRESSIVE=$STATION_KEEP_AGGRESSIVE \
	START_ACTION=$START_ACTION


if [ ${JUST_BUILD} = "yes" ] ; then
    echo "Files assembled; vehicle not launched; exiting per request."
    exit 0
fi

#-------------------------------------------------------
#  Part 3: Launch the processes
#-------------------------------------------------------

echo "Launching $RNAME MOOS Community "
pAntler targ_${RNAME}.moos >& /dev/null &

uMAC targ_${RNAME}.moos

echo "Killing all processes ..."
kill -- -$$
echo "Done killing processes."
