configure_file(${CMAKE_CURRENT_SOURCE_DIR}/startMQSamplerSink.sh.in ${CMAKE_BINARY_DIR}/bin/MQ/topologies/startMQSamplerSink.sh)

set(INCLUDE_DIRECTORIES
    ${CMAKE_CURRENT_SOURCE_DIR}
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

set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/MQ/sink")

Set(BOOST_LIBS
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_SERIALIZATION_LIBRARY}
  ${Boost_PROGRAM_OPTIONS_LIBRARY}
  ${Boost_LOG_LIBRARY}
)
If(UNIX AND NOT APPLE)
  List(APPEND BOOST_LIBS pthread)
EndIf()


set(EXE_NAME DevNullSink)
set(SRCS CbmDevNullSink.cxx runDevNullSink.cxx)
set(DEPENDENCIES
  ${DEPENDENCIES}
  FairMQ
  ${BOOST_LIBS}
  fles_ipc
)
GENERATE_EXECUTABLE()

set(EXE_NAME TsaComponentSink)
set(SRCS CbmTsaComponentSink.cxx runTsaComponentSink.cxx)
set(DEPENDENCIES
  ${DEPENDENCIES}
  FairMQ
  ${BOOST_LIBS}
  fles_ipc
)
GENERATE_EXECUTABLE()

