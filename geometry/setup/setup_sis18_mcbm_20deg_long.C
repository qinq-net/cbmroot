/** ROOT macro to define the CBM setup sis18_mcbm
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

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



void setup_sis18_mcbm_20deg_long()
{
  
  // -----  Geometry Tags  --------------------------------------------------
//  TString platGeoTag      = "v18a_mcbm";    // concrete wall
  TString platGeoTag      = "v18c_mcbm";    // support table
  TString pipeGeoTag      = "v18f_mcbm";    // 20 degree beampipe
  TString mvdGeoTag       = "v18b_mcbm";    // "v18a_mcbm";
  TString stsGeoTag       = "v18f_mcbm";    // 1-1-1-2 ladder configuration 
  TString muchGeoTag      = "v18g_mcbm";    // 11 o'clock, Mv2 size
//  TString trdGeoTag       = "v18m_mcbm";    // long, with mBUCH
  TString trdGeoTag       = "v18n_mcbm";    // long, without mBUCH
  TString tofGeoTag       = "v18i_mcbm";    // long distance
  TString richGeoTag      = "v18d_mcbm";    // long distance
  TString psdGeoTag       = "v18c_mcbm";    // below the beampipe
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
  setup->SetModule(kPsd,  psdGeoTag);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
