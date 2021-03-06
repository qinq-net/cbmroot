// Macro creating a ToF geometry for STAR wheel 
//
// Usage:
// in root all sizes are given in cm

#include "TSystem.h"
#include "TGeoManager.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoPgon.h"
#include "TGeoMatrix.h"
#include "TGeoCompositeShape.h"
#include "TFile.h"
#include "TString.h"
#include "TList.h"
#include "TROOT.h"
#include "TMath.h"

#include <iostream>

// Name of geometry version and output file
const TString geoVersion = "tof_v14s";
const TString FileName = geoVersion + ".root";

// Names of the different used materials which are used to build the modules
// The materials are defined in the global media.geo file 
const TString KeepingVolumeMedium     = "air";
const TString BoxVolumeMedium         = "aluminium";
const TString NoActivGasMedium        = "RPCgas_noact";
const TString ActivGasMedium          = "RPCgas";
const TString GlasMedium              = "RPCglass";
const TString ElectronicsMedium       = "carbon";

// Distance to the center of the TOF wall [cm];
const Float_t Wall_Z_Position = 480;  // -240, positiv for testing in CBMROOT
// Form automatically phi rotated modules
const Double_t RING_DelZ   = -13.;   // +13 for STAR
const Double_t RING_DelPhi = 30.;
const Double_t Deg2Rad = 0.017453293;
const Double_t RING_DPhi = 10.;
const Double_t RING_DR   = 5.;

// Counters: 
// 0 conductive glass 
// 1 thin glass

const Int_t NumberOfDifferentCounterTypes = 2;
const Float_t Glass_X[NumberOfDifferentCounterTypes] = {32., 32.};
const Float_t Glass_Y[NumberOfDifferentCounterTypes] = {27., 27.};
const Float_t Glass_Z[NumberOfDifferentCounterTypes] = {0.07, 0.028};

const Float_t GasGap_X[NumberOfDifferentCounterTypes] = {32., 32.};
const Float_t GasGap_Y[NumberOfDifferentCounterTypes] = {27., 27.};
const Float_t GasGap_Z[NumberOfDifferentCounterTypes] = {0.022,0.014};

const Int_t NumberOfGaps[NumberOfDifferentCounterTypes] = {8,12};
const Int_t NumberOfReadoutStrips[NumberOfDifferentCounterTypes] = {32,32};

const Float_t Electronics_X[NumberOfDifferentCounterTypes] = {34.0, 34.};
const Float_t Electronics_Y[NumberOfDifferentCounterTypes] = {10.0, 10.};
const Float_t Electronics_Z[NumberOfDifferentCounterTypes] = { 0.3, 0.3};

const Int_t NofModuleTypes = 1;
// Aluminum box for all supermodule types
// 0 default 

const Float_t Module_Size_X[NofModuleTypes] = {100.};
const Float_t Module_Size_Y[NofModuleTypes] = { 49.};
const Float_t Module_Size_Z[NofModuleTypes] = { 11.};
const Float_t Module_Thick_Alu_X_left =  0.1;
const Float_t Module_Thick_Alu_X_right = 1.0;
const Float_t Module_Thick_Alu_Y = 0.1;
const Float_t Module_Thick_Alu_Z = 0.1;

//Type of Counter for module 
const Int_t CounterTypeInModule[NofModuleTypes] = {1};
const Int_t NCounterInModule[NofModuleTypes]    = {3};

// Placement of the counter inside the module
const Float_t CounterXStartPosition[NofModuleTypes] = {-31.};
const Float_t CounterXDistance[NofModuleTypes]      = {31.0};
const Float_t CounterZDistance[NofModuleTypes]      = {2.5};
//const Float_t CounterRotationAngle[NofModuleTypes]  = {-8.7}; // for STAR
const Float_t CounterRotationAngle[NofModuleTypes]  = {8.7}; // for CBM

// Position for module placement
const Float_t Module_X_Position=170.;
const Float_t Module_Y_Position=0.;
const Float_t Module_Z_Position=Wall_Z_Position;

const Int_t Module_NTypes = 1;
const Float_t Module_Types[Module_NTypes]  = {0.};  // Module Type specifier
const Float_t Module_Number[Module_NTypes] = {36.}; //debugging,

// some global variables
TGeoManager* gGeoMan = NULL;  // Pointer to TGeoManager instance
TGeoVolume* gModules[NofModuleTypes]; // Global storage for module types
TGeoVolume* gCounter[NumberOfDifferentCounterTypes];

// Forward declarations
void create_materials_from_media_file();
TGeoVolume* create_counter(Int_t);
TGeoVolume* create_tof_module(Int_t);

void position_tof_modules(Int_t); // STAR modules 

void Create_TOF_Geometry_Star_1() {
  // Load the necessary FairRoot libraries 
//  gROOT->LoadMacro("$VMCWORKDIR/gconfig/basiclibs.C");
//  basiclibs();
//  gSystem->Load("libGeoBase");
//  gSystem->Load("libParBase");
//  gSystem->Load("libBase");

  // Load needed material definition from media.geo file
  create_materials_from_media_file();

  // Get the GeoManager for later usage
  gGeoMan = (TGeoManager*) gROOT->FindObject("FAIRGeom");
  gGeoMan->SetVisLevel(5);  // 2 = super modules   
  gGeoMan->SetVisOption(0);  

  // Create the top volume 
  /*
  TGeoBBox* topbox= new TGeoBBox("", 1000., 1000., 1000.);
  TGeoVolume* top = new TGeoVolume("top", topbox, gGeoMan->GetMedium("air"));
  gGeoMan->SetTopVolume(top);
  */

  TGeoVolume* top = new TGeoVolumeAssembly("TOP");
  gGeoMan->SetTopVolume(top);
 
  TGeoVolume* tof = new TGeoVolumeAssembly(geoVersion);
  top->AddNode(tof, 1);
  
  for(Int_t counterType = 0; counterType < NumberOfDifferentCounterTypes; counterType++) { 
    gCounter[counterType] = create_counter(counterType);
  }

  for(Int_t moduleType = 0; moduleType < NofModuleTypes; moduleType++) { 
    gModules[moduleType] = create_tof_module(moduleType);
    gModules[moduleType]->SetVisContainers(1); 
  }
  
  position_tof_modules(1);
  
  gGeoMan->CloseGeometry();
  gGeoMan->CheckOverlaps(0.001);
  gGeoMan->PrintOverlaps();
  gGeoMan->Test();

  TFile* outfile = new TFile(FileName,"RECREATE");
  top->Write();
  //gGeoMan->Write();
  outfile->Close();
  top->SetVisContainers(1); 
  gGeoMan->SetVisLevel(5); 
  top->Draw("ogl");
  //top->Draw();
  //gModules[0]->Draw("ogl");
  //  gModules[0]->Draw("");
  gModules[0]->SetVisContainers(1); 
  //  gModules[1]->Draw("");
  gModules[1]->SetVisContainers(1); 
  //gModules[5]->Draw("");
  //  top->Raytrace();

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

  FairGeoMedium* air              = geoMedia->getMedium("air");
  FairGeoMedium* aluminium        = geoMedia->getMedium("aluminium");
  FairGeoMedium* RPCgas           = geoMedia->getMedium("RPCgas");
  FairGeoMedium* RPCgas_noact     = geoMedia->getMedium("RPCgas_noact");
  FairGeoMedium* RPCglass         = geoMedia->getMedium("RPCglass");
  FairGeoMedium* carbon           = geoMedia->getMedium("carbon");

  // include check if all media are found

  geoBuild->createMedium(air);
  geoBuild->createMedium(aluminium);
  geoBuild->createMedium(RPCgas);
  geoBuild->createMedium(RPCgas_noact);
  geoBuild->createMedium(RPCglass);
  geoBuild->createMedium(carbon);
}

TGeoVolume* create_counter(Int_t modType)
{

  //glass
  Float_t gdx=Glass_X[modType]; 
  Float_t gdy=Glass_Y[modType];
  Float_t gdz=Glass_Z[modType];

  //gas gap
  Int_t  nstrips=NumberOfReadoutStrips[modType];
  Int_t  ngaps=NumberOfGaps[modType];


  Float_t ggdx=GasGap_X[modType];  
  Float_t ggdy=GasGap_Y[modType];
  Float_t ggdz=GasGap_Z[modType];
  Float_t gsdx=ggdx/(Float_t)(nstrips);

  // electronics
  //pcb dimensions 
  Float_t dxe=Electronics_X[modType]; 
  Float_t dye=Electronics_Y[modType];
  Float_t dze=Electronics_Z[modType];
  Float_t yele=gdy/2.+dye/2.;
 
  // counter size (calculate from glas, gap and electronics sizes)
  Float_t cdx = TMath::Max(gdx, ggdx);
  cdx = TMath::Max(cdx, dxe)+ 0.2;
  Float_t cdy = TMath::Max(gdy, ggdy) + 2*dye + 0.2;
  Float_t cdz = ngaps * (gdz+ggdz) + gdz + 0.2;

  //calculate thickness and first position in coonter of single stack
  Float_t dzpos=gdz+ggdz;
  Float_t startzposglas=(-cdz+gdz)/2.;
  Float_t startzposgas=-cdz/2.+ gdz + ggdz/2.;


  // needed materials
  TGeoMedium* glassPlateVolMed   = gGeoMan->GetMedium(GlasMedium);
  TGeoMedium* noActiveGasVolMed  = gGeoMan->GetMedium(NoActivGasMedium);
  TGeoMedium* activeGasVolMed    = gGeoMan->GetMedium(ActivGasMedium);
  TGeoMedium* electronicsVolMed  = gGeoMan->GetMedium(ElectronicsMedium);


  // define counter volume
  TGeoBBox* counter_box = new TGeoBBox("", cdx/2., cdy/2., cdz/2.);
  TGeoVolume* counter = 
    new TGeoVolume("counter", counter_box, noActiveGasVolMed);
  counter->SetLineColor(kCyan); // set line color for the counter
  counter->SetTransparency(98); // set transparency for the TOF

  // define single glass plate volume
  TGeoBBox* glass_plate = new TGeoBBox("", gdx/2., gdy/2., gdz/2.);
  TGeoVolume* glass_plate_vol = 
    new TGeoVolume("tof_glass", glass_plate, glassPlateVolMed);
  glass_plate_vol->SetLineColor(kMagenta); // set line color for the glass plate
  glass_plate_vol->SetTransparency(98); // set transparency for the TOF
  // define single gas gap volume
  TGeoBBox* gas_gap = new TGeoBBox("", ggdx/2., ggdy/2., ggdz/2.);
  TGeoVolume* gas_gap_vol = 
    new TGeoVolume("Gap", gas_gap, activeGasVolMed);
  gas_gap_vol->Divide("Cell",1,nstrips,-ggdx/2.,0);
  gas_gap_vol->SetLineColor(kRed); // set line color for the gas gap
  gas_gap_vol->SetTransparency(99); // set transparency for the TOF
 
  // place 8 gas gaps and 9 glas plates in the counter
  for( Int_t igap = 0; igap < ngaps; igap++) {

    Float_t zpos_glas = startzposglas + igap*dzpos;
    Float_t zpos_gas = startzposgas + igap*dzpos;

    //    cout <<"Zpos(Glas): "<< zpos_glas << endl;
    //    cout <<"Zpos(Gas): "<< zpos_gas << endl;

    TGeoTranslation* glass_plate_trans 
      = new TGeoTranslation("", 0., 0., zpos_glas);
    TGeoTranslation* gas_gap_trans 
      = new TGeoTranslation("", 0., 0., zpos_gas);
    counter->AddNode(glass_plate_vol, igap, glass_plate_trans);
    counter->AddNode(gas_gap_vol, igap, gas_gap_trans);

  }

  Float_t zpos_glas = startzposglas + (ngaps+1)*dzpos;
  TGeoTranslation* glass_plate_trans 
    = new TGeoTranslation("", 0., 0., zpos_glas);
  counter->AddNode(glass_plate_vol, ngaps, glass_plate_trans);
  
  // create and place the electronics above and below the glas stack
  TGeoBBox* pcb = new TGeoBBox("", dxe/2., dye/2., dze/2.);
  TGeoVolume* pcb_vol = 
    new TGeoVolume("pcb", pcb, electronicsVolMed);
  pcb_vol->SetLineColor(kGreen); // set line color for electronics
  pcb_vol->SetTransparency(90); // set transparency for the TOF
  for (Int_t l=0; l<2; l++){
    yele *= -1.;
    TGeoTranslation* pcb_trans 
      = new TGeoTranslation("", 0., yele, 0.);
    counter->AddNode(pcb_vol, l, pcb_trans);
  }
 
  return counter;

}


TGeoVolume* create_tof_module(Int_t modType)
{
  Int_t cType = CounterTypeInModule[modType];
  Float_t dx=Module_Size_X[modType];
  Float_t dy=Module_Size_Y[modType];
  Float_t dz=Module_Size_Z[modType];
  Float_t width_aluxl=Module_Thick_Alu_X_left;
  Float_t width_aluxr=Module_Thick_Alu_X_right;
  Float_t width_aluy=Module_Thick_Alu_Y;
  Float_t width_aluz=Module_Thick_Alu_Z;

  Float_t shift_gas_box = (Module_Thick_Alu_X_right - Module_Thick_Alu_X_left)/2;

  Float_t dxpos=CounterXDistance[modType];
  Float_t startxpos=CounterXStartPosition[modType];
  Float_t dzoff=CounterZDistance[modType];
  Float_t rotangle=CounterRotationAngle[modType];

  TGeoMedium* boxVolMed          = gGeoMan->GetMedium(BoxVolumeMedium);
  TGeoMedium* noActiveGasVolMed  = gGeoMan->GetMedium(NoActivGasMedium);

  TString moduleName = Form("module_%d", modType);

  TGeoBBox* module_box = new TGeoBBox("", dx/2., dy/2., dz/2.);
  TGeoVolume* module = 
  new TGeoVolume(moduleName, module_box, boxVolMed);
  module->SetLineColor(kGreen); // set line color for the alu box
  module->SetTransparency(90); // set transparency for the TOF

  TGeoBBox* gas_box = new TGeoBBox("", (dx-(width_aluxl+width_aluxr))/2., (dy-2*width_aluy)/2., (dz-2*width_aluz)/2.);
  TGeoVolume* gas_box_vol = 
    new TGeoVolume("gas_box", gas_box, noActiveGasVolMed);
  gas_box_vol->SetLineColor(kBlue); // set line color for the alu box
  gas_box_vol->SetTransparency(80); // set transparency for the TOF
  TGeoTranslation* gas_box_trans 
    = new TGeoTranslation("", shift_gas_box, 0., 0.);
  module->AddNode(gas_box_vol, 0, gas_box_trans);
  
  for (Int_t j=0; j< NCounterInModule[modType]; j++){ //loop over counters (modules)
    Float_t zpos;

    zpos = 0.;

    TGeoTranslation* counter_trans 
      = new TGeoTranslation("", startxpos+ j*dxpos , 0.0 , zpos);

    TGeoRotation* counter_rot = new TGeoRotation();
    counter_rot->RotateY(rotangle);
    TGeoCombiTrans* counter_combi_trans = new TGeoCombiTrans(*counter_trans, *counter_rot);
    gas_box_vol->AddNode(gCounter[cType], j, counter_combi_trans);
  }

  return module;
}

void position_tof_modules(Int_t modNType)
{
 TGeoTranslation* module_trans=NULL;

 Int_t ii=0; 
 Float_t xPos  = Module_X_Position;
 Float_t yPos=   Module_Y_Position;
 Float_t zPos  = Module_Z_Position;
 Double_t Radius = TMath::Sqrt(xPos*xPos + yPos*yPos);

 for (Int_t j=0; j<modNType; j++){
  Int_t modType= Module_Types[j];
  Int_t modNum = 0;
  for(Int_t i=0; i<Module_Number[j]; i++) { 
    ii++; 
    if(i==12 || i==24) Radius += RING_DR;
    Int_t iRing = TMath::Floor(i/12);
    Double_t Phi_Pos=RING_DelPhi*(i%12) + RING_DPhi*iRing;
    Double_t Phi=Phi_Pos*Deg2Rad;
    xPos = Radius*TMath::Cos(Phi);
    yPos = Radius*TMath::Sin(Phi);
    zPos  = Module_Z_Position-RING_DelZ*iRing;
    module_trans = new TGeoTranslation("", xPos, yPos, zPos);
    TGeoRotation* module_rot = new TGeoRotation();
    module_rot->RotateZ(Phi_Pos);
    TGeoCombiTrans* module_combi_trans = NULL;
    module_combi_trans = new TGeoCombiTrans(*module_trans, *module_rot);
    gGeoMan->GetVolume(geoVersion)->AddNode(gModules[modType], modNum, module_combi_trans);
    modNum++;
  }
 }
}
