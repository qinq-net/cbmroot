include(DownloadProject)

set(DATATREE_VERSION "1.4")
set(DATATREE_SRC_URL "https://cbmgsi.githost.io/pwg-c2f/DataTree.git")
set(DATATREE_DESTDIR "${CMAKE_BINARY_DIR}/analysis/flow/DATATREE-prefix")
set(DATATREE_LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}DataTree${CMAKE_SHARED_LIBRARY_SUFFIX}")

download_project(PROJ            DataTree_source
                 GIT_REPOSITORY  ${DATATREE_SRC_URL}
                 GIT_TAG         ${DATATREE_VERSION}
                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/DataTree
)

ExternalProject_Add(DATATREE
  BUILD_IN_SOURCE 0
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DataTree
  BUILD_BYPRODUCTS ${DATATREE_LIBRARY}
  LOG_DOWNLOAD 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
  CMAKE_ARGS -G ${CMAKE_GENERATOR}
             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
             -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
             -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
             -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
             -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
             -DFIXTARGET=TRUE
             -DROOTSYS=${SIMPATH}
             -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
             -DEXPERIMENT=CBM
  INSTALL_COMMAND  ${CMAKE_COMMAND} --build . --target install
)

add_library(DataTree SHARED IMPORTED)
set_target_properties(DataTree PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib)
add_dependencies(DataTree DATATREE)

set(DataTree_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
set(DataTree_LIBRARIES DataTree)
set(DataTree_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include")
set(DataTree_FOUND TRUE)

Install(FILES ${CMAKE_BINARY_DIR}/lib/${DATATREE_LIBNAME} 
              ${CMAKE_BINARY_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}DataTree.rootmap
              ${CMAKE_BINARY_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}DataTree_rdict.pcm
        DESTINATION lib
       )


set(DATATREECBMINTERFACE_VERSION "beta")
set(DATATREECBMINTERFACE_SRC_URL "https://cbmgsi.githost.io/pwg-c2f/DataTreeCbmInterface.git")
set(DATATREECBMINTERFACE_DESTDIR "${CMAKE_BINARY_DIR}/analysis/flow/DATATREECBMINTERFACE-prefix")
set(DATATREECBMINTERFACE_LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}DataTreeCbmInterface${CMAKE_SHARED_LIBRARY_SUFFIX}")

download_project(PROJ            DataTreeCbmInterface_source
                 GIT_REPOSITORY  ${DATATREECBMINTERFACE_SRC_URL}
                 GIT_TAG         ${DATATREECBMINTERFACE_VERSION}
                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/DataTreeCbmInterface
)

add_subdirectory (DataTreeCbmInterface)
