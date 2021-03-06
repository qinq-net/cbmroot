Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/tracking/BinnedTracker
  ${CBMROOT_SOURCE_DIR}/base 
  ${CBMROOT_SOURCE_DIR}/data
  ${CBMROOT_SOURCE_DIR}/data/sts
  ${CBMROOT_SOURCE_DIR}/data/much
  ${CBMROOT_SOURCE_DIR}/data/trd
  ${CBMROOT_SOURCE_DIR}/data/tof
  ${CBMROOT_SOURCE_DIR}/sts
  ${CBMROOT_SOURCE_DIR}/much
  ${CBMROOT_SOURCE_DIR}/run
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES}
  #${Boost_INCLUDE_DIRS}
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

Set(LINK_DIRECTORIES
  ${ROOT_LIBRARY_DIR}
  ${FAIRROOT_LIBRARY_DIR}
  ${Boost_LIBRARY_DIRS}
)
 
Link_Directories( ${LINK_DIRECTORIES})

Set(SRCS
  CbmBinnedTrackerConfigurator.cxx
  CbmBinnedTrackerTask.cxx
  CbmBinnedTrackerQA.cxx
  CbmBinnedTrackerQANew.cxx
  ContFactory.cxx
  Settings.cxx
  CbmTrackParam2.cxx
)

Set(NO_DICT_SRCS
  GeoReader.cxx
  HitReader.cxx
  Station.cxx
  Station3D.cxx
  Station4D.cxx
  Tracker.cxx
)

set(LINKDEF CbmBinnedTrackerLinkDef.h)
Set(LIBRARY_NAME CbmBinnedTracker)
Set(DEPENDENCIES
    CbmBase CbmData Base
)

GENERATE_LIBRARY()
