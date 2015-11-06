#include "StsCosyClusterFinder.h"

#include "CbmStsDigi.h"
#include "CbmStsCluster.h"
#include "CbmStsAddress.h"
#include "CbmCluster.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairLogger.h"

#include "TClonesArray.h"
#include "TMath.h"
#include "TF1.h"
#include <iostream>

using std::map;
using std::set;
using namespace std;

// ---- Default constructor -------------------------------------------
StsCosyClusterFinder::StsCosyClusterFinder()
  :FairTask("StsCosyClusterFinder",1),
   fDigis(NULL),
   fClusters(NULL),
   finalClusters(NULL),
   fTriggeredMode(kFALSE),
   fTriggeredStation(1),
   fChargeMinStrip(),
   fChargeMaxStrip(),
   fChargeMinCluster(),
   fTimeLimit(),
   fTimeShift(),
   fCutName(""),
   cluster_size(),
   fEvent(0),
   fDigiMap()
{
  for(int i=0; i<3; i++)
  {
    fTimeLimit[i]= 40.;
    fTimeShift[i]= 0.;
    
    fChargeMinStrip[i]= 50.;
    fChargeMaxStrip[i]= 500.;
    
    fChargeMinCluster[i]= 100.;
    fCutName = "";
  }
}
// --------------------------------------------------------------------

// ---- Destructor ----------------------------------------------------
StsCosyClusterFinder::~StsCosyClusterFinder()
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
void StsCosyClusterFinder::SetParContainers()
{

  // Get Base Container
/*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();
*/  
}
// --------------------------------------------------------------------

// ---- ReInit  -------------------------------------------------------
InitStatus StsCosyClusterFinder::ReInit(){
  
/*
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb=ana->GetRuntimeDb();
*/
  
  return kSUCCESS;
}
// --------------------------------------------------------------------

// ---- Init ----------------------------------------------------------
InitStatus StsCosyClusterFinder::Init()
{

  FairRootManager *ioman = FairRootManager::Instance();
  
  fDigis =(TClonesArray *)  ioman->GetObject("StsCalibDigi");
  if ( ! fDigis ) {
    LOG(ERROR) << "No StsDigi array!" << FairLogger::endl;
    LOG(ERROR) << "Task will be inactive" << FairLogger::endl;
    return kERROR;
  }
    
  finalClusters = new TClonesArray("CbmStsCluster", 100);
  ioman->Register("StsCluster","STS",finalClusters,kTRUE);

  fClusters = new TClonesArray("CbmStsCluster", 100);
  ioman->Register("StsClusterCandidate","STS",fClusters,kTRUE);
    
  if(fCutName != ""){
    
  TFile *file = new TFile(fCutName);
    
    LOG(INFO) << "Use inpute file for time cuts " << fCutName << FairLogger::endl;
   
   TTree *sts0 = (TTree*)file->Get("STS0");
   sts0->SetBranchAddress("TimeLimit",&fTimeLimit[0]);   
   sts0->SetBranchAddress("TimeShift",&fTimeShift[0]);   
   sts0->GetEntry(0);
 
  TTree * sts1 = (TTree*)file->Get("STS1");
   sts1->SetBranchAddress("TimeLimit",&fTimeLimit[1]);   
   sts1->SetBranchAddress("TimeShift",&fTimeShift[1]);   
   sts1->GetEntry(0);
 
  TTree * sts2 = (TTree*)file->Get("STS2");
   sts2->SetBranchAddress("TimeLimit",&fTimeLimit[2]);   
   sts2->SetBranchAddress("TimeShift",&fTimeShift[2]);   
   sts2->GetEntry(0);
  }
  
   for(int i =0; i<3;i++)for(int ii =0; ii<2;ii++)
	  {
	    Char_t inName[25]; 
	    sprintf(inName,"cluster_size_STS%i_side%i",i,ii);
	    cluster_size[i][ii] = new TH1F(inName, inName, 100, -0.5, 99.5);
	  }
 
  fEvent = 0;
  
  cout << "-I- StsCosyClusterFinder: Intialisation successfull " << kSUCCESS<< endl;
 
  return kSUCCESS;
  
} 
// --------------------------------------------------------------------

// ---- Exec ----------------------------------------------------------
void StsCosyClusterFinder::Exec(Option_t *)
{
  if(fEvent%100000 == 0)cout << "-I- StsCosyClusterFinder: ----- " << fEvent << endl;
  
  fClusters->Clear();
  finalClusters->Clear();
  
  map<Int_t, set<CbmStsDigi*, classcomp1> >::iterator mapIt;
  for (mapIt=fDigiMap.begin(); mapIt!=fDigiMap.end(); mapIt++) 
    {
      ((*mapIt).second).clear();
    }
  fDigiMap.clear();
  
  set<Int_t> layerSet;
  
  CbmStsDigi* digi = NULL;
  
  map<CbmStsDigi*, Int_t> fIndices;
  
  Int_t nofDigis = fDigis->GetEntries();

  for (Int_t iDigi=0; iDigi < nofDigis; iDigi++ ) 
    {
      digi = (CbmStsDigi*) fDigis->At(iDigi);
      int station = CbmStsAddress::GetElementId(digi->GetAddress(),kStsStation);
      int side = CbmStsAddress::GetElementId(digi->GetAddress(),kStsSide);
      Int_t layer= 2*station+side;

      if(digi->GetCharge() > fChargeMinStrip[station] && digi->GetCharge() < fChargeMaxStrip[station])
      {
	layerSet.insert(layer);
	fDigiMap[layer].insert(digi);
	fIndices[digi]=iDigi;          
      }
    }
  
  set <CbmStsDigi*, classcomp1> digiSet;

  for (set<Int_t>::iterator i = layerSet.begin(); i != layerSet.end(); i++) 
    {
      digiSet = fDigiMap[*i];
      Bool_t newCluster = kTRUE;
      Int_t stripNr = -1;
      Int_t stripNrPrev = -1;
      CbmStsCluster* cluster=NULL;
      Double_t time = -1;
      Double_t timePrev = -1;

      for (set<CbmStsDigi*, classcomp1>::iterator j = digiSet.begin(); j != digiSet.end(); j++) 
	{
	  if (newCluster) 
	    {
	      Int_t size = fClusters->GetEntriesFast();
	      cluster = new ((*fClusters)[size]) CbmStsCluster();
	      Int_t index = fIndices[(*j)];
	      //----time----
	      digi = static_cast<CbmStsDigi*>(fDigis->At(index));
	      timePrev = digi->GetTime();

	      cluster->AddDigi(index);  
	      stripNrPrev = CbmStsAddress::GetElementId((*j)->GetAddress(),kStsChannel);
	      newCluster = kFALSE;
	    } 
	  else 
	    {
	      Int_t index = fIndices[(*j)];
	      digi = static_cast<CbmStsDigi*>(fDigis->At(index));
	      time = digi->GetTime();
	      stripNr = CbmStsAddress::GetElementId((*j)->GetAddress(),kStsChannel);
	      
	      int station = CbmStsAddress::GetElementId(digi->GetAddress(),kStsStation);
//	      int side = CbmStsAddress::GetElementId(digi->GetAddress(),kStsSide);
	      
	      Bool_t TrCl = kTRUE;
	      
	      if(fTriggeredMode && station==fTriggeredStation)
	      {
		if(TMath::Abs(time - timePrev) > 1)TrCl=kFALSE;
	      }
	      if (1 == stripNr-stripNrPrev && TMath::Abs(time - timePrev- fTimeShift[station])< fTimeLimit[station] && TrCl) 
		{
		  stripNrPrev = stripNr;
		  timePrev = time;
		  cluster = (CbmStsCluster*) fClusters->Last();
		  cluster->AddDigi(index);  
		} 
	      else if(TrCl)
		{
		  Int_t size = fClusters->GetEntriesFast();
		  cluster = new ((*fClusters)[size]) CbmStsCluster();
		  index = fIndices[(*j)];
                  cluster->AddDigi(index);  
		  stripNrPrev = CbmStsAddress::GetElementId((*j)->GetAddress(),kStsChannel);
		  newCluster = kFALSE;
		}
 	    }
	}
    }
  
  if(nofDigis>0)
    {
      Int_t nofClusterCandidates = fClusters->GetEntriesFast();
//      int clust[6]={0,0,0,0,0,0}; 
//      Int_t layer=0;
      for (Int_t iclus = 0; iclus < nofClusterCandidates; iclus++)
	{
	  Double_t chanNr = 0;
	  Double_t chanADC = 0.;
//	  Double_t adc = 100.;
	  Double_t sumW = 0;
	  Double_t sumWX = 0;
	  Double_t error = 0;
//	  layer=0;
	  Double_t chanNrMean = 0;
	  const CbmStsCluster* cluster = static_cast<const CbmStsCluster*>(fClusters->At(iclus));
	  Int_t nofStrips = cluster->GetNofDigis();
	  
	  //if(nofStrips > 1)continue; //produce only 1 strip cluster
	  
//	  Double_t time = -999.;
	  double mean_time = 0.;
	  
	  int station;
	  int side;
	  for(int st=0; st<nofStrips; st++)
	    {
	      const CbmStsDigi* temp_digi = (CbmStsDigi*)fDigis->At(cluster->GetDigi(st));
	      station = CbmStsAddress::GetElementId(temp_digi->GetAddress(),kStsStation);
	      side = CbmStsAddress::GetElementId(temp_digi->GetAddress(),kStsSide);
	      int ch = CbmStsAddress::GetElementId(temp_digi->GetAddress(),kStsChannel);
//	      layer = 2*station+side;
	      chanNr += ch;
	      chanADC = temp_digi->GetCharge();
	      sumW += chanADC;
	      sumWX += ch * chanADC;
	      error += (chanADC * chanADC);
	      
//	      time = temp_digi->GetTime();
	      mean_time+=temp_digi->GetTime();
	      
	      //if(chanADC > adc){adc=chanADC; mean_time=time;}
	    }
          chanNrMean = chanNr/(Double_t)nofStrips;
          if(sumW < fChargeMinCluster[station]){continue;}

	  Int_t size = finalClusters->GetEntriesFast();
	  CbmStsCluster* new_cluster = new ((*finalClusters)[size]) CbmStsCluster();
	  
	  for (Int_t i = 0; i < nofStrips; ++i)
	    {
	      Int_t digi_index=cluster->GetDigi(i);
	      const CbmStsDigi* temp_digi = static_cast<const CbmStsDigi*>(fDigis->At(digi_index));
	      if (i == 0)
		{
		  new_cluster->SetAddress(temp_digi->GetAddress());
		}
	      new_cluster->AddDigi(digi_index);
	 
	    }
	  //new_cluster->SetCentre(sumWX / sumW);
	  //new_cluster->SetProperties(sumW,chanNrMean,0.,mean_time/nofStrips);
	  //new_cluster->SetProperties(0.,0.,0.,mean_time);
	  //new_cluster->SetCentreError((1. / (sumW)) * TMath::Sqrt(error));
	  
	  new_cluster->SetProperties(sumW, chanNrMean, (1./(sumW))*TMath::Sqrt(error), mean_time/nofStrips);
	  
	 cluster_size[station][side]->Fill(new_cluster->GetNofDigis());
	}
    }
  
  fEvent++;
}
// --------------------------------------------------------------------

  // ---- Finish --------------------------------------------------------
  void StsCosyClusterFinder::Finish()
  {
        for(int i=0; i<3; i++)for(int j=0; j<2; j++)cluster_size[i][j]->Write();

  LOG(INFO) << FairLogger::endl;
  LOG(INFO) << "=====================================================" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Finish" << FairLogger::endl;
  LOG(INFO) << "=====================================================" << FairLogger::endl;
  LOG(INFO) << FairLogger::endl;
  
  }





  ClassImp(StsCosyClusterFinder)

