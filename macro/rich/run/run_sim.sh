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
export GEO_SETUP_FILE=geosetup_25gev.C

#number of events for each thread
export NEVENTS=1000

for Z in 1; do

export DIR=/Users/slebedev/Development/cbm/data/sim/rich/reco/
 #export LIT_RESULT_DIR=recqa_cyl_${Z}/
 #  mkdir -p ${LIT_RESULT_DIR}
   
   export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
export MC_FILE=${DIR}/mc.v16a.${Z}.root
export PAR_FILE=$DIR/param.v16a.${Z}.root
export RECO_FILE=$DIR/reco.v16a.${Z}.root

    if [ ${Z} = "0" ] ; then
        export GEO_SETUP_FILE=geosetup_8gev.C
        export NEVENTS=20000
        export NELECTRONS=1
		export NPOSITRONS=1
		export URQMD=no
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
        export LIT_RESULT_DIR=results_v16a/recqa_electrons_v16a_1e_rich_TrackRing/
		mkdir -p ${LIT_RESULT_DIR}

    elif [ ${Z} = "1" ]; then
    	export URQMD=yes
    	export NEVENTS=1000
    	export NELECTRONS=5
		export NPOSITRONS=5
        export GEO_SETUP_FILE=geosetup_8gev.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/8gev/centr/urqmd.auau.8gev.centr.00001.root
		export LIT_RESULT_DIR=results_v16a/recqa_8gev_v16a_1e_rich_TrackRing/
		mkdir -p ${LIT_RESULT_DIR}
		
	elif [ ${Z} = "2" ]; then
		export URQMD=yes
		export NEVENTS=1000
		export NELECTRONS=5
		export NPOSITRONS=5
        export GEO_SETUP_FILE=geosetup_25gev.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
		export LIT_RESULT_DIR=results_v16a/recqa_25gev_v16a_3e_rich_TrackRing/
		mkdir -p ${LIT_RESULT_DIR}
    fi
 
  #. ./sim.sh
  
   xterm -hold -e ". ./sim.sh"&
   # bsub -q batch -J mc.$Z.$XXXX.run -o $DIR/log/$XXXX.log -N sh ./sim.sh
   
done
export SCRIPT=no
