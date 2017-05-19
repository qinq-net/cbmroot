#!/bin/bash

cbmroot_config_path=/lustre/nyx/cbm/users/jbendar/CBMINSTALL/bin/CbmRootConfig.sh
source ${cbmroot_config_path}

export SCRIPT=no

nEvs=5000
#outDir=/data/Sim_Outputs/Correction_test/old_code/
outDir=/data/Sim_Outputs/Correction_test/new_code/
#for FLAG in 0 1
#do
	root -b -q -l "run_sim.C($nEvs, $outDir)"
	root -b -q -l "run_reco.C($nEvs, $outDir)"
	root -b -q -l "run_reco_alignment.C($nEvs, $outDir)"
	root -b -q -l "run_reco_correction.C($nEvs, $outDir)"
#done
