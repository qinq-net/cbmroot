#--- Tests for continuous builds
add_subdirectory(run)
add_subdirectory(mcbm)
add_subdirectory(mvd)
add_subdirectory(much)
add_subdirectory(tof)
add_subdirectory(beamtime)
add_subdirectory(include)
 

#--- Additional tests for nightly builds
If($ENV{ctest_model} MATCHES Nightly)
  add_subdirectory(field)
  add_subdirectory(KF)
#  add_subdirectory(C2F)
  add_subdirectory(dimuon)
  Message("Additional Nightly tests")
EndIf($ENV{ctest_model}  MATCHES Nightly)


#--- Deprecated (not running)
#add_subdirectory(trd)
#add_subdirectory(littrack)

