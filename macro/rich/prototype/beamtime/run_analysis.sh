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

#example how to run analysis for single (or multiple files)
#filesArray=( file1.hld file2.hld file3.hld file4.hld)
#analyze_run_files outputRootFileWithHist.root outputDirWithImages/ titleOfAnalysis ${filesArray[@]}

analyze_run_files wls_off_ring_a1.root wls_off_ring_a1/ wls_off_ring_a1 wls_off_ring_a1.hld
analyze_run_files wls_off_ring_b1.root wls_off_ring_b1/ wls_off_ring_b1 wls_off_ring_b1.hld
analyze_run_files wls_off_ring_b2.root wls_off_ring_b2/ wls_off_ring_b2 wls_off_ring_b2.hld
analyze_run_files wls_off_ring_b3.root wls_off_ring_b3/ wls_off_ring_b3 wls_off_ring_b3.hld
analyze_run_files wls_off_ring_d1.root wls_off_ring_d1/ wls_off_ring_d1 wls_off_ring_d1.hld
analyze_run_files wls_off_ring_dg1.root wls_off_ring_dg1/ wls_off_ring_dg1 wls_off_ring_dg1.hld
analyze_run_files wls_off_ring_g1.root wls_off_ring_g1/ wls_off_ring_g1 wls_off_ring_g1.hld
analyze_run_files wls_off_ring_gh1.root wls_off_ring_gh1/ wls_off_ring_gh1 wls_off_ring_gh1.hld
analyze_run_files wls_off_ring_h1.root wls_off_ring_h1/ wls_off_ring_h1 wls_off_ring_h1.hld
analyze_run_files wls_off_ring_h2.root wls_off_ring_h2/ wls_off_ring_h2 wls_off_ring_h2.hld


#example how to run analysis for all .hld files in directory
#analyze_run_directory outputRootFileWithHist.root outputDirWithImages/ titleOfAnalysis directoryWithHldFiles/

#analyze_run_directory all_test_file.root recoRR/ testRun dirWithHld


export SCRIPT=no
