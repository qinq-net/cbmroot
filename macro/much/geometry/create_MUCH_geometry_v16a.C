///                                             
/// \file create_MUCH_geometry_v16a.C
/// \brief Generates MUCH geometry in Root format.
///                                             

// 2016-04-19 - DE - v16a - initial version derived from TRD

// in root all sizes are given in cm

#include "TSystem.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoPgon.h"
#include "TGeoMatrix.h"
#include "TGeoCompositeShape.h"
#include "TGeoXtru.h"
#include "TFile.h"
#include "TString.h"
#include "TList.h"
#include "TRandom3.h"
#include "TDatime.h"

#include <iostream>

// Name of output file with geometry
const TString tagVersion   = "v16a";
const TString subVersion   = "";
//const TString subVersion   = "_1h";
//const TString subVersion   = "_1e";
//const TString subVersion   = "_1m";
//const TString subVersion   = "_3e";
//const TString subVersion   = "_3m";
const TString geoVersion   = "much_" + tagVersion + subVersion;
const TString FileNameSim  = geoVersion + ".geo.root";
const TString FileNameGeo  = geoVersion + "_geo.root";
const TString FileNameInfo = geoVersion + ".geo.info";

// display switches
const Bool_t IncludeRadiator    = true;  // false;  // true, if radiator is included in geometry
const Bool_t IncludeKaptonFoil  = true;  // false;  // true, if entrance window is included in geometry
const Bool_t IncludeGasFrame    = true;  // false;  // true, if frame around gas volume is included in geometry
const Bool_t IncludePadplane    = true;  // false;  // true, if padplane is included in geometry
const Bool_t IncludeBackpanel   = true;  // false;  // true, if backpanel is included in geometry

// Parameters defining the layout of the complete detector build out of different detector layers.
const Int_t   MaxLayers = 10;   // max layers

const Int_t    ShowLayer[MaxLayers] = { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };  // SIS300-e   // 1: plot, 0: hide

Int_t    PlaneId[MaxLayers]; // automatically filled with layer ID

const Int_t   LayerType[MaxLayers]        = { 10, 11, 10, 11, 10, 11, 10, 11, 10, 11 };  // ab: a [1-3] - layer type, b [0,1] - vertical/horizontal pads
// ### Layer Type 11 is Layer Type 1 with detector modules rotated by 90°
// ### Layer Type 21 is Layer Type 2 with detector modules rotated by 90°
// ### Layer Type 31 is Layer Type 3 with detector modules rotated by 90°
// In the subroutine creating the layers this is recognized automatically 

const Int_t   LayerNrInStation[MaxLayers] = { 1, 2, 3, 4, 1, 2, 3, 4, 1, 2 };

Double_t LayerPosition[MaxLayers] = { 410. }; // start position - 2013-10-28 - DE - v14_3e - SIS 300 electron       (10 layers, z = 4100 )

const Double_t LayerThickness = 45.0; // Thickness of one TRD layer in cm

const Double_t LayerOffset[MaxLayers] = {   0.,   0.,   0.,   0.,   5.,   0.,   0.,   0.,   5.,   0. };  // v13x[4,5] - z offset in addition to LayerThickness 

const Int_t LayerArraySize[3][4] =  { { 5, 5, 9, 11 },    // for layer[1-3][i,o] below
                                      { 5, 5, 9, 11 },
                                      { 5, 5, 9, 11 } };


// ### Layer Type 1
// v14x - module types in the inner sector of layer type 1 - looking upstream
const Int_t layer1i[5][5] = { { 323,  323,  321,  321,  321 },    // abc: a module type - b orientation (x90 deg) in odd - c even layers
                              { 223,  123,  121,  121,  221 },  
                              { 203,  103,    0,  101,  201 },  
                              { 203,  103,  101,  101,  201 },  
                              { 303,  303,  301,  301,  301 } };
// number of modules: 24

// v14x - module types in the outer sector of layer type 1 - looking upstream
const Int_t layer1o[9][11]= { {  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },
                              {  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },
                              {  0,    0,  823,  823,  723,  721,  721,  821,  821,    0,    0 },
                              {  0,    0,  823,  623,    0,    0,    0,  621,  821,    0,    0 },
                              {  0,    0,  703,  603,    0,    0,    0,  601,  701,    0,    0 },
                              {  0,    0,  803,  603,    0,    0,    0,  601,  801,    0,    0 },
                              {  0,    0,  803,  803,  703,  701,  701,  801,  801,    0,    0 },
                              {  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 },
                              {  0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0 } };
// number of modules: 26
// Layer1 =  24 + 26;   // v14a

// Parameters defining the layout of the different detector modules
const Int_t NofModuleTypes = 8;
const Int_t ModuleType[NofModuleTypes]    = {  0,  0,  0,  0,  1,  1,  1,  1 }; // 0 = small module, 1 = large module

// GBTx ROB definitions
const Int_t RobsPerModule[NofModuleTypes] = {  2,  2,  1,  1,  2,  2,  1,  1 }; // number of GBTx ROBs on module
const Int_t GbtxPerRob[NofModuleTypes]    = {107,105,105,103,107,105,105,103 }; // number of GBTx ASICs on ROB

const Int_t GbtxPerModule[NofModuleTypes] = { 14,  8,  5,  0,  0, 10,  5,  3 }; // for .geo.info - TODO: merge with above GbtxPerRob
const Int_t RobTypeOnModule[NofModuleTypes] = { 77, 53,  5,  0,  0, 55,  5,  3 }; // for .geo.info - TODO: merge with above GbtxPerRob

// ultimate density - 540 mm
const Int_t FebsPerModule[NofModuleTypes] = {  6,  5,  6,  4, 12,  8,  4,  3 }; // number of FEBs on backside - reduced FEBs (64 ch ASICs)
const Int_t AsicsPerFeb[NofModuleTypes]   = {315,210,105,105,108,108,108,108 }; //  %100 gives number of ASICs on FEB, /100 gives grouping

const Double_t feb_z_offset = 0.1;  // 1 mm - offset in z of FEBs to backpanel
const Double_t asic_offset  = 0.2;  // 2 mm - offset of ASICs to FEBs to avoid overlaps

// ASIC parameters
const Double_t asic_thickness = 0.25; // asic_thickness; // 2.5 mm
const Double_t asic_width     = 3.0;  // 2.0;  1.0;   // 1 cm
//const Double_t asic_distance  = 0.4;  // 0.40; // a factor of width for ASIC pairs
Double_t asic_distance; //  = 0.40; // for 10 ASICs - a factor of width for ASIC pairs

//const Double_t FrameWidth[2]    = { 1.5, 2.0 };   // Width of detector frames in cm
const Double_t FrameWidth[2]    = { 1.5, 1.5 };   // Width of detector frames in cm
// mini - production
const Double_t DetectorSizeX[2] = { 57., 95.};   // => 54 x 54 cm2 & 91 x 91 cm2 active area
const Double_t DetectorSizeY[2] = { 57., 95.};   // quadratic modules
//// default
//const Double_t DetectorSizeX[2] = { 60., 100.};   // => 57 x 57 cm2 & 96 x 96 cm2 active area
//const Double_t DetectorSizeY[2] = { 60., 100.};   // quadratic modules

// Parameters tor the lattice grid reinforcing the entrance window
//const Double_t lattice_o_width[2] = { 1.5, 2.0 };   // Width of outer lattice frame in cm
const Double_t lattice_o_width[2] = { 1.5, 1.5 };   // Width of outer lattice frame in cm
const Double_t lattice_i_width[2] = { 0.2, 0.2 };   // { 0.4, 0.4 };   // Width of inner lattice frame in cm
// Thickness (in z) of lattice frames in cm - see below

// statistics
Int_t ModuleStats[MaxLayers][NofModuleTypes] = { 0 };

// z - geometry of TRD modules
const Double_t radiator_thickness     =  30.0;    // 30 cm thickness of radiator + shift pad plane to integer multiple of 1 mm
const Double_t radiator_position      =  - LayerThickness/2. + radiator_thickness/2.;

//const Double_t lattice_thickness      =   1.0;  // 1.0;  // 10 mm thick lattice frames
const Double_t lattice_thickness      =   1.0 - 0.0025; // 0.9975;  // 1.0;  // 10 mm thick lattice frames
const Double_t lattice_position       =  radiator_position + radiator_thickness/2. + lattice_thickness/2.;

const Double_t kapton_thickness       =   0.0025; //  25 micron thickness of kapton
const Double_t kapton_position        =  lattice_position + lattice_thickness/2. + kapton_thickness/2.;

const Double_t gas_thickness          =   1.2;    //  12 mm thickness of gas
const Double_t gas_position           =  kapton_position + kapton_thickness/2. + gas_thickness/2.;

// frame thickness
const Double_t frame_thickness        =  gas_thickness;   // frame covers gas volume: from kapton foil to pad plane
const Double_t frame_position         =  - LayerThickness /2. + radiator_thickness + lattice_thickness + kapton_thickness + frame_thickness/2.;

// pad plane
const Double_t padcopper_thickness    =   0.0025; //  25 micron thickness of copper pads
const Double_t padcopper_position     =  gas_position + gas_thickness/2. + padcopper_thickness/2.;

const Double_t padplane_thickness     =   0.0360; // 360 micron thickness of padplane
const Double_t padplane_position      =  padcopper_position + padcopper_thickness/2. + padplane_thickness/2.;

// backpanel components
const Double_t carbon_thickness       =   0.0190 * 2; // use 2 layers!!   // 190 micron thickness for 1 layer of carbon fibers
const Double_t honeycomb_thickness    =   2.3 - kapton_thickness - padcopper_thickness - padplane_thickness - carbon_thickness;    //  ~ 2.3 mm thickness of honeycomb
const Double_t honeycomb_position     =  padplane_position + padplane_thickness/2. + honeycomb_thickness/2.;
const Double_t carbon_position        =  honeycomb_position + honeycomb_thickness/2. + carbon_thickness/2.;

// readout boards
const  Double_t febvol_thickness      =  10.0;    // 10 cm length of FEBs
const  Double_t febvol_position       =  carbon_position + carbon_thickness/2. + febvol_thickness/2.;
const  Double_t feb_thickness         =   0.25;  // light //  2.5 mm thickness of FEBs



// Names of the different used materials which are used to build the modules
// The materials are defined in the global media.geo file 
const TString KeepingVolumeMedium     = "air";
const TString RadiatorVolumeMedium    = "TRDpefoam20";
const TString LatticeVolumeMedium     = "TRDG10";
const TString KaptonVolumeMedium      = "TRDkapton";
const TString GasVolumeMedium         = "TRDgas";
const TString PadCopperVolumeMedium   = "TRDcopper";
const TString PadPcbVolumeMedium      = "TRDG10";    // todo - put correct FEB material here
const TString HoneycombVolumeMedium   = "TRDaramide";
const TString CarbonVolumeMedium      = "TRDcarbon";
const TString FebVolumeMedium         = "TRDG10";    // todo - put correct FEB material here
const TString AsicVolumeMedium        = "air";       // todo - put correct ASIC material here
const TString TextVolumeMedium        = "air";       // leave as air
const TString FrameVolumeMedium       = "TRDG10";
const TString AluminiumVolumeMedium   = "aluminium";
//const TString MylarVolumeMedium       = "mylar";
//const TString RadiatorVolumeMedium    = "polypropylene";
//const TString ElectronicsVolumeMedium = "goldcoatedcopper";


// some global variables
TGeoManager* gGeoMan = NULL;  // Pointer to TGeoManager instance
TGeoVolume*  gModules[NofModuleTypes]; // Global storage for module types

// Forward declarations
void create_materials_from_media_file();
void create_trd_module_type(Int_t moduleType);
void create_detector_layers(Int_t layer);
void dump_info_file();
void dump_digi_file();


void create_MUCH_geometry_v16a() {

  // Load needed material definition from media.geo file
  create_materials_from_media_file();

  // Position the layers in z
  for (Int_t iLayer = 1; iLayer < MaxLayers; iLayer++)
    LayerPosition[iLayer] = LayerPosition[iLayer-1] + LayerThickness + LayerOffset[iLayer];  // add offset for extra gaps

  // Get the GeoManager for later usage
  gGeoMan = (TGeoManager*) gROOT->FindObject("FAIRGeom");
  gGeoMan->SetVisLevel(10);  
  
  // Create the top volume 
  TGeoBBox* topbox= new TGeoBBox("", 1000., 1000., 2000.);
  TGeoVolume* top = new TGeoVolume("top", topbox, gGeoMan->GetMedium("air"));
  gGeoMan->SetTopVolume(top);

  TGeoVolume* trd = new TGeoVolumeAssembly(geoVersion);
  top->AddNode(trd, 1);

  for (Int_t iModule = 0; iModule < NofModuleTypes; iModule++) {
     Int_t moduleType = iModule + 1;
     gModules[iModule] = create_trd_module_type(moduleType);
  }

  Int_t nLayer = 0;  // active layer counter
  for (Int_t iLayer = 0; iLayer < MaxLayers; iLayer++) {
    //    if ((iLayer != 0) && (iLayer != 3))  continue;  // first layer only - comment later on
    //    if (iLayer != 0) continue;  // first layer only - comment later on
    if (ShowLayer[iLayer])
    {
      PlaneId[iLayer]=++nLayer;
      create_detector_layers(iLayer);
      //      printf("calling layer %2d\n",iLayer);
    }
  }

  // TODO: remove or comment out
  // test PlaneId
  printf("generated TRD layers: ");
  for (Int_t iLayer = 0; iLayer < MaxLayers; iLayer++)
    if (ShowLayer[iLayer])
      printf(" %2d",PlaneId[iLayer]);
  printf("\n");

  gGeoMan->CloseGeometry();
//  gGeoMan->CheckOverlaps(0.001);
//  gGeoMan->PrintOverlaps();
  gGeoMan->Test();

  //  top->Export(FileNameSim);   // an alternative way of writing the top volume

  TFile* outfile = new TFile(FileNameSim,"RECREATE");
  top->Write();      // use this as input to simulations (run_sim.C)
  outfile->Close();
  TFile* outfile = new TFile(FileNameGeo,"RECREATE");
  gGeoMan->Write();  // use this is you want GeoManager format in the output
  outfile->Close();

  dump_info_file();
  dump_digi_file();

  top->Draw("ogl");

  //top->Raytrace();

//  cout << "Press Return to exit" << endl;
//  cin.get();
//  exit();
}


//==============================================================
void dump_digi_file()
{
  TDatime  datetime;   // used to get timestamp

  const Double_t ActiveAreaX[2] = { DetectorSizeX[0] - 2 * FrameWidth[0],
                                    DetectorSizeX[1] - 2 * FrameWidth[1] };
  const Int_t NofSectors = 3;
  const Int_t NofPadsInRow[2]  = { 80, 128 };  // numer of pads in rows
  Double_t nrow = 0;   // number of rows in module

  const Double_t PadHeightInSector[NofModuleTypes][NofSectors] =  // pad height
        { {  1.50,  1.50,  1.50 },   // module type 1 -  1.01 mm2
          {  2.75,  2.50,  2.75 },   // module type 2 -  1.86 mm2
          {  4.50,  4.50,  4.50 },   // module type 3 -  3.04 mm2
          {  6.75,  6.75,  6.75 },   // module type 4 -  4.56 mm2

          {  3.75,  4.00,  3.75 },   // module type 5 -  2.84 mm2
          {  5.75,  5.75,  5.75 },   // module type 6 -  4.13 mm2
          { 11.50, 11.50, 11.50 },   // module type 7 -  8.26 mm2
          { 15.25, 15.50, 15.25 } }; // module type 8 - 11.14 mm2
//          {  7.50,  7.75,  7.50 },   // module type 6 -  5.51 mm2
//          {  5.50,  5.75,  5.50 },   // module type 6 -  4.09 mm2
//          { 11.25, 11.50, 11.25 },   // module type 7 -  8.18 mm2

  const Int_t NofRowsInSector[NofModuleTypes][NofSectors] =   // number of rows per sector
        { {  12,  12,  12 },         // module type 1
          {   8,   4,   8 },         // module type 2
          {   1,  10,   1 },         // module type 3
          {   2,   4,   2 },         // module type 4

          {   8,   8,   8 },         // module type 5
          {   4,   8,   4 },         // module type 6
          {   2,   4,   2 },         // module type 7
          {   2,   2,   2 } };       // module type 8
//          {  10,   4,  10 },         // module type 5
//          {   4,   4,   4 },         // module type 6
//          {   2,  12,   2 },         // module type 6
//          {   2,   4,   2 },         // module type 7
//          {   2,   2,   2 } };       // module type 8

  Double_t HeightOfSector[NofModuleTypes][NofSectors];
  Double_t PadWidth[NofModuleTypes];

  // calculate pad width
  for (Int_t im = 0; im < NofModuleTypes; im++)
    PadWidth[im] = ActiveAreaX[ModuleType[im]] / NofPadsInRow[ModuleType[im]];

  // calculate height of sectors
  for (Int_t im = 0; im < NofModuleTypes; im++)
    for (Int_t is = 0; is < NofSectors; is++)
      HeightOfSector[im][is] = NofRowsInSector[im][is] * PadHeightInSector[im][is];

  // check, if the entire module size is covered by pads
  for (Int_t im = 0; im < NofModuleTypes; im++)
    if (ActiveAreaX[ModuleType[im]] - (HeightOfSector[im][0] + HeightOfSector[im][1] + HeightOfSector[im][2]) != 0)
    {
      printf("WARNING: sector size does not add up to module size for module type %d\n", im+1);
      printf("%.2f = %.2f + %.2f + %.2f\n", ActiveAreaX[ModuleType[im]], HeightOfSector[im][0], HeightOfSector[im][1], HeightOfSector[im][2]);
      exit();
    }

}


void dump_info_file()
{
  TDatime  datetime;   // used to get timestamp

  Double_t z_first_layer = 2000;   // z position of first layer (front)
  Double_t z_last_layer  = 0;      // z position of last  layer (front)

  Double_t xangle;   // horizontal angle
  Double_t yangle;   // vertical   angle

  Double_t total_surface = 0;   // total surface
  Double_t total_actarea = 0;   // total active area

  Int_t    channels_per_module[NofModuleTypes+1] = { 0 };   // number of channels per module
  Int_t    channels_per_feb[NofModuleTypes+1]    = { 0 };   // number of channels per feb
  Int_t    asics_per_module[NofModuleTypes+1]    = { 0 };   // number of asics per module

  Int_t    total_modules[NofModuleTypes+1]       = { 0 };   // total number of modules
  Int_t    total_febs[NofModuleTypes+1]          = { 0 };   // total number of febs
  Int_t    total_asics[NofModuleTypes+1]         = { 0 };   // total number of asics
  Int_t    total_gbtx[NofModuleTypes+1]          = { 0 };   // total number of gbtx
  Int_t    total_rob3[NofModuleTypes+1]          = { 0 };   // total number of gbtx rob3
  Int_t    total_rob5[NofModuleTypes+1]          = { 0 };   // total number of gbtx rob5
  Int_t    total_rob7[NofModuleTypes+1]          = { 0 };   // total number of gbtx rob7
  Int_t    total_channels[NofModuleTypes+1]      = { 0 };   // total number of channels

  Int_t    total_channels_u = 0;  // total number of ultimate channels
  Int_t    total_channels_s = 0;  // total number of super    channels
  Int_t    total_channels_r = 0;  // total number of regular  channels

  printf("writing summary information file: %s\n", FileNameInfo.Data());

  FILE *ifile;
  ifile = fopen(FileNameInfo.Data(),"w");

  if (ifile == NULL)
    {
      printf("error opening %s\n", FileNameInfo.Data());
      exit(1);
    }

  fprintf(ifile,"#\n##   %s information file\n#\n\n", geoVersion.Data());

  fprintf(ifile,"# created %d\n\n", datetime.GetDate());

  // determine first and last TRD layer
  for (Int_t iLayer = 0; iLayer < MaxLayers; iLayer++)
  {
    if (ShowLayer[iLayer])
    {
      if (z_first_layer > LayerPosition[iLayer])
        z_first_layer = LayerPosition[iLayer];
      if (z_last_layer < LayerPosition[iLayer])
        z_last_layer  = LayerPosition[iLayer];
    }
  }

  fprintf(ifile,"# envelope\n");
  // Show extension of TRD
  fprintf(ifile,"%4d cm   start of TRD (z)\n", z_first_layer);
  fprintf(ifile,"%4d cm   end   of TRD (z)\n", z_last_layer + LayerThickness);
  fprintf(ifile,"\n");

  // Layer thickness
  fprintf(ifile,"# thickness\n");
  fprintf(ifile,"%4d cm   per single layer (z)\n", LayerThickness);
  fprintf(ifile,"\n");

  // Show extra gaps
  fprintf(ifile,"# extra gaps\n ");
  for (Int_t iLayer = 0; iLayer < MaxLayers; iLayer++)
    if (ShowLayer[iLayer])
      fprintf(ifile,"%3d ", LayerOffset[iLayer]);
  fprintf(ifile,"   extra gaps in z (cm)\n");
  fprintf(ifile,"\n");

  fclose(ifile);
}


void create_materials_from_media_file()
{
  // Use the FairRoot geometry interface to load the media which are already defined
  FairGeoLoader* geoLoad = new FairGeoLoader("TGeo", "FairGeoLoader");
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  TString geoPath = gSystem->Getenv("VMCWORKDIR");
  TString geoFile = geoPath + "/geometry/media.geo";
  geoFace->setMediaFile(geoFile);
  geoFace->readMedia();

  // Read the required media and create them in the GeoManager
  FairGeoMedia* geoMedia = geoFace->getMedia();
  FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

  FairGeoMedium* air              = geoMedia->getMedium(KeepingVolumeMedium);
  geoBuild->createMedium(air);

  FairGeoMedium* pefoam20         = geoMedia->getMedium(RadiatorVolumeMedium);
  FairGeoMedium* G10              = geoMedia->getMedium(LatticeVolumeMedium);
  FairGeoMedium* kapton           = geoMedia->getMedium(KaptonVolumeMedium);
  FairGeoMedium* trdGas           = geoMedia->getMedium(GasVolumeMedium);
  FairGeoMedium* copper           = geoMedia->getMedium(PadCopperVolumeMedium);
  FairGeoMedium* carbon           = geoMedia->getMedium(CarbonVolumeMedium);
  FairGeoMedium* honeycomb        = geoMedia->getMedium(HoneycombVolumeMedium);
  FairGeoMedium* aluminium        = geoMedia->getMedium(AluminiumVolumeMedium);

  geoBuild->createMedium(pefoam20);
  geoBuild->createMedium(trdGas);
  geoBuild->createMedium(honeycomb);
  geoBuild->createMedium(carbon);
  geoBuild->createMedium(G10);
  geoBuild->createMedium(copper);
  geoBuild->createMedium(kapton);
  geoBuild->createMedium(aluminium);

}


TGeoVolume* create_trd_module_type(Int_t moduleType)
{
  Int_t type = ModuleType[moduleType - 1];
  Double_t sizeX = DetectorSizeX[type];
  Double_t sizeY = DetectorSizeY[type];
  Double_t frameWidth = FrameWidth[type];
  Double_t activeAreaX = sizeX - 2 * frameWidth;
  Double_t activeAreaY = sizeY - 2 * frameWidth;

  TGeoMedium* keepVolMed        = gGeoMan->GetMedium(KeepingVolumeMedium);
  TGeoMedium* radVolMed         = gGeoMan->GetMedium(RadiatorVolumeMedium);
  TGeoMedium* latticeVolMed     = gGeoMan->GetMedium(LatticeVolumeMedium);
  TGeoMedium* kaptonVolMed      = gGeoMan->GetMedium(KaptonVolumeMedium);
  TGeoMedium* gasVolMed         = gGeoMan->GetMedium(GasVolumeMedium);
  TGeoMedium* padcopperVolMed   = gGeoMan->GetMedium(PadCopperVolumeMedium);
  TGeoMedium* padpcbVolMed      = gGeoMan->GetMedium(PadPcbVolumeMedium);
  TGeoMedium* honeycombVolMed   = gGeoMan->GetMedium(HoneycombVolumeMedium);
  TGeoMedium* carbonVolMed      = gGeoMan->GetMedium(CarbonVolumeMedium);
  TGeoMedium* frameVolMed       = gGeoMan->GetMedium(FrameVolumeMedium);
  TGeoMedium* febVolMed         = gGeoMan->GetMedium(FebVolumeMedium);
  TGeoMedium* asicVolMed        = gGeoMan->GetMedium(AsicVolumeMedium);

  TString name = Form("module%d", moduleType);
  TGeoVolume* module = new TGeoVolumeAssembly(name);


  if(IncludeRadiator)
   {
     // Radiator
     //   TGeoBBox* trd_radiator = new TGeoBBox("", activeAreaX /2., activeAreaY /2., radiator_thickness /2.);
     TGeoBBox* trd_radiator = new TGeoBBox("", sizeX /2., sizeY /2., radiator_thickness /2.);
     TGeoVolume* trdmod1_radvol = new TGeoVolume("radiator", trd_radiator, radVolMed);
     //     TGeoVolume* trdmod1_radvol = new TGeoVolume(Form("module%d_radiator", moduleType), trd_radiator, radVolMed);
     //     TGeoVolume* trdmod1_radvol = new TGeoVolume(Form("trd1mod%dradiator", moduleType), trd_radiator, radVolMed);
     trdmod1_radvol->SetLineColor(kBlue);
     trdmod1_radvol->SetTransparency(70);  // (60);  // (70);  // set transparency for the TRD radiator
     TGeoTranslation* trd_radiator_trans = new TGeoTranslation("", 0., 0., radiator_position);
     module->AddNode(trdmod1_radvol, 1, trd_radiator_trans);
   }

   if(IncludeKaptonFoil)
   {
     // Kapton Foil
     TGeoBBox* trd_kapton = new TGeoBBox("", sizeX /2., sizeY /2., kapton_thickness /2.);
     TGeoVolume* trdmod1_kaptonvol = new TGeoVolume("kaptonfoil", trd_kapton, kaptonVolMed);
     //   TGeoVolume* trdmod1_kaptonvol = new TGeoVolume(Form("module%d_kaptonfoil", moduleType), trd_kapton, kaptonVolMed);
     //   TGeoVolume* trdmod1_kaptonvol = new TGeoVolume(Form("trd1mod%dkapton", moduleType), trd_kapton, kaptonVolMed);
     trdmod1_kaptonvol->SetLineColor(kGreen);
     TGeoTranslation* trd_kapton_trans = new TGeoTranslation("", 0., 0., kapton_position);
     module->AddNode(trdmod1_kaptonvol, 1, trd_kapton_trans);
   }

   // start of Frame in z
   // Gas
   TGeoBBox* trd_gas = new TGeoBBox("", activeAreaX /2., activeAreaY /2., gas_thickness /2.);
   TGeoVolume* trdmod1_gasvol = new TGeoVolume("gas", trd_gas, gasVolMed);
   //   TGeoVolume* trdmod1_gasvol = new TGeoVolume(Form("module%d_gas", moduleType), trd_gas, gasVolMed);
   //   TGeoVolume* trdmod1_gasvol = new TGeoVolume(Form("trd1mod%dgas", moduleType), trd_gas, gasVolMed);
   //   trdmod1_gasvol->SetLineColor(kBlue);
   trdmod1_gasvol->SetLineColor(kGreen); // to avoid blue overlaps in the screenshots
   trdmod1_gasvol->SetTransparency(40);  // set transparency for the TRD gas
   TGeoTranslation* trd_gas_trans = new TGeoTranslation("", 0., 0., gas_position);
   module->AddNode(trdmod1_gasvol, 1, trd_gas_trans);
   // end of Frame in z

   if(IncludeGasFrame)
   {
     // frame1
     TGeoBBox* trd_frame1 = new TGeoBBox("", sizeX /2., frameWidth /2., frame_thickness/2.);
     TGeoVolume* trdmod1_frame1vol = new TGeoVolume("frame1", trd_frame1, frameVolMed);
     //   TGeoVolume* trdmod1_frame1vol = new TGeoVolume(Form("module%d_frame1", moduleType), trd_frame1, frameVolMed);
     //   TGeoVolume* trdmod1_frame1vol = new TGeoVolume(Form("trd1mod%dframe1", moduleType), trd_frame1, frameVolMed);
     trdmod1_frame1vol->SetLineColor(kRed);
     
     // translations 
     TGeoTranslation* trd_frame1_trans = new TGeoTranslation("", 0., activeAreaY /2. + frameWidth /2., frame_position);
     module->AddNode(trdmod1_frame1vol, 1, trd_frame1_trans);
     trd_frame1_trans = new TGeoTranslation("", 0., -(activeAreaY /2. + frameWidth /2.), frame_position);
     module->AddNode(trdmod1_frame1vol, 2, trd_frame1_trans);
     
     
     // frame2
     TGeoBBox* trd_frame2 = new TGeoBBox("", frameWidth /2., activeAreaY /2., frame_thickness /2.);
     TGeoVolume* trdmod1_frame2vol = new TGeoVolume("frame2", trd_frame2, frameVolMed);
     //   TGeoVolume* trdmod1_frame2vol = new TGeoVolume(Form("module%d_frame2", moduleType), trd_frame2, frameVolMed);
     //   TGeoVolume* trdmod1_frame2vol = new TGeoVolume(Form("trd1mod%dframe2", moduleType), trd_frame2, frameVolMed);
     trdmod1_frame2vol->SetLineColor(kRed);
     
     // translations 
     TGeoTranslation* trd_frame2_trans = new TGeoTranslation("", activeAreaX /2. + frameWidth /2., 0., frame_position);
     module->AddNode(trdmod1_frame2vol, 1, trd_frame2_trans);
     trd_frame2_trans = new TGeoTranslation("", -(activeAreaX /2. + frameWidth /2.), 0., frame_position);
     module->AddNode(trdmod1_frame2vol, 2, trd_frame2_trans);
   }

   if(IncludePadplane)
   {
     // Pad Copper
     TGeoBBox *trd_padcopper = new TGeoBBox("", sizeX /2., sizeY /2., padcopper_thickness /2.);
     TGeoVolume* trdmod1_padcoppervol = new TGeoVolume("padcopper", trd_padcopper, padcopperVolMed);
     //   TGeoVolume* trdmod1_padcoppervol = new TGeoVolume(Form("module%d_padcopper", moduleType), trd_padcopper, padcopperVolMed);
     //   TGeoVolume* trdmod1_padcoppervol = new TGeoVolume(Form("trd1mod%dpadcopper", moduleType), trd_padcopper, padcopperVolMed);
     trdmod1_padcoppervol->SetLineColor(kOrange);
     TGeoTranslation *trd_padcopper_trans = new TGeoTranslation("", 0., 0., padcopper_position);
     module->AddNode(trdmod1_padcoppervol, 1, trd_padcopper_trans);
  
     // Pad Plane
     TGeoBBox* trd_padpcb = new TGeoBBox("", sizeX /2., sizeY /2., padplane_thickness /2.);
     TGeoVolume* trdmod1_padpcbvol = new TGeoVolume("padplane", trd_padpcb, padpcbVolMed);
     //   TGeoVolume* trdmod1_padpcbvol = new TGeoVolume(Form("module%d_padplane", moduleType), trd_padpcb, padpcbVolMed);
     //   TGeoVolume* trdmod1_padpcbvol = new TGeoVolume(Form("trd1mod%dpadplane", moduleType), trd_padpcb, padpcbVolMed);
     trdmod1_padpcbvol->SetLineColor(kBlue);
     TGeoTranslation *trd_padpcb_trans = new TGeoTranslation("", 0., 0., padplane_position);
     module->AddNode(trdmod1_padpcbvol, 1, trd_padpcb_trans);
   }

   if(IncludeBackpanel)
   { 
     // Honeycomb
     TGeoBBox* trd_honeycomb = new TGeoBBox("", sizeX /2., sizeY /2., honeycomb_thickness /2.);
     TGeoVolume* trdmod1_honeycombvol = new TGeoVolume("honeycomb", trd_honeycomb, honeycombVolMed);
     //   TGeoVolume* trdmod1_honeycombvol = new TGeoVolume(Form("module%d_honeycomb", moduleType), trd_honeycomb, honeycombVolMed);
     //   TGeoVolume* trdmod1_honeycombvol = new TGeoVolume(Form("trd1mod%dhoneycomb", moduleType), trd_honeycomb, honeycombVolMed);
     trdmod1_honeycombvol->SetLineColor(kOrange);
     TGeoTranslation* trd_honeycomb_trans = new TGeoTranslation("", 0., 0., honeycomb_position);
     module->AddNode(trdmod1_honeycombvol, 1, trd_honeycomb_trans);
  
     // Carbon fiber layers
     TGeoBBox* trd_carbon = new TGeoBBox("", sizeX /2., sizeY /2., carbon_thickness /2.);
     TGeoVolume* trdmod1_carbonvol = new TGeoVolume("carbonsheet", trd_carbon, carbonVolMed);
     //   TGeoVolume* trdmod1_carbonvol = new TGeoVolume(Form("module%d_carbonsheet", moduleType), trd_carbon, carbonVolMed);
     //   TGeoVolume* trdmod1_carbonvol = new TGeoVolume(Form("trd1mod%dcarbon", moduleType), trd_carbon, carbonVolMed);
     trdmod1_carbonvol->SetLineColor(kGreen);
     TGeoTranslation* trd_carbon_trans = new TGeoTranslation("", 0., 0., carbon_position);
     module->AddNode(trdmod1_carbonvol, 1, trd_carbon_trans);
   }

   return module;
}


Int_t copy_nr(Int_t stationNr, Int_t copyNr, Int_t isRotated, Int_t planeNr, Int_t modinplaneNr)
{
  if (modinplaneNr > 128)
    printf("Warning: too many modules in this layer %02d (max 128 according to CbmTrdAddress)\n", planeNr);

  return (stationNr      * 100000000    // 1 digit
        + copyNr         *   1000000    // 2 digit
        + isRotated      *    100000    // 1 digit
        + planeNr        *      1000    // 2 digit
        + modinplaneNr   *         1 ); // 3 digit
}


void create_detector_layers(Int_t layerId)
{
  Int_t module_id = 0;
  Int_t layerType = LayerType[layerId] / 10;  // this is also a station number
  Int_t isRotated = LayerType[layerId] % 10;  // is 1 for layers 2,4, ...
  TGeoRotation* module_rotation = new TGeoRotation();

  Int_t stationNr = layerType;

// rotation is now done in the for loop for each module individually
//  if ( isRotated == 1 ) {
//    module_rotation = new TGeoRotation();
//    module_rotation->RotateZ(90.);
//  } else {
//    module_rotation = new TGeoRotation();
//    module_rotation->RotateZ( 0.);
//  }
  
  Int_t innerarray_size1 = LayerArraySize[layerType - 1][0];
  Int_t innerarray_size2 = LayerArraySize[layerType - 1][1];
  Int_t* innerLayer;
  
  Int_t outerarray_size1 = LayerArraySize[layerType - 1][2];
  Int_t outerarray_size2 = LayerArraySize[layerType - 1][3];
  Int_t* outerLayer;
  
  if ( 1 == layerType ) {
    innerLayer = layer1i;      
    outerLayer = layer1o; 
  } else {
    std::cout << "Type of layer not known" << std::endl;
  } 

// add MUCH stations
  TString layername = Form("muchstation%02d", PlaneId[layerId]);
  TGeoVolume* layer = new TGeoVolumeAssembly(layername);

  // compute layer copy number
  Int_t i = LayerType[layerId] / 10   * 10000   // 1 digit  // fStation
          + LayerType[layerId] % 10   *  1000   // 1 digit  // isRotated
          + LayerNrInStation[layerId] *   100   // 1 digit  // fLayer
          + PlaneId[layerId];                   // 2 digits // fPlane   // layer type as leading digit in copy number of layer
  gGeoMan->GetVolume(geoVersion)->AddNode(layer, i);

//  Int_t i = 100 + PlaneId[layerId];
//  gGeoMan->GetVolume(geoVersion)->AddNode(layer, 1);
//  cout << layername << endl;

  Int_t modId = 0;  // module id, only within this layer
  
  Int_t copyNrIn[4] = { 0, 0, 0, 0 }; // copy number for each module type
  for ( Int_t type = 1; type <= 4; type++) {
    for ( Int_t j = (innerarray_size1-1); j >= 0; j--)  { // start from the bottom 
      for ( Int_t i = 0; i < innerarray_size2; i++) {
        module_id = *(innerLayer + (j * innerarray_size2 + i));
        if ( module_id /100 == type) {
          Int_t y = -(j-2);
          Int_t x =   i-2;

          // displacement
          Double_t dx = 0;
          Double_t dy = 0;
          Double_t dz = 0;

          Double_t xPos = DetectorSizeX[0] * x + dx;
          Double_t yPos = DetectorSizeY[0] * y + dy;
          copyNrIn[type - 1]++;
          modId++;

          // statistics per layer and module type
          ModuleStats[layerId][type - 1]++;

//          Int_t copy = copy_nr_modid(stationNr, layerNrInStation, copyNrIn[type - 1], PlaneId[layerId], modId);  // with modID
//          Int_t copy = copy_nr(stationNr, copyNrIn[type - 1], isRotated, PlaneId[layerId], modId);

          // take care of FEB orientation - away from beam
          Int_t copy = 0;
          module_rotation = new TGeoRotation();   // need to renew rotation to start from 0 degree angle
          if ( isRotated == 0 )  // layer 1,3 ...
	  {
            copy = copy_nr(stationNr, copyNrIn[type - 1], module_id /10 %10, PlaneId[layerId], modId);
   	    module_rotation->RotateZ( (module_id /10 %10) * 90. );  // rotate module by   0 or 180 degrees, see layer[1-3][i,o] - vertical pads
	  }
          else  // layer 2,4 ...
	  {
            copy = copy_nr(stationNr, copyNrIn[type - 1], module_id %10    , PlaneId[layerId], modId);
   	    module_rotation->RotateZ( (module_id %10) * 90. );      // rotate module by  90 or 270 degrees, see layer[1-3][i,o] - horizontal pads
	  }

          TGeoCombiTrans* module_placement = new TGeoCombiTrans(xPos, yPos, LayerPosition[layerId] + LayerThickness/2 + dz, module_rotation);  // shift by half layer thickness
//          gGeoMan->GetVolume(geoVersion)->AddNode(gModules[type - 1], copy, module_placement);
// add module to layer
          gGeoMan->GetVolume(layername)->AddNode(gModules[type - 1], copy, module_placement);
//
        }
      }
    }
  }
  
  Int_t copyNrOut[4] = { 0, 0, 0, 0 }; // copy number for each module type
  for ( Int_t type = 5; type <= 8; type++) {
    for ( Int_t j = (outerarray_size1-1); j >= 0; j--)  { // start from the bottom 
      for ( Int_t i = 0; i < outerarray_size2; i++) {
	module_id = *(outerLayer + (j * outerarray_size2 + i));
        if ( module_id /100 == type) {
          Int_t y = -(j-4);
          Int_t x =   i-5;

          // displacement
          Double_t dx = 0;
          Double_t dy = 0;
          Double_t dz = 0;

          Double_t xPos = DetectorSizeX[1] * x + dx;
          Double_t yPos = DetectorSizeY[1] * y + dy;
          copyNrOut[type - 5]++;
          modId++;

          // statistics per layer and module type
          ModuleStats[layerId][type - 1]++;

//          Int_t copy = copy_nr_modid(stationNr, layerNrInStation, copyNrOut[type - 5],  PlaneId[layerId], modId);  // with modID
//          Int_t copy = copy_nr(stationNr, copyNrOut[type - 5], isRotated, PlaneId[layerId], modId);

          // take care of FEB orientation - away from beam
          Int_t copy = 0;
          module_rotation = new TGeoRotation();   // need to renew rotation to start from 0 degree angle
          if ( isRotated == 0 )  // layer 1,3 ...
	  {
            copy = copy_nr(stationNr, copyNrOut[type - 5], module_id /10 %10, PlaneId[layerId], modId);
            module_rotation->RotateZ( (module_id /10 %10) * 90. );  // rotate module by   0 or 180 degrees, see layer[1-3][i,o] - vertical pads
	  }
          else  // layer 2,4 ...
	  {
            copy = copy_nr(stationNr, copyNrOut[type - 5], module_id %10    , PlaneId[layerId], modId);
            module_rotation->RotateZ( (module_id %10) * 90. );      // rotate module by  90 or 270 degrees, see layer[1-3][i,o] - horizontal pads
	  }
    
          TGeoCombiTrans* module_placement = new TGeoCombiTrans(xPos, yPos, LayerPosition[layerId] + LayerThickness/2 + dz, module_rotation);  // shift by half layer thickness
// add module to layer
          gGeoMan->GetVolume(layername)->AddNode(gModules[type - 1], copy, module_placement);
//
        }
      }
    }
  }  
}


