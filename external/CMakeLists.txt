# Build all libraries which come from external sources like other subversion or git repositories

include(DownloadProject)
download_project(PROJ            cppzmq
                 GIT_REPOSITORY  "https://github.com/zeromq/cppzmq/"
                 GIT_TAG         "05a0256d0eeea8063690fde6a156e14b70ed2280"
                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/cppzmq
)

Set(IPC_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ipc/ipc/lib/fles_ipc PARENT_SCOPE)

Add_Subdirectory(ipc)
Add_Subdirectory(ipc_legacy)
Add_Subdirectory(flib_dpb)
Add_Subdirectory(spadic)

Execute_Process(COMMAND ${ROOT_CONFIG_EXECUTABLE} --has-vc
                OUTPUT_VARIABLE ROOT_HAS_VC
               )
String(STRIP ${ROOT_HAS_VC} ROOT_HAS_VC)
Set(ROOT_HAS_VC ${ROOT_HAS_VC} PARENT_SCOPE)

If(NOT ${ROOT_HAS_VC})
  Message(STATUS "Root does not support VC. Compile the internal version.")

  set(_LIBDIR_DEFAULT "lib")

  set(Vc_VERSION "1.3.2")
  set(Vc_SRC_URL "https://github.com/VcDevel/Vc")
  set(Vc_DESTDIR "${CMAKE_BINARY_DIR}/external/VC-prefix")
  set(Vc_ROOTDIR "${Vc_DESTDIR}/${CMAKE_BINARY_DIR}")
  set(Vc_LIBNAME "${CMAKE_STATIC_LIBRARY_PREFIX}Vc${CMAKE_STATIC_LIBRARY_SUFFIX}")
  set(Vc_LIBRARY "${Vc_ROOTDIR}/${_LIBDIR_DEFAULT}/${Vc_LIBNAME}")

  ExternalProject_Add(VC
    GIT_REPOSITORY ${Vc_SRC_URL}
    GIT_TAG ${Vc_VERSION}
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

EndIf()
