#!/bin/sh

#inmed, qgp, rho0, omegaepem, omegadalitz, phi

nofJobs=100
dataDir=/lustre/nyx/cbm/users/slebedev/cbm/data/lmvm/may18_100k/
#dataDir=/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm

#set full path to geo setup macro
geoSetupMacroPath=/lustre/nyx/cbm/users/slebedev/cbm/trunk/cbmroot/macro/analysis/dielectron/geosetup/diel_setup_sis100.C
collEnergy=8gev

#for plutoParticle in omegaepem; do
for plutoParticle in inmed qgp omegaepem omegadalitz phi; do

outdir=${dataDir}/${collEnergy}/${plutoParticle}
logFile=${outdir}/log/log_slurm-%A_%a.out
errorFile=${outdir}/error/error_slurm-%A_%a.out
jobName=${collEnergy}_${plutoParticle}
#directories must be created by user
mkdir -p $(dirname "${logFile}")
mkdir -p $(dirname "${errorFile}")

sbatch --job-name=${jobName} --time=8:00:00 --workdir=${outdir} --output=${logFile} --error=${errorFile} --array=1-${nofJobs} sim.sh ${outdir} ${collEnergy} ${plutoParticle} ${geoSetupMacroPath}

done