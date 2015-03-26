#!/bin/sh

nEvs=10

for RotMir in 1 -10 ;do
    for RX in 10 ;do
	for RY in 10;do
	    root -b -q "CreateGeo/CreateGDMLfileNew.C($RX, $RY, $RotMir)"
	    root -b -q "Import_GDML_Export_ROOT.c($RX, $RY, $RotMir)"
	    #RunSimOnBatch.sh rotx roty rotmir nevs geocaes ptnotp mommin mommax
	    #qsub 
	    . ./RunSimOnBatch.sh $RX $RY $RotMir $nEvs 2 1 0 4
	done
    done
done
