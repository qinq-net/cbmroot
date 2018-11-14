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

  ${CBMROOT_SOURCE_DIR}/fles/star2017/unpacker # For the star_rhicf.c interface file

  ${CBMROOT_SOURCE_DIR}/fles/star2018/param # for the STAR2018 parameter in event builder, TO REMOVE when updated for new param
  ${CBMROOT_SOURCE_DIR}/fles/star2018/unpacker # for the STAR2018 Event and Trigger format

  ${CBMROOT_SOURCE_DIR}/fles/mcbm2018/
  ${CBMROOT_SOURCE_DIR}/fles/mcbm2018/dataformat

  ${CBMROOT_SOURCE_DIR}/fles/star2019/
  ${CBMROOT_SOURCE_DIR}/fles/star2019/parameter
  ${CBMROOT_SOURCE_DIR}/fles/star2019/commonMQ
  ${CBMROOT_SOURCE_DIR}/fles/star2019/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/star2019/monitor
  ${CBMROOT_SOURCE_DIR}/fles/star2019/eventbuilder
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
   parameter/CbmStar2019TofPar.cxx
   parameter/CbmStar2019ContFact.cxx

#   unpacker/CbmStar2019TofUnpackerAlgo.cxx
#   unpacker/CbmStar2019TofUnpacker.cxx

#   monitor/CbmStar2019MonitorTofAlgo.cxx
#   monitor/CbmStar2019MonitorTof.cxx

   eventbuilder/CbmTofStarData2019.cxx
   eventbuilder/CbmStar2019EventBuilderEtofAlgo.cxx
   eventbuilder/CbmStar2019EventBuilderEtof.cxx
)

#Set(NO_DICT_SRCS
#  unpacker/rocMess_wGet4v1.cxx
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


Set(LINKDEF CbmFlibStar2019LinkDef.h )
Set(LIBRARY_NAME CbmFlibStar2019)
Set(DEPENDENCIES
    CbmFlibMcbm2018 CbmFlibStar2018 CbmBase CbmBeamtimeBase fles_ipc Eve Base
)

GENERATE_LIBRARY()
