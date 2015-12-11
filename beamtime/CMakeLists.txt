# Create a library called "libCbmBeamtime" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/base
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/sts
  ${CBMROOT_SOURCE_DIR}/data/much
  ${CBMROOT_SOURCE_DIR}/data/beamtime
  ${CBMROOT_SOURCE_DIR}/data/beamtime/fhodo
  ${CBMROOT_SOURCE_DIR}/sts/setup
  ${CBMROOT_SOURCE_DIR}/roclight
  ${CBMROOT_SOURCE_DIR}/beamtime
  ${CBMROOT_SOURCE_DIR}/beamtime/fhodo
  ${CBMROOT_SOURCE_DIR}/beamtime/param
  ${CBMROOT_SOURCE_DIR}/sts/setup
  ${CBMROOT_SOURCE_DIR}/beamtime/reco
  ${CBMROOT_SOURCE_DIR}/beamtime/unpacker
  ${CBMROOT_SOURCE_DIR}/beamtime/eventbuilder
)


Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR}
  ${GEANT4_INCLUDE_DIR}
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
  ${GEANT4_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
)
 
Link_Directories( 
  ${LINK_DIRECTORIES}
)

Set(SRCS
  CbmMuchDigiTb.cxx
  CbmSourceLmd.cxx
  CbmSourceLmdNew.cxx
  CbmLmdDebug.cxx
  CbmTbDaqBuffer.cxx

  CbmDaqMap.cxx
  CbmDaqMapCosy2014.cxx
  CbmDaqMapCosy2014Much.cxx
  CbmDaqMapCosy2013.cxx
  
  unpacker/CbmROCUnpack.cxx
  unpacker/CbmROCUnpackAux.cxx
  unpacker/CbmROCUnpackDummy.cxx
  unpacker/CbmROCUnpackEpoch.cxx
  unpacker/CbmROCUnpackSystemMessage.cxx
  unpacker/CbmROCUnpackSts.cxx
  unpacker/CbmROCUnpackHodo.cxx
  unpacker/CbmROCUnpackMuch.cxx

  eventbuilder/CbmTbEventBuilder.cxx
  eventbuilder/CbmTbEventBuilderFixedTimeWindow.cxx

  fhodo/CbmFiberHodoClusterFinder.cxx
  fhodo/CbmHodoRawHistos.cxx

  param/CbmBeamTimeContFact.cxx
  param/CbmBeamTimeFirstPar.cxx

  reco/CbmStsHodoCorrelations.cxx
  reco/CbmMuchRawHistos.cxx
  
  sts/reco/cosy/StsCosyBL.cxx
  sts/reco/cosy/StsCosyClusterFinder.cxx
  sts/reco/cosy/StsCosyHitFinder.cxx
  sts/reco/cosy/StsCosyAnalysis.cxx
  
  sts/settings/StsAlignment.cxx
  sts/settings/StsClusterSet.cxx
  sts/settings/StsHitSet.cxx


)

# Add artificial dependency which delays the compilation of the defined file
# until the libRoc.so is produced which is needed for the linking of the
# beamtime library. I couldn't find any better solution for the problem up
# to now.
Set_Source_Files_Properties(PROPERTIES CbmRocSource.cxx PROPERTIES
                            OBJECT_DEPENDS ${LIBRARY_OUTPUT_PATH}/libRoc.so
)

If(_UINT8_T_EXIST)
#  Add_Definitions(-DHAVE_UINT8_T_HEADER_FILE)
  Set(DEFINITIONS HAVE_UINT8_T_HEADER_FILE)
EndIf()

If(NOT OldFairSource)
  Add_Definitions(-D_NewFairSource)
  Set(DEFINITIONS ${DEFINITIONS} _NewFairSource)
EndIf()

Set(LINKDEF  CbmBeamtimeLinkDef.h)
Set(LIBRARY_NAME CbmBeamtime)
Set(DEPENDENCIES
#    ${LIBRARY_OUTPUT_PATH}/libDabcRoc.so 
#    ${LIBRARY_OUTPUT_PATH}/libDabcMbs.so 
#    ${LIBRARY_OUTPUT_PATH}/libDabcBase.so 
#    ${LIBRARY_OUTPUT_PATH}/libRoc.so 
    CbmRocLight
    CbmSts
    CbmData 
    CbmBase
#    Base Trd
    Base 
)

GENERATE_LIBRARY()
