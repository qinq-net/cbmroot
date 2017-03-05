# Create a library called "libBase" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/tof
${CBMROOT_SOURCE_DIR}/tof/TofMC
${CBMROOT_SOURCE_DIR}/tof/TofParam
${CBMROOT_SOURCE_DIR}/tof/TofReco
${CBMROOT_SOURCE_DIR}/tof/TofTools
${CBMROOT_SOURCE_DIR}/tof/TofData
${CBMROOT_SOURCE_DIR}/tof/TofDigi
${CBMROOT_SOURCE_DIR}/tof/TofTests
${CBMROOT_SOURCE_DIR}/tof/TofQA

${CBMROOT_SOURCE_DIR}/beamtime/tof
${CBMROOT_SOURCE_DIR}/beamtime/tof/tdc
${CBMROOT_SOURCE_DIR}/beamtime/tof/mapping

${CBMROOT_SOURCE_DIR}/base 
${CBMROOT_SOURCE_DIR}/eventdisplay

${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/tof

${CBMROOT_SOURCE_DIR}/run
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
)

link_directories( ${LINK_DIRECTORIES})

# For the time being compile tof beamtime related software only if the
# FairRoot version is v-15.11b => 15.11.2 
#If(FairRoot_VERSION VERSION_EQUAL 15.11.2)
  Set(TOF_BEAM_SOURCES 
    TofReco/CbmTofTestBeamClusterizer.cxx 
    TofTests/CbmTofAnaTestbeam.cxx 
  )
  Set(TOF_BEAM_DEPEND
    CbmTofUnpack
  )
  Set(DEFINITIONS ${DEFINITIONS} BUILD_TOF_BEAMTIME)
#EndIf()

set(SRCS
TofMC/CbmGeoTof.cxx 
TofMC/CbmTof.cxx 
TofMC/CbmTofMergeMcPoints.cxx 

TofParam/CbmGeoTofPar.cxx 
TofParam/CbmTofContFact.cxx 
TofParam/CbmTofDigiPar.cxx 
TofParam/CbmTofDigiBdfPar.cxx 
TofParam/CbmTofCreateDigiPar.cxx

#TofReco/CbmTofHitProducer.cxx 
#TofReco/CbmTofHitProducerIdeal.cxx 
#TofReco/CbmTofHitProducerNew.cxx 

TofTools/CbmTofGeoHandler.cxx

TofData/CbmTofCell.cxx

TofDigi/CbmTofDigitizerBDF.cxx

TofReco/CbmTofSimpClusterizer.cxx 
TofReco/CbmTofFindTracks.cxx 
TofReco/CbmTofTrackFinderNN.cxx 
TofReco/LKFMinuit.cxx 

TofTests/CbmTofTests.cxx 

TofQA/CbmTofGeometryQa.cxx
TofQA/CbmTofHitFinderQa.cxx

  ${TOF_BEAM_SOURCES}
)

set(LINKDEF CbmTofLinkDef.h)
Set(LIBRARY_NAME CbmTof)
Set(DEPENDENCIES
    CbmBase CbmData CbmDisplay Base 
    ${TOF_BEAM_DEPEND}
)

GENERATE_LIBRARY()
