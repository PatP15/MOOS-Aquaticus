#!/bin/bash
TIME_WARP=1

CMD_ARGS=""
NO_HERON=""
NO_MOKAI=""
NO_SHORESIDE=""
LOGPATH=""

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP="yes"
    elif [ "${ARGI}" = "--no_shoreside" -o "${ARGI}" = "-ns" ] ; then
        NO_SHORESIDE="true"
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then
        TIME_WARP=$ARGI
        echo "Time warp set up to $TIME_WARP."
    elif [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
        JUST_BUILD="yes"
        echo "Just building files; no vehicle launch."
    elif [ "${ARGI:0:10}" = "--logpath=" ]; then
        LOGPATH="${ARGI#--logpath=*}"
    else
        CMD_ARGS=$CMD_ARGS" "$ARGI
    fi
done


if [ "${HELP}" = "yes" ]; then
  echo "$0 [SWITCHES]"
  echo "  XX                  : Time warp"
  echo "  --no_shoreside, -ns"
  echo "  --just_build, -j"
  echo "  --logpath="
  echo "  --help, -h"
  exit 0;
fi

if [ -n "${LOGPATH}" ]; then
  LOGDIR=--logpath=${LOGPATH}
fi

echo Logging to $LOGDIR

#-------------------------------------------------------
#  Part 2: Launching herons
#-------------------------------------------------------
if [[ -z $NO_HERON ]]; then
  cd ./surveyor
  # Gus Red
  ./launch_surveyor.sh g r1 r2 $TIME_WARP $LOGDIR -s --start-x=140 --start-y=45 --start-a=240 > /dev/null &
  sleep 1
  # Luke Red
  ./launch_surveyor.sh l r2 r1 $TIME_WARP $LOGDIR -s --start-x=140 --start-y=35 --start-a=240 > /dev/null &
  sleep 1
  # Kirk Blue
  ./launch_surveyor.sh k b1 b2 $TIME_WARP $LOGDIR -s --start-x=20 --start-y=45 --start-a=60 > /dev/null &
  sleep 1
  # Jing Blue
  ./launch_surveyor.sh j b2 b1 $TIME_WARP $LOGDIR -s --start-x=20 --start-y=35 --start-a=60 > /dev/null &
  sleep 1
  cd ..
fi



#-------------------------------------------------------
#  Part 3: Launching shoreside
#-------------------------------------------------------
if [[ -z $NO_SHORESIDE ]]; then
  cd ./shoreside
  ./launch_shoreside.sh $TIME_WARP >& /dev/null &
  cd ..
fi

#-------------------------------------------------------
#  Part 4: Launching uMAC
#-------------------------------------------------------
uMAC shoreside/targ_shoreside.moos

#-------------------------------------------------------
#  Part 5: Killing all processes launched from script
#-------------------------------------------------------
echo "Killing Simulation..."
kill -- -$$
# sleep is to give enough time to all processes to die
sleep 3
echo "All processes killed"
