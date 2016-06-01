# start of test CBM setups from geometry/setup
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_mc.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_digi_tb.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_digi.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_reco_new.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_qa.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/check_overlaps.C)

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/run)

# Define the number of events to run
Set(NumEvents 2)

# Define the different setups to be tested
If($ENV{ctest_model} MATCHES Nightly)
  List(APPEND cbm_setup sis100_hadron sis100_electron sis100_muon_lmvm sis100_muon_jpsi sis300_electron)
Else()
  List(APPEND cbm_setup sis100_electron)
EndIf()

ForEach(setup IN LISTS cbm_setup)

  Set(testname run_mc_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_mc.sh ${NumEvents} \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  
  Set(dep_mc ${testname})

  Set(testname run_reco_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_reco_new.sh ${NumEvents} \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_mc})
  Set(_FileName_Dependency data/run_mc_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

  Set(dep_reco ${testname})

  Set(testname run_digi_tb_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_digi_tb.sh ${NumEvents} \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "300")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_mc})
  Set(_FileName_Dependency data/run_mc_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

  Set(testname run_digi_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_digi.sh ${NumEvents} \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "300")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_mc})
  Set(_FileName_Dependency data/run_mc_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

  Set(testname run_qa_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_qa.sh ${NumEvents} \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "300")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_reco})
  Set(_FileName_Dependency data/run_reco_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

  Set(testname run_overlap_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/check_overlaps.sh \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_mc})
  Set(_FileName_Dependency data/run_mc_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

EndForEach(setup IN LISTS cbm_setup)
# end of test CBM setups from geometry/setup

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run.C)
add_test(run_run ${CBMROOT_BINARY_DIR}/macro/run/run.sh)
SET_TESTS_PROPERTIES(run_run PROPERTIES TIMEOUT "300")
SET_TESTS_PROPERTIES(run_run PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/checkFields.C)
add_test(run_checkFields ${CBMROOT_BINARY_DIR}/macro/run/checkFields.sh)
SET_TESTS_PROPERTIES(run_checkFields PROPERTIES TIMEOUT "60")

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/checkFieldMap.C)
add_test(run_checkFieldMap ${CBMROOT_BINARY_DIR}/macro/run/checkFieldMap.sh)
SET_TESTS_PROPERTIES(run_checkFieldMap PROPERTIES TIMEOUT "60")

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/checkFieldSym.C)
add_test(run_checkFieldSym ${CBMROOT_BINARY_DIR}/macro/run/checkFieldSym.sh)
SET_TESTS_PROPERTIES(run_checkFieldSym PROPERTIES TIMEOUT "60")

Install(FILES .rootrc run_mc.C run_digi_tb.C run_reco.C run_qa.C check_overlaps.C 
              run_digi.C run.C checkFields.C checkFieldMap.C checkFieldSym.C
        DESTINATION share/cbmroot/macro/run
       )
Install(DIRECTORY modules DESTINATION share/cbmroot/macro/run)
Install(CODE "FILE(MAKE_DIRECTORY \${CMAKE_INSTALL_PREFIX}/share/cbmroot/macro/run/data)")
