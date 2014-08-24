# Copyright 2014 Dirk Hutter <hutter@compeng.uni-frankfurt.de>

file(GLOB LIB_SOURCES *.cpp)
file(GLOB LIB_HEADERS *.hpp)

add_library(flib_dpb SHARED ${LIB_SOURCES} ${LIB_HEADERS})
set_target_properties(flib_dpb PROPERTIES  SUFFIX ".so")

