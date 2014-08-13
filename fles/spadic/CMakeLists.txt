# Build the fles_ipc library which is tacken from git@code.compeng.uni-frankfurt.de:flesnet.git
# This library provides the interface to the FLIB board or to files in tca
# format written by the FLIB 
#
# This file is only called if the compiler provides c++11 support

# Add some extra compile warnings 
Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Winit-self -Wundef -Wold-style-cast -Woverloaded-virtual -Wwrite-strings -Wnon-virtual-dtor")

Set(INCLUDE_DIRECTORIES
${CBMROOT_CURRENT_SOURCE_DIR}
${CBMROOT_SOURCE_DIR}/fles/ipc
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(LINK_DIRECTORIES
)
 
Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
message.c
message_reader.c
Message.cpp
)


Set(LIBRARY_NAME spadicMessage)
Set(DEPENDENCIES)

GENERATE_LIBRARY()

#Set(SRCS
#TimesliceReader.cpp
#timeslice_reader.cpp
#)

#Set(EXE_NAME timeslice_reader)
#Set(DEPENDENCIES fles_ipc boost_thread boost_system boost_serialization spadicMessage)

#GENERATE_EXECUTABLE()

