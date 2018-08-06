#include "CbmTrdClusterFinder.h"

#include "CbmTrdParSetDigi.h"
#include "CbmTrdParModDigi.h"
#include "CbmTrdParSetAsic.h"
#include "CbmTrdParSetGas.h"
#include "CbmTrdParSetGain.h"
#include "CbmTrdParAsic.h"
#include "CbmTrdParModGas.h"
#include "CbmTrdParModGain.h"

#include "CbmTrdDigi.h"
#include "CbmTrdCluster.h"
#include "CbmTrdModuleRecR.h"
#include "CbmTrdModuleRecT.h"
#include "CbmTrdGeoHandler.h"

#include <FairLogger.h>
#include <FairRootManager.h>
#include <FairRunAna.h>
#include <FairRuntimeDb.h>

#include <TStopwatch.h>
#include <TClonesArray.h>
#include <TArray.h>
#include <TBits.h>
// #include "TH2F.h"
// #include "TCanvas.h"
// #include "TImage.h"

#include <iostream>
#include <iomanip>
#include <cmath>
using std::fabs;
using std::cout;
using std::endl;
using std::setprecision;

Int_t CbmTrdClusterFinder::fgConfig = 0;
Float_t CbmTrdClusterFinder::fgMinimumChargeTH = .5e-06;
//_____________________________________________________________________
CbmTrdClusterFinder::CbmTrdClusterFinder()
  :FairTask("TrdClusterFinder",1),
   fDigis(NULL),
   fClusters(NULL),
   fGeoHandler(new CbmTrdGeoHandler()),
   fDigiMap(),
   fModuleMap(),
   fNeighbours(),
   fModDigiMap(),
   fDigiRow(),
   fDigiCol(),
   fDigiCharge(),
   fClusterBuffer(),
   fModClusterDigiMap(),
//=======================
  fModules()
  ,fAsicPar(NULL)
  ,fGasPar(NULL)
  ,fDigiPar(NULL)
  ,fGainPar(NULL)

{
}
// --------------------------------------------------------------------

// ---- Destructor ----------------------------------------------------
CbmTrdClusterFinder::~CbmTrdClusterFinder()
{

  if(fClusters){
    fClusters->Clear("C");
    fClusters->Delete();
    delete fClusters;
  }
  if(fDigis){
    fDigis->Delete();
    delete fDigis;
  }
  if(fDigiPar){
    delete fDigiPar;
  }
//   if(fModuleInfo){
//     delete fModuleInfo;
//   }

}


//_____________________________________________________________________
CbmTrdModuleRec* CbmTrdClusterFinder::AddModule(CbmTrdDigi *digi)
{
  // locate digi
  Int_t digiAddress   = digi->GetAddress();
  Int_t detId         = digi->GetAddressModule();

  CbmTrdModuleRec *module(NULL);
  if(digi->IsFlagged(CbmTrdDigi::kType)==CbmTrdDigi::kFASP){
    module = fModules[detId] = new CbmTrdModuleRecT(detId);//, orientation, x, y, z, sizeX, sizeY, sizeZ, UseFASP());
  }
  else {
    module = fModules[detId] = new CbmTrdModuleRecR(detId);//, orientation, x, y, z, sizeX, sizeY, sizeZ);  
  }
  
  //  try to load read-out parameters for module
  const CbmTrdParModDigi *pDigi(NULL);
  if(!fDigiPar || !(pDigi = (const CbmTrdParModDigi *)fDigiPar->GetModulePar(detId))){
    LOG(WARNING) << GetName() << "::AddModule : No Read-Out params for modAddress "<< detId <<". Using default."<< FairLogger::endl;
  } else module->SetDigiPar(pDigi);

  // try to load ASIC parameters for module
  CbmTrdParSetAsic *pAsic(NULL);
  if(!fAsicPar || !(pAsic = (CbmTrdParSetAsic *)fAsicPar->GetModuleSet(detId))){
    LOG(WARNING) << GetName() << "::AddModule : No ASIC params for modAddress "<< detId <<". Using default."<< FairLogger::endl;
//    module->SetAsicPar(); // map ASIC channels to read-out channels - need ParModDigi already loaded
  } else module->SetAsicPar(pAsic);

  // try to load Chamber parameters for module
  const CbmTrdParModGas *pChmb(NULL);
  if(!fGasPar || !(pChmb = (const CbmTrdParModGas *)fGasPar->GetModulePar(detId))){
    LOG(WARNING) << GetName() << "::AddModule : No Gas params for modAddress "<< detId <<". Using default."<< FairLogger::endl;
  } else module->SetChmbPar(pChmb);

  // try to load Gain parameters for module
  const CbmTrdParModGain *pGain(NULL);
  if(!fGainPar || !(pGain = (const CbmTrdParModGain *)fGainPar->GetModulePar(detId))){
    LOG(WARNING) << GetName() << "::AddModule : No Gain params for modAddress "<< detId <<". Using default."<< FairLogger::endl;
  } else module->SetGainPar(pGain);

  return module;
}

//_____________________________________________________________________
void CbmTrdClusterFinder::SetParContainers()
{
  fAsicPar = static_cast<CbmTrdParSetAsic*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetAsic"));
  fGasPar = static_cast<CbmTrdParSetGas*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetGas"));
  fDigiPar = static_cast<CbmTrdParSetDigi*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetDigi"));
  fGainPar = static_cast<CbmTrdParSetGain*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmTrdParSetGain"));
}

//_____________________________________________________________________
InitStatus CbmTrdClusterFinder::Init()
{

  FairRootManager *ioman = FairRootManager::Instance();
  
  fDigis =(TClonesArray *)  ioman->GetObject("TrdDigi");
  if ( ! fDigis ) {
    LOG(FATAL) << "No TrdDigi array found." << FairLogger::endl;
  }
  
  fClusters = new TClonesArray("CbmTrdCluster", 100);
  ioman->Register("TrdCluster","TRD",fClusters,IsOutputBranchPersistent("TrdCluster"));

  fGeoHandler->Init();
  
//   // new call needed when parameters are initialized from ROOT file
//   fDigiPar->Initialize();

  LOG(INFO) << "================ TRD Cluster Finder ===============" << FairLogger::endl;
  LOG(INFO) << " Free streaming    : " << (IsTimeBased()?"yes":"no")<< FairLogger::endl;
  LOG(INFO) << " Multi hit detect  : " << (HasMultiHit()?"yes":"no")<< FairLogger::endl;
  LOG(INFO) << " Row merger        : " << (HasRowMerger()?"yes":"no")<< FairLogger::endl;
  LOG(INFO) << " c-Neighbour enable: " << (HasNeighbourCol()?"yes":"no")<< FairLogger::endl;
  LOG(INFO) << " r-Neighbour enable: " << (HasNeighbourRow()?"yes":"no")<< FairLogger::endl;
  LOG(INFO) << " Write clusters    : " << (HasDumpClusters()?"yes":"no")<< FairLogger::endl;

  return kSUCCESS;
  
} 

//_____________________________________________________________________
void CbmTrdClusterFinder::Exec(Option_t* /*option*/)
{
/**
* Digis are sorted according to the moduleAddress. A combiId is calculted based 
* on the rowId and the colId to have a neighbouring criterion for digis within 
* the same pad row. The digis of each module are sorted according to this combiId.
* All sorted digis of one pad row are 'clustered' into rowCluster. For a new row
* the rowClusters are compared to the rowClusters of the last row. If an overlap 
* is found they are marked to be parents(last row) and childrens(activ row) 
* (mergeRowCluster()). After this, the finale clusters are created. Therefor 
* walkCluster() walks along the list of marked parents and markes every visited 
* rowCluster to avoid multiple usage of one rowCluster. drawCluster() can be used to
* get a visual output.
*/

  fClusters->Delete();
    
  TStopwatch timer;
  timer.Start();
  
  Int_t nentries = fDigis->GetEntries();  
  //printf("processing %d entries\n", nentries);
  CbmTrdModuleRec *mod(NULL);
  for (Int_t iDigi=0; iDigi < nentries; iDigi++ ) {
    CbmTrdDigi *digi = (CbmTrdDigi*) fDigis->At(iDigi);
    Int_t moduleAddress = digi->GetAddressModule();
    std::map<Int_t, CbmTrdModuleRec*>::iterator imod = fModules.find(moduleAddress);
    if(imod==fModules.end()) mod = AddModule(digi);
    else mod = imod->second;
    //cout<<iDigi<<" : "<<digi->ToString()<<endl;
    mod->AddDigi(digi, iDigi);    
  }
  
  Int_t digiCounter(0), clsCounter(0);
  for(std::map<Int_t, CbmTrdModuleRec*>::iterator imod = fModules.begin(); imod!=fModules.end(); imod++){
    mod = imod->second;
    digiCounter += mod->GetOverThreshold();
    //printf("Processing module %d digi[%d]\n", imod->first, digiCounter);
    clsCounter += mod->FindClusters();
    //printf("  clusters %d\n", clsCounter);
    AddClusters(mod->GetClusters(), kTRUE);
  }

  // remove local data from all modules
  for(std::map<Int_t, CbmTrdModuleRec*>::iterator imod = fModules.begin(); imod!=fModules.end(); imod++) imod->second->Clear("cls");
  
  timer.Stop();

  LOG(INFO) << GetName() << "::Exec : Digis    : " << nentries << " / " << digiCounter << " above threshold (" << 1e6*fgMinimumChargeTH << " keV)"<< FairLogger::endl;
  LOG(INFO) << GetName() << "::Exec : Clusters : " << clsCounter << FairLogger::endl;
  LOG(INFO) << GetName() << "::Exec : real time=" << timer.RealTime() << " CPU time=" << timer.CpuTime() << FairLogger::endl;

            //   //cout << "  " << counterI << " (" << counterI*100.0/Float_t(counterJ) << "%)" <<  " are reconstructed with fRowClusterMerger" << endl;
//   //printf("   %4d modules (%6.3f%%) are reconstructed with fRowClusterMerger\n",counterI,counterI*100/Float_t(counterJ));
//   LOG(INFO) << "CbmTrdClusterFinder::Exec : RowClusterMerger are used " << fRowMergerCounter << " times" << FairLogger::endl;
}

//_____________________________________________________________________
Int_t CbmTrdClusterFinder::AddClusters(TClonesArray *clusters, Bool_t/* move*/) 
{
  if(!clusters) return 0;
  CbmTrdCluster *cls(NULL), *clsSave(NULL);
  CbmTrdDigi *digi(NULL);
  CbmTrdParModDigi *digiPar(NULL);
  TBits cols, rows;
  Int_t ncl(fClusters->GetEntriesFast()), mcl(0), ncols(0);
  for(Int_t ic(0); ic<clusters->GetEntriesFast(); ic++){
    if(!(cls=(CbmTrdCluster*)(*clusters)[ic])) continue;

    if(!ncols){
      digiPar = (CbmTrdParModDigi*)fDigiPar->GetModulePar(cls->GetAddress());
      if(!digiPar){
        LOG(ERROR) << "CbmTrdClusterFinder::AddClusters : Can't find ParModDigi for address"<<cls->GetAddress()<<FairLogger::endl;
        continue;
      }
      ncols=digiPar->GetNofColumns();
    }
    
    cols.Clear(); rows.Clear();
    for(Int_t id=0; id<cls->GetNofDigis(); id++) {
      digi = (CbmTrdDigi*) fDigis->At( cls->GetDigi(id) );
      Int_t digiChannel = digi->GetAddressChannel();
      Int_t colId     = digiChannel % ncols;
      Int_t globalRow = digiChannel / ncols;

      Int_t combiId   = globalRow * ncols + colId;
      cols.SetBitNumber(combiId);
      rows.SetBitNumber(globalRow);
    }
    // store information in cluster
    cls->SetNCols( cols.CountBits() );
    cls->SetNRows( rows.CountBits() );
//     if(move) (*fClusters)[ncl++] = cls;
//     else{ 
      clsSave = new((*fClusters)[ncl++]) CbmTrdCluster(/**cls*/); // TODO implement copy constructor
      clsSave->SetAddress(cls->GetAddress());
      clsSave->SetDigis(cls->GetDigis());
      clsSave->SetNCols(cls->GetNCols());
      clsSave->SetNRows(cls->GetNRows());
      if(cls->GetMatch()!=NULL) delete cls; //only the matches have pointers to allocated memory, so otherwise the clear does the trick
//     }
    mcl++;
  }
  //clusters->Clear();
  return mcl;
}

//_____________________________________________________________________
void CbmTrdClusterFinder::Finish()
{

  
}

ClassImp(CbmTrdClusterFinder)

