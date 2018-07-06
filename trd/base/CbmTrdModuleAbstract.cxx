#include "CbmTrdModuleAbstract.h"

#include "CbmTrdParModGas.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParModGain.h"

//_______________________________________________________________________________
CbmTrdModuleAbstract::CbmTrdModuleAbstract()
  : TNamed()
  ,fModuleId(0)
  ,fLayerId(-1)
  ,fRotation(0)
  ,fX(0.)
  ,fY(0.)
  ,fZ(0.)
  ,fDx(0.)
  ,fDy(0.)
  ,fDz(0.)
  ,fDigiPar(NULL)
  ,fChmbPar(NULL)
  ,fAsicPar(NULL)
  ,fGainPar(NULL)
  ,fGeoPar(NULL)
{
}

//_______________________________________________________________________________
CbmTrdModuleAbstract::CbmTrdModuleAbstract(Int_t mod, Int_t ly, Int_t rot, 
                  Double_t x, Double_t y, Double_t z, 
                  Double_t dx, Double_t dy, Double_t dz)
  : TNamed("CbmTrdModule", "Abstract TRD module implementation")
  ,fModuleId(mod)
  ,fLayerId(ly)
  ,fRotation(rot)
  ,fX(x)
  ,fY(y)
  ,fZ(z)
  ,fDx(dx)
  ,fDy(dy)
  ,fDz(dz)
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


ClassImp(CbmTrdModuleAbstract)
