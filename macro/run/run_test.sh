#!/bin/sh

setup=$1
opt=$2

nevents=1

if [ "${opt}" = "sim" ] ; then
   ${ROOTSYS}/bin/root -b -q -l "${VMCWORKDIR}/macro/run/run_sim.C(${nevents}, \"${setup}\")"
elif [ "${opt}" = "reco" ] ; then
   ${ROOTSYS}/bin/root -b -q -l "${VMCWORKDIR}/macro/run/run_reco.C(${nevents}, \"${setup}\")"
elif [ "${opt}" = "digi" ] ; then
   ${ROOTSYS}/bin/root -b -q -l "${VMCWORKDIR}/macro/run/run_digi.C(${nevents}, \"${setup}\")"
elif [ "${opt}" = "qa" ] ; then
   ${ROOTSYS}/bin/root -b -q -l "${VMCWORKDIR}/macro/run/run_qa.C(${nevents}, \"${setup}\")"
fi
