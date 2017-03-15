#!/bin/bash

# param1=1
# param2='test'
# sbatch --array=1-3 -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position ./test.sh $param1
# sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned -o %j_%N.out.log -e %j_%N.err.log --time=0:30:00 --array=1-3 ./test1.sh

# for FLAG in 0 1
# do
# WORK_DIR=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned
FLAG=0		# (== 0) for Reference ; (== 1) for misaligned w/o corrections ; (== 2) for misaligned w/ corrections ; (== 3) to calculate correction parameters
NEVTS=1250	# nb of events to run
COLL_ENERGY=8gev
##	sbatch -J matching -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Matching/Reference --time=8:00:00 --array=1-800 --partition=long ./matching.sh $FLAG $NEVTS $COLL_ENERGY
##	sbatch -J matching -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Matching/WO_Corrections --time=2:00:00 --array=1-200 ./matching.sh $FLAG $NEVTS $COLL_ENERGY
##	sbatch -J matching -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Matching/W_Corrections --time=2:00:00 --array=1-200 ./matching.sh $FLAG $NEVTS $COLL_ENERGY
##	sbatch -J matching -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Matching/Compute_Corrections --time=2:00:00 --array=1-2 ./matching.sh $FLAG $NEVTS $COLL_ENERGY

	sbatch -J test_match -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/test_matching --time=0:20:00 --array=1-800 ./matching.sh $FLAG $NEVTS $COLL_ENERGY

# done
