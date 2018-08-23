#!/bin/sh

#inmed, qgp, rho0, omegaepem, omegadalitz, phi

nofJobs=100
#dataDir=/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/auautest/
#dataDir=/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/agag/august18_1kk_field60/
dataDir=/lustre/nyx/cbm/users/gpitsch/CbmRoot/data/lmvm/august18_100k_field50/
#set full path to geo setup macro
geoSetupMacroPath=/lustre/nyx/cbm/users/gpitsch/CbmRoot/trunk/macro/analysis/dielectron/geosetup/diel_setup_sis100.C
collEnergy=8gev


for plutoParticle in inmed qgp omegaepem omegadalitz phi; do
#for plutoParticle in omegaepem; do
#for XXXX in 0220 0221 0222 0223 0224 0225 0226 0227 0228 0229 0720 0721 0722 0723 0724 0725 0726 0727 0728 0729; do
outdir=${dataDir}/${collEnergy}/${plutoParticle}
logFile=${outdir}/log/log_slurm-%A_%a.out
errorFile=${outdir}/error/error_slurm-%A_%a.out
jobName=${collEnergy}_${plutoParticle}
#directories must be created by user
mkdir -p $(dirname "${logFile}")
mkdir -p $(dirname "${errorFile}")

sbatch --job-name=${jobName} --time=8:00:00 --workdir=${outdir} --output=${logFile} --error=${errorFile} --array=1-${nofJobs} sim.sh ${outdir} ${collEnergy} ${plutoParticle} ${geoSetupMacroPath} ${XXXX}
done
#done

