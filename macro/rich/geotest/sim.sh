#!/bin/sh
cd $MY_BUILD_DIR
. ./config.sh
cd -

if [ ${1} = "geotest" ] ; then
	root -l -q "./run_sim_geotest.C($NEVENTS)"
	root  -l -b "./run_reco_geotest.C($NEVENTS)"
	
elif [ ${1} = "urqmdtest" ]; then
	root -l -q "./run_sim_urqmdtest.C($NEVENTS)"
	root  -l -b "./run_reco_urqmdtest.C($NEVENTS)"
fi
    

