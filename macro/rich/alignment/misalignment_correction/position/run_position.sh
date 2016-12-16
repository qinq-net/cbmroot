#!/bin/bash

cp -i ./run_sim_position2.C /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/macros/.
# cp -i ./run_reco_position.C /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/macros/.

# param1=1
# param2='test'
# sbatch --array=1-3 -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position ./test.sh $param1
# sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned -o %j_%N.out.log -e %j_%N.err.log --time=0:30:00 --array=1-3 ./test1.sh

# for FLAG in 0 1
# do
# WORK_DIR=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned
FLAG=1
NEVTS=10	# nb of events to run
COLL_ENERGY=8gev
	sbatch -J test_position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned -o %j_%N.out.log -e %j_%N.err.log --time=0:30:00 --array=1-2 ./position.sh $FLAG $NEVTS $COLL_ENERGY
# done
