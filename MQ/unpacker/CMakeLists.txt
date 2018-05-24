configure_file(${CMAKE_CURRENT_SOURCE_DIR}/startMQSamplerUnpackerParserver.sh.in ${CMAKE_BINARY_DIR}/bin/MQ/topologies/startMQSamplerUnpackerParserver.sh)

set(INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CBMROOT_SOURCE_DIR}/fles/cern2016/param
    ${CBMROOT_SOURCE_DIR}/fles/cern2016/unpacker
    ${CBMROOT_SOURCE_DIR}/beamtime/base
    ${CBMROOT_SOURCE_DIR}/data
    ${CBMROOT_SOURCE_DIR}/data/tof
    ${CBMROOT_SOURCE_DIR}/base
)

Set(SYSTEM_INCLUDE_DIRECTORIES
    ${SYSTEM_INCLUDE_DIRECTORIES}
    ${ZeroMQ_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${FAIRROOT_INCLUDE_DIR}
    ${FAIRROOT_INCLUDE_DIR}/fairmq
    ${FAIRROOT_INCLUDE_DIR}/fairmq/options

    ${IPC_INCLUDE_DIRECTORY}
    ${CBMROOT_SOURCE_DIR}/external/cppzmq
)

include_directories(${INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
)

link_directories(${LINK_DIRECTORIES})

set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/MQ/unpacker")

Set(BOOST_LIBS
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_LOG_LIBRARY}
)
If(UNIX AND NOT APPLE)
  List(APPEND BOOST_LIBS pthread)
EndIf()


set(EXE_NAME UnpackTofStar2018)
set(SRCS CbmDeviceUnpackTofStar2018.cxx runUnpackTofStar2018.cxx)
set(DEPENDENCIES
  ${DEPENDENCIES}
  FairMQ
  ${BOOST_LIBS}
  fles_ipc
  CbmFlibCern2016
  CbmBeamtime
  CbmBase
  CbmData
  Core
  RIO
  Net
  Hist
)
GENERATE_EXECUTABLE()

