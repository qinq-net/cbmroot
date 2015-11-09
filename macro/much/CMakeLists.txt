# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/much)
File(COPY ${CBMROOT_SOURCE_DIR}/macro/much/data/much_digi_sector.seg DESTINATION ${CBMROOT_BINARY_DIR}/macro/much/data)

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/much/much_sim.C)
add_test(much_sim ${CBMROOT_BINARY_DIR}/macro/much/much_sim.sh)
SET_TESTS_PROPERTIES(much_sim PROPERTIES TIMEOUT "450")
SET_TESTS_PROPERTIES(much_sim PROPERTIES PASS_REGULAR_EXPRESSION "TestPassed;All ok")

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/much/much_seg_sector.C)
add_test(much_seg ${CBMROOT_BINARY_DIR}/macro/much/much_seg_sector.sh)
SET_TESTS_PROPERTIES(much_seg PROPERTIES TIMEOUT "450")
SET_TESTS_PROPERTIES(much_seg PROPERTIES DEPENDS much_sim)
SET_TESTS_PROPERTIES(much_seg PROPERTIES PASS_REGULAR_EXPRESSION "TestPassed;All ok")
Set(_FileName_Dependency data/much_sim_ok) 
Set_Tests_Properties(much_seg PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/much/much_reco.C)
add_test(much_reco ${CBMROOT_BINARY_DIR}/macro/much/much_reco.sh)
SET_TESTS_PROPERTIES(much_reco PROPERTIES TIMEOUT "600")
SET_TESTS_PROPERTIES(much_reco PROPERTIES DEPENDS much_seg)
SET_TESTS_PROPERTIES(much_reco PROPERTIES PASS_REGULAR_EXPRESSION "TestPassed;All ok")
Set(_FileName_Dependency data/much_seg_ok) 
Set_Tests_Properties(much_reco PROPERTIES REQUIRED_FILES ${_FileName_Dependency})



