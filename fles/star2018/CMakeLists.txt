# Create a library called "libCbmFlibReader" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/base

  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/beamtime
  ${CBMROOT_SOURCE_DIR}/data/tof

  ${CBMROOT_SOURCE_DIR}/beamtime/base
  ${CBMROOT_SOURCE_DIR}/fles/cern2016
  ${CBMROOT_SOURCE_DIR}/fles/cern2016/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/cern2016/param

  ${CBMROOT_SOURCE_DIR}/fles/star2017
  ${CBMROOT_SOURCE_DIR}/fles/star2017/unpacker

  ${CBMROOT_SOURCE_DIR}/fles/star2018
  ${CBMROOT_SOURCE_DIR}/fles/star2018/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/star2018/param
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
  CbmTofStar2018Source.cxx

  param/CbmTofStar2018Par.cxx
  param/CbmTofStar2018ContFact.cxx

  unpacker/rocMess_wGet4v2.cxx
  unpacker/CbmTofStarData2018.cxx
  unpacker/CbmTofStarMonitor2018.cxx
  unpacker/CbmTofStarMonitorShift2018.cxx
  unpacker/CbmTofStarEventBuilder2018.cxx
  unpacker/CbmUnpackTofStar2018.cxx
)

#Set(NO_DICT_SRCS
#  unpacker/rocMess_wGet4v2.cxx
#)

If( STAR_SUBEVT_BUILDER )
  ADD_DEFINITIONS(-DSTAR_SUBEVT_BUILDER)
  Message(STATUS "Compiling with support for STAR DAQ Event writing")
Set(NO_DICT_SRCS
  # STAR sub-event writer source code
#  unpacker/star_rhicf.c
)
EndIf()

If(_UINT8_T_EXIST)
  Add_Definitions(-DHAVE_UINT8_T_HEADER_FILE)
  Set(DEFINITIONS HAVE_UINT8_T_HEADER_FILE)
EndIf()

Set(LINKDEF CbmFlibStar2018LinkDef.h )
Set(LIBRARY_NAME CbmFlibStar2018)
Set(DEPENDENCIES
    CbmFlibStar2017 CbmFlibCern2016 CbmBase CbmBeamtimeBase fles_ipc Eve Base
)

GENERATE_LIBRARY()
