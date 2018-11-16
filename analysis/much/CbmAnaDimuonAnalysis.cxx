/** CbmAnaDimuonAnalysis.cxx
 *@author E.Kryshen <e.kryshen@gsi.de>
 *@since 2010-01-02

modified by Partha Pratim Bhaduri 18/01/2016
changed the procedure to get the MC track id of a reconstructed track

MUCH matching is not yet done


 **/

#include "CbmAnaDimuonAnalysis.h"
#include "CbmAnaMuch.h"
#include "CbmMCTrack.h"
#include "CbmStsTrack.h"
#include "CbmTrackMatchNew.h"
#include "CbmMuchTrack.h"
#include "CbmMuchPoint.h"
#include "CbmMuchPixelHit.h"
#include "CbmMuchStrawHit.h"
#include "CbmAnaMuonCandidate.h"
#include "CbmGlobalTrack.h"
#include "CbmHit.h"
#include "CbmTrdTrack.h"
#include "CbmTrdHit.h"
#include "CbmMuchGeoScheme.h"
#include "CbmMuchPixelHit.h"
#include "CbmTofHit.h"
#include "CbmAnaDimuonCandidate.h"
#include "CbmKFTrack.h"
#include "CbmStsKFTrackFitter.h"
#include "CbmLink.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairTrackParam.h"
#include "CbmMuchCluster.h"
#include "CbmMuchDigiMatch.h"
#include "CbmVertex.h"


#include "TClonesArray.h"
#include "TMath.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TLorentzVector.h"
#include "TRandom.h"
#include "TMCProcess.h"
#include <stdlib.h>
#include <iostream>
using std::vector;
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmAnaDimuonAnalysis::CbmAnaDimuonAnalysis(TString digiFileName, Int_t nSignalPairs):
  FairTask("AnaDimuonAnalysis"),
  fEvent(0),
  fmuon(0),
  fmuonprim(0),
  fMCTracks(NULL),
   fStsTracks(NULL),         //!
   fStsPoints(NULL),         //!
   fMuchPoints(NULL),        //!
   fMuchPixelHits(NULL),     //!
   fMuchStrawHits(NULL),     //!
   fMuchTracks(NULL),        //!
   fTRDTracks(NULL),         //!
   fTRDPoints(NULL),
   fTRDTrackMatches(NULL),
   fMuchTrackMatches(NULL),  //!
   fStsTrackMatches(NULL),   //!
   fGlobalTracks(NULL),      //!
   fPixelDigiMatches(NULL),
   fStrawDigiMatches(NULL),
   fClusters(NULL),
   fTofHit(NULL),
   fTRDHit(NULL),
   fFitter(NULL),
  fStsPointsAccQuota(4),
  fStsTrueHitQuota(0.7),
  fMuchPointsAccQuota(10),
  fMuchTrueHitQuota(0.7),
  //fIsTriggerEnabled(1),  
  fChi2MuchCut(10),  
  fChi2VertexCut(3),
  fNofMuchCut(3),
  //  fNofTRDCut(2),
  fNofStsCut(4),
  fHistoFileName("histo.root"),
  fDigiFileName(digiFileName),
  fSignalPairs(nSignalPairs),
  fMuCandidates(new TClonesArray("CbmAnaMuonCandidate",1)),
  fDimuonCandidates(new TClonesArray("CbmAnaDimuonCandidate",1)),
  fVertex(NULL),
  fNGhostSTS(0),   
  fNGhostMUCHreco(0),    
  fNLayers(0),
  mapRecSts(),
  mapRecMuch(),
  mapAllMuPlus(),
  mapAccMuPlus(),
  mapRecMuPlus(),
  mapAllMuMinus(),
  mapAccMuMinus(),
  mapRecMuMinus(),
  fGeoScheme(NULL),
  fIsTriggerEnabled(kTRUE),
  //  fTrdTracks(NULL),
//  fTrdHit(NULL),
  fTriggerStationIndex(4)  
{
 }
// -------------------------------------------------------------------------


// -----  SetParContainers -------------------------------------------------
void CbmAnaDimuonAnalysis::SetParContainers(){
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();

  rtdb->getContainer("FairBaseParSet");
  rtdb->getContainer("CbmGeoPassivePar");
  rtdb->getContainer("CbmGeoStsPar");
  rtdb->getContainer("CbmGeoMuchPar");
  rtdb->getContainer("CbmFieldPar");
}
// -------------------------------------------------------------------------


// -----   Public method Init (abstract in base class)  --------------------
InitStatus CbmAnaDimuonAnalysis::Init()
{
  // Get and check FairRootManager
  FairRootManager* fManager = FairRootManager::Instance();
  fMCTracks         = (TClonesArray*) fManager->GetObject("MCTrack");
  fStsPoints        = (TClonesArray*) fManager->GetObject("StsPoint");
  fMuchPoints       = (TClonesArray*) fManager->GetObject("MuchPoint");
  fMuchPixelHits    = (TClonesArray*) fManager->GetObject("MuchPixelHit");
  fMuchStrawHits    = (TClonesArray*) fManager->GetObject("MuchStrawHit");
  fStsTracks        = (TClonesArray*) fManager->GetObject("StsTrack");
  fMuchTracks       = (TClonesArray*) fManager->GetObject("MuchTrack");
  fTRDTracks        = (TClonesArray*) fManager->GetObject("TrdTrack");
  fMuchTrackMatches = (TClonesArray*) fManager->GetObject("MuchTrackMatch");
  fStsTrackMatches  = (TClonesArray*) fManager->GetObject("StsTrackMatch");
  fGlobalTracks     = (TClonesArray*) fManager->GetObject("GlobalTrack");
  fPixelDigiMatches = (TClonesArray*) fManager->GetObject("MuchDigiMatch");
  fStrawDigiMatches = (TClonesArray*) fManager->GetObject("MuchStrawDigiMatch");
  fClusters         = (TClonesArray*) fManager->GetObject("MuchCluster");
  fTofHit           = (TClonesArray*) fManager->GetObject("TofHit");
  fTRDHit           = (TClonesArray*) fManager->GetObject("TrdHit");
  fTRDPoints        = (TClonesArray*) fManager->GetObject("TrdPoint");
  fTRDTrackMatches  = (TClonesArray*) fManager->GetObject("TrdTrackMatch");

  //fVertex           = (CbmVertex*)    fManager->GetObject("PrimaryVertex");
  // Get pointer to PrimaryVertex object from IOManager if it exists
  // The old name for the object is "PrimaryVertex" the new one
  // "PrimaryVertex." Check first for the new name
  fVertex = dynamic_cast<CbmVertex*>(fManager->GetObject("PrimaryVertex."));
  if (nullptr == fVertex) {
    fVertex = dynamic_cast<CbmVertex*>(fManager->GetObject("PrimaryVertex"));
  }

  fEvent=0;
  //if (!(fMCTracks&&fStsPoints&&fMuchPoints&&fMuchPixelHits&&fStsTracks&&fMuchTracks&&fMuchTrackMatches&&fStsTrackMatches&&fTRDHit&&fTRDPoints&&fTRDTracks)){

     if (!(fMCTracks&&fStsPoints&&fMuchPoints&&fMuchPixelHits&&fStsTracks&&fMuchTracks&&fMuchTrackMatches&&fStsTrackMatches)){
  
    printf(" %p",fStsPoints);
    printf(" %p",fMuchPoints);
    printf(" %p",fStsTracks);
    printf(" %p",fMuchPixelHits);
    printf(" %p",fMuchTracks);
    printf(" %p",fMuchTrackMatches);
    printf(" %p",fMuchPixelHits);
    printf(" %p",fMuchStrawHits);
    printf(" %p",fTRDPoints);
    printf(" %p",fTRDHit);
    printf(" %p",fTRDTracks);
    
    printf("\n");
    Fatal("Init","One of TCloneArrays not available");
  }  
  
 // fHchi2STS = new TH1D("Chi2Vertex_ghost","Chi2Vertex_ghost",200,0,100);
 // fHchi2STSmu = new TH1D("Pghost","Pghost",200,0,10);
//  fHchi2STSmu->SetLineColor(kRed);

  //fChi2 = new TH1D("fChi2","fChi2",200,0,50);
  //fChi2refit = (TH1D*)fChi2->Clone("fChi2refit");fChi2refit->SetLineColor(kRed);

  //fP_chi2Vertex = new TH2D("fP_chi2Vertex","fP_chi2Vertex",200,0,10,200,0,100);
  
 // fNGhostSTS = 0;   
 // fNGhostMUCHreco = 0;   
  
  fManager->Register("DimuonCandidates" ,"Much",fDimuonCandidates,IsOutputBranchPersistent("DimuonCandidates"));
  fManager->Register("MuCandidates" ,"Much",fMuCandidates,IsOutputBranchPersistent("MuCandidates"));
  
  fGeoScheme = CbmMuchGeoScheme::Instance();
  fGeoScheme->Init(fDigiFileName,0);
//  fLastStationIndex = fGeoScheme->GetNStations()-1;
//  fNLayers = 0;
//  for (Int_t i=0;i<=fLastStationIndex;i++){
//    fNLayers+=fGeoScheme->GetLayerSides(i).size()/2;
//  }

  fFitter = new CbmStsKFTrackFitter();
  fFitter->Init();

  cout<<" Analysis initialized successfully "<<endl;
  return kSUCCESS;
}
// -------------------------------------------------------------------------


// -----   Public method Exec   --------------------------------------------
void CbmAnaDimuonAnalysis::Exec(Option_t* opt){
  Int_t nMCTracks      = fMCTracks->GetEntriesFast();
  Int_t nStsTracks     = fStsTracks->GetEntriesFast();
  Int_t nMuchTracks    = fMuchTracks->GetEntriesFast();
  Int_t nStsPoints     = fStsPoints->GetEntriesFast();
  Int_t nMuchPoints    = fMuchPoints->GetEntriesFast();
  Int_t nMuchPixelHits = fMuchPixelHits ? fMuchPixelHits->GetEntriesFast() : 0;
  Int_t nMuchStrawHits = fMuchStrawHits ? fMuchStrawHits->GetEntriesFast() : 0;
  Int_t nGlobalTracks  = fGlobalTracks->GetEntriesFast();

  Int_t nTRDTracks=0,nTRDPoints=0,nTRDHit=0,nTRDTrackMatch=0;
  if (fTRDHit && fTRDPoints && fTRDTracks){
    nTRDTracks     = fTRDTracks->GetEntriesFast();
    nTRDPoints     = fTRDPoints->GetEntriesFast();
    nTRDHit        = fTRDHit->GetEntriesFast();
    nTRDTrackMatch = fTRDTrackMatches->GetEntriesFast();
  }
  
  Int_t nTOFHit=0;

  if (fTofHit) nTOFHit=fTofHit->GetEntriesFast();
  
  printf(" Event: %4i",fEvent++);
  cout<<" Execution startss "<<endl;
 // cout<<"Event # "<<fEvent<<endl;
  if(fEvent%500 == 0)   
{
  if (fVerbose>-1) printf(" Event: %4i",fEvent);
  if (fVerbose>0) printf(" MCtracks: %4i",nMCTracks);
  if (fVerbose>0) printf(" GlobalTracks: %4i",nGlobalTracks);
  if (fVerbose>0) printf(" StsTracks: %4i",nStsTracks);
  if (fVerbose>0) printf(" MuchTracks: %4i",nMuchTracks);
  if (fVerbose>0) printf(" MCtracks: %4i",nMCTracks);
  if (fVerbose>0) printf(" StsPoints: %4i",nStsPoints);
  if (fVerbose>0) printf(" MuchPoints: %4i",nMuchPoints);
  if (fVerbose>0) printf(" MuchPixelHits: %4i",nMuchPixelHits);
  if (fVerbose>0) printf(" MuchStrawHits: %4i",nMuchStrawHits);
  if (fVerbose>0) printf(" TrdTracks: %4i",nTRDTracks);
  if (fVerbose>0) printf(" TrdPoints: %4i",nTRDPoints);
  if (fVerbose>0) printf(" TrdHit: %4i",nTRDHit);
  
  if (fVerbose>-1) printf("\n");

}
  fMuCandidates->Clear();
  fDimuonCandidates->Clear();
  Int_t iMuCandidates = 0;
  Int_t NofTof = 0;
  Int_t NofMuTracks = 0;

  // Create array of signal dimuons
  for (Int_t iDimuon=0;iDimuon<fSignalPairs;iDimuon++){
    new((*fDimuonCandidates)[iDimuon]) CbmAnaDimuonCandidate();
  }
  

   
  // Loop over global tracks
  
  
  
  for (Int_t iTrack=0;iTrack<nGlobalTracks;iTrack++){
 
    //  cout<<" global track index "<<iTrack<<"  Event # "<<fEvent<<" global tracks "<<nGlobalTracks<<endl;
    CbmGlobalTrack* globalTrack = (CbmGlobalTrack*) fGlobalTracks->At(iTrack);
    Int_t iMuchTrack = globalTrack->GetMuchTrackIndex();
    
    Int_t iStsTrack  = globalTrack->GetStsTrackIndex();
    
   // cout<<" sts track index "<<iStsTrack<<"   much track index  "<<iMuchTrack<<endl;
    // Check much track
    if (iMuchTrack<0) continue;

    // check STS Track
    if (iStsTrack<0) continue;
    
    // cout<<" analysis starts "<<endl;

    CbmMuchTrack*  muchTrack = (CbmMuchTrack*)  fMuchTracks->At(iMuchTrack);
    if (muchTrack->GetNDF()<=0) continue;
    if(muchTrack->GetChiSq()/muchTrack->GetNDF() > fChi2MuchCut)continue;
    
    //cout<<"  step 1 "<<endl;
 // cout<<" much track hits "<<muchTrack->GetNofHits()<<endl;
    if(muchTrack->GetNofHits() < fNofMuchCut)continue;

    // cout<<"  step 2 "<<endl;

    Int_t nTriggerHits=0;
    Int_t nHits=0;
    
    for (Int_t i=0;i<muchTrack->GetNofHits();i++){
      Int_t hitIndex = muchTrack->GetHitIndex(i);
    //   cout<<" hit index "<<hitIndex<<endl;
      if (hitIndex < 0) continue;
      Int_t hitType = muchTrack->GetHitType(i);
      CbmHit* hit=NULL;
      if      (hitType==kMUCHPIXELHIT) hit = (CbmHit*) fMuchPixelHits->At(hitIndex);
      else if (hitType==kMUCHSTRAWHIT) hit = (CbmHit*) fMuchStrawHits->At(hitIndex);
      else Fatal("Exec","%i - wrong hit type, must be %i for pixel and %i for straw",hitType,kMUCHPIXELHIT,kMUCHSTRAWHIT);
      Int_t stationIndex = fGeoScheme->GetStationIndex(hit->GetAddress());
      if (stationIndex==fTriggerStationIndex) nTriggerHits++;
      nHits++;
    }
   //  cout<<" MUCH track No. "<<iMuchTrack<<"  trigger hits "<<nTriggerHits<<endl;
   
 
    CbmAnaMuonCandidate* mu;
    new((*fMuCandidates)[iMuCandidates++]) CbmAnaMuonCandidate();
    mu = (CbmAnaMuonCandidate*) (*fMuCandidates)[iMuCandidates-1];

    Int_t iTRDTrack  = globalTrack->GetTrdTrackIndex();

    if (iTRDTrack >=0){
      CbmTrdTrack*  trdTrack = (CbmTrdTrack*)  fTRDTracks->At(iTRDTrack);
    //    if(trdTrack->GetNofHits() < fNofTRDCut)continue;
    
      mu->SetTRDTrackId(iTRDTrack);
      mu->SetNTRDHits(trdTrack->GetNofHits());
      if (trdTrack->GetNDF() <=0) continue;
      mu->SetChiTRD(trdTrack->GetChiSq()/trdTrack->GetNDF());
        
    } 

 //printf("trd : hits %d chi2 %f trackid %d \n",trdTrack->GetNofHits(),trdTrack->GetChiSq()/trdTrack->GetNDF(),iTRDTrack);    
    
    mu->SetMuchTrackId(iMuchTrack);
    mu->SetNTriggerHits(nTriggerHits);
    mu->SetNMuchHits(muchTrack->GetNofHits());
    if (muchTrack->GetNDF() <=0) continue;
  //  cout<<" much normalized chi^ 2 "<<muchTrack->GetChiSq()/muchTrack->GetNDF()<<endl;
    mu->SetChiMuch(muchTrack->GetChiSq()/muchTrack->GetNDF());

    // cout<<" Now sts "<<endl;

    CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(iStsTrack);
    if(stsTrack->GetNofHits() < fNofStsCut)continue;

    mu->SetSTSTrackId(iStsTrack);
    mu->SetNStsHits(stsTrack->GetNofHits());
    if (stsTrack->GetNDF()<=0) continue;
    //cout<<" sts normalized chi^ 2 "<<stsTrack->GetChiSq()/stsTrack->GetNDF()<<endl;
    mu->SetChiSTS(stsTrack->GetChiSq()/stsTrack->GetNDF()); 


    Double_t chi = fFitter->GetChiToVertex(stsTrack);
    if(chi > fChi2VertexCut)continue;
    mu->SetChiToVertex(chi);

    //cout<<" sts track no  "<<iStsTrack<<"  chi^2 vertex  "<<chi<<endl;

    FairTrackParam par;
    fFitter->Extrapolate(stsTrack,fVertex->GetZ(),&par);
    TLorentzVector mom;
    TVector3 p;
    par.Momentum(p);
    mom.SetVectM(p,0.105658);

    mu->SetMomentumRC(mom);
    mu->SetSign(par.GetQp()>0 ? 1 : -1);
    
    //cout<<" I am here 0 "<<endl;
    // Now do the MC matching 

    // MUCH RECO to MC Matching
  /*  CbmTrackMatchNew* muchTrackMatch = (CbmTrackMatchNew*) fMuchTrackMatches->At(iMuchTrack);
    
    if(muchTrackMatch -> GetNofLinks() == 0) continue;
    
    //Int_t mcMuchTrackId = -1;
    
   // mcMuchTrackId=muchTrackMatch->(const CbmLink& GetMatchedLink().GetIndex());
     
    CbmLink ltrkmuch=muchTrackMatch->GetMatchedLink();
    Int_t mcMuchTrackId =ltrkmuch.GetIndex();*/


  
   // mu->SetMCMuchTrackId(mcMuchTrackId);
   // cout<<"TrueMu:  "<<mu->GetTrueMu()<<"  MotherPdg:  "<<mu->GetMotherPdg()<<endl;
    mu->SetTrueMu(0);
    mu->SetMotherPdg(0);
  
    /*
    if(mcMuchTrackId >= 0)
	{
	  CbmMCTrack* mctmuch = (CbmMCTrack*) fMCTracks->At(mcMuchTrackId);
	  //	  if(!mctmuch)continue;
	  TLorentzVector pMC;
	  mctmuch->Get4Momentum(pMC);
	  mu->SetMomentumMC(pMC);
	  mu->SetMCMuchPdg(mctmuch->GetPdgCode());
	  cout<<" MC Track ID "<<mcMuchTrackId<<" much  pdg  "<<mctmuch->GetPdgCode()<<endl;
	  if (mctmuch->GetGeantProcessId() == kPPrimary)
	    {
	    if(TMath::Abs(mctmuch->GetPdgCode()) == 13)
	      {
	      mu->SetTrueMu(1);
	      CbmMCTrack* mcmother = (CbmMCTrack*) fMCTracks->At(mctmuch->GetMotherId());
	      if (mcmother) mu->SetMotherPdg(mcmother->GetPdgCode());
	      cout<<"true mu  "<<mu->GetTrueMu()<<endl;
	      //cout<<" mother pdg "<<mcmother->GetPdgCode()<<endl;
	      cout<<" processor id "<<kPPrimary<<endl;
	      }
	    } 
	}
*/	   
           // }

      //}

// Now do STS RECO MC matching
  


     
    CbmTrackMatchNew* stsTrackMatch = (CbmTrackMatchNew*) fStsTrackMatches->At(iStsTrack);
    
    if(stsTrackMatch -> GetNofLinks() == 0) continue;
    
    CbmLink ltrksts=stsTrackMatch->GetMatchedLink();
    Int_t mcStsTrackId =ltrksts.GetIndex();

    //   cout<<" I am here 1 "<<endl;
    mu->SetMCSTSTrackId(mcStsTrackId);
    if(mcStsTrackId >= 0)
      {
	CbmMCTrack* mctsts = (CbmMCTrack*) fMCTracks->At(mcStsTrackId);
	if(!mctsts)continue;
        mu->SetMCStsPdg(mctsts->GetPdgCode());
	// 	printf("\n  sts pdg code %d \n",mctsts->GetPdgCode());
	TLorentzVector pMC;
	mctsts->Get4Momentum(pMC);
	mu->SetMomentumMC(pMC);
	mu->SetMCStsPdg(mctsts->GetPdgCode());

		
	
	//if (mctsts->GetGeantProcessId() == kPPrimary)
	
	if (mctsts->GetMotherId() == 0)
	  {
	 // cout<<"PDG:  "<<mctsts->GetPdgCode()<<endl;
	    if(TMath::Abs(mctsts->GetPdgCode()) == 13)
	      {
		mu->SetTrueMu(1);
		CbmMCTrack* mcmother = (CbmMCTrack*) fMCTracks->At(mctsts->GetMotherId());
		if (mcmother) mu->SetMotherPdg(mcmother->GetPdgCode());
		//cout<<"true mu  "<<mu->GetTrueMu()<<endl;
		//cout<<" mother pdg "<<mcmother->GetPdgCode()<<endl;
		cout<<" processor id "<<kPPrimary<<" mother id "<< mctsts->GetMotherId() <<endl;
	      }
      }
      }

       
	NofMuTracks++;

	//	cout<<" I am here 2 "<<endl;

	//if(globalTrack->GetTofHitIndex() < 0)continue;
	if (nTOFHit>0){
	  Int_t iTofHitIndex=globalTrack->GetTofHitIndex(); 
	    if (iTofHitIndex <0) continue;
	    CbmTofHit* th = (CbmTofHit*)fTofHit->At(iTofHitIndex);
	    if(!th)continue;
	  NofTof++;
    
	  Double_t time = th->GetTime();
	  if(time==0) continue;	
    Double_t beta = globalTrack->GetLength()*0.01/(time*1e-9*TMath::C());
    if(beta == 0)continue;

    TVector3 momL;

    FairTrackParam* stpf = (FairTrackParam*)globalTrack->GetParamLast();  
    stpf->Momentum(momL);
    mu->SetTofM(momL.Mag()*momL.Mag()*(1./beta/beta-1.));
 
	}

	//cout<<" I am here 3 "<<endl;   

    //    if(muchTrackMatch -> GetNofLinks() == 0) continue;          
    //if(stsTrackMatch -> GetNofLinks() == 0) continue;       

  }

 
  // cout<<" one loop finished "<<endl;  

  //  fNGhostSTS += NofTof;
  fNGhostMUCHreco += NofMuTracks;

 cout<<" one loop finished "<<endl; 
}
// -------------------------------------------------------------------------


// -----   Public method Finish   ------------------------------------------
void CbmAnaDimuonAnalysis::Finish(){
  //fP_chi2Vertex->Draw("colz");
  //printf("fNGhostSTS = %i\n",fNGhostSTS);
  //printf("fNGhostMUCHreco = %i\n",fNGhostMUCHreco);
  //printf("NofMuTracks = %i, NofTof = %i\n",fNGhostMUCHreco, fNGhostSTS);

  // cout<<" Total MC muons  "<<fmuon<<"    primary muons   "<<fmuonprim<<endl;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
CbmAnaMuonCandidate* CbmAnaDimuonAnalysis::GetMu(Int_t trackId){
  if (trackId<0) Fatal("GetMu","MCtrackId for signal muon < 0");
  if (trackId>=2*fSignalPairs) Fatal("GetMu","MCtrackId for signal muon > 2*dimuons");
  Int_t iDimuon = trackId/2;
  Int_t sign = trackId%2; // 0 - muPlus, 1 - muMinus
  
  return ((CbmAnaDimuonCandidate*) fDimuonCandidates->At(iDimuon))->GetMu(sign); 
}
// -------------------------------------------------------------------------

Int_t CbmAnaDimuonAnalysis::GetMCTrackId(Int_t iMuchTrack){
  //  cout<<" call function "<<"  track no  "<<iMuchTrack<<endl;
  CbmMuchTrack*  muchTrack = (CbmMuchTrack*)  fMuchTracks->At(iMuchTrack);
  std::map<Int_t, Int_t> matchMap;
  for (Int_t i=0;i<muchTrack->GetNofHits();i++){
    Int_t hitIndex = muchTrack->GetHitIndex(i);
    //  cout<<" hit index "<<hitIndex<<" total hits  "<<muchTrack->GetNofHits()<<endl; 
    Int_t hitType = muchTrack->GetHitType(i);
    //  cout<<"  Hit Type "<<hitType<<endl;
    CbmHit* hit;
    if      (hitType==kMUCHPIXELHIT) hit = (CbmHit*) fMuchPixelHits->At(hitIndex);
    else if (hitType==kMUCHSTRAWHIT) hit = (CbmHit*) fMuchStrawHits->At(hitIndex);
    else Fatal("Exec","%i - wrong hit type, must be 6 for pixel and 7 for straw",hitType);
   // Int_t stationIndex = fGeoScheme->GetStationIndex(hit->GetAddress());
     Int_t stationIndex = fGeoScheme->GetStationIndex(hit->GetRefId());
     // cout<<" station no.  "<<stationIndex<<endl;
    if (stationIndex!=fTriggerStationIndex) continue;
    if (hitType==kMUCHPIXELHIT) {
      CbmMuchPixelHit* phit = (CbmMuchPixelHit*) hit;
      Int_t clusterId = phit->GetRefId();
      //   cout<<" cluster id "<<endl;
      CbmMuchCluster* cluster = (CbmMuchCluster*) fClusters->At(clusterId);
      for (Int_t iDigi = 0; iDigi < cluster->GetNofDigis(); iDigi++){
              Int_t digiIndex = cluster->GetDigi(iDigi);
	      // cout<<" digi index  "<<digiIndex<<endl;
              DigiToTrackMatch(fPixelDigiMatches, digiIndex, matchMap);
      }
    } else{
      CbmMuchStrawHit* phit = (CbmMuchStrawHit*) hit;
      Int_t digiIndex = phit->GetRefId();
      DigiToTrackMatch(fStrawDigiMatches, digiIndex, matchMap);
    }
  }
  Int_t bestMcTrackId = -1;
  Int_t nofTrue = 0;
      for (std::map<Int_t, Int_t>::iterator it = matchMap.begin(); it != matchMap.end(); it++) {
              if (it->first != -1 && it->second > nofTrue) {
                      bestMcTrackId = it->first;
                      nofTrue = it->second;
              }
      }
      //  printf("%i\n",nofTrue);
  return bestMcTrackId;
}

void CbmAnaDimuonAnalysis::DigiToTrackMatch(
                const TClonesArray* digiMatches,
                Int_t digiIndex,
                std::map<Int_t, Int_t> &matchMap)
{
        CbmMuchDigiMatch* digiMatch = (CbmMuchDigiMatch*) digiMatches->At(digiIndex);
        if (digiMatch == NULL) return;
        for (Int_t iPoint = 0; iPoint < digiMatch->GetNofLinks(); iPoint++) {
                Int_t pointIndex = digiMatch->GetLink(iPoint).GetIndex();
		//	cout<< " point index  "<<pointIndex<<endl;
                if (pointIndex < 0) { // Fake or background hit
                        matchMap[-1]++;
                        continue;
                }
                FairMCPoint* point = (FairMCPoint*) fMuchPoints->At(pointIndex);
                if (point == NULL) continue;
                matchMap[point->GetTrackID()]++;
        }
}


ClassImp(CbmAnaDimuonAnalysis);
