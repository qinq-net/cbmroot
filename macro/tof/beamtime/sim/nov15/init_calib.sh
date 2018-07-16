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

if [ -z "$iDeadTime" ]; then
  if [ -z "$7" ]; then
    echo 'no dead time specified'
    exit 1
  else
    iDeadTime=$7
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


iNEvents=100000

echo "Initialize clusterizer calibration for run ${UnpackDir}"
echo "Calibration setup is ${iDut}${iMRef}${iBRef}"


iRestart=0
iStep=0
iStepLast=0


InitIndices=( $(find ${CalibDir} -maxdepth 1 -type d -name 'Init*' -printf '%f\n' | cut -c5- | sort -n) )

for iInitIndex in ${InitIndices[@]}
do
  if [ "$iInitIndex" -gt "$iRestart" ]; then
    rm -rf ${CalibDir}/Init${iInitIndex}
  fi
done


#if false; then
if true; then

  OptList=(
    ''${iNEvents}',0,0,0,50'
    ''${iNEvents}',03,0,0,50'
    ''${iNEvents}',13,0,0,50'
    ''${iNEvents}',23,0,0,50'
    ''${iNEvents}',23,0,0,50'
    ''${iNEvents}',33,0,0,50'
    ''${iNEvents}',33,0,0,50'
    ''${iNEvents}',33,0,0,50'
    ''${iNEvents}',43,0,0,50'
    ''${iNEvents}',43,0,0,50'
    ''${iNEvents}',43,0,0,50'
    ''${iNEvents}',53,0,0,50'
    ''${iNEvents}',53,0,0,50'
    ''${iNEvents}',63,0,0,50'
    ''${iNEvents}',63,0,0,50'
  )

  for inOpt in ${OptList[@]}
  do

    ((iStepLast = ${iStep}))
    ((iStep += 1))

    mkdir ${CalibDir}/Init${iStep}

    cp -v ${MacroDir}/.rootrc ${CalibDir}/Init${iStep}/
    cp -v ${MacroDir}/rootlogon.C ${CalibDir}/Init${iStep}/

    cd ${CalibDir}/Init${iStep}


    if [ "$iStep" -gt "$iRestart" ]; then
      # copy previous calibration file
      if [ "$iStep" -gt "1" ]; then
        cp -v ${CalibDir}/Init${iStepLast}/calib_cluster.cor_out.root ${CalibDir}/Init${iStep}/calib_cluster.cor_in.root
      fi

      # generate new calibration file
      root -b -q ''${MacroDir}'/ana_digi_ini.C('$inOpt',"'${UnpackDir}'",'${iDut}','${iMRef}','${iBRef}','${iDeadTime}')'

      rm ${CalibDir}/Init${iStep}/all_*
      rm ${CalibDir}/Init${iStep}/digi_ini.out.root

      echo "Init step $iStep with option $inOpt finished"
    else
      echo "Init step $iStep with option $inOpt skipped"
    fi

  done

  cp -v ${CalibDir}/Init${iStep}/calib_cluster_ini.hst.root ${CalibDir}/hst/final_calib_cluster_init_ini.hst.root
 
fi


OptList=(
  ''${iNEvents}',03,0,0,50'
  ''${iNEvents}',03,1,'${iMRef}',0'
  ''${iNEvents}',03,0,'${iDut}',0'
  ''${iNEvents}',03,1,'${iMRef}',0'
  ''${iNEvents}',03,0,'${iDut}',0'
  ''${iNEvents}',13,0,0,50'
  ''${iNEvents}',13,0,0,50'
  ''${iNEvents}',13,1,'${iMRef}',0'
  ''${iNEvents}',13,0,'${iDut}',0'
  ''${iNEvents}',13,1,'${iMRef}',0'
  ''${iNEvents}',13,0,'${iDut}',0'
  ''${iNEvents}',23,0,0,50'
  ''${iNEvents}',23,0,0,50'
  ''${iNEvents}',23,1,'${iMRef}',0'
  ''${iNEvents}',23,0,'${iDut}',0'
  ''${iNEvents}',23,1,'${iMRef}',0'
  ''${iNEvents}',23,0,'${iDut}',0'
  ''${iNEvents}',33,0,0,50'
  ''${iNEvents}',33,0,0,50'
  ''${iNEvents}',33,1,'${iMRef}',0'
  ''${iNEvents}',33,0,'${iDut}',0'
  ''${iNEvents}',33,1,'${iMRef}',0'
  ''${iNEvents}',43,0,0,50'
  ''${iNEvents}',43,0,0,50'
  ''${iNEvents}',43,1,'${iMRef}',0'
  ''${iNEvents}',43,0,'${iDut}',0'
  ''${iNEvents}',43,1,'${iMRef}',0'
  ''${iNEvents}',43,0,'${iDut}',0'
  ''${iNEvents}',43,1,'${iMRef}',0'
  ''${iNEvents}',43,0,0,50'
  ''${iNEvents}',43,0,0,50'
  ''${iNEvents}',43,1,'${iMRef}',0'
  ''${iNEvents}',43,0,'${iDut}',0'
  ''${iNEvents}',43,1,'${iMRef}',0'
  ''${iNEvents}',43,0,'${iDut}',0'
  ''${iNEvents}',53,0,0,50'
  ''${iNEvents}',53,0,0,50'
  ''${iNEvents}',53,1,'${iMRef}',0'
  ''${iNEvents}',53,0,'${iDut}',0'
  ''${iNEvents}',53,1,'${iMRef}',0'
  ''${iNEvents}',53,0,'${iDut}',0'
  '500000,14,0,-'${iDut}',0'
  '500000,14,1,-'${iMRef}',0'
  ''${iNEvents}',53,0,'${iDut}',0'
  ''${iNEvents}',53,1,'${iMRef}',0'
  ''${iNEvents}',53,0,'${iDut}',0'
  ''${iNEvents}',53,1,'${iMRef}',0'
  ''${iNEvents}',63,0,'${iDut}',0'
  ''${iNEvents}',63,1,'${iMRef}',0'
  ''${iNEvents}',63,0,0,50'
  ''${iNEvents}',63,0,0,50'
  ''${iNEvents}',63,0,'${iDut}',0'
  ''${iNEvents}',63,1,'${iMRef}',0'
  ''${iNEvents}',63,0,'${iDut}',0'
  ''${iNEvents}',63,1,'${iMRef}',0'
  ''${iNEvents}',73,0,'${iDut}',0'
  ''${iNEvents}',73,1,'${iMRef}',0'
  ''${iNEvents}',73,0,0,50'
  ''${iNEvents}',73,0,0,50'
  ''${iNEvents}',73,0,'${iDut}',0'
  ''${iNEvents}',73,1,'${iMRef}',0'
  ''${iNEvents}',73,0,'${iDut}',0'
  ''${iNEvents}',73,1,'${iMRef}',0'
  ''${iNEvents}',73,0,'${iDut}',0'
  '500000,24,0,-'${iDut}',0'
  ''${iNEvents}',73,1,'${iMRef}',0'
  '500000,24,1,-'${iMRef}',0'
  ''${iNEvents}',73,0,0,50'
  ''${iNEvents}',73,0,0,50'
  ''${iNEvents}',73,0,'${iDut}',0'
  ''${iNEvents}',73,1,'${iMRef}',0'
  ''${iNEvents}',73,0,0,50'
  ''${iNEvents}',73,0,0,50'
  ''${iNEvents}',73,0,'${iDut}',0'
  ''${iNEvents}',73,1,'${iMRef}',0'
  ''${iNEvents}',83,0,'${iDut}',0'
  ''${iNEvents}',83,1,'${iMRef}',0'
  ''${iNEvents}',83,0,'${iDut}',0'
  ''${iNEvents}',83,1,'${iMRef}',0'
  '500000,34,0,-'${iDut}',0'
  ''${iNEvents}',83,1,'${iMRef}',0'
  '500000,34,1,-'${iMRef}',0'
  ''${iNEvents}',93,0,'${iDut}',0'
  ''${iNEvents}',93,1,'${iMRef}',0'
  ''${iNEvents}',93,0,'${iDut}',0'
  ''${iNEvents}',93,1,'${iMRef}',0'
)

for inOpt in ${OptList[@]}

do   
  ((iStepLast = ${iStep}))
  ((iStep += 1))

  mkdir ${CalibDir}/Init${iStep}

  cp -v ${MacroDir}/.rootrc ${CalibDir}/Init${iStep}/
  cp -v ${MacroDir}/rootlogon.C ${CalibDir}/Init${iStep}/

  cd ${CalibDir}/Init${iStep}


  if [ "$iStep" -gt "$iRestart" ]; then
    # copy previous calibration file
    if [ "$iStep" -gt "1" ]; then
      cp -v ${CalibDir}/Init${iStepLast}/calib_cluster.cor_out.root ${CalibDir}/Init${iStep}/calib_cluster.cor_in.root
    fi

    # generate new calibration file
    root -b -q ''${MacroDir}'/ana_digi_cal.C('$inOpt',"'${UnpackDir}'",'${iDut}','${iMRef}','${iBRef}','${iDeadTime}')'

    rm ${CalibDir}/Init${iStep}/all_*
    rm ${CalibDir}/Init${iStep}/digi_cal.out.root

    echo "Init step $iStep with option $inOpt finished"
  else
    echo "Init step $iStep with option $inOpt skipped"
  fi

done

cp -v ${CalibDir}/Init${iStep}/calib_cluster_cal.hst.root ${CalibDir}/hst/final_calib_cluster_init_cal.hst.root
cp -v ${CalibDir}/Init${iStep}/calib_cluster.cor_out.root ${CalibDir}/hst/final_calib_cluster_init.cor_out.root


