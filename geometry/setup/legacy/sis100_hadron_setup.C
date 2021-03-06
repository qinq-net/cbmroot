//
//   comments
//

// 2016-02-25 - DE - use STS v16c as new default
// 2015-12-11 - DE - use STS v15c as new default
// 2015-07-20 - FU - magnet v15a is v12b with correct keeping volume material
// 2015-07-20 - FU - add material budget files
// 2015-03-13 - DE - use STS v13y with correct ladder orientation as new default
// 2015-01-29 - DE - use chopped PIPE v14q as default
// 2015-01-19 - DE - use TRD v15a as new default
// 2014-06-30 - DE - make TRD v14a the new default
// 2014-06-25 - DE - define digi files through tags for STS, TRD and TOF
// 2013-11-05 - DE - switch to trd_v13p geometry
// 2013-10-11 - DE - add empty string defining the platform
// 2013-10-07 - DE - pipe_v13c.geo fixes overlap with TOF, fills RICH section
// 2013-11-04 - VF - make STS v15b the new default
//

/*
static TString caveGeom;
static TString pipeGeom;
static TString magnetGeom;
static TString mvdGeom;
static TString stsGeom;
static TString richGeom;
static TString muchGeom;
static TString shieldGeom;
static TString trdGeom;
static TString tofGeom;
static TString ecalGeom;
static TString platformGeom;
static TString psdGeom;
static Double_t psdZpos;
static Double_t psdXpos;

static TString mvdTag;
static TString stsTag;
static TString trdTag;
static TString tofTag;

static TString stsDigi;
static TString trdDigi;
static TString tofDigi;

static TString mvdMatBudget;
static TString stsMatBudget;

static TString  fieldMap;
static Double_t fieldZ;
static Double_t fieldScale;
static Int_t    fieldSymType;

static TString defaultInputFile;
*/

void sis100_hadron_setup()
{
  // -----   Magnetic field   -----------------------------------------------
  fieldMap     = "field_v12b";   // name of field map
  fieldZ       = 40.;            // field centre z position
  fieldScale   =  1.;            // field scaling factor
  fieldSymType =  3;

  // -----  Tags  -----------------------------------------------------------
  mvdTag       = "v15a";
  stsTag       = "v16c";
  trdTag       = "v15a_1h";
  tofTag       = "v16a_1h";

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
  pipeGeom     = "pipe/pipe_v14q.root";
  magnetGeom   = "magnet/magnet_v15a.geo.root";
  mvdGeom      = "mvd/mvd_" + mvdTag + ".geo.root";
  stsGeom      = "sts/sts_" + stsTag + ".geo.root";
  richGeom     = "";
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
