# Create a library called "libCbmBase" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR}
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/base 
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/sts
  ${CBMROOT_SOURCE_DIR}/data/tof
  ${CBMROOT_SOURCE_DIR}/data/rich
  # needed by rootcling
  ${CBMROOT_SOURCE_DIR}/base/report
)
Include_Directories( ${INCLUDE_DIRECTORIES})

Set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
#  ${CMAKE_SOURCE_DIR}/base
)
 
link_directories( ${LINK_DIRECTORIES})

Set(SRCS
CbmAddress.cxx
CbmModuleList.cxx

#CbmDigiManager.cxx
CbmPropagator.cxx

CbmTrdTrackFinder.cxx  
CbmTrdTrackFitter.cxx  

CbmTofTrackFinder.cxx
CbmTofTrackFitter.cxx

CbmStsTrackFinder.cxx 
CbmStsTrackFitter.cxx 

CbmMuchTrackFinder.cxx

CbmRichRingFinder.cxx 
CbmRichRingFitter.cxx 
CbmRichTrackExtrapolation.cxx 
CbmRichRingTrackAssign.cxx 
CbmRichRingSelect.cxx 

CbmGlobalTrackFitter.cxx
CbmPrimaryVertexFinder.cxx
CbmRichMerger.cxx
CbmTofMerger.cxx
CbmTrackMerger.cxx

CbmMCMatch.cxx
CbmMCMatchCreatorTask.cxx
CbmMCMatchLoaderTask.cxx
CbmMCMatchSelectorTask.cxx
CbmMCObject.cxx
CbmMCStage.cxx
CbmMCResult.cxx
#CbmMCTestMomentumCompare.cxx
CbmMCTestHitCompare.cxx
CbmMCEntry.cxx
CbmMCList.cxx
CbmMCDataArray.cxx
CbmMCDataManager.cxx

CbmModule.cxx

CbmHistManager.cxx
draw/CbmDrawHist.cxx
report/CbmReport.cxx      
report/CbmStudyReport.cxx
report/CbmSimulationReport.cxx
report/CbmTextReportElement.cxx
report/CbmHtmlReportElement.cxx       
report/CbmLatexReportElement.cxx

utils/CbmUtils.cxx

CbmMatchRecoToMC.cxx
)

Execute_Process(COMMAND ${ROOT_CONFIG_EXECUTABLE} --has-gdml
                OUTPUT_VARIABLE ROOT_HAS_GDML 
)

If ( ${ROOT_HAS_GDML} STREQUAL yes )
  Message(STATUS "Build CbmModule with Gdml support")
  Add_Definitions(-DROOT_HAS_GDML)
EndIf ( ${ROOT_HAS_GDML} STREQUAL yes )



Set(LINKDEF CbmBaseLinkDef.h)
Set(LIBRARY_NAME CbmBase)
Set(DEPENDENCIES
    CbmData Base boost_regex
)

If ( ${ROOT_HAS_GDML} STREQUAL yes )
 Set (DEPENDENCIES ${DEPENDENCIES} Gdml)
EndIf ( ${ROOT_HAS_GDML} STREQUAL yes )

GENERATE_LIBRARY()


