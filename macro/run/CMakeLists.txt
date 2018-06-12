# start of test CBM setups from geometry/setup
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_transport.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_digi.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_reco_event.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/check_overlaps.C)
#GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_qa.C)

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/run)

# Define the number of events to run
Set(NumEvents 3)

# Define the different setups to be tested
If($ENV{ctest_model} MATCHES Nightly)
  List(APPEND cbm_setup sis100_debug sis100_hadron sis100_electron sis100_muon_lmvm sis100_muon_jpsi sis300_electron)
Else()
  List(APPEND cbm_setup sis100_electron sis100_muon_jpsi)
EndIf()

ForEach(setup IN LISTS cbm_setup)

  # The test dependencies are set such that parallel execution is prevented, even is there is no logical
  # dependence. The reason is that all runs (for a given setup) use the same ROOT parameter file,
  # and concurrent access to this file results in erros.
  # The logical dependences are expressed in the required "ok" files.

  # --- Transport run, using run_transport.C
  Set(testname run_transport_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_transport.sh ${NumEvents} \"${setup}\" \"data/${setup}_test\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set(dep_transport ${testname})
  
  # --- Digitisation run, event-by-event
  Set(testname run_digi_event_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_digi.sh ${NumEvents} \"data/${setup}_test\" 0. 0. 1)
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "300")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_transport})
  Set(_FileName_Dependency data/run_transport_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})
  Set(dep_digi_event ${testname})
  
  # --- Reconstruction run, event-by-event
  Set(testname run_reco_event_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_reco_event.sh ${NumEvents} \"data/${setup}_test\" \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_digi_event})
  Set(_FileName_Dependency data/run_digi_event_${setup}_test_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})
  Set(dep_reco_event ${testname})
  
  # --- Digitisation run, time-based
  Set(testname run_digi_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_digi.sh ${NumEvents} \"data/${setup}_test\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "300")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_reco_event})
  Set(_FileName_Dependency data/run_transport_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})
  Set(dep_digi ${testname})
  
  # --- QA run
  # --- I have to check the macro first for being up-to-date
  #Set(testname run_qa_${setup})
  #Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/run_qa.sh ${NumEvents} \"${setup}\")
  #Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "300")
  #Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  #Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_digi})
  #Set(_FileName_Dependency data/run_reco_${setup}_ok)
  #Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})
  #Set(dep_qa ${testname})
  
  # --- Geometry checking; only for nightly test
  If($ENV{ctest_model} MATCHES Nightly)
  	Set(testname run_overlap_${setup})
  	Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/run/check_overlaps.sh \"data/${setup}_test\")
  	Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "900")
  	Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  	Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_digi})
  	Set(_FileName_Dependency data/run_transport_${setup}_ok)
  	Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})
  EndIf()
 
EndForEach(setup IN LISTS cbm_setup)
# end of test CBM setups from geometry/setup

# I will have to check these macros for being up-to-date or necessary
#GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run.C)
#add_test(run_run ${CBMROOT_BINARY_DIR}/macro/run/run.sh)
#SET_TESTS_PROPERTIES(run_run PROPERTIES TIMEOUT "300")
#SET_TESTS_PROPERTIES(run_run PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")

#GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/checkFields.C)
#add_test(run_checkFields ${CBMROOT_BINARY_DIR}/macro/run/checkFields.sh)
#SET_TESTS_PROPERTIES(run_checkFields PROPERTIES TIMEOUT "60")

#GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/checkFieldMap.C)
#add_test(run_checkFieldMap ${CBMROOT_BINARY_DIR}/macro/run/checkFieldMap.sh)
#SET_TESTS_PROPERTIES(run_checkFieldMap PROPERTIES TIMEOUT "60")

#GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/checkFieldSym.C)
#add_test(run_checkFieldSym ${CBMROOT_BINARY_DIR}/macro/run/checkFieldSym.sh)
#SET_TESTS_PROPERTIES(run_checkFieldSym PROPERTIES TIMEOUT "60")


Install(FILES .rootrc run_transport.C run_digi.C run_reco_event.C check_overlaps.C 
        DESTINATION share/cbmroot/macro/run
       )
Install(DIRECTORY modules DESTINATION share/cbmroot/macro/run)
Install(CODE "FILE(MAKE_DIRECTORY \${CMAKE_INSTALL_PREFIX}/share/cbmroot/macro/run/data)")
