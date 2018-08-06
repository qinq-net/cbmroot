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
#include "CbmTrdParAsic.h"
#include "CbmTrdParModGas.h"
#include "CbmTrdParModGain.h"

#include <TStopwatch.h>
#include <TClonesArray.h>
#include <TGeoManager.h>
#include <TVector3.h>

#include <FairRootManager.h>
#include <FairLogger.h>
#include <FairRunAna.h>
#include <FairRuntimeDb.h>

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
{
}

//____________________________________________________________________________________
CbmTrdHitProducer::~CbmTrdHitProducer()
{
    fHits->Clear();
    delete fHits;
}

//____________________________________________________________________________________
Int_t CbmTrdHitProducer::AddHits(TClonesArray* hits, Bool_t moveOwner)
{  
  /** Read hits from module array and set position in the global coordinates.
   * Should also apply mis-alignment (TODO)
   *  Take away ownership of hits from module to the CbmTrdHitProducer
   */   
  
  if(!hits) return 0;
  TVector3 lpos, gpos, poserr;
  Double_t lhit[3], ghit[3], Eloss;
  Int_t nhits(0), jhits=fHits->GetEntriesFast();
  CbmTrdHit *hit(NULL), *hitSave(NULL);
  for(Int_t ihit=0; ihit<hits->GetEntriesFast(); ihit++){
    if(!(hit=(CbmTrdHit*)hits->At(ihit))) continue;
    hit->Position(lpos); lpos.GetXYZ(lhit);
    gGeoManager->LocalToMaster(lhit, ghit);
    gpos.SetXYZ(ghit[0], ghit[1], ghit[2]);
    hit->SetPosition(gpos);
    poserr[0]=hit->GetDx();
    poserr[1]=hit->GetDy();
    poserr[2]=0;
    Eloss=hit->GetELoss();
    
    hitSave = new((*fHits)[jhits++]) CbmTrdHit();
    hitSave->SetPosition(gpos);
    hitSave->SetPositionError(poserr);
    hitSave->SetELoss(Eloss);
    
    if(hit->GetMatch()!=NULL)  delete hit; //only the matches have pointers to allocated memory, so otherwise the clear does the trick
    nhits++;
  }
  hits->Clear();
  return nhits;
}

//____________________________________________________________________________________
CbmTrdModuleRec* CbmTrdHitProducer::AddModule(const CbmTrdCluster *c)
{
  // locate module
  Int_t address   = c->GetAddress();
  CbmTrdDigi *digi = (CbmTrdDigi*) fDigis->At( c->GetDigi(0) );
  
  printf("CbmTrdHitProducer::AddModule(%d) ... \n", address);
  
  CbmTrdModuleRec *module(NULL);
  if(digi->IsFlagged(CbmTrdDigi::kType)==CbmTrdDigi::kFASP){
    module = fModules[address] = new CbmTrdModuleRecT(address);//, layerId);//, orientation, x, y, z, sizeX, sizeY, sizeZ, UseFASP());
  } else {
    module = fModules[address] = new CbmTrdModuleRecR(address);// layerId);//, orientation, x, y, z, sizeX, sizeY, sizeZ);  
  }

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
  const CbmTrdParModGain *pGain(NULL);
  if(!fGainPar || !(pGain = (const CbmTrdParModGain *)fGainPar->GetModulePar(address))){
    LOG(WARNING) << GetName() << "::AddModule : No Gain params for modAddress "<< address <<". Using default."<< FairLogger::endl;
  } else module->SetGainPar(pGain);

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
    if(imod==fModules.end()) mod = AddModule(cluster);
    else mod=imod->second;

    // get digi for current cluster
    for (Int_t iDigi = 0; iDigi < cluster->GetNofDigis(); iDigi++) {
      const CbmTrdDigi* digi = static_cast<const CbmTrdDigi*>(fDigis->At(cluster->GetDigi(iDigi)));
      if (digi->GetCharge() <= 0) continue;
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
    hitCounter+=AddHits(mod->GetHits(), kTRUE);
    
  }
  // remove local data from all modules
  for(std::map<Int_t, CbmTrdModuleRec*>::iterator imod = fModules.begin(); imod!=fModules.end(); imod++) imod->second->Clear("hit");

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
}


ClassImp(CbmTrdHitProducer);
