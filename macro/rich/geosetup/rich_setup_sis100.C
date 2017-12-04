
void do_setup()
{

  TString magnetGeoTag    = "v15a";
  TString pipeGeoTag      = "v16b_1e";
 // TString mvdGeoTag       = "v17a_tr";
  TString stsGeoTag       = "v16g";
  TString richGeoTag      = "v18a_1e";
  TString trdGeoTag       = "v17n_1e";
  TString tofGeoTag       = "v16c_1e";
 // TString psdGeoTag       = "v17a";
 // TString platGeoTag      = "v13a";

  TString fieldTag = "v12b";
  Double_t fieldZ = 40.; // field centre z position
  Double_t fieldScale = 1.; // field scaling factor


  CbmSetup* setup = CbmSetup::Instance();
  if ( ! setup->IsEmpty() ) {
  	std::cout << "-W- setup_sis100_electron: overwriting existing setup" << setup->GetTitle() << std::endl;
  	setup->Clear();
  }
  setup->SetTitle("RICH SIS100");
  setup->SetModule(kMagnet, magnetGeoTag);
  setup->SetModule(kPipe, pipeGeoTag);
 // setup->SetModule(kMvd, mvdGeoTag);
  setup->SetModule(kSts, stsGeoTag);
  setup->SetModule(kRich, richGeoTag);
 // setup->SetModule(kTrd, trdGeoTag);
 // setup->SetModule(kTof, tofGeoTag);
  //setup->SetModule(kPsd, psdGeoTag);
  //setup->SetModule(kPlatform, platGeoTag);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);


  setup->Print();

}
