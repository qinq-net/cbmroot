#
##   Howto run macros in macro/mcbm
#

# by David Emschermann
# 04.04.2017

# to initially convert MUCH inteo miniMUCH run the following sctript
mcbm_with_much.sh
# it modifies the much geometry class

# then either run
root -l run_mc.C
# or
root -l run_sim.C


#
##   UrQMD input files for mCBM energies
#

# more urqmd input files are available here:

/hera/cbm/prod/gen/urqmd/agag/1.65gev/centr/urqmd.agag.1.65gev.centr.00001.root
/hera/cbm/prod/gen/urqmd/agag/1.65gev/mbias/urqmd.agag.1.65gev.mbias.00001.root
/hera/cbm/prod/gen/urqmd/nini/1.93gev/centr/urqmd.nini.1.93gev.centr.00001.root
/hera/cbm/prod/gen/urqmd/nini/1.93gev/mbias/urqmd.nini.1.93gev.mbias.00001.root
/hera/cbm/prod/gen/urqmd/pau/4.5gev/mbias/urqmd.pau.4.5gev.mbias.00001.root
