# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/macro/beamtime/tests)

# Define the number of events to run
Set(NumEvents 2)

#If($ENV{ctest_model} MATCHES Nightly)
  If(EXISTS ${CBMROOT_SOURCE_DIR}/input/run106_production_sts3exttrig_0000.lmd)
    # STS COSY December 2014 beamtime => LMD
    GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/beamtime/tests/StsCosyDec14.C)
    add_test(StsCosyDec14 ${CBMROOT_BINARY_DIR}/macro/beamtime/tests/StsCosyDec14.sh ${NumEvents} )
    SET_TESTS_PROPERTIES(StsCosyDec14 PROPERTIES TIMEOUT "600")
    SET_TESTS_PROPERTIES(StsCosyDec14 PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
  EndIf()

  If(FairRoot_VERSION VERSION_EQUAL 15.11.2)
    If(EXISTS ${CBMROOT_SOURCE_DIR}/input/CernSps05Mar0041_0000.lmd)
      # TOF CERN February 2015 beamtime => LMD
      GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/beamtime/tests/TofCernFeb15Calib.C)
      add_test(TofCernFeb15Calib ${CBMROOT_BINARY_DIR}/macro/beamtime/tests/TofCernFeb15Calib.sh )
      SET_TESTS_PROPERTIES(TofCernFeb15Calib PROPERTIES TIMEOUT "600")
      SET_TESTS_PROPERTIES(TofCernFeb15Calib PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")

      GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/beamtime/tests/TofCernFeb15Run.C)
      add_test(TofCernFeb15Run ${CBMROOT_BINARY_DIR}/macro/beamtime/tests/TofCernFeb15Run.sh ${NumEvents} )
      SET_TESTS_PROPERTIES(TofCernFeb15Run PROPERTIES TIMEOUT "600")
      SET_TESTS_PROPERTIES(TofCernFeb15Run PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
      SET_TESTS_PROPERTIES(TofCernFeb15Run PROPERTIES DEPENDS TofCernFeb15Calib)
      Set(_FileName_Dependency tests/data/TofCernFeb15Calib_ok)
      Set_Tests_Properties(TofCernFeb15Run PROPERTIES REQUIRED_FILES ${_FileName_Dependency})
    EndIf()

    If(EXISTS ${CBMROOT_SOURCE_DIR}/input/CbmTofSps_01Dec0206_15335020645.hld)
      # TOF CERN November 2015 beamtime => HLD
      GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/beamtime/tests/TofCernNov15Calib.C)
      add_test(TofCernNov15Calib ${CBMROOT_BINARY_DIR}/macro/beamtime/tests/TofCernNov15Calib.sh )
      SET_TESTS_PROPERTIES(TofCernNov15Calib PROPERTIES TIMEOUT "600")
      SET_TESTS_PROPERTIES(TofCernNov15Calib PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
      #SET_TESTS_PROPERTIES(TofCernNov15Calib PROPERTIES DEPENDS TofCernFeb15Run)
      #Set(_FileName_Dependency tests/data/TofCernFeb15Run_ok)
      #Set_Tests_Properties(TofCernNov15Calib PROPERTIES REQUIRED_FILES ${_FileName_Dependency})

      GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/macro/beamtime/tests/TofCernNov15Run.C)
      add_test(TofCernNov15Run ${CBMROOT_BINARY_DIR}/macro/beamtime/tests/TofCernNov15Run.sh ${NumEvents} )
      SET_TESTS_PROPERTIES(TofCernNov15Run PROPERTIES TIMEOUT "600")
      SET_TESTS_PROPERTIES(TofCernNov15Run PROPERTIES PASS_REGULAR_EXPRESSION "Test Passed;All ok")
      SET_TESTS_PROPERTIES(TofCernNov15Run PROPERTIES DEPENDS TofCernNov15Calib)
      Set(_FileName_Dependency tests/data/TofCernNov15Calib_ok)
      Set_Tests_Properties(TofCernNov15Run PROPERTIES REQUIRED_FILES ${_FileName_Dependency})
    EndIf()
  EndIf()
#EndIf()
