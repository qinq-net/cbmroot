#!/bin/bash

# param1=1
# param2='test'
# sbatch --array=1-3 -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position ./test.sh $param1
# sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned -o %j_%N.out.log -e %j_%N.err.log --time=0:30:00 --array=1-3 ./test1.sh

# for FLAG in 0 1
# do
# WORK_DIR=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned
FLAG=1
NEVTS=1000	# nb of events to run
COLL_ENERGY=8gev
##	sbatch -J test_position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned -o %j_%N.out.log -e %j_%N.err.log --time=12:00:00 --array=1-100 ./position.sh $FLAG $NEVTS $COLL_ENERGY
	sbatch -J test_position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned -o %j_%N.out.log -e %j_%N.err.log --time=12:00:00 --array=1-100 ./position.sh $FLAG $NEVTS $COLL_ENERGY
# done
