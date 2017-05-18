/** ROOT macro to define the CBM setup sis18_mcbm
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

// 2017-05-02 - DE - switch back to 5x5 RPC TOF
// 2017-05-02 - DE - skip MVD in the initial setup
// 2016-02-05 - VF - Replaces former sis18_mcbm_setup.C,
//                   now using the CbmSetup class.
// 2015-06-18 - DE - mCBM @ SIS18 setup



void setup_sis18_mcbm()
{
  
  // -----  Geometry Tags  --------------------------------------------------
  TString pipeGeoTag      = "v18d_mcbm";    // "v18c_mcbm";
  TString mvdGeoTag       = "v18b_mcbm";    // "v18a_mcbm";						  
  TString stsGeoTag       = "v18f_mcbm";    // "v18e_mcbm"; // "v18d_mcbm";
  TString muchGeoTag      = "v18a_mcbm";
  TString trdGeoTag       = "v18e_1e_mcbm"; // "v18g_1e_mcbm"; // "v18d_1e_mcbm"; // "v18c_1e_mcbm";  // "v18b_1e_mcbm";  // "v18a_1e_mcbm";
  TString tofGeoTag       = "v18e_mcbm";    //  "v18d_mcbm";
  TString platGeoTag      = "v18a_mcbm";
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
  setup->SetModule(kMuch, muchGeoTag);
  setup->SetModule(kTrd, trdGeoTag);
  setup->SetModule(kTof, tofGeoTag);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
