#include "CbmTrdHitProducer.h"
#include "CbmTrdModuleRecR.h"
#include "CbmTrdModuleRecT.h"
#include "CbmTrdDigi.h"
#include "CbmTrdCluster.h"
#include "CbmTrdHit.h"
#include "CbmTrdAddress.h"
#include "CbmTrdParSetDigi.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParSetAsic.h"
#include "CbmTrdParSetGas.h"
#include "CbmTrdParSetGain.h"
#include "CbmTrdParSetGeo.h"
#include "CbmTrdParAsic.h"
#include "CbmTrdParModGas.h"
#include "CbmTrdParModGain.h"
#include "CbmTrdGeoHandler.h"

#include <TStopwatch.h>
#include <TClonesArray.h>
#include <TGeoManager.h>
#include <TVector3.h>
#include "TGeoPhysicalNode.h"

#include <FairRootManager.h>
#include <FairLogger.h>
#include <FairRunAna.h>
#include <FairRuntimeDb.h>

#include <map>
//____________________________________________________________________________________
CbmTrdHitProducer::CbmTrdHitProducer()
  :FairTask("TrdHitProducer")
  ,fDigis(NULL)
  ,fClusters(NULL)
  ,fHits(NULL)
  ,fModules()
  ,fAsicPar(NULL)
  ,fGasPar(NULL)
  ,fDigiPar(NULL)
  ,fGainPar(NULL)
  ,fGeoPar(NULL)
{
}

//____________________________________________________________________________________
CbmTrdHitProducer::~CbmTrdHitProducer()
{
  fHits->Clear();
  delete fHits;
  if(fGeoPar) delete fGeoPar;  
}

//____________________________________________________________________________________
Int_t CbmTrdHitProducer::AddHits(TClonesArray* hits, Bool_t moveOwner)
{  

  /** Absorb the TClonesArrays of the individual modules into the global
      TClonesArray.
   */
  
  if(!hits) return 0;

  Int_t nhits{hits->GetEntriesFast()};
  fHits->AbsorbObjects(hits);

  return nhits;
}

//____________________________________________________________________________________
CbmTrdModuleRec* CbmTrdHitProducer::AddModule(Int_t address, TGeoPhysicalNode* node)
{
  Bool_t tripad=kFALSE;
  if( TString(node->GetName()).Contains("moduleBu", TString::kIgnoreCase) ) tripad=kTRUE;

  CbmTrdModuleRec *module(NULL);
  if(tripad){
    module = fModules[address] = new CbmTrdModuleRecT(address);
  } else {
    module = fModules[address] = new CbmTrdModuleRecR(address);
  }

  // Try to load geometry parameters for the module
  const CbmTrdParModGeo* pGeo = nullptr;
  if ( ! fGeoPar ) LOG(WARNING) << GetName() << ": No geometry parameter container!"
      << FairLogger::endl;
  else pGeo = (const CbmTrdParModGeo*) fGeoPar->GetModulePar(address);
  if ( ! pGeo ) LOG(WARNING) << GetName() << ": No geometry parameters for module " << address
        << FairLogger::endl;
  else module->SetGeoPar(pGeo);


  // try to load read-out parameters for module
  const CbmTrdParModDigi *pDigi(NULL);
  if(!fDigiPar || !(pDigi = (const CbmTrdParModDigi *)fDigiPar->GetModulePar(address))){
    LOG(WARNING) << GetName() << "::AddModule : No Read-Out params for modAddress "<< address <<". Using default."<< FairLogger::endl;
  } else module->SetDigiPar(pDigi);

  // try to load ASIC parameters for module
  CbmTrdParSetAsic *pAsic(NULL);
  if(!fAsicPar || !(pAsic = (CbmTrdParSetAsic *)fAsicPar->GetModuleSet(address))){
    LOG(WARNING) << GetName() << "::AddModule : No ASIC params for modAddress "<< address <<". Using default."<< FairLogger::endl;
//    module->SetAsicPar(); // map ASIC channels to read-out channels - need ParModDigi already loaded
  } else module->SetAsicPar(pAsic);

  // try to load Chamber parameters for module
  const CbmTrdParModGas *pChmb(NULL);
  if(!fGasPar || !(pChmb = (const CbmTrdParModGas *)fGasPar->GetModulePar(address))){
    LOG(WARNING) << GetName() << "::AddModule : No Gas params for modAddress "<< address <<". Using default."<< FairLogger::endl;
  } else module->SetChmbPar(pChmb);

  // try to load Gain parameters for module
  if(tripad){
    const CbmTrdParModGain *pGain(NULL);
    if(!fGainPar || !(pGain = (const CbmTrdParModGain *)fGainPar->GetModulePar(address))){
      LOG(WARNING) << GetName() << "::AddModule : No Gain params for modAddress "<< address <<". Using default."<< FairLogger::endl;
    } else module->SetGainPar(pGain);
  }
  return module;
}

//____________________________________________________________________________________
InitStatus CbmTrdHitProducer::Init()
{
   FairRootManager* rootMgr = FairRootManager::Instance();
   if (NULL == rootMgr) {
      LOG(ERROR) << GetName() << "::Init: " << "ROOT manager is not instantiated!" << FairLogger::endl;
      return kFATAL;
   }

   fDigis = (TClonesArray *) rootMgr->GetObject("TrdDigi");
   if (!fDigis){ 
     LOG(FATAL) <<  GetName() << "::Init No TrdDigi array." << FairLogger::endl;
     return kFATAL;
   }
   
   fClusters = (TClonesArray*) rootMgr->GetObject("TrdCluster");
   if (!fClusters) {
     LOG(ERROR) << GetName() << "::Init: " << "no TrdCluster array!" << FairLogger::endl;
     return kFATAL;
   }
 
   fHits = new TClonesArray("CbmTrdHit", 100);
   rootMgr->Register("TrdHit", "TRD", fHits, IsOutputBranchPersistent("TrdHit"));
  
   // Get the full geometry information of the detector gas layers and store
   // them with the CbmTrdModuleRec. This information can then be used for
   // transformation calculations 
   CbmTrdGeoHandler geoHandler;
   std::map<Int_t, TGeoPhysicalNode*> moduleMap = geoHandler.FillModuleMap();

   Int_t nrModules = fDigiPar->GetNrOfModules();
   Int_t nrNodes = moduleMap.size();
   if (nrModules != nrNodes) LOG(FATAL) << "Geometry and parameter files have different number of modules.";
   for (Int_t loop=0; loop< nrModules; ++loop) {
      Int_t address = fDigiPar->GetModuleId(loop);
      std::map<Int_t, TGeoPhysicalNode*>::iterator it = moduleMap.find(address);
      if ( it  == moduleMap.end() ) {
        LOG(FATAL) << "Expected module with address " << address << " wasn't found in the map with TGeoNode information.";
      }
      AddModule(address, it->second);
   }
   return kSUCCESS;
}

//____________________________________________________________________________________
void CbmTrdHitProducer::Exec(Option_t*)
{
  fHits->Delete();

  TStopwatch timer;
  timer.Start();

  CbmTrdModuleRec *mod(NULL);
  std::vector<const CbmTrdDigi*> digis;
  Int_t nofCluster = fClusters->GetEntries();  
  for (Int_t iCluster = 0; iCluster < nofCluster; iCluster++) {

    const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*>(fClusters->At(iCluster));
    //    if(!cluster) continue;
    
    // get/build module for current cluster
    std::map<Int_t, CbmTrdModuleRec*>::iterator imod = fModules.find(cluster->GetAddress()); 
    mod=imod->second;

    // get digi for current cluster
    for (Int_t iDigi = 0; iDigi < cluster->GetNofDigis(); iDigi++) {
      const CbmTrdDigi* digi = static_cast<const CbmTrdDigi*>(fDigis->At(cluster->GetDigi(iDigi)));
      if (digi->GetType()==CbmTrdDigi::kSPADIC && digi->GetCharge() <= 0) continue;
      digis.push_back(digi);
    }
    
    // run hit reconstruction
    //    std::cout<<" make hit"<<std::endl;
    mod->MakeHit(iCluster, cluster, &digis);
    digis.clear();
  }
  
  Int_t hitCounter(0);
  for(std::map<Int_t, CbmTrdModuleRec*>::iterator imod = fModules.begin(); imod!=fModules.end(); imod++){
    mod = imod->second;
    //hitCounter += mod->GetNhits();

    mod->Finalize();
    //    std::cout<<" add hit"<<std::endl;
    hitCounter+=AddHits(mod->GetHits(), kTRUE);
    
  }
  // remove local data from all modules
  // Not needed any longer since AbsorbObjects take care
//  for(std::map<Int_t, CbmTrdModuleRec*>::iterator imod = fModules.begin(); imod!=fModules.end(); imod++) imod->second->Clear("hit");

  timer.Stop();
  LOG(INFO) << GetName() << "::Exec: " << " Clusters : " << fClusters->GetEntriesFast() << FairLogger::endl;
  LOG(INFO) << GetName() << "::Exec: " << " Hits     : " << hitCounter << FairLogger::endl;
  LOG(INFO) << GetName() << "::Exec: real time=" << timer.RealTime() << " CPU time=" << timer.CpuTime() << FairLogger::endl;
}

//____________________________________________________________________________________
void CbmTrdHitProducer::Finish()
{

}

//________________________________________________________________________________________
void CbmTrdHitProducer::SetParContainers()
{
  fAsicPar = static_cast<CbmTrdParSetAsic*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetAsic"));
  fGasPar = static_cast<CbmTrdParSetGas*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetGas"));
  fDigiPar = static_cast<CbmTrdParSetDigi*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetDigi"));
  fGainPar = static_cast<CbmTrdParSetGain*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetGain"));
  fGeoPar = new CbmTrdParSetGeo();
}


ClassImp(CbmTrdHitProducer);
