# Create a library called "libL1" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/base 

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/much
${CBMROOT_SOURCE_DIR}/data/mvd
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/data/tof
${CBMROOT_SOURCE_DIR}/data/rich
${CBMROOT_SOURCE_DIR}/data/global

${CBMROOT_SOURCE_DIR}/rich/fitter

${CBMROOT_SOURCE_DIR}/L1
${CBMROOT_SOURCE_DIR}/L1/L1Algo
${CBMROOT_SOURCE_DIR}/L1/OffLineInterface
${CBMROOT_SOURCE_DIR}/L1/ParticleFinder

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/KFQA
${CBMROOT_SOURCE_DIR}/KF/KFParticle
${CBMROOT_SOURCE_DIR}/KF/KFParticlePerformance
${CBMROOT_SOURCE_DIR}/KF/Interface
${CBMROOT_SOURCE_DIR}/run
${CBMROOT_SOURCE_DIR}/data

${CBMROOT_SOURCE_DIR}/sts
${CBMROOT_SOURCE_DIR}/mvd
${CBMROOT_SOURCE_DIR}/trd/param
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)


If(NOT ${ROOT_HAS_VC})
  set(SYSTEM_INCLUDE_DIRECTORIES
      ${SYSTEM_INCLUDE_DIRECTORIES}
      ${VC_INCLUDE_DIRS}
  )
EndIf()

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
${Vc_LIB_DIR}
)
 
link_directories( ${LINK_DIRECTORIES})


set(SRCS
#L1Algo/L1CATrackFinder.cxx 
#CbmL1Performance.cxx 
#CbmL1ReadEvent.cxx 
CbmL1CATrdTrackFinderSA.cxx 
CbmL1TrdTracklet.cxx 
CbmL1TrdTracklet4.cxx 
CbmL1.cxx 
#CbmL1TrdTrackFinderSts.cxx 
CbmL1TrackMerger.cxx 
CbmL1TofMerger.cxx
L1AlgoInputData.cxx
OffLineInterface/CbmL1RichENNRingFinder.cxx
OffLineInterface/CbmL1RichENNRingFinderParallel.cxx 
OffLineInterface/CbmL1StsTrackFinder.cxx 
#OffLineInterface/CbmL1MuchFinder.cxx 
#OffLineInterface/CbmL1MuchHit.cxx 
#OffLineInterface/CbmL1MuchTrack.cxx 
#OffLineInterface/CbmL1MuchFinderQa.cxx
OffLineInterface/CbmL1RichRingQa.cxx 
#OffLineInterface/CbmL1SttHit.cxx 
#OffLineInterface/CbmL1SttTrackFinder.cxx 
#OffLineInterface/CbmL1SttTrack.cxx 

L1Algo/L1Algo.cxx
L1Algo/L1CATrackFinder.cxx
L1Algo/L1TrackExtender.cxx
L1Algo/L1TrackFitter.cxx
L1Algo/L1CAMergeClones.cxx
L1Algo/L1HitsSortHelper.cxx
L1Algo/L1Grid.cxx
CbmL1Performance.cxx 
CbmL1ReadEvent.cxx
L1Algo/L1TrackParFit.cxx
L1Algo/L1Event.cxx
L1Algo/L1EventMatch.cxx
L1Algo/L1MCEvent.cxx

CbmL1MCTrack.cxx

ParticleFinder/CbmL1PFFitter.cxx
ParticleFinder/CbmL1PFMCParticle.cxx
)

set(HEADERS
CbmL1CATrdTrackFinderSA.h
CbmL1.h
CbmL1MCTrack.h
CbmL1MCPoint.h
CbmL1StsHit.h
CbmL1TofMerger.h
CbmL1Track.h
CbmL1TrackMerger.h
CbmL1TrackPar.h
CbmL1TrdHit.h
#CbmL1TrdTrackFinderSts.h
CbmL1TrdTracklet4.h
CbmL1TrdTracklet.h
CbmL1Vtx.h
#OffLineInterface/CbmL1MuchFinder.h
#OffLineInterface/CbmL1MuchFinderQa.h
#OffLineInterface/CbmL1MuchHit.h
#OffLineInterface/CbmL1MuchTrack.h
OffLineInterface/CbmL1RichENNRingFinder.h
OffLineInterface/CbmL1RichRingQa.h
OffLineInterface/CbmL1StsTrackFinder.h
#OffLineInterface/CbmL1SttHit.h
#OffLineInterface/CbmL1SttTrackFinder.h
#OffLineInterface/CbmL1SttTrack.h
# L1Algo/L1Algo.h
)

find_package(OpenMP)
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

If(APPLE)
  ADD_DEFINITIONS(-Wall -Wabi -Wsign-promo -Wctor-dtor-privacy -Wreorder -Wno-deprecated -Wno-parentheses -DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS) #   -Weffc++ -Wnon-virtual-dtor -Woverloaded-virtual -Wold-style-cast : wait for other parts of cbmroot\root. 
  #---Check for compiler flags
  CHECK_CXX_COMPILER_FLAG("-Wno-pmf-conversions" HAS_PMF)
  If(HAS_PMF)
    ADD_DEFINITIONS(-Wno-pmf-conversions)
  EndIf()
  CHECK_CXX_COMPILER_FLAG("-Wstrict-null-sentinel" HAS_SENTINEL)
  If(HAS_SENTINEL)
    ADD_DEFINITIONS(-Wstrict-null-sentinel)
  EndIf()
  CHECK_CXX_COMPILER_FLAG("-Wno-non-template-friend" HAS_TEMPLATE_FRIEND)
  If(HAS_TEMPLATE_FRIEND)
    ADD_DEFINITIONS(-Wno-non-template-friend)
  EndIf()

  if (OPENMP_FOUND)
    Execute_Process(COMMAND clang++ -print-prog-name=clang++
                  OUTPUT_VARIABLE _Exe)
    Get_Filename_Component(link_directory ${_Exe} DIRECTORY)
    Set(link_directory "${link_directory}/../lib") 
    Set(LINK_DIRECTORIES
      ${LINK_DIRECTORIES}
      ${link_directory}
    )
    link_directories( ${LINK_DIRECTORIES})
  endif()

Else()
  
  ADD_DEFINITIONS(-Wall -Wabi -fabi-version=0 -Wsign-promo  -Wno-pmf-conversions -Wctor-dtor-privacy -Wreorder -Wno-deprecated -Wstrict-null-sentinel -Wno-non-template-friend -Wno-parentheses -DDO_TPCCATRACKER_EFF_PERFORMANCE -DNonhomogeneousField -DCBM -DUSE_TIMERS) #   -Weffc++ -Wnon-virtual-dtor -Woverloaded-virtual -Wold-style-cast : wait for other parts of cbmroot\root. 

EndIf()


IF (SSE_FOUND)
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-msse -O3")
  Message(STATUS "L1 will be compiled with SSE support")
ELSE (SSE_FOUND)
  Message(STATUS "L1 will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS 
  "-O3")
ENDIF (SSE_FOUND)

set(LINKDEF L1LinkDef.h)
Set(LIBRARY_NAME L1)
Set(DEPENDENCIES
    #Rich 
    KF CbmTrd CbmSts CbmBase CbmData Base boost_regex
)

GENERATE_LIBRARY()

