# Create a library called "libCbmFlibReader" which includes the source files given in
# the array .
# The extension is already found.  Any number of sources could be listed here.

Set(INCLUDE_DIRECTORIES
  ${CBMROOT_SOURCE_DIR}/roclight
  ${CBMROOT_SOURCE_DIR}/roclight/roc
  ${CBMROOT_SOURCE_DIR}/roclight/src_roc
  ${CBMROOT_SOURCE_DIR}/roclight/feet
)

Set(SYSTEM_INCLUDE_DIRECTORIES
  ${Boost_INCLUDE_DIR} 
)

Include_Directories(${INCLUDE_DIRECTORIES})
Include_Directories(SYSTEM ${SYSTEM_INCLUDE_DIRECTORIES})
 
Set(SRCS
  src_roc/Message.cxx
  src_roc/Board.cxx
  src_roc/Iterator.cxx
  src_roc/OperList.cxx
)

Set(HEADERS 
  roc/Message.h
  roc/Board.h
  roc/Iterator.h
  roc/OperList.h
)

Set(LIBRARY_NAME RocLight)

GENERATE_LIBRARY()
