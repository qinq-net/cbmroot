Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/analysis/much

${CBMROOT_SOURCE_DIR}/base 

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/much
${CBMROOT_SOURCE_DIR}/data/global

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface 

${CBMROOT_SOURCE_DIR}/much/geo
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
${Vc_LIB_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmAnaMuch.cxx
CbmAnaMuchJpsiPolarization.cxx
CbmPolarizedGenerator.cxx
CbmAnaMuonCandidate.cxx 
CbmAnaDimuonCandidate.cxx
CbmAnaDimuonAnalysis.cxx
CbmAnaDimuonHisto.cxx
CbmAnaAngularAcceptance.cxx
CbmAnaMuchBgd.cxx
)

Set(LINKDEF CbmMuchAnalysisLinkDef.h)
Set(LIBRARY_NAME CbmMuchAnalysis)
Set(DEPENDENCIES CbmBase CbmData CbmMuch KF)

GENERATE_LIBRARY()




