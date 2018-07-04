# Create a library called "libCbmTrdQa" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/trd/attic
  ${CBMROOT_SOURCE_DIR}/trd/attic/reco
  ${CBMROOT_SOURCE_DIR}/trd/base
  ${CBMROOT_SOURCE_DIR}/trd/sim
  ${CBMROOT_SOURCE_DIR}/trd/qa

  ${CBMROOT_SOURCE_DIR}/base
  ${CBMROOT_SOURCE_DIR}/base/report
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/trd 
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR}
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
  CbmTrdClusterizerFastQa.cxx
  CbmTrdDigitizerPRFQa.cxx
  CbmTrdHitDensityQa.cxx
  CbmTrdHitProducerClusterQa.cxx
  CbmTrdHitProducerQa.cxx
  CbmTrdHitRateFastQa.cxx
  CbmTrdHitRateQa.cxx
  CbmTrdMCQa.cxx
  CbmTrdOccupancyQa.cxx
  CbmTrdQa.cxx
  CbmTrdRecoQa.cxx
  #CbmTrdRecoQaTb.cxx
  #CbmTrdRecoTbQa.cxx
  CbmTrdTracksPidQa.cxx
)

IF (SSE_FOUND)
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-msse -O3")
  Message(STATUS "TRD QA will be compiled with SSE support")
ELSE (SSE_FOUND)
  Message(STATUS "TRD QA will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-O3")
ENDIF (SSE_FOUND)



set(LINKDEF CbmTrdQaLinkDef.h)
Set(LIBRARY_NAME CbmTrdQa)
Set(DEPENDENCIES
    CbmBase CbmData Base CbmTrd CbmTrdSim
#    CbmBase CbmData Base CbmTrd CbmTrdSim CbmTrdRec
)

GENERATE_LIBRARY()
