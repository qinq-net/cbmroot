# Create a library called "libCbmGenerators" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
 ${CBMROOT_SOURCE_DIR}/data
 ${CBMROOT_SOURCE_DIR}/generators
 ${CBMROOT_SOURCE_DIR}/generators/pluto
 ${CBMROOT_SOURCE_DIR}/generators/unigen
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${BASE_INCLUDE_DIRECTORIES} 
)

Set(LINK_DIRECTORIES
 ${ROOT_LIBRARY_DIR}
 ${FAIRROOT_LIBRARY_DIR}
)

If(PLUTO_FOUND)
  Set(SYSTEM_INCLUDE_DIRECTORIES
    ${SYSTEM_INCLUDE_DIRECTORIES} 
    ${PLUTO_INCLUDE_DIR}
  )
  Set(LINK_DIRECTORIES
    ${LINK_DIRECTORIES}
    ${PLUTO_LIBRARY_DIR}
  )
EndIf()

Include_Directories( ${INCLUDE_DIRECTORIES})
Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})
link_directories( ${LINK_DIRECTORIES})

set(SRCS
CbmDirectPhotonGenerator.cxx
CbmEcalGenerator.cxx
CbmHsdLvmGenerator.cxx
CbmHsdGenerator.cxx
CbmShieldGenerator.cxx
CbmUnigenGenerator.cxx
CbmIonGenerator.cxx
CbmPlutoGenerator.cxx
unigen/URun.cxx
unigen/UEvent.cxx
unigen/UParticle.cxx
pluto/PDataBase.cxx
pluto/PMesh.cxx
pluto/PParticle.cxx
pluto/PStaticData.cxx
pluto/PStdData.cxx
pluto/PValues.cxx
)

If(PLUTO_FOUND)
  Set(SRCS 
    ${SRCS}
    CbmPlutoReactionGenerator.cxx
    CbmDYMuonGenerator.cxx
    CbmSemiMuonGenerator.cxx
  )
  Set(DEPENDENCIES
    CbmData Base Pluto
  )
  Set(DEFINITIONS -DHAS_PLUTO)
Else()
  Set(DEPENDENCIES
    CbmData Base
  )
EndIf()

set(LINKDEF CbmGeneratorsLinkDef.h)
Set(LIBRARY_NAME CbmGenerators)

GENERATE_LIBRARY()
