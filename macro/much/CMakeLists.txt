# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/much)

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/much/much_sim.C)
add_test(much_sim ${CBMROOT_BINARY_DIR}/macro/much/much_sim.sh)
SET_TESTS_PROPERTIES(much_sim PROPERTIES TIMEOUT "450")
SET_TESTS_PROPERTIES(much_sim PROPERTIES PASS_REGULAR_EXPRESSION "TestPassed;All ok")
set_tests_properties(much_sim PROPERTIES FIXTURES_SETUP fixture_much_seg)

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/much/much_seg_sector.C)
add_test(much_seg ${CBMROOT_BINARY_DIR}/macro/much/much_seg_sector.sh)
SET_TESTS_PROPERTIES(much_seg PROPERTIES TIMEOUT "450")
SET_TESTS_PROPERTIES(much_seg PROPERTIES PASS_REGULAR_EXPRESSION "TestPassed;All ok")
set_tests_properties(much_seg PROPERTIES FIXTURES_REQUIRED fixture_much_seg)
set_tests_properties(much_seg PROPERTIES FIXTURES_SETUP fixture_much_digi)

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/much/much_digi.C)
add_test(much_digi ${CBMROOT_BINARY_DIR}/macro/much/much_digi.sh)
SET_TESTS_PROPERTIES(much_digi PROPERTIES TIMEOUT "450")
SET_TESTS_PROPERTIES(much_digi PROPERTIES PASS_REGULAR_EXPRESSION "TestPassed;All ok")
set_tests_properties(much_digi PROPERTIES FIXTURES_REQUIRED fixture_much_digi)
set_tests_properties(much_digi PROPERTIES FIXTURES_SETUP fixture_much_reco)

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/much/much_reco.C)
add_test(much_reco ${CBMROOT_BINARY_DIR}/macro/much/much_reco.sh)
SET_TESTS_PROPERTIES(much_reco PROPERTIES TIMEOUT "600")
SET_TESTS_PROPERTIES(much_reco PROPERTIES PASS_REGULAR_EXPRESSION "TestPassed;All ok")
set_tests_properties(much_reco PROPERTIES FIXTURES_REQUIRED fixture_much_reco)


