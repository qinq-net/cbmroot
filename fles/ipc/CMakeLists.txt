# Build the fles_ipc library which is tacken from git@code.compeng.uni-frankfurt.de:flesnet.git
# This library provides the interface to the FLIB board or to files in tca
# format written by the FLIB 
#
# This file is only called if the compiler provides c++11 support

# Add some extra compile warnings 
Set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Winit-self -Wundef -Wold-style-cast -Woverloaded-virtual -Wwrite-strings -Wnon-virtual-dtor")

Set(INCLUDE_DIRECTORIES
${CBMROOT_CURRENT_SOURCE_DIR}
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Include_Directories(SYSTEM ${Boost_INCLUDE_DIR})
 

Set(LINK_DIRECTORIES
${Boost_LIBRARY_DIRS}
)
 
Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
StorableTimeslice.cpp
System.cpp
TimesliceInputArchive.cpp
TimesliceReceiver.cpp
TimesliceView.cpp
MicrosliceContents.cpp
)


Set(LIBRARY_NAME fles_ipc)
Set(DEPENDENCIES boost_thread boost_system boost_serialization)

GENERATE_LIBRARY()
