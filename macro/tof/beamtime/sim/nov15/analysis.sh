#!/bin/bash

################################################################################
# General settings                                                             #
################################################################################
cPartition=long             # Kronos partitions (long, main, debug)
iMemoryPerCore=4096         # requested memory per physical core [MB]
iNCores=2                   # requested number of cores per array job
                            # (here > 1 to reserve additional job memory)

iUnpackTimeLimit=0-02:00:00 # expected unpacking   job completion time
iCalibTimeLimit=1-12:00:00  # expected calibration job completion time
iAnaTimeLimit=1-00:00:00    # expected analysis    job completion time
iTrackTimeLimit=1-12:00:00  # expected tracking    job completion time

bRunUnpack=0                # run unpacking   stage
bRunCalib=0                 # run calibration stage
bRunAna=0                   # run analysis    stage
bRunTrack=1                 # run tracking    stage

################################################################################
# User-specific settings                                                       #
################################################################################
ConfigFile=/lustre/nyx/cbm/users/csimon/cbmroot/trunk/bld/v1703_mar17/config.sh
MacroDir=/lustre/nyx/cbm/users/csimon/cbmroot/trunk/src/macro/tof/beamtime/sim/nov15

SimulationDir=/lustre/nyx/cbm/users/csimon/simulation/production/sps_nov15_sim
AnalysisDir=/lustre/nyx/cbm/users/csimon/analysis/sps_nov15_sim

################################################################################
# Debug settings (overwrite parts of the above)                                #
################################################################################
#cPartition=debug
#iUnpackTimeLimit=0-00:15:00
#bRunCalib=0
#bRunAna=0
################################################################################
################################################################################

ListDir=${AnalysisDir}/runs/`date -u +"%Y-%m-%dT%H-%M-%SZ"`

if [ ! -f "${MacroDir}/unpack_list.sh" ] || \
   [ ! -f "${MacroDir}/calib_list.sh" ] || \
   [ ! -f "${MacroDir}/ana_list.sh" ] || \
   [ ! -f "${MacroDir}/track_list.sh" ]; then
  echo 'missing at least one analysis list file'
  return 1
fi

unset UnpackList
unset CalibList
unset AnaList
unset TrackList

source "${MacroDir}/unpack_list.sh"
source "${MacroDir}/calib_list.sh"
source "${MacroDir}/ana_list.sh"
source "${MacroDir}/track_list.sh"

if [ -z "${UnpackList}" ] || [ -z "${CalibList}" ] || [ -z "${AnaList}" ] || [ -z "${TrackList}" ]; then
  echo 'at least one analysis list is empty'
  return 1
fi

iNUnpackRuns=${#UnpackList[@]}
iNCalibRuns=${#CalibList[@]}
iNCalibRuns=$((iNUnpackRuns*iNCalibRuns))
iNAnaRuns=${#AnaList[@]}
iNAnaRuns=$((iNCalibRuns*iNAnaRuns))
iNTrackRuns=${#TrackList[@]}
iNTrackRuns=$((iNCalibRuns*iNTrackRuns))


mkdir -p ${ListDir}

cp ${MacroDir}/unpack_list.sh ${ListDir}/
cp ${MacroDir}/calib_list.sh ${ListDir}/
cp ${MacroDir}/ana_list.sh ${ListDir}/
cp ${MacroDir}/track_list.sh ${ListDir}/


if [ "${bRunUnpack}" == "1" ]; then
  cDepUnpack=$(sbatch --time=${iUnpackTimeLimit} --cpus-per-task=${iNCores} --mem-per-cpu=${iMemoryPerCore} \
                      --partition=${cPartition} --workdir=${AnalysisDir} --array=1-${iNUnpackRuns} \
                      ${MacroDir}/run_unpack.sh ${ConfigFile} ${ListDir} ${SimulationDir} ${MacroDir})
  cDepUnpack=${cDepUnpack##* }
fi


if [ "${bRunCalib}" == "1" ]; then
  if [ "${bRunUnpack}" == "1" ]; then
    cDepCalib=$(sbatch --dependency=afterany:${cDepUnpack} \
                       --time=${iCalibTimeLimit} --cpus-per-task=${iNCores} --mem-per-cpu=${iMemoryPerCore} \
                       --partition=${cPartition} --workdir=${AnalysisDir} --array=1-${iNCalibRuns} \
                       ${MacroDir}/run_calib.sh ${ConfigFile} ${ListDir} ${MacroDir})
  else
    cDepCalib=$(sbatch \
                       --time=${iCalibTimeLimit} --cpus-per-task=${iNCores} --mem-per-cpu=${iMemoryPerCore} \
                       --partition=${cPartition} --workdir=${AnalysisDir} --array=1-${iNCalibRuns} \
                       ${MacroDir}/run_calib.sh ${ConfigFile} ${ListDir} ${MacroDir})
  fi

  cDepCalib=${cDepCalib##* }
fi


if [ "${bRunAna}" == "1" ]; then
  if [ "${bRunCalib}" == "1" ]; then
    cDepAna=$(sbatch --dependency=afterany:${cDepCalib} \
                     --time=${iAnaTimeLimit} --cpus-per-task=${iNCores} --mem-per-cpu=${iMemoryPerCore} \
                     --partition=${cPartition} --workdir=${AnalysisDir} --array=1-${iNAnaRuns} \
                     ${MacroDir}/run_ana.sh ${ConfigFile} ${ListDir} ${MacroDir})
  else
    cDepAna=$(sbatch \
                     --time=${iAnaTimeLimit} --cpus-per-task=${iNCores} --mem-per-cpu=${iMemoryPerCore} \
                     --partition=${cPartition} --workdir=${AnalysisDir} --array=1-${iNAnaRuns} \
                     ${MacroDir}/run_ana.sh ${ConfigFile} ${ListDir} ${MacroDir})
  fi

  cDepAna=${cDepAna##* }
fi


if [ "${bRunTrack}" == "1" ]; then
  if [ "${bRunCalib}" == "1" ]; then
    cDepTrack=$(sbatch --dependency=afterany:${cDepCalib} \
                       --time=${iTrackTimeLimit} --cpus-per-task=${iNCores} --mem-per-cpu=${iMemoryPerCore} \
                       --partition=${cPartition} --workdir=${AnalysisDir} --array=1-${iNTrackRuns} \
                       ${MacroDir}/run_track.sh ${ConfigFile} ${ListDir} ${MacroDir})
  else
    cDepTrack=$(sbatch \
                       --time=${iTrackTimeLimit} --cpus-per-task=${iNCores} --mem-per-cpu=${iMemoryPerCore} \
                       --partition=${cPartition} --workdir=${AnalysisDir} --array=1-${iNTrackRuns} \
                       ${MacroDir}/run_track.sh ${ConfigFile} ${ListDir} ${MacroDir})
  fi

  cDepTrack=${cDepTrack##* }
fi

