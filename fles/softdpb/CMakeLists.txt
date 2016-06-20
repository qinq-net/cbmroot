# Build the softdpb executable
# This executable uses the fles_ipc library to read files
# in the tsa format
# This file is only called if the compiler provides c++11 support

Set(INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Include_Directories(SYSTEM 
  ${Boost_INCLUDE_DIR}
  ${ZMQ_INCLUDE_DIR}
  ${CBMROOT_SOURCE_DIR}/external/ipc_legacy/ipc/src
  ${CBMROOT_SOURCE_DIR}/external/flib_dpb/flib_dpb
)

Set(LINK_DIRECTORIES
  ${Boost_LIBRARY_DIRS}
  ${ZMQ_LIBRARY_DIR}
)
 
Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
main.cpp
Application.cpp
Parameters.cpp
TransparentFilter.cpp
)

Set_Source_Files_Properties(${SRCS} COMPILE_FLAGS "-Wall -pedantic -Wextra -Winit-self -Wundef -Wold-style-cast -Woverloaded-virtual -Wwrite-strings -Wnon-virtual-dtor")

Set(EXE_NAME softdpb)
If(UNIX AND NOT APPLE)
  Set(DEPENDENCIES fles_ipc_legacy flib_dpb boost_thread boost_system boost_serialization boost_program_options ${CMAKE_THREAD_LIBS_INIT} zmq rt)
Else()
  Set(DEPENDENCIES fles_ipc_legacy flib_dpb boost_thread boost_system boost_serialization boost_program_options ${CMAKE_THREAD_LIBS_INIT} zmq)
EndIf()

GENERATE_EXECUTABLE()
