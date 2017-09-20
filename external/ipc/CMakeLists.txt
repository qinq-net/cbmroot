# Build the fles_ipc library which is tacken from
# github as defined in the svn:externals definition
# This library provides the interface to the FLIB board or to files in tca
# format written by the FLIB

include(DownloadProject)
download_project(PROJ            fles_ipc
                 GIT_REPOSITORY  "https://github.com/cbm-fles/flesnet"
                 GIT_TAG         "21d9038ff9c97188499b6225da6971b42336e349"
                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/ipc
)

Set(INCLUDE_DIRECTORIES
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${Boost_INCLUDE_DIR}
  ${ZMQ_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/external/cppzmq
  ${CMAKE_SOURCE_DIR}/external/ipc/ipc
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})


Set(LINK_DIRECTORIES
  ${Boost_LIBRARY_DIRS}
  ${ZMQ_LIBRARY_DIRS}
)

Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
ipc/lib/fles_ipc/Microslice.cpp
ipc/lib/fles_ipc/MicrosliceView.cpp
ipc/lib/fles_ipc/StorableMicroslice.cpp
ipc/lib/fles_ipc/StorableTimeslice.cpp
ipc/lib/fles_ipc/System.cpp
ipc/lib/fles_ipc/Timeslice.cpp
ipc/lib/fles_ipc/TimeslicePublisher.cpp
ipc/lib/fles_ipc/TimesliceReceiver.cpp
ipc/lib/fles_ipc/TimesliceSubscriber.cpp
ipc/lib/fles_ipc/TimesliceView.cpp
)

Set_Source_Files_Properties(${SRCS} COMPILE_FLAGS "-std=c++11 -O3 -ggdb -msse4.2 -Wall -Wpedantic -Wextra -Winit-self -Wundef -Wold-style-cast -Woverloaded-virtual -Wwrite-strings -Wnon-virtual-dtor -fno-omit-frame-pointer")

Set(LIBRARY_NAME fles_ipc)
If(UNIX AND NOT APPLE)
  Set(DEPENDENCIES boost_thread boost_system boost_serialization zmq rt)
Else()
  Set(DEPENDENCIES boost_thread boost_system boost_serialization zmq)
EndIf()


GENERATE_LIBRARY()
