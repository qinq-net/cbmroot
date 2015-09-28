#!/bin/sh
#$ -wd /tmp 

rotx=$1
roty=$2
rotmir=$3
nEvs=$4
transy=$5
transz=$6
Theta=$7
DefaultDims=$8
DefaultDimsLargePMT=$9

GeoCase=-2
PtNotP=1
MomMin=0
MomMax=4
StartPhi=90
EndPhi=180

# specify input and output directories
echo "rotx = $rotx, roty = $roty"
outdir=/hera/cbm/users/tariq/GeoOptRootFiles/
cbmroot_config_path=/hera/cbm/users/tariq/cbmroot/buildcbm/config.sh
macro_dir=/hera/cbm/users/tariq/cbmroot/macro/rich/geotest/RotPMT
# Needed to run macro via script
export SCRIPT=yes
export N_EVS=$nEvs
export ROTMIR=$rotmir
export PMT_ROTX=$rotx
export PMT_ROTY=$roty
export PMT_TRAY=$transy
export PMT_TRAZ=$transz
export GEO_CASE=$GeoCase
export PT_NOT_P=$PtNotP
export MOM_MIN=$MomMin
export MOM_MAX=$MomMax
export THETA=$Theta
export STARTPHI=$StartPhi
export ENDPHI=$EndPhi

export DEFAULDIMS=$DefaultDims
export DEFAULDIMSLPMT=$DefaultDimsLargePMT

# setup the run environment
source ${cbmroot_config_path}
 
# This line is needed, otherwise root will crash
export DISPLAY=localhost:0.0

#create needed directories
mkdir -p ${outdir}/log

# create special and enter special workdir for job
username=$(whoami)
#workdir=/tmp/$username/$rotmir.$rotx.$roty.$GeoCase.$PtNotP.
workdir=/tmp/$username/$JOB_ID.$SGE_TASK_ID
mkdir -p $workdir
cd $workdir

root -b -l -q "${macro_dir}/Run_Sim_GeoOpt_Batch.C()"
root -b -l -q "${macro_dir}/Run_Reco_GeoOpt_Batch.C()"
root -b -l -q "${macro_dir}/Run_Ana_GeoOpt_Batch.C()"

cp -v ${SGE_STDOUT_PATH} ${outdir}/log/"RX.${rotx}.RY.${roty}.TransY.${transy}.TransZ.${transz}.DeDi.${DefaultDims}.LPMT.${DefaultDimsLargePMT}.${JOB_ID}.${SGE_TASK_ID}.log"
#cp -v ${SGE_STDOUT_PATH} ${outdir}/log/${JOB_ID}.${SGE_TASK_ID}.log

export SCRIPT=no
