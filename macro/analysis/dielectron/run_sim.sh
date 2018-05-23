#!/bin/sh

#inmed, qgp, rho0, omegaepem, omegadalitz, phi

nofJobs=5
#dataDir=/lustre/nyx/cbm/users/slebedev/cbm/data/lmvm/
dataDir=/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm


collEnergy=8gev
plutoParticle=omegaepem
outdir=${dataDir}/${collEnergy}/${plutoParticle}
#logFile=${outdir}/log/log_slurm-%A_%a.out
#errorFile=${outdir}/error/error_slurm-%A_%a.out
jobName=lmvm_${collEnergy}_${plutoParticle}
#directories must be created by user
#mkdir -p $(dirname "${logFile}")
#mkdir -p $(dirname "${errorFile}")

geoSetupMacro=diel_setup_sis100.C

sbatch --job-name=${jobName}  --workdir=${outdir}  --time=8:00:00 --array=1-${nofJobs} ./sim.sh ${outdir} ${collEnergy} ${plutoParticle} ${geoSetupMacro}
