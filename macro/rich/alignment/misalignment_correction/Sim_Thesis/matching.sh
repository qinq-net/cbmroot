#!/bin/bash

XXXXX=$(printf "%05d" "$SLURM_ARRAY_TASK_ID")
#FLAG=1
#NEVTS=10
#COLL_ENERGY=8gev

# Specify input and output directories
if [ $1 -eq 0 ] ; then
	outdir=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Matching/Reference
	geoSetupFile=/lustre/nyx/cbm/users/jbendar/CBMINSTALL/share/cbmroot/macro/rich/geosetup/setup_align.C
elif [ $1 -eq 1 ] ; then
	outdir=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Matching/WO_Corr_3mrad
	geoSetupFile=/lustre/nyx/cbm/users/jbendar/CBMINSTALL/share/cbmroot/macro/rich/geosetup/setup_misalign_gauss_sigma_3.C
fi

cbmroot_config_path=/lustre/nyx/cbm/users/jbendar/CBMINSTALL/bin/CbmRootConfig.sh
#macro_dir=/u/jbendar/cbmroot/CBMSRC/macro/rich/alignment/misalignment_correction/Sim_Thesis/
macro_dir=/lustre/nyx/cbm/users/jbendar/CBMINSTALL/share/cbmroot/macro/rich/Sim_Thesis

echo 'outdir: ' ${outdir}
echo 'FLAG: ' ${1}
echo 'geoSetupFile: ' ${geoSetupFile}

# Needed to run macro via script
export SCRIPT=yes

# Create needed directories
mkdir -p ${outdir}/log

# Create directory for output results
mkdir -p ${outdir}/results/${XXXXX}/

# Setup the run environment
source ${cbmroot_config_path}

# This line is needed, otherwise root will crash
export DISPLAY=localhost:0.0

# Define urqmd and output files
export URQMD_FILE=/lustre/nyx/cbm/prod/gen/urqmd/auau/${3}/centr/urqmd.auau.${3}.centr.${XXXXX}.root
export MC_FILE=${outdir}/mc.root
export PAR_FILE=${outdir}/params.root
export RECO_FILE=${outdir}/reco.root
export ANALYSIS_FILE=${outdir}/analysis.root
export LIT_RESULT_DIR=${outdir}
export GEO_SETUP_FILE=geoSetupFile

#Simulation parameters
#--------------------------------------------------
# number of embedded electrons
export NOF_ELECTRONS=5
# number of embedded positrons
export NOF_POSITRONS=5
# If "yes" then UrQMD will be used as background
export URQMD=yes
# If "yes" PLUTO particles will be embedded
export PLUTO=no
# Collision energy: 25gev or 8gev -> set proper weight into analysis
export ENERGY=${3}

# Geometry setup macro
#export GEO_SETUP_FILE=${setupMacro}

# If "yes" DELTA electrons will be embedded
#export DELTA=no
#export DELTA_FILE=/lustre/cbm/user/ebelolap/aug11/sep12/${ENERGY}/${FIELDDIR}/deltasource/mc.delta.root

# Create special and enter special workdir for job
# username=$(whoami)
# workdir=/tmp/$username/$JOB_ID.$SGE_TASK_ID
# mkdir -p $workdir
# cd $workdir

export OUT_DIR=${outdir}
# Run the root simulation
root -b -l -q "${macro_dir}/run_sim.C(${2}, 1, 0)"
root -b -l -q "${macro_dir}/run_reco.C(${2}, 1, 0)"

# cp -v ${SGE_STDOUT_PATH} ${outdir}/log/${JOB_ID}.${SGE_TASK_ID}.log

export SCRIPT=yes
