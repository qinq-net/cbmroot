#####!/bin/sh

mainDirFrom=/hera/cbm/users/slebedev/data/lmvm/apr16/
mainDirTo=/u/slebedev/Baykal/copytemp/apr16_tofclustering/

copy_func() {
   dirFrom=${1}
   dirTo=${2}
   filename=${3}
   meson=${4}
   mkdir -p ${dirTo}/${meson}/
   cp ${dirFrom}/${meson}/${filename} ${dirTo}/${meson}/
}

copy_all_mesons() {
  copy_func ${1} ${2} ${3} rho0
  copy_func ${1} ${2} ${3} omegaepem
  copy_func ${1} ${2} ${3} omegadalitz
  copy_func ${1} ${2} ${3} phi 
  copy_func ${1} ${2} ${3} inmed
  copy_func ${1} ${2} ${3} qgp
  #copy_func ${1} ${2} ${3} urqmd
}

copy_all_mesons_ana_litqa() {
	subPath=${1}
	fileNameAna=analysis${2}
	copy_all_mesons ${mainDirFrom}/${subPath} ${mainDirTo}/${subPath} ${fileNameAna}
	
	fileNameLitqa=litqa${2}
	copy_all_mesons ${mainDirFrom}/${subPath} ${mainDirTo}/${subPath} ${fileNameLitqa}
}

rm -rf ${mainDirTo}
copy_all_mesons_ana_litqa /8gev/geosetup_v1512_8gev/ .tofclustering.auau.8gev.centr.all.root




