# This file is only called if the compiler provides c++11 support

Add_Subdirectory(tsclient)
Add_Subdirectory(trb)
Add_Subdirectory(softdpb)
If(NOT APPLE)
  Add_Subdirectory(ctrl)
#  Add_Subdirectory(scv3)
EndIf()
Add_Subdirectory(reader)
