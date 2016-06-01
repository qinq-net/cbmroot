//
//   comments
//

// 2016-02-25 - DE - use STS v16c as new default
// 2015-12-14 - DE - use TRD v15c starting at z = 490 cm for muon_jpsi setup
// 2015-12-14 - DE - rename long SIS100 MUCH version to muon_JPSI setup
// 2015-12-14 - DE - use TOF v16a
// 2015-12-11 - DE - use STS v15c as new default
// 2015-12-11 - DE - reproduce MUCH v15b setup from macro/much/much_sim.C
// 2015-07-20 - FU - magnet v15a is v12b with correct keeping volume material
// 2015-07-20 - FU - add material budget files
// 2015-01-19 - DE - use TRD v15a as new default
// 2014-06-30 - DE - make TRD v14a the new default
// 2014-06-25 - DE - define digi files through tags for STS, TRD and TOF
// 2013-11-05 - DE - switch to trd_v13p geometry
// 2013-10-11 - DE - add empty string defining the platform
// 2013-10-07 - DE - pipe_v13e.geo fixes overlap with TOF, also avoids TRD
// 2013-11-04 - VF - make STS v15b the new default
//


void sis100_muon_jpsi_setup()
{
  // -----   Magnetic field   -----------------------------------------------
  fieldMap     = "field_v12b";   // name of field map
  fieldZ       = 40.;            // field centre z position
  fieldScale   =  1.;            // field scaling factor
  fieldSymType =  3;

  // -----  Tags  -----------------------------------------------------------
  mvdTag       = "";
  stsTag       = "v16c";
  trdTag       = "v15c_1m";
  tofTag       = "v16a_1m";

  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = ""; // "sts/sts_" + stsTag + "_std.digi.par";
  muchDigi     = "much/much_v15c_digi_sector.root";
  trdDigi      = "trd/trd_" + trdTag + ".digi.par";
  tofDigi      = "tof/tof_" + tofTag + ".digi.par";
  tofDigiBdf   = "tof/tof_" + tofTag + ".digibdf.par";

  // -----  Material budget files -------------------------------------------
  stsMatBudget = "sts/sts_matbudget_" + stsTag + ".root";
  mvdMatBudget = "";

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "cave.geo";
  pipeGeom     = "pipe/pipe_much_v15b_125cm_no.geo";
  magnetGeom   = "magnet/magnet_v15b_much.geo.root";
  mvdGeom      = "";
  stsGeom      = "sts/sts_" + stsTag + ".geo.root";
  richGeom     = "";
  muchGeom     = "much/much_v15c_SIS100-C_gemtrd.geo";
//  muchGeom     = "much/much_v15d_SIS100-C_allgem.geo";
  shieldGeom   = "much/shield_v15c_SIS100C_149_3part_125cm.geo";
  platformGeom = ""; // "passive/platform_v13a.geo";
  trdGeom      = "trd/trd_" + trdTag + ".geo.root";
  tofGeom      = "tof/tof_" + tofTag + ".geo.root";
  ecalGeom     = "";
  psdGeom      = ""; // "psd_geo_xy.txt";
  psdZpos = 800.;
  psdXpos = 11.;

  // ------ Input file ------------------------------------------------------

  defaultInputFile = "/input/urqmd.auau.10gev.centr.root";

  cout << "Reading geometry setup and magnetic field settings from "
       << gSystem->UnixPathName(gInterpreter->GetCurrentMacroName())
       << endl;
}
