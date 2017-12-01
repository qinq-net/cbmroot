# start of test CBM setups from geometry/setup
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_mc.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_reco.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_check_overlaps.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_mc_dev.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/mcbm/mcbm_reco_dev.C)

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/mcbm)

# Define the number of events to run
Set(NumEvents 2)

# Define the different setups to be tested
If($ENV{ctest_model} MATCHES Nightly)
  List(APPEND cbm_setup sis18_mcbm_20deg_long sis18_mcbm_20deg_short sis18_mcbm_25deg)
Else()
  List(APPEND cbm_setup sis18_mcbm_20deg_long)
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


# Define the different configs to be tested
# test all subsystems
#List(APPEND cbm_config 901101110 910000000 901000000 900100000 900010000 900001000 900000100 900000010 900000001)
# test selected subsystems
List(APPEND cbm_config 901101110 901000000 900100000 900001000 900000100 900000010)

ForEach(config IN LISTS cbm_config)

  Set(testname mcbm_mc_${config})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/mcbm/mcbm_mc_dev.sh ${NumEvents} \"sis18_mcbm_20deg_long_dev\" \"${config}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  
  Set(dep_mc ${testname})

  Set(testname mcbm_reco_${config})
  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/mcbm/mcbm_reco_dev.sh ${NumEvents} \"sis18_mcbm_20deg_long_dev\" \"${config}\")
  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_mc})
  Set(_FileName_Dependency data/mcbm_mc_dev_${config}_ok)
  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

  Set(dep_reco ${testname})

#DE  Set(testname mcbm_overlap_${config})
#DE  Add_Test(${testname} ${CBMROOT_BINARY_DIR}/macro/mcbm/mcbm_check_overlaps.sh \"sis18_mcbm_20deg_long_dev\")
#DE  Set_Tests_Properties(${testname} PROPERTIES TIMEOUT "600")
#DE  Set_Tests_Properties(${testname} PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
#DE  Set_Tests_Properties(${testname} PROPERTIES DEPENDS ${dep_mc})
#DE  Set(_FileName_Dependency data/mcbm_mc_${config}_ok)
#DE  Set_Tests_Properties(${testname} PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

EndForEach(config IN LISTS cbm_config)
# end of test CBM configs from geometry/config

Install(FILES .rootrc mcbm_mc.C mcbm_reco.C mcbm_mc_dev.C mcbm_reco_dev.C mcbm_check_overlaps.C 
        DESTINATION share/cbmroot/macro/mcbm
       )
Install(DIRECTORY modules DESTINATION share/cbmroot/macro/mcbm)
Install(CODE "FILE(MAKE_DIRECTORY \${CMAKE_INSTALL_PREFIX}/share/cbmroot/macro/mcbm/data)")
