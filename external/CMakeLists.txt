# Build all libraries which come from external sources like other subversion or git repositories

download_project_if_needed(PROJECT         cppzmq
                           GIT_REPOSITORY  "https://github.com/zeromq/cppzmq/"
                           GIT_TAG         "05a0256d0eeea8063690fde6a156e14b70ed2280"
                           SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/cppzmq
                           TEST_FILE       zmq.hpp
                          )

download_project_if_needed(PROJECT         flib_dpb_20
                           GIT_REPOSITORY  "https://cbmgsi.githost.io/garcia_AT_iri.uni-frankfurt.de/flib_dpb_20.git"
                           GIT_TAG         "ce30a7530204f860c433753ec0db9b649795bf94"
                           SOURCE_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/flib_dpb_20
                           TEST_FILE       CMakeLists.txt
                          )


Set(IPC_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ipc/ipc/lib/fles_ipc PARENT_SCOPE)
Set(IPCLOG_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ipc/ipc/lib/logging PARENT_SCOPE)

Add_Subdirectory(ipc)
Add_Subdirectory(ipc_legacy)
Add_Subdirectory(flib_dpb)
Add_Subdirectory(flib_dpb_20)
Add_Subdirectory(spadic)

Include(InstallVC.cmake)
Include(InstallKFParticle.cmake)
Include(InstallDataTree.cmake)
Include(InstallDataTreeQA.cmake)
