#!/bin/bash

nEvs=500000
NUM=00001

for FLAG in 0 1
do
	root -b -q -l "run_sim_position.C($nEvs, \"$NUM\", $FLAG)"
	root -b -q -l "run_reco_position.C($nEvs, \"$NUM\", $FLAG)"
	root -b -q -l "Compute_distance.C(\"$NUM\", $FLAG)"
done