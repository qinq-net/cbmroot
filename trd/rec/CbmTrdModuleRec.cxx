#include "CbmTrdModuleRec.h"

#include <TClonesArray.h>

//_______________________________________________________________________________
CbmTrdModuleRec::CbmTrdModuleRec()
  : CbmTrdModuleAbstract()
  ,fCalled(0)
  ,fClusters(NULL)
  ,fHits(NULL)
{
    
}

//_______________________________________________________________________________
CbmTrdModuleRec::CbmTrdModuleRec(Int_t mod, Int_t ly, Int_t rot, 
                  Double_t x, Double_t y, Double_t z, 
                  Double_t dx, Double_t dy, Double_t dz)
  : CbmTrdModuleAbstract(mod, ly, rot, x, y, z, dx, dy, dz)
  ,fCalled(0)
  ,fClusters(NULL)
  ,fHits(NULL)
{
    
}

//_______________________________________________________________________________
CbmTrdModuleRec::~CbmTrdModuleRec()
{
  if(fHits){fHits->Delete(); delete fHits;}
  if(fClusters){ fClusters->Delete(); delete fClusters;}
}

//_______________________________________________________________________________
void CbmTrdModuleRec::Clear(Option_t *opt)
{
  fCalled++;
  if(strcmp(opt, "hit")==0 && fHits) fHits->Clear();
  if(strcmp(opt, "cls")==0 && fClusters) fClusters->Clear();
}

ClassImp(CbmTrdModuleRec)