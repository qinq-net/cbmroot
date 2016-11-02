// -------------------------------------------------------------------------
// -----                CbmFHodoUnpackPar source file                  -----
// -----                Created 02/11/16  by F. Uhlig                  -----
// -------------------------------------------------------------------------

#include "CbmFHodoUnpackPar.h"

#include "FairParamList.h"
#include "FairDetParIo.h"
#include "FairParIo.h"
#include "FairLogger.h"

#include "TString.h"

// -----   Standard constructor   ------------------------------------------
CbmFHodoUnpackPar::CbmFHodoUnpackPar(const char* name, 
			             const char* title,
			             const char* context)
  : FairParGenericSet(name, title, context), 
    fModuleIdArray(),
    fNrOfModules(-1)
{
  detName="FHodo";
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmFHodoUnpackPar::~CbmFHodoUnpackPar() 
{
}
// -------------------------------------------------------------------------


// -----   Public method clear   -------------------------------------------
void CbmFHodoUnpackPar::clear() 
{
  status = kFALSE;
  resetInputVersions();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------

void CbmFHodoUnpackPar::putParams(FairParamList* l) 
{
  if (!l) return;
  l->add("NrOfModules",   fNrOfModules);
  l->add("ModuleIdArray", fModuleIdArray);
}

//------------------------------------------------------

Bool_t CbmFHodoUnpackPar::getParams(FairParamList* l) {

  if (!l) return kFALSE;
  
  if ( ! l->fill("NrOfModules", &fNrOfModules) ) return kFALSE;
  
  fModuleIdArray.Set(fNrOfModules);
  if ( ! l->fill("ModuleIdArray", &fModuleIdArray) ) return kFALSE;

  return kTRUE;
}


ClassImp(CbmFHodoUnpackPar)
