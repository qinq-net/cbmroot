#
##   create Magnet v15b MUCH from Magnet v15a with the help of a patch file
#

# by David Emschermann

# compared to magnet v15a, the magnet v15b for MUCH
# has an additional cutout in the keeping volume,
# which removes an overlap with the shield v15b mantle1
#
# see redmine isse #404 for details:
# https://lxcbmredmine01.gsi.de/issues/404

#
SOURCE=magnet_v15a.gdml
TARGET=magnet_v15b.gdml
PATCH=magnet_v15a_to_v15b.patch
#
echo 
echo cp $SOURCE $TARGET		  
echo patch $TARGET $PATCH		 
echo root -l -q Import_GDML_Export_ROOT.c
echo root -l create_much_magnet.C     
echo 
#
cp $SOURCE $TARGET
patch $TARGET $PATCH
root -l -q Import_GDML_Export_ROOT.c
root -l create_much_magnet.C     
