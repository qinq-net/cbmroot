/** ROOT macro to define the CBM setup sis300_muon
 **
 ** Contains: STS + MUCH + TRD + TOF
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

// 2016-06-27 - DE - use STS v16x as new default, see issue #647
// 2016-05-19 - VF - Add platform module
// 2016-02-25 - DE - use STS v16c as new default
// 2016-02-05 - VF - Replaces former sis300_electron_muon.C,
//                   now using the CbmSetup class.
// 2015-12-14 - DE - use TOF v16a
// 2015-12-11 - DE - use STS v15c as new default
// 2015-07-20 - FU - magnet v15a is v12b with correct keeping volume material
// 2015-07-20 - FU - add material budget files
// 2015-01-19 - DE - use TRD v15a as new default
// 2014-06-30 - DE - make TRD v14a the new default
// 2014-06-25 - DE - define digi files through tags for STS, TRD and TOF
// 2013-11-05 - DE - switch to trd_v13p geometry
// 2013-10-11 - DE - add empty string defining the platform
// 2013-10-07 - DE - pipe_v13e.geo fixes overlap with TOF, also avoids TRD
// 2013-11-04 - VF - make STS v15b the new default


void setup_sis300_muon()
{

  // -----  Geometry Tags  --------------------------------------------------
  TString magnetGeoTag    = "v15a";
  TString pipeGeoTag      = "v13c";
  TString stsGeoTag       = "v16x";
  TString muchGeoTag      = "v13f";
  TString trdGeoTag       = "v15a_3m";
  TString tofGeoTag       = "v16a_3m";
  TString platGeoTag      = "v13b";
  // ------------------------------------------------------------------------


  // -----  Magnetic field  -------------------------------------------------
  TString fieldTag      = "v12b";
  Double_t fieldZ       = 40.;            // field centre z position
  Double_t fieldScale   =  1.;            // field scaling factor
  // ------------------------------------------------------------------------


  // -----  PSD  ------------------------------------------------------------
  TString  psdGeoFile = "psd_geo_xy.txt";
  Double_t psdZpos = 1500.;
  Double_t psdXpos = 10.;
  // ------------------------------------------------------------------------


  // -----  Create setup  ---------------------------------------------------
  CbmSetup* setup = CbmSetup::Instance();
  if ( ! setup->IsEmpty() ) {
  	std::cout << "-W- setup_sis300_muon: overwriting existing setup"
  			      << setup->GetTitle() << std::endl;
  	setup->Clear();
  }
  setup->SetTitle("SIS300 - Muon Setup");
  setup->SetModule(kMagnet, magnetGeoTag);
  setup->SetModule(kPipe, pipeGeoTag);
  setup->SetModule(kSts, stsGeoTag);
  setup->SetModule(kMuch, muchGeoTag);
  setup->SetModule(kTrd, trdGeoTag);
  setup->SetModule(kTof, tofGeoTag);
  setup->SetModule(kPlatform, platGeoTag);
  setup->SetPsd(psdGeoFile, psdZpos, psdXpos);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
