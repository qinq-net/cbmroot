#####!/bin/sh

subPath=/8gev/stsv14_4cm/richv14a/trd4/tofv13/1.0field/nomvd/

dirFrom=/hera/cbm/users/slebedev/mc/dielectron/dec14/${subPath}
dirTo=/u/slebedev/Baykal/copytemp/dec14/${subPath}
filename=litqa.auau.8gev.centr.all.root

function copy_func() {
   mkdir -p ${dirTo}/${1}/
   cp ${dirFrom}/${1}/${filename} ${dirTo}/${1}/
}

function copy_all() {
   copy_func rho0
  # copy_func omegaepem
  # copy_func omegadalitz
  # copy_func phi 
  # copy_func urqmd
}

copy_all

