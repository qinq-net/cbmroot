# Create a library called "libBase" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
${CBMROOT_SOURCE_DIR}/base
${CBMROOT_SOURCE_DIR}/base/utils
${CBMROOT_SOURCE_DIR}/psd
${CBMROOT_SOURCE_DIR}/data
${CBMROOT_SOURCE_DIR}/data/psd
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


set(SRCS
CbmPsd.cxx
CbmPsdMC.cxx  
CbmGeoPsd.cxx  
CbmGeoPsdPar.cxx  
# CbmPsdv1.cxx 
CbmPsdIdealDigitizer.cxx
CbmPsdSimpleDigitizer.cxx
CbmPsdHitProducer.cxx
# CbmPsdv1_44mods_hole6cm.cxx 
# CbmPsdWoutv1_for44mods_full_holes.cxx
# CbmPsdWoutv1_for44mods_sub1.cxx 
# CbmPsdWoutv1_for44mods_sub2.cxx 
# CbmPsdWoutv1_for44mods_sub3.cxx
# CbmPsdTest.cxx
# CbmPsdNA61.cxx

)

set(LINKDEF CbmPsdLinkDef.h)
Set(LIBRARY_NAME CbmPsd)
Set(DEPENDENCIES
    CbmData CbmBase Base 
)

GENERATE_LIBRARY()

