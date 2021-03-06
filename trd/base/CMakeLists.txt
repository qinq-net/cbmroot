# Create a library called "libTrd" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
# needed by run/FairMCBuffer.h
${CBMROOT_SOURCE_DIR}/base
${CBMROOT_SOURCE_DIR}/data 
${CBMROOT_SOURCE_DIR}/data/trd 
${CBMROOT_SOURCE_DIR}/trd/base
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
) 

link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmGeoTrd.cxx 
CbmGeoTrdPar.cxx 
CbmTrdGas.cxx 
CbmTrdContFact.cxx 
CbmTrdParManager.cxx 
CbmTrdModuleAbstract.cxx 
CbmTrdParSet.cxx 
CbmTrdParSetAsic.cxx 
CbmTrdParSetGas.cxx 
CbmTrdParSetGain.cxx 
CbmTrdParSetGeo.cxx 
CbmTrdParSetDigi.cxx 
CbmTrdParMod.cxx 
CbmTrdParAsic.cxx 
CbmTrdParSpadic.cxx 
CbmTrdSPADIC.cxx 
CbmTrdFASP.cxx 
CbmTrdParFasp.cxx 
CbmTrdParModDigi.cxx 
CbmTrdParModGas.cxx 
CbmTrdParModGain.cxx 
CbmTrdParModGeo.cxx 
#tools
CbmTrdGeoHandler.cxx
CbmTrdUtils.cxx
)

IF (SSE_FOUND)
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-msse -O3")
  Message(STATUS "TRD BASE will be compiled with SSE support")
ELSE (SSE_FOUND)
  Message(STATUS "TRD BASE will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-O3")
ENDIF (SSE_FOUND)



set(LINKDEF CbmTrdBaseLinkDef.h)
Set(LIBRARY_NAME CbmTrd)
Set(DEPENDENCIES
    CbmBase Base
)

GENERATE_LIBRARY()
