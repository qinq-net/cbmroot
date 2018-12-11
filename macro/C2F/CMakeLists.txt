# start of test CBM setups from geometry/setup
Set(MACRO_DIR ${CBMROOT_BINARY_DIR}/macro/C2F)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/C2F/c2f_transport.C)
GENERATE_CBM_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_digi.C ${MACRO_DIR})
GENERATE_CBM_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/run/run_reco_event.C ${MACRO_DIR})
GENERATE_CBM_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/analysis/flow/DataTreeCbmInterface/macro/run_treemaker.C ${MACRO_DIR})

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/C2F)

# Define the number of events to run
Set(NumEvents 10)

# Define the different setups to be tested
If($ENV{ctest_model} MATCHES Nightly)
  List(APPEND cbm_setup sis100_electron)

  ForEach(setup IN LISTS cbm_setup)

    # The test dependencies are set such that parallel execution is prevented, even is there is no logical
    # dependence. The reason is that all runs (for a given setup) use the same ROOT parameter file,
    # and concurrent access to this file results in erros.
    # The logical dependences are expressed in the required "ok" files.

    # --- Transport run, using run_transport.C
    Set(testname c2f_transport_${setup})
    Add_Test(${testname} ${MACRO_DIR}/c2f_transport.sh ${NumEvents} \"${setup}\" \"data/${setup}_test\" \"\" \"TGeant4\" false)
    Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "3600")
    Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
    Set(fixture_c2f_digi fixture_c2f_digi_${testname})
    set_tests_properties(${testname} PROPERTIES FIXTURES_SETUP ${fixture_c2f_digi})
  
    # --- Digitisation run, event-by-event
    Set(testname c2f_digi_event_${setup})
    Add_Test(${testname} ${MACRO_DIR}/run_digi.sh ${NumEvents} \"data/${setup}_test\" 0. 0. 1)
    Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "300")
    Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
    set_tests_properties(${testname} PROPERTIES FIXTURES_REQUIRED ${fixture_c2f_digi})
    Set(fixture_c2f_reco_event fixture_c2f_reco_event_${testname})
    set_tests_properties(${testname} PROPERTIES FIXTURES_SETUP ${fixture_c2f_reco_event})

    # --- Reconstruction run, event-by-event
    Set(testname c2f_reco_event_${setup})
    Add_Test(${testname} ${MACRO_DIR}/run_reco_event.sh ${NumEvents} \"data/${setup}_test\" \"${setup}\")
    Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
    Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
    set_tests_properties(${testname} PROPERTIES FIXTURES_REQUIRED ${fixture_c2f_reco_event})
    Set(fixture_c2f_ana fixture_c2f_ana_${testname})
    set_tests_properties(${testname} PROPERTIES FIXTURES_SETUP ${fixture_c2f_ana})
  
    # --- DataTree
    Set(testname c2f_treemaker_${setup})
    Add_Test(${testname} ${MACRO_DIR}/run_treemaker.sh ${NumEvents} \"data/${setup}_test\" \"${setup}\")
    Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
    Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
    set_tests_properties(${testname} PROPERTIES FIXTURES_REQUIRED ${fixture_c2f_ana})
    Set(fixture_c2f_datatree fixture_c2f_datatree_${testname})
    set_tests_properties(${testname} PROPERTIES FIXTURES_SETUP ${fixture_c2f_datatree})
  
    # --- DataTreeQA input
#    Set(testname c2f_datatreeqa_input_${setup})
#    Add_Test(${testname} ${CBMROOT_SOURCE_DIR}/external/DataTreeQA/macro/PopulateQAConfigurations.py)
#    Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "300")
#    set_tests_properties(${testname} PROPERTIES FIXTURES_REQUIRED ${fixture_c2f_datatree})
#    Set(fixture_c2f_datatreeqa_input fixture_c2f_datatreeqa_input_${testname})
  
  EndForEach(setup IN LISTS cbm_setup)
  # end of test CBM setups from geometry/setup
EndIf()

Install(FILES .rootrc c2f_transport.C 
        DESTINATION share/cbmroot/macro/c2f
       )
Install(CODE "FILE(MAKE_DIRECTORY \${CMAKE_INSTALL_PREFIX}/share/cbmroot/macro/run/data)")
