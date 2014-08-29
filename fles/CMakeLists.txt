# This file is only called if the compiler provides c++11 support

Include(Build_Fles_Ipc.cmake)
Include(Build_Flib_Dpb.cmake)

#Add_Subdirectory(flib_dpb)

Add_Subdirectory(tsclient)
Add_Subdirectory(softdpb)
If(NOT APPLE)
  Add_Subdirectory(ctrl)
EndIf()
Add_Subdirectory(reader)
Add_Subdirectory(spadic_old)
