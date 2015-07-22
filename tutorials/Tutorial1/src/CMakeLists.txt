# Create a library called "libTutorial1" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)

set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/tutorials/Tutorial1/src
  ${CBMROOT_SOURCE_DIR}/cbmdata
)

include_directories( ${INCLUDE_DIRECTORIES})

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
) 

link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmTutorialDet.cxx
CbmTutorialDetContFact.cxx
CbmTutorialDetGeo.cxx
CbmTutorialDetGeoPar.cxx
CbmTutorialDetPoint.cxx
)

Set(LINKDEF Tutorial1LinkDef.h)
Set(LIBRARY_NAME Tutorial1)
Set(DEPENDENCIES
    CbmData
)

GENERATE_LIBRARY()

