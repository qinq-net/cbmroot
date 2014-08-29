# This file is only called if the compiler provides c++11 support

Include(Build_Fles_Ipc.cmake)
Include(Build_Flib_Dpb.cmake)
Include(Build_Spadic_Message.cmake)


Add_Subdirectory(tsclient)
Add_Subdirectory(softdpb)
If(NOT APPLE)
  Add_Subdirectory(ctrl)
EndIf()
Add_Subdirectory(reader)
