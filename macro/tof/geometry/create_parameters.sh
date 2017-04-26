#!/bin/bash
root -l -q Create_TOF_Geometry_v15c.C
cp tof_v14a.root ../../../geometry/tof/tof_v15c.root
cp tof_v14a.root tof_v15c.root
root -l -q 'make_geofile_noBfield.C("tof_v15c")'
cp geofile_tof_v15c.root ../../../geometry/tof/
root -l 'create_digipar.C("tof_v15c")'
cp tof_v15c.digi.par ../../../parameters/tof/
