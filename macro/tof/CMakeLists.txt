#TODO: Test the different tof geometries

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/tof/digitizer)

# Define the number of events to run
Set(NumEvents 2)

# Define the setup to run
Set(setup sis100_electron)

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/tof/digitizer/run_tof_disim.C)
add_test(tof_disim ${CBMROOT_BINARY_DIR}/macro/tof/digitizer/run_tof_disim.sh ${NumEvents} \"${setup}\")
SET_TESTS_PROPERTIES(tof_disim PROPERTIES TIMEOUT "600")
SET_TESTS_PROPERTIES(tof_disim PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/tof/digitizer/run_digi_test.C)
add_test(tof_digi1 ${CBMROOT_BINARY_DIR}/macro/tof/digitizer/run_digi_test.sh ${NumEvents} \"${setup}\")
SET_TESTS_PROPERTIES(tof_digi1 PROPERTIES TIMEOUT "300")
SET_TESTS_PROPERTIES(tof_digi1 PROPERTIES DEPENDS tof_disim)
SET_TESTS_PROPERTIES(tof_digi1 PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
Set(_FileName_Dependency data/run_tof_disim_${setup}_ok)
Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})


### To do later: run with different digitizer/clusterizer options
#GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/tof/digitizer/tof_digi2.C)
#add_test(tof_digi2 ${CBMROOT_BINARY_DIR}/macro/tof/digitizer/tof_digi2.sh)
#SET_TESTS_PROPERTIES(tof_digi2 PROPERTIES TIMEOUT "300")
#SET_TESTS_PROPERTIES(tof_digi2 PROPERTIES DEPENDS tof_disim)
#SET_TESTS_PROPERTIES(tof_digi2 PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")


