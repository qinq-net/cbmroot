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

for Z in 1700mm 1710mm 1720mm 1730mm 1740mm 1750mm 1760mm 1770mm 1780mm 1790mm 1800mm 1810mm 1820mm 1830mm 1840mm 1850mm 1860mm 1870mm 1880mm 1890mm; do
#    export DIR=/Users/slebedev/Development/cbm/data/sim/rich/urqmdtest/
	export DIR=/Users/slebedev/Development/cbm/data/sim/rich/geotest/
	
    export MC_FILE=${DIR}/mc.${Z}.root
    export PAR_FILE=${DIR}/param.${Z}.root
    export RECO_FILE=${DIR}/reco.${Z}.root
	export GEOSIM_FILE=${DIR}/geosim.${Z}.root

#    if [ ${Z} = "0" ] ; then
#    	export NEVENTS=1000
#		export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100.C
#        export RESULT_DIR=results_v18a/urqmdtest/
#        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
#		mkdir -p ${RESULT_DIR}
#		
#    elif [ ${Z} = "1" ]; then
#    	export NEVENTS=1000
#    	export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_v17a.C
#        export RESULT_DIR=results_v17a/urqmdtest/
#        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
#        mkdir -p ${RESULT_DIR}
#    fi

#    if [ ${Z} = "0" ] ; then
#    	export NEVENTS=100000
#    	export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100.C
#        export RESULT_DIR=results_v18a/geotest/
#		mkdir -p ${RESULT_DIR}
#		
#    elif [ ${Z} = "1" ]; then
#    	export NEVENTS=100000
#    	export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_v17a.C
#        export RESULT_DIR=results_v17a/geotest/
#        mkdir -p ${RESULT_DIR}
#    fi

	export NEVENTS=10000
	export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_${Z}.C
	export RESULT_DIR=results_geotest_v18a_scan_radius/${Z}/
    mkdir -p ${RESULT_DIR}

xterm -hold -e ". ./sim.sh geotest"&
#xterm -hold -e ". ./sim.sh urqmdtest"&

done
export SCRIPT=no
