# Create a library called "libBase" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed
# here.

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/tracking/vector
${CBMROOT_SOURCE_DIR}/base 
${CBMROOT_SOURCE_DIR}/run 
${CBMROOT_SOURCE_DIR}/data 
${CBMROOT_SOURCE_DIR}/data/much 
${CBMROOT_SOURCE_DIR}/much
${CBMROOT_SOURCE_DIR}/much/geo
${CBMROOT_SOURCE_DIR}/much/reco
${CBMROOT_SOURCE_DIR}/much/tracking
${CBMROOT_SOURCE_DIR}/trd/param
${CBMROOT_SOURCE_DIR}/trd/data

# needed by run/FairMCBuffer.h
${CBMROOT_SOURCE_DIR}/data/mvd 
${CBMROOT_SOURCE_DIR}/data/rich 
${CBMROOT_SOURCE_DIR}/data/sts 
${CBMROOT_SOURCE_DIR}/data/trd 
${CBMROOT_SOURCE_DIR}/data/tof 
${CBMROOT_SOURCE_DIR}/data/ecal 
${CBMROOT_SOURCE_DIR}/data/psd 

# needed by CbmMuchFindVectors
${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
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
CbmVectorFinder.cxx
CbmTrdFindVectors.cxx
CbmTrdToTofVector.cxx
CbmMuchFindVectors.cxx
CbmMuchFindVectorsGem.cxx
#CbmMuchFindVectorsQA.cxx
CbmMuchMergeVectors.cxx
#CbmMuchMergeVectorsQA.cxx
CbmMuchToTrdVectors.cxx
)

#AZ - debug
#SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS
#  "-gdwarf-2 -O0")

set(LINKDEF CbmVectorLinkDef.h)
Set(LIBRARY_NAME CbmVector)
Set(DEPENDENCIES
    CbmBase CbmData Base KF
)

GENERATE_LIBRARY()

