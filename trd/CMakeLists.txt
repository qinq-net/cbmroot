# Create a library called "libTrd" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

add_subdirectory (pid)

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
${CBMROOT_SOURCE_DIR}/trd/digi
${CBMROOT_SOURCE_DIR}/trd/param
${CBMROOT_SOURCE_DIR}/trd/reco
${CBMROOT_SOURCE_DIR}/trd/tools
${CBMROOT_SOURCE_DIR}/trd/data
${CBMROOT_SOURCE_DIR}/trd/beamtime

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
base/CbmGeoTrd.cxx 

mc/CbmTrd.cxx 		 

base/CbmGeoTrdPar.cxx 
base/CbmTrdContFact.cxx   

attic/CbmTrdDigiPar.cxx
attic/CbmTrdCreateDigiPar.cxx

#digi/CbmTrdDigitizerMathieson.cxx 
attic/CbmTrdDigitizerPRF.cxx
attic/CbmTrdDigitizerPRF_TB.cxx
digi/CbmTrdDigitizer.cxx
#attic/CbmTrdDigiWriteoutBuffer.cxx

base/CbmTrdSPADIC.cxx

qa/CbmTrdHitProducerQa.cxx  
qa/CbmTrdTracksPidQa.cxx
qa/CbmTrdQa.cxx
qa/CbmTrdHitDensityQa.cxx
qa/CbmTrdHitRateQa.cxx
qa/CbmTrdHitRateFastQa.cxx
qa/CbmTrdOccupancyQa.cxx
qa/CbmTrdDigitizerPRFQa.cxx
qa/CbmTrdClusterizerFastQa.cxx
qa/CbmTrdHitProducerClusterQa.cxx
qa/CbmTrdRecoQa.cxx
qa/CbmTrdMCQa.cxx

attic/reco/CbmTrdHitProducerIdeal.cxx    
attic/reco/CbmTrdHitProducerSmearing.cxx
attic/reco/CbmTrdHitProducerDigi.cxx
#reco/CbmTrdClusterFinder.cxx
attic/reco/CbmTrdClusterFinderFast.cxx
attic/reco/CbmTrdClusterFinderDPB.cxx
attic/reco/CbmTrdHitProducerCluster.cxx
#attic/reco/CbmTrdSimpleHitProducerCluster.cxx

base/CbmTrdUtils.cxx
tools/CbmTrdElectronsTrainAnn.cxx
base/CbmTrdGeoHandler.cxx
tools/CbmTrdTriangle.cxx

data/CbmTrdRadiator.cxx  
base/CbmTrdGas.cxx
attic/CbmTrdModule.cxx
attic/CbmTrdAsic.cxx
#beamtime/CbmTrdRawToDigiSpadic03.cxx

#${ROCSYS}/include/spadic/Message.h
#${ROCSYS}/include/roc/Message.h
#${ROCSYS}/onlinemonitor/rocmonitor/TRocEvent.cxx
#${ROCSYS}/onlinemonitor/spadicmonitor/TSpadicEvent.cxx
#${ROCSYS}/onlinemonitor/framework/TTriglogEvent.cxx
#${ROCSYS}/beamtime/cern-oct12/go4/TMbsCrateEvent.cxx
#${ROCSYS}/beamtime/cern-oct12/go4/TCernOct12UnpackEvent.cxx
#${ROCSYS}/beamtime/cern-oct12/go4/TCernOct12DetectorEvent.cxx
#${ROCSYS}/beamtime/cern-oct12/go4/TBeamMonitorEvent.cxx

attic/CbmTrdFindTracks.cxx  
CbmTrdTrackFinderIdeal.cxx  
CbmTrdFitTracks.cxx  
CbmTrdTrackFitterIdeal.cxx  
#CbmTrdMatchTracks.cxx
#CbmTrdDigiSorterTask.cxx
#CbmTrdDigiRingSorter.cxx

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
