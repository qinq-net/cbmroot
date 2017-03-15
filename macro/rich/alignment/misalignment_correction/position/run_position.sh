#!/bin/bash

# param1=1
# param2='test'
# sbatch --array=1-3 -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position ./test.sh $param1
# sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned -o %j_%N.out.log -e %j_%N.err.log --time=0:30:00 --array=1-3 ./test1.sh

# for FLAG in 0 1
# do
# WORK_DIR=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned
FLAG=1		# (== 0) for aligned case ; (== 1) for misaligned 5 mrad case ; (== 2) for misaligned 1 mrad case ; (== 3) for standard v16a_1e geo
NEVTS=5000	# nb of events to run
COLL_ENERGY=8gev
##	sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned --time=2:00:00 --array=1-200 ./position.sh $FLAG $NEVTS $COLL_ENERGY
##	sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned_5mrad_Full --time=2:00:00 --array=1-200 ./position.sh $FLAG $NEVTS $COLL_ENERGY
##	sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned_1mrad_Full --time=2:00:00 --array=1-200 ./position.sh $FLAG $NEVTS $COLL_ENERGY
##	sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Standard --time=2:00:00 --array=1-200 ./position.sh $FLAG $NEVTS $COLL_ENERGY
# done

##	sbatch -J test_posi -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned -o %j_%N.out.log -e %j_%N.err.log --time=2:00:00 --array=1-200 ./position.sh $FLAG $NEVTS $COLL_ENERGY
	sbatch -J test_posi -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/test_position --time=2:00:00 --array=1-200 ./position.sh $FLAG $NEVTS $COLL_ENERGY
