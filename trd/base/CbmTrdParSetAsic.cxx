#include "CbmTrdParSetAsic.h"
#include "CbmTrdParAsic.h"
#include "CbmTrdParFasp.h"
#include "CbmTrdParSpadic.h"

#include <FairLogger.h>
#include <FairParamList.h>
#include <TArrayI.h>

using std::map;
//_______________________________________________________________________________
CbmTrdParSetAsic::CbmTrdParSetAsic(const char* name, 
           const char* title,
           const char* context)
  : CbmTrdParSet(name, title, context)
  ,fType(8)
  ,fModPar()
{
}

//_______________________________________________________________________________
CbmTrdParSetAsic::~CbmTrdParSetAsic()
{
  if(fNrOfModules){
    std::map<Int_t, CbmTrdParSetAsic*>::iterator it=fModPar.begin();
    while(it!=fModPar.end()){ delete it->second; it++;}
    fModPar.clear();
  }
}
//_______________________________________________________________________________
Bool_t CbmTrdParSetAsic::getParams(FairParamList* l) 
{
  if (!l) return kFALSE;
  if ( ! l->fill("NrOfModules", &fNrOfModules) ) return kFALSE;
  TArrayI  moduleId(fNrOfModules);
  if ( ! l->fill("ModuleId", &moduleId) ) return kFALSE;
  TArrayI  nAsic(fNrOfModules);
  if ( ! l->fill("nAsic", &nAsic) ) return kFALSE;
  TArrayI  typeAsic(fNrOfModules);
  if ( ! l->fill("typeAsic", &typeAsic) ) return kFALSE;

  CbmTrdParAsic *asic(NULL);
  Int_t address(0);
  TArrayI values(500);
  for (Int_t i=0; i < fNrOfModules; i++){
    fModPar[moduleId[i]] = new CbmTrdParSetAsic(GetName(), Form("%s for Module %d", GetTitle(), moduleId[i])/*, GetContext()*/);
    if (!l->fill(Form("%d", moduleId[i]), &values) ) continue;
    //printf("%2d modId[%d] nAsic[%d]\n", i, moduleId[i], nAsic[i]);
    for (Int_t j=0; j < nAsic[i]; j++){
      address = values[j];
      switch(typeAsic[i]){
        case 9:  asic = new CbmTrdParFasp(address); break;
        default:  asic = new CbmTrdParSpadic(address); break;
      }
      asic->LoadParams(l);
      fModPar[moduleId[i]]->SetAsicPar(address, asic);
    }
  }
  return kTRUE;
}  

//_______________________________________________________________________________
void CbmTrdParSetAsic::putParams(FairParamList *l)
{
  if (!l) return;
  LOG(INFO)<<GetName()<<"::putParams(FairParamList*)"<<FairLogger::endl;

  Int_t idx(0);
  TArrayI moduleId(fNrOfModules),
          nAsic(fNrOfModules),
          typeAsic(fNrOfModules);
  for(std::map<Int_t, CbmTrdParSetAsic*>::iterator imod=fModPar.begin(); imod!=fModPar.end(); imod++){
    moduleId[idx]   = imod->first;
    nAsic[idx]      = imod->second->GetNofAsics(); 
    typeAsic[idx++] = imod->second->GetAsicType();
  }
  l->add("NrOfModules",   fNrOfModules);
  l->add("ModuleId", moduleId);
  l->add("nAsic",    nAsic);
  l->add("typeAsic",  typeAsic);

  CbmTrdParSetAsic *mod(NULL);
  for (Int_t i=0; i < fNrOfModules; i++){
    mod = (CbmTrdParSetAsic*)fModPar[moduleId[i]];
    TArrayI asicAddress(nAsic[i]); Int_t k(0);
    for(std::map<Int_t, CbmTrdParMod*>::iterator imod=mod->fModuleMap.begin(); imod!=mod->fModuleMap.end(); imod++, k++) asicAddress[k] = imod->first; 
    l->add(Form("%d", moduleId[i]),  asicAddress);
    
    k=0;
    for(std::map<Int_t, CbmTrdParMod*>::iterator imod=mod->fModuleMap.begin(); imod!=mod->fModuleMap.end(); imod++, k++){ 
      Int_t nchannels(((CbmTrdParAsic*)imod->second)->GetNchannels());
      TArrayI chAddress(nchannels);
      if(typeAsic[i] == 9){
        CbmTrdParFasp *fasp = (CbmTrdParFasp*)imod->second;
        TArrayI PUT(nchannels), THR(nchannels), MDS(nchannels);
        for(Int_t ich(0); ich<nchannels; ich+=2){
          for(Int_t ipair(0); ipair<2; ipair++){
            chAddress[ich+ipair] = fasp->GetChannelAddress(ich);
            const CbmTrdParFaspChannel *ch = fasp->GetChannel(chAddress[ich+ipair], ipair);
            PUT[ich+ipair] = ch->GetPileUpTime();
            THR[ich+ipair] = ch->GetThreshold();
            MDS[ich+ipair] = ch->GetMinDelaySignal();
          }
        }
        l->add(Form("CHS%d", asicAddress[k]),  chAddress);
        l->add(Form("PUT%d", asicAddress[k]),  PUT);
        l->add(Form("THR%d", asicAddress[k]),  THR);
        l->add(Form("MDS%d", asicAddress[k]),  MDS);
      }
    }
  }
}

//_______________________________________________________________________________
void CbmTrdParSetAsic::addParam(CbmTrdParSetAsic *mod)
{
  //printf("CbmTrdParSetAsic::addParam() :\n");

  fModPar[mod->fModuleMap.begin()->first/1000] = mod;
  fNrOfModules++;
}

//_______________________________________________________________________________
Int_t CbmTrdParSetAsic::GetAsicAddress(Int_t chAddress) const
{
/** Query the ASICs in the module set for the specified read-out channel. 
 * Returns the id of the ASIC within the module or -1 if all returns false.   
 */
  std::map<Int_t, CbmTrdParMod*>::const_iterator it=fModuleMap.begin();
  CbmTrdParAsic *asic(NULL);
  while(it != fModuleMap.end()){
    asic = (CbmTrdParAsic*)it->second;
    if(asic->QueryChannel(chAddress)>=0) return it->first;
    it++;
  }
  return -1;
}

//_______________________________________________________________________________
void CbmTrdParSetAsic::GetAsicAddresses(std::vector<Int_t> *a) const
{
/** Query the ASICs in the module set for their addresses. 
 * Returns the list of these addresses in the vector prepared by the user   
 */
  std::map<Int_t, CbmTrdParMod*>::const_iterator it=fModuleMap.begin();
  CbmTrdParAsic *asic(NULL);
  while(it != fModuleMap.end()){
    asic = (CbmTrdParAsic*)it->second;
    a->push_back(asic->GetAddress());
    it++;
  }
}

//_______________________________________________________________________________
const CbmTrdParSet* CbmTrdParSetAsic::GetModuleSet(Int_t detId) const
{
/** Access the list of ASICs operating on detector detId. Use the class in the sense of
 * container of all module wise containers.
 */
  map<Int_t, CbmTrdParSetAsic*>::const_iterator imod = fModPar.find(detId); 
  if(imod==fModPar.end()) return NULL;
  return imod->second;
}

//_______________________________________________________________________________
void CbmTrdParSetAsic::Print(Option_t *opt) const
{
  if(fModPar.size()){
    printf(" %s Modules[%d]\n", GetName(), fNrOfModules);
    map<Int_t, CbmTrdParSetAsic*>::const_iterator imod = fModPar.begin(); 
    while(imod!=fModPar.end()){
      printf("  %d %s(%s)\n", imod->first, imod->second->GetName(), imod->second->GetTitle());
      imod->second->Print(opt);
      imod++;
    }
  } else if(fModuleMap.size()){
    printf(" %s Asics[%d]\n", GetName(), GetNofAsics());
    if(strcmp(opt, "all")==0){
      map<Int_t, CbmTrdParMod*>::const_iterator iasic = fModuleMap.begin(); 
      while(iasic!=fModuleMap.end()){
        iasic->second->Print(opt);
        iasic++;
      }
    }
  }
}

//_______________________________________________________________________________
void CbmTrdParSetAsic::SetAsicPar(Int_t address, CbmTrdParAsic *p)
{
  std::map<Int_t, CbmTrdParMod*>::iterator it=fModuleMap.find(address);
  if(it!=fModuleMap.end()){
    LOG(WARNING) << GetName() << "::SetAsicPar : The ASIC @ "<< address <<" already initialized. Skip."<< FairLogger::endl;
    return;
  }
  fModuleMap[address] = p;
  fNrOfModules++;
}

ClassImp(CbmTrdParSetAsic)
