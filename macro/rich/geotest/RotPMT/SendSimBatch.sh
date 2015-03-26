#!/bin/sh

nEvs=10000
cbmroot_config_path=/hera/cbm/users/tariq/cbmroot/buildcbm/config.sh
source ${cbmroot_config_path}

for RotMir in 1 ;do
    for RX in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50;do
	for RY in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50;do

	    root -b -q "CreateGeo/CreateGDMLfileNew.C($RX, $RY, $RotMir)"
	    root -b -q "Import_GDML_Export_ROOT.c($RX, $RY, $RotMir)"
	    #RunSimOnBatch.sh rotx roty rotmir nevs geocaes ptnotp mommin mommax
	    qsub ./RunSimOnBatch.sh $RX $RY $RotMir $nEvs 2 1 0 4
	done
    done
done
