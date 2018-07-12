#include "CbmTrdParSetGain.h"
#include "CbmTrdParModGain.h"

#include <FairParamList.h>
#include <FairLogger.h>

#include <TArrayI.h>

CbmTrdParSetGain::CbmTrdParSetGain(const char* name, 
           const char* title,
           const char* context)
  : CbmTrdParSet(name, title, context)
{

}

//_____________________________________________________________________
void CbmTrdParSetGain::putParams(FairParamList* l) 
{
  if (!l) return;
  LOG(INFO)<<GetName()<<"::putParams(FairParamList*)"<<FairLogger::endl;
  
  TArrayI moduleId(fNrOfModules); Int_t idx(0);
  for(std::map<Int_t, CbmTrdParMod*>::iterator imod=fModuleMap.begin(); imod!=fModuleMap.end(); imod++){
    moduleId[idx++]=imod->first;
  }
  l->add("NrOfModules",   fNrOfModules);
  l->add("ModuleIdArray", moduleId);
}
ClassImp(CbmTrdParSetGain)
