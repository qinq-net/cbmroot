# Create a library called "libCbmBeamtime" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/beamtime
  ${CBMROOT_SOURCE_DIR}/roclight
  ${CBMROOT_SOURCE_DIR}/beamtime/base
)


Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR}
)

Include_Directories( 
  ${INCLUDE_DIRECTORIES}
)

Include_Directories( 
  SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES}
)

Set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
  ${FAIRROOT_LIBRARY_DIR}
)
 
Link_Directories( 
  ${LINK_DIRECTORIES}
)

Set(SRCS
  CbmROCUnpack.cxx
  CbmTbDaqBuffer.cxx
  CbmTbEventBuilder.cxx
  CbmTbEventBuilderFixedTimeWindow.cxx
)

If(_UINT8_T_EXIST)
  Add_Definitions(-DHAVE_UINT8_T_HEADER_FILE)
  Set(DEFINITIONS HAVE_UINT8_T_HEADER_FILE)
EndIf()

Set(LINKDEF  CbmBeamtimeBaseLinkDef.h)
Set(LIBRARY_NAME CbmBeamtimeBase)
Set(DEPENDENCIES
    CbmRocLight
    CbmBase
    Base 
)

GENERATE_LIBRARY()
