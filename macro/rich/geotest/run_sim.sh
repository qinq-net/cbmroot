#!/bin/sh

#Macro will be running via script
export SCRIPT=yes

#Build directory of CBMROOT
export MY_BUILD_DIR=/Users/slebedev/Development/cbm/trunk/build/

export RESULT_DIR=results/

# Geometry setup file name gSystem->Getenv("VMCWORKDIR") + GEO_SETUP_FILE
export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100.C

#number of events for each thread
export NEVENTS=100

for Z in 0; do
    export DIR=/Users/slebedev/Development/cbm/data/sim/rich/urqmdtest/
	#export DIR=/Users/slebedev/Development/cbm/data/sim/rich/geotest/
	
    export MC_FILE=${DIR}/mc.${Z}.root
    export PAR_FILE=${DIR}/param.${Z}.root
    export RECO_FILE=${DIR}/reco.${Z}.root
    export DIGI_FILE=${DIR}/digi.${Z}.root
	export GEOSIM_FILE=${DIR}/geosim.${Z}.root

    if [ ${Z} = "0" ] ; then
    	export NEVENTS=1000
		export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100.C
        export RESULT_DIR=results_v18b_1e/urqmdtest/
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
		mkdir -p ${RESULT_DIR}
		
    elif [ ${Z} = "1" ]; then
    	export NEVENTS=1000
    	export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_v18a_3e.C
        export RESULT_DIR=results_v18a_3e/urqmdtest/
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
        mkdir -p ${RESULT_DIR}
    fi

#    if [ ${Z} = "0" ] ; then
#    	export NEVENTS=100000
#    	export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100.C
#        export RESULT_DIR=results_v18b_1e/geotest/
#		mkdir -p ${RESULT_DIR}
#		
#    elif [ ${Z} = "1" ]; then
#    	export NEVENTS=100000
#    	export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_v18a_3e.C
#        export RESULT_DIR=results_v18a_3e/geotest/
#        mkdir -p ${RESULT_DIR}
#    fi

    mkdir -p ${RESULT_DIR}

#xterm -hold -e ". ./sim.sh geotest"&
xterm -hold -e ". ./sim.sh urqmdtest"&

done
export SCRIPT=no
