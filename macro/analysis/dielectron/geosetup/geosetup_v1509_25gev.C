
#ifndef __CLING__
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

static TString trdHitProducerType;

static TString mvdMatBudget;
static TString stsMatBudget;

static TString  fieldMap;
static Double_t fieldZ;
static Double_t fieldScale;
static Int_t    fieldSymType;

static TString defaultInputFile;

static Double_t trdAnnCut;
static Double_t pionMisidLevel;
static Int_t litQaMinNofPointsTrd;
#endif

void init_geo_setup()
{
  // -----   Magnetic field   -----------------------------------------------
  fieldMap     = "field_v12b";   // name of field map
  fieldZ       = 40.;            // field centre z position
  fieldScale   =  1.;            // field scaling factor
  fieldSymType =  3;

  // -----  Tags  -----------------------------------------------------------
  mvdTag       = "";
  stsTag       = "v13y";
  trdTag       = "v15a_3e";
  tofTag       = "v13-5d";

  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = "sts/sts_" + stsTag + "_std.digi.par";
  trdDigi      = "trd/trd_" + trdTag + ".digi.par";
  tofDigi      = "tof/tof_" + tofTag + ".digi.par";

  // -----  Material budget files -------------------------------------------
  mvdMatBudget = "";
  stsMatBudget = "sts/sts_matbudget_" + stsTag + ".root";

  //TRD and TOF hit producer types
  trdHitProducerType = "clustering"; // smearing, digi, clustering

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "cave.geo";
  pipeGeom     = "pipe/pipe_v14n.root";
  magnetGeom   = "magnet/magnet_v15a.geo.root";
  mvdGeom      = "";
  stsGeom      = "sts/sts_" + stsTag + ".geo.root";
  richGeom     = "rich/rich_v14a_3e.root";
  platformGeom = ""; // "passive/platform_v13b.geo";
  trdGeom      = "trd/trd_" + trdTag + ".geo.root";
  tofGeom      = "tof/tof_" + tofTag + ".geo.root";

  // define some LMVM and QA cuts
  trdAnnCut = 0.85;
  pionMisidLevel = -1.;
  litQaMinNofPointsTrd = 8;


  // ------ Input file ------------------------------------------------------

  defaultInputFile = "/input/urqmd.auau.25gev.centr.root";

  cout << "Reading geometry setup and magnetic field settings from "
       << gSystem->UnixPathName(gInterpreter->GetCurrentMacroName())
       << endl;
}
