#!/bin/bash
#SBATCH --job-name="run_track"
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


if [ ! -f "${ListDir}/unpack_list.sh" ] || [ ! -f "${ListDir}/calib_list.sh" ] || [ ! -f "${ListDir}/track_list.sh" ]; then
  echo 'missing the unpack/calib/track list file'
  exit 1
fi

unset UnpackList
unset CalibList
unset TrackList

source "${ListDir}/unpack_list.sh"
source "${ListDir}/calib_list.sh"
source "${ListDir}/track_list.sh"

if [ -z "${UnpackList}" ] || [ -z "${CalibList}" ] || [ -z "${TrackList}" ]; then
  echo 'unpack/calib/track list is empty'
  exit 1
fi

iNUnpackRuns=${#UnpackList[@]}
iNCalibRuns=${#CalibList[@]}
iNTrackRuns=${#TrackList[@]}


UnpackParameters=${UnpackList[$(((SLURM_ARRAY_TASK_ID-1)/(iNCalibRuns*iNTrackRuns)))]}
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


CalibParameters=${CalibList[$((((SLURM_ARRAY_TASK_ID-1)/iNTrackRuns)%iNCalibRuns))]}
CalibParameters=($CalibParameters)

if [ "${#CalibParameters[@]}" != "5" ]; then
  echo 'more/less than 5 calib parameters given'
  exit 1
fi

iCalDut=${CalibParameters[0]}
iCalMRef=${CalibParameters[1]}
iBRef=${CalibParameters[2]}
iCalSel2=${CalibParameters[3]}
iDeadTime=${CalibParameters[4]}


CalibDir=${UnpackDir}/Calib_${iCalDut}${iCalMRef}${iBRef}_${iCalSel2}_DT${iDeadTime}ns


if [ ! -d "${UnpackDir}" ] || [ ! -d "${CalibDir}" ]; then
  echo 'unpack/calib directory does not exist'
  exit 1
fi


TrackParameters=${TrackList[$(((SLURM_ARRAY_TASK_ID-1)%iNTrackRuns))]}
TrackParameters=($TrackParameters)

if [ "${#TrackParameters[@]}" != "5" ]; then
  echo 'more/less than 5 track parameters given'
  exit 1
fi

iSetup=${TrackParameters[0]}
iDut=${TrackParameters[1]}
iMRef=${TrackParameters[2]}
iSel2=${TrackParameters[3]}
dScaleFactor=${TrackParameters[4]}


TrackDir=${CalibDir}/Track_STP${iSetup}_${iDut}_${iMRef}_${iSel2}_SF${dScaleFactor}


mkdir -p ${TrackDir}/data
mkdir -p ${TrackDir}/hst
cd ${TrackDir}


${MacroDir}/iter_tracks.sh ${MacroDir} ${UnpackDir} ${CalibDir} ${TrackDir} ${iSetup} ${iDut} ${iMRef} ${iBRef} ${iSel2} ${dScaleFactor}


echo ""
echo "Array job ran on ${SLURM_JOB_CPUS_PER_NODE} CPUs of node ${SLURM_JOB_NODELIST} in partition ${SLURM_JOB_PARTITION}."
echo ""

mv -v ${WorkingDir}/slurm-${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}.out ${TrackDir}/

