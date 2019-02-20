

add_subdirectory(source)
add_subdirectory(sink)
add_subdirectory(unpacker)

# The following subdirectories contain files which do not compile with
# FairRoot v18
# Don't compile it for the time being
# TODO: Fix the compilation problems
#  add_subdirectory(parmq)
#  add_subdirectory(hitbuilder)
