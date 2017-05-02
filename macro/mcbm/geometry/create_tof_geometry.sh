#!/bin/bash
VGEO=$1

root -l -q Create_TOF_Geometry_${VGEO}.C
cp mtof_v14a.geo.root ../../../geometry/tof/tof_${VGEO}_mcbm.geo.root
cp mtof_v14a.geo.root mtof_${VGEO}.geo.root
cp mtof_v14a_geo.root mtof_${VGEO}_geo.root
#root -l -q 'make_geofile_noBfield.C("mtof_'${VGEO}'")'
#cp geofile_mtof_${VGEO}.root ../../../geometry/tof/
root -l 'create_tof_digipar.C("mtof_'${VGEO}'")'
cp mtof_${VGEO}.digi.par ../../../parameters/tof/tof_${VGEO}_mcbm.digi.par
