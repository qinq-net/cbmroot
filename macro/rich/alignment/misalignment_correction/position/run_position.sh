#!/bin/bash

sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Aligned -o %j_%N.out.log -e %j_%N.err.log --time=0:30:00 --array=1-3 ./position.sh
# sbatch -J position -D /lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position/Misaligned -o %j_%N.out.log -e %j_%N.err.log --time=0:30:00 --array=1-3 ./test1.sh

# sbatch --array=1-3 ./test.sh
