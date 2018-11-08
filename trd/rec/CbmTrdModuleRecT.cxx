#include "CbmTrdModuleRecT.h"
#include "CbmTrdDigi.h"
#include "CbmTrdCluster.h"
#include "CbmTrdHit.h"

#include "TGeoPhysicalNode.h"

#include <vector>

//_______________________________________________________________________________
CbmTrdModuleRecT::CbmTrdModuleRecT()
  : CbmTrdModuleRec()
{
    
}

//_______________________________________________________________________________
CbmTrdModuleRecT::CbmTrdModuleRecT(Int_t mod, Int_t ly, Int_t rot)
  : CbmTrdModuleRec(mod, ly, rot)
{
      printf("AddModule %s\n", GetName());

}

//_______________________________________________________________________________
CbmTrdModuleRecT::~CbmTrdModuleRecT()
{
}

//_______________________________________________________________________________
Int_t CbmTrdModuleRecT::FindClusters()
{
  return 0;
}

//_______________________________________________________________________________
Bool_t CbmTrdModuleRecT::MakeHits()
{
  return kTRUE;  
}


//_______________________________________________________________________________
CbmTrdHit* CbmTrdModuleRecT::MakeHit(Int_t ic, const CbmTrdCluster *c, std::vector<const CbmTrdDigi*> *digis)
{
  return NULL;
}

ClassImp(CbmTrdModuleRecT)