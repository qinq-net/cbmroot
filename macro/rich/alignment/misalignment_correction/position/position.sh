#!/bin/bash

XXXXX=$(printf "%05d" "$SLURM_ARRAY_TASK_ID")

cbmroot_config_path=/lustre/nyx/cbm/users/jbendar/CBMINSTALL/bin/CbmRootConfig.sh
# macro_dir=/u/jbendar/CBMSRC/macro/rich/alignment/misalignment_correction/position
output_dir=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position
macro_dir=/lustre/nyx/cbm/users/jbendar/CBMINSTALL/share/cbmroot/macro/rich

# Specify input and output directories
if [ $1 -eq 0 ] ; then
        outdir=${output_dir}/Misaligned
elif [ $1 -eq 1 ] ; then
        outdir=${output_dir}/Aligned
fi

# Needed to run macro via script
export SCRIPT=yes

# Create needed directories
mkdir -p ${outdir}/log

# Create directory for output results
mkdir -p ${outdir}/results/${XXXXX}/

# Setup the run environment
source ${cbmroot_config_path}
echo ${VMCWORKDIR}

# This line is needed, otherwise root will crash
export DISPLAY=localhost:0.0

# Define urqmd and output files
export URQMD_FILE=/lustre/nyx/cbm/prod/gen/urqmd/auau/${3}/centr/urqmd.auau.${3}.centr.${XXXXX}.root
export MC_FILE=${outdir}/mc.${XXXXX}.root
export PAR_FILE=${outdir}/params.${XXXXX}.root
export RECO_FILE=${outdir}/reco.${XXXXX}.root
export ANALYSIS_FILE=${outdir}/analysis.${XXXXX}.root
export LIT_RESULT_DIR=${outdir}/${XXXXX}

#Simulation parameters
#--------------------------------------------------
# set generation of primary electrons
export ELECTRONS=yes
# number of embedded electrons
export NOF_ELECTRONS=1
export NELECTRONS=1
# number of embedded positrons
export NOF_POSITRONS=1
export NPOSITRONS=1
# If "yes" then UrQMD will be used as background
export URQMD=no
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

echo ${output_dir}
echo ${macro_dir}
echo ${outdir}
export OUT_DIR=${outdir}
# Run the root simulation
root -b -l -q "${macro_dir}/position/run_sim_position2.C(${2}, ${1})"
# root -b -l -q "${macro_dir}/position/run_reco_position.C(${2}, ${1})"
# root -b -l -q "${macro_dir}/position/Compute_distance.C(${2}, ${1})"

# cp -v ${SGE_STDOUT_PATH} ${outdir}/log/${JOB_ID}.${SGE_TASK_ID}.log

export SCRIPT=no
