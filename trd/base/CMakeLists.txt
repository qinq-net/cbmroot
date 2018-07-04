# Create a library called "libTrd" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
# needed by run/FairMCBuffer.h
${CBMROOT_SOURCE_DIR}/run 
${CBMROOT_SOURCE_DIR}/data/mvd 
${CBMROOT_SOURCE_DIR}/data/rich 
${CBMROOT_SOURCE_DIR}/data/much 
${CBMROOT_SOURCE_DIR}/data/trd 
${CBMROOT_SOURCE_DIR}/data/tof 
${CBMROOT_SOURCE_DIR}/data/ecal 
${CBMROOT_SOURCE_DIR}/data/psd 

${CBMROOT_SOURCE_DIR}/base
${CBMROOT_SOURCE_DIR}/base/draw
${CBMROOT_SOURCE_DIR}/base/report

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/data/global

${CBMROOT_SOURCE_DIR}/L1/vectors

${CBMROOT_SOURCE_DIR}/trd
${CBMROOT_SOURCE_DIR}/trd/attic
${CBMROOT_SOURCE_DIR}/trd/attic/reco
${CBMROOT_SOURCE_DIR}/trd/base
${CBMROOT_SOURCE_DIR}/trd/qa
${CBMROOT_SOURCE_DIR}/trd/rec
${CBMROOT_SOURCE_DIR}/trd/sim
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
CbmGeoTrd.cxx 
CbmGeoTrdPar.cxx 
CbmTrdContFact.cxx   
CbmTrdSPADIC.cxx
CbmTrdUtils.cxx
CbmTrdGeoHandler.cxx
CbmTrdGas.cxx

../sim/CbmTrdRadiator.cxx  

../attic/CbmTrdDigiPar.cxx
../attic/CbmTrdCreateDigiPar.cxx
../attic/CbmTrdDigitizerPRF.cxx
../attic/CbmTrdDigitizerPRF_TB.cxx
../attic/reco/CbmTrdHitProducerIdeal.cxx    
../attic/reco/CbmTrdHitProducerSmearing.cxx
../attic/reco/CbmTrdHitProducerDigi.cxx
../attic/reco/CbmTrdClusterFinderFast.cxx
../attic/reco/CbmTrdClusterFinderDPB.cxx
../attic/reco/CbmTrdHitProducerCluster.cxx
../attic/CbmTrdTriangle.cxx
../attic/CbmTrdModule.cxx
../attic/CbmTrdAsic.cxx
../attic/CbmTrdFindTracks.cxx  
../attic/CbmTrdFitTracks.cxx  
)

IF (SSE_FOUND)
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-msse -O3")
  Message(STATUS "TRD will be compiled with SSE support")
ELSE (SSE_FOUND)
  Message(STATUS "TRD will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-O3")
ENDIF (SSE_FOUND)



set(LINKDEF CbmTrdLinkDef.h)
Set(LIBRARY_NAME CbmTrd)
Set(DEPENDENCIES
    CbmBase CbmData Base TMVA
)

GENERATE_LIBRARY()
