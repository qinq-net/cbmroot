# Create a library called "libCbmFlibReader" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/trd
  ${CBMROOT_SOURCE_DIR}/data/raw
  ${CBMROOT_SOURCE_DIR}/data/beamtime/fhodo
  ${CBMROOT_SOURCE_DIR}/base
  ${CBMROOT_SOURCE_DIR}/beamtime/base
  ${CBMROOT_SOURCE_DIR}/fles/reader
  ${CBMROOT_SOURCE_DIR}/fles/reader/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/reader/tasks
  ${CBMROOT_SOURCE_DIR}/fles/reader/get4
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES}
  ${Boost_INCLUDE_DIR}
  ${ROOT_INCLUDE_DIR}
  ${CBMROOT_SOURCE_DIR}/external/ipc_legacy/ipc/src
  ${CBMROOT_SOURCE_DIR}/external/flib_dpb/flib_dpb
  ${CBMROOT_SOURCE_DIR}/external/flib_dpb_20/flib_dpb_20
  ${CBMROOT_SOURCE_DIR}/external/spadic/spadic
  ${CBMROOT_SOURCE_DIR}/fles/trb/trbnet_lib
  ${CBMROOT_SOURCE_DIR}/rich/prototype/beamtime
  ${CBMROOT_SOURCE_DIR}/beamtime/tof
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc/get4

  # needed by rootcling
  ${CBMROOT_SOURCE_DIR}/external/spadic/spadic/lib/message/wrap/cpp
  ${CBMROOT_SOURCE_DIR}/external/spadic/spadic/lib/message
  ${CMAKE_SOURCE_DIR}/external/cppzmq
)

Include_Directories(${INCLUDE_DIRECTORIES})
Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})


Set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
  ${Vc_LIB_DIR}
  ${KFParticle_LIB_DIR}
)

Link_Directories(
  ${LINK_DIRECTORIES}
)

Set(SRCS
  CbmFlibFileSource.cxx
#  CbmNxFlibFileSource.cxx
  CbmFlibFileSourceNew.cxx
  CbmFiberHodoMapping.cxx

  unpacker/CbmTSUnpackSpadic.cxx
  unpacker/CbmTSUnpackSpadic20.cxx
  unpacker/CbmTSUnpackSpadic11OnlineMonitor.cxx
  unpacker/CbmTSUnpackSpadic20OnlineMonitor.cxx
  unpacker/CbmTSUnpackSpadic20DesyOnlineMonitor.cxx
  unpacker/CbmTSUnpackSpadicLegacy.cxx
  unpacker/CbmTSUnpackNxyter.cxx
  unpacker/CbmTSUnpackStsxyter.cxx
  unpacker/CbmTSUnpackTestMiniRich.cxx
  unpacker/CbmTSUnpackFiberHodo.cxx
  unpacker/CbmTSUnpackDummy.cxx
  unpacker/CbmTSUnpackTrb.cxx
  unpacker/CbmGet4v1xHackDef.cxx
  unpacker/CbmTSUnpackGet4v1x.cxx
  unpacker/CbmTSUnpackSpadicOnlineFex.cxx

  get4/CbmGet4FastMonitor.cxx
  get4/CbmGet4EpochBuffer.cxx
  get4/CbmGet4EventBuilder.cxx
  get4/CbmGet4EventBuffer.cxx

  tasks/CbmTrdRawBeamProfile.cxx
  tasks/CbmTrdRawBuchData.cxx
  tasks/CbmTrdRawPulseMonitor.cxx
  tasks/CbmTrdOnlineDisplay.cxx
  tasks/CbmTrdEveOnlineDisplay.cxx
  tasks/CbmTrdDaqBuffer.cxx

  tasks/CbmTestMiniRichPairBuilder.cxx

  ${FLES_TOF_BEAM_SOURCES}
)

Set(NO_DICT_SRCS
  unpacker/TimesliceReader.cpp
  unpacker/TimesliceReader20.cpp
# unpacker/TimesliceReader20.cpp
)

If(_UINT8_T_EXIST)
  Add_Definitions(-DHAVE_UINT8_T_HEADER_FILE)
  Set(DEFINITIONS HAVE_UINT8_T_HEADER_FILE)
EndIf()

Set(LINKDEF  CbmFlibReaderLinkDef.h )
Set(LIBRARY_NAME CbmFlibReader)
Set(DEPENDENCIES
  CbmBase CbmBeamtimeBase CbmRich fles_ipc_legacy flib_dpb flib_dpb_20
  spadicMessage trbnet ${FLES_TOF_BEAM_DEPEND} Eve Base
)

GENERATE_LIBRARY()
