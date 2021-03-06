#!/bin/sh

#Macro will be running via script
export SCRIPT=yes

#Build directory of CBMROOT
export MY_BUILD_DIR=/Users/slebedev/Development/cbm/trunk/build

export TRACKINGTYPE=branch

#Simulation parameters
#--------------------------------------------------
# number of embedded electrons
export NELECTRONS=5

# number of embedded positrons
export NPOSITRONS=5

# If "yes" than UrQMD will be used as background
export URQMD=yes

# If "yes" than primary electrons will be generated
export ELECTRONS=yes

# If "yes" PLUTO particles will be embedded
export PLUTO=no

# Geometries
export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_v18a_1e.C

#number of events for each thread
export NEVENTS=1000

for Z in 6 7 8; do

	export DIR=/Users/slebedev/Development/cbm/data/sim/rich/reco/
	export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
	export MC_FILE=${DIR}/mc.${Z}.root
	export PAR_FILE=$DIR/param.${Z}.root
	export RECO_FILE=$DIR/reco.${Z}.root
	export DIGI_FILE=$DIR/digi.${Z}.root
	export GEOSIM_FILE=$DIR/geosim.${Z}.root

    if [ ${Z} = "0" ] ; then
		export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/10gev/centr/urqmd.auau.10gev.centr.00001.root
        export LIT_RESULT_DIR=results_v17a_10gev/
		mkdir -p ${LIT_RESULT_DIR}
		
	elif [ ${Z} = "1" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_18b.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/10gev/centr/urqmd.auau.10gev.centr.00001.root
		export LIT_RESULT_DIR=results_v18b_10gev/
		mkdir -p ${LIT_RESULT_DIR}
		
	elif [ ${Z} = "2" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/12gev/centr/urqmd.auau.12gev.centr.00001.root
		export LIT_RESULT_DIR=results_v17a_12gev/
		mkdir -p ${LIT_RESULT_DIR}
		
	elif [ ${Z} = "3" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_18b.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/12gev/centr/urqmd.auau.12gev.centr.00001.root
		export LIT_RESULT_DIR=results_v18b_12gev/
		mkdir -p ${LIT_RESULT_DIR}
		
	elif [ ${Z} = "4" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
		export LIT_RESULT_DIR=results_v17a_25gev/
		mkdir -p ${LIT_RESULT_DIR}

	elif [ ${Z} = "5" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_18b.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
		export LIT_RESULT_DIR=results_v18b_25gev/
		mkdir -p ${LIT_RESULT_DIR}
		
	elif [ ${Z} = "6" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_18b_no_sh_box.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
		export LIT_RESULT_DIR=results_v18b_no_sh_box/
		mkdir -p ${LIT_RESULT_DIR}
		
	elif [ ${Z} = "7" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_18b_no_sh_box.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/10gev/centr/urqmd.auau.10gev.centr.00001.root
		export LIT_RESULT_DIR=results_v18b_no_sh_box_10gev/
		mkdir -p ${LIT_RESULT_DIR}
		
	elif [ ${Z} = "8" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_sis100_18b_no_sh_box.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/12gev/centr/urqmd.auau.12gev.centr.00001.root
		export LIT_RESULT_DIR=results_v18b_no_sh_box_12gev/
		mkdir -p ${LIT_RESULT_DIR}
		

    fi
 
  #. ./sim.sh
  
   xterm -hold -e ". ./sim.sh"&
   
done
export SCRIPT=no
