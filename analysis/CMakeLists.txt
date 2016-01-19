# add subdirectories
add_subdirectory (papa)
add_subdirectory (much)
add_subdirectory (flow)
#add_subdirectory (hyperon)

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/analysis
#${CBMROOT_SOURCE_DIR}/analysis/hyperon
${CBMROOT_SOURCE_DIR}/analysis/dielectron
${CBMROOT_SOURCE_DIR}/analysis/jpsiana

${CBMROOT_SOURCE_DIR}/base 
${CBMROOT_SOURCE_DIR}/base/draw
${CBMROOT_SOURCE_DIR}/base/report
${CBMROOT_SOURCE_DIR}/base/utils

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/much
${CBMROOT_SOURCE_DIR}/data/mvd
${CBMROOT_SOURCE_DIR}/data/global
${CBMROOT_SOURCE_DIR}/data/rich
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/data/tof
${CBMROOT_SOURCE_DIR}/data/psd

${CBMROOT_SOURCE_DIR}/sts 

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface 
${CBMROOT_SOURCE_DIR}/KF/KFQA
${CBMROOT_SOURCE_DIR}/KF/KFParticle

${CBMROOT_SOURCE_DIR}/L1
${CBMROOT_SOURCE_DIR}/L1/L1Algo
${CBMROOT_SOURCE_DIR}/L1/ParticleFinder 

${CBMROOT_SOURCE_DIR}/much/geo

${CBMROOT_SOURCE_DIR}/rich

${CBMROOT_SOURCE_DIR}/littrack

${CBMROOT_SOURCE_DIR}/generators
#${CBMROOT_SOURCE_DIR}/analysis/flow

#needed by rootcling
#${CBMROOT_SOURCE_DIR}/analysis/dielectron
${CBMROOT_SOURCE_DIR}/analysis/conversion
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
)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS
#CbmD0Analysis.cxx
#V0/CbmV0Analysis.cxx

#msh/CbmMSHConverter.cxx
#msh/CbmMSHAnalysis.cxx

#timing/CbmAnaTimingAnalysis.cxx 
#timing/CbmAnaEpochTest.cxx
#timing/CbmAnaTimingMuchDigitizer.cxx
#timing/CbmAnaTimingMuchHitFinder.cxx
#timing/CbmAnaTimingStsDigitizer.cxx
#timing/CbmAnaTimingStsHitFinder.cxx

dielectron/CbmAnaDielectronTask.cxx
dielectron/CbmAnaDielectronTaskDraw.cxx
dielectron/CbmAnaDielectronTaskDrawAll.cxx
dielectron/CbmAnaLmvmDrawStudy.cxx
dielectron/CbmLmvmHist.cxx
dielectron/CbmAnaDielectronStudyReportAll.cxx
dielectron/CbmAnaDielectronReports.cxx
dielectron/CbmHaddBase.cxx

jpsiana/CbmAnaJpsiTask.cxx
jpsiana/CbmAnaJpsiReport.cxx
jpsiana/CbmAnaJpsiHist.cxx
jpsiana/CbmAnaJpsiCandidate.cxx
jpsiana/CbmAnaJpsiSuperEvent.cxx
jpsiana/CbmAnaJpsiSuperEventReport.cxx

conversion/CbmAnaConversion.cxx
conversion/CbmAnaConversionTomography.cxx
conversion/CbmAnaConversionRich.cxx
#conversion/CbmAnaConversionKF.cxx
conversion/CbmAnaConversionReco.cxx
conversion/CbmAnaConversionPhotons.cxx
conversion/CbmAnaConversionPhotons2.cxx
conversion/CbmAnaConversionRecoFull.cxx
conversion/CbmAnaConversionTest2.cxx

#photon/CbmTrdPhotonAnalysis.cxx
)


Set(HEADERS )

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


Set(LINKDEF CbmAnalysisLinkDef.h)
Set(LIBRARY_NAME CbmAnalysis)
Set(DEPENDENCIES
    Littrack KF L1 CbmMuch CbmRich CbmSts CbmBase CbmData CbmGenerators Base Vc.a)
Set(DEFINITIONS -DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS)

GENERATE_LIBRARY()




