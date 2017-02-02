#!/bin/bash

# param1=1
# param2='test'
# sbatch --array=1-3 -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position ./test.sh $param1
# sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned -o %j_%N.out.log -e %j_%N.err.log --time=0:30:00 --array=1-3 ./test1.sh

# for FLAG in 0 1
# do
# WORK_DIR=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned
FLAG=0		# (= 0) for Reference ; (= 1) for w/o corrections ; (= 2) for w/ corrections
NEVTS=5000	# nb of events to run
COLL_ENERGY=8gev
	sbatch -J matching -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Reference -o %j_%N.out.log -e %j_%N.err.log --time=2:00:00 --array=1-10 ./position.sh $FLAG $NEVTS $COLL_ENERGY
##	sbatch -J matching -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/WO_Corrections -o %j_%N.out.log -e %j_%N.err.log --time=2:00:00 --array=1-10 ./position.sh $FLAG $NEVTS $COLL_ENERGY
##	sbatch -J matching -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/W_Corrections -o %j_%N.out.log -e %j_%N.err.log --time=2:00:00 --array=1-10 ./position.sh $FLAG $NEVTS $COLL_ENERGY
# done
