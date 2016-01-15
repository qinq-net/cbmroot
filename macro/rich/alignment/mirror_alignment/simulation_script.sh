#!/bin/sh
 
# Launch simulation, reconstruction and analyze files
# for a given nb of events and geometry files
 
#for RX in 2 4 5 6 8 10 ;do
 
for geom_nb in 0 1 2 3 4 5 ;do
    nEvs=1000
    root -b -q "run_sim_jordan.C($nEvs, $geom_nb)"
    root -b -q "run_reco_jordan.C($nEvs, $geom_nb)"
    root -b -q "Analyze.C($geom_nb)"
done
