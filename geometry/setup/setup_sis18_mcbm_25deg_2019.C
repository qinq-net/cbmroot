
/** ROOT macro to define the CBM setup sis18_mcbm
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

// 2019-03-15 - DE - this is supposed to be the start version of mCBM in March 2019
// 2018-08-24 - DE - this is supposed to be the start version of mCBM in 2018
// 2018-06-27 - DE - set flipped mTOF v18j geometry as new default
// 2018-05-24 - DE - place 20deg_long setup back at 25 degrees, see issue #1078
// 2018-02-27 - DE - use mTRD v18o to fix redmine issue #1046 with tracking
// 2017-12-11 - DE - enable mBUCH v18m
// 2017-12-04 - DE - add mPSD to mCBM setup
// 2017-11-22 - DE - use TRD v17n with 22 cm spacing from CAD
// 2017-11-17 - DE - set aerogel mRICH v18d as default
// 2017-11-10 - DE - switch to mMUCH v18e with Mv2 dimensions
// 2017-11-03 - DE - add long setup, (acceptance matching +-12 degrees from mSTS)
// 2017-11-02 - DE - add common support table v18c
// 2017-11-02 - DE - include mBUCH with mTRD v18j
// 2017-10-23 - DE - use mMUCH with vertically aligned left rim
// 2017-10-18 - DE - use mTOF v18h with vertical orientation
// 2017-10-16 - DE - use 20 degree beampipe
// 2017-06-03 - DE - add RICH v18a_mcbm
// 2017-05-02 - DE - switch back to 5x5 RPC TOF
// 2017-05-02 - DE - skip MVD in the initial setup
// 2016-02-05 - VF - Replaces former sis18_mcbm_setup.C,
//                   now using the CbmSetup class.
// 2015-06-18 - DE - mCBM @ SIS18 setup



void setup_sis18_mcbm_25deg_2019()
{
  
  // -----  Geometry Tags  --------------------------------------------------
//  TString platGeoTag      = "v18a_mcbm";    // concrete wall
  TString platGeoTag      = "v18d_mcbm";    // 2018 // support table
  TString pipeGeoTag      = "v18g_mcbm";    // 2018 // 25 degree beampipe
  TString mvdGeoTag       = "v18b_mcbm";    // "v18a_mcbm";
  TString stsGeoTag       = "v18n_mcbm";    // 2019 // 1-0-0-1 ladder configuration 
  //  TString muchGeoTag      = "v18g_mcbm";    // 11 o'clock, Mv2 size
  TString muchGeoTag      = "v18i_mcbm";    // 2018 // only 2 layers
  //  TString trdGeoTag       = "v18n_mcbm";    // 2018 // 4 TRD modules 
  TString trdGeoTag       = "v18q_mcbm";    // 2018 // 2 TRD modules 
  TString tofGeoTag       = "v18l_mcbm";    // at z = 300 cm
  TString richGeoTag      = "v18h_mcbm";    // long distance
  TString psdGeoTag       = "v18d_mcbm";    // at 25 degree, below the beampipe
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
  setup->SetModule(kPlatform, platGeoTag);
  setup->SetModule(kPipe, pipeGeoTag);
  //  setup->SetModule(kMvd, mvdGeoTag);  // skip mvd in the initial setup
  setup->SetModule(kSts,  stsGeoTag);
  setup->SetModule(kMuch, muchGeoTag);
  setup->SetModule(kTrd,  trdGeoTag);
  setup->SetModule(kTof,  tofGeoTag);
  setup->SetModule(kRich, richGeoTag);
  // 2019  setup->SetModule(kPsd,  psdGeoTag);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
