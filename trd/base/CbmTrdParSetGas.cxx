#include "CbmTrdParSetGas.h"
#include "CbmTrdParModGas.h"

#include <FairParamList.h>
#include <FairLogger.h>

#include <TDirectory.h>
#include <TFile.h>
#include <TSystem.h>
#include <TArrayI.h>
#include <TH2F.h>

CbmTrdParSetGas::CbmTrdParSetGas(const char* name, 
           const char* title,
           const char* context)
  : CbmTrdParSet(name, title, context)
{

}


//_______________________________________________________________________________
Bool_t CbmTrdParSetGas::getParams(FairParamList* l) 
{
  if (!l) return kFALSE;
  if ( ! l->fill("NrOfModules", &fNrOfModules) ) return kFALSE;
  Text_t repo[100];
  if ( ! l->fill("Repository", repo, 100) ) return kFALSE;
  
  TDirectory *cwd(gDirectory);
  if(!TFile::Open(Form("%s/%s", gSystem->Getenv("VMCWORKDIR"), repo))){
    LOG(ERROR) << "Missing TRD DriftMap Repository : "<<repo << FairLogger::endl;
    return kFALSE;
  } else LOG(DEBUG) << "TRD DriftMap Repository : "<<gFile->GetName() << FairLogger::endl;

  
  TArrayI  moduleId(fNrOfModules);
  if ( ! l->fill("ModuleIdArray", &moduleId) ) return kFALSE;
  
  TArrayI value(2);
  for (Int_t i=0; i < fNrOfModules; i++){
    if ( ! l->fill(Form("%d", moduleId[i]), &value) ) return kFALSE;
    fModuleMap[moduleId[i]] = new CbmTrdParModGas(Form("Module/%d/Ua/%d/Ud/%d/Gas/%s", moduleId[i], value[0], value[1], "Ar"));
    ((CbmTrdParModGas*)fModuleMap[moduleId[i]])->SetDriftMap(ScanDriftMap(), cwd);
  }
  gFile->Close();
  return kTRUE;
}

//_____________________________________________________________________
void CbmTrdParSetGas::putParams(FairParamList* l) 
{
  if (!l) return;
  LOG(INFO)<<GetName()<<"::putParams(FairParamList*)"<<FairLogger::endl;
  
  TArrayI moduleId(fNrOfModules); Int_t idx(0);
  for(std::map<Int_t, CbmTrdParMod*>::iterator imod=fModuleMap.begin(); imod!=fModuleMap.end(); imod++){
    moduleId[idx++]=imod->first;
  }
  l->add("Repository",   "trd/data/gasMapAr.root");
  l->add("NrOfModules",   fNrOfModules);
  l->add("ModuleIdArray", moduleId);

  TArrayI values(2);
  CbmTrdParModGas *mod(NULL);
  for (Int_t i=0; i < fNrOfModules; i++){
    mod = (CbmTrdParModGas*)fModuleMap[moduleId[i]];
    values[0] = mod->GetUanode();
    values[1] = mod->GetUdrift();
    l->add(Form("%d", moduleId[i]), values);
  }
}
  
//_______________________________________________________________________________
TH2F* CbmTrdParSetGas::ScanDriftMap() 
{
  return (TH2F*)gFile->Get("h");
}

ClassImp(CbmTrdParSetGas)
