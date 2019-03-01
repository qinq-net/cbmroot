## Create a library called "libRun" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
${SIMPATH}/include/Geant4
${SIMPATH}/include/geant4vmc
${SIMPATH}/include/TGeant3
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
${CBMROOT_SOURCE_DIR}/data/global

${CBMROOT_SOURCE_DIR}/mvd
${CBMROOT_SOURCE_DIR}/sts/mc
${CBMROOT_SOURCE_DIR}/sts/digitize
${CBMROOT_SOURCE_DIR}/rich
${CBMROOT_SOURCE_DIR}/rich/detector
${CBMROOT_SOURCE_DIR}/much/geo
${CBMROOT_SOURCE_DIR}/much/reco
${CBMROOT_SOURCE_DIR}/trd/sim
${CBMROOT_SOURCE_DIR}/tof/TofMC
${CBMROOT_SOURCE_DIR}/tof/TofDigi
${CBMROOT_SOURCE_DIR}/ecal
${CBMROOT_SOURCE_DIR}/psd
${CBMROOT_SOURCE_DIR}/passive
${CBMROOT_SOURCE_DIR}/field
${CBMROOT_SOURCE_DIR}/generators
${CBMROOT_SOURCE_DIR}/run
)

Include_Directories( ${INCLUDE_DIRECTORIES})

set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)
message("System includes: " ${SYSTEM_INCLUDE_DIRECTORIES})

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
${ROOT_LIBRARY_DIR}
${FAIRROOT_LIBRARY_DIR}
${Boost_LIBRARY_DIRS}
${Vc_LIB_DIR}
${KFParticle_LIB_DIR}
${SIMPATH}/lib
)
 
link_directories( ${LINK_DIRECTORIES})

set(SRCS

CbmBuildEventsIdeal.cxx
CbmBuildEventsQA.cxx
CbmBuildEventsSimple.cxx
CbmDaq.cxx
CbmDigitization.cxx
CbmDigitizationSource.cxx
CbmDigitizeInfo.cxx
CbmMCBuffer.cxx
CbmMCInput.cxx
CbmMCInputSet.cxx
CbmMCStreamer.cxx
CbmMCStreamerQa.cxx
CbmMCTimeSim.cxx
CbmRunAna.cxx
CbmRunTransport.cxx
CbmSetup.cxx

# De-activated
#CbmDaqQa.cxx
#CbmEvBuildSource.cxx
#CbmRunOnline.cxx

)

set(LINKDEF CbmRunLinkDef.h)
Set(LIBRARY_NAME CbmRun)
Set(DEPENDENCIES CbmData CbmBase CbmField CbmGenerators CbmMvd CbmSts CbmRich 
CbmMuch CbmTrd CbmTrdSim CbmTof CbmPsd geant321 geant4vmc EGPythia6)



GENERATE_LIBRARY()
