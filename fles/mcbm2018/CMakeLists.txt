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

  ${CBMROOT_SOURCE_DIR}/fles/star2018/param

  ${CBMROOT_SOURCE_DIR}/fles/mcbm2018
  ${CBMROOT_SOURCE_DIR}/fles/mcbm2018/dataformat
  ${CBMROOT_SOURCE_DIR}/fles/mcbm2018/parameter
  ${CBMROOT_SOURCE_DIR}/fles/mcbm2018/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/mcbm2018/monitor
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES}
  ${Boost_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIR}
  ${IPC_INCLUDE_DIRECTORY}
  ${CBMROOT_SOURCE_DIR}/external/flib_dpb/flib_dpb
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/tof
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
   CbmMcbmUnpack.cxx
   CbmMcbm2018Source.cxx

   dataformat/gDpbMessv100.cxx

   parameter/CbmMcbm2018StsPar.cxx
#   parameter/CbmMcbm2018MuchPar.cxx
   parameter/CbmMcbm2018TofPar.cxx
   parameter/CbmMcbm2018ContFact.cxx

#   unpacker/CbmMcbm2018UnpackerT0.cxx
#   unpacker/CbmMcbm2018UnpackerSts.cxx
#   unpacker/CbmMcbm2018UnpackerMuch.cxx
#   unpacker/CbmMcbm2018UnpackerTrd.cxx
   unpacker/CbmMcbm2018TofUnpacker.cxx

   monitor/CbmMcbm2018MonitorMcbmRate.cxx
   monitor/CbmMcbm2018MonitorMcbmSync.cxx
   monitor/CbmMcbm2018MonitorStsSync.cxx
#   monitor/CbmMcbm2018MonitorStsPulser.cxx
   monitor/CbmMcbm2018MonitorSts.cxx
#   monitor/CbmMcbm2018MonitorMuch.cxx
#   monitor/CbmMcbm2018MonitorTrd.cxx
   monitor/CbmMcbm2018MonitorTof.cxx
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
  ADD_DEFINITIONS(-DVERSION_GREATER_160601)
  Add_Definitions(-DUSE_HTTP_SERVER)
  Message("Found FairRoot version larger 16.06.1")
EndIf()


Set(LINKDEF CbmFlibMcbm2018LinkDef.h )
Set(LIBRARY_NAME CbmFlibMcbm2018)
Set(DEPENDENCIES
    CbmFlibStar2018 CbmFlibCern2017 CbmBase CbmBeamtimeBase fles_ipc Eve Base
)

GENERATE_LIBRARY()
