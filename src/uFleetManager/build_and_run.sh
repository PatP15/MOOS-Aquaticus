#!/bin/bash
ARTIFACT_NAME=MOOSFleetManager

echo "Building the MOOS Fleet Manager..."
bash build.sh $ARTIFACT_NAME && ./bin/$ARTIFACT_NAME
exit $?