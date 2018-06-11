// -------------------------------------------------------------------------
// -----                    CbmMuchGeoScheme source file               -----
// -----                  Created 18/02/08  by E. Kryshen
//------                  Modilfied 18/10/2017 by Omveer Singh         -----
// -------------------------------------------------------------------------
#include "CbmMuchGeoScheme.h"
#include "CbmMuchAddress.h"
#include "CbmMuchStation.h"
#include "CbmMuchLayer.h"
#include "CbmMuchLayerSide.h"
#include "CbmMuchModuleGem.h"
#include "CbmMuchModuleGemRadial.h"
#include "CbmMuchModuleGemRectangular.h"
#include "CbmMuchModuleStraws.h"
#include "TObjArray.h"
#include "TFile.h"
#include "TMath.h"
#include "TGeoCone.h"
//Added for biconical and parallelopiped
#include "TGeoCone.h"
#include "TGeoBBox.h"
#include "TGeoTube.h"
#include "TGeoCompositeShape.h"


#include "TGeoArb8.h"
#include "TGeoBBox.h"
#include "TGeoVolume.h"
#include "TGeoManager.h"
#include "TGeoCompositeShape.h"
#include "TGeoTube.h"
#include "TGeoCone.h"
#include "FairGeoMedia.h"
#include "FairGeoMedium.h"
#include "TGeoBoolNode.h"



#include <iostream>
#include <fstream>
#include <cassert>
#include <stdexcept>
#include "TClonesArray.h"
using std::cout;
using std::endl;

CbmMuchGeoScheme* CbmMuchGeoScheme::fInstance = NULL;
Bool_t CbmMuchGeoScheme::fInitialized = kFALSE;
Bool_t CbmMuchGeoScheme::fModulesInitialized = kFALSE;

// -------------------------------------------------------------------------
CbmMuchGeoScheme::CbmMuchGeoScheme() 
  : TObject(),
    fGeoPathHash(0),
    fCurrentVolume(NULL),
    fVolumeBoxShape(NULL),
    fVolumeTrapShape(NULL),    
    fGlobal(),    
    fGlobalTrap(),    
    fGlobalMatrix(NULL),
    fStation(0),
    fLayer(0),
    fActive(0),
    fGeoID(0),
    //fIsSimulation(kFALSE),
    fModules(),
    fSides(),
    fMapSides(),
    fStations(NULL),
    fAbsorbers(new TObjArray()),
    fMuchCave(NULL),
    fMuchZ1(0.),
    fAcceptanceTanMin(0.),
    fAcceptanceTanMax(0.),
    fNabs(0),
    fNst(0),
    fActiveLx(0.),
    fActiveLy(0.),
    fActiveLz(0.),
    fSpacerLx(0.),
    fSpacerLy(0.),
    fOverlapY(0.),
    fStrawLz(0.),
    fNSectorsPerLayer(0),
    fActiveLzSector(0.),
    fSpacerR(0.),
    fSpacerPhi(0),
    fOverlapR(0.),
    fAbsorberZ1(0),
    fAbsorberLz(0),
    fAbsorberMat(0),
    fStationZ0(0),
    fNlayers(0),
    fDetType(0),
    fLayersDz(0),
    fSupportLz(0),
    fModuleDesign(0),
    muchSt(NULL),
    muchLy(NULL),
    muchLySd(NULL),
    Rmin(-1.),
    Rmax(-1.),
    Dx2(-1.)
{
  LOG(DEBUG) << "CbmMuchGeoScheme created" << FairLogger::endl;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMuchGeoScheme::~CbmMuchGeoScheme() {

  if (fInstance != NULL)
    delete fInstance;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
CbmMuchGeoScheme* CbmMuchGeoScheme::Instance() {

  if (!fInstance)
    fInstance = new CbmMuchGeoScheme();
  return fInstance;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMuchGeoScheme::Init(TObjArray* stations, Int_t flag) {

  if (!fInitialized) {
    fStations = stations;
    //cout<<" enter ini loop  check"<<fStations->GetEntries()<<"    "<<stations->GetEntries()<<endl;
    fGeoID=flag;
    fInitialized = kTRUE;
  }
  gLogger->Debug(MESSAGE_ORIGIN,"CbmMuchGeoScheme init successful");
  InitModules();
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMuchGeoScheme::Init(TString digiFileName, Int_t flag) {

  TFile* oldfile=gFile;
  TFile* file=new TFile(digiFileName);
  TObjArray* stations = (TObjArray*) file->Get("stations");
  file->Close();
  file->Delete();
  gFile=oldfile;
  Init(stations,flag);
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMuchGeoScheme::InitModules() {

  if (!fModulesInitialized) {


    if (!fStations) Fatal("InitModules", "No input array of stations.");
    Int_t incSides = 0;
    fMapSides.clear();
    fSides.clear();
    fModules.clear();

    //cout<<"Total No. of Stations  "<<"  "<<GetNStations()<<" absorbers "<<GetNAbsorbers()<<endl;
    for (Int_t iStation = 0; iStation < GetNStations(); iStation++) {

      const CbmMuchStation* station = GetStation(iStation);
      if (!station)  continue;

      assert(iStation == CbmMuchAddress::GetStationIndex(station->GetDetectorId()));
      vector<CbmMuchLayerSide*> sides;
      vector<CbmMuchModule*> modules;

      for (Int_t iLayer = 0; iLayer < station->GetNLayers(); iLayer++) {
        CbmMuchLayer* layer = station->GetLayer(iLayer);
        if (!layer)  continue;

        assert(iLayer == CbmMuchAddress::GetLayerIndex(layer->GetDetectorId()));
        for (Int_t iSide = 0; iSide < 2; iSide++) {
          CbmMuchLayerSide* side = (CbmMuchLayerSide*) layer->GetSide(iSide);
          if (!side) continue;
          assert(iSide == CbmMuchAddress::GetLayerSideIndex(side->GetDetectorId()));
          if(side->GetNModules()!=0) fMapSides[side->GetDetectorId()] = incSides++;
          sides.push_back(side);

          for (Int_t iModule = 0; iModule < side->GetNModules(); iModule++) {
            CbmMuchModule* mod = side->GetModule(iModule);
            if (!mod) continue;

            assert(iModule == CbmMuchAddress::GetModuleIndex(mod->GetDetectorId()));
            assert(iStation == CbmMuchAddress::GetStationIndex(mod->GetDetectorId()));
            if(!mod->InitModule()) continue;
            modules.push_back(mod);
          } // Modules
        } // Sides
      } // Layers
      fSides.push_back(sides);
      fModules.push_back(modules);
    } // Stations

    fModulesInitialized = kTRUE;
  }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMuchStation* CbmMuchGeoScheme::GetStation(Int_t iStation) const {

  if (!fStations)
    return NULL;
  Bool_t result = (iStation >= 0) || (iStation < fStations->GetEntries());

  return result ? (CbmMuchStation*) fStations->At(iStation) : NULL;
}
// -------------------------------------------------------------------------



// -------------------------------------------------------------------------
CbmMuchLayer* CbmMuchGeoScheme::GetLayer(Int_t iStation, Int_t iLayer) const {

  CbmMuchStation* station = GetStation(iStation);
  return station ? station->GetLayer(iLayer) : NULL;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMuchLayerSide* CbmMuchGeoScheme::GetLayerSide(Int_t iStation, Int_t iLayer,
    Bool_t iSide) const {

  CbmMuchLayer* layer = GetLayer(iStation, iLayer);
  return layer ? layer->GetSide(iSide) : NULL;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMuchModule* CbmMuchGeoScheme::GetModule(Int_t iStation, Int_t iLayer,
    Bool_t iSide, Int_t iModule) const {

  CbmMuchLayerSide* side = GetLayerSide(iStation, iLayer, iSide);
  return side ? side->GetModule(iModule) : NULL;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMuchStation* CbmMuchGeoScheme::GetStationByDetId(Int_t detId) const {

  Int_t iStation = CbmMuchAddress::GetStationIndex(detId);
  assert(iStation < GetNStations());
  return GetStation(iStation);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMuchLayer* CbmMuchGeoScheme::GetLayerByDetId(Int_t detId) const {

  CbmMuchStation* station = GetStationByDetId(detId);
  Int_t iLayer = CbmMuchAddress::GetLayerIndex(detId);
  assert(iLayer < station->GetNLayers());
  return station ? station->GetLayer(iLayer) : NULL;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMuchLayerSide* CbmMuchGeoScheme::GetLayerSideByDetId(Int_t detId) const {

  CbmMuchLayer* layer = GetLayerByDetId(detId);
  Int_t iSide = CbmMuchAddress::GetLayerSideIndex(detId);
  assert(iSide < 2);
  return layer ? layer->GetSide(iSide) : NULL;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmMuchModule* CbmMuchGeoScheme::GetModuleByDetId(Int_t detId) const {

  CbmMuchLayerSide* side = GetLayerSideByDetId(detId);
  Int_t iModule = CbmMuchAddress::GetModuleIndex(detId);
  assert(iModule < side->GetNModules());
  return side ? side->GetModule(iModule) : NULL;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMuchGeoScheme::CreatePointArrays() {

  vector<CbmMuchModule*> modules = GetModules();
  for(vector<CbmMuchModule*>::iterator it=modules.begin(); it!=modules.end(); it++){
    CbmMuchModule* module = (*it);
    module->SetPoints(new TClonesArray("CbmVisPoint", 1));
  }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMuchGeoScheme::CreateHitArrays() {

  vector<CbmMuchModule*> modules = GetModules();
  for(vector<CbmMuchModule*>::iterator it=modules.begin(); it!=modules.end(); it++){
    CbmMuchModule* module = (*it);
    if (module->GetDetectorType()==1) {
      CbmMuchModuleGem* mod = (CbmMuchModuleGem*)module;
      mod->SetHits(new TClonesArray("CbmVisPixelHit",1));
    } else if (module->GetDetectorType()==2) {
      CbmMuchModuleStraws* mod = (CbmMuchModuleStraws*)module;
      mod->SetHits(new TClonesArray("CbmVisStripHit",1));
    }
  }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMuchGeoScheme::CreateClusterArrays() {

  vector<CbmMuchModule*> modules = GetModules();
  for(vector<CbmMuchModule*>::iterator it=modules.begin(); it!=modules.end(); it++){
    CbmMuchModule* module = (*it);
    if(module->GetDetectorType()!=1) continue;
    CbmMuchModuleGem* mod = (CbmMuchModuleGem*)module;
    mod->SetClusters(new TClonesArray("CbmVisMuchCluster", 1));
  }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMuchGeoScheme::ClearPointArrays() {

  vector<CbmMuchModule*> modules = GetModules();
  for(vector<CbmMuchModule*>::iterator it=modules.begin(); it!=modules.end(); it++){
    (*it)->GetPoints()->Clear();
  }
}
// -------------------------------------------------------------------------




// -------------------------------------------------------------------------
void CbmMuchGeoScheme::ClearHitArrays() {

  vector<CbmMuchModule*> modules = GetModules();
  for(vector<CbmMuchModule*>::iterator it=modules.begin(); it!=modules.end(); it++){
    (*it)->GetHits()->Clear();
  }
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmMuchGeoScheme::ClearClusterArrays() {

  vector<CbmMuchModule*> modules = GetModules();
  for(vector<CbmMuchModule*>::iterator it=modules.begin(); it!=modules.end(); it++){
    CbmMuchModule* module = (*it);
    if(module->GetDetectorType()!=1) continue;
    CbmMuchModuleGem* mod = (CbmMuchModuleGem*)module;
    mod->GetClusters()->Clear();
  }

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
vector<CbmMuchLayerSide*> CbmMuchGeoScheme::GetLayerSides(Int_t iStation) const {

  try
  {
    return fSides.at(iStation);
  }
  catch ( std::out_of_range exc)
  {
    Fatal("GetLayerSides","No input array of stations.");
  }
  return vector<CbmMuchLayerSide*>();
}

//Int_t CbmMuchGeoScheme::GetLayerSideNr(Int_t detId){
//  int i = 0;
//  Int_t sideId = GetLayerSideByDetId(detId)->GetDetectorId();
//  Int_t iStation = GetStationIndex(detId);
//  for(Int_t iSt=0; iSt<=iStation; iSt++){
//    vector<CbmMuchLayerSide*> sides = GetLayerSides(iSt);
//    for(Int_t iSide=0; iSide<sides.size(); iSide++){
//      i++;
//    }
//  }
//  printf("%i, %i\n",fMapSides[sideId] + 1, i);
//  assert(fMapSides[sideId] + 1 == i);
//  return i;
//}

Int_t CbmMuchGeoScheme::GetLayerSideNr(Int_t detId) const {
  Int_t sideId = GetLayerSideByDetId(detId)->GetDetectorId();
  if (fMapSides.find(sideId) == fMapSides.end())
    Fatal("GetLayerSideNr", "Wrong side id or no modules in the layer side");
  return fMapSides.find(sideId)->second + 1;
}

// -------------------------------------------------------------------------
void CbmMuchGeoScheme::NavigateTo(
				  const TString& path)
{
  
    gGeoManager->cd(path.Data());
//    fGeoPathHash;
    fGeoPathHash = path.Hash();
    fCurrentVolume = gGeoManager->GetCurrentVolume();
    fVolumeBoxShape = (TGeoBBox*)fCurrentVolume->GetShape();
    Double_t local[3] = {0., 0., 0.};
    gGeoManager->LocalToMaster(local, fGlobal);
  
}
 
//------------------------------------------------------
Double_t CbmMuchGeoScheme::GetSizeX(
				    const TString& path )
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateTo(path);
  }
  return  fVolumeBoxShape->GetDX();
  
}


Double_t CbmMuchGeoScheme::GetSizeY(
				    const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateTo(path);
  }
  return fVolumeBoxShape->GetDY(); 
  
}


Double_t CbmMuchGeoScheme::GetSizeZ(
				    const TString& path)
{
  if (fGeoPathHash != path.Hash()) {
    NavigateTo(path);
  }
  return fVolumeBoxShape->GetDZ(); 
  
  
}


Double_t CbmMuchGeoScheme::GetZ(
				const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateTo(path);
  }
  return fGlobal[2];
}


Double_t CbmMuchGeoScheme::GetY(
				const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateTo(path);
  }
  return fGlobal[1];
}


Double_t CbmMuchGeoScheme::GetX(
				const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateTo(path);
  }
  return fGlobal[0];
}
//-----------------------------------------------------------------------------------------------------
void CbmMuchGeoScheme::ExtractGeoParameter(TGeoNode* ncave, const char* volumeName){
  
  TObjArray* caveArr = ncave->GetNodes();
  
  for(int iSystem=0; iSystem<caveArr->GetEntriesFast(); iSystem++){
    TGeoNode* SystemNode = static_cast<TGeoNode*>(caveArr->At(iSystem));
     
    if(!TString(SystemNode->GetName()).Contains(volumeName))continue;
    TObjArray* MuchObj = SystemNode->GetNodes();

    for(Int_t iMuchObj=0;iMuchObj<MuchObj->GetEntriesFast(); iMuchObj++){
      
      TGeoNode* MuchObjNode = static_cast<TGeoNode*>(MuchObj->At(iMuchObj));
  	
      // if (!TString(MuchObjNode->GetName()).Contains("Station")) continue; 
      if (!TString(MuchObjNode->GetName()).Contains("station")) continue; 
      TString MuchObjPath = TString("/")  + ncave->GetName()  + "/"+ SystemNode->GetName()+ "/" +MuchObjNode->GetName();
      
      StationNode(MuchObjNode, MuchObjPath);//StationNode
    }//iMuchObj
    
  }//iSystem
}//SetParameters

//---------------------------------------------------------------------------------------------
void CbmMuchGeoScheme::StationNode(TGeoNode* fSNode, TString Path){
  
  TObjArray* stations =fSNode->GetNodes();
  //fStations=stations; // by PPB 1/11/2017
  fNst = stations->GetEntriesFast();
  //  cout<<"check stations   "<<fNst<<endl;
  fStationZ0.Set(fNst); // Station Zceneter [cm]
  fNlayers.Set(fNst); // Number of layers
  fDetType.Set(fNst); // Detector type
  fLayersDz.Set(fNst); // Distance between layers [cm]
  fSupportLz.Set(fNst); // Support thickness [cm]
  fLayersDz.Set(fNst); //
  fModuleDesign.Set(fNst); // Module design (0/1)
  fNSectorsPerLayer.Set(fNst); // Number of sectors per layer in sector GEM geometry
  
  
  for (Int_t iStation = 0; iStation < fNst; iStation++) {
    TGeoNode* station = static_cast<TGeoNode*>(stations->At(iStation));
    TString StationPath = Path + "/" + station->GetName();
    TObjArray* layers = station->GetNodes();
    fNlayers[iStation] = layers->GetEntriesFast();
    
 

    fDetType[iStation]=3;
    //      fLayersDz[iStation]=10.0;
    //-----------------------------Layer2-------------------------------------------------------
    TGeoNode* layer2 = static_cast<TGeoNode*>(layers->At(1));//second layer position of station
    TString Layer2Path = StationPath + "/" + layer2->GetName();
   
    TObjArray* Supportlayer2 = layer2->GetNodes(); 
    TGeoNode* Supportlayer2Node = static_cast<TGeoNode*>(Supportlayer2->At(0));
    
    TString Supportlayer2Path = Layer2Path + "/" + Supportlayer2Node->GetName();

   Double_t  fLayer2Z0;
   //if(Supportlayer2Path.Contains("mcbm")) fLayer2Z0=GetModuleZ(Supportlayer2Path);
 if (fGeoID ==1)fLayer2Z0=GetModuleZ(Supportlayer2Path);
 else fLayer2Z0 = GetZ(Supportlayer2Path);
    fStationZ0[iStation] = fLayer2Z0;

    
    //------------------------------Layer3-----------------------------------------------------------
    TGeoNode* layer3 = static_cast<TGeoNode*>(layers->At(2));
    TString Layer3Path = StationPath + "/" + layer3->GetName();
    
    TObjArray* Supportlayer3 = layer3->GetNodes(); 
    TGeoNode* Supportlayer3Node = static_cast<TGeoNode*>(Supportlayer3->At(0));
    TString Supportlayer3Path = Layer3Path + "/" + Supportlayer3Node->GetName();
 
    Double_t  fLayer3Z0;
    //if(Supportlayer3Path.Contains("mcbm"))  fLayer3Z0=GetModuleZ(Supportlayer3Path);
 if(fGeoID==1)  fLayer3Z0=GetModuleZ(Supportlayer3Path);
   else fLayer3Z0 = GetZ(Supportlayer3Path);
    fLayersDz[iStation]=fLayer3Z0 - fLayer2Z0;
    
    //if(Supportlayer3Path.Contains("mcbm")) fSupportLz[iStation]=2.0*GetModuleDZ(Supportlayer3Path);
 if (fGeoID==1) fSupportLz[iStation]=2.0*GetModuleDZ(Supportlayer3Path);
else fSupportLz[iStation]=2.0*GetSizeZ(Supportlayer3Path);
//cout<<fSupportLz[iStation]<<"  "<<fLayersDz[iStation]<<endl;

    //------------------------------Station_Rmax&Rmin-------------------------------------------
    TGeoNode* Activelayer3Node = static_cast<TGeoNode*>(Supportlayer3->At(1));
    TString Activelayer3Path = Layer3Path + "/" + Activelayer3Node->GetName();
    
    Double_t PosY=GetModuleY(Activelayer3Path);
    // Double_t PosX=GetModuleX(Activelayer3Path);
    Double_t Phi = GetModulePhi(Activelayer3Path);
    Double_t Dy = GetModuleH1(Activelayer3Path);
    fActiveLzSector=2.0*GetModuleDZ(Activelayer3Path);
    
    

    //chaned by PPB on 16.11.2017
    //Double_t yMin = -(PosX/TMath::Sin(Phi))-Dy;
    Double_t yMin = (PosY/TMath::Cos(Phi))-Dy;
    Double_t yMax = 2*Dy + yMin;
    cout<<" Geo Scheme "<<" posY  "<<PosY<<" phi "<<Phi<<"  Dy "<<Dy<<endl;
    Rmax = yMax;
    Rmin = yMin;//-2.0; // Spacer width[cm] = 2.0
       
   
    muchSt = new CbmMuchStation(iStation, fStationZ0[iStation]);
    muchSt->SetRmin(Rmin);
    muchSt->SetRmax(Rmax);
    
    if (Supportlayer3->GetEntriesFast()>0) fModuleDesign[iStation]=1;
    muchSt->SetModuleDesign(fModuleDesign[iStation]);
    
    
    LayerNode(station,iStation,StationPath);
    
    
    
    muchLy = muchSt->GetLayer(0);
    Double_t supDx  = muchLy->GetSupportDx();
    Double_t supDy  = muchLy->GetSupportDy();
//    Double_t supDz  = muchLy->GetSupportDz();
    muchSt->SetTubeRmin(muchSt->GetRmin());
    muchSt->SetTubeRmax(TMath::Sqrt(supDx*supDx+supDy*supDy)+10);
    //cout<<" fill fStations array "<<iStation<<" z cent "<<fStationZ0[iStation]<<endl;
    fStations->Add(muchSt);  
    
  }//Station
}//StationNode
//---------------------------------------------------------------------------------------------------------

void CbmMuchGeoScheme::LayerNode(TGeoNode* StNode,Int_t iStation, TString StPath){

  TObjArray* layerArray = StNode->GetNodes();
  for(Int_t iLayer =0; iLayer<layerArray->GetEntriesFast(); iLayer++){
    TGeoNode* layerNode = static_cast<TGeoNode*>(layerArray->At(iLayer));
    TString layerPath = StPath + "/" + layerNode->GetName(); 

    ModuleNode(layerNode,iStation,iLayer,layerPath);

    //Set support shape
        muchLy->SetSupportDx(sqrt(Rmax*Rmax+Dx2*Dx2));
        muchLy->SetSupportDy(sqrt(Rmax*Rmax+Dx2*Dx2));
        muchLy->SetSupportDz(fSupportLz[iStation] / 2.);

muchSt->AddLayer(muchLy);
    
  }//iLayer
}
//---------------------------------------------------------------------------------------------------------------------------------

void CbmMuchGeoScheme::ModuleNode(TGeoNode* layerNode, Int_t iStation, Int_t iLayer, TString layerPath){
  
  
  TObjArray* moduleArray = layerNode->GetNodes();
  for(Int_t iModule =0; iModule<moduleArray->GetEntriesFast(); iModule++){
    TGeoNode* moduleNode = static_cast<TGeoNode*>(moduleArray->At(iModule));
    
    TString modulePath = layerPath + "/" + moduleNode->GetName();

    ActiveModuleNode(moduleNode,iStation,iLayer,iModule,modulePath);
    


  }//iModule
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void CbmMuchGeoScheme::ActiveModuleNode(TGeoNode* moduleNode, Int_t iStation, Int_t iLayer, Int_t /*iModule*/, TString modulePath){

  TString modName = moduleNode->GetName();



if(modName.Contains("support")){  


Double_t layerGlobalZ0;
// if(modulePath.Contains("mcbm")) layerGlobalZ0=GetModuleZ(modulePath);
 if (fGeoID==1) layerGlobalZ0=GetModuleZ(modulePath);
 else layerGlobalZ0 = GetZ(modulePath);

Double_t layerZ0 = (iLayer - (fNlayers[iStation] - 1) / 2.) * fLayersDz[iStation];
Double_t sideDz = fSupportLz[iStation]/2. + fActiveLzSector/2.;

        muchLy = new CbmMuchLayer(iStation, iLayer, layerGlobalZ0, layerZ0);  //CbmMuchLayer Class
	muchLy->GetSideB()->SetZ(layerGlobalZ0 + sideDz);
	muchLy->GetSideF()->SetZ(layerGlobalZ0 - sideDz);

 }




  if(modName.Contains("active")){
    
    gGeoManager->cd(modulePath.Data());
    //   TGeoNode* moduleNode = gGeoManager->GetMother(0);
    moduleNode = gGeoManager->GetMother(0);
//    Int_t nModule=moduleNode->GetNumber();

    Bool_t iSide;
    if(modName.Contains("factive")) iSide = 0;
    if(modName.Contains("bactive")) iSide = 1;
    TVector3 pos;
    pos[0]=GetModuleX(modulePath);
    pos[1]=GetModuleY(modulePath);
    pos[2]=GetModuleZ(modulePath);
    //   cout<<" positions  "<<pos[0]<<"  "<<pos[1]<<"  "<<pos[2]<<endl;
    Double_t Dz = GetModuleDZ(modulePath);
    Double_t Phi = GetModulePhi(modulePath);
    Double_t Dy = GetModuleH1(modulePath);
    Double_t Dx1 = GetModuleBl1(modulePath);
    Dx2 = GetModuleTl1(modulePath);
    cout<<" positions  "<<pos[0]<<"  "<<pos[1]<<"  "<<pos[2]<<" phi "<<Phi<<endl;

    // Is this formula correct? Check Omveer (PPB 16.11.2017)
//    Double_t yMin = (pos[1]/TMath::Cos(Phi))-Dy;
//    Double_t yMax = 2*Dy + yMin;
   // Double_t Rmin = yMin-2.0; // Spacer width[cm] = 2.0
    // Why is R0 required?
//    Double_t R0 = yMax-Dy;
   // Double_t Rmax = yMax;


  muchLySd = muchLy->GetSide(iSide);

  muchLySd->AddModule(new CbmMuchModuleGemRadial(iStation,iLayer, iSide, muchLySd->GetNModules(), pos,Dx1,Dx2,Dy,Dz,Rmin));


  }//activeLayer
 



}//ActiveModuleNode
//----------------------------------------------------------------------------------------------------------
void CbmMuchGeoScheme::NavigateModule(
				      const TString& path)
{
    gGeoManager->cd(path.Data());
//    fGeoPathHash;
    fGeoPathHash = path.Hash();
    fCurrentVolume = gGeoManager->GetCurrentVolume();
    fVolumeTrapShape = (TGeoTrap*)fCurrentVolume->GetShape();
    Double_t local[3] = {0., 0., 0.};
    gGeoManager->LocalToMaster(local, fGlobalTrap);
 
}//NavigateModule

//----------------------------------------------------------------------------------------------------------

Double_t CbmMuchGeoScheme::GetModuleDZ(
				      const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateModule(path);
  }
  return  fVolumeTrapShape->GetDZ();
}


Double_t CbmMuchGeoScheme::GetModuleZ(
				      const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateModule(path);
  }
  return fGlobalTrap[2];
}


Double_t CbmMuchGeoScheme::GetModuleY(
				      const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateModule(path);
  }
  return fGlobalTrap[1];
}


Double_t CbmMuchGeoScheme::GetModuleX(
				      const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateModule(path);
  }
  return fGlobalTrap[0];
}

Double_t CbmMuchGeoScheme::GetModulePhi(
					const TString& path)
{

  if (fGeoPathHash != path.Hash()) {
    NavigateModule(path);
  }
  return fVolumeTrapShape->GetPhi();
}

Double_t CbmMuchGeoScheme::GetModuleH1(
				       const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateModule(path);
  }
  return fVolumeTrapShape->GetH1();
}

Double_t CbmMuchGeoScheme::GetModuleBl1(
					const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateModule(path);
  }
  return fVolumeTrapShape->GetBl1();
}

Double_t CbmMuchGeoScheme::GetModuleTl1(
					const TString& path)
{
  
  if (fGeoPathHash != path.Hash()) {
    NavigateModule(path);
  }
  return fVolumeTrapShape->GetTl1();
}



// -------------------------------------------------------------------------
/*void CbmMuchGeoScheme::ReadGeoFile(const char* geoName) {
cout<<"----------------37---------------"<<endl;
// Read geometry parameters from file
fstream geo(geoName, fstream::in);
Char_t b[200]; // read buffer
  Int_t i;
  // Skip header
  while (!geo.eof()) {
    geo.getline(b, 200);
    if (b[0] != '#')
      break;
  }

  geo.getline(b, 200); // # General Information
  geo.get(b, 30);
  geo >> fMuchZ1;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fAcceptanceTanMin;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fAcceptanceTanMax;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fNabs;
  geo.getline(b, 200);
  geo.get(b, 30);
 // geo >> fNst;
 // geo.getline(b, 200);
fNst=4;
  // Create arrays of absorber parameters
  fAbsorberZ1.Set(fNabs); // Absorber Zin position [cm]
  fAbsorberLz.Set(fNabs); // Absorber thickness [cm]
  fAbsorberMat.Set(fNabs); // Absorber material
  // Create arrays of station parameters
  fStationZ0.Set(fNst); // Station Zceneter [cm]
  fNlayers.Set(fNst); // Number of layers
  fDetType.Set(fNst); // Detector type
  fLayersDz.Set(fNst); // Distance between layers [cm]
  fSupportLz.Set(fNst); // Support thickness [cm]
  fModuleDesign.Set(fNst); // Module design (0/1)
  fNSectorsPerLayer.Set(fNst); // Number of sectors per layer in sector GEM geometry
  geo.getline(b, 200);

  // # Absorber specification
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNabs; i++)
    geo >> fAbsorberZ1[i];
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNabs; i++)
    geo >> fAbsorberLz[i];
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNabs; i++)
    geo >> fAbsorberMat[i];
  geo.getline(b, 200);

  geo.getline(b, 200);

  // # Station specification
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNst; i++)
    geo >> fStationZ0[i];
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNst; i++)
    geo >> fNlayers[i];
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNst; i++)
    geo >> fDetType[i];
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNst; i++)
    geo >> fLayersDz[i];
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNst; i++)
    geo >> fSupportLz[i];
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNst; i++)
    geo >> fModuleDesign[i];
  geo.getline(b, 200);

  geo.getline(b, 200);

  // # GEM module specification
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fActiveLx;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fActiveLy;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fActiveLz;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fSpacerLx;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fSpacerLy;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fOverlapY;
  geo.getline(b, 200);

  geo.getline(b, 200);

  // # Straw module specification
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fStrawLz;
  geo.getline(b, 200);

  geo.getline(b, 200);

  // # Sector-type GEM module specification
  geo.getline(b, 200);
  geo.get(b, 30);
  for (i = 0; i < fNst; i++)  geo >> fNSectorsPerLayer[i];
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fActiveLzSector;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fSpacerR;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fSpacerPhi;
  geo.getline(b, 200);
  geo.get(b, 30);
  geo >> fOverlapR;
  geo.getline(b, 200);

  geo.close();
}*/
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
/*void CbmMuchGeoScheme::Print() {
cout<<"----------------38---------------"<<endl;
  Int_t i;
  // Print geometry parameters
  printf("\n");
  printf("General information:\n");
  printf("  MuchCave Zin position [cm]  :%7.1f\n", fMuchZ1);
  printf("  Acceptance tangent min      :%7.1f\n", fAcceptanceTanMin);
  printf("  Acceptance tangent max      :%7.1f\n", fAcceptanceTanMax);
  printf("  Number of absorbers         :%7i\n", fNabs);
  printf("  Number of stations          :%7i\n", fNst);
  printf("\n");
  printf("Absorber specification:\n");
  printf("  Absorber Zin position [cm]  :");
  for (i = 0; i < fNabs; i++)
    printf("%7.1f", fAbsorberZ1[i]);
  printf("\n");
  printf("  Absorber thickness [cm]     :");
  for (i = 0; i < fNabs; i++)
    printf("%7.1f", fAbsorberLz[i]);
  printf("\n");
  printf("  Absorber material           :");
  for (i = 0; i < fNabs; i++)
    printf("      %c", fAbsorberMat[i]);
  printf("\n");
  printf("\n");
  printf("Station specification:\n");
  printf("  Station Zceneter [cm]       :");
  for (i = 0; i < fNst; i++)
    printf("%7.1f", fStationZ0[i]);
  printf("\n");
  printf("  Number of layers            :");
  for (i = 0; i < fNst; i++)
    printf("%7i", fNlayers[i]);
  printf("\n");
  printf("  Detector type               :");
  for (i = 0; i < fNst; i++)
    printf("%7i", fDetType[i]);
  printf("\n");
  printf("  Distance between layers [cm]:");
  for (i = 0; i < fNst; i++)
    printf("%7.1f", fLayersDz[i]);
  printf("\n");
  printf("  Support thickness [cm]      :");
  for (i = 0; i < fNst; i++)
    printf("%7.1f", fSupportLz[i]);
  printf("\n");
  printf("  Module design (0/1)         :");
  for (i = 0; i < fNst; i++)
    printf("%7i", fModuleDesign[i]);
  printf("\n");

  printf("\n");
  printf("GEM module specification:\n");
  printf("  Active volume lx [cm]       :%7.1f\n", fActiveLx);
  printf("  Active volume ly [cm]       :%7.1f\n", fActiveLy);
  printf("  Active volume lz [cm]       :%7.1f\n", fActiveLz);
  printf("  Spacer lx [cm]              :%7.1f\n", fSpacerLx);
  printf("  Spacer ly [cm]              :%7.1f\n", fSpacerLy);
  printf("  Overlap along y axis [cm]   :%7.1f\n", fOverlapY);
  printf("\n");

  printf("\n");
  printf("Straw module specification:\n");
  printf("  Straw thickness [cm]        :%7.1f\n", fStrawLz);

  printf("\n");
  printf("Sector-type GEM module specification (type 3):\n");
  printf("  Number of sectors/layer     :");
  for (i = 0; i < fNst; i++)
    printf("%7i", fNSectorsPerLayer[i]);
  printf("\n");
  printf("  Active volume lz [cm]       :%7.1f\n", fActiveLzSector);
  printf("  Spacer in r [cm]            :%7.1f\n", fSpacerR);
  printf("  Spacer in phi [cm]          :%7.1f\n", fSpacerPhi);
  printf("  Overlap in r [cm]           :%7.1f\n", fOverlapR);
  printf("\n");
}*/
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
/*void CbmMuchGeoScheme::CreateMuchCave() {
cout<<"----------------39---------------"<<endl;
  CreateAbsorbers();
  CreateStations();
cout<<"--------chk_NStations----------------"<<fStations->GetEntries()<<endl;
  // Determine Z2;
  Double_t muchZ2=fMuchZ1;
  Double_t z2;
  for (Int_t i = 0; i <fAbsorbers->GetEntries(); i++) {
   // TGeoCone* absorber = (TGeoCone*) fAbsorbers->At(i);
   // Double_t z2 = fMuchZ1+fAbsorberZ1[i]+2*absorber->GetDz();
    //if (z2 > muchZ2) muchZ2 = z2;
  if(i==0)
	{
	  // TGeoCone* absorber = (TGeoCone*) fAbsorbers->At(i);
	  //z2 = fMuchZ1+fAbsorberZ1[i]+2*absorber->GetDz();
	   TGeoCompositeShape* absorber = (TGeoCompositeShape*) fAbsorbers->At(i);
	   z2 = fMuchZ1+fAbsorberZ1[i]+2*absorber->GetDZ();
	  // printf("  cave z2  %f", z2);
	}
      else
      	{
	  TGeoBBox* absorber = (TGeoBBox*) fAbsorbers->At(i);
    	  z2 = fMuchZ1+fAbsorberZ1[i]+2*absorber->GetDZ();
	}
      
      
      if (z2 > muchZ2) muchZ2 = z2;
    }



  for (Int_t i = 0; i <fStations->GetEntries(); i++) {
    CbmMuchStation* station = (CbmMuchStation*) fStations->At(i);
    z2=station->GetZ()+station->GetTubeDz();
    if (z2>muchZ2) muchZ2=z2;
  }
  Double_t muchRmin1 = fMuchZ1 * fAcceptanceTanMin - 0.001;
  Double_t muchRmin2 = muchZ2 * fAcceptanceTanMin - 0.001;
  Double_t muchRmax2 = muchZ2 * fAcceptanceTanMax + 150; // 100 cm added for safety - avoid overlap with platform v13a
  Double_t muchRmax1 = muchRmax2;
  // Calculate cone thickness
  Double_t muchDz = (muchZ2 - fMuchZ1) / 2.;
  // Calculate cone position
  Double_t fMuchZ0 = fMuchZ1 + muchDz;
  fMuchCave = new TGeoCone(muchDz, muchRmin1, muchRmax1, muchRmin2, muchRmax2);
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMuchGeoScheme::CreateAbsorbers() {
cout<<"----------------40---------------"<<endl;

  for (Int_t i = 0; i < fNabs; i++) {
 TString pipename =  Form("beampipe_%d",i);
    TString conename =  Form("cone_%d",i);
    TString BoxName   = Form("Box_%d",i);
    TString supportShapeName = Form("Support_%d",i);
    Double_t dz = fAbsorberLz[i] / 2.;
    Double_t globalZ1 = fAbsorberZ1[i] + fMuchZ1;
    Double_t globalZ2 = fAbsorberZ1[i] + 2 * dz + fMuchZ1;
    Double_t rmin1 = globalZ1 * fAcceptanceTanMin;
    Double_t rmax1 = globalZ1 * fAcceptanceTanMax + 30;
    Double_t rmin2 = globalZ2 * fAcceptanceTanMin;
    Double_t rmax2 = globalZ2 * fAcceptanceTanMax + 30;
//    fAbsorbers->Add(new TGeoCone(dz, rmin1, rmax1, rmin2, rmax2));

   if(i==0)
   {
     TGeoCone * ab1 =new TGeoCone(conename,dz, 0.0, rmax1, 0.0, rmax2);
     //  TGeoBBox * box = new TGeoBBox(BoxName,rmax2,rmax2,dz);
     TGeoCone * tube = new TGeoCone(pipename,dz+0.001,0.0,rmin1,0.0,rmin2);   
      TString expression = conename +"-"+pipename;
     //TString expression = BoxName+"-"+pipename;
     TGeoCompositeShape* shSupport = new TGeoCompositeShape(supportShapeName,expression);
     fAbsorbers->Add(shSupport);
   }
    else
    {
      TGeoBBox * box = new TGeoBBox(BoxName,rmax2,rmax2,dz);
      TGeoCone * tube = new TGeoCone(pipename,dz+0.001,0.,rmin1,0.,rmin2);   
      TString expression = BoxName +"-"+pipename;
      TGeoCompositeShape* shSupport = new TGeoCompositeShape(supportShapeName,expression);
      fAbsorbers->Add(shSupport);
    }
 
  }
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMuchGeoScheme::CreateStations() {
cout<<"----------------41---------------"<<endl;
  Info("CreateStations", "...");
  // Create stations
  for (Int_t st = 0; st < fNst; st++) {
    CbmMuchStation* station;
    switch (fDetType[st]){
      case 1:
        station = CreateStationGem(st);
        break;
      case 2:
        station = CreateStationStraw(st);
        break;
      case 3:
        station = CreateStationGem(st);
        break;
      default:
        Fatal("CbmMuchGeoScheme", "Detector type %i not defined",fDetType[st]);
    }
    CbmMuchLayer* layer0 = station->GetLayer(0);
    Double_t supDx  = layer0->GetSupportDx();
    Double_t supDy  = layer0->GetSupportDy();
    Double_t supDz  = layer0->GetSupportDz();
    station->SetTubeRmin(station->GetRmin());
    station->SetTubeRmax(TMath::Sqrt(supDx*supDx+supDy*supDy)+10);
    fStations->Add(station);
  } // stations
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
CbmMuchStation* CbmMuchGeoScheme::CreateStationGem(Int_t st){
cout<<"----------------42---------------"<<endl;
  
  Double_t stGlobalZ0 = fStationZ0[st] + fMuchZ1;
  Double_t stDz = ((fNlayers[st] - 1) * fLayersDz[st] + fSupportLz[st]+2*fActiveLzSector)/2.;
  Double_t stGlobalZ2 = stGlobalZ0 + stDz;
  Double_t rmin = stGlobalZ2 * fAcceptanceTanMin;
  Double_t rmax = stGlobalZ2 * fAcceptanceTanMax;

  CbmMuchStation* station = new CbmMuchStation(st, stGlobalZ0);
  station->SetRmin(rmin);
  station->SetRmax(rmax);
  station->SetModuleDesign(fModuleDesign[st]);

  // Create layers
  for (Int_t l = 0; l < fNlayers[st]; l++) {
    Double_t layerZ0 = (l - (fNlayers[st] - 1) / 2.) * fLayersDz[st];
    Double_t layerGlobalZ0 = layerZ0 + stGlobalZ0;
    Double_t sideDz = fSupportLz[st]/2. + fActiveLzSector/2.; // distance between side's and layer's centers
    CbmMuchLayer* layer = new CbmMuchLayer(st, l, layerGlobalZ0, layerZ0);
    layer->GetSideB()->SetZ(layerGlobalZ0 + sideDz);
    layer->GetSideF()->SetZ(layerGlobalZ0 - sideDz);

    Double_t moduleZ = sideDz; // Z position of the module center in the layer cs
    if (fModuleDesign[st]) {
      // Create modules
      if (fDetType[st]==3){ // start sector modules
        Double_t phi0 = TMath::Pi()/fNSectorsPerLayer[st];
        Double_t ymin = rmin+fSpacerR;
        Double_t ymax = rmax;
        Double_t dy  = (ymax-ymin)/2.;
        Double_t dx1 = ymin*TMath::Tan(phi0)+fOverlapR/TMath::Cos(phi0); 
        Double_t dx2 = ymax*TMath::Tan(phi0)+fOverlapR/TMath::Cos(phi0); 
        Double_t dz  = fActiveLzSector/2.;
        TVector3 size = TVector3(fActiveLx, fActiveLy, fActiveLz);
        TVector3 pos;
        
        for (Int_t iModule=0;iModule<fNSectorsPerLayer[st];iModule++){
          Double_t phi = 2*phi0*iModule;
          Bool_t isBack = iModule%2; 
          pos[0] = (ymin+dy)*cos(phi);
          pos[1] = (ymin+dy)*sin(phi);
          pos[2] = (isBack ? 1 : -1)*moduleZ + layerGlobalZ0;
          CbmMuchLayerSide* side = layer->GetSide(isBack);
          side->AddModule(new CbmMuchModuleGemRadial(st, l, isBack, side->GetNModules(), pos,dx1,dx2,dy,dz,rmin));
        }
        // Set support shape
        layer->SetSupportDx(sqrt(rmax*rmax+dx2*dx2));
        layer->SetSupportDy(sqrt(rmax*rmax+dx2*dx2));
        layer->SetSupportDz(fSupportLz[st] / 2.);

      } else { // start rectangular modules
        Double_t moduleDx = fSpacerLx + fActiveLx / 2.; // half-width of the module including spacers
        Double_t moduleDy = fSpacerLy + fActiveLy / 2.; // half-length of the module including spacers
        Int_t nx = Int_t(TMath::Ceil(rmax / 2. / moduleDx)); // half-number of modules along the X direction
        Int_t ny = Int_t(TMath::Ceil(rmax / (fActiveLy - fOverlapY))); // half-number of modules along the Y direction
        for (Int_t ix = 1; ix <= nx; ++ix) {
          Double_t moduleX = moduleDx * (2 * ix - 1);
          for (Int_t iy = -ny; iy < ny; ++iy) {
            moduleZ *= -1;
            Bool_t isBack = (moduleZ > 0);
            CbmMuchLayerSide* side = layer->GetSide(isBack);
            Double_t moduleY = (fActiveLy - fOverlapY) * (iy + 1. / 2.);

            Int_t intersect = Intersect(moduleX, moduleY, moduleDx, moduleDy, rmin);
            if (intersect == 2)
              continue;
            Double_t rHole = (intersect == 1) ? rmin : -1;
            // Skip module if not in the acceptance
            if (!Intersect(moduleX, moduleY, fActiveLx / 2., fActiveLy / 2., rmax))
              continue;
            // Create modules with positive and negative x
            for(Int_t i=0; i<2; ++i){
              TVector3 size = TVector3(fActiveLx, fActiveLy, fActiveLz);
              TVector3 pos = TVector3(TMath::Power(-1, i)*moduleX, moduleY, moduleZ + layerGlobalZ0);
              side->AddModule(new CbmMuchModuleGemRectangular(st, l, isBack, side->GetNModules(), pos, size,
                  rHole));
            }
          } // mY
        } // mX
        // Set support shape
        layer->SetSupportDx(nx * (fActiveLx + 2. * fSpacerLx));
        layer->SetSupportDy((2 * ny + 1) * (fActiveLy - fOverlapY) / 2.
            + fOverlapY / 2. + fSpacerLy);
        layer->SetSupportDz(fSupportLz[st] / 2.);
      } // end rectangular modules
    } else {
      TVector3 size = TVector3(2 * rmax, 2 * rmax, fActiveLz);
      TVector3 pos = TVector3(0, 0, layerGlobalZ0 - moduleZ);
      layer->GetSideF()->AddModule(new CbmMuchModuleGemRectangular(st, l, 0, 0, pos, size, rmin));
      layer->SetSupportDx(rmax);
      layer->SetSupportDy(rmax);
      layer->SetSupportDz(fSupportLz[st] / 2.);
    }
    station->AddLayer(layer);
  } // layers
  return station;
}


// -------------------------------------------------------------------------
CbmMuchStation* CbmMuchGeoScheme::CreateStationStraw(Int_t st){
cout<<"----------------42---------------"<<endl;
  Double_t stGlobalZ0 = fStationZ0[st] + fMuchZ1;
  Double_t stDz = ((fNlayers[st] - 1) * fLayersDz[st] + 2*fStrawLz) / 2.;
  Double_t stGlobalZ2 = stGlobalZ0 + stDz;
  Double_t rmin = stGlobalZ2 * fAcceptanceTanMin;
  Double_t rmax = stGlobalZ2 * fAcceptanceTanMax;

  CbmMuchStation* station = new CbmMuchStation(st, stGlobalZ0);
  station->SetRmin(rmin);
  station->SetRmax(rmax);
  station->SetModuleDesign(fModuleDesign[st]);

  // Create layers
  for (Int_t l = 0; l < fNlayers[st]; l++) {
    Double_t layerZ0 = (l - (fNlayers[st] - 1) / 2.) * fLayersDz[st];
    Double_t layerGlobalZ0 = layerZ0 + stGlobalZ0;
    Double_t sideDz = fStrawLz / 2. + 0.001;
    CbmMuchLayer* layer = new CbmMuchLayer(st, l, layerGlobalZ0,layerZ0);
    layer->GetSideF()->SetZ(layerGlobalZ0 - sideDz);
    layer->GetSideB()->SetZ(layerGlobalZ0 + sideDz);

    if (fModuleDesign[st]){
      Fatal("CbmMuchGeoScheme","Station %i - detailed module design not implemented for straws",st);
    }

    // Create two modules (one per each layer side)
    TVector3 posF = TVector3(0, 0, layerGlobalZ0 - sideDz);
    TVector3 posB = TVector3(0, 0, layerGlobalZ0 + sideDz);
    TVector3 size = TVector3(2 * rmax, 2 * rmax, fStrawLz);
    layer->GetSideF()->AddModule(new CbmMuchModuleStraws(st, l, 0, 0, posF, size, rmin));
    layer->GetSideB()->AddModule(new CbmMuchModuleStraws(st, l, 1, 0, posB, size, rmin));

    layer->SetSupportDx(rmax);
    layer->SetSupportDy(rmax);
    station->AddLayer(layer);
  } // layers
  return station;
}
*/
// -------------------------------------------------------------------------
Int_t CbmMuchGeoScheme::Intersect(Float_t x, Float_t y, Float_t dx, Float_t dy,
    Float_t r) {

  Float_t x1 = x - dx;
  Float_t x2 = x + dx;
  Float_t y1 = y - dy;
  Float_t y2 = y + dy;
  Int_t nCornersInside = 0;
  if (x1 * x1 + y1 * y1 < r * r)
    nCornersInside++;
  if (x2 * x2 + y1 * y1 < r * r)
    nCornersInside++;
  if (x1 * x1 + y2 * y2 < r * r)
    nCornersInside++;
  if (x2 * x2 + y2 * y2 < r * r)
    nCornersInside++;
  if (nCornersInside == 4)
    return 2;
  if (nCornersInside)
    return 1;
  if (!nCornersInside && x1 < r && y1 < 0 && y2 > 0)
    return 1;
  return 0;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
vector<CbmMuchModule*> CbmMuchGeoScheme::GetModules() const {

  vector<CbmMuchModule*> modules;
  for(Int_t iStation =0; iStation < GetNStations(); ++iStation){
    vector<CbmMuchModule*> stationModules = GetModules(iStation);
    for(vector<CbmMuchModule*>::iterator it=stationModules.begin(); it!=stationModules.end(); it++){
      CbmMuchModule* module = (*it);
      modules.push_back(module);
      assert(CbmMuchAddress::GetStationIndex(module->GetDetectorId()) == iStation);
    }
  }
  return modules;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
vector<CbmMuchModuleGem*> CbmMuchGeoScheme::GetGemModules() const {

  vector<CbmMuchModuleGem*> modules;
  for(Int_t iStation =0; iStation < GetNStations(); ++iStation){
    vector<CbmMuchModule*> stationModules = GetModules(iStation);
    for(vector<CbmMuchModule*>::iterator it=stationModules.begin(); it!=stationModules.end(); it++){
      CbmMuchModule* module = (*it);
      if (module->GetDetectorType()!=1 && module->GetDetectorType()!=3) continue;
      modules.push_back((CbmMuchModuleGem*)module);
      assert(CbmMuchAddress::GetStationIndex(module->GetDetectorId()) == iStation);
    }
  }
  return modules;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
vector<CbmMuchModule*> CbmMuchGeoScheme::GetModules(Int_t iStation) const {

  try
  {
    return fModules.at(iStation);
  }
  catch ( std::out_of_range exc)
  {
    Fatal("GetModules","No input array of stations.");
  }
  return vector<CbmMuchModule*>();

}
// -------------------------------------------------------------------------

ClassImp(CbmMuchGeoScheme)
