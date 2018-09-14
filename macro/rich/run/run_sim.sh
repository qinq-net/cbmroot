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

for Z in 0; do

	export DIR=/Users/slebedev/Development/cbm/data/sim/rich/reco/
	export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
	export MC_FILE=${DIR}/mc.${Z}.root
	export PAR_FILE=$DIR/param.${Z}.root
	export RECO_FILE=$DIR/reco.${Z}.root
	export DIGI_FILE=$DIR/digi.${Z}.root
	export GEOSIM_FILE=$DIR/geosim.${Z}.root

    if [ ${Z} = "0" ] ; then
		export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_v18a_1e.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
        export LIT_RESULT_DIR=results_v18a_1e/recoqa_range_0_6/
		mkdir -p ${LIT_RESULT_DIR}

    elif [ ${Z} = "1" ]; then
        export GEO_SETUP_FILE=/macro/rich/geosetup/rich_setup_v18a_3e.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
		export LIT_RESULT_DIR=results_v18a_3e/recoqa_range_0_6/
		mkdir -p ${LIT_RESULT_DIR}
    fi
 
  #. ./sim.sh
  
   xterm -hold -e ". ./sim.sh"&
   
done
export SCRIPT=no
