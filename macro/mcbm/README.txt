#
##   Howto run macros in macro/mcbm
#

# by David Emschermann
# 04.04.2017

# to initially convert MUCH into miniMUCH run the following script once
mcbm_with_much.sh
# it modifies the much geometry class in your cbmroot installation

# then either run
root -l mcbm_mc.C
root -l mcbm_reco.C


# Details by Volker Friese:
The "correct" sequence is mcbm_mc.C (transport) and mcbm_reco.C (digitisation + reconstruction).

mcbm_sim.C uses the legacy setup scheme, but it is working also; I have tried to update it
accordingly.

To the naming: In macro/run, run_sim.C combines transport and digitisation. This is currently not
needed, since it is more convenient to do the digitisation in the reco macro, before the actual
reconstruction. However, in the time-based mode, digitisation and reconstruction will have to be
separated. But for the moment, in the event-by-event mode, you can just use these two macros (mc and
reco).

#  mcbm_sim.C was therefore moved to the legacy folder


#
##   UrQMD input files for mCBM energies
#

# more urqmd input files are available here:

/hera/cbm/prod/gen/urqmd/agag/1.65gev/centr/urqmd.agag.1.65gev.centr.00001.root
/hera/cbm/prod/gen/urqmd/agag/1.65gev/mbias/urqmd.agag.1.65gev.mbias.00001.root
/hera/cbm/prod/gen/urqmd/nini/1.93gev/centr/urqmd.nini.1.93gev.centr.00001.root
/hera/cbm/prod/gen/urqmd/nini/1.93gev/mbias/urqmd.nini.1.93gev.mbias.00001.root
/hera/cbm/prod/gen/urqmd/pau/4.5gev/mbias/urqmd.pau.4.5gev.mbias.00001.root
