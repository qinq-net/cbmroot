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
  mvdTag       = "v18b_mcbm";      // must be identical to ../setup_sis18_mcbm.C
  stsTag       = "v18f_mcbm";	   // must be identical to ../setup_sis18_mcbm.C
  trdTag       = "v18e_1e_mcbm";   // must be identical to ../setup_sis18_mcbm.C
  tofTag       = "v18e_mcbm";      // must be identical to ../setup_sis18_mcbm.C
  
  // -----  DigiFiles  ------------------------------------------------------
  stsDigi      = "std/sts_" + stsTag + "_std.digi.par";
  trdDigi      = "trd/trd_" + trdTag + ".digi.par";
  tofDigi      = "tof/tof_" + tofTag + ".digi.par";
  tofDigiBdf   = "tof/tof_" + tofTag + ".digibdf.par";

  // -----  Material budget files -------------------------------------------
  mvdMatBudget = "mvd/mvd_" + mvdTag + "_matbudget.root";
  stsMatBudget = "sts/sts_" + stsTag + "_matbudget.root";

  // -----  Geometries  -----------------------------------------------------
  caveGeom     = "cave_mcbm.geo";
  pipeGeom     = "pipe/pipe_v18c_mcbm.geo.root";
  magnetGeom   = "";
  mvdGeom      = "mvd/mvd_" + mvdTag + ".geo.root";
  stsGeom      = "sts/sts_" + stsTag + ".geo.root";
  richGeom     = ""; //no reco// "rich/mrich_prototype_v18b.geo";
  muchGeom     = "much/much_v18a_mcbm.geo";  // no reco // patch CbmMuch.cxx for mCBM first
  // patch much/geo/CbmMuch.cxx macro/mcbm/CbmMuch.cxx_to_McbmGeo.patch
  // make -j3 -C build	   
  shieldGeom   = "";
  platformGeom = ""; // "passive/platform_v13a.geo";
  trdGeom      = "trd/trd_" + trdTag + ".geo.root";
  tofGeom      = "tof/tof_" + tofTag + ".geo.root";
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
