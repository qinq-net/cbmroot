# Create a library called "libRun" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/base

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/mvd
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/rich
${CBMROOT_SOURCE_DIR}/data/much
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/data/tof
${CBMROOT_SOURCE_DIR}/data/ecal
${CBMROOT_SOURCE_DIR}/data/psd

${CBMROOT_SOURCE_DIR}/mvd
${CBMROOT_SOURCE_DIR}/sts/mc
${CBMROOT_SOURCE_DIR}/rich
${CBMROOT_SOURCE_DIR}/much/geo
${CBMROOT_SOURCE_DIR}/trd/mc
${CBMROOT_SOURCE_DIR}/tof/TofMC
${CBMROOT_SOURCE_DIR}/ecal
${CBMROOT_SOURCE_DIR}/psd
${CBMROOT_SOURCE_DIR}/passive
${CBMROOT_SOURCE_DIR}/field

${CBMROOT_SOURCE_DIR}/run
)

Include_Directories( ${INCLUDE_DIRECTORIES})

set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
)
 
link_directories( ${LINK_DIRECTORIES})

Execute_Process(COMMAND $ENV{FAIRROOTPATH}/bin/fairroot-config --version
                OUTPUT_VARIABLE _fairroot_version
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
String(REGEX MATCH "v-([0-9]+)\\.([0-9]+)([a-z]*)" _version_matches "${_fairroot_version}")
Set(FairRoot_VERSION_MAJOR ${CMAKE_MATCH_1})
Set(FairRoot_VERSION_MINOR ${CMAKE_MATCH_2})
Set(FairRoot_VERSION_PATCH ${CMAKE_MATCH_3})
Set(_fairroot_version_found ${FairRoot_VERSION_MAJOR}.${FairRoot_VERSION_MINOR})
If(_fairroot_version_found VERSION_LESS 15.11)
 Set(DEFINITIONS -DOLDFAIRROOTVERSION)
 Add_Definitions(-DOLDFAIRROOTVERSION) 
EndIf()

set(SRCS

CbmBuildEventsSimple.cxx
CbmDaq.cxx
CbmDaqQa.cxx
CbmDaqBuffer.cxx
CbmMCBuffer.cxx
CbmMCTimeSim.cxx
CbmTimeSlice.cxx

CbmMCStreamer.cxx
CbmRunAna.cxx
#CbmRunOnline.cxx
CbmMCStreamerQa.cxx

CbmSetup.cxx

)

set(LINKDEF CbmRunLinkDef.h)
Set(LIBRARY_NAME CbmRun)
Set(DEPENDENCIES
    CbmData CbmBase CbmField CbmMuch CbmMvd Base 
)

GENERATE_LIBRARY()
