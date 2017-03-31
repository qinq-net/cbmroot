//
//   comments
//

// 2015-06-18 - DE - mCBM @ SIS18 setup
//


void sis18_mcbm_setup()
{
  // -----   Magnetic field   -----------------------------------------------
  fieldMap     = "field_v12b";   // name of field map
  fieldZ       = 40.;            // field centre z position
  fieldScale   =  0.; // 1.;     // field scaling factor
  fieldSymType =  3;

  // -----  Tags  -----------------------------------------------------------
  mvdTag       = "v18b";     // "v18a";
  stsTag       = "v18e";     // "v18d";
  trdTag       = "v18d_1e";  // "v18c_1e";  // "v18b_1e";  // "v18a_1e";
  tofTag       = "v13_7a";   // "v13_6a";   // "v13_5a"; 
  
  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = "mcbm/sts_" + stsTag + "_std.digi.par";
  trdDigi      = "trd/mtrd_" + trdTag + ".digi.par";
  tofDigi      = "tof/mtof_" + tofTag + ".digi.par";
  tofDigiBdf   = "tof/mtof_" + tofTag + ".digibdf.par";

  // -----  Material budget files -------------------------------------------
  mvdMatBudget = "mcbm/mvd_" + mvdTag + "_matbudget.root";
  stsMatBudget = "mcbm/sts_" + stsTag + "_matbudget.root";

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "mcave_mcbm.geo";
  pipeGeom     = "pipe/mpipe_v18c.root";
  magnetGeom   = "";
  mvdGeom      = "mvd/mmvd_" + mvdTag + ".geo.root";
  stsGeom      = "sts/msts_" + stsTag + ".geo.root";
  richGeom     = ""; //no reco// "rich/mrich_prototype_v18b.geo";
  muchGeom     = "much/mmuch_v18a.geo";  // no reco // patch CbmMuch.cxx for mCBM first
  // patch much/geo/CbmMuch.cxx macro/mcbm/CbmMuch.cxx_to_McbmGeo.patch
  // make -j3 -C build	   
  shieldGeom   = "";
  platformGeom = ""; // "passive/platform_v13a.geo";
  trdGeom      = "trd/mtrd_" + trdTag + ".geo.root";
  tofGeom      = "tof/mtof_" + tofTag + ".geo.root";
  ecalGeom     = "";
  psdGeom      = ""; // "psd_geo_xy.txt";
  psdZpos = 800.;
  psdXpos = 11.;

  // ------ Input file ------------------------------------------------------

  defaultInputFile = "/input/urqmd.agag.1.65gev.centr.00001.root";

  //  defaultInputFile = "/input/urqmd.agag.1.65gev.mbias.00001.root";
  //
  //  defaultInputFile = "/input/urqmd.niau.1.93gev.centr.00001.root";
  //  defaultInputFile = "/input/urqmd.niau.1.93gev.mbias.00001.root";
  //
  //  defaultInputFile = "/input/urqmd.nini.1.93gev.centr.00001.root";
  //  defaultInputFile = "/input/urqmd.nini.1.93gev.mbias.00001.root";
  //
  //  defaultInputFile = "/input/urqmd.pau.4.5gev.mbias.00001.root";

  cout << "Reading geometry setup and magnetic field settings from "
       << gSystem->UnixPathName(gInterpreter->GetCurrentMacroName())
       << endl;
}
