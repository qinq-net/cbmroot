# Create a library called "libDilept" which includes the source files
# given in the array . The extension is already found.  Any number of
# sources could be listed here.

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/base

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/rich
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/data/tof
${CBMROOT_SOURCE_DIR}/data/global

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface

${CBMROOT_SOURCE_DIR}/dilept
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
CbmDileptonTrack.cxx
CbmDileptonTrackReal.cxx
CbmDileptonTrackRealCollection.cxx
CbmDileptonAssignPid.cxx
CbmDileptonTrackSim.cxx
CbmDileptonTrackSimCollection.cxx
CbmDileptonAssignMCid.cxx
)


set(LINKDEF CbmDileptonLinkDef.h)
Set(LIBRARY_NAME CbmDilept)
Set(DEPENDENCIES
    KF CbmBase CbmData Base 
)

GENERATE_LIBRARY()
