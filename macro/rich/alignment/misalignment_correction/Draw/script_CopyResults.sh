#!/bin/bash

INDIR=/lustre/nyx/cbm/users/jbendar/Sim_Outputs/Ring_Track_VS_Position
OUTDIR1=/u/jbendar/Files
OUTDIR2=Position_3

cp -R ${INDIR}/Aligned/results_* ${OUTDIR1}/${OUTDIR2}/Aligned/.
cp -R ${INDIR}/Misaligned_1mrad_Full/results_* ${OUTDIR1}/${OUTDIR2}/Misaligned_1mrad/.
cp -R ${INDIR}/Misaligned_2mrad_Full/results_* ${OUTDIR1}/${OUTDIR2}/Misaligned_2mrad/.
cp -R ${INDIR}/Misaligned_3mrad_Full/results_* ${OUTDIR1}/${OUTDIR2}/Misaligned_3mrad/.
cp -R ${INDIR}/Misaligned_4mrad_Full/results_* ${OUTDIR1}/${OUTDIR2}/Misaligned_4mrad/.
cp -R ${INDIR}/Misaligned_5mrad_Full/results_* ${OUTDIR1}/${OUTDIR2}/Misaligned_5mrad/.
