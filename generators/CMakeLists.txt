# Create a library called "libCbmGenerators" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
 ${CBMROOT_SOURCE_DIR}/data
 ${CBMROOT_SOURCE_DIR}/generators
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
URun.cxx
UEvent.cxx
UParticle.cxx
CbmIonGenerator.cxx
)

If(PLUTO_FOUND)
  Set(SRCS 
    ${SRCS}
    CbmPlutoGenerator.cxx
    CbmPlutoReactionGenerator.cxx
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
