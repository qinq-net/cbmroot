# Build the softdpb executable
# This executable uses the fles_ipc library to read files
# in the tsa format
# This file is only called if the compiler provides c++11 support

# Add some extra compile warnings 
Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Winit-self -Wundef -Wold-style-cast -Woverloaded-virtual -Wwrite-strings -Wnon-virtual-dtor")

Set(INCLUDE_DIRECTORIES
${CBMROOT_CURRENT_SOURCE_DIR}
${CBMROOT_SOURCE_DIR}/fles/ipc
${Boost_INCLUDE_DIR}
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(LINK_DIRECTORIES
${Boost_LIBRARY_DIRS}
)
 
Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
main.cpp
Application.cpp
Parameters.cpp
TimesliceAnalyzer.cpp
TimesliceDebugger.cpp
)

Set(EXE_NAME softdpb)
If(UNIX AND NOT APPLE)
  Set(DEPENDENCIES fles_ipc boost_thread boost_system boost_serialization boost_program_options ${CMAKE_THREAD_LIBS_INIT} rt)
Else()
  Set(DEPENDENCIES fles_ipc boost_thread boost_system boost_serialization boost_program_options ${CMAKE_THREAD_LIBS_INIT})
EndIf()

GENERATE_EXECUTABLE()

