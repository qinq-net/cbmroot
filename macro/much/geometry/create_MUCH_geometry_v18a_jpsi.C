//                                             
/// \file create_MUCH_geometry_v18a.C
/// \brief Generates MUCH geometry in Root format.
///                                             
// 2018-05-22 - OS & PPB- v18a - Included additional material(drift+G10+Al)
// 2017-11-10 - PPB & OS   - change the structure 
// 2017-05-16 - DE  - v17b - position the modules in a way to split layers left-right along y axis
// 2017-05-16 - DE  - v17b - attribute name to module frames
// 2017-05-16 - DE  - v17b - remove rim from support CompositeShape
// 2017-05-02 - PPB - v17a - Change the shape of the first absorber according to latest design
// 2017-04-27 - DE  - v17a - fix GEM module positions and angles
// 2017-04-22 - PPB - v17a - Define the absorber, shield and station shapes sizes ...
// 2016-04-19 - DE  - v17a - initial version derived from TRD

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
#include "TGeoCone.h"
#include "TGeoBBox.h"
#include "TGeoTube.h"
#include "TFile.h"
#include "TString.h"
#include "TList.h"
#include "TRandom3.h"
#include "TDatime.h"
#include "TClonesArray.h"

#include "TObjArray.h"
#include "TFile.h"
#include "TMath.h"

#include <iostream>
#include <fstream>
#include <cassert>
#include <stdexcept>



// Name of output file with geometry
const TString tagVersion   = "_v18a_";
const TString subVersion   = "sis100_1m_jpsi";
const TString geoVersion   = "much";// + tagVersion + subVersion;
const TString FileNameSim  = geoVersion +  tagVersion + subVersion+".geo.root";
const TString FileNameGeo  = geoVersion +  tagVersion + subVersion+"_geo.root";
const TString FileNameInfo = geoVersion +  tagVersion + subVersion+".geo.info";

// Names of the different used materials which are used to build the modules
// The materials are defined in the global media.geo file 
const TString KeepingVolumeMedium     = "air";
const TString L= "MUCHlead";
const TString W= "MUCHwolfram";  
const TString C= "MUCHcarbon";
const TString I= "MUCHiron";
const TString activemedium="MUCHargon";
const TString spacermedium="MUCHnoryl";
const TString supportmedium="MUCHsupport";
const TString coolmedium="aluminium"; //Al cooling plates
const TString g10= "G10";  //G10 Plate
const TString copper = "copper"; //Drift


// Universal input parameters

Double_t fMuchZ1 =125.0; // MuchCave Zin position [cm]
Double_t fAcceptanceTanMin = 0.1; // Acceptance tangent min
Double_t fAcceptanceTanMax = 0.466; // Acceptance tangent max

//************************************************************


// Input parameters for absorbers 
//***********************************************************

const Int_t fNabs = 6; // Number of absorbers
// Absorber Zin position [cm] in the cave reference frame
Double_t fAbsorberZ1[6]={0, 16, 90, 140, 190, 250}; 
// Absorber thickness [cm]
Double_t fAbsorberLz[6]= {16, 44, 20, 20, 30,100}; 
Double_t safetyrad[6]={0.0,30.0,30.0,30.0,30.0,5.0};



// Input parameters for MUCH stations
//********************************************

const Int_t fNst = 4; // Number of stations
// Sector-type module parameters
// Number of sectors per layer (should be even for symmetry)
// Needs to be fixed with actual numbers
Int_t fNSectorsPerLayer[4] = {16, 20, 24, 28}; 
Double_t fActiveLzSector =0.3;  // Active volume thickness [cm]
Double_t fSpacerR = 2.0;         // Spacer width in R [cm]
Double_t fSpacerPhi = 2.0;       // Spacer width in Phi [cm]
Double_t fOverlapR = 2.0;        // Overlap in R direction [cm]

// Station Zceneter [cm] in  the cave reference frame

Double_t fStationZ0[4]={75,125,175,235};//,365}; 
Int_t fNlayers[4]={3,3,3,3};//,3}; // Number of layers
Int_t fDetType[4]={3,3,3,3};//,3}; // Detector type
Double_t fLayersDz[4]={10,10,10,10};//,10}; 
Double_t fSupportLz[4]={1.0,1.0,1.0,1.0};//,1.0};  Aluminum Support
Double_t fDriftDz=0.0035; //35 micron copper Drift  
Double_t fG10Dz = 0.3; // 3 mm G10
Double_t fCoolLz=1.0;  // thickness of the cooling plate 

/* 
   1 - detailed design (modules at two sides)
   * 0 - simple design (1 module per layer) 
 */

Int_t fModuleDesign[4]={1,1,1,1};//,1}; 


// Input parameters for beam pipe shielding
// spans from 2.9 degree to 5.1 degree
//Inner radius is tan(2.9) + 2 cm, extra 20 mm for clamp connection

const Int_t fNshs=4;
Double_t fShieldZin[4]={215.0, 265.0, 315.0, 375.0};
Double_t fShieldLz[4]={20, 20, 30,100};
Double_t fShield_AcceptanceTanMin = 0.051; // Acceptance tangent min for shield
Double_t fShield_AcceptanceTanMax = 0.1; // Acceptance tangent max for shield


//***********************************************************

// some global variables
TGeoManager* gGeoMan = NULL;  // Pointer to TGeoManager instance
TGeoVolume*  gModules[fNabs]; // Global storage for module types
TGeoVolume*  gModules_shield[fNshs]; // Global storage for module types
TGeoVolume*  gModules_station[fNst]; // Global storage for module types

// Forward declarations
void create_materials_from_media_file();
TGeoVolume* CreateAbsorbers(int iabs);
TGeoVolume* CreateShields(int ishield);
TGeoVolume* CreateStations(int ist);
TGeoVolume* CreateLayers(int istn, int ily);


void create_MUCH_geometry_v18a_jpsi() {
  
  // Load needed material definition from media.geo file
  create_materials_from_media_file();
  
  // Get the GeoManager for later usage
  gGeoMan = (TGeoManager*) gROOT->FindObject("FAIRGeom");
  gGeoMan->SetVisLevel(10);  
  
  // Create the top volume 
  TGeoBBox* topbox= new TGeoBBox("", 1000., 1000., 2000.);
  TGeoVolume* top = new TGeoVolume("top", topbox, gGeoMan->GetMedium("air"));
  gGeoMan->SetTopVolume(top);
  
  TString topName=geoVersion+tagVersion+subVersion;
  TGeoVolume* much = new TGeoVolumeAssembly(topName);
  top->AddNode(much, 1);

  TGeoVolume *absr = new TGeoVolumeAssembly("absorber");
  much->AddNode(absr,1);
  
  TGeoVolume *shld = new TGeoVolumeAssembly("shield");
  much->AddNode(shld,1);
  
  TGeoVolume *sttn = new TGeoVolumeAssembly("station");
  much->AddNode(sttn,1);
  
  
  for (Int_t iabs = 0; iabs <fNabs ; iabs++) { // 6 pieces of absorbers
    // first abosrber is divided into two halves
    // first half inserted inside the dipole magnet
    
    gModules[iabs] = CreateAbsorbers(iabs);
    
    absr->AddNode(gModules[iabs],iabs); 
  }
  
  
  for (Int_t ishi = 0; ishi <fNshs ; ishi++) {
    
    gModules_shield[ishi] = CreateShields(ishi);
    
    shld->AddNode(gModules_shield[ishi],ishi);

  }
  
  
  for (Int_t istn = 0; istn < fNst; istn++) { // 4 Stations
  
    
    gModules_station[istn] = CreateStations(istn);
    
    sttn->AddNode(gModules_station[istn],istn);
  }
  
  gGeoMan->CloseGeometry();
  //gGeoMan->CheckOverlaps(0.00001);
  gGeoMan->CheckOverlaps(0.0001, "s");
  
  gGeoMan->PrintOverlaps();
  //  gGeoMan->Test();
  
  
  much->Export(FileNameSim);   // an alternative way of writing the much

  TFile* outfile = new TFile(FileNameSim, "UPDATE");
  TGeoTranslation* much_placement = new TGeoTranslation("much_trans", 0., 0., 0.);
  much_placement->Write();
  outfile->Close();
  
  outfile = new TFile(FileNameGeo,"RECREATE");
  gGeoMan->Write();  // use this if you want GeoManager format in the output
  outfile->Close();
  
  top->Draw("ogl");
  
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
  
  FairGeoMedium* air = geoMedia->getMedium(KeepingVolumeMedium);
  geoBuild->createMedium(air);

  FairGeoMedium* MUCHiron = geoMedia->getMedium(I);
  geoBuild->createMedium(MUCHiron);
  
  FairGeoMedium* MUCHlead = geoMedia->getMedium(L);
  geoBuild->createMedium(MUCHlead);
  
  FairGeoMedium* MUCHwolfram = geoMedia->getMedium(W);
  geoBuild->createMedium(MUCHwolfram);

  FairGeoMedium* MUCHcarbon = geoMedia->getMedium(C);
  geoBuild->createMedium(MUCHcarbon);
  
  FairGeoMedium* MUCHargon = geoMedia->getMedium(activemedium);
  geoBuild->createMedium(MUCHargon);

  FairGeoMedium* MUCHnoryl = geoMedia->getMedium(spacermedium);
  geoBuild->createMedium(MUCHnoryl);
  
  FairGeoMedium* MUCHsupport = geoMedia->getMedium(supportmedium);
  geoBuild->createMedium(MUCHsupport);
  
  
  
  FairGeoMedium*aluminium  = geoMedia->getMedium(coolmedium);
  // geoBuild->createMedium(MUCHcool);
  geoBuild->createMedium(aluminium);
  
  FairGeoMedium* g10plate  = geoMedia->getMedium(g10); //G10
  geoBuild->createMedium(g10plate);
    
  FairGeoMedium* copperplate  = geoMedia->getMedium(copper); //Copper for Drift
  geoBuild->createMedium(copperplate);
 
}



TGeoVolume* CreateShields(int ish) {
  
  TGeoMedium* graphite = gGeoMan->GetMedium(C);
  TGeoMedium* iron = gGeoMan->GetMedium(I);
  
  TString name = Form("shieldblock%d", ish);
  TGeoVolumeAssembly* shieldblock = new TGeoVolumeAssembly(name);
  
  TString conename_sh =  Form("conesh_%d",ish);
  
  
  Double_t dz = fShieldLz[ish]/2.0 ;
  Double_t globalZ1 = fShieldZin[ish] ;
  Double_t globalZ2 = fShieldZin[ish] + 2 * dz ;
  
  
  
  Double_t rmin1 = globalZ1 * fShield_AcceptanceTanMin+2.0;
  Double_t rmax1 = globalZ1 * fShield_AcceptanceTanMax;
  Double_t rmin2 = globalZ2 * fShield_AcceptanceTanMin+2.0;
  Double_t rmax2 = globalZ2 * fShield_AcceptanceTanMax;
  //cout<<" Shields:  rmin1 "<<rmin1<<" rmax 1  "<<rmax1<<"  rmin2 "<<rmin2<<"  rmax2  "<<rmax2<<endl;
  
  
  TGeoCone * sh =new TGeoCone(conename_sh,dz, rmin1, rmax1, rmin2, rmax2);
  TGeoVolume* shield = new TGeoVolume("shield", sh, iron);
  
  shield->SetLineColor(kBlack);
  shield->SetTransparency(2);
  TGeoTranslation *sh_trans = new TGeoTranslation("", 0., 0., globalZ1+dz);
  shieldblock->AddNode(shield,ish, sh_trans); 
  return shieldblock; 
  
}


TGeoVolume* CreateAbsorbers(int i) {
  
  TGeoMedium* graphite = gGeoMan->GetMedium(C);
  TGeoMedium* iron = gGeoMan->GetMedium(I);

  TString name = Form("absblock%d", i);
  TGeoVolumeAssembly* absblock = new TGeoVolumeAssembly(name);
  
  TString pipename =  Form("beampipe_%d",i);
  TString conename =  Form("cone_%d",i);
  TString BoxName   = Form("Box_%d",i);
  TString supportShapeName = Form("Support_%d",i);
  TString TrapName =  Form("Trap_%d",i);
  
  Double_t dz = fAbsorberLz[i]/2.0 ;
  Double_t globalZ1 = fAbsorberZ1[i] + fMuchZ1;
  Double_t globalZ2 = fAbsorberZ1[i] + 2 * dz + fMuchZ1;
  
  Double_t rmin1 = globalZ1 * fAcceptanceTanMin;
  Double_t rmin2 = globalZ2 * fAcceptanceTanMin;
  Double_t rmax1 = globalZ1 * fAcceptanceTanMax + safetyrad[i];
  Double_t rmax2 = globalZ2 * fAcceptanceTanMax + safetyrad[i];//
  
  // 1st part of 1st absorber trapezium
  //dimensions are hardcoded
    if(i==0)
      {
	printf("absorber %d \n",i);
	
	TGeoTrd2 * trap = new TGeoTrd2(TrapName,70.0,70.0,46.0,71.0,dz);
	TGeoCone * tube = new TGeoCone(pipename,dz+0.001,0.,rmin1,0.,rmin2);
	TString expression = TrapName +"-"+pipename;
	TGeoCompositeShape* shSupport = new TGeoCompositeShape(supportShapeName,expression);
	TGeoVolume* abs0 = new TGeoVolume("absorber", shSupport, graphite);
	abs0->SetLineColor(kRed);
	abs0->SetTransparency(2);
	TGeoTranslation *abs0_trans = new TGeoTranslation("", 0., 0., globalZ1+dz);
	absblock->AddNode(abs0, i, abs0_trans);
      }
    
    // 2nd part of 1st absorber box
  if(i==1)
    {
      printf("absorber %d \n",i);
      TGeoBBox * box = new TGeoBBox(BoxName,130.0,125.0,dz);
      TGeoCone * tube = new TGeoCone(pipename,dz+0.001,0.,rmin1,0.,rmin2);
      TString expression = BoxName +"-"+pipename;
      TGeoCompositeShape* shSupport = new TGeoCompositeShape(supportShapeName,expression);
      
	TGeoVolume* abs1 = new TGeoVolume("absorber", shSupport, graphite);
	abs1->SetLineColor(kBlue);
	abs1->SetTransparency(2);
	TGeoTranslation *abs1_trans = new TGeoTranslation("", 0., 0., globalZ1+dz);
	absblock->AddNode(abs1, i, abs1_trans);
	
	
    }
  
  
  //rest of the absorbers
  if (!(i==0 || i ==1))
    {
      TGeoBBox * box = new TGeoBBox(BoxName,rmax2,rmax2,dz);
      TGeoCone * tube = new TGeoCone(pipename,dz+0.001,0.,rmin1,0.,rmin2);   
      TString expression = BoxName +"-"+pipename;
      TGeoCompositeShape* shSupport = new TGeoCompositeShape(supportShapeName,expression);
      
      TGeoVolume* abs2 = new TGeoVolume("absorber", shSupport, iron);
      
      abs2->SetLineColor(kBlue);
      abs2->SetTransparency(2);
      
      
      TGeoTranslation *abs_trans = new TGeoTranslation("", 0., 0., globalZ1+dz);
      absblock->AddNode(abs2,i, abs_trans); 
      //  cout<<" Abosrber # "<<i<<"  z  "<<globalZ1+dz<<endl; 
      
      
    }
  
  return absblock; 
  
}

TGeoVolume * CreateStations(int ist){
  
  TString stationName = Form("muchstation%02i",ist+1);

  TGeoVolumeAssembly* station = new TGeoVolumeAssembly(stationName);//, shStation, air);
  
  
  
  TGeoVolume*  gLayer[4];
  
  for (int ii=0;ii<3;ii++){  // 3 Layers
    
    
    gLayer[ii] = CreateLayers(ist, ii);
    station->AddNode(gLayer[ii],ii);

  }
  
  return station;
}


TGeoVolume * CreateLayers(int istn, int ily){
  
  TString layerName = Form("muchstation%02ilayer%i",istn+1,ily+1);
  //cout<<" check          "<<layerName<<endl;
  TGeoVolumeAssembly* volayer = new TGeoVolumeAssembly(layerName);
  
  
  Double_t stGlobalZ0 = fStationZ0[istn] + fMuchZ1; //z position of station center (midplane) [cm]
  Double_t stDz = ((fNlayers[istn] - 1) * fLayersDz[istn] + fSupportLz[istn]+2*fActiveLzSector)/2.;
  Double_t stGlobalZ2 = stGlobalZ0 + stDz;
  Double_t stGlobalZ1 = stGlobalZ0 - stDz;
  
  Double_t rmin = stGlobalZ1 * fAcceptanceTanMin;
  Double_t rmax = stGlobalZ2 * fAcceptanceTanMax;
  
  
  Double_t layerZ0 = (ily - (fNlayers[istn] - 1) / 2.) * fLayersDz[istn];
  Double_t layerGlobalZ0 = layerZ0 + stGlobalZ0;
  Double_t sideDz = fSupportLz[istn]/2. +fDriftDz+ fG10Dz+fActiveLzSector/2.; // distance between side's and layer's centers
  
 
  Double_t moduleZ = sideDz; // Z position of the module center in the layer cs
  Double_t phi0 = TMath::Pi()/fNSectorsPerLayer[istn]; // azimuthal half widh of each module
  Double_t ymin = rmin+fSpacerR;
  Double_t ymax = rmax;    
  
  //define the dimensions of the trapezoidal module
  Double_t dy  = (ymax-ymin)/2.; //y (length)
  Double_t dx1 = ymin*TMath::Tan(phi0)+fOverlapR/TMath::Cos(phi0);  // large x
  Double_t dx2 = ymax*TMath::Tan(phi0)+fOverlapR/TMath::Cos(phi0); // small x
  Double_t dz  = fActiveLzSector/2.; // thickness
  
  
  
//define the spacer dimensions      
  Double_t tg = (dx2-dx1)/2/dy;
  Double_t dd1 = fSpacerPhi*tg;
  Double_t dd2 = fSpacerPhi*sqrt(1+tg*tg);
  Double_t sdx1 = dx1+dd2-dd1-0.1; // 0.1 cm extra to avoid overlaps 
  Double_t sdx2 = dx2+dd2+dd1; 
  Double_t sdy  = dy+fSpacerR;
  Double_t sdz  = dz-0.1;
  
  //define Additional material as realistic GEM module
  Double_t dz_sD =fDriftDz/2.; //35 micron copper Drift 
  Double_t dz_sG=fG10Dz/2.; // 3mm G10
  Double_t dz_s=fCoolLz/2.;//10mm Al plate
  Double_t DriftZ_in=moduleZ-dz-dz_sD;//distance of the inner copper Drift center from layer layer center cs
  Double_t G10Z_in=DriftZ_in-dz_sD-dz_sG;//distance of the innerG10 center from layer layer center cs
  Double_t DriftZ_out=moduleZ+dz+dz_sD;//distance of the outer copper Drift center from layer layer center cs
  Double_t G10Z_out=DriftZ_out+dz_sD+dz_sG;//distance of the outerG10 center from layer layer center cs
  Double_t coolZ=G10Z_out+dz_sG+dz_s;//distance of the cooling plate center from layer center cs
  
  
  const Int_t Nsector=fNSectorsPerLayer[istn];
  TGeoVolume* gsector[Nsector];

  TVector3 pos,posDin,posGin,posAl,posDout,posGout;
  TVector3 size = TVector3(0.0, 0.0, fActiveLzSector);
  
  // Add the support structure
// Create support
  
  Double_t supportDx=sqrt(rmax*rmax+dx2*dx2);
  Double_t supportDy=sqrt(rmax*rmax+dx2*dx2);
  Double_t supportDz=fSupportLz[istn]/ 2.;
  
  
  TString supportBoxName   = Form("shStation%02iSupportBox",istn+1);
  TString supportHoleName  = Form("shStation%02iSupportHole",istn+1);
  TString translationName  = Form("trSt%02i",istn+1);
  TString supportShapeName = Form("shSt%02iSupport",istn+1);
  
  TGeoTube* shSupportHole = new TGeoTube(supportHoleName,0.,rmin,supportDz+0.001);
  TGeoBBox* shSupportBox  = new TGeoBBox(supportBoxName,supportDx,supportDy,supportDz);
  
  TString expression = supportBoxName+"-"+supportHoleName;
  TGeoCompositeShape* shSupport = new TGeoCompositeShape(supportShapeName,expression);
  
  TString  supportName1  = Form("muchstation%02ilayer%isupport",istn+1,ily+1);
  TGeoMedium* coolMatAl = gGeoMan->GetMedium(coolmedium);

  TGeoVolume* voSupport1 = new TGeoVolume(supportName1,shSupport,coolMatAl);
  voSupport1->SetLineColor(kCyan);
  
  TGeoTranslation *support_trans1 = new TGeoTranslation("supportName1", 0,0,layerGlobalZ0);
  volayer->AddNode(voSupport1,0,support_trans1);
  
  
  Int_t iMod =0;
for (Int_t iSide=0;iSide<2;iSide++){
  
  // Now start adding the GEM modules  
  for (Int_t iModule=0; iModule<fNSectorsPerLayer[istn]; iModule++){ 

    Double_t phi  = 2 * phi0 * (iModule + 0.2);  // add 0.5 to not overlap with y-axis for left-right layer separation
    Bool_t isBack = iModule%2; 
    Char_t cside  = (isBack==1) ? 'b' : 'f'; 
     
    // correct the x, y positions
    pos[0] = -(ymin+dy)*sin(phi);
    pos[1] =  (ymin+dy)*cos(phi);
    
    // different z positions for odd/even modules
      pos[2]  =(isBack ? 1 : -1)*moduleZ + layerGlobalZ0;  //z pos for ArgonActive
      posDin[2] =(isBack ? 1 : -1)*(DriftZ_in) + layerGlobalZ0; // Z pos for copper Drift innerside 
      posGin[2] =(isBack ? 1 : -1)*(G10Z_in) + layerGlobalZ0; // Z pos for G10......................
      posDout[2] =(isBack ? 1 : -1)*(DriftZ_out) + layerGlobalZ0; // Z pos for copper Drift outerside 
      posGout[2] =(isBack ? 1 : -1)*(G10Z_out) + layerGlobalZ0; // Z pos for G10.................
      posAl[2] =(isBack ? 1 : -1)*(coolZ) + layerGlobalZ0; // Z pos for Al Cooling Plates
      
      if(iSide!=isBack)continue;
      if(iModule!=0)iMod =iModule/2;
      
      //cout<<"Layer: "<<ily<<" Side: "<<cside<<" PosG10in: "<<posGin[2]<<" PosG10out: "<<posGout[2]<<" PosDriftin: "<<posDin[2]<<" PosDriftout: "<<posDout[2]<<" PosActive: "<<pos[2]<<" Al: "<<posAl[2]<<" Layer: "<<layerGlobalZ0<<endl;
 
      TGeoMedium* argon = gGeoMan->GetMedium(activemedium); // active medium
      TGeoMedium* noryl = gGeoMan->GetMedium(spacermedium); // spacer medium
      TGeoMedium* aluminium = gGeoMan->GetMedium(coolmedium); // cool medium
      TGeoMedium* g10plate = gGeoMan->GetMedium(g10); // G10 medium
      TGeoMedium* copperplate = gGeoMan->GetMedium(copper); // copper Drift medium

        
      // Define and place Additional material  G10 innerSide
      TGeoTrap* innerG10 = new TGeoTrap(dz_sG,0,phi,sdy,sdx1,sdx2,0,sdy,sdx1,sdx2,0);
      innerG10->SetName(Form("shStation%02iLayer%i%cModule%03iinner", istn, ily, cside, iModule));
      TString fG10Name = Form("muchstation%02ilayer%i%cside%03iinnerG10",istn+1,ily+1,cside,iMod+1);
      TGeoVolume* vofG10 = new TGeoVolume(fG10Name,innerG10,g10plate);
      vofG10->SetLineColor(28);
      
      // Define and place Additional material  copper Drift on innerSide
      TGeoTrap* innerDrift = new TGeoTrap(dz_sD,0,phi,sdy,sdx1,sdx2,0,sdy,sdx1,sdx2,0);
      innerDrift->SetName(Form("shStation%02iLayer%i%cModule%03iinner", istn, ily, cside, iModule));
      TString fDriftName = Form("muchstation%02ilayer%i%cside%03iinnercopperDrift",istn+1,ily+1,cside,iMod+1);
      TGeoVolume* vofDrift = new TGeoVolume(fDriftName,innerDrift,copperplate);
      vofDrift->SetLineColor(kRed);
      
      // Define and place Additional material  G10 on outer
      TGeoTrap* outerG10 = new TGeoTrap(dz_sG,0,phi,sdy,sdx1,sdx2,0,sdy,sdx1,sdx2,0);
      outerG10->SetName(Form("shStation%02iLayer%i%cModule%03iouter", istn, ily, cside, iModule));
      TString bG10Name = Form("muchstation%02ilayer%i%cside%03iouterG10",istn+1,ily+1,cside,iMod+1);
      TGeoVolume* vobG10 = new TGeoVolume(bG10Name,outerG10,g10plate);
      vobG10->SetLineColor(28);
      
      // Define and place Additional material  copper Drift on outer
      TGeoTrap* outerDrift = new TGeoTrap(dz_sD,0,phi,sdy,sdx1,sdx2,0,sdy,sdx1,sdx2,0);
      outerDrift->SetName(Form("shStation%02iLayer%i%cModule%03iouter", istn, ily, cside, iModule));
      TString bDriftName = Form("muchstation%02ilayer%i%cside%03ioutercopperDrift",istn+1,ily+1,cside,iMod+1);
      TGeoVolume* vobDrift = new TGeoVolume(bDriftName,outerDrift,copperplate);
      vobDrift->SetLineColor(kRed);
      
      
    // Define and place Additional material  Al Cooling Plate
      TGeoTrap* cool = new TGeoTrap(dz_s,0,phi,sdy,sdx1,sdx2,0,sdy,sdx1,sdx2,0);
      cool->SetName(Form("shStation%02iLayer%i%cModule%03icool", istn, ily, cside, iModule));
      TString CoolName = Form("muchstation%02ilayer%i%cside%03iAluminum",istn+1,ily+1,cside,iMod+1);
      TGeoVolume* voCool = new TGeoVolume(CoolName,cool,aluminium);
      voCool->SetLineColor(18); 
      

      // Define and place the trapezoidal GEM module in X-Y plane
      TGeoTrap* shape = new TGeoTrap(dz,0,phi,dy,dx1,dx2,0,dy,dx1,dx2,0);
      shape->SetName(Form("shStation%02iLayer%i%cModule%03iActiveNoHole", istn, ily, cside, iModule));
      TString activeName = Form("muchstation%02ilayer%i%cactive%03igasArgon",istn+1,ily+1,cside,iMod+1);
      TGeoVolume* voActive = new TGeoVolume(activeName,shape,argon);
      voActive->SetLineColor(kGreen);

      // Define the trapezoidal spacers 
      TGeoTrap* shapeFrame = new TGeoTrap(sdz,0,phi,sdy,sdx1,sdx2,0,sdy,sdx1,sdx2,0);
      shapeFrame->SetName(Form("shStation%02iLayer%i%cModule%03iFullFrameNoHole", istn, ily, cside, iModule));
      expression = Form("shStation%02iLayer%i%cModule%03iFullFrameNoHole-shStation%02iLayer%i%cModule%03iActiveNoHole", istn, ily, cside, iModule, istn, ily, cside, iModule);
      TGeoCompositeShape* shFrame = new TGeoCompositeShape(Form("shStation%02iLayer%i%cModule%03iFrameNoHole", istn, ily, cside, iModule), expression);
      TString frameName = Form("muchstation%02ilayer%i%cframe%03i",istn+1,ily+1,cside,iMod+1);
      TGeoVolume* voFrame = new TGeoVolume(frameName,shFrame,noryl);  // add a name to the frame
      voFrame->SetLineColor(kMagenta);

      
      // Calculate the phi angle of the sector where it has to be placed 
      Double_t angle = 180. / TMath::Pi() * phi;  // convert angle phi from rad to deg
      
      TGeoTranslation*trans2=new TGeoTranslation("",pos[0],pos[1],pos[2]);
      TGeoTranslation*trans2Din=new TGeoTranslation("",pos[0],pos[1],posDin[2]);
      TGeoTranslation*trans2Gin=new TGeoTranslation("",pos[0],pos[1],posGin[2]);
      TGeoTranslation*trans2Dout=new TGeoTranslation("",pos[0],pos[1],posDout[2]);
      TGeoTranslation*trans2Gout=new TGeoTranslation("",pos[0],pos[1],posGout[2]);
      TGeoTranslation*transAl=new TGeoTranslation("",pos[0],pos[1],posAl[2]);
      

cout<<"layerGlobalZ0: "<<layerGlobalZ0<<" Active Modules: "<<pos[2]<<" DriftIn: "<<posDin[2]<<" G10In: "<<posGin[2]<<" Driftout: "<<posDout[2]<<" G10out: "<<posGout[2]<<endl;

      // cout << "DE i: " << iModule << " x: " << pos[0] << " y: " << pos[1] << " z: " << pos[2] << " angle: " << angle <<" "<<istn<<" "<<ily<<endl;
      
      TGeoRotation *r2 = new TGeoRotation("r2");
      //rotate in the vertical plane (per to z axis) with angle 
      r2->RotateZ(angle);
      
      TGeoHMatrix *incline_mod = new TGeoHMatrix("");
      (*incline_mod) =  (*trans2) * (*r2);  // OK
      
      TGeoHMatrix *incline_modGin = new TGeoHMatrix("");//Inner copper Drift and G10
      (*incline_modGin) =  (*trans2Gin) * (*r2);
      TGeoHMatrix *incline_modDin = new TGeoHMatrix("");
      (*incline_modDin) =  (*trans2Din) * (*r2);
      
      TGeoHMatrix *incline_modGout = new TGeoHMatrix("");//Outer copper Drift and G10
      (*incline_modGout) =  (*trans2Gout) * (*r2);
      TGeoHMatrix *incline_modDout = new TGeoHMatrix("");
      (*incline_modDout) =  (*trans2Dout) * (*r2);
      
      TGeoHMatrix *incline_modAl = new TGeoHMatrix("");//Al cooling Plate
      (*incline_modAl) =  (*transAl) * (*r2);
      
      volayer->AddNode(vobDrift, iMod, incline_modDout); // copper Drift volume Outer Side
      volayer->AddNode(vobG10, iMod, incline_modGout);  // G10 volume ------------
      
      volayer->AddNode(voActive, iMod, incline_mod); // add active volume 
      volayer->AddNode(voFrame, iMod, incline_mod);  // add spacer ------
      
      volayer->AddNode(vofDrift, iMod, incline_modDin); //copper  Drift volume Inner Side
      volayer->AddNode(vofG10, iMod, incline_modGin);  //  G10 volume ------------
      
     // volayer->AddNode(voCool, iMod, incline_modAl);  // Al plate
      
     
      
  }
 }	     
  
  return volayer;
}

