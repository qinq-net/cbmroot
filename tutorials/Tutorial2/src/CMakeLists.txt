# Create a library called "libTutorial1" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)

set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/tutorials/Tutorial2/src
  ${CBMROOT_SOURCE_DIR}/data
)

include_directories( ${INCLUDE_DIRECTORIES})

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
) 

link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmTutorial2Det.cxx
CbmTutorial2DetDigiPar.cxx
CbmTutorial2DetGeo.cxx		
CbmTutorial2DetPoint.cxx
CbmTutorial2DetContFact.cxx
CbmTutorial2DetDigitizer.cxx
CbmTutorial2DetGeoPar.cxx
)

Set(LINKDEF Tutorial2LinkDef.h)
Set(LIBRARY_NAME Tutorial2)
Set(DEPENDENCIES
    CbmData
)

GENERATE_LIBRARY()

