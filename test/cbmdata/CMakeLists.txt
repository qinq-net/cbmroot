GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/test/cbmdata/TestCbmHit.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/test/cbmdata/TestCbmPixelHit.C)

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/test/cbmdata)

#Add_Test(TestCbmHit ${CBMROOT_BINARY_DIR}/test/cbmdata/TestCbmHit.sh)
#Add_Test(TestCbmPixelHit ${CBMROOT_BINARY_DIR}/test/cbmdata/TestCbmPixelHit.sh)

Macro(CreateGTestExeAndAddTest _testname _includeDirs _linkDirs _sources _dependencies)

  Include_Directories(SYSTEM "${_includeDirs}")
  Link_Directories("${_linkDirs}")

  Add_Executable(${_testname} "${_sources}")
  Target_Link_Libraries(${_testname} "${_dependencies}")
  Add_Test(${_testname} ${PROJECT_BINARY_DIR}/bin/${_testname})

EndMacro(CreateGTestExeAndAddTest)

Set(INCLUDE_DIRECTORIES
  ${CMAKE_SOURCE_DIR}/data
  ${BASE_INCLUDE_DIRECTORIES} 
)

Set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
)

Set(DEPENDENCIES
  ${ROOT_LIBRARIES} 
  ${GTEST_BOTH_LIBRARIES}
  CbmData
)

Set(CbmLinkSources
  ${CMAKE_SOURCE_DIR}/data/CbmLink.cxx
  _GTestCbmLink.cxx
)

CreateGTestExeAndAddTest(_GTestCbmLink "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmLinkSources}" "${DEPENDENCIES}")

Set(CbmMatchSources
  ${CMAKE_SOURCE_DIR}/data/CbmMatch.cxx
  _GTestCbmMatch.cxx
)

CreateGTestExeAndAddTest(_GTestCbmMatch "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmMatchSources}" "${DEPENDENCIES}")

Set(CbmDigiSources
  ${CMAKE_SOURCE_DIR}/data/CbmDigi.cxx
  _GTestCbmDigi.cxx
)

CreateGTestExeAndAddTest(_GTestCbmDigi "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmDigiSources}" "${DEPENDENCIES}")

Set(CbmHitSources
  ${CMAKE_SOURCE_DIR}/data/CbmHit.cxx
  _GTestCbmHit.cxx
)  

CreateGTestExeAndAddTest(_GTestCbmHit "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmHitSources}" "${DEPENDENCIES}")

