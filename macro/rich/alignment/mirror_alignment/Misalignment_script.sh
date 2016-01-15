#!/bin/sh
 
# Launch simulation, reconstruction and analyze files
# for a given nb of events and geometry files
 
#for RX in 2 4 5 6 8 10 ;do
 
root -b -q "run_sim_jordan.C(1, 0)"
root -b -q "DrawPoints.C(0)"
