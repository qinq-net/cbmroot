#!/bin/bash
#SBATCH --job-name="run_calib"
#SBATCH --comment="CBM TOF"
#SBATCH --open-mode=truncate


if [ -z "$SLURM_SUBMIT_DIR" ]; then
  echo 'script needs to be invoked by sbatch'
  exit 1
fi


WorkingDir=`pwd`


if [ -z "$ConfigFile" ]; then
  if [ -z "$1" ]; then
    echo 'no CbmRoot config file specified'
    exit 1
  else
    ConfigFile=$1
  fi
fi

if [ -z "$ListDir" ]; then
  if [ -z "$2" ]; then
    echo 'no analysis list directory specified'
    exit 1
  else
    ListDir=$2
  fi
fi

if [ -z "$MacroDir" ]; then
  if [ -z "$3" ]; then
    echo 'no macro directory specified'
    exit 1
  else
    MacroDir=$3
  fi
fi


source ${ConfigFile}

if [ -z "$VMCWORKDIR" ]; then
  echo 'no ROOT environment available'
  exit 1
fi


if [ ! -f "${ListDir}/unpack_list.sh" ] || [ ! -f "${ListDir}/calib_list.sh" ]; then
  echo 'missing the unpack/calib list file'
  exit 1
fi

unset UnpackList
unset CalibList

source "${ListDir}/unpack_list.sh"
source "${ListDir}/calib_list.sh"

if [ -z "${UnpackList}" ] || [ -z "${CalibList}" ]; then
  echo 'unpack/calib list is empty'
  exit 1
fi

iNUnpackRuns=${#UnpackList[@]}
iNCalibRuns=${#CalibList[@]}


UnpackParameters=${UnpackList[$(((SLURM_ARRAY_TASK_ID-1)/iNCalibRuns))]}
UnpackParameters=($UnpackParameters)

if [ "${#UnpackParameters[@]}" != "4" ]; then
  echo 'more/less than 4 unpack parameters given'
  exit 1
fi

RunID=${UnpackParameters[0]}
iEventWindow=${UnpackParameters[1]}
iTriggerSet=${UnpackParameters[2]}
iMultiplicity=${UnpackParameters[3]}


RunID=${RunID//-/}
RunID=${RunID//[T]/_}
RunID=${RunID//[Z]/}

UnpackDir=${WorkingDir}/CbmTofSim_${RunID}/Unpack_EVT${iEventWindow}ns_TRG${iTriggerSet}_MUL${iMultiplicity}

if [ ! -d "${UnpackDir}" ]; then
  echo 'unpack directory does not exist'
  exit 1
fi


CalibParameters=${CalibList[$(((SLURM_ARRAY_TASK_ID-1)%iNCalibRuns))]}
CalibParameters=($CalibParameters)

if [ "${#CalibParameters[@]}" != "5" ]; then
  echo 'more/less than 5 calib parameters given'
  exit 1
fi

iDut=${CalibParameters[0]}
iMRef=${CalibParameters[1]}
iBRef=${CalibParameters[2]}
iSel2=${CalibParameters[3]}
iDeadTime=${CalibParameters[4]}


CalibDir=${UnpackDir}/Calib_${iDut}${iMRef}${iBRef}_${iSel2}_DT${iDeadTime}ns

mkdir -p ${CalibDir}/data
mkdir -p ${CalibDir}/hst
cd ${CalibDir}


${MacroDir}/init_calib.sh ${MacroDir} ${UnpackDir} ${CalibDir} ${iDut} ${iMRef} ${iBRef} ${iDeadTime}
${MacroDir}/iter_calib.sh ${MacroDir} ${UnpackDir} ${CalibDir} ${iDut} ${iMRef} ${iBRef} ${iSel2} ${iDeadTime}
${MacroDir}/gen_digi.sh ${MacroDir} ${UnpackDir} ${CalibDir} ${iDut} ${iMRef} ${iBRef} ${iSel2} ${iDeadTime}


echo ""
echo "Array job ran on ${SLURM_JOB_CPUS_PER_NODE} CPUs of node ${SLURM_JOB_NODELIST} in partition ${SLURM_JOB_PARTITION}."
echo ""

mv -v ${WorkingDir}/slurm-${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}.out ${CalibDir}/

