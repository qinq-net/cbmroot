#include "CbmTrdModuleAbstract.h"

#include "CbmTrdParModGas.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParModGain.h"

//_______________________________________________________________________________
CbmTrdModuleAbstract::CbmTrdModuleAbstract()
  : TNamed()
  ,fModAddress(0)
  ,fLayerId(-1)
  ,fRotation(0)
  ,fDigiPar(NULL)
  ,fChmbPar(NULL)
  ,fAsicPar(NULL)
  ,fGainPar(NULL)
  ,fGeoPar(NULL)
{
}

//_______________________________________________________________________________
CbmTrdModuleAbstract::CbmTrdModuleAbstract(Int_t mod, Int_t ly, Int_t rot)
  : TNamed("CbmTrdModule", "Abstract TRD module implementation")
  ,fModAddress(mod)
  ,fLayerId(ly)
  ,fRotation(rot)
  ,fDigiPar(NULL)
  ,fChmbPar(NULL)
  ,fAsicPar(NULL)
  ,fGainPar(NULL)
  ,fGeoPar(NULL)
{
}

//_______________________________________________________________________________
CbmTrdModuleAbstract::~CbmTrdModuleAbstract()
{
  if(fAsicPar) delete fAsicPar;
}

//_______________________________________________________________________________
void CbmTrdModuleAbstract::LocalToMaster(Double_t in[3], Double_t out[3])
{
  if(!fGeoPar) return;
  fGeoPar->LocalToMaster(in, out);
}

ClassImp(CbmTrdModuleAbstract)
