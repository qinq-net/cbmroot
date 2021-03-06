# Create a library called "libGlobal" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/global

${CBMROOT_SOURCE_DIR}/base
${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/data/tof
${CBMROOT_SOURCE_DIR}/data/ecal
${CBMROOT_SOURCE_DIR}/data/global

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface
${CBMROOT_SOURCE_DIR}/ecal
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
${KFParticle_LIB_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmEcalTrackingMuch.cxx
CbmFindGlobalTracks.cxx    
#CbmFindGlobalTracksQa.cxx  
CbmFindPrimaryVertex.cxx   
CbmFindPrimaryVertexEvents.cxx   
CbmFitGlobalTracks.cxx 
CbmFitGlobalTracksQa.cxx 
CbmGlobalTrackFitterIdeal.cxx 
CbmPVFinderIdeal.cxx       
CbmTrackMergerIdeal.cxx    
#CbmTofMergerIdeal.cxx 
)


set(LINKDEF CbmGlobalLinkDef.h)
Set(LIBRARY_NAME CbmGlobal)
Set(DEPENDENCIES
    KF CbmEcal
    CbmBase CbmData Base 
)

GENERATE_LIBRARY()

