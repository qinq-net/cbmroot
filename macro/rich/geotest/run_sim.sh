#!/bin/sh

#Macro will be running via script
export SCRIPT=yes

#Build directory of CBMROOT
export MY_BUILD_DIR=/Users/slebedev/Development/cbm/trunk/build/

export RESULT_DIR=results/

# Geometries
export CAVE_GEOM=cave.geo
export PIPE_GEOM=pipe/pipe_v16b_1e.geo.root
export STS_GEOM=sts/sts_v16x.geo.root
export RICH_GEOM=rich/rich_v16a_1e.geo.root
export FIELD_MAP=field_v16a
export MAGNET_GEOM=magnet/magnet_v15a.geo.root
export FIELD_MAP_SCALE=1.0

export STS_MATERIAL_BUDGET_FILE_NAME=/sts/sts_matbudget_v16x.root

#number of events for each thread
export NEVENTS=1000

for Z in 0 1; do
#    export DIR=/Users/slebedev/Development/cbm/data/sim/rich/urqmdtest/
#    export MC_FILE=${DIR}/mc.${Z}.root
#    export PAR_FILE=${DIR}/param.${Z}.root
#    export RECO_FILE=${DIR}/reco.${Z}.root
#
#    if [ ${Z} = "0" ] ; then
#    	export NEVENTS=1000
#        export RICH_GEOM=rich/test.root
#        export RESULT_DIR=results_urqmdtest_cyl_8gev_full/
#        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
#		mkdir -p ${RESULT_DIR}
#		
#    elif [ ${Z} = "1" ]; then
#    	export NEVENTS=1000
#        export RICH_GEOM=rich/test_nobpmt.root
#        export RESULT_DIR=results_urqmdtest_cyl_8gev_nobpmt/
#        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
#        mkdir -p ${RESULT_DIR}
#    
#    elif  [ ${Z} = "2" ] ; then
#    	export NEVENTS=250
#        export RICH_GEOM=rich/test.root
#        export RESULT_DIR=results_urqmdtest_cyl_25gev_full/
#        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
#		mkdir -p ${RESULT_DIR}
#		
#	elif [ ${Z} = "3" ]; then
#		export NEVENTS=1000
#        export RICH_GEOM=rich/test_nobpmt.root
#        export RESULT_DIR=results_urqmdtest_cyl_25gev_nobpmt/
#        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
#        mkdir -p ${RESULT_DIR}
#    fi

    export DIR=/Users/slebedev/Development/cbm/data/sim/rich/geotest/
    export MC_FILE=${DIR}/mc.${Z}.root
    export PAR_FILE=${DIR}/param.${Z}.root
    export RECO_FILE=${DIR}/reco.${Z}.root

    if [ ${Z} = "0" ] ; then
    	export NEVENTS=50000
        export RICH_GEOM=rich/test.root
        export RESULT_DIR=results_geotest_cyl/
		mkdir -p ${RESULT_DIR}
		
    elif [ ${Z} = "1" ]; then
    	export NEVENTS=50000
        export RICH_GEOM=rich/test_nobpmt.root
        export RESULT_DIR=results_geotest_cyl_nobpmt/
        mkdir -p ${RESULT_DIR}
    fi

# ./sim.sh
xterm -hold -e ". ./sim.sh"&
#  bsub -q batch -J mc.$Z.$XXXX.run -o $DIR/log/$XXXX.log -N sh ./sim.sh

done
export SCRIPT=no
