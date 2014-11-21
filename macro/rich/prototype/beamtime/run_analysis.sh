#!/bin/sh

#Macro will be running via script
export SCRIPT=yes

#Build directory of CBMROOT
export MY_BUILD_DIR=/Users/slebedev/Development/cbm/trunk/build
cd $MY_BUILD_DIR
   . ./config.sh
   cd -

analyze_run_files() {
   export OUTPUT_ROOT_FILE_ALL=${1}
   export OUTPUT_DIR=${2}
   export RUN_TITLE=${3}
   shift
   shift
   shift
   filesArray=("${@}")

   ROOT_FILES=
   SPACE=" "
   for FF in "${filesArray[@]}"; do
      export INPUT_HLD_FILE=${FF}
      export OUTPUT_ROOT_FILE=${FF}.root
      rm -rf ${OUTPUT_ROOT_FILE}
      ROOT_FILES=${ROOT_FILES}${SPACE}${OUTPUT_ROOT_FILE}
      echo ${ROOT_FILES}
      echo ${INPUT_HLD_FILE}
      echo ${OUTPUT_ROOT_FILE}
      
      root -b -q -l "./run_analysis.C()"
   done
   
   hadd -f -T ${OUTPUT_ROOT_FILE_ALL} ${ROOT_FILES}
   rm -rf ${ROOT_FILES}
   
   root -b -q -l "./draw_analysis.C()"
}

analyze_run_directory() {
   filesArray=( ${4}/*.hld )
   analyze_run_files ${1} ${2} ${3} ${filesArray[@]}
}

#filesArray=( te14324152650.hld te14324200946.hld )
#analyze_run_files all_test_file.root recoRR/ testRun ${filesArray[@]}

analyze_run_directory all_test_file.root recoRR/ testRun .


export SCRIPT=no
