Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/analysis/dielectron

${CBMROOT_SOURCE_DIR}/base 
${CBMROOT_SOURCE_DIR}/base/draw
${CBMROOT_SOURCE_DIR}/base/report
${CBMROOT_SOURCE_DIR}/base/utils

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/mvd
${CBMROOT_SOURCE_DIR}/data/global
${CBMROOT_SOURCE_DIR}/data/rich
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/data/tof

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface 

${CBMROOT_SOURCE_DIR}/L1
${CBMROOT_SOURCE_DIR}/L1/L1Algo
${CBMROOT_SOURCE_DIR}/L1/ParticleFinder 

${CBMROOT_SOURCE_DIR}/rich

${CBMROOT_SOURCE_DIR}/littrack
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR}
)

If(NOT ${ROOT_HAS_VC})
  set(SYSTEM_INCLUDE_DIRECTORIES
      ${SYSTEM_INCLUDE_DIRECTORIES}      
      ${VC_INCLUDE_DIRS}
  )
EndIf()

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
${Vc_LIB_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmAnaDielectronTask.cxx
CbmAnaDielectronTaskDraw.cxx
CbmAnaDielectronTaskDrawAll.cxx
CbmAnaLmvmDrawStudy.cxx
CbmLmvmHist.cxx
CbmAnaDielectronStudyReportAll.cxx
CbmAnaDielectronReports.cxx
CbmHaddBase.cxx
)


IF (SSE_FOUND)
  Message(STATUS "Analysis will be compiled with SSE support")
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-msse -O3")
ELSE (SSE_FOUND)
  MESSAGE(STATUS "Analysis will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-O3")
ENDIF (SSE_FOUND)

ADD_DEFINITIONS(-DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS)


Set(LINKDEF CbmDiElectronAnalysisLinkDef.h)
Set(LIBRARY_NAME CbmDiElectronAnalysis)
Set(DEPENDENCIES
    Littrack KF L1 CbmRich CbmSts CbmBase CbmData)
Set(DEFINITIONS -DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS)

GENERATE_LIBRARY()




