set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/vis
${CBMROOT_SOURCE_DIR}/mvd
${CBMROOT_SOURCE_DIR}/much/geo
${CBMROOT_SOURCE_DIR}/base
${CBMROOT_SOURCE_DIR}/data 
${CBMROOT_SOURCE_DIR}/data/much 
${CBMROOT_SOURCE_DIR}/data/mvd 
${CBMROOT_SOURCE_DIR}/data/sts 
)

Include_Directories(${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmVisMuch.cxx
CbmVisPoint.cxx
CbmVisHit.cxx
CbmVisStripHit.cxx
CbmVisPixelHit.cxx
CbmVisMuchStationFrame.cxx  
CbmVisMuchCluster.cxx 
CbmVisMuchModuleFrame.cxx 
CbmVisMvd.cxx
CbmVisMvdStationFrame.cxx
)

set(LINKDEF VisLinkDef.h)
Set(LIBRARY_NAME Vis)
Set(DEPENDENCIES
    Base 
)

GENERATE_LIBRARY()
