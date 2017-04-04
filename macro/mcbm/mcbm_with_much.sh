#
##   modify cbmMuch.cxx to include mMUCH
#

# by David Emschermann
# 10.03.2017

# 04.04.2017 - DE - added possibility to run mcbm_mc.C

cd ../..
patch much/geo/CbmMuch.cxx macro/mcbm/CbmMuch.cxx_to_McbmGeo.patch
nice make -C build
cd macro/mcbm

# now run mcbm_mc.C or mcbm_sim.C yourself
#root -l mcbm_mc.C
#root -l mcbm_sim.C

# to restore CbmMuch to the original version do:
# cd "trunk"
# rm much/geo/CbmMuch.cxx
# svn up much/geo/CbmMuch.cxx 
# nice make -j3 -C build
