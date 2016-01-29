#!/bin/sh

#Macro will be running via script
export SCRIPT=yes

#Build directory of CBMROOT
export MY_BUILD_DIR=/Users/slebedev/Development/cbm/trunk/build/

export RESULT_DIR=results/

# Geometries
export CAVE_GEOM=cave.geo
export PIPE_GEOM=pipe/pipe_v14n.root
export STS_GEOM=sts/sts_v15c.geo.root
export RICH_GEOM=rich/Rich_jan2016_3e.gdml
export FIELD_MAP=field_v12b
export MAGNET_GEOM=magnet/magnet_v15a.geo.root
export FIELD_MAP_SCALE=1.0

export STS_MATERIAL_BUDGET_FILE_NAME=/sts/sts_matbudget_v15c.root

#number of events for each thread
export NEVENTS=500

for Z in 0 1 2 3; do
    export DIR=/Users/slebedev/Development/cbm/data/simulations/rich/urqmdtest/

    export RESULT_DIR=results_urqmdtest_newrich_${Z}/
    mkdir -p ${RESULT_DIR}

    export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
    export MC_FILE=${DIR}/mc.newrich.${Z}.root
    export PAR_FILE=${DIR}/param.newrich.${Z}.root
    export RECO_FILE=${DIR}/reco.newrich.${Z}.root

    if [ ${Z} = "0" ] ; then
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
        export PIPE_GEOM=pipe/pipe_v14n.root
        export RICH_GEOM=rich/Rich_jan2016_3e.gdml

    elif [ ${Z} = "1" ]; then
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
        export PIPE_GEOM=pipe/pipe_v14n.root
        export RICH_GEOM=rich/Rich_jan2016_3e_nobpmt.gdml

    elif [ ${Z} = "2" ]; then
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
        export PIPE_GEOM=pipe/pipe_v14l.root
        export RICH_GEOM=rich/Rich_jan2016_1e.gdml

    elif [ ${Z} = "3" ]; then
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
        export PIPE_GEOM=pipe/pipe_v14l.root
        export RICH_GEOM=rich/Rich_jan2016_1e_nobpmt.gdml
    fi
# ./sim.sh
xterm -hold -e ". ./sim.sh"&
#  bsub -q batch -J mc.$Z.$XXXX.run -o $DIR/log/$XXXX.log -N sh ./sim.sh

done
export SCRIPT=no
