GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/test/cbmdata/TestCbmHit.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/test/cbmdata/TestCbmPixelHit.C)

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/test/cbmdata)

#Add_Test(TestCbmHit ${CBMROOT_BINARY_DIR}/test/cbmdata/TestCbmHit.sh)
#Add_Test(TestCbmPixelHit ${CBMROOT_BINARY_DIR}/test/cbmdata/TestCbmPixelHit.sh)

Macro(CreateGTestExeAndAddTest _testname _includeDirs _linkDirs _sources _dependencies _testdepend)

  Include_Directories(SYSTEM "${_includeDirs}")
  Link_Directories(${_linkDirs})

  Add_Executable(${_testname} ${_sources})
  Target_Link_Libraries(${_testname} "${_dependencies}")
  Gen_Exe_Script(${_testname})
  string(REPLACE ${PROJECT_SOURCE_DIR}
         ${PROJECT_BINARY_DIR} new_path ${CMAKE_CURRENT_SOURCE_DIR}
        )
  Add_Test(${_testname} ${new_path}/${_testname}.sh)
  If(NOT ${_testdepend} STREQUAL "")
    Set_Tests_Properties(${_testname} PROPERTIES DEPENDS ${_testdepend})
  EndIf()  
EndMacro(CreateGTestExeAndAddTest)

Set(INCLUDE_DIRECTORIES
  ${CMAKE_SOURCE_DIR}/base
  ${CMAKE_SOURCE_DIR}/data
  ${CMAKE_SOURCE_DIR}/data/global
  ${BASE_INCLUDE_DIRECTORIES} 
)

MESSAGE("FAIRROOT_LIBRARY_DIR: ${FAIRROOT_LIBRARY_DIR}")
Set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
)


Set(DEPENDENCIES
  ${ROOT_LIBRARIES} 
  ${GTEST_BOTH_LIBRARIES}
  FairTools
  CbmData
  Base
  ParBase
  GeoBase
  MbsAPI
)

if (CMAKE_SYSTEM_NAME MATCHES Linux)
  set(DEPENDENCIES ${DEPENDENCIES} rt)
endif()

Set(CbmLinkSources
  ${CMAKE_SOURCE_DIR}/data/CbmLink.cxx
  _GTestCbmLink.cxx
)

CreateGTestExeAndAddTest(_GTestCbmLink "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmLinkSources}" "${DEPENDENCIES}" "")

Set(CbmMatchSources
  ${CMAKE_SOURCE_DIR}/data/CbmMatch.cxx
  _GTestCbmMatch.cxx
)

CreateGTestExeAndAddTest(_GTestCbmMatch "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmMatchSources}" "${DEPENDENCIES}" "_GTestCbmLink")

Set(CbmDigiSources
  ${CMAKE_SOURCE_DIR}/data/CbmDigi.cxx
  _GTestCbmDigi.cxx
)

CreateGTestExeAndAddTest(_GTestCbmDigi "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmDigiSources}" "${DEPENDENCIES}" "_GTestCbmMatch")

Set(CbmClusterSources
  ${CMAKE_SOURCE_DIR}/data/CbmCluster.cxx
  _GTestCbmCluster.cxx
)

CreateGTestExeAndAddTest(_GTestCbmCluster "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmClusterSources}" "${DEPENDENCIES}" "_GTestCbmDigi")

Set(CbmHitSources
  ${CMAKE_SOURCE_DIR}/data/CbmHit.cxx
  _GTestCbmHit.cxx
)  

CreateGTestExeAndAddTest(_GTestCbmHit "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmHitSources}" "${DEPENDENCIES}" "_GTestCbmCluster")

Set(CbmPixelHitSources
  ${CMAKE_SOURCE_DIR}/data/CbmPixelHit.cxx
  _GTestCbmPixelHit.cxx
)  

CreateGTestExeAndAddTest(_GTestCbmPixelHit "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmPixelHitSources}" "${DEPENDENCIES}" "_GTestCbmHit")

Set(CbmStripHitSources
  ${CMAKE_SOURCE_DIR}/data/CbmStripHit.cxx
  _GTestCbmStripHit.cxx
)  

CreateGTestExeAndAddTest(_GTestCbmStripHit "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmStripHitSources}" "${DEPENDENCIES}" "_GTestCbmPixelHit")


Set(CbmEventSources
  ${CMAKE_SOURCE_DIR}/data/CbmEvent.cxx
  _GTestCbmEvent.cxx
)  

CreateGTestExeAndAddTest(_GTestCbmEvent "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmEventSources}" "${DEPENDENCIES}" "_GTestCbmPixelHit")


Set(CbmConstantsSources
  _GTestCbmConstants.cxx
)  

CreateGTestExeAndAddTest(_GTestCbmConstants "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmConstantsSources}" "${GTEST_BOTH_LIBRARIES};-pthread" "_GTestCbmEvent")

#Set(CbmTrackSources
#  ${CMAKE_SOURCE_DIR}/data/CbmTrack.cxx
#  _GTestCbmTrack.cxx
#)  

#CreateGTestExeAndAddTest(_GTestCbmTrack "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
#                         "${CbmTrackSources}" "${DEPENDENCIES}" "_GTestCbmConstants")

Set(CbmVertexSources
  ${CMAKE_SOURCE_DIR}/data/global/CbmVertex.cxx
  global/_GTestCbmVertex.cxx
)  

CreateGTestExeAndAddTest(_GTestCbmVertex "${INCLUDE_DIRECTORIES}" "${LINK_DIRECTORIES}"
                         "${CbmVertexSources}" "${DEPENDENCIES}" "_GTestCbmConstants")
#                         "${CbmVertexSources}" "${DEPENDENCIES}" "_GTestCbmTrack")
                         

#GENERATE_ROOT_TEST_SCRIPT(${CMAKE_SOURCE_DIR}/test/cbmdata/load_all_libs.C)
#  Add_Test(load_all_libs
#          ${CMAKE_BINARY_DIR}/test/cbmdata/load_all_libs.sh)
#  Set_Tests_Properties(load_all_libs PROPERTIES TIMEOUT "30")
#  Set_Tests_Properties(load_all_libs PROPERTIES PASS_REGULAR_EXPRESSION "Macro finished succesfully")
                         