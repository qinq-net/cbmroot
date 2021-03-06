set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/tof
  ${CBMROOT_SOURCE_DIR}/base
)

Set(SYSTEM_INCLUDE_DIRECTORIES
    ${SYSTEM_INCLUDE_DIRECTORIES}
    ${Boost_INCLUDE_DIR}
)

include_directories(${INCLUDE_DIRECTORIES})
include_directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
)

link_directories(${LINK_DIRECTORIES})

set(EXECUTABLE_OUTPUT_PATH
"${EXECUTABLE_OUTPUT_PATH}/tutorials/boost_serialization")

Set(BOOST_LIBS
  ${Boost_SERIALIZATION_LIBRARY}
)

set(EXE_NAME example1)
set(SRCS example1.cxx)
set(DEPENDENCIES
  ${DEPENDENCIES}
  ${BOOST_LIBS}
)
GENERATE_EXECUTABLE()

set(EXE_NAME example2)
set(SRCS example2.cxx)
set(DEPENDENCIES
  ${DEPENDENCIES}
  ${BOOST_LIBS}
  CbmData
  Base
  Core
)
GENERATE_EXECUTABLE()

set(EXE_NAME example3)
set(SRCS example3.cxx)
set(DEPENDENCIES
  ${DEPENDENCIES}
  ${BOOST_LIBS}
  CbmData
  Base
  Core
)
GENERATE_EXECUTABLE()

set(EXE_NAME example4)
set(SRCS example4.cxx)
set(DEPENDENCIES
  ${DEPENDENCIES}
  ${BOOST_LIBS}
  CbmData
  Base
  Core
)
GENERATE_EXECUTABLE()
