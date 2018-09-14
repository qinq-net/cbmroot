#!/bin/sh
cd $MY_BUILD_DIR
. ./config.sh
cd -

root -b -q "./run_sim.C($NEVENTS)"
root -b -l "./run_digi.C($NEVENTS)"
root -b -l "./run_reco.C($NEVENTS)"

