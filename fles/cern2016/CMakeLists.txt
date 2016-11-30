# Create a library called "libCbmFlibReader" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/base

  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/beamtime/fhodo
  ${CBMROOT_SOURCE_DIR}/data/raw
  ${CBMROOT_SOURCE_DIR}/data/tof
  ${CBMROOT_SOURCE_DIR}/data/much

  ${CBMROOT_SOURCE_DIR}/beamtime/base
  ${CBMROOT_SOURCE_DIR}/fles/cern2016
  ${CBMROOT_SOURCE_DIR}/fles/cern2016/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/cern2016/param
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR} 
  ${ROOT_INCLUDE_DIR}  
  ${CBMROOT_SOURCE_DIR}/external/ipc/ipc
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
  CbmFlibTestSource.cxx   
  CbmFlibCern2016Source.cxx   

  unpacker/CbmTSUnpackFHodo.cxx
#  unpacker/CbmTSUnpackMuch.cxx
  unpacker/CbmTSUnpackTof.cxx
  
  unpacker/CbmTSMonitorTofLegacy.cxx
  unpacker/CbmTSMonitorTof.cxx
  unpacker/CbmTSMonitorMuch.cxx

  param/CbmTofUnpackPar.cxx
  param/CbmFHodoUnpackPar.cxx
  param/CbmMuchUnpackPar.cxx
  param/CbmFlesTestContFact.cxx
)

Set(NO_DICT_SRCS
  unpacker/rocMess_wGet4v1.cxx
)

If(_UINT8_T_EXIST)
  Add_Definitions(-DHAVE_UINT8_T_HEADER_FILE)
  Set(DEFINITIONS HAVE_UINT8_T_HEADER_FILE)
EndIf()

If(FairRoot_VERSION VERSION_LESS 15.11.2)
  ADD_DEFINITIONS(-DVERSION_LESS_151102)
  Set(DEFINITIONS ${DEFINITIONS} VERSION_LESS_151102)
  Message("FairRoot Version less 15.11.2")
Else()
  Message("FairRoot Version at least 15.11.2")
EndIf()

If(FairRoot_VERSION VERSION_GREATER 16.06.1)
  Add_Definitions(-DUSE_HTTP_SERVER)
  Message("Found FairRoot version larger 16.06.1")
EndIf()


Set(LINKDEF CbmFlibCern2016LinkDef.h )
Set(LIBRARY_NAME CbmFlibCern2016)
Set(DEPENDENCIES
    CbmBase CbmBeamtimeBase fles_ipc Eve Base 
)

GENERATE_LIBRARY()
