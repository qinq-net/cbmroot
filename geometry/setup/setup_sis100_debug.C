/** ROOT macro to define the CBM setup sis100_debug
 **
 ** Contains: MVD + STS + TRD + TOF + PSD
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

// 2017-11-27 - DE - update geometry tags from sis100_electron
// 2017-05-22 - DE - use TRD v17c as new default
// 2016-06-27 - DE - use STS v16x as new default, see issue #647
// 2016-05-19 - VF - Add platform module
// 2016-02-25 - DE - use STS v16c as new default
// 2016-02-05 - VF - Replaces former sis100_hadron_setup.C,
//                   now using the CbmSetup class.
// 2015-12-11 - DE - use STS v15c as new default
// 2015-07-20 - FU - magnet v15a is v12b with correct keeping volume material
// 2015-07-20 - FU - add material budget files
// 2015-03-13 - DE - use STS v13y with correct ladder orientation as new default
// 2015-01-29 - DE - use chopped PIPE v14q as default
// 2015-01-19 - DE - use TRD v15a as new default
// 2014-06-30 - DE - make TRD v14a the new default
// 2014-06-25 - DE - define digi files through tags for STS, TRD and TOF
// 2013-11-05 - DE - switch to trd_v13p geometry
// 2013-10-11 - DE - add empty string defining the platform
// 2013-10-07 - DE - pipe_v13c.geo fixes overlap with TOF, fills RICH section
// 2013-11-04 - VF - make STS v15b the new default


void setup_sis100_debug()
{

  // -----  Geometry Tags  --------------------------------------------------
  TString magnetGeoTag    = "v18a";
  TString pipeGeoTag      = "v16b_1e";
  TString mvdGeoTag       = "v17a_tr";
  TString stsGeoTag       = "v17a";   
  //  TString stsGeoTag       = "v16g";
  TString richGeoTag      = "v17a_1e";
  TString trdGeoTag       = "v17n_1e";
  TString tofGeoTag       = "v16c_1e";
  TString psdGeoTag       = "v17a";
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
  	std::cout << "-W- setup_sis100_debug: overwriting existing setup"
  			      << setup->GetTitle() << std::endl;
  	setup->Clear();
  }
  setup->SetTitle("SIS100 - Debug Setup");
  setup->SetModule(kMagnet, magnetGeoTag);
  setup->SetModule(kPipe, pipeGeoTag);
  //  setup->SetModule(kMvd, mvdGeoTag);
  setup->SetModule(kSts, stsGeoTag);
  setup->SetModule(kTrd, trdGeoTag);
  setup->SetModule(kTof, tofGeoTag);
  setup->SetModule(kPsd, psdGeoTag);

  setup->SetModule(kPlatform, platGeoTag);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
