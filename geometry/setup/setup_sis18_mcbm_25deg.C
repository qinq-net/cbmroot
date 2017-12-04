/** ROOT macro to define the CBM setup sis18_mcbm
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

// 2017-10-18 - DE - use mTOF v18h with vertical orientation
// 2017-10-16 - DE - use 25 degree beampipe
// 2017-06-03 - DE - add RICH v18a_mcbm
// 2017-05-02 - DE - switch back to 5x5 RPC TOF
// 2017-05-02 - DE - skip MVD in the initial setup
// 2016-02-05 - VF - Replaces former sis18_mcbm_setup.C,
//                   now using the CbmSetup class.
// 2015-06-18 - DE - mCBM @ SIS18 setup



void setup_sis18_mcbm_25deg()
{
  
  // -----  Geometry Tags  --------------------------------------------------
//  TString platGeoTag      = "v18a_mcbm";    // concrete wall
  TString platGeoTag      = "v18c_mcbm";    // support table
  TString pipeGeoTag      = "v18d_mcbm";    // 25 degreee beampipe 
  TString mvdGeoTag       = "v18b_mcbm";    // "v18a_mcbm";						  
  TString stsGeoTag       = "v18f_mcbm";    // 1-1-1-2 ladder configuration
//  TString muchGeoTag      = "v18c_mcbm";    // 12 o'clock
  TString muchGeoTag      = "v18d_mcbm";    // 11 o'clock
  TString trdGeoTag       = "v18e_mcbm";    // short, without mBUCH
//  TString trdGeoTag       = "v18k_mcbm";    // short, with mBUCH
  TString tofGeoTag       = "v18h_mcbm";    // short distance
  TString richGeoTag      = "v18b_mcbm";    // short distance
  // ------------------------------------------------------------------------


  // -----  Magnetic field  -------------------------------------------------
  TString fieldTag      = "v12b";
  Double_t fieldZ       = 40.;            // field centre z position
  Double_t fieldScale   =  0.;            // field scaling factor
  // ------------------------------------------------------------------------


  // -----  Create setup  ---------------------------------------------------
  CbmSetup* setup = CbmSetup::Instance();
  if ( ! setup->IsEmpty() ) {
  	std::cout << "-W- setup_sis18_mcbm: overwriting existing setup"
  			      << setup->GetTitle() << std::endl;
  	setup->Clear();
  }
  setup->SetTitle("SIS18 - MCBM Setup");
  setup->SetModule(kPipe, pipeGeoTag);
  setup->SetModule(kPlatform, platGeoTag);
  //  setup->SetModule(kMvd, mvdGeoTag);  // skip mvd in the initial setup
  setup->SetModule(kSts, stsGeoTag);
  // disable, reco missing
  //  setup->SetModule(kMuch, muchGeoTag);
  setup->SetModule(kTrd, trdGeoTag);
  setup->SetModule(kTof, tofGeoTag);
  // disable, reco missing
  //  setup->SetModule(kRich, richGeoTag);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
