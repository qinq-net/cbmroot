//
//   comments
//

// 2015-12-14 - DE - use TOF v16a
// 2015-12-11 - DE - use STS v15c as new defaut
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


void sis300_muon_setup()
{
  // -----   Magnetic field   -----------------------------------------------
  fieldMap     = "field_v12b";   // name of field map
  fieldZ       = 40.;            // field centre z position
  fieldScale   =  1.;            // field scaling factor
  fieldSymType =  3;

  // -----  Tags  -----------------------------------------------------------
  mvdTag       = "";
  stsTag       = "v15c";
  trdTag       = "v15a_3m";
  tofTag       = "v16a_3m";

  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = ""; // "sts/sts_" + stsTag + "_std.digi.par";
  trdDigi      = "trd/trd_" + trdTag + ".digi.par";
  tofDigi      = "tof/tof_" + tofTag + ".digi.par";

  // -----  Material budget files -------------------------------------------
  mvdMatBudget = "";
  stsMatBudget = "sts/sts_matbudget_" + stsTag + ".root";

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "cave.geo";
  pipeGeom     = "pipe/pipe_v13c.geo.root";
  magnetGeom   = "magnet/magnet_v15a.geo.root";
  mvdGeom      = "";
  stsGeom      = "sts/sts_" + stsTag + ".geo.root";
  richGeom     = "";
  muchGeom     = "much/much_v13f.geo";
  shieldGeom   = "much/shield_v13f.geo";
  platformGeom = ""; // "passive/platform_v13b.geo";
  trdGeom      = ""; // "trd/trd_" + trdTag + ".geo.root";
  tofGeom      = "tof/tof_" + tofTag + ".geo.root";
  ecalGeom     = "";
  psdGeom      = "psd_geo_xy.txt";
  psdZpos = 1500.;
  psdXpos = 10.;

  // ------ Input file ------------------------------------------------------

  defaultInputFile = "/input/urqmd.auau.25gev.centr.root";

  cout << "Reading geometry setup and magnetic field settings from "
       << gSystem->UnixPathName(gInterpreter->GetCurrentMacroName())
       << endl;
}
