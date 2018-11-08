#include "CbmTrdParSetGeo.h"
#include "CbmTrdParModGeo.h"
#include "CbmTrdGeoHandler.h"

#include <FairParamList.h>
#include <FairLogger.h>

#include <TGeoVolume.h>
#include <TGeoBBox.h>
#include <TGeoNode.h>
#include <TGeoManager.h>
#include <TGeoMatrix.h>
#include <TVirtualMC.h>
#include <TGeoPhysicalNode.h>
#include <TObjArray.h>

//________________________________________________________________________________________
CbmTrdParSetGeo::CbmTrdParSetGeo(const char* name,const char* title,const char* context)
: CbmTrdParSet(name,title,context)
{
  Init();
}

//________________________________________________________________________________________
CbmTrdParSetGeo::~CbmTrdParSetGeo(void) {
}

//________________________________________________________________________________________
Bool_t CbmTrdParSetGeo::Init()
{
  CbmTrdGeoHandler geo;
  TGeoNode* topNode = gGeoManager->GetTopNode();
  TObjArray* nodes = topNode->GetNodes();
  for (Int_t iNode = 0; iNode < nodes->GetEntriesFast(); iNode++) {
    TGeoNode* node = static_cast<TGeoNode*>(nodes->At(iNode));
    if (!TString(node->GetName()).Contains("trd", TString::kIgnoreCase)) continue; // trd_vXXy top node, e.g. trd_v13a, trd_v14b
    TGeoNode* station = node;

    TObjArray* layers = station->GetNodes();
    for (Int_t iLayer = 0; iLayer < layers->GetEntriesFast(); iLayer++) {
      TGeoNode* layer = static_cast<TGeoNode*>(layers->At(iLayer));
      if (!TString(layer->GetName()).Contains("layer", TString::kIgnoreCase)) continue; // only layers

      TObjArray* modules = layer->GetNodes();
      for (Int_t iModule = 0; iModule < modules->GetEntriesFast(); iModule++) {
        TGeoNode* module = static_cast<TGeoNode*>(modules->At(iModule));
        TObjArray* parts = module->GetNodes();
        for (Int_t iPart = 0; iPart < parts->GetEntriesFast(); iPart++) {
          TGeoNode* part = static_cast<TGeoNode*>(parts->At(iPart));
          if (!TString(part->GetName()).Contains("gas", TString::kIgnoreCase)) continue; // only active gas volume

          // Put together the full path to the interesting volume, which
          // is needed to navigate with the geomanager to this volume.
          // Extract the geometry information (size, global position)
          // from this volume.
          TString path = TString("/") + topNode->GetName() + "/" + station->GetName() + "/"
            + layer->GetName() + "/" + module->GetName() + "/" + part->GetName();

          LOG(DEBUG) << "Adding detector with path " << path.Data()<< FairLogger::endl;
          // Generate a physical node which has all needed information
          gGeoManager->cd(path.Data());
          Int_t address = geo.GetModuleAddress();
          fModuleMap[address] = new CbmTrdParModGeo(Form("TRD_%d", address), path.Data());
        }
      }
    }
  }   
}

//________________________________________________________________________________________
void CbmTrdParSetGeo::Print(Option_t *) const
{
  std::map<Int_t, CbmTrdParMod*>::const_iterator imod = fModuleMap.begin(); 
  CbmTrdParModGeo *mod(NULL);
  while(imod!=fModuleMap.end()){
    mod = (CbmTrdParModGeo*)imod->second;
    printf("Module %4d [%p]:\n", imod->first, (void*)mod);
    printf("  %s[%s]\n", mod->GetName(), mod->GetPath());
    Double_t xyz[3]; mod->GetXYZ(xyz);
    printf("  X[%.2f] Y[%.2f] Z[%.2f] DX[%.2f] DY[%.2f] DZ[%.2f]\n", 
           xyz[0], xyz[1], xyz[2], mod->GetDX(), mod->GetDY(), mod->GetDZ());
    imod++;
  }
}

// //________________________________________________________________________________________
// void CbmTrdParSetGeo::putParams(FairParamList* l) 
// {
//   if (!l) return;
//   TObjArray *snodes(NULL), *pnodes(NULL);
//   //for()
//   l->addObject("FairGeoNodes Sensitive List", snodes);
//   l->addObject("FairGeoNodes Passive List", pnodes);
// }
// 
// //________________________________________________________________________________________
// Bool_t CbmTrdParSetGeo::getParams(FairParamList* l) 
// {
//   if (!l) return kFALSE;
//   TObjArray *snodes(NULL), *pnodes(NULL);
//   if (!l->fillObject("FairGeoNodes Sensitive List", snodes)) return kFALSE;
//   if (!l->fillObject("FairGeoNodes Passive List", pnodes)) return kFALSE;
//   
//   //CbmTrdParModGeo *geo(NULL);
//   for (Int_t i=0; i < snodes->GetEntriesFast(); i++){
// //     fModPar[moduleId[i]] = new CbmTrdParModGeo(GetName());
// //     fModPar
// 
//     ((FairGeoNode*)(*snodes)[i])->print();
//   }
//   return kTRUE;
// }

ClassImp(CbmTrdParSetGeo)

