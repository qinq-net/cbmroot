# Create a library called "libLxTrack" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed
# here.

Set(INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${CBMROOT_SOURCE_DIR}/tracking/lxTriplet
  ${CBMROOT_SOURCE_DIR}/base 
  ${CBMROOT_SOURCE_DIR}/data 
  ${CBMROOT_SOURCE_DIR}/data/much 
  ${CBMROOT_SOURCE_DIR}/data/sts 
  ${CBMROOT_SOURCE_DIR}/data/global
  ${CBMROOT_SOURCE_DIR}/much/geo
  ${CBMROOT_SOURCE_DIR}/KF
  ${CBMROOT_SOURCE_DIR}/KF/Interface
  ${CBMROOT_SOURCE_DIR}/sts
  ${CBMROOT_SOURCE_DIR}/littrack/std
  ${CBMROOT_SOURCE_DIR}/littrack/cbm
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${Boost_INCLUDE_DIRS}
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

Set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
)
 
Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
  Lx.cxx
  LxTrackAna.cxx
)

find_package(OpenMP)

Set(NO_DICT_SRCS
  LxCATriplets.cxx
  LxEff.cxx
  LxMC.cxx
  LxTrackAnaSegments.cxx
  LxUtils.cxx
)

If (OPENMP_FOUND)
  Set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx -fopenmp" )
EndIf (OPENMP_FOUND)

set(LINKDEF LxLinkDef.h)
Set(LIBRARY_NAME LxTrackTripplet)
Set(DEPENDENCIES
    CbmBase CbmData Base boost_regex
)

GENERATE_LIBRARY()
