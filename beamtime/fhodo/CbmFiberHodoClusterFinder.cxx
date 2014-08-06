#include "CbmFiberHodoClusterFinder.h"

#include "CbmFiberHodoDigi.h"
#include "CbmFiberHodoCluster.h"
#include "CbmFiberHodoAddress.h"

#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"

#include "TClonesArray.h"
#include "TMath.h"
#include <iostream>

using std::map;
using std::set;
using namespace std;

// ---- Default constructor -------------------------------------------
CbmFiberHodoClusterFinder::CbmFiberHodoClusterFinder()
  :FairTask("FiberHodoClusterFinder",1),
   fDigis(NULL),
   fClusters(NULL),
   finalClusters(NULL)
{
}
// --------------------------------------------------------------------

// ---- Destructor ----------------------------------------------------
CbmFiberHodoClusterFinder::~CbmFiberHodoClusterFinder()
{

  if(fClusters){
    fClusters->Clear("C");
    fClusters->Delete();
    delete fClusters;
  }
  if(finalClusters){
    finalClusters->Clear("C");
    finalClusters->Delete();
    delete finalClusters;
  }
  if(fDigis){
    fDigis->Delete();
    delete fDigis;
  }

}

// ----  Initialisation  ----------------------------------------------
void CbmFiberHodoClusterFinder::SetParContainers()
{

  // Get Base Container
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();
  
}
// --------------------------------------------------------------------

// ---- ReInit  -------------------------------------------------------
InitStatus CbmFiberHodoClusterFinder::ReInit(){
  
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();
  
  return kSUCCESS;
}
// --------------------------------------------------------------------

// ---- Init ----------------------------------------------------------
InitStatus CbmFiberHodoClusterFinder::Init()
{

  FairRootManager *ioman = FairRootManager::Instance();
  
  fDigis =(TClonesArray *)  ioman->GetObject("HodoCalibDigi");
  if ( ! fDigis ) {
    LOG(ERROR) << "No FiberHodoDigi array!" << FairLogger::endl;
    LOG(ERROR) << "Task will be inactive" << FairLogger::endl;
    return kERROR;
  }
  
  finalClusters = new TClonesArray("CbmFiberHodoCluster", 100);
  ioman->Register("FiberHodoCluster","TRD",finalClusters,kTRUE);

  fClusters = new TClonesArray("CbmFiberHodoCluster", 100);
  ioman->Register("FiberHodoClusterCandidate","TRD",fClusters,kTRUE);

  return kSUCCESS;
  
} 
// --------------------------------------------------------------------

// ---- Exec ----------------------------------------------------------
void CbmFiberHodoClusterFinder::Exec(Option_t * option)
{


  fClusters->Clear();
  finalClusters->Clear();
  
  map<Int_t, set<CbmFiberHodoDigi*, classcomp> >::iterator mapIt;
  for (mapIt=fDigiMap.begin(); mapIt!=fDigiMap.end(); mapIt++) 
    {
      ((*mapIt).second).clear();
    }
  fDigiMap.clear();
  
  set<Int_t> layerSet;
  
  CbmFiberHodoDigi* digi = NULL;
  
  // sort all digis according the detector layer into different sets
  // defining a order functor for the set should result in a set which
  // is already ordered from small to big strip numbers 
 
  map<CbmFiberHodoDigi*, Int_t> fIndices;
  
  Int_t nofDigis = fDigis->GetEntries();
  
  for (Int_t iDigi=0; iDigi < nofDigis; iDigi++ ) 
    {
      // cout << " CbmFiberHodoClusterFinder:: Clear all output structures/container" << endl;
      digi = (CbmFiberHodoDigi*) fDigis->At(iDigi);
      Int_t layer = CbmFiberHodoAddress::GetLayerAddress(digi->GetAddress());
      if(digi->GetCharge() > 100)
      {
	layerSet.insert(layer);
	fDigiMap[layer].insert(digi);
	fIndices[digi]=iDigi;          
      }
    }
  set <CbmFiberHodoDigi*, classcomp> digiSet;
  
  //  cout << " loop over all detector layers and for each detector layer loop over the " << endl;
  // already sorted strips to do the cluster finding. A cluster is defined by
  // neighboring strips which are above the threshold
  // TODO: Decide if the condition of the ADC threshold should be done here or
  //       in the unpacking
  //       Check if the set is realy ordered with increasing strip number
  for (set<Int_t>::iterator i = layerSet.begin(); i != layerSet.end(); i++) 
    {
      digiSet = fDigiMap[*i];
      Bool_t   newCluster = kTRUE;
      Int_t    fiberNr = -1;
      Int_t    fiberNrPrev = -1;
      Int_t    cluster_size =0;
      Double_t time = -1;
      Double_t timePrev = -1;
      for (set<CbmFiberHodoDigi*, classcomp>::iterator j = digiSet.begin(); j != digiSet.end(); j++) 
	{
	  if (newCluster) 
	    {
	      new ((*fClusters)[fClusters->GetEntriesFast()]) 
		CbmFiberHodoCluster(fIndices[*j]);
	      fiberNrPrev = CbmFiberHodoAddress::GetStripId((*j)->GetAddress());
	      newCluster = kFALSE;
	      digi = static_cast<const CbmFiberHodoDigi*>(fDigis->At(fIndices[*j]));
	      timePrev = digi->GetTime();
	    } 
	  else 
	    {
	      // check if the next fiber is a direct neighbor of the previos one
	      fiberNr = CbmFiberHodoAddress::GetStripId((*j)->GetAddress());
	      digi = static_cast<const CbmFiberHodoDigi*>(fDigis->At(fIndices[*j]));
	      time = digi->GetTime();
	      if (1 == fiberNr-fiberNrPrev && TMath::Abs(time -timePrev)< 40 ) 
		{
		  fiberNrPrev = fiberNr;
		  CbmFiberHodoCluster* cluster = (CbmFiberHodoCluster*) fClusters->Last();
		  Int_t index = fIndices[(*j)];
                  cluster->AddDigi(index);  
		}
	      else 
		{
		  new ((*fClusters)[fClusters->GetEntriesFast()]) 
		    CbmFiberHodoCluster(fIndices[*j]);
		  fiberNrPrev = CbmFiberHodoAddress::GetStripId((*j)->GetAddress());
		  newCluster = kFALSE;
		}
 	    }
	}
    }
  if(nofDigis>0)
    {
      Int_t nofClusterCandidates = fClusters->GetEntriesFast();
      int clust[4]={0.,0.,0.,0.}; 
      for (Int_t iclus = 0; iclus < nofClusterCandidates; iclus++)
	{
	  const CbmFiberHodoCluster* cluster = static_cast<const CbmFiberHodoCluster*>(fClusters->At(iclus));
	  Int_t nofFiber = cluster->GetNofDigis();
	  const CbmFiberHodoDigi* hodoDigi = (CbmFiberHodoDigi*)fDigis->At(cluster->GetDigi(0));
	  int layer= CbmFiberHodoAddress::GetLayerAddress(hodoDigi->GetAddress());
	  int fiber = CbmFiberHodoAddress::GetStripId(hodoDigi->GetAddress());
	  if(layer>2)
	    {
	      if(layer==16)
		clust[2]++;
	      if(layer==17)
		clust[3]++;
	    }
	  else
	    clust[layer]++;
	  
	}
      
      for(Int_t iclus = 0; iclus < nofClusterCandidates; iclus++)
	{
	  const CbmFiberHodoCluster* cluster = static_cast<const CbmFiberHodoCluster*>(fClusters->At(iclus));
	  Int_t nofFiber = cluster->GetNofDigis();
	  if(nofFiber>2)
	    return;
	  double strip=0.;
	  double time=0;
	  for (Int_t i = 0; i < nofFiber; ++i)
	    {
	      const CbmFiberHodoDigi* hodoDigi = (CbmFiberHodoDigi*)fDigis->At(cluster->GetDigi(i));
	      strip += CbmFiberHodoAddress::GetStripId(hodoDigi->GetAddress());
	      time += hodoDigi->GetTime();
	    }
	  Int_t size = finalClusters->GetEntriesFast();
	  CbmFiberHodoCluster* new_cluster = new ((*finalClusters)[size]) CbmFiberHodoCluster();
	  for (Int_t i = 0; i < nofFiber; ++i)
	    {
	      const CbmFiberHodoDigi* hodo_digi = static_cast<const CbmFiberHodoDigi*>(fDigis->At(cluster->GetDigi(i)));
	      if (i == 0)
		new_cluster->SetAddress(hodo_digi->GetAddress());
	      new_cluster->AddDigi(cluster->GetDigi(i));
	    }
	  
	  new_cluster->SetMean(strip/nofFiber);
	  new_cluster->SetTime(time/nofFiber);
	  new_cluster->SetMeanError(nofFiber / TMath::Sqrt(12.));
	}  
    }
}
// --------------------------------------------------------------------

  // ---- Finish --------------------------------------------------------
  void CbmFiberHodoClusterFinder::Finish()
  {
  }





  ClassImp(CbmFiberHodoClusterFinder)

