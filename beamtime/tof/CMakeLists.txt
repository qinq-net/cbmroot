# Create a library called "libTofUnpack" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

#If(BUILD_BEAMTIME)
#  set(ROC_DIRECTORIES
##    ${CBMROOT_SOURCE_DIR}/MbsAPI
#    ${CBMROOT_BINARY_DIR}/roc/roc/roclib
#)
#else(BUILD_BEAMTIME)
  set(ROC_DIRECTORIES
    ${CBMROOT_SOURCE_DIR}/roclight
  )
#EndIf(BUILD_BEAMTIME)

set(INCLUDE_DIRECTORIES
#  ${CBMROOT_SOURCE_DIR}/MbsAPI
#  ${CBMROOT_BINARY_DIR}/roc/roc/roclib
#  ${CBMROOT_SOURCE_DIR}/roclight
  ${ROC_DIRECTORIES}
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/tof
  ${CBMROOT_SOURCE_DIR}/base 
  ${CBMROOT_SOURCE_DIR}/beamtime/tof
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/hadaq
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc/v1290
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc/vftx
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc/trb
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc/get4
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/scalers
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/scalers/triglog
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/scalers/scalormu
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/scalers/scal2014
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/unpMoni
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/calib
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/calib/tdc
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/calib/scaler
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/mapping
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/output
  ${CBMROOT_SOURCE_DIR}/beamtime/tof/display
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


set(SRCS
  TMbsUnpackTof.cxx
  TTriglogUnpackTof.cxx
  TGet4UnpackTof.cxx
  hadaq/HadaqDefines.cxx
  hadaq/HadaqTrbIterator.cxx
  hadaq/HadaqTdcMessage.cxx
  TTrbUnpackTof.cxx
  TMbsUnpackTofPar.cxx
  TMbsUnpackTofContFact.cxx
  scalers/TTofScalerBoard.cxx
  scalers/triglog/TTofTriglogBoard.cxx
  scalers/triglog/TTofTriglogUnpacker.cxx
  scalers/triglog/TTofTriglogScalUnpacker.cxx
  scalers/scalormu/TTofScomUnpacker.cxx
  scalers/scal2014/TTofScal2014Unpacker.cxx
  tdc/TTofTdcData.cxx
  tdc/TTofTdcBoard.cxx
  tdc/vftx/TTofVftxData.cxx
  tdc/vftx/TTofVftxBoard.cxx
  tdc/vftx/TTofVftxUnpacker.cxx
  tdc/trb/TTofTrbTdcData.cxx
  tdc/trb/TTofTrbTdcBoard.cxx
  tdc/trb/TTofTrbTdcUnpacker.cxx
  tdc/get4/TGet4v1Event.cxx
  tdc/get4/TTofGet4Data.cxx
  tdc/get4/TTofGet4Board.cxx
  tdc/get4/TGet4v1Rec.cxx
  tdc/get4/TGet4v1Param.cxx
  tdc/get4/TTofGet4Unpacker.cxx
  unpMoni/TMbsUnpTofMonitor.cxx
  calib/tdc/TTofCalibData.cxx
  calib/tdc/TMbsCalibTdcTof.cxx
  calib/scaler/TTofCalibScaler.cxx
  calib/scaler/TMbsCalibScalTof.cxx
  calib/TMbsCalibTof.cxx
  calib/TMbsCalibTofPar.cxx
  mapping/TMbsMappingTof.cxx
  mapping/TMbsMappingTofPar.cxx
  output/Plastics_Event.cxx
  output/Rpc_Event.cxx
  output/Scalers_Event.cxx
  output/TVftxBoardData.cxx
  output/TMbsConvTof.cxx
  output/TMbsConvTofPar.cxx
  display/CbmTofOnlineDisplay.cxx
)

Set(HEADERS
  TofDef.h
  TMbsUnpackTof.h
  TTriglogUnpackTof.h
  TGet4UnpackTof.h
  hadaq/HadaqDefines.h
  hadaq/HadaqTrbIterator.h
  hadaq/HadaqTimeStamp.h
  hadaq/HadaqTdcMessage.h
  hadaq/HadaqTdcIterator.h
  TTrbUnpackTof.h
  TMbsUnpackTofPar.h
  TMbsUnpackTofContFact.h
  scalers/TofScalerDef.h
  scalers/TTofScalerBoard.h
  scalers/triglog/TofTriglogDef.h
  scalers/triglog/TTofTriglogBoard.h
  scalers/triglog/TTofTriglogUnpacker.h
  scalers/triglog/TTofTriglogScalUnpacker.h
  scalers/scal2014/TofScal2014Def.h  
  scalers/scalormu/TofScomDef.h  
  scalers/scalormu/TTofScomUnpacker.h   
  scalers/scal2014/TTofScal2014Unpacker.h
  tdc/TofTdcDef.h
  tdc/TTofTdcData.h
  tdc/TTofTdcBoard.h
  tdc/v1290/TofCaenDef.h
  tdc/vftx/TofVftxDef.h
  tdc/vftx/TTofVftxData.h
  tdc/vftx/TTofVftxBoard.h
  tdc/vftx/TTofVftxUnpacker.h
  tdc/trb/TofTrbTdcDef.h
  tdc/trb/TTofTrbTdcData.h
  tdc/trb/TTofTrbTdcBoard.h
  tdc/trb/TTofTrbTdcUnpacker.h
  tdc/get4/TofGet4Def.h
  tdc/get4/TGet4v1Event.h
  tdc/get4/TTofGet4Data.h
  tdc/get4/TTofGet4Board.h
  tdc/get4/TGet4v1Rec.h
  tdc/get4/TGet4v1Param.h
  tdc/get4/TTofGet4Unpacker.h
  unpMoni/TMbsUnpTofMonitor.h
  calib/tdc/TTofCalibData.h
  calib/tdc/TMbsCalibTdcTof.h
  calib/scaler/TTofCalibScaler.h
  calib/scaler/TMbsCalibScalTof.h
  calib/TMbsCalibTof.h
  calib/TMbsCalibTofPar.h
  mapping/TMbsMappingTof.h
  mapping/TMbsMappingTofPar.h
  output/MbsCrateDefines.h
  output/Plastics_Event.h
  output/Rpc_Event.h
  output/Scalers_Event.h
  output/TVftxBoardData.h
  output/TMbsConvTof.h
  output/TMbsConvTofPar.h
  display/CbmTofOnlineDisplay.h
)


#If(BUILD_BEAMTIME)
#  Set_Source_Files_Properties(PROPERTIES tdc/get4/TTofGet4Unpacker.cxx PROPERTIES
#                              OBJECT_DEPENDS ${LIBRARY_OUTPUT_PATH}/libRoc.so
#  )
#  set(ROC_DEPENDENCIES
##      FairTools MbsAPI 
#      ${LIBRARY_OUTPUT_PATH}/libDabcRoc.so
#      ${LIBRARY_OUTPUT_PATH}/libDabcMbs.so
#      ${LIBRARY_OUTPUT_PATH}/libDabcBase.so
#      ${LIBRARY_OUTPUT_PATH}/libRoc.so
#  )
#else(BUILD_BEAMTIME)
  set(ROC_DEPENDENCIES
      CbmRocLight
  )
#EndIf(BUILD_BEAMTIME)


Set(LINKDEF CbmTofUnpackLinkDef.h)
Set(LIBRARY_NAME CbmTofUnpack)
Set(DEPENDENCIES
    Base ParBase 
#    FairTools MbsAPI 
    CbmData CbmBase
#    ${LIBRARY_OUTPUT_PATH}/libDabcRoc.so
#    ${LIBRARY_OUTPUT_PATH}/libDabcMbs.so
#    ${LIBRARY_OUTPUT_PATH}/libDabcBase.so
#    ${LIBRARY_OUTPUT_PATH}/libRoc.so
#    RocLight
    ${ROC_DEPENDENCIES}
)

GENERATE_LIBRARY()
