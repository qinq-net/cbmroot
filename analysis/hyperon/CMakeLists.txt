Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/analysis/hyperon

${CBMROOT_SOURCE_DIR}/base 
${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/global

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface 
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmAnaHypYPtGenerator.cxx 
CbmAnaHypTools.cxx
CbmAnaHypSecFinder.cxx 
CbmAnaHypSecFinderQa.cxx
CbmAnaHypPair.cxx 
CbmAnaHypLambdaCandidate.cxx
CbmAnaHypTrack.cxx 
CbmAnaHypRecEff.cxx
CbmAnaHypRecLambda.cxx 
CbmAnaHypSelectLambda.cxx
CbmAnaHypRecCascade.cxx
CbmAnaHypMix.cxx
)

Set(LINKDEF CbmHyperonAnalysisLinkDef.h)
Set(LIBRARY_NAME CbmHyperonAnalysis)
Set(DEPENDENCIES)
#    Littrack KF L1 CbmMuch CbmRich CbmSts CbmBase CbmData CbmGenerators Base Vc.a)
#Set(DEFINITIONS -DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS)

GENERATE_LIBRARY()




