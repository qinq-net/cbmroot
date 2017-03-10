#
##   modify cbmMuch.cxx to include mMUCH and run a mCBM simulation
#

# by David Emschermann
# 10.03.2017

cd ../..
patch much/geo/CbmMuch.cxx macro/mcbm/CbmMuch.cxx_to_McbmGeo.patch
nice make -j3 -C build
cd macro/mcbm
root -l mcbm_sim.C

# to restore CbmMuch to the original version do:
# cd "trunk"
# rm much/geo/CbmMuch.cxx
# svn up much/geo/CbmMuch.cxx 
# nice make -j3 -C build
