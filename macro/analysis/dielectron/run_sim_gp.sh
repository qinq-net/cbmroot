#!/bin/sh

JOBS=800
DIR1=/lustre/nyx/cbm/users/gpitsch/CbmRoot/results/results_dielectron/script_output/


sbatch -J dielectrons -D ${DIR1} --time=6:00:00 --array=1-$JOBS sim_gp.sh
        


