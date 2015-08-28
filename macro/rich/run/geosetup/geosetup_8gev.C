
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

static int nofNoiseHitsInRich;
static double collectionEff;
static double sigmaErrorRich;
static double crosstalkRich;

static Double_t trdAnnCut;
static Int_t minNofPointsTrd;
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
  trdTag       = "v15a_1e";
  tofTag       = "v13-5d";

  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = "sts/sts_" + stsTag + "_std.digi.par";
  trdDigi      = "trd/trd_" + trdTag + ".digi.par";
  tofDigi      = "tof/tof_" + tofTag + ".digi.par";

  // -----  Material budget files -------------------------------------------
  mvdMatBudget = "";
  stsMatBudget = "sts/sts_matbudget_" + stsTag + ".root";

  //TRD and TOF hit producer types
  trdHitProducerType = "smearing"; // smearing, digi, clustering

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "cave.geo";
  pipeGeom     = "pipe/pipe_v14l.root";
  magnetGeom   = "magnet/magnet_v15a.geo.root";
  mvdGeom      = "";
  stsGeom      = "sts/sts_" + stsTag + ".geo.root";
  richGeom     = "rich/rich_v14a_1e.root";
  trdGeom      = "trd/trd_" + trdTag + ".geo.root";
  tofGeom      = "tof/tof_" + tofTag + ".geo.root";

  // rich parameters
  nofNoiseHitsInRich = 220;
  collectionEff = 1.0;
  sigmaErrorRich = 0.06;
  crosstalkRich = 0.02;

  //some other parameters
  trdAnnCut = 0.85;
  minNofPointsTrd = 6;

  cout << "Reading geometry setup and magnetic field settings from "
       << gSystem->UnixPathName(gInterpreter->GetCurrentMacroName())
       << endl;
}
