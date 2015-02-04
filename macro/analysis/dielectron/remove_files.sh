#!/bin/bash

export DIR=/hera/cbm/users/slebedev/mc/dielectron/dec14/8gev/stsv14_2cm/richv14a/trd4/tofv13/1.0field/nomvd/

function remove_func {
   echo ${DIR}/${1}
   rm -r -f ${DIR}/${1}/log/*.log
   
   rm -r -f ${DIR}/${1}/analysis.*.root
   rm -r -f ${DIR}/${1}/FairRunInfo_analysis.*.root
   
 #  rm -r -f ${DIR}/${1}/reco.*.root
   rm -r -f ${DIR}/${1}/litqa.*.root
  # rm -r ${DIR}/${1}/FairRunInfo_reco.*.root
   
  # rm -r -f ${DIR}/${1}/litqa.*.root    
  # rm -r ${DIR}/${1}/FairRunInfo_litqa.*.root 
  # rm -r ${DIR}/${1}/reco.delta.0*.root
  # rm -r ${DIR}/${1}/mc.0*.root
}

remove_func phi
remove_func rho0
remove_func omegadalitz
remove_func omegaepem
#remove_func urqmd


