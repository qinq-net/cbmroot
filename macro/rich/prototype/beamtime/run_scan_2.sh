#!/bin/bash

export SCRIPT=yes

DIR="/mnt/data/WLS/WLS_on/stretched/ringB1"
DIR2="${DIR}/offset00100"

FILE="${DIR2}/te14324021749.hld"

   echo ${FILE}

   export INPUT_HLD_FILE=${FILE}
   export OUTPUT_ROOT_FILE="out.root"
   root -l -b -q "./run_analysis.C"
   PNGFILENAME=${FILE#"${DIR2}/"}
   cp out.png ${PNGFILENAME}.png

FILE="${DIR2}/te14324021920.hld"

   echo ${FILE}

   export INPUT_HLD_FILE=${FILE}
   export OUTPUT_ROOT_FILE="out.root"
   root -l -b -q "./run_analysis.C"
   PNGFILENAME=${FILE#"${DIR2}/"}
   cp out.png ${PNGFILENAME}.png

