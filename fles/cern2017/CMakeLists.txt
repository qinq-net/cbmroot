# Create a library called "libCbmFlibReader" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/base

  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/beamtime
  ${CBMROOT_SOURCE_DIR}/data/beamtime/fhodo
  ${CBMROOT_SOURCE_DIR}/data/raw
  ${CBMROOT_SOURCE_DIR}/data/sts
  ${CBMROOT_SOURCE_DIR}/data/much

  ${CBMROOT_SOURCE_DIR}/beamtime/base
  ${CBMROOT_SOURCE_DIR}/fles/cern2017
  ${CBMROOT_SOURCE_DIR}/fles/cern2017/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/cern2017/param
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
  unpacker/StsXyterMessage.cxx

#  unpacker/CbmCern2017UnpackerSts.cxx
#  unpacker/CbmCern2017UnpackerMuch.cxx

  unpacker/CbmCern2017MonitorSts.cxx
#  unpacker/CbmCern2017MonitorMuch.cxx

  param/CbmCern2017ContFact.cxx
  param/CbmCern2017UnpackParSts.cxx
#  param/CbmCern2017UnpackParMuch.cxx
)

#Set(NO_DICT_SRCS
#  unpacker/rocMess_wGet4v1.cxx
#)

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


Set(LINKDEF CbmFlibCern2017LinkDef.h )
Set(LIBRARY_NAME CbmFlibCern2017)
Set(DEPENDENCIES
    CbmBase CbmBeamtimeBase fles_ipc Eve Base
)

GENERATE_LIBRARY()
