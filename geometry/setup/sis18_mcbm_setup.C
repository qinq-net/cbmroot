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
  trdTag       = "v18b_1e";  // "v18a_1e";
  tofTag       = "v13_6a";   // "v13_5a"; 
  
  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = "mcbm/sts_" + stsTag + "_std.digi.par";
  trdDigi      = "mcbm/trd_" + trdTag + ".digi.par";
  tofDigi      = "mcbm/tof_" + tofTag + ".digi.par";

  // -----  Material budget files -------------------------------------------
  mvdMatBudget = "mcbm/mvd_" + mvdTag + "_matbudget.root";
  stsMatBudget = "mcbm/sts_" + stsTag + "_matbudget.root";

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "mcbm/cave_mcbm.geo";
  pipeGeom     = "mcbm/pipe_v18b.root";
  magnetGeom   = ""; // magnet/magnet_v15a.geo.root";
  mvdGeom      = "mcbm/mvd_" + mvdTag + ".geo.root";
  stsGeom      = "mcbm/sts_" + stsTag + ".geo.root";
  richGeom     = ""; //no reco// "mcbm/rich_prototype_v18b.geo";
  muchGeom     = ""; //no reco// "mcbm/much_v18a.geo";
  shieldGeom   = "";
  platformGeom = ""; // "passive/platform_v13a.geo";
  trdGeom      = "mcbm/trd_" + trdTag + ".geo.root";
  tofGeom      = "mcbm/tof_" + tofTag + ".geo.root";
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
