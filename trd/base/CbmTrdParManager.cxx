/**
 * \file CbmTrdParManager.cxx
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \date 06/06/2008
 */

#include "CbmTrdParManager.h"

#include "CbmTrdAddress.h"
#include "CbmTrdPads.h"
//#include "CbmTrdPadsMap.h"
#include "CbmTrdGeoHandler.h"
#include "CbmTrdParSetAsic.h"
#include "CbmTrdParSetDigi.h"
#include "CbmTrdParSetGas.h"
#include "CbmTrdParSetGain.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParFasp.h"
#include "CbmTrdParSpadic.h"
#include "CbmTrdParModGas.h"
#include "CbmTrdParModGain.h"

#include <FairRootManager.h>
#include <FairRunAna.h>
#include <FairRuntimeDb.h>
#include <FairBaseParSet.h>
#include <FairLogger.h>

#include <TRandom.h>

CbmTrdParManager::CbmTrdParManager(Bool_t fasp)
  :FairTask("TrdParManager")
  ,fMaxSectors(0)
  ,fFASP(fasp)
//   fModuleMap(), 
  ,fAsicPar(NULL)
  ,fDigiPar(NULL)
  ,fGasPar(NULL)
  ,fGainPar(NULL)
  ,fGeoHandler(new CbmTrdGeoHandler())
{
  // Get the maximum number of sectors. All arrays will have this number of entries.
  fMaxSectors = fst1_sect_count;
}

CbmTrdParManager::~CbmTrdParManager()
{
}

void CbmTrdParManager::SetParContainers()
{
  FairRuntimeDb* rtdb = FairRunAna::Instance()->GetRuntimeDb();
  fAsicPar = (CbmTrdParSetAsic*)(rtdb->getContainer("CbmTrdParSetAsic"));
  fDigiPar = (CbmTrdParSetDigi*)(rtdb->getContainer("CbmTrdParSetDigi"));
  fGasPar = (CbmTrdParSetGas*)(rtdb->getContainer("CbmTrdParSetGas"));
  fGainPar = (CbmTrdParSetGain*)(rtdb->getContainer("CbmTrdParSetGain"));
}

InitStatus CbmTrdParManager::Init()
{
// The geometry structure is treelike with cave as
// the top node. For the TRD there are keeping volume
// trd_vXXy_1 which is only container for the different layers.
// The trd layer is again only a container for all volumes of this layer.
// Loop over all nodes below the top node (cave). If one of
// the nodes contains a string trd it must be TRD detector.
// Now loop over the layers and
// then over all modules of the layer to extract in the end
// all active regions (gas) of the complete TRD. For each
// of the gas volumes get the information about size and
// position from the geomanager and the sizes of the sectors
// and pads from the definitions in CbmTrdPads. This info
// is then stored in a CbmTrdModuleSim object for each of the
// TRD modules.

  TGeoNode* topNode = gGeoManager->GetTopNode();
  TObjArray* nodes = topNode->GetNodes();
  for (Int_t iNode = 0; iNode < nodes->GetEntriesFast(); iNode++) {
    TGeoNode* node = static_cast<TGeoNode*>(nodes->At(iNode));
    if (!TString(node->GetName()).Contains("trd")) continue; // trd_vXXy top node, e.g. trd_v13a, trd_v14b
    TGeoNode* station = node;
    TObjArray* layers = station->GetNodes();
    for (Int_t iLayer = 0; iLayer < layers->GetEntriesFast(); iLayer++) {
        TGeoNode* layer = static_cast<TGeoNode*>(layers->At(iLayer));
        if (!TString(layer->GetName()).Contains("layer")) continue; // only layers

        TObjArray* modules = layer->GetNodes();
        for (Int_t iModule = 0; iModule < modules->GetEntriesFast(); iModule++) {
          TGeoNode* module = static_cast<TGeoNode*>(modules->At(iModule));
          TObjArray* parts = module->GetNodes();
          for (Int_t iPart = 0; iPart < parts->GetEntriesFast(); iPart++) {
              TGeoNode* part = static_cast<TGeoNode*>(parts->At(iPart));
              if (!TString(part->GetName()).Contains("gas")) continue; // only active gas volume

              // Put together the full path to the interesting volume, which
              // is needed to navigate with the geomanager to this volume.
              // Extract the geometry information (size, global position)
              // from this volume.
              TString path = TString("/") + topNode->GetName() + "/" + station->GetName() + "/"
                + layer->GetName() + "/" + module->GetName() + "/" + part->GetName();

              CreateModuleParameters(path);
          }
        }
    }
  }

  return kSUCCESS;
}

void CbmTrdParManager::Finish()
{
  FairRuntimeDb* rtdb = FairRunAna::Instance()->GetRuntimeDb();
//   fDigiPar = (CbmTrdParSetDigi*) (rtdb->getContainer("CbmTrdParSetDigi"));
//   fDigiPar->print();
  fAsicPar = (CbmTrdParSetAsic*) (rtdb->getContainer("CbmTrdParSetAsic"));
  fAsicPar->Print();
}

void CbmTrdParManager::Exec(
      Option_t *)
{
}


void CbmTrdParManager::CreateModuleParameters(
      const TString& path)
{
/**   
 * Create TRD module parameters. Add triangular support (Alex Bercuci/21.11.2017)
*/   
 
  Int_t moduleAddress = fGeoHandler->GetModuleAddress(path);
  Int_t orientation   = fGeoHandler->GetModuleOrientation(path);

  Double_t sizeX = fGeoHandler->GetSizeX(path);
  Double_t sizeY = fGeoHandler->GetSizeY(path);
  Double_t sizeZ = fGeoHandler->GetSizeZ(path);
  Double_t x = fGeoHandler->GetX(path);
  Double_t y = fGeoHandler->GetY(path);
  Double_t z = fGeoHandler->GetZ(path);

  TArrayD sectorSizeX(fMaxSectors);
  TArrayD sectorSizeY(fMaxSectors);
  TArrayD padSizeX(fMaxSectors);
  TArrayD padSizeY(fMaxSectors);
  Int_t moduleType = fGeoHandler->GetModuleType(path);
  // special care for Bucharest module type with triangular pads
  if(moduleType<=0) moduleType=9;

  printf("\nCbmTrdParManager::CreateModuleParameters(%s) type[%d]\n", path.Data(), moduleType);
   for (Int_t i = 0; i < fst1_sect_count; i++) {
      sectorSizeX.AddAt(fst1_pad_type[moduleType - 1][i][0], i);
      sectorSizeY.AddAt(fst1_pad_type[moduleType - 1][i][1], i);
      padSizeX.AddAt(fst1_pad_type[moduleType - 1][i][2], i);
      padSizeY.AddAt(fst1_pad_type[moduleType - 1][i][3], i);
      //printf("  sec[%d] dx[%5.2f] dy[%5.2f] px[%5.2f] py[%5.2f]\n", i, sectorSizeX[i], sectorSizeY[i], padSizeX[i],padSizeY[i] );
   }

  // Orientation of the detector layers
  // Odd  layers (1,3,5..) have resolution in x-direction (isRotated == 0) - vertical pads
  // Even layers (2,4,6..) have resolution in y-direction (isRotated == 1) - horizontal pads
//   Int_t layerNr = CbmTrdAddress::GetLayerId(moduleAddress) + 1;
//   Int_t isRotated = fGeoHandler->GetModuleOrientation(path);
//      printf("layer %02d %d isRotated\n", layerNr, isRotated);   // check, if even layers are isRotated == 1
//   if( (isRotated%2) == 1 ) {  // flip pads for even layers
//      Double_t copybuf;
//      for (Int_t i = 0; i < fMaxSectors; i++) {
//         copybuf = padSizeX.At(i);
//         padSizeX.AddAt(padSizeY.At(i), i);
//         padSizeY.AddAt(copybuf, i);
//         copybuf = sectorSizeX.At(i);
//         sectorSizeX.AddAt(sectorSizeY.At(i), i);
//         sectorSizeY.AddAt(copybuf, i);
//      }
//   }

  // Create new digi par for this module
  CbmTrdParModDigi *digi = 
    new CbmTrdParModDigi(x, y, z, sizeX, sizeY, sizeZ, fMaxSectors, orientation, sectorSizeX, sectorSizeY, padSizeX, padSizeY);
  digi->SetModuleId(moduleAddress);  
  if(moduleType==9){ // anode wire geometry for the Bucharest detector
    digi->SetAnodeWireToPadPlaneDistance(0.4);
    digi->SetAnodeWireOffset(0.15);
    digi->SetAnodeWireSpacing(0.3);
  }
  digi->Print();
  fDigiPar->addParam(digi);
  
  // Create new asic par for this module
  CbmTrdParSetAsic *asics(NULL);
  if(moduleType==9 && fFASP){
    asics = new CbmTrdParSetAsic("TrdParModFasp", Form("Fasp set for Module %d", moduleAddress)/*, GetContext()*/);
    asics->SetAsicType(moduleType);
    Double_t par[6]; 
          par[1] = 14;
          par[4] =  4.181e-6;
    Int_t iasic(0),
          ncol(digi->GetNofColumns()), 
          asicAddress, chAddress;
    CbmTrdParFasp *asic(NULL);
    for(Int_t r(0); r<digi->GetNofRows(); r++){
      for(Int_t c(0); c<ncol; c++){
        if(c%8==0){
          if(asic) asics->SetAsicPar(asicAddress, asic);
          asicAddress = moduleAddress*1000+iasic;
          asic = new CbmTrdParFasp(asicAddress);
          iasic++;
        }
        for(Int_t ipair(0); ipair<2; ipair++){
          par[0] = gRandom->Gaus(300, 4);
          par[2] = gRandom->Gaus(600, 40);
          par[3] = gRandom->Gaus(2580, 10);
          chAddress = 2*(r*ncol+c)+ipair;
          par[5] = ipair;
          asic->SetChannelAddress(chAddress);
          Int_t chIdFasp = asic->QueryChannel(chAddress);
          asic->SetCalibParameters(chIdFasp, par);
        }
      }
    }
    if(asic) asics->SetAsicPar(asicAddress, asic);
  } else { // should be implemented for SPADIC
    asics = new CbmTrdParSetAsic("TrdParModSpadic", Form("Spadic set for Module %d", moduleAddress));     asics->SetAsicType(moduleType);
    CbmTrdParSpadic *asic(NULL);
    asic = new CbmTrdParSpadic(1000*moduleAddress);
    asics->SetAsicPar(1000*moduleAddress, asic);
  }
  asics->Print();
  fAsicPar->addParam(asics);

  // Create new gas par for this module
  CbmTrdParModGas *gas(NULL); 
  if(moduleType==9) gas = new CbmTrdParModGas(Form("Module/%d/Ua/%d/Ud/%d/Gas/Ar", moduleAddress, 2000, 800));
  else gas = new CbmTrdParModGas(Form("Module/%d/Ua/%d/Ud/%d/Gas/Ar", moduleAddress, 1650, 600));
  fGasPar->addParam(gas);

  // Create new gain par for this module
  CbmTrdParModGain *gain(NULL); 
  if(moduleType==9) gain = new CbmTrdParModGain();
  else gain = new CbmTrdParModGain();
  gain->SetModuleId(moduleAddress);
  fGainPar->addParam(gain);
}

// void CbmTrdParManager::FillDigiPar()
// {
//   printf("CbmTrdParManager::FillDigiPar()\n");
// //    Int_t nofModules = fModuleMap.size();
// //    fDigiPar->SetNrOfModules(nofModules);
// //    fDigiPar->SetMaxSectors(fMaxSectors);
// // 
// //    TArrayI moduleId(nofModules);
// //    Int_t iModule = 0;
// //    std::map<Int_t, CbmTrdModuleSim*>::iterator it;
// //    for (it = fModuleMap.begin() ; it != fModuleMap.end(); it++) {
// //       moduleId.AddAt(it->second->GetModuleAddress(), iModule);
// //       iModule++;
// //    }
// // 
// //    fDigiPar->SetModuleIdArray(moduleId);
// //    fDigiPar->SetModuleMap(fModuleMap);
// }

ClassImp(CbmTrdParManager)
