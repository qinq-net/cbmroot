#!/bin/sh

#Macro will be running via script
export SCRIPT=yes

#Build directory of CBMROOT
export MY_BUILD_DIR=/Users/slebedev/Development/cbm/trunk/build/

export RESULT_DIR=results/

# Geometry setup file name gSystem->Getenv("VMCWORKDIR") + GEO_SETUP_FILE
export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis18_mcbm_20deg_long.C

#number of events for each thread
export NEVENTS=10

for Z in 0 1 2 3 4 5 6 7 8 9; do
	export DIR=/Users/slebedev/Development/cbm/data/sim/rich/mcbm/
	
	export IN_FILE=/Users/slebedev/Development/cbm/trunk/cbmroot/input/urqmd.agag.1.65gev.centr.0000${Z}.root
    export MC_FILE=${DIR}/mc.${Z}.root
    export PAR_FILE=${DIR}/param.${Z}.root
    export RECO_FILE=${DIR}/reco.${Z}.root
	export GEOSIM_FILE=${DIR}/geosim.${Z}.root
	export RESULT_DIR=results_mcbm_qa/${Z}/
    mkdir -p ${RESULT_DIR}

	xterm -hold -e ". ./sim_mcbm.sh"&

done
export SCRIPT=no
