configure_file(${CMAKE_CURRENT_SOURCE_DIR}/startMQSamplerUnpackerParserverHitBuilder.sh.in ${CMAKE_BINARY_DIR}/bin/MQ/topologies/startMQSamplerUnpackerParserverHitBuilder.sh)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/startMQ_Mcbm.sh.in ${CMAKE_BINARY_DIR}/bin/MQ/topologies/startMQ_Mcbm.sh)

set(INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CBMROOT_SOURCE_DIR}/fles/cern2016/param
    ${CBMROOT_SOURCE_DIR}/fles/cern2016/unpacker
    ${CBMROOT_SOURCE_DIR}/beamtime/base
    ${CBMROOT_SOURCE_DIR}/data
    ${CBMROOT_SOURCE_DIR}/data/tof
    ${CBMROOT_SOURCE_DIR}/base
    ${CBMROOT_SOURCE_DIR}/tof/TofData
    ${CBMROOT_SOURCE_DIR}/tof/TofParam
    ${CBMROOT_SOURCE_DIR}/tof/TofTools
)

Set(SYSTEM_INCLUDE_DIRECTORIES
    ${SYSTEM_INCLUDE_DIRECTORIES}
    ${ZeroMQ_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${FAIRROOT_INCLUDE_DIR}
    ${FAIRMQ_INCLUDE_DIR}
    ${FAIRMQ_INCLUDE_DIR}/options

    ${IPC_INCLUDE_DIRECTORY}
    ${CBMROOT_SOURCE_DIR}/external/cppzmq    
    ${CBMROOT_SOURCE_DIR}/tof/TofReco
)

include_directories(${INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
)

link_directories(${LINK_DIRECTORIES})

set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/MQ/hitbuilder")

Set(BOOST_LIBS
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_LOG_LIBRARY}
)
If(UNIX AND NOT APPLE)
  List(APPEND BOOST_LIBS pthread)
EndIf()

set(FAIR_LIBS
  Base
  ParBase
  FairMQ
)


If(FAIRLOGGER_FOUND)
  set(FAIR_LIBS
      ${FAIR_LIBS}
      FairLogger
     )
EndIf()

set(EXE_NAME HitBuilderTof)
set(SRCS CbmDeviceHitBuilderTof.cxx runHitBuilderTof.cxx)
set(DEPENDENCIES
  ${DEPENDENCIES}
#     FairMQ
  ${FAIR_LIBS}
  ${BOOST_LIBS}
#    fles_ipc
  CbmFlibCern2016
  CbmBase
  CbmData
  CbmTof	
  Geom		
  Core
  MathCore
  Tree
  Physics
  RIO
  Net
  Hist
)
GENERATE_EXECUTABLE()

