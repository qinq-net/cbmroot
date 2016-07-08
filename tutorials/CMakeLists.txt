#add_subdirectory (Tutorial1/src)
#add_subdirectory (Tutorial2/src)
#add_subdirectory(MQTest)

Execute_Process(COMMAND $ENV{FAIRROOTPATH}/bin/fairroot-config --version
                OUTPUT_VARIABLE _fairroot_version
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
String(REGEX MATCH "v-([0-9]+)\\.([0-9]+)([a-z]*)" _version_matches "${_fairroot_version}")
Set(FairRoot_VERSION_MAJOR ${CMAKE_MATCH_1})
Set(FairRoot_VERSION_MINOR ${CMAKE_MATCH_2})
Set(FairRoot_VERSION_PATCH ${CMAKE_MATCH_3})
Set(_fairroot_version_found ${FairRoot_VERSION_MAJOR}.${FairRoot_VERSION_MINOR})
If(NOT _fairroot_version_found VERSION_LESS 15.11)
#  add_subdirectory(cbmMQTest)
EndIf()
