#!/bin/bash
#
##   semi-automated script installing FairSoft, FairRoot and CbmRoot
#

# 17.07.2015 - introduce option to compile dev settings 
# 02.06.2015 - introduce parameters for individual package selection
# 13.03.2015 - initial version
# by David Emschermann

#-------------------------------------
# usage:
# svn co https://subversion.gsi.de/cbmsoft/cbmroot/trunk $CBMSRCDIR
# cd $CBMSRCDIR
# ./autoinstall_framework.sh
# or
# ./autoinstall_framework.sh 1 1 1
#-------------------------------------

# put your desired variants here:
export FSOFTDEV=jul15
export FROOTDEV=v-15.07

export FSOFTPRO=mar15p2
export FROOTPRO=v-15.03a

export NUMOFCPU=`cat /proc/cpuinfo | grep processor | wc -l`
export CBMSRCDIR=`pwd`

#-------------------------------------

# set default version to pro
export FSOFTVER=$FSOFTPRO
export FROOTVER=$FROOTPRO

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
  git clone https://github.com/FairRootGroup/FairSoft fairsoft_$FSOFTVER
  cd fairsoft_$FSOFTVER
  git tag -l
  git checkout -b $FSOFTVER $FSOFTVER
  #emacs -nw automatic.conf
  #./configure.sh automatic.conf
  sed s/compiler=/compiler=gcc/ automatic.conf > automatic_gcc.conf
  ./configure.sh automatic_gcc.conf
  
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
  cd fairsoft_$FSOFTVER/installation/
  export SIMPATH=`pwd`
  echo "SIMPATH	now   : $SIMPATH"
  cd $CBMSRCDIR

  cd ..
  git clone https://github.com/FairRootGroup/FairRoot.git fairroot_src_$FROOTVER
  cd fairroot_src_$FROOTVER
  git tag -l
  git checkout -b $FROOTVER $FROOTVER
  mkdir build
  cd build
  cmake .. -DCMAKE_INSTALL_PREFIX=../../fairroot_$FROOTVER-fairsoft_$FSOFTVER
  nice make install -j$NUMOFCPU
  
  cd $CBMSRCDIR
  echo done installing FairRoot
fi


#
##   CbmRoot
#

echo "SIMPATH     : $SIMPATH"
echo "FAIRROOTPATH: $FAIRROOTPATH"
  
if [ $SETUP_CBMROOT -ge 1 ]; then
  echo "Setting up Cbmroot ..."

  # set SIMPATH
  cd ..
  echo "SIMPATH	before: $SIMPATH"
  cd fairsoft_$FSOFTVER/installation/
  export SIMPATH=`pwd`
  echo "SIMPATH	now   : $SIMPATH"
  cd $CBMSRCDIR

  # set FAIRROOTPATH
  cd ..
  cd fairroot_$FROOTVER-fairsoft_$FSOFTVER
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
  cmake ..
  nice make -j$NUMOFCPU
  cd ..
  
  cd input
  ln -s ../../fieldmaps/* .
  cd ..
  
  . build/config.sh
  
  echo done installing CbmRoot
fi


#####################################################################################
#####################################################################################
