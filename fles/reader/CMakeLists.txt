# Create a library called "libCbmFlibReader" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/cbmdata/raw
  ${CBMROOT_SOURCE_DIR}/cbmdata
  ${CBMROOT_SOURCE_DIR}/cbmbase
  ${CBMROOT_SOURCE_DIR}/fles/reader
  ${CBMROOT_SOURCE_DIR}/fles/tasks
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR} 
  ${ROOT_INCLUDE_DIR}  
  ${CBMROOT_SOURCE_DIR}/fles/ipc/src
  ${CBMROOT_SOURCE_DIR}/fles/flib_dpb
  ${CBMROOT_SOURCE_DIR}/fles/spadic
)

Include_Directories(${INCLUDE_DIRECTORIES})
Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})
 

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
  CbmNxFlibFileSource.cxx   

  CbmTSUnpack.cxx
  CbmTSUnpackSpadic.cxx
  CbmTSUnpackNxyter.cxx
  CbmFlibFileSourceNew.cxx   
  CbmDaqBuffer.cxx

  tasks/CbmTrdRawBeamProfile.cxx
)

Set(NO_DICT_SRCS
 TimesliceReader.cpp
)

Set(LINKDEF  FlibReaderLinkDef.h )
Set(LIBRARY_NAME CbmFlibReader)
Set(DEPENDENCIES
    Base CbmBase fles_ipc flib_dpb spadicMessage
)

GENERATE_LIBRARY()
