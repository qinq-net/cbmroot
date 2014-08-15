# Create a library called "libCbmFlibReader" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
#  ${CBMROOT_SOURCE_DIR}/cbmbase
#  ${CBMROOT_SOURCE_DIR}/cbmdata
  ${CBMROOT_SOURCE_DIR}/fles/ipc
  ${CBMROOT_SOURCE_DIR}/fles/spadic
  ${CBMROOT_SOURCE_DIR}/fles/reader
)

Include_Directories(${INCLUDE_DIRECTORIES})
Include_Directories(SYSTEM ${Boost_INCLUDE_DIR})
 

Set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
)
 
Link_Directories( 
  ${LINK_DIRECTORIES}
)

Set(SRCS
  CbmFlibFileSource.cxx
)

Set(LINKDEF  FlibReaderLinkDef.h )
Set(LIBRARY_NAME CbmFlibReader)
Set(DEPENDENCIES
    Base fles_ipc spadicMessage
)

GENERATE_LIBRARY()
