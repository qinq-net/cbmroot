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
export URQMD=no

# If "yes" than primary electrons will be generated
export ELECTRONS=yes

# If "yes" PLUTO particles will be embedded
export PLUTO=no

# Geometries
export GEO_SETUP_FILE=geosetup_25gev.C

#number of events for each thread
export NEVENTS=5000

for Z in 0 1; do

   export DIR=/Users/slebedev/Development/cbm/data/simulations/rich/newrich/
   export LIT_RESULT_DIR=recqa_bfield_${Z}/
   mkdir -p ${LIT_RESULT_DIR}
   
   export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root
   export MC_FILE=${DIR}/mc.${Z}.root
   export PAR_FILE=$DIR/param.${Z}.root
   export RECO_FILE=$DIR/reco.${Z}.root

    if [ ${Z} = "0" ] ; then
        export GEO_SETUP_FILE=geosetup_25gev.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root

    elif [ ${Z} = "1" ]; then
        export GEO_SETUP_FILE=geosetup_25gev_newfield.C
        export IN_FILE=/Users/slebedev/Development/cbm/data/urqmd/auau/25gev/centr/urqmd.auau.25gev.centr.00001.root

    fi
 
  #. ./sim.sh
  
   xterm -hold -e ". ./sim.sh"&
   # bsub -q batch -J mc.$Z.$XXXX.run -o $DIR/log/$XXXX.log -N sh ./sim.sh
   
done
export SCRIPT=no
