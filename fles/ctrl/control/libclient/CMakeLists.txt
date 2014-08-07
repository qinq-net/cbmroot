Set(INCLUDE_DIRECTORIES
${CBMROOT_CURRENT_SOURCE_DIR}
${CBMROOT_SOURCE_DIR}/fles/ctrl
${Boost_INCLUDE_DIR}
${ZMQ_INCLUDE_DIR}
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(LINK_DIRECTORIES
${Boost_LIBRARY_DIRS}
)
 
Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
ControlClient.cpp
ListPar.cpp
ListSeq.cpp
)


Set(LIBRARY_NAME cbmnetcntlclient)
Set(DEPENDENCIES boost_system zmq cbmnetutil)

GENERATE_LIBRARY()
