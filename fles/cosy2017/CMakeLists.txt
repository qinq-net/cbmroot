# Create a library called "CbmFlibCosy2017" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/base

  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/raw
  ${CBMROOT_SOURCE_DIR}/data/sts
#  ${CBMROOT_SOURCE_DIR}/data/tof
#  ${CBMROOT_SOURCE_DIR}/data/much

  ${CBMROOT_SOURCE_DIR}/beamtime/base
  ${CBMROOT_SOURCE_DIR}/fles/cern2016
  ${CBMROOT_SOURCE_DIR}/fles/cern2016/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/cern2016/param
  
  ${CBMROOT_SOURCE_DIR}/fles/cosy2017
  ${CBMROOT_SOURCE_DIR}/fles/cosy2017/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/cosy2017/param
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR} 
  ${ROOT_INCLUDE_DIR}  
  ${IPC_INCLUDE_DIRECTORY}
  ${CBMROOT_SOURCE_DIR}/external/flib_dpb/flib_dpb
  ${CMAKE_SOURCE_DIR}/external/cppzmq
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
  unpacker/CbmTsMonitorSts.cxx
  
  param/CbmStsUnpackPar.cxx
  param/CbmFlesCosy2017ContFact.cxx
)

If(_UINT8_T_EXIST)
  Add_Definitions(-DHAVE_UINT8_T_HEADER_FILE)
  Set(DEFINITIONS HAVE_UINT8_T_HEADER_FILE)
EndIf()

Set(LINKDEF CbmFlibCosy2017LinkDef.h )
Set(LIBRARY_NAME CbmFlibCosy2017)
Set(DEPENDENCIES
    CbmFlibCern2016 CbmBase CbmBeamtimeBase fles_ipc Eve Base
)

GENERATE_LIBRARY()
