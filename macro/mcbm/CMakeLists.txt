# start of test CBM setups from geometry/setup
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_mc.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_reco.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_check_overlaps.C)

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/mcbm)

# Define the number of events to run
Set(NumEvents 2)

# Define the different setups to be tested
If($ENV{ctest_model} MATCHES Nightly)
  List(APPEND cbm_setup setup_sis18_mcbm_20deg_long setup_sis18_mcbm_20deg_short setup_sis18_mcbm_25deg)
Else()
  List(APPEND cbm_setup setup_sis18_mcbm_20deg_long)
EndIf()

ForEach(setup IN LISTS cbm_setup)

  Set(testname mcbm_mc_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/mcbm/mcbm_mc.sh ${NumEvents} \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  
  Set(dep_mc ${testname})

  Set(testname mcbm_reco_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/mcbm/mcbm_reco.sh ${NumEvents} \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_mc})
  Set(_FileName_Dependency data/mcbm_mc_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

  Set(dep_reco ${testname})

  Set(testname mcbm_overlap_${setup})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/mcbm/mcbm_check_overlaps.sh \"${setup}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_mc})
  Set(_FileName_Dependency data/mcbm_mc_${setup}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

EndForEach(setup IN LISTS cbm_setup)
# end of test CBM setups from geometry/setup

Install(FILES .rootrc mcbm_mc.C mcbm_reco.C mcbm_check_overlaps.C 
        DESTINATION share/cbmroot/macro/mcbm
       )
Install(DIRECTORY modules DESTINATION share/cbmroot/macro/mcbm)
Install(CODE "FILE(MAKE_DIRECTORY \${CMAKE_INSTALL_PREFIX}/share/cbmroot/macro/mcbm/data)")
