/** ROOT macro to define the CBM setup sis300_electron
 **
 ** Contains: MVD + STS + RICH + TRD + TOF
 **
 ** @author Volker Friese <v.friese@gsi.de>
 ** @author David Emschermann <d.emschermann@gsi.de>
 ** @date   5 February 2016
 **
 **/

// 2016-06-27 - DE - use STS v16x as new default, see issue #647
// 2016-02-25 - DE - use STS v16c as new default
// 2016-02-05 - VF - Replaces former sis300_electron_setup.C,
//                   now using the CbmSetup class.
// 2015-12-11 - DE - use STS v15c as new default
// 2015-07-20 - FU - magnet v15a is v12b with correct keeping volume material
// 2015-07-20 - FU - add material budget files
// 2015-03-13 - DE - use STS v13y with correct ladder orientation as new default
// 2015-01-28 - DE - use STS v13x fitting with with PIPE v14n
// 2015-01-22 - DE - use MVD v14b with PIPE v14n as default
// 2015-01-19 - DE - use TRD v15a as new default
// 2014-07-05 - DE - use PIPE v14y as interim solution
// 2014-07-03 - DE - use PIPE v14h together with RICH v14a -> overlap with STS
// 2014-06-30 - DE - make RICH v14a the new default
// 2014-06-30 - DE - make TRD v14a the new default
// 2014-06-25 - DE - define digi files through tags for STS, TRD and TOF
// 2013-11-05 - DE - switch to trd_v13p geometry
// 2013-10-11 - DE - add empty string defining the platform
// 2013-10-07 - DE - pipe_v13d.geo fixes overlap with TOF
// 2013-10-07 - DE - rich_v13a is not at nominal position 1800mm, but at 1600mm
// 2013-11-04 - VF - make STS v15b the new default


void setup_sis300_electron_flow()
{

  // -----  Geometry Tags  --------------------------------------------------
  TString magnetGeoTag    = "v15a";
  TString pipeGeoTag      = "v14c";
  TString mvdGeoTag       = "v17a_tr";
  TString stsGeoTag       = "v16x";
  TString richGeoTag      = "v17a_3e";
  TString trdGeoTag       = "v17c_3e";
  TString tofGeoTag       = "v16c_3e";
  // ------------------------------------------------------------------------


  // -----  Magnetic field  -------------------------------------------------
  TString fieldTag      = "v12b";
  Double_t fieldZ       = 40.;            // field centre z position
  Double_t fieldScale   =  1.;            // field scaling factor
  TString En = gSystem->Getenv("BEAM_ENERGY");
  cout << "Energy: " << En << endl;
  Int_t energy;
  if (En.IsDec()) {
    energy = En.Atoi();
  } else {
    cout << "Value is not an integer" << endl;
  }
  cout << "Energy: " << energy << endl;
  // field scaling factor
  if ( 35 == energy ) { fieldScale = 1.; }
  else if ( 25 == energy ) { fieldScale = 1.; }
  else if ( 15 == energy ) { fieldScale = 1.; }
  else if ( 10 == energy ) { fieldScale = 1.; }            
  else if ( 6 == energy)  { fieldScale = 0.632; }
  else if ( 4 == energy )  { fieldScale = 0.632; }    
  else if ( 2 == energy )  { fieldScale = 0.5; } 
  else {
    cout << "Energy " << En << "not supported" << endl;
  }

  // -----  PSD  ------------------------------------------------------------
  TString  psdGeoFile = "psd_geo_xy.txt";
  Double_t psdZpos = 1500.;
  // PSD X-shift (in cm) vs evergy (2, 4, 6, 10, 15, 25, 35) AGeV vs Z-position (6, 8, 10, 15)m
  Float_t psdXshiftvsZ[7][4];
  Float_t psdXshift;
  int energy_ind;
  int distance_ind;

  // WARNING: PSD X-shift below given for field v10e !!

  // 2 agev
  psdXshiftvsZ[0][0] = 21.4;   // 8m
  psdXshiftvsZ[0][1] = 27.3;   // 10m
  psdXshiftvsZ[0][2] = 33.1;   // 12m
  psdXshiftvsZ[0][3] = 41.9;   // 15m

  // 4 agev
  psdXshiftvsZ[1][0] = 15.5;
  psdXshiftvsZ[1][1] = 19.8;
  psdXshiftvsZ[1][2] = 24.;
  psdXshiftvsZ[1][3] = 30.4;

  // 6 agev
  psdXshiftvsZ[2][0] = 10.9;
  psdXshiftvsZ[2][1] = 13.9;
  psdXshiftvsZ[2][2] = 16.9;
  psdXshiftvsZ[2][3] = 21.4;

  // 10 agev
  psdXshiftvsZ[3][0] = 10.9;
  psdXshiftvsZ[3][1] = 13.9;
  psdXshiftvsZ[3][2] = 16.9;
  psdXshiftvsZ[3][3] = 21.4;

  // 15 agev
  psdXshiftvsZ[4][0] = 7.5;
  psdXshiftvsZ[4][1] = 9.5;
  psdXshiftvsZ[4][2] = 11.6;
  psdXshiftvsZ[4][3] = 14.7;

  // 25 agev
  psdXshiftvsZ[5][0] = 4.6;
  psdXshiftvsZ[5][1] = 5.8;
  psdXshiftvsZ[5][2] = 7.1;
  psdXshiftvsZ[5][3] = 9.;

  // 35 agev
  psdXshiftvsZ[6][0] = 3.3;
  psdXshiftvsZ[6][1] = 4.2;
  psdXshiftvsZ[6][2] = 5.1;
  psdXshiftvsZ[6][3] = 6.5;

  
  if ( 35 == energy ) { energy_ind = 6; }
  else if ( 25 == energy ) { energy_ind = 5; }
  else if ( 15 == energy ) { energy_ind = 4; }
  else if ( 10 == energy ) { energy_ind = 3; }            
  else if ( 6 == energy )  { energy_ind = 2; }
  else if ( 4 == energy )  { energy_ind = 1; }    
  else if ( 2 == energy )  { energy_ind = 0; } 
  else {
    cout << "Energy " << En << "not supported" << endl;
  }

  if (psdZpos == 800.) distance_ind = 0;
  if (psdZpos == 1000.) distance_ind = 1;
  if (psdZpos == 1200.) distance_ind = 2;
  if (psdZpos == 1500.) distance_ind = 3;

  Double_t psdXpos = psdXshiftvsZ[energy_ind][distance_ind];
  // ------------------------------------------------------------------------

  // -----  Create setup  ---------------------------------------------------
  CbmSetup* setup = CbmSetup::Instance();
  if ( ! setup->IsEmpty() ) {
  	std::cout << "-W- setup_sis300_electron_flow: overwriting existing setup"
  			      << setup->GetTitle() << std::endl;
  	setup->Clear();
  }
  setup->SetTitle("SIS300 - Electron Setup");
  setup->SetModule(kMagnet, magnetGeoTag);
  setup->SetModule(kPipe, pipeGeoTag);
  setup->SetModule(kSts, stsGeoTag);
  setup->SetModule(kRich, richGeoTag);
  setup->SetModule(kTrd, trdGeoTag);
  setup->SetModule(kTof, tofGeoTag);
  setup->SetPsd(psdGeoFile, psdZpos, psdXpos);
  setup->SetField(fieldTag, fieldScale, 0., 0., fieldZ);
  // ------------------------------------------------------------------------


  // -----   Screen output   ------------------------------------------------
  setup->Print();
  // ------------------------------------------------------------------------

}
