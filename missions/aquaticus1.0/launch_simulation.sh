#!/bin/bash
TIME_WARP=4

CMD_ARGS=""

#-------------------------------------------------------
#  Part 1: Check for and handle command-line arguments
#-------------------------------------------------------
for ARGI; do
    if [ "${ARGI}" = "--help" -o "${ARGI}" = "-h" ] ; then
        HELP="yes"
    elif [ "${ARGI//[^0-9]/}" = "$ARGI" -a "$TIME_WARP" = 1 ]; then
        TIME_WARP=$ARGI
        echo "Time warp set up to $TIME_WARP."
    elif [ "${ARGI}" = "--just_build" -o "${ARGI}" = "-j" ] ; then
        JUST_BUILD="yes"
        echo "Just building files; no vehicle launch."
    else
        CMD_ARGS=$CMD_ARGS" "$ARGI
    fi
done


if [ "${HELP}" = "yes" ]; then
  echo "$0 [SWITCHES]"
  echo "  XX                : Time warp"
  echo "  --just_build, -j       "
  echo "  --help, -h             "
  exit 0;
fi

#-------------------------------------------------------
#  Part 2: Launching M200s
#-------------------------------------------------------
cd ./m200
# Evan Blue
./launch_m200.sh $TIME_WARP -e -b -s > /dev/null &
# Felix Red
./launch_m200.sh $TIME_WARP -f -r -s > /dev/null &
cd ..

#-------------------------------------------------------
#  Part 3: Launching MOKAIs
#-------------------------------------------------------
cd ./mokai
# Evan Blue
./launch_mokai.sh $TIME_WARP -e -b -ss >& /dev/null &
# Felix Red
./launch_mokai.sh $TIME_WARP -f -r -ss >& /dev/null &
cd ..

#-------------------------------------------------------
#  Part 4: Launching shoreside
#-------------------------------------------------------
cd ./shoreside
./launch_shoreside.sh $TIME_WARP >& /dev/null &
cd ..

#-------------------------------------------------------
#  Part 4: Launching uMAC
#-------------------------------------------------------
uMAC shoreside/targ_shoreside.moos

#-------------------------------------------------------
#  Part 5: Killing all processes launched from script
#-------------------------------------------------------
echo "Killing Simulation..."
kill -- -$$
sleep 1
echo "All processes killed"
