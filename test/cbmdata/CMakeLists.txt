GENERATE_ROOT_TEST_SCRIPT(${CBMROOT_SOURCE_DIR}/test/cbmdata/TestCbmHit.C)

# Put the .rootrc file into the directory from which root is executed.
# Otherwise the rootalias file is not loaded
File(COPY ${CBMROOT_SOURCE_DIR}/macro/include/.rootrc DESTINATION ${CBMROOT_BINARY_DIR}/test/cbmdata)

Add_Test(TestCbmHit ${CBMROOT_BINARY_DIR}/test/cbmdata/TestCbmHit.sh)