# Build all libraries which come from external sources like other subversion or git repositories

include(DownloadProject)
download_project(PROJ            cppzmq
                 GIT_REPOSITORY  "https://github.com/zeromq/cppzmq/"
                 GIT_TAG         "05a0256d0eeea8063690fde6a156e14b70ed2280"
                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/cppzmq
)

download_project(PROJ            flib_dpb_20
                 GIT_REPOSITORY  "https://cbmgsi.githost.io/garcia_AT_iri.uni-frankfurt.de/flib_dpb_20.git"
                 GIT_TAG         "ce30a7530204f860c433753ec0db9b649795bf94"
                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/flib_dpb_20
)

set(Vc_VERSION "1.3.2")
set(Vc_SRC_URL "https://github.com/VcDevel/Vc")

download_project(PROJ            vc_source
                 GIT_REPOSITORY  ${Vc_SRC_URL}
                 GIT_TAG         ${Vc_VERSION}
                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/Vc
)

set(KFPARTICLE_SRC_URL "https://cbmgsi.githost.io/m.zyzak/KFParticle.git")
set(KFPARTICLE_DESTDIR "${CMAKE_BINARY_DIR}/external/KFPARTICLE-prefix")

download_project(PROJ            kfparticle_source
                 GIT_REPOSITORY  ${KFPARTICLE_SRC_URL}
                 GIT_TAG         ${KFPARTICLE_VERSION}
                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/KFParticle
)


Set(IPC_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ipc/ipc/lib/fles_ipc PARENT_SCOPE)

Add_Subdirectory(ipc)
Add_Subdirectory(ipc_legacy)
Add_Subdirectory(flib_dpb)
Add_Subdirectory(flib_dpb_20)
Add_Subdirectory(spadic)

# Build Vc as external project
set(_LIBDIR_DEFAULT "lib")

set(Vc_DESTDIR "${CMAKE_BINARY_DIR}/external/VC-prefix")
set(Vc_ROOTDIR "${Vc_DESTDIR}/${CMAKE_BINARY_DIR}")
set(Vc_LIBNAME "${CMAKE_STATIC_LIBRARY_PREFIX}Vc${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(Vc_LIBRARY "${Vc_ROOTDIR}/${_LIBDIR_DEFAULT}/${Vc_LIBNAME}")

ExternalProject_Add(VC
#  GIT_REPOSITORY ${Vc_SRC_URL}
#  GIT_TAG ${Vc_VERSION}
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vc
  BUILD_IN_SOURCE 0   
  BUILD_BYPRODUCTS ${Vc_LIBRARY}
  LOG_DOWNLOAD 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
  CMAKE_ARGS -G ${CMAKE_GENERATOR}
             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
             -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
             -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
             -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
             -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
  INSTALL_COMMAND env DESTDIR=${Vc_DESTDIR} ${CMAKE_COMMAND} --build . --target install
)

add_library(Vc STATIC IMPORTED)
set_target_properties(Vc PROPERTIES IMPORTED_LOCATION ${Vc_LIBRARY})
add_dependencies(Vc VC)

set(Vc_LIBRARIES Vc)
set(Vc_INCLUDE_DIR "${Vc_ROOTDIR}/include")
set(Vc_CMAKE_MODULES_DIR "${Vc_ROOTDIR}/${_LIBDIR_DEFAULT}/cmake/Vc")

Include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Vc
  FOUND_VAR Vc_FOUND
  REQUIRED_VARS Vc_INCLUDE_DIR Vc_LIBRARIES Vc_CMAKE_MODULES_DIR
  VERSION_VAR Vc_VERSION
)

install(DIRECTORY ${Vc_ROOTDIR}/ DESTINATION ".")

if(Vc_FOUND)
  # Missing from VcConfig.cmake
  set(VC_INCLUDE_DIRS ${Vc_INCLUDE_DIR} PARENT_SCOPE)
  set(Vc_LIB_DIR ${Vc_ROOTDIR}/${_LIBDIR_DEFAULT} PARENT_SCOPE)
endif()


set(KFPARTICLE_ROOTDIR "${KFPARTICLE_DESTDIR}/${CMAKE_BINARY_DIR}")
set(KFPARTICLE_LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}KFParticle${CMAKE_SHARED_LIBRARY_SUFFIX}")
#If(APPLE)
#  set(KFPARTICLE_PCMNAME "${CMAKE_SHARED_LIBRARY_PREFIX}KFParticle_rdict.pcm")
#Else()
  set(KFPARTICLE_PCMNAME "G__KFParticle_rdict.pcm")
#EndIf()
set(KFPARTICLE_LIBRARY "${KFPARTICLE_ROOTDIR}/${_LIBDIR_DEFAULT}/${KFPARTICLE_LIBNAME}")
set(KFPARTICLE_PCMFILE "${KFPARTICLE_ROOTDIR}/${_LIBDIR_DEFAULT}/${KFPARTICLE_PCMNAME}")

ExternalProject_Add(KFPARTICLE
  DEPENDS Vc
  BUILD_IN_SOURCE 0
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/KFParticle
  BUILD_BYPRODUCTS ${KFPARTICLE_LIBRARY} ${KFPARTICLE_PCMFILE}
  LOG_DOWNLOAD 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
  CMAKE_ARGS -G ${CMAKE_GENERATOR}
             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
             -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
             -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
             -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
             -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
             -DVc_INCLUDE_DIR=${Vc_INCLUDE_DIR}
             -DVc_LIBRARIES=${Vc_LIBRARY}
             -DFIXTARGET=TRUE
             -DROOTSYS=${SIMPATH}
             -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
  INSTALL_COMMAND env DESTDIR=${KFPARTICLE_DESTDIR} ${CMAKE_COMMAND} --build . --target install
)

add_library(KFParticle SHARED IMPORTED)
set_target_properties(KFParticle PROPERTIES IMPORTED_LOCATION ${KFPARTICLE_LIBRARY})
add_custom_command(TARGET KFPARTICLE POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                   ${KFPARTICLE_LIBRARY} ${CMAKE_BINARY_DIR}/lib)
#add_custom_command(TARGET KFPARTICLE POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                   ${KFPARTICLE_PCMFILE} ${CMAKE_BINARY_DIR}/lib)
add_dependencies(KFParticle KFPARTICLE)

set(KFParticle_LIB_DIR ${KFPARTICLE_ROOTDIR}/lib PARENT_SCOPE)
set(KFParticle_LIBRARIES KFParticle PARENT_SCOPE)
set(KFParticle_INCLUDE_DIR "${KFPARTICLE_ROOTDIR}/include" PARENT_SCOPE)
set(KFParticle_FOUND TRUE PARENT_SCOPE)

install(DIRECTORY ${KFPARTICLE_ROOTDIR}/ DESTINATION ".")
