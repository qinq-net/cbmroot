/** ROOT macro to define the CBM setup sis100_muon_lmvm
 **
 ** Contains: MVD + STS + MUCH + TRD + TOF
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

// 2016-06-27 - DE - use STS v16x as new default, see issue #647
// 2016-06-21 - DE - switch to shortened root pipe v16b_1m geometry ref #721
// 2016-05-19 - VF - Add platform module
// 2016-02-25 - DE - use STS v16c as new default
// 2016-02-05 - VF - Replaces former sis100_muon_lmvm_setup.C,
//                   now using the CbmSetup class.
// 2015-12-14 - DE - use TRD v15c starting at z = 490 cm for muon_jpsi setup
// 2015-12-14 - DE - rename short SIS100 MUCH version to muon_LMVM setup
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


void setup_sis100_muon_lmvm()
{

  // -----  Geometry Tags  --------------------------------------------------
  TString magnetGeoTag    = "v15b_much";
  TString pipeGeoTag      = "v16b_1m";
  TString stsGeoTag       = "v16x";
  TString muchGeoTag      = "v15b_STS100-B_125cm_no";
  TString shieldGeoTag    = "v15b_SIS100B_149_3part_125cm";
  TString trdGeoTag       = "v15c_1m";
  TString tofGeoTag       = "v16a_1m";
  TString platGeoTag      = "v13a";
  // ------------------------------------------------------------------------


  // -----  Magnetic field  -------------------------------------------------
  TString fieldTag      = "v12b";
  Double_t fieldZ       = 40.;            // field centre z position
  Double_t fieldScale   =  1.;            // field scaling factor
  // ------------------------------------------------------------------------


  // -----  Create setup  ---------------------------------------------------
  CbmSetup* setup = CbmSetup::Instance();
  if ( ! setup->IsEmpty() ) {
  	std::cout << "-W- setup_sis100_muon_lmvm: overwriting existing setup"
  			      << setup->GetTitle() << std::endl;
  	setup->Clear();
  }
  setup->SetTitle("SIS100 - Muon Lmvm Setup");
  setup->SetModule(kMagnet, magnetGeoTag);
  setup->SetModule(kPipe, pipeGeoTag);
  setup->SetModule(kSts, stsGeoTag);
  setup->SetModule(kMuch, muchGeoTag);
  setup->SetModule(kShield, shieldGeoTag);
  setup->SetModule(kTrd, trdGeoTag);
  setup->SetModule(kTof, tofGeoTag);
  setup->SetModule(kPlatform, platGeoTag);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
