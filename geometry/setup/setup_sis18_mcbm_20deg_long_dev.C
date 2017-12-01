/** ROOT macro to define the CBM setup sis18_mcbm
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

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



void setup_sis18_mcbm_20deg_long_dev(Int_t nSetup = 901101110)
{
   
  TString myName = "setup_sis18_mcbm_20deg_long_dev";  // this macro's name for screen output

  // -----  Geometry Tags  --------------------------------------------------
  //  TString platGeoTag      = "v18a_mcbm";    // concrete wall
  TString platGeoTag      = "v18c_mcbm";    // support table
  TString pipeGeoTag      = "v18f_mcbm";    // 20 degree beampipe

  TString mvdGeoTag       = "v18b_mcbm";    // "v18a_mcbm";
  TString stsGeoTag       = "v18f_mcbm";    // 1-1-1-2 ladder configuration
  TString muchGeoTag      = "v18g_mcbm";    // 11 o'clock, Mv2 size
  TString buchGeoTag      = "v18m_1e_mcbm"; // long, with mBUCH

  TString trdGeoTag       = "v18n_1e_mcbm"; // long, without mBUCH
  TString tofGeoTag       = "v18i_mcbm";    // long distance
  TString richGeoTag      = "v18d_mcbm";    // long distance
  TString psdGeoTag       = "";             // not yet available
  // ------------------------------------------------------------------------


  // -----  Magnetic field  -------------------------------------------------
  TString fieldTag      = "v12b";
  Double_t fieldZ       = 40.;            // field centre z position
  Double_t fieldScale   =  0.;            // field scaling factor
  // ------------------------------------------------------------------------

  // -----  Define Activity  ------------------------------------------------
  Int_t mMvd  = 0;
  Int_t mSts  = 0;
  Int_t mMuch = 0;
  Int_t mBuch = 0;

  Int_t mTrd  = 0;
  Int_t mTof  = 0;
  Int_t mRich = 0;
  Int_t mPsd  = 0;

  cout << "DE setup: "<< nSetup << endl;

////  cout << nSetup /100000000000 %10 << endl; 
////  cout << nSetup /10000000000  %10 << endl; 
////  cout << nSetup /1000000000   %10 << endl; 
////  cout << nSetup /100000000    %10 << endl; 
//
//  cout << nSetup /10000000     %10 << endl; 
//  cout << nSetup /1000000      %10 << endl; 
//  cout << nSetup /100000       %10 << endl; 
//  cout << nSetup /10000        %10 << endl; 
//
//  cout << nSetup /1000         %10 << endl; 
//  cout << nSetup /100          %10 << endl; 
//  cout << nSetup /10           %10 << endl; 
//  cout << nSetup               %10 << endl; 

  if (nSetup /10000000     %10) { mMvd  = 1; cout << myName << ": mMVD  enabled" << endl; }
  if (nSetup /1000000      %10) { mSts  = 1; cout << myName << ": mSTS  enabled" << endl; }
  if (nSetup /100000       %10) { mMuch = 1; cout << myName << ": mMUCH enabled" << endl; }
  if (nSetup /10000        %10) { mBuch = 1; cout << myName << ": mBUCH enabled" << endl; }

  if (nSetup /1000         %10) { mTrd  = 1; cout << myName << ": mTRD  enabled" << endl; }
  if (nSetup /100          %10) { mTof  = 1; cout << myName << ": mTOF  enabled" << endl; }
  if (nSetup /10           %10) { mRich = 1; cout << myName << ": mRICH enabled" << endl; }
  if (nSetup               %10) { mPsd  = 1; cout << myName << ": mPSD  enabled" << endl; }
  cout << endl;

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

  if (mMvd  == 1) setup->SetModule(kMvd,  mvdGeoTag); 
  if (mSts  == 1) setup->SetModule(kSts,  stsGeoTag);
  if (mMuch == 1) setup->SetModule(kMuch, muchGeoTag);
  if (mBuch == 1) setup->SetModule(kTrd,  buchGeoTag);
  else if (mTrd  == 1) setup->SetModule(kTrd,  trdGeoTag);

  if (mTof  == 1) setup->SetModule(kTof,  tofGeoTag);
  if (mRich == 1) setup->SetModule(kRich, richGeoTag);
  if (mPsd  == 1) setup->SetModule(kPsd,  psdGeoTag); 
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);

  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
