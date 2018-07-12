#include "CbmTrdModuleSim.h"

#include "CbmTrdParSetAsic.h"
#include "CbmTrdParModGas.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParModGain.h"
#include <FairLogger.h>
#include <vector>

//_______________________________________________________________________________
CbmTrdModuleSim::CbmTrdModuleSim()
  : CbmTrdModuleAbstract()
  ,fPointId(-1)
  ,fEventId(-1)
  ,fInputId(-1)
  ,fRadiator(NULL)
  ,fDigiMap()
  ,fBuffer()
{
  memset(fXYZ, 0, 3*sizeof(Double_t));
}

//_______________________________________________________________________________
CbmTrdModuleSim::CbmTrdModuleSim(Int_t mod, Int_t ly, Int_t rot, 
                  Double_t x, Double_t y, Double_t z, 
                  Double_t dx, Double_t dy, Double_t dz)
  : CbmTrdModuleAbstract(mod, ly, rot, x, y, z, dx, dy, dz)
  ,fPointId(-1)
  ,fEventId(-1)
  ,fInputId(-1)
  ,fRadiator(NULL)
  ,fDigiMap()
  ,fBuffer()
{
  memset(fXYZ, 0, 3*sizeof(Double_t));    
}

//_______________________________________________________________________________
CbmTrdModuleSim::~CbmTrdModuleSim()
{
  LOG(DEBUG) << GetName() << "::delete[%s]"<< GetTitle() << FairLogger::endl;
  //if(fAsicPar) delete fAsicPar;
}


ClassImp(CbmTrdModuleSim)
