Set(INCLUDE_DIRECTORIES

${CBMROOT_SOURCE_DIR}/analysis
${CBMROOT_SOURCE_DIR}/analysis/papa

${CBMROOT_SOURCE_DIR}/generators

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
${CBMROOT_SOURCE_DIR}/rich 

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface 
${CBMROOT_SOURCE_DIR}/KF/KFQA
${CBMROOT_SOURCE_DIR}/KF/KFParticle

${CBMROOT_SOURCE_DIR}/L1
${CBMROOT_SOURCE_DIR}/L1/ParticleFinder
${CBMROOT_SOURCE_DIR}/L1/L1Algo
##${CBMROOT_SOURCE_DIR}/L1/vectors

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

#Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Weffc++ -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-ignored-qualifiers -Wno-overloaded-virtual")

link_directories( ${LINK_DIRECTORIES})

set(SRCS

AnalysisCuts.cxx
AnalysisFilter.cxx
PairAnalysisCutGroup.cxx
PairAnalysisTrack.cxx
PairAnalysisEvent.cxx
PairAnalysisPair.cxx
#PairAnalysisPairKF.cxx
PairAnalysisPairLV.cxx
PairAnalysisCutQA.cxx
PairAnalysisPairLegCuts.cxx
PairAnalysisVarCuts.cxx
PairAnalysisHelper.cxx
PairAnalysisMetaData.cxx
PairAnalysisStyler.cxx
PairAnalysisVarManager.cxx

AnalysisTaskMultiPairAnalysis.cxx
PairAnalysis.cxx
#PairAnalysisDebugTree.cxx
#PairAnalysisMixedEvent.cxx
PairAnalysisHF.cxx
###PairAnalysisHFhelper.cxx
PairAnalysisHistos.cxx
PairAnalysisMC.cxx

PairAnalysisVarCutsCombi.cxx
PairAnalysisSignalMC.cxx
#PairAnalysisSpectrum.cxx
PairAnalysisTrackRotator.cxx
PairAnalysisMixedEvent.cxx
PairAnalysisMixingHandler.cxx

PairAnalysisFunction.cxx
PairAnalysisSignalBase.cxx
PairAnalysisSignalExt.cxx
PairAnalysisSignalFit.cxx
)


Set(HEADERS )

IF (SSE_FOUND)
  Message(STATUS "Papa will be compiled with SSE support")
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-msse -O3")
ELSE (SSE_FOUND)
  MESSAGE(STATUS "Papa will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-O3")
ENDIF (SSE_FOUND)

If(CMAKE_SYSTEM_NAME MATCHES Darwin)
  If(${MAC_OS_VERSION} LESS 10.9)
    ADD_DEFINITIONS(-DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS -DOLD_MAC)
    Set(DEFINITIONS OLD_MAC)
  Else()
    ADD_DEFINITIONS(-DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS)
  EndIf()
EndIf()

Set(LINKDEF CbmPapaLinkDef.h)
Set(LIBRARY_NAME CbmPapa)
Set(DEPENDENCIES
    L1 KF CbmSts CbmBase CbmData CbmGenerators Base Vc.a)

GENERATE_LIBRARY()



