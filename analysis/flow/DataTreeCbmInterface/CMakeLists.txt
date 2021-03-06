Set(INCLUDE_DIRECTORIES

  ${CBMROOT_SOURCE_DIR}/analysis/flow/DataTreeCbmInterface

  ${CBMROOT_SOURCE_DIR}/base

  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/sts
  ${CBMROOT_SOURCE_DIR}/data/global
  ${CBMROOT_SOURCE_DIR}/data/psd
  ${CBMROOT_SOURCE_DIR}/data/tof

  ${CBMROOT_SOURCE_DIR}/generators

  ${CBMROOT_SOURCE_DIR}/KF
  ${CBMROOT_SOURCE_DIR}/KF/Interface 
  ${CBMROOT_SOURCE_DIR}/KF/KFQA

  ${KFParticle_INCLUDE_DIR}
  ${DataTree_INCLUDE_DIR}

  ${CBMROOT_SOURCE_DIR}/L1
  ${CBMROOT_SOURCE_DIR}/L1/L1Algo
  ${CBMROOT_SOURCE_DIR}/L1/ParticleFinder 
)

Include_Directories (${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${VC_INCLUDE_DIRS}
  ${BASE_INCLUDE_DIRECTORIES}
  ${Boost_INCLUDE_DIR} 
)

Include_Directories (SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set (LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
  ${Vc_LIB_DIR}
  ${KFParticle_LIB_DIR}
  ${DataTree_LIB_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})

set (SRCS
  DataTreeCbmInterface.cxx
)

set (HEADERS
  DataTreeCbmInterface.h
)

IF (SSE_FOUND)
  Message(STATUS "DataTreeCbmInterface will be compiled with SSE support")
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-msse -O3")
ELSE (SSE_FOUND)
  MESSAGE(STATUS "DataTreeCbmInterface will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-O3")
ENDIF (SSE_FOUND)

Set(LINKDEF DataTreeCbmInterfaceLinkDef.h)
Set(LIBRARY_NAME DataTreeCbmInterface)

If(UNIX AND NOT APPLE)
  Set(_DataTree_LIB  DataTree.so)
  Else()
  Set(_DataTree_LIB  DataTree.dylib)
EndIf()

Set(DEPENDENCIES 
	${_DataTree_LIB} 
	CbmData 
	CbmBase 
	CbmGenerators 
	KF 
	L1 
	Vc.a)
Set(DEFINITIONS -DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS -DDATATREE_CBM)

ADD_DEFINITIONS(${DEFINITIONS})

GENERATE_LIBRARY()

Add_Dependencies(DataTreeCbmInterface DATATREE)
