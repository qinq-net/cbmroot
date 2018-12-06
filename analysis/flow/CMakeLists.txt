
set(DATATREE_VERSION b25b3e5a5dbc0348c45fb0f505871c0e6b7ff3be) # hash is tag 1.4. Needed for test

set(DATATREE_SRC_URL "https://cbmgsi.githost.io/pwg-c2f/DataTree.git")
set(DATATREE_DESTDIR "${CMAKE_BINARY_DIR}/analysis/flow/DATATREE-prefix")
set(DATATREE_LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}DataTree${CMAKE_SHARED_LIBRARY_SUFFIX}")

download_project_if_needed(PROJECT         DataTree_source
                           GIT_REPOSITORY  ${DATATREE_SRC_URL}
                           GIT_TAG         ${DATATREE_VERSION}
                           SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/DataTree
                           TEST_FILE       CMakeLists.txt
                          )

If(ProjectUpdated)
  File(REMOVE_RECURSE ${DATATREE_DESTDIR})
  Message("DataTree source directory was changed so build directory was deleted")  
EndIf()

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

add_subdirectory (DataTreeCbmInterface)

set(DATATREEQA_VERSION 63dff41c794999b72b0b8351715830569b8d0821) # hash is tag 1.0. Needed for test
set(DATATREEQA_SRC_URL "https://cbmgsi.githost.io/pwg-c2f/DataTreeQA.git")
set(DATATREEQA_DESTDIR "${CMAKE_BINARY_DIR}/analysis/flow/DATATREEQA-prefix")
set(DATATREEQA_LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}DataTreeQA${CMAKE_SHARED_LIBRARY_SUFFIX}")


download_project_if_needed(PROJECT         DataTreeQA_source
                           GIT_REPOSITORY  ${DATATREEQA_SRC_URL}
                           GIT_TAG         ${DATATREEQA_VERSION}
                           SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/DataTreeQA
                           TEST_FILE       CMakeLists.txt
                          )

If(ProjectUpdated)
  File(REMOVE_RECURSE ${DATATREEQA_DESTDIR})
  Message("DataTreeQA source directory was changed so build directory was deleted")  
EndIf()

ExternalProject_Add(DATATREEQA
  DEPENDS DataTree
  BUILD_IN_SOURCE 0
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DataTreeQA
  BUILD_BYPRODUCTS ${DATATREEQA_LIBRARY}
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

add_library(DataTreeQA SHARED IMPORTED)
set_target_properties(DataTreeQA PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib)
add_dependencies(DataTreeQA DATATREE)

set(DataTreeQA_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
set(DataTreeQA_LIBRARIES DataTreeQA)
set(DataTreeQA_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include")
set(DataTreeQA_FOUND TRUE)

Install(FILES ${CMAKE_BINARY_DIR}/lib/${DATATREEQA_LIBNAME}
              ${CMAKE_BINARY_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}DataTreeQA.rootmap
              ${CMAKE_BINARY_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}DataTreeQA_rdict.pcm
        DESTINATION lib
       )


#set(UNIGENQA_VERSION "1.0")
#set(UNIGENQA_SRC_URL "https://cbmgsi.githost.io/pwg-c2f/UnigenQA.git")
#set(UNIGENQA_DESTDIR "${CMAKE_BINARY_DIR}/analysis/flow/UNIGENQA-prefix")
#set(UNIGENQA_LIBNAME "${CMAKE_SHARED_LIBRARY_PREFIX}UnigenQA${CMAKE_SHARED_LIBRARY_SUFFIX}")

#download_project(PROJ            UnigenQA_source
#                 GIT_REPOSITORY  ${UNIGENQA_SRC_URL}
#                 GIT_TAG         ${UNIGENQA_VERSION}
#                 SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/UnigenQA
#)


#If(ProjectUpdated)
#  File(REMOVE_RECURSE ${UNIGENQA_DESTDIR})
#  Message("UnigenQA source directory was changed so build directory was deleted")  
#EndIf()

#ExternalProject_Add(UNIGENQA
#  BUILD_IN_SOURCE 0
#  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/UnigenQA
#  BUILD_BYPRODUCTS ${UNIGENQA_LIBRARY}
#  LOG_DOWNLOAD 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
#  CMAKE_ARGS -G ${CMAKE_GENERATOR}
#             -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
#             -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
#             -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
#             -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
#             -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
#             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}
#             -DFIXTARGET=TRUE
#             -DROOTSYS=${SIMPATH}
#             -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
#  INSTALL_COMMAND  ${CMAKE_COMMAND} --build . --target install
#)

#add_library(UnigenQA SHARED IMPORTED)
#set_target_properties(UnigenQA PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib)
#add_dependencies(UnigenQA UNIGENQA)

#set(UnigenQA_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
#set(UnigenQA_LIBRARIES UnigenQA)
#set(UnigenQA_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include")
#set(UnigenQA_FOUND TRUE)

#Install(FILES ${CMAKE_BINARY_DIR}/lib/${UNIGENQA_LIBNAME}
#              ${CMAKE_BINARY_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}UnigenQA.rootmap
#              ${CMAKE_BINARY_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}UnigenQA_rdict.pcm
#        DESTINATION lib
#       )
