//
//   comments
//

// 2016-02-25 - DE - use STS v16c as new default
// 2015-12-11 - DE - use STS v15c as new default
// 2015-07-20 - FU - magnet v15a is v12b with correct keeping volume material
// 2015-07-20 - FU - add material budget files
// 2015-03-13 - DE - use STS v13y with correct ladder orientation as new default 
// 2015-01-28 - DE - use STS v13x fitting with with PIPE v14l
// 2015-01-22 - DE - use MVD v14b with PIPE v14l as default
// 2015-01-19 - DE - use TRD v15a as new default
// 2014-07-05 - DE - use PIPE v14x as interim solution
// 2014-07-01 - DE - use PIPE v14f -> overlap with STS
// 2014-06-30 - DE - use PIPE v14d together with RICH v14a
// 2014-06-30 - DE - make RICH v14a the new default
// 2014-06-30 - DE - make TRD v14a the new default
// 2014-06-25 - DE - define digi files through tags for STS, TRD and TOF
// 2013-11-05 - DE - switch to trd_v13p geometry
// 2013-10-11 - DE - add empty string defining the platform
// 2013-10-07 - DE - pipe_v13d.geo fixes overlap with TOF
// 2013-10-07 - DE - rich_v13a is not at nominal position 1800mm, but at 1600mm
// 2013-11-04 - VF - make STS v15b the new default
//


void sis100_electron_setup()
{
  // -----   Magnetic field   -----------------------------------------------
  fieldMap     = "field_v12b";   // name of field map
  fieldZ       = 40.;            // field centre z position
  fieldScale   =  1.;            // field scaling factor
  fieldSymType =  3;

  // -----  Tags  -----------------------------------------------------------
  mvdTag       = "v15a";
  stsTag       = "v16c";
  trdTag       = "v15a_1e";
  tofTag       = "v16a_1e";

  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = ""; // "sts/sts_" + stsTag + "_std.digi.par";
  muchDigi     = "";
  trdDigi      = "trd/trd_" + trdTag + ".digi.par";
  tofDigi      = "tof/tof_" + tofTag + ".digi.par";
  tofDigiBdf   = "tof/tof_" + tofTag + ".digibdf.par";

  // -----  Material budget files -------------------------------------------
  stsMatBudget = "sts/sts_matbudget_" + stsTag + ".root";
  mvdMatBudget = "mvd/mvd_matbudget_" + mvdTag + ".root";

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "cave.geo";
  pipeGeom     = "pipe/pipe_v14l.root";
  magnetGeom   = "magnet/magnet_v15a.geo.root";
  mvdGeom      = "mvd/mvd_" + mvdTag + ".geo.root";
  stsGeom      = "sts/sts_" + stsTag + ".geo.root";
  richGeom     = "rich/rich_v16a_1e.geo.root";
  muchGeom     = "";
  shieldGeom   = "";
  platformGeom = ""; // "passive/platform_v13a.geo";
  trdGeom      = "trd/trd_" + trdTag + ".geo.root";
  tofGeom      = "tof/tof_" + tofTag + ".geo.root";
  ecalGeom     = "";
  psdGeom      = "psd_geo_xy.txt";
  psdZpos = 800.;
  psdXpos = 11.;

  // ------ Input file ------------------------------------------------------

  defaultInputFile = "/input/urqmd.auau.10gev.centr.root";

  cout << "Reading geometry setup and magnetic field settings from "
       << gSystem->UnixPathName(gInterpreter->GetCurrentMacroName())
       << endl;
}
