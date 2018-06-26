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

if [ -z "$TrackDir" ]; then
  if [ -z "$4" ]; then
    echo 'no track directory specified'
    exit 1
  else
    TrackDir=$4
  fi
fi

if [ -z "$iSetup" ]; then
  if [ -z "$5" ]; then
    echo 'no tracking setup specified'
    exit 1
  else
    iSetup=$5
  fi
fi

if [ -z "$iDut" -o "${#iDut}" != "3" ]; then
  if [ -z "$6" -o "${#6}" != "3" ]; then
    echo 'no valid Dut specified'
    exit 1
  else
    iDut=$6
  fi
fi

if [ -z "$iMRef" -o "${#iMRef}" != "3" ]; then
  if [ -z "$7" -o "${#7}" != "3" ]; then
    echo 'no valid MRef specified'
    exit 1
  else
    iMRef=$7
  fi
fi

if [ -z "$iBRef" -o "${#iBRef}" != "3" ]; then
  if [ -z "$8" -o "${#8}" != "3" ]; then
    echo 'no valid BRef specified'
    exit 1
  else
    iBRef=$8
  fi
fi

if [ -z "$iSel2" -o "${#iSel2}" != "3" ]; then
  if [ -z "$9" -o "${#9}" != "3" ]; then
    echo 'no valid Sel2 specified'
    exit 1
  else
    iSel2=$9
  fi
fi

if [ -z "$dScaleFactor" ]; then
  if [ -z "${10}" ]; then
    echo 'no scale factor specified'
    exit 1
  else
    dScaleFactor=${10}
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


if [ ! -d "${UnpackDir}" -o ! -d "${CalibDir}" -o ! -d "${TrackDir}" ]; then
  echo 'unpack/calib/track directory does not exist'
  exit 1
fi


iNEvents=1000000 # FIXME

dDTres=2000
dDTRMSres=2000

dRange1=2
dRange1Limit=0.9
dRange2=8
dRange2Limit=4

iStep=0
iStepLast=0
iNIterations=1


rm -rf ${TrackDir}/data/*
rm -rf ${TrackDir}/hst/*
rm -rf ${TrackDir}/Final
rm -rf ${TrackDir}/Iter*


while [[ $dDTres > 0 ]]; do

  dRange1=`echo "${dRange1} * 0.8" | bc`
  compare_Range1=`echo "${dRange1} < ${dRange1Limit}" | bc`
  if [[ ${compare_Range1} > 0 ]]; then
    dRange1=$dRange1Limit
  fi

  dRange2=`echo "${dRange2} * 0.8" | bc`
  compare_Range2=`echo "${dRange2} < ${dRange2Limit}" | bc`
  if [[ ${compare_Range2} > 0 ]]; then
    dRange2=$dRange2Limit
  fi

  for iCal in 3 2 4 5
  do

    for ((n=0; n < ${iNIterations}; n++))
    do

      ((iStepLast = ${iStep}))
      ((iStep += 1))

      mkdir ${TrackDir}/Iter${iStep}

      cp -v ${MacroDir}/.rootrc ${TrackDir}/Iter${iStep}/
      cp -v ${MacroDir}/rootlogon.C ${TrackDir}/Iter${iStep}/

      cd ${TrackDir}/Iter${iStep}

      # copy previous calibration file
      if [ "$iStep" -gt "1" ]; then
        cp -v ${TrackDir}/Iter${iStepLast}/calib_track.cor_out.root ${TrackDir}/Iter${iStep}/calib_track.cor_in.root
        cp -v ${TrackDir}/Iter${iStepLast}/calib_ana.cor_out.root ${TrackDir}/Iter${iStep}/calib_ana.cor_in.root
      fi

      # generate new calibration file
      root -b -q ''${MacroDir}'/ana_trks.C('${iNEvents}','${iCal}',"'${UnpackDir}'","'${CalibDir}'",\
                                           '${iSetup}','${dRange1}','${dRange2}',kFALSE,\
                                           '${iDut}','${iMRef}','${iBRef}','${iSel2}','${dScaleFactor}')'

      rm ${TrackDir}/Iter${iStep}/all_*
      rm ${TrackDir}/Iter${iStep}/hits.out.root

    done

  done

  iTres=`cat TCalib.res`
  if [[ $iTres = 0 ]]; then
    echo "All tracks lost, stop at iteration ${iStep}!"
    exit 1
  fi

  ((TRMSres=${iTres}%1000))
  ((iTres -= TRMSres ))
  ((Tres   = iTres / 1000)) 
  dTdif=`echo "${dDTres} - ${Tres}" | bc`
  compare_result=`echo "${Tres} < ${dDTres}" | bc`

  dTRMSdif=`echo "${dDTRMSres} - ${TRMSres}" | bc`
  compare_RMS=`echo "${TRMSres} < ${dDTRMSres}" | bc`

  echo "Iteration ${iStep}: got TOff = ${Tres}, compare to ${dDTres}, dTdif = ${dTdif}, result = ${compare_result},\
        TRMS = ${TRMSres}, old ${dDTRMSres}, dif = ${dTRMSdif}, result = ${compare_RMS}"

  ((compare_result += ${compare_RMS}))
  echo "result_summary: ${compare_result}"

  if [[ ${compare_result} > 0 ]]; then
    if [[ ${Tres} = 0 ]]; then
      Tres=1
    fi
    dDTres=$Tres
    dDTRMSres=$TRMSres
  else
    dDTres=0
  fi

done


# final action -> scan full statistics
mkdir ${TrackDir}/Final

cp -v ${MacroDir}/.rootrc ${TrackDir}/Final/
cp -v ${MacroDir}/rootlogon.C ${TrackDir}/Final/

cd ${TrackDir}/Final

if [ "$iStep" -gt "1" ]; then
  cp -v ${TrackDir}/Iter${iStep}/calib_track.cor_out.root ${TrackDir}/Final/calib_track.cor_in.root
  cp -v ${TrackDir}/Iter${iStep}/calib_ana.cor_out.root ${TrackDir}/Final/calib_ana.cor_in.root
fi

root -b -q ''${MacroDir}'/ana_trks.C(0,0,"'${UnpackDir}'","'${CalibDir}'",\
                                     '${iSetup}','${dRange1Limit}','${dRange2Limit}',kTRUE,\
                                     '${iDut}','${iMRef}','${iBRef}','${iSel2}','${dScaleFactor}')'

rm ${TrackDir}/Final/all_*

mv -v ${TrackDir}/Final/hits.out.root ${TrackDir}/data/hits.out.root

cp -v ${TrackDir}/Final/ana_track.hst.root ${TrackDir}/hst/final_ana_track.hst.root
cp -v ${TrackDir}/Final/calib_track.cor_out.root ${TrackDir}/hst/final_calib_track.cor_out.root
cp -v ${TrackDir}/Final/calib_ana.cor_out.root ${TrackDir}/hst/final_calib_ana.cor_out.root

