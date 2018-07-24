# Create a library called "libTrd" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/eventdisplay 
${CBMROOT_SOURCE_DIR}/base
${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/tof/TofData
${CBMROOT_SOURCE_DIR}/data/global
${CBMROOT_SOURCE_DIR}/trd/base
)

include_directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})


set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
) 

link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmTrdDigiDraw.cxx
CbmPixelHitSetDraw.cxx
CbmEvDisTracks.cxx
CbmRecoTracks.cxx
)

set(LINKDEF  CbmDisplayLinkDef.h)
Set(LIBRARY_NAME CbmDisplay)
Set(DEPENDENCIES
    CbmData EventDisplay Base CbmTrdSim
)

GENERATE_LIBRARY()
