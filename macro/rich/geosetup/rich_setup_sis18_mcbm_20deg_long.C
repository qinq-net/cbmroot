
void do_setup()
{
  
  // -----  Geometry Tags  --------------------------------------------------
//  TString platGeoTag      = "v18a_mcbm";    // concrete wall
  TString platGeoTag      = "v18c_mcbm";    // support table
  TString pipeGeoTag      = "v18f_mcbm";    // 20 degree beampipe
  TString mvdGeoTag       = "v18b_mcbm";    // "v18a_mcbm";
  TString stsGeoTag       = "v18g_mcbm";    // 1-1-1-2 ladder configuration 
  TString muchGeoTag      = "v18g_mcbm";    // 11 o'clock, Mv2 size
  TString trdGeoTag       = "v18m_mcbm";    // long, with mBUCH
  //  TString trdGeoTag       = "v18n_mcbm";    // long, without mBUCH
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
