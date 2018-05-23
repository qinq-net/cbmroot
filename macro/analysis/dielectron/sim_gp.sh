#!/bin/sh
echo Hey


#Macro will be running via script
export SCRIPT=yes


XXXXX=$(printf "%05d" "$SLURM_ARRAY_TASK_ID")
XXXX=$(printf "%04d" "$SLURM_ARRAY_TASK_ID")


#Build directory of CBMROOT
#export MY_BUILD_DIR=/Users/slebedev/Development/cbm/trunk/build/
export MY_BUILD_DIR=/lustre/nyx/cbm/users/gpitsch/CbmRoot/build/
export RESULT_DIR=/lustre/nyx/cbm/users/gpitsch/CbmRoot/results/

# Geometry setup file name gSystem->Getenv("VMCWORKDIR") + GEO_SETUP_FILE
export GEO_SETUP_FILE=/lustre/nyx/cbm/users/gpitsch/CbmRoot/trunk/macro/analysis/dielectron/geosetup/diel_setup_sis100.C

#number of events for each thread
export NEVENTS=1250
 
	#export DIR=/Users/slebedev/Development/cbm/data/sim/rich/mcbm/
        #export DIR=/home/aghoehne/Documents/CbmRoot/Gregor/
        export DIR=/lustre/nyx/cbm/users/gpitsch/CbmRoot/results/results_dielectron
	export IN_FILE=/lustre/nyx/cbm/prod/gen/urqmd/auau/4gev/centr/urqmd.auau.4gev.centr.${XXXXX}.root
    	export PLUTO_FILE=/lustre/nyx/cbm/prod/gen/pluto/auau/cktA/8gev/rho0/epem/pluto.auau.8gev.rho0.epem.${XXXX}.root
        export PLUTO_PARTICLE=rho0
        export NELECTRONS=5
        export NPOSITRONS=5
        export ELECTRONS=yes
        export URQMD=yes
        export PLUTO=yes
        export MC_FILE=${DIR}/mc.${XXXXX}.root
    	export PAR_FILE=${DIR}/param.${XXXXX}.root
    	export RECO_FILE=${DIR}/reco.${XXXXX}.root
	export ANALYSIS_FILE=${DIR}/analysis.${XXXXX}.root
	#export GEOSIM_FILE=${DIR}/geosim.${Z}.root
	export  GEOSIM_FILE=${DIR}/sis18_mcbm_20deg_long_geofile_full.root
	export RESULT_DIR=results_dielectron/${XXXXX}/
#    mkdir -p ${RESULT_DIR}

echo "MY_BUILD_DIR: " $MY_BUILD_DIR
cd $MY_BUILD_DIR
. ./config.sh
cd /lustre/nyx/cbm/users/gpitsch/CbmRoot/trunk/macro/analysis/dielectron


root -l -q -b "./run_sim.C($NEVENTS)"
root -l -q -b "./run_reco.C($NEVENTS)"
root -l -q -b "./run_analysis.C($NEVENTS)"

	
        

export SCRIPT=no




    

