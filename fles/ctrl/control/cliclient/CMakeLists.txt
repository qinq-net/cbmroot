Set(INCLUDE_DIRECTORIES
${CBMROOT_CURRENT_SOURCE_DIR}
${CBMROOT_SOURCE_DIR}/fles/ctrl
)

Set(SYSTEM_INCLUDE_DIRECTORIES
${Boost_INCLUDE_DIR}
${ZMQ_INCLUDE_DIR}
${CMAKE_SOURCE_DIR}/external/cppzmq
)

Include_Directories( ${INCLUDE_DIRECTORIES})
Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

Set(LINK_DIRECTORIES
${Boost_LIBRARY_DIRS}
)
 
Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
cliclient.cpp
)


Set(EXE_NAME cliclient)
Set(DEPENDENCIES boost_system zmq cbmnetutil cbmnetcntlclient)

GENERATE_EXECUTABLE()
