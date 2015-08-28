=#!/bin/sh

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

for Z in 1 2 3 4 5; do

   export DIR=/Users/slebedev/Development/cbm/data/simulations/rich/richreco/
   export LIT_RESULT_DIR=recqa_richreco_000${Z}/
   mkdir -p ${LIT_RESULT_DIR}
   
   export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
   export MC_FILE=${DIR}/25gev.mc.000${Z}.root
   export PAR_FILE=$DIR/25gev.param.000${Z}.root
   export RECO_FILE=$DIR/25gev.reco.000${Z}.root
   
   if [ ${Z} = "1" ] ; then
   		export RICH_GEOM=rich/rich_v08a.geo
	elif [ ${Z} = "2" ]; then
  		export RICH_GEOM=rich/rich_v14a.root
	elif [ ${Z} = "3" ]; then
   		export RICH_GEOM=rich/geotest/rich_geo_RotMir_m10_RotPMT_Xpos20point0_Ypos19point0.root
   	elif [ ${Z} = "4" ]; then
   		export RICH_GEOM=rich/geotest/rich_geo_RotMir_m10_RotPMT_Xpos35point0_Ypos17point0.root
   	elif [ ${Z} = "5" ]; then
   		export RICH_GEOM=rich/geotest/rich_geo_RotMir_p1_RotPMT_Xpos5point0_Ypos5point0.root
	fi

  #. ./sim.sh
  
   xterm -hold -e ". ./sim.sh"&
   # bsub -q batch -J mc.$Z.$XXXX.run -o $DIR/log/$XXXX.log -N sh ./sim.sh
   
done
export SCRIPT=no
