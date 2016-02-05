/** ROOT macro to define the CBM setup sis18_mcbm
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

// 2016-02-05 - VF - Replaces former sis18_mcbm_setup.C,
//                   now using the CbmSetup class.
// 2015-06-18 - DE - mCBM @ SIS18 setup



void setup_sis18_mcbm()
{

  // -----  Geometry Tags  --------------------------------------------------
  TString pipeGeoTag      = "v18b";
  TString mvdGeoTag       = "v18b";
  TString stsGeoTag       = "v18e";
  TString trdGeoTag       = "v18b_1e";
  TString tofGeoTag       = "v13a_6a";
  TString psdGeoTag       = "psd_geo_xy.txt";
  // ------------------------------------------------------------------------


  // -----  Magnetic field  -------------------------------------------------
  TString fieldTag      = "v12b";
  Double_t fieldZ       = 40.;            // field centre z position
  Double_t fieldScale   =  1.;            // field scaling factor
  // ------------------------------------------------------------------------


  // -----  PSD  ------------------------------------------------------------
  TString  psdGeoFile = "psd_geo_xy.txt";
  Double_t psdZpos = 800.;
  Double_t psdXpos = 11.;
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
  setup->SetModule(kMvd, mvdGeoTag);
  setup->SetModule(kSts, stsGeoTag);
  setup->SetModule(kTrd, trdGeoTag);
  setup->SetModule(kTof, tofGeoTag);
  setup->SetPsd(psdGeoFile, psdZpos, psdXpos);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
