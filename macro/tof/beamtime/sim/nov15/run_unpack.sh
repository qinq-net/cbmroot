#!/bin/bash
#SBATCH --job-name="run_unpack"
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

if [ -z "$SimulationDir" ]; then
  if [ -z "$3" ]; then
    echo 'no simulation directory specified'
    exit 1
  else
    SimulationDir=$3
  fi
fi

if [ -z "$MacroDir" ]; then
  if [ -z "$4" ]; then
    echo 'no macro directory specified'
    exit 1
  else
    MacroDir=$4
  fi
fi


source ${ConfigFile}

if [ -z "$VMCWORKDIR" ]; then
  echo 'no ROOT environment available'
  exit 1
fi


if [ ! -f "${ListDir}/unpack_list.sh" ]; then
  echo 'missing the unpack list file'
  exit 1
fi

unset UnpackList

source "${ListDir}/unpack_list.sh"

if [ -z "${UnpackList}" ]; then
  echo 'unpack list is empty'
  exit 1
fi

UnpackParameters=${UnpackList[$((SLURM_ARRAY_TASK_ID-1))]}
UnpackParameters=($UnpackParameters)

if [ "${#UnpackParameters[@]}" != "4" ]; then
  echo 'more/less than 4 unpack parameters given'
  exit 1
fi

RunID=${UnpackParameters[0]}
iEventWindow=${UnpackParameters[1]}
iTriggerSet=${UnpackParameters[2]}
iMultiplicity=${UnpackParameters[3]}


SimulationDir=`find ${SimulationDir} -type d -name ${RunID}`

if [ ! -d "${SimulationDir}" ]; then
  echo 'simulation directory does not exist'
  exit 1
fi


iNDirectories=`find ${SimulationDir} -maxdepth 1 -name '[0-9][0-9][0-9]' | wc -l`

if [ "${iNDirectories}" == "0" ]; then
  echo 'no simulated data subdirectories found'
  exit 1
fi


iNFiles=0

for Item in ${SimulationDir}/* ; do
  if [ -d "$Item" ] && [[ ! ${Item: -3} =~ [^[:digit:]] ]]; then
    FileName=`ls ${Item}/data/*.raw.root`
    if [ -f "${FileName}" ]; then
      ((iNFiles += 1))
    fi
  fi
done

if [ "${iNDirectories}" != "${iNFiles}" ]; then
  echo 'some simulated data files missing'
  exit 1
fi


cFileName=`basename $(find ${SimulationDir}/001/data -maxdepth 1 -type f -name *.raw.root)`


RunID=${RunID//-/}
RunID=${RunID//[T]/_}
RunID=${RunID//[Z]/}

UnpackDir=${WorkingDir}/CbmTofSim_${RunID}/Unpack_EVT${iEventWindow}ns_TRG${iTriggerSet}_MUL${iMultiplicity}
mkdir -p ${UnpackDir}/data
cd ${UnpackDir}


cp -v ${MacroDir}/.rootrc ${UnpackDir}/
cp -v ${MacroDir}/rootlogon.C ${UnpackDir}/


root -b -q ''${MacroDir}'/build_events.C("'${SimulationDir}'","'${cFileName}'",'${iNFiles}',\
                                         '${iEventWindow}','${iTriggerSet}','${iMultiplicity}')'


echo ""
echo "Array job ran on ${SLURM_JOB_CPUS_PER_NODE} CPUs of node ${SLURM_JOB_NODELIST} in partition ${SLURM_JOB_PARTITION}."
echo ""

mv -v ${WorkingDir}/slurm-${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}.out ${UnpackDir}/

