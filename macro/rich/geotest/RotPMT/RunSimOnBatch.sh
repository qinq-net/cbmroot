#!/bin/sh

rotx=$1
roty=$2
rotmir=$3
nEvs=$4

GeoCase=$5
PtNotP=$6
MomMin=$7
MomMax=$8


echo "rotx = $rotx, roty = $roty"

cbmroot_config_path=/hera/cbm/users/tariq/cbmroot/buildcbm/config.sh
macro_dir=/hera/cbm/users/tariq/cbmroot/macro/rich/geotest/RotPMT/
# Needed to run macro via script
export SCRIPT=yes
export N_EVS=$nEvs
export ROTMIR=$rotmir
export PMT_ROTX=$rotx
export PMT_ROTY=$roty
export GEO_CASE=$GeoCase
export PT_NOT_P=$PtNotP
export MOM_MIN=$MomMin
export MOM_MAX=$MomMax
# setup the run environment
source ${cbmroot_config_path}
 
# This line is needed, otherwise root will crash
export DISPLAY=localhost:0.0

#create needed directories
mkdir -p ${outdir}/log

# create special and enter special workdir for job
username=$(whoami)
workdir=/tmp/$username/$JOB_ID.$SGE_TASK_ID
mkdir -p $workdir
cd $workdir

#root -b -l -q "Run_Sim_GeoOpt_Batch.C()"
#root -b -l -q "Run_Reco_GeoOpt_Batch.C()"

root -b -l -q "${macro_dir}/Run_Sim_GeoOpt_Batch.C()"
root -b -l -q "${macro_dir}/Run_Rec_GeoOpt_Batch.C()"

cp -v ${SGE_STDOUT_PATH} ${outdir}/log/${JOB_ID}.${SGE_TASK_ID}.log

export SCRIPT=no
