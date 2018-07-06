# Create a library called "libCbmTrdPid" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/trd/rec/pid
  ${CBMROOT_SOURCE_DIR}/trd/base

  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/trd

  ${CBMROOT_SOURCE_DIR}/base
  ${CBMROOT_SOURCE_DIR}/base/draw

  ${CBMROOT_SOURCE_DIR}/L1/vectors

  ${CBMROOT_SOURCE_DIR}/analysis/papa
  ${CBMROOT_SOURCE_DIR}/rich
  ${CBMROOT_SOURCE_DIR}/data/tof 
  ${CBMROOT_SOURCE_DIR}/data/rich 
  ${CBMROOT_SOURCE_DIR}/data/sts
  ${CBMROOT_SOURCE_DIR}/data/mvd 
  ${CBMROOT_SOURCE_DIR}/data/much 
  ${CBMROOT_SOURCE_DIR}/data/global
  ${CBMROOT_SOURCE_DIR}/KF/Interface
  ${CBMROOT_SOURCE_DIR}/KF
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
  ${Boost_INCLUDE_DIR}
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
${Vc_LIB_DIR}
${KFParticle_LIB_DIR}
) 

link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmTrdElectronsTrainAnn.cxx
CbmTrdSetTracksPidWkn.cxx   
CbmTrdSetTracksPidModWkn.cxx  
CbmTrdSetTracksPidLike.cxx     
CbmTrdSetTracksPidANN.cxx       
)

IF (SSE_FOUND)
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-msse -O3")
  Message(STATUS "TRD PID will be compiled with SSE support")
ELSE (SSE_FOUND)
  Message(STATUS "TRD PID will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-O3")
ENDIF (SSE_FOUND)



set(LINKDEF CbmTrdPidLinkDef.h)
Set(LIBRARY_NAME CbmTrdPid)
Set(DEPENDENCIES
    CbmBase CbmData CbmPapa Base TMVA CbmTrd CbmTrdRec
)
GENERATE_LIBRARY()
