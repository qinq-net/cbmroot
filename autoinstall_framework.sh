#!/bin/bash
#
##   semi-automated script installing FairSoft, FairRoot and CbmRoot
#

# 17.12.2015 - split fairsoft directory into src and install
# 01.12.2015 - add selection of root version
# 17.07.2015 - introduce option to compile dev settings 
# 02.06.2015 - introduce parameters for individual package selection
# 13.03.2015 - initial version
# by David Emschermann

#-------------------------------------
# usage:
# svn co https://subversion.gsi.de/cbmsoft/cbmroot/trunk $CBMSRCDIR
# cd $CBMSRCDIR
# ./autoinstall_framework.sh dev 0 0 1
# or
# ./autoinstall_framework.sh 0 0 1
# or
# ./autoinstall_framework.sh 1 1 1
#-------------------------------------

# choose your root verion
#export ROOTVER=6
export ROOTVER=5

# put your desired variants here:
export FSOFTDEV=may16
export FROOTDEV=v-15.11e

export FSOFTPRO=nov15p6
export FROOTPRO=v-15.11e 

# former versions
# fairsoft: mar15p2
# fairroot: v-15.03a

export NUMOFCPU=`cat /proc/cpuinfo | grep processor | wc -l`
export CBMSRCDIR=`pwd`

#-------------------------------------

# set default version to pro
export FSOFTVER=$FSOFTPRO
export FROOTVER=$FROOTPRO

# check if we want to run with GSI compiler
if [ $# -ge 1 ]; then
  if [ "$1" == "gsi" ]; then
    # use a different compiler from GSI
    module use /cvmfs/it.gsi.de/modulefiles/
    module load compiler/gcc/4.9.2
    export CXX=g++
    shift
  fi
fi

# check if we want to run with dev
if [ $# -ge 1 ]; then
  if [ "$1" == "dev" ]; then
    export FSOFTVER=$FSOFTDEV
    export FROOTVER=$FROOTDEV
    shift
  fi
fi

echo FSOFTVER: $FSOFTVER
echo FROOTVER: $FROOTVER

# install everything by default
SETUP_FAIRSOFT=1
SETUP_FAIRROOT=1
SETUP_CBMROOT=1

echo number of parameters: $#

# handle parameters, if supplied
if [ $# -eq 1 ]; then
  SETUP_FAIRSOFT=$1
  SETUP_FAIRROOT=0
  SETUP_CBMROOT=0
fi
# handle parameters, if supplied
if [ $# -eq 2 ]; then
  SETUP_FAIRSOFT=$1
  SETUP_FAIRROOT=$2
  SETUP_CBMROOT=0
fi
# handle parameters, if supplied
if [ $# -eq 3 ]; then
  SETUP_FAIRSOFT=$1
  SETUP_FAIRROOT=$2
  SETUP_CBMROOT=$3
fi

echo "Install Fairsoft:" $SETUP_FAIRSOFT
echo "Install Fairroot:" $SETUP_FAIRROOT
echo "Install Cbmroot :" $SETUP_CBMROOT
echo

# exit

#
##   FairSoft
#

if [ $SETUP_FAIRSOFT -ge 1 ]; then
  echo "Setting up Fairsoft ..."

  cd ..
  git clone https://github.com/FairRootGroup/FairSoft fairsoft_src_${FSOFTVER}_root${ROOTVER}
  cd fairsoft_src_${FSOFTVER}_root${ROOTVER}
  git tag -l
  git checkout -b $FSOFTVER $FSOFTVER

  #emacs -nw automatic.conf
  #./configure.sh automatic.conf

  if [ $ROOTVER -eq 6 ]; then
    sed s/build_root6=no/build_root6=yes/ automatic.conf > automatic1_root.conf
  else 
    cp automatic.conf automatic1_root.conf
  fi
  FSOFTINSTALLPATH=`pwd | sed s/fairsoft_src_/fairsoft_/`
  sed /SIMPATH_INSTALL/d automatic1_root.conf > automatic2_path.conf
  echo "  SIMPATH_INSTALL=$FSOFTINSTALLPATH/installation" >> automatic2_path.conf
  sed s/compiler=/compiler=gcc/ automatic2_path.conf > automatic3_gcc.conf
  ./configure.sh automatic3_gcc.conf
  
  cd $CBMSRCDIR
  echo done installing FairSoft
fi


#
##   FairRoot
#

if [ $SETUP_FAIRROOT -ge 1 ]; then
  echo "Setting up Fairroot ..."

  # set SIMPATH
  cd ..
  echo "SIMPATH	before: $SIMPATH"
  cd fairsoft_${FSOFTVER}_root${ROOTVER}/installation/
  export SIMPATH=`pwd`
  echo "SIMPATH	now   : $SIMPATH"
  cd $CBMSRCDIR

  cd ..
  git clone https://github.com/FairRootGroup/FairRoot.git fairroot_src_$FROOTVER-fairsoft_${FSOFTVER}_root${ROOTVER}
  cd fairroot_src_$FROOTVER-fairsoft_${FSOFTVER}_root${ROOTVER}
  git tag -l
  git checkout -b $FROOTVER $FROOTVER
  mkdir build
  cd build
  cmake \
    -DCMAKE_CXX_COMPILER=$($SIMPATH/bin/fairsoft-config --cxx) \
    -DCMAKE_C_COMPILER=$($SIMPATH/bin/fairsoft-config --cc) \
    -DCMAKE_INSTALL_PREFIX=../../fairroot_$FROOTVER-fairsoft_${FSOFTVER}_root${ROOTVER} \
    ..
  nice make install -j$NUMOFCPU
  
  cd $CBMSRCDIR
  echo done installing FairRoot
fi


#
##   CbmRoot
#

if [ $SETUP_CBMROOT -ge 1 ]; then
  echo "Setting up Cbmroot ..."

  # set SIMPATH
  cd ..
  echo "SIMPATH	before: $SIMPATH"
  cd fairsoft_${FSOFTVER}_root${ROOTVER}/installation/
  export SIMPATH=`pwd`
  echo "SIMPATH	now   : $SIMPATH"
  cd $CBMSRCDIR

  # set FAIRROOTPATH
  cd ..
  cd fairroot_$FROOTVER-fairsoft_${FSOFTVER}_root${ROOTVER}
  export FAIRROOTPATH=`pwd`
  echo "FAIRROOTPATH: $FAIRROOTPATH"
  cd $CBMSRCDIR
  
  cd ..
  if [ -d fieldmaps ]; then
    svn up fieldmaps    
  else
    svn co https://subversion.gsi.de/cbmsoft/cbmroot/fieldmaps fieldmaps
  fi
  
  #svn co https://subversion.gsi.de/cbmsoft/cbmroot/trunk cbm_$CBMSRCDIR
  cd $CBMSRCDIR
  mkdir build
  cd build
  cmake \
    -DCMAKE_CXX_COMPILER=$($SIMPATH/bin/fairsoft-config --cxx) \
    -DCMAKE_C_COMPILER=$($SIMPATH/bin/fairsoft-config --cc) \
    ..
  nice make -j$NUMOFCPU
  cd ..
  
  cd input
  ln -s ../../fieldmaps/* .
  cd ..
    
  echo done installing CbmRoot

  echo
  echo ". build/config.sh"
  echo "export SIMPATH=$SIMPATH"
  echo "export FAIRROOTPATH=$FAIRROOTPATH"

fi

#####################################################################################
#####################################################################################
