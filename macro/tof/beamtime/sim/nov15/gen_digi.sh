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


echo "Finalize clusterizer calibration for run ${UnpackDir}"
echo "Calibration setup is ${iDut}${iMRef}${iBRef}_${iSel2}"


mkdir ${CalibDir}/Gen

cp -v ${MacroDir}/.rootrc ${CalibDir}/Gen/
cp -v ${MacroDir}/rootlogon.C ${CalibDir}/Gen/

cd ${CalibDir}/Gen

#cp -v ${CalibDir}/hst/final_calib_cluster_init.cor_out.root ${CalibDir}/Gen/calib_cluster.cor_in.root
cp -v ${CalibDir}/hst/final_calib_cluster_iter.cor_out.root ${CalibDir}/Gen/calib_cluster.cor_in.root
cp -v ${CalibDir}/hst/final_calib_ana_iter.cor_out.root ${CalibDir}/Gen/calib_ana.cor_in.root

root -b -q ''${MacroDir}'/ana_digi.C(100000000,93,1,'${iMRef}',0,"'${UnpackDir}'",'${iDut}','${iMRef}','${iBRef}','${iSel2}','${iDeadTime}',1)'

rm ${CalibDir}/Gen/all_*

mv -v ${CalibDir}/Gen/digi.out.root ${CalibDir}/data/digi.out.root

cp -v ${CalibDir}/Gen/calib_cluster.hst.root ${CalibDir}/hst/final_calib_cluster_gen.hst.root
cp -v ${CalibDir}/Gen/calib_cluster.cor_out.root ${CalibDir}/hst/final_calib_cluster_gen.cor_out.root
cp -v ${CalibDir}/Gen/calib_ana.cor_out.root ${CalibDir}/hst/final_calib_ana_gen.cor_out.root

