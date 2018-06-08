#!/bin/bash


if [ -z "$MacroDir" ]; then
  if [ -z "$1" ]; then
    echo 'no macro directory specified'
    exit 1
  else
    MacroDir=$1
  fi
fi

if [ -z "$UnpackDir" ]; then
  if [ -z "$2" ]; then
    echo 'no unpack directory specified'
    exit 1
  else
    UnpackDir=$2
  fi
fi

if [ -z "$CalibDir" ]; then
  if [ -z "$3" ]; then
    echo 'no calib directory specified'
    exit 1
  else
    CalibDir=$3
  fi
fi

if [ -z "$iDut" -o "${#iDut}" != "3" ]; then
  if [ -z "$4" -o "${#4}" != "3" ]; then
    echo 'no valid Dut specified'
    exit 1
  else
    iDut=$4
  fi
fi

if [ -z "$iMRef" -o "${#iMRef}" != "3" ]; then
  if [ -z "$5" -o "${#5}" != "3" ]; then
    echo 'no valid MRef specified'
    exit 1
  else
    iMRef=$5
  fi
fi

if [ -z "$iBRef" -o "${#iBRef}" != "3" ]; then
  if [ -z "$6" -o "${#6}" != "3" ]; then
    echo 'no valid BRef specified'
    exit 1
  else
    iBRef=$6
  fi
fi

if [ -z "$iSel2" -o "${#iSel2}" != "3" ]; then
  if [ -z "$7" -o "${#7}" != "3" ]; then
    echo 'no valid Sel2 specified'
    exit 1
  else
    iSel2=$7
  fi
fi

if [ -z "$iDeadTime" ]; then
  if [ -z "$8" ]; then
    echo 'no dead time specified'
    exit 1
  else
    iDeadTime=$8
  fi
fi


if [ -z "$VMCWORKDIR" ]; then
  echo 'no ROOT environment available'
  exit 1
fi


if [ ! -d "${MacroDir}" ]; then
  echo 'macro directory does not exist'
  exit 1
fi


if [ ! -d "${UnpackDir}" -o ! -d "${CalibDir}" ]; then
  echo 'unpack/calib directory does not exist'
  exit 1
fi


c0='0000'


echo "Iterate clusterizer calibration for run ${UnpackDir}"
echo "Calibration setup is ${iDut}${iMRef}${iBRef}_${iSel2}"


iRestart=0
iStep=0
iStepLast=0


for iMode in 1 6 8 
do

  case $iMode in 
    1)
      iNIterations=5
      OptList=(
        '1'${c0}'0,93,1,'${iMRef}''
        '1'${c0}'0,44,0,-'${iDut}''
        '1'${c0}'0,93,0,'${iDut}''
        '1'${c0}'0,44,1,-'${iMRef}''
        '1'${c0}'00,41,1,-'${iMRef}''
        '1'${c0}'0,93,0,'${iMRef}''
        '1'${c0}'00,41,0,-'${iDut}''
        '1'${c0}'0,93,1,'${iMRef}''
      )
      ;;
    2)
      iNIterations=2
      OptList=(
        '1'${c0}'0,92,1,'${iMRef}''
        '1'${c0}'00,64,1,-'${iMRef}''
        '1'${c0}'00,64,1,'${iBRef}''
        '1'${c0}'0,92,0,'${iDut}''
        '1'${c0}'00,64,0,-'${iDut}''
        '1'${c0}'00,64,0,'${iBRef}''
        '1'${c0}'0,92,1,'${iMRef}''
      )
      ;;
    3)
      iNIterations=5
      OptList=(
        '1'${c0}'0,44,0,-'${iDut}''
        '1'${c0}'0,44,1,-'${iMRef}''
      )
      ;;
    4)
      iNIterations=5
      OptList=(
        '5'${c0}'0,93,1,'${iMRef}''
        '5'${c0}'0,93,-2,2'
        '1'${c0}'000,61,-1,-2'
        '5'${c0}'0,93,-3,2'
      )
      ;;
    5)
      iNIterations=2
      OptList=(
        '5'${c0}'0,93,1,'${iMRef}''
        '5'${c0}'0,93,-2,2'
        '1'${c0}'00,64,1,-'${iMRef}''
        '1'${c0}'00,64,0,-'${iDut}''
        '5'${c0}'0,93,-3,2'
      )
      ;;
    6)
      iNIterations=2
      OptList=(
        '1'${c0}'0,93,1,'${iMRef}''
        '1'${c0}'00,93,-2,-2'
        '1'${c0}'0,93,0,'${iDut}''
        '1'${c0}'00,93,-2,-2'
        '1'${c0}'0,93,1,'${iMRef}''
      )
      ;;
    7)
      iNIterations=3
      OptList=(
        '1'${c0}',92,1,4'
        '1'${c0}'00,93,-2,-2'
        '1'${c0}',92,0,3'
        '1'${c0}'00,93,-2,2'
        '1'${c0}',92,1,4'
      )
      ;;
    8)
      iNIterations=2
      OptList=(
        '1'${c0}'00,93,1,'${iMRef}''
        '1'${c0}'0,92,0,'${iDut}''
        '1'${c0}'000,61,-1,-2'
        '1'${c0}'00,93,-2,2'
        '1'${c0}'00,93,1,'${iMRef}''
      )
      ;;
    9)
      iNIterations=2
      OptList=(
        '1'${c0}',92,1,4'
        '1'${c0}',92,0,3'
        '1'${c0}'00,61,-1,-2'
        '1'${c0}'0,93,-2,2'
        '1'${c0}',92,1,4'
      )
      ;;
   10)
      iNIterations=1
      OptList=(
        '1'${c0}'0,92,1,'${iMRef}''
        '1'${c0}'00,93,-3,2'
        '1'${c0}'00,93,-2,2'
        '1'${c0}'00,61,-1,-2'
        '1'${c0}'0,92,1,'${iMRef}''
      )
      ;;
   11)
      iNIterations=1
      OptList=(
        '1'${c0}',92,1,'${iMRef}''
        '1'${c0}'00,93,-3,2'
        '1'${c0}'00,93,-2,2'
        '1'${c0}'00,61,-1,-2'
        '1'${c0}'0,92,1,'${iMRef}''
      )
      ;;
  esac


  echo "execute ${iNIterations} iterations of ${OptList}"

  while [ "${iNIterations}" -gt "0" ]; do

    for inOpt in ${OptList[@]}
    do

      ((iStepLast = ${iStep}))
      ((iStep += 1))

      mkdir ${CalibDir}/Iter${iStep}

      cp -v ${MacroDir}/.rootrc ${CalibDir}/Iter${iStep}/
      cp -v ${MacroDir}/rootlogon.C ${CalibDir}/Iter${iStep}/

      cd ${CalibDir}/Iter${iStep}

      if [ "$iStep" -gt "$iRestart" ]; then

        # copy previous calibration files
        if [ "$iStep" -eq "1" ]; then
          cp -v ${CalibDir}/hst/final_calib_cluster_init.cor_out.root ${CalibDir}/Iter${iStep}/calib_cluster.cor_in.root
        else
          cp -v ${CalibDir}/Iter${iStepLast}/calib_cluster.cor_out.root ${CalibDir}/Iter${iStep}/calib_cluster.cor_in.root
          cp -v ${CalibDir}/Iter${iStepLast}/calib_ana.cor_out.root ${CalibDir}/Iter${iStep}/calib_ana.cor_in.root
        fi

        # generate new calibration file
        root -b -q ''${MacroDir}'/ana_digi.C('$inOpt',1,"'${UnpackDir}'",'${iDut}','${iMRef}','${iBRef}','${iSel2}','${iDeadTime}',0)'

        rm ${CalibDir}/Iter${iStep}/all_*
        rm ${CalibDir}/Iter${iStep}/digi.out.root

        echo "Iter step $iStep with option $inOpt finished"
      else
        echo "Iter step $iStep with option $inOpt skipped"
      fi

    done
    (( iNIterations -= 1))
  done

done 

cp -v ${CalibDir}/Iter${iStep}/calib_cluster.hst.root ${CalibDir}/hst/final_calib_cluster_iter.hst.root
cp -v ${CalibDir}/Iter${iStep}/calib_cluster.cor_out.root ${CalibDir}/hst/final_calib_cluster_iter.cor_out.root
cp -v ${CalibDir}/Iter${iStep}/calib_ana.cor_out.root ${CalibDir}/hst/final_calib_ana_iter.cor_out.root

