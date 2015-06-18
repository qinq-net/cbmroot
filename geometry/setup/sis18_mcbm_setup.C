//
//   comments
//

// 2015-06-18 - DE - mCBM @ SIS18 setup
//

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

static TString stsTag;
static TString trdTag;
static TString tofTag;

static TString stsDigi;
static TString trdDigi;
static TString tofDigi;

static TString  fieldMap;
static Double_t fieldZ;
static Double_t fieldScale;
static Int_t    fieldSymType;

static TString defaultInputFile;

void sis18_mcbm_setup()
{
  // -----   Magnetic field   -----------------------------------------------
  fieldMap     = "field_v12b";   // name of field map
  fieldZ       = 40.;            // field centre z position
  fieldScale   =  1.;            // field scaling factor
  fieldSymType =  3;

  // -----  Tags  -----------------------------------------------------------
  stsTag       = "v15a";
  trdTag       = "v15a_1e";
  tofTag       = "v13-5b";

  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = "sts/sts_" + stsTag + "_std.digi.par";
  trdDigi      = "trd/trd_" + trdTag + ".digi.par";
  tofDigi      = "tof/tof_" + tofTag + ".digi.par";

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "cave.geo";
  pipeGeom     = "pipe/pipe_v14l.root";
  magnetGeom   = "magnet/magnet_v15a.geo.root";
  mvdGeom      = ""; // "mvd/mvd_v15a.geo.root";
  stsGeom      = "sts/sts_" + stsTag + ".geo.root";
  richGeom     = ""; // "rich/rich_v14a_1e.root";
  muchGeom     = "";
  shieldGeom   = "";
  platformGeom = ""; // "passive/platform_v13a.geo";
  trdGeom      = ""; // "trd/trd_" + trdTag + ".geo.root";
  tofGeom      = ""; // "tof/tof_" + tofTag + ".geo.root";
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
