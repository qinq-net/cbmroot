GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/test/cbmdata/TestCbmHit.C)
GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/test/cbmdata/TestCbmPixelHit.C)

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/test/cbmdata)

Add_Test(TestCbmHit ${CBMROOT_BINARY_DIR}/test/cbmdata/TestCbmHit.sh)
Add_Test(TestCbmPixelHit ${CBMROOT_BINARY_DIR}/test/cbmdata/TestCbmPixelHit.sh)


set(INCLUDE_DIRECTORIES
 ${CMAKE_SOURCE_DIR}/data
)

include_directories( ${INCLUDE_DIRECTORIES})

Set(SYSTEM_INCLUDE_DIRECTORIES
${BASE_INCLUDE_DIRECTORIES} 
)

Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})

set(LINK_DIRECTORIES
 ${ROOT_LIBRARY_DIR}
 ${FAIRROOT_LIBRARY_DIR}
)

link_directories( ${LINK_DIRECTORIES})

###############################
Set ( Sources
    _GTestCbmHit.cxx
    ${CMAKE_SOURCE_DIR}/data/CbmHit.cxx
    )  

add_executable(_GTestCbmHit ${Sources})
target_link_libraries(_GTestCbmHit ${ROOT_LIBRARIES} ${GTEST_BOTH_LIBRARIES} CbmData)
add_test(_GTestCbmHit ${PROJECT_BINARY_DIR}/bin/_GTestCbmHit)

