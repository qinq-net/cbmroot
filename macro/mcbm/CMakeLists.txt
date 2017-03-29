#
##   mcbm test routines
#

# added 30.03.2017
# copied from macro/trd
# by David Emschermann

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/mcbm)

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_sim.C)
add_test(mcbm_sim ${CBMROOT_BINARY_DIR}/macro/mcbm/mcbm_sim.sh)
SET_TESTS_PROPERTIES(mcbm_sim PROPERTIES TIMEOUT "600")
SET_TESTS_PROPERTIES(mcbm_sim PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")

GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_reco.C)
add_test(mcbm_reco ${CBMROOT_BINARY_DIR}/macro/mcbm/mcbm_reco.sh)
SET_TESTS_PROPERTIES(mcbm_reco PROPERTIES TIMEOUT "600")
SET_TESTS_PROPERTIES(mcbm_reco PROPERTIES DEPENDS mcbm_sim)
SET_TESTS_PROPERTIES(mcbm_reco PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
Set(_FileName_Dependency data/mcbm_sim_sis18_mcbm_ok)
Set_Tests_Properties(mcbm_reco PROPERTIES REQUIRED_FILES ${_FileName_Dependency})
