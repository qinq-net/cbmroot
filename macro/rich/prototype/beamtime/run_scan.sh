#!/bin/bash

export SCRIPT=yes

#DIR=/mnt/data/WLS/WLS_on/stretched/ringB3

for DIR in /mnt/data/WLS/WLS_on/stretched/*
do
   DIR2=${DIR}/offset00100

   for FILE in ${DIR2}/*
   do

      echo ${FILE}
      PNGFILENAME=${FILE#"${DIR2}/"}
#      echo ${PNGFILENAME}.png

      export INPUT_HLD_FILE=${FILE}
      export OUTPUT_ROOT_FILE="out.root"
      root -l -b -q "./run_analysis.C"
      mv out.png ${PNGFILENAME}.png

      break

   done

done
