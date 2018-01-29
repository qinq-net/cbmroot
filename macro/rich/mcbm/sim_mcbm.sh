#!/bin/sh
cd $MY_BUILD_DIR
. ./config.sh
cd -

root -l -q "./run_sim_mcbm.C($NEVENTS)"
root  -l -b "./run_reco_mcbm.C($NEVENTS)"

    

