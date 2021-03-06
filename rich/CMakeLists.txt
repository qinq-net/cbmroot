# Create a library called "libRich" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/rich
${CBMROOT_SOURCE_DIR}/rich/fitter
${CBMROOT_SOURCE_DIR}/rich/finder
#${CBMROOT_SOURCE_DIR}/rich/qa
${CBMROOT_SOURCE_DIR}/rich/tracks
${CBMROOT_SOURCE_DIR}/rich/selection
${CBMROOT_SOURCE_DIR}/rich/detector
${CBMROOT_SOURCE_DIR}/rich/utils
# needed by rootcling
${CBMROOT_SOURCE_DIR}/rich/prototype/beamtime

${CBMROOT_SOURCE_DIR}/base
${CBMROOT_SOURCE_DIR}/base/report
${CBMROOT_SOURCE_DIR}/base/draw
${CBMROOT_SOURCE_DIR}/base/utils

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/rich
${CBMROOT_SOURCE_DIR}/data/raw
${CBMROOT_SOURCE_DIR}/data/trd
${CBMROOT_SOURCE_DIR}/data/tof
${CBMROOT_SOURCE_DIR}/data/sts
${CBMROOT_SOURCE_DIR}/data/global

${CBMROOT_SOURCE_DIR}/littrack
${CBMROOT_SOURCE_DIR}/littrack/std
${CBMROOT_SOURCE_DIR}/littrack/cbm
${CBMROOT_SOURCE_DIR}/littrack/cbm/utils

${CBMROOT_SOURCE_DIR}/KF
${CBMROOT_SOURCE_DIR}/KF/Interface

${CBMROOT_SOURCE_DIR}/L1/OffLineInterface

${CBMROOT_SOURCE_DIR}/run
)

Include_Directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES}
  ${Boost_INCLUDE_DIR}
)

If(TBB_FOUND)
  Set(SYSTEM_INCLUDE_DIRECTORIES
      ${SYSTEM_INCLUDE_DIRECTORIES}
      ${TBB_INCLUDE_DIRS}
     )
EndIf(TBB_FOUND)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})


set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
${Vc_LIB_DIR}
${KFParticle_LIB_DIR}
)

If(TBB_FOUND)
  Set(LINK_DIRECTORIES
      ${LINK_DIRECTORIES}
      ${TBB_LIBRARY_DIRS}
     )
EndIf(TBB_FOUND)

link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmGeoRich.cxx
CbmRich.cxx
CbmRichEventDisplay.cxx
CbmGeoRichPar.cxx
CbmRichContFact.cxx
CbmRichElectronIdAnn.cxx
CbmRichTrainAnnSelect.cxx
CbmRichTrainAnnElectrons.cxx
CbmRichReconstruction.cxx
CbmRichConverter.cxx
detector/CbmRichHitProducer.cxx
detector/CbmRichDigitizer.cxx
detector/CbmRichDigiMapManager.cxx
detector/CbmRichPmt.cxx
detector/CbmRichGeoManager.cxx
finder/CbmRichRingFinderIdeal.cxx
#finder/CbmRichRingFinderHough.cxx
finder/CbmRichRingFinderHoughImpl.cxx
fitter/CbmRichRingFitterCircle.cxx
fitter/CbmRichRingFitterCOP.cxx
fitter/CbmRichRingFitterRobustCOP.cxx
fitter/CbmRichRingFitterTAU.cxx
fitter/CbmRichRingFitterEllipseMinuit.cxx
fitter/CbmRichRingFitterEllipseTau.cxx
tracks/CbmRichProjectionProducerAnalytical.cxx
tracks/CbmRichProjectionProducerTGeo.cxx
tracks/CbmRichRingTrackAssignClosestD.cxx
tracks/CbmRichRingTrackAssignIdeal.cxx
tracks/CbmRichTrackExtrapolationIdeal.cxx
tracks/CbmRichTrackExtrapolationKF.cxx
tracks/CbmRichTrackExtrapolationLittrack.cxx
tracks/CbmRichTrackExtrapolationMirrorIdeal.cxx
qa/CbmRichTestSim.cxx
qa/CbmRichTestHits.cxx
qa/CbmRichRingFitterQa.cxx
qa/CbmRichGeoOpt.cxx
qa/CbmRichRecoTbQa.cxx
#qa/CbmRichParallelQa.cxx
selection/CbmRichRingSelectAnn.cxx
prototype/CbmRichProt.cxx
prototype/CbmRichProtHitProducer.cxx
prototype/CbmRichProtPrepareExtrapolation.cxx
prototype/CbmRichProtProjectionProducer.cxx
prototype/CbmRichProtRingFinderHough.cxx
prototype/CbmRichProtRingFinderHoughImpl.cxx
prototype/CbmRichPrototypeQa.cxx
prototype/smallprototype/CbmRichSmallPrototypeQa.cxx
prototype/smallprototype/CbmRichSmallPrototypeStudyReport.cxx
mcbm/CbmRichMCbmQa.cxx
qa/CbmRichGeoTest.cxx
qa/CbmRichUrqmdTest.cxx
qa/CbmRichGeoTestStudyReport.cxx
qa/CbmRichRecoQa.cxx
#alignment/CbmRichMirror.cxx
#alignment/CbmRichMirrorSortingAlignment.cxx
#alignment/CbmRichMirrorSortingCorrection.cxx
prototype/beamtime/CbmRichTrbUnpack.cxx
prototype/beamtime/CbmRichTrbUnpack2.cxx
prototype/beamtime/CbmTrbEdgeMatcher.cxx
prototype/beamtime/CbmTrbCalibrator.cxx
prototype/beamtime/CbmRichTrbEventBuilder.cxx
prototype/beamtime/CbmRichProtoAnalysis.cxx
prototype/beamtime/CbmRichEventDebug.cxx
prototype/beamtime/CbmRichRingHitsAnalyser.cxx
prototype/beamtime/CbmRichTrbRecoQa.cxx
prototype/beamtime/CbmRichTrbPulserQa.cxx
prototype/beamtime/CbmRichHitInfo.cxx
prototype/beamtime/CbmRichTrbRecoQaStudyReport.cxx
#mirrors/CbmRichRonchiAna.cxx
)

If(TBB_FOUND)
  Set(SRCS
      ${SRCS}
      finder/CbmRichRingFinderHoughParallel.cxx
     )
EndIf(TBB_FOUND)


set(HEADERS
CbmGeoRich.h
CbmRich.h
CbmRichEventDisplay.h
CbmGeoRichPar.h
CbmRichContFact.h
CbmRichTrainAnnSelect.h
CbmRichTrainAnnElectrons.h
CbmRichReconstruction.h
detector/CbmRichHitProducer.h
detector/CbmRichDigitizer.h
qa/CbmRichTestSim.h
qa/CbmRichTestHits.h
qa/CbmRichRingFitterQa.h
#qa/CbmRichParallelQa.h
finder/CbmRichRingFinderHough.cxx
prototype/CbmRichProt.h
prototype/CbmRichProtHitProducer.h
prototype/CbmRichProtPrepareExtrapolation.h
prototype/CbmRichProtProjectionProducer.h
prototype/CbmRichPrototypeQa.h
mcbm/CbmRichMCbmQa.h
qa/CbmRichGeoTest.h
qa/CbmRichUrqmdTest.h
qa/CbmRichGeoOpt.h
qa/CbmRichRecoQa.h
qa/CbmRichRecoTbQa.h
#alignment/CbmRichMirror.h
#alignment/CbmRichMirrorSortingAlignment.h
#alignment/CbmRichMirrorSortingCorrection.h
#alignment/CbmRichMirrorMisalignmentCorrectionUtils.h
prototype/beamtime/CbmRichTrbUnpack.h
prototype/beamtime/CbmRichTrbUnpack2.h
prototype/beamtime/CbmTrbEdgeMatcher.h
prototype/beamtime/CbmTrbCalibrator.h
prototype/beamtime/CbmRichTrbEventBuilder.h
prototype/beamtime/CbmRichProtoAnalysis.h
prototype/beamtime/CbmRichEventDebug.h
prototype/beamtime/CbmRichRingHitsAnalyser.h
prototype/beamtime/CbmRichTrbRecoQa.h
prototype/beamtime/CbmRichTrbPulserQa.h
prototype/beamtime/CbmRichHitInfo.h
prototype/beamtime/CbmRichTrbRecoQaStudyReport.h
prototype/smallprototype/CbmRichSmallPrototypeQa.h
prototype/smallprototype/CbmRichSmallPrototypeStudyReport.h
#mirrors/CbmRichRonchiAna.h
)

If(TBB_FOUND)
  Set(HEADERS
      ${HEADERS}
      finder/CbmRichRingFinderHoughParallel.h
     )
EndIf(TBB_FOUND)

IF (SSE_FOUND)
  ADD_DEFINITIONS(-DHAVE_SSE)
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS "-msse -O3")
  Message(STATUS "Rich will be compiled with SSE support")
ELSE (SSE_FOUND)
  Message(STATUS "Rich will be compiled without SSE support")
  SET_SOURCE_FILES_PROPERTIES(${SRCS} PROPERTIES COMPILE_FLAGS "-O3")
ENDIF (SSE_FOUND)

# END SSE

If(TBB_FOUND)
  set(LINKDEF  CbmRichLinkDef_tbb.h)
Else(TBB_FOUND)
  set(LINKDEF  CbmRichLinkDef.h)
EndIf(TBB_FOUND)

Set(LIBRARY_NAME CbmRich)
Set(DEPENDENCIES
    # KF L1 ????
    KF L1 CbmBase CbmData Base MLP boost_regex Gdml)

If(TBB_FOUND)
  Set(DEPENDENCIES
      ${DEPENDENCIES} ${TBB_LIBRARIES}
  )
EndIf(TBB_FOUND)

GENERATE_LIBRARY()
