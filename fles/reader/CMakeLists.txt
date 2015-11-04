# Create a library called "libCbmFlibReader" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/trd
  ${CBMROOT_SOURCE_DIR}/data/raw
  ${CBMROOT_SOURCE_DIR}/data/beamtime/fhodo
  ${CBMROOT_SOURCE_DIR}/base
  ${CBMROOT_SOURCE_DIR}/fles/reader
  ${CBMROOT_SOURCE_DIR}/fles/reader/unpacker
  ${CBMROOT_SOURCE_DIR}/fles/reader/tasks
  ${CBMROOT_SOURCE_DIR}/fles/reader/get4
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR} 
  ${ROOT_INCLUDE_DIR}  
  ${CBMROOT_SOURCE_DIR}/fles/ipc/src
  ${CBMROOT_SOURCE_DIR}/fles/flib_dpb
  ${CBMROOT_SOURCE_DIR}/fles/spadic
  ${CBMROOT_SOURCE_DIR}/fles/trb/trbnet_lib
  ${CBMROOT_SOURCE_DIR}/rich/prototype/beamtime
  ${CBMROOT_SOURCE_DIR}/beamtime/tof
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc/get4

  # needed by rootcling
  ${CBMROOT_SOURCE_DIR}/fles/spadic/message/wrap/cpp
  ${CBMROOT_SOURCE_DIR}/fles/spadic/message
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

#If(BUILD_BEAMTIME)
#  add_definitions(-DBUILD_BEAMTIME)
#  Set(DEFINITION -DBUILD_BEAMTIME)
#  Set(FLES_TOF_BEAM_SOURCES 
#  )
#  Set(FLES_TOF_BEAM_DEPEND
#  )
#Else(BUILD_BEAMTIME)
  Set(FLES_TOF_BEAM_SOURCES 
    get4/CbmGet4EventDumper.cxx
#    get4/CbmGet4EventMonitor.cxx
  )
  Set(FLES_TOF_BEAM_DEPEND
    CbmTofUnpack
  )
#EndIf(BUILD_BEAMTIME)

Set(SRCS
  CbmFlibFileSource.cxx   
#  CbmNxFlibFileSource.cxx   
  CbmFlibFileSourceNew.cxx   
  CbmDaqBuffer.cxx
  CbmFiberHodoMapping.cxx

  unpacker/CbmTSUnpack.cxx
  unpacker/CbmTSUnpackSpadic.cxx
  unpacker/CbmTSUnpackNxyter.cxx
  unpacker/CbmTSUnpackStsxyter.cxx
  unpacker/CbmTSUnpackFiberHodo.cxx
  unpacker/CbmTSUnpackDummy.cxx
  unpacker/CbmTSUnpackTrb.cxx
  unpacker/CbmGet4v1xHackDef.cxx
  unpacker/CbmTSUnpackGet4v1x.cxx
  
  get4/CbmGet4FastMonitor.cxx
  get4/CbmGet4EpochBuffer.cxx
  get4/CbmGet4EventBuilder.cxx
  get4/CbmGet4EventBuffer.cxx

  tasks/CbmTrdRawBeamProfile.cxx
  tasks/CbmTrdRawPulseMonitor.cxx
  tasks/CbmTrdOnlineDisplay.cxx
  tasks/CbmTrdEveOnlineDisplay.cxx
  tasks/CbmTrdDaqBuffer.cxx
  tasks/CbmTrdClusterAnalysis.cxx
  tasks/CbmTrdTestBeamAnalysis.cxx

  ${FLES_TOF_BEAM_SOURCES}
)

Set(NO_DICT_SRCS
 unpacker/TimesliceReader.cpp
)

If(NOT OldFairSource)
  Add_Definitions(-D_NewFairSource)
  Set(DEFINITIONS ${DEFINITIONS} _NewFairSource)
EndIf()

Set(LINKDEF  CbmFlibReaderLinkDef.h )
Set(LIBRARY_NAME CbmFlibReader)
Set(DEPENDENCIES
    CbmBase CbmRich fles_ipc flib_dpb spadicMessage trbnet ${FLES_TOF_BEAM_DEPEND} Eve Base 
)

GENERATE_LIBRARY()
