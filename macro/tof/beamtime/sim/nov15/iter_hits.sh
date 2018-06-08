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

if [ -z "$AnaDir" ]; then
  if [ -z "$4" ]; then
    echo 'no ana directory specified'
    exit 1
  else
    AnaDir=$4
  fi
fi

if [ -z "$iDut" -o "${#iDut}" != "3" ]; then
  if [ -z "$5" -o "${#5}" != "3" ]; then
    echo 'no valid Dut specified'
    exit 1
  else
    iDut=$5
  fi
fi

if [ -z "$iMRef" -o "${#iMRef}" != "3" ]; then
  if [ -z "$6" -o "${#6}" != "3" ]; then
    echo 'no valid MRef specified'
    exit 1
  else
    iMRef=$6
  fi
fi

if [ -z "$iBRef" -o "${#iBRef}" != "3" ]; then
  if [ -z "$7" -o "${#7}" != "3" ]; then
    echo 'no valid BRef specified'
    exit 1
  else
    iBRef=$7
  fi
fi

if [ -z "$iSel2" -o "${#iSel2}" != "3" ]; then
  if [ -z "$8" -o "${#8}" != "3" ]; then
    echo 'no valid Sel2 specified'
    exit 1
  else
    iSel2=$8
  fi
fi

if [ -z "$dScaleFactor" ]; then
  if [ -z "$9" ]; then
    echo 'no scale factor specified'
    exit 1
  else
    dScaleFactor=$9
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


if [ ! -d "${UnpackDir}" -o ! -d "${CalibDir}" -o ! -d "${AnaDir}" ]; then
  echo 'unpack/calib/ana directory does not exist'
  exit 1
fi


dDTres=10000000
compare_result=1


iStep=0
iStepLast=0

while [ "$compare_result" -eq "1" ]; do

  for iCal in 1 2 3 5 6 1
  do

    ((iStepLast = ${iStep}))
    ((iStep += 1))

    mkdir ${AnaDir}/Iter${iStep}

    cp -v ${MacroDir}/.rootrc ${AnaDir}/Iter${iStep}/
    cp -v ${MacroDir}/rootlogon.C ${AnaDir}/Iter${iStep}/

    cd ${AnaDir}/Iter${iStep}

    # copy previous calibration file
    if [ "$iStep" -gt "1" ]; then
      cp -v ${AnaDir}/Iter${iStepLast}/calib_ana.cor_out.root ${AnaDir}/Iter${iStep}/calib_ana.cor_in.root
    fi

    # generate new calibration file
    root -b -q ''${MacroDir}'/ana_hits.C(10000000,'${iCal}',"'${UnpackDir}'","'${CalibDir}'",'${iDut}','${iMRef}','${iBRef}','${iSel2}','${dScaleFactor}')'

    rm ${AnaDir}/Iter${iStep}/all_*
    rm ${AnaDir}/Iter${iStep}/hits.out.root

  done

  if [ ! -e ./Test.res ]; then
    echo 'file Test.res does not exist'
    exit 1
  fi

  Tres=`cat ./Test.res`
  dTdif=`echo "$dDTres - $Tres" | bc`
  dDTres=`echo "$dDTres - 0.001" | bc`
  compare_result=`echo "$Tres < $dDTres" | bc`

  echo "got Tres = $Tres, compare to $dDTres, dTdif = $dTdif, compare_result = $compare_result"

  if [ "$compare_result" -eq "1" ]; then
    dDTres=$Tres
  fi

done


# final action -> scan full statistics
mkdir ${AnaDir}/Final

cp -v ${MacroDir}/.rootrc ${AnaDir}/Final/
cp -v ${MacroDir}/rootlogon.C ${AnaDir}/Final/

cd ${AnaDir}/Final

if [ "$iStep" -gt "1" ]; then
  cp -v ${AnaDir}/Iter${iStep}/calib_ana.cor_out.root ${AnaDir}/Final/calib_ana.cor_in.root
fi

root -b -q ''${MacroDir}'/ana_hits.C(1000000000,1,"'${UnpackDir}'","'${CalibDir}'",'${iDut}','${iMRef}','${iBRef}','${iSel2}','${dScaleFactor}')'

rm ${AnaDir}/Final/all_*
rm ${AnaDir}/Final/hits.out.root

cp -v ${AnaDir}/Final/ana_cluster.hst.root ${AnaDir}/hst/final_ana_cluster.hst.root
cp -v ${AnaDir}/Final/calib_ana.cor_out.root ${AnaDir}/hst/final_calib_ana.cor_out.root

