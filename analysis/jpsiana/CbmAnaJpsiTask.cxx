
#include "CbmAnaJpsiTask.h"
#include <iostream>
#include "TCanvas.h"
#include "CbmMCTrack.h"
#include "CbmStsTrack.h"
#include "CbmRichRing.h"
#include "CbmTrdTrack.h"
#include "CbmDrawHist.h"
#include "CbmStsHit.h"
#include "CbmRichHit.h"
#include "CbmRichPoint.h"
#include "CbmTrdHit.h"
#include "CbmTofHit.h"
#include "CbmHistManager.h"
#include "TFile.h"
#include "TMath.h"
#include "CbmGlobalTrack.h"
#include "CbmAnaJpsiCandidate.h"
#include "CbmAnaJpsiUtils.h"
#include "CbmTrackMatchNew.h"


using namespace std;

CbmAnaJpsiTask::CbmAnaJpsiTask()
  : FairTask("CbmAnaJpsiTask"),
  fEventNum(0)

{
}

CbmAnaJpsiTask::~CbmAnaJpsiTask()
{

}

InitStatus CbmAnaJpsiTask::Init()
{
   //cout << "CbmRichUrqmdTest::Init"<<endl;
   FairRootManager* ioman = FairRootManager::Instance();
   if (NULL == ioman) { Fatal("CbmAnaJpsiTask::Init","RootManager not instantised!"); }
   
   fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
   if ( NULL == fMcTracks) {Fatal("CbmAnaJpsiTask::Init","No MCtrack Array! "); }
   
   fStsPoints = (TClonesArray*) ioman->GetObject("StsPoint");
   if ( NULL == fStsPoints) {Fatal("CbmAnaJpsiTask::Init","No StsPoint Array! "); }
   
   fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
   if ( NULL == fRichPoints) {Fatal("CbmAnaJpsiTask::Init","No RichPoint Array! "); }
   
   fTrdPoints = (TClonesArray*) ioman->GetObject("TrdPoint");
   if ( NULL == fTrdPoints) {Fatal("CbmAnaJpsiTask::Init","No TrdPoint Array! "); }
   
   fTofPoints = (TClonesArray*) ioman->GetObject("TofPoint");
   if ( NULL == fTofPoints) {Fatal("CbmAnaJpsiTask::Init","No TofPoint Array! "); }
   
   fStsHits = (TClonesArray*) ioman->GetObject("StsHit");
   if ( NULL == fStsHits) {Fatal("CbmAnaJpsiTask::Init","No StsHit Array! "); }
   
   fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
   if ( NULL == fRichHits) {Fatal("CbmAnaJpsiTask::Init","No RichHits Array! ");}
   
   fTrdHits = (TClonesArray*) ioman->GetObject("TrdHit");
   if ( NULL == fTrdHits) {Fatal("CbmAnaJpsiTask::Init","No TrdHits Array! ");}
   
   fTofHits = (TClonesArray*) ioman->GetObject("TofHit");
   if ( NULL == fTofHits) {Fatal("CbmAnaJpsiTask::Init","No TofHits Array! ");}
   
	fTrdTrackMatches = (TClonesArray*) ioman->GetObject("TrdTrackMatch");
	if (NULL == fTrdTrackMatches) { Fatal("CbmAnaDielectronTask::Init","No TrdTrackMatch array!"); }

   fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
   if ( NULL == fStsTracks) {Fatal("CbmAnaJpsiTask::Init","No StsTracks Array! ");}
   
   fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
   if ( NULL == fRichRings) {Fatal("CbmAnaJpsiTask::Init","No RichRings Array! ");}
   
   fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
   if (NULL == fRichRingMatches) { Fatal("CbmAnaJpsiTask::Init","No RichRingMatch array!"); }

   fTrdTracks = (TClonesArray*) ioman->GetObject("TrdTrack");
   if ( NULL == fTrdTracks ) {Fatal("CbmAnaJpsiTask::Init","No TrdTracks Array!");}
   
   fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
   if ( NULL == fGlobalTracks ) {Fatal("CbmAnaJpsiTask::Init","No GlobalTracks Array!");}

   fPrimVertex = (CbmVertex*) ioman->GetObject("PrimaryVertex");
   if (NULL == fPrimVertex) { Fatal("CbmAnaJpsiTask::Init","No PrimaryVertex array!"); }

   fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
   if ( NULL == fStsTrackMatches ) {Fatal("CbmAnaJpsiTask::Init","No StsTrackMatches Array!");}

   InitHist();
   return kSUCCESS;
}

void CbmAnaJpsiTask::InitHist()
{
	fHM = new CbmHistManager();
	//Mom / Pt
   fHM->Create1<TH1D>("fhMcMomAll","fhMcMomAll;Momentum [GeV/c];Entries",150,0,15);
   fHM->Create1<TH1D>("fhMcMomElectrons","fhMcMomElectrons;Momentum [GeV/c];Entries",150,0,15);
   fHM->Create2<TH2D>("fhMcPtYElectrons","fhMcPtYElectrons;Pt [GeV/c];Rapidity;Entries",40,0,4,40,0,4);
   //PDGCode
   fHM->Create1<TH1D>("fhPdgCodeAll","fhPdgCodeAll;PdgCode of participating particles; Entries",2000,-1000,1000);

   //NofHits in ...
   fHM->Create1<TH1D>("fhNofStsHitsInEvent","fhNofStsHitsInEvent;Number of StsHits per Event;Entries",100,0,25000);
   fHM->Create1<TH1D>("fhNofRichHitsInEvent","fhNofRichHitsInEvent;Number of RichHits per Event;Entries",100,0,2000);
   fHM->Create1<TH1D>("fhNofTrdHitsInEvent","fhNofTrdHitsInEvent;Number of TrdHits per Event;Entries",100,0,15000);
   fHM->Create1<TH1D>("fhNofTofHitsInEvent","fhNofTofHitsInEvent;Number of TofHits per Event;Entries",100,0,3000);

   //NofTracks/Rings
   fHM->Create1<TH1D>("fhNofStsTracksInEvent","fhNofStsTracksInEvent;Number of StsTracks per Event;Entries",90,0,900);
   fHM->Create1<TH1D>("fhNofRichRingsInEvent","fhNofRichRingsInEvent;Number of RichRings per Event;Entries",70,0,70);
   fHM->Create1<TH1D>("fhNofTrdTracksInEvent","fhNofTrdTracksInEvent;Number of TrdTracks per Event;Entries",70,0,700);

   //HitsInPlane
   fHM->Create2<TH2D>("fhStsHitXY","fhStsHitXY;X [cm];Y [cm];Entries",100,-50,50,100,-50,50);
   fHM->Create2<TH2D>("fhRichHitXY","fhRichHitXY;X [cm];Y [cm];Entries",200,-100,100,400,-200,200);
   fHM->Create2<TH2D>("fhTrdHitXY","fhTrdHitXY;X [cm];Y [cm];Entries",1000,-500,500,1000,-500,500);
   fHM->Create2<TH2D>("fhTofHitXY","fhTofHitXY;X [cm];Y [cm];Entries",1000,-500,500,1000,-500,500);
   fHM->Create1<TH1D>("fhAllDetektorHitsInZPosition","fhAllDetektorHitsInZPosition;Z [cm];Entries",1100,0,1100);

   //NofTracks
   fHM->Create1<TH1D>("fhNofMcTracksInEvent","fhNofMcTracksInEvent;Number of MC Tracks per Event;Entries",200,0,25000);

   //NofPoints in ...
   fHM->Create1<TH1D>("fhNofStsPointInEvent","fhNofStsPointInEvent;Number of StsPoints per Event;Entries",200,0,10000);
   fHM->Create1<TH1D>("fhNofRichPointInEvent","fhNofRichPointInEvent;Number of RhichPoints per Event;Entries",200,0,20000);
   fHM->Create1<TH1D>("fhNofTrdPointInEvent","fhNofTrdPointInEvent;Number of TrdPoints per Event;Entries",200,0,15000);
   fHM->Create1<TH1D>("fhNofTofPointInEvent","fhNofTofPointInEvent;Number of TofPoints per Event;Entries",200,0,20000);

   //signal electrons
   fHM->Create1<TH1D>("fhSignalElectronMom","fhSignalElectronMom;P [GeV/c];Entries",150,0,15);
   fHM->Create1<TH1D>("fhSignalElectronPt","fhSignalElectronPt;Pt [GeV/c];Entries",40,0,4);
   fHM->Create1<TH1D>("fhSignalMotherPdgCode","fhSignalMotherPdgCode;PdgCode;Entries",28,-14,14);
   fHM->Create2<TH2D>("fhRichPointSignalElectronPMTPlane","fhRichPointSignalElectronPMTPlane;X[cm];Y[cm];Entries",220,-110,110,400,-200,200);
   fHM->Create2<TH2D>("fhRichHitSignalElectronPMTPlane","fhRichHitSignalElectronPMTPlane;X[cm];Y[cm];Entries",220,-110,110,400,-200,200);


   //conversion electrons
   fHM->Create1<TH1D>("fhGammaConvElectronMom","fhGammaConvElectronMom;P [GeV/c];Entries",150,0,15);
   fHM->Create1<TH1D>("fhGammaConvElectronPt","fhGammaConvElectronPt;P [GeV/c];Entries",25,0,2.5);
   fHM->Create2<TH2D>("fhGammaConvElectronStartVertXY","fhGammaConvElectronStartVert;X[cm];Y[cm];Entries",600,-300,300,600,-300,300);
   fHM->Create2<TH2D>("fhGammaConvElectronStartVertZX","fhGammaConvElectronStartVert;Z[cm];X[cm];Entries",1100,0,1100,400,-200,200);
   fHM->Create2<TH2D>("fhGammaConvElectronStartVertZY","fhGammaConvElectronStartVert;Z[cm];Y[cm];Entries",1100,0,1100,400,-200,200);
   fHM->Create2<TH2D>("fhGammaConvElectronStartVertZSqrt(X2+Y2)","fhGammaConvElectronStartVertZSqrt(X2+Y2);Z[cm];Sqrt(X²+Y²)[cm];Entries",1100,0,1100,600,0,600);

   //Dalitz
   fHM->Create1<TH1D>("fhpi0DalitzDecayMomentum","fhpi0DalitzDecayMomentum;P [GeV/c];Entries",140,0,14);

   //Photons from Gamma And Pi0 PMTPlane | RichPoint
   fHM->Create2<TH2D>("fhRichPointGammaAndPi0PMTPlane","fhRichPointGammaAndPi0PMTPlane;X[cm];Y[cm];Entries",220,-110,110,400,-200,200);

   //GammaConversion Photon in PMTPlane XY | RichPoint
   fHM->Create2<TH2D>("fhRichPointGammaConversionPhotonInPETPlaneXY","fhRichPointGammaConversionPhotonInPETPlaneXY;X[cm];Y[cm];Entries",220,-110,110,400,-200,200);

   //Dalitz Decay in PMTPlane XY | RichPoint
   fHM->Create2<TH2D>("fhRichPointDalitzDecayInPETPlaneXY","fhRichPointDalitzDecayInPETPlaneXY;X[cm];Y[cm];Entries",220,-110,110,400,-200,200);


   //GammaConversion Photon in PMTPlane XY | RichHit
   fHM->Create2<TH2D>("fhRichHitGammaConversionPhotonInPETPlaneXY","fhRichHitGammaConversionPhotonInPETPlaneXY;X[cm];Y[cm];Entries",220,-110,110,400,-200,200);

   //Dalitz Decay in PMTPlane XY | RichHit
   fHM->Create2<TH2D>("fhRichHitDalitzDecayInPETPlaneXY","fhRichHitDalitzDecayInPETPlaneXY;X[cm];Y[cm];Entries",220,-110,110,400,-200,200);

   //reconstructed momenta
   fHM->Create1<TH1D>("fhReconMomSignalElectron","fhReconMomSignalElectron;P [GeV/c];Entries",150,0,15);
   fHM->Create1<TH1D>("fhReconMomGammaConv","fhReconMomGammaConv;P [GeV/c];Entries",150,0,15);
   fHM->Create1<TH1D>("fhReconMomDalitzDecay","fhReconMomDalitzDecay;P [GeV/c];Entries",150,0,15);
}


void CbmAnaJpsiTask::Exec(
      Option_t* option)
{
   fEventNum++;
   cout << "CbmAnaJpsiTask, event No. " <<  fEventNum << endl;
   
   if (fPrimVertex != NULL){
      fKFVertex = CbmKFVertex(*fPrimVertex);
   } else {
      Fatal("CbmAnaJpsiTask::Exec","No PrimaryVertex array!");
   }

   Int_t nofMcTracks = fMcTracks->GetEntriesFast();
   //fhNofMcTracksInEvent->Fill(nofMcTracks);
   fHM->H1("fhNofMcTracksInEvent")->Fill(nofMcTracks);
   
   for (Int_t i=0; i<nofMcTracks; i++)
   {
      CbmMCTrack* track = (CbmMCTrack*)fMcTracks->At(i);
      Double_t McMomenta = track->GetP();
      //fhMcMomAll->Fill(McMomenta);
      fHM->H1("fhMcMomAll")->Fill(McMomenta);
      
      Int_t pdgcode = track->GetPdgCode();
      Int_t MotherId = track->GetMotherId();
      //fhPdgCodeAll->Fill(pdgcode);
      fHM->H1("fhPdgCodeAll")->Fill(pdgcode);
       
      if ( TMath::Abs(pdgcode)==11 && MotherId==-1) 
      {
    	  //fhMcMomElectrons->Fill(McMomenta);
    	  fHM->H1("fhMcMomElectrons")->Fill(McMomenta);
    	  Double_t McPt = track->GetPt();
    	  Double_t McY = track->GetRapidity();
    	  fHM->H2("fhMcPtYElectrons")->Fill(McPt,McY);
      }
      
   } 
   
   
   Int_t nofStsPoint = fStsPoints->GetEntriesFast();
   fHM->H1("fhNofStsPointInEvent")->Fill(nofStsPoint);
   
   Int_t nofRichPoint = fRichPoints->GetEntriesFast();
   fHM->H1("fhNofRichPointInEvent")->Fill(nofRichPoint);
   
   Int_t nofTrdPoint = fTrdPoints->GetEntriesFast();
   fHM->H1("fhNofTrdPointInEvent")->Fill(nofTrdPoint);
   
   Int_t nofTofPoint = fTofPoints->GetEntriesFast();
   fHM->H1("fhNofTofPointInEvent")->Fill(nofTofPoint);
   
   Int_t nofStsHit = fStsHits->GetEntriesFast();
   fHM->H1("fhNofStsHitsInEvent")->Fill(nofStsHit);
   
   Int_t nofRichHit = fRichHits->GetEntriesFast();
   fHM->H1("fhNofRichHitsInEvent")->Fill(nofRichHit);
   
   Int_t nofTrdHit = fTrdHits->GetEntriesFast();
   fHM->H1("fhNofTrdHitsInEvent")->Fill(nofTrdHit);
   
   Int_t nofTofHit = fTofHits->GetEntriesFast();
   fHM->H1("fhNofTofHitsInEvent")->Fill(nofTofHit);
   
   Int_t nofStsTracks = fStsTracks->GetEntriesFast();
   fHM->H1("fhNofStsTracksInEvent")->Fill(nofStsTracks);
   
   Int_t nofRichRings = fRichRings->GetEntriesFast();
   fHM->H1("fhNofRichRingsInEvent")->Fill(nofRichRings);
   
   Int_t nofTrdTracks = fTrdTracks->GetEntriesFast();
   fHM->H1("fhNofTrdTracksInEvent")->Fill(nofTrdTracks);
 
   
   for (Int_t i=0;i<nofStsHit;i++)
   {
     CbmStsHit* StsHit = (CbmStsHit*)fStsHits->At(i);
     Double_t StsHitXPos = StsHit->GetX();
     Double_t StsHitYPos = StsHit->GetY();
     
     TVector3 vPos;
     StsHit->Position(vPos);
     vPos.Z();
     
     fHM->H2("fhStsHitXY")->Fill(StsHitXPos,StsHitYPos);

     fHM->H1("fhAllDetektorHitsInZPosition")->Fill(vPos.Z());
  }
  
   for (Int_t i=0;i<nofRichHit;i++)
   {
     CbmRichHit* RichHit = (CbmRichHit*)fRichHits->At(i);
     Double_t RichHitXPos = RichHit->GetX();
     Double_t RichHitYPos = RichHit->GetY();
     
     
     TVector3 vPos;
     RichHit->Position(vPos);
     vPos.Z();
     
     fHM->H2("fhRichHitXY")->Fill(RichHitXPos,RichHitYPos);
     fHM->H1("fhAllDetektorHitsInZPosition")->Fill(vPos.Z());
  }
  
  for (Int_t i=0;i<nofTrdHit;i++)
   {
     CbmTrdHit* TrdHit = (CbmTrdHit*)fTrdHits->At(i);
     Double_t TrdHitXPos = TrdHit->GetX();
     Double_t TrdHitYPos = TrdHit->GetY();
     
     TVector3 vPos;
     TrdHit->Position(vPos);
     vPos.Z();
     
     fHM->H2("fhTrdHitXY")->Fill(TrdHitXPos,TrdHitYPos);
     fHM->H1("fhAllDetektorHitsInZPosition")->Fill(vPos.Z());
  }
  
  for (Int_t i=0;i<nofTofHit;i++)
   {
     CbmTofHit* TofHit = (CbmTofHit*)fTofHits->At(i);
     Double_t TofHitXPos = TofHit->GetX();
     Double_t TofHitYPos = TofHit->GetY();
     
     TVector3 vPos;
     TofHit->Position(vPos);
     vPos.Z();
     
     fHM->H2("fhTofHitXY")->Fill(TofHitXPos,TofHitYPos);
     fHM->H1("fhAllDetektorHitsInZPosition")->Fill(vPos.Z());
  }

  if (fPrimVertex != NULL){
     fKFVertex = CbmKFVertex(*fPrimVertex);
  } else {
     Fatal("CbmAnaDielectronTask::Exec","No PrimaryVertex array!");
  }

  McPair();

  MCPointPMT();

  RichHitPMT();

  FillCandidates();

  AssignMcToCandidates();
}


void CbmAnaJpsiTask::FillCandidates()
{
	fCandidates.clear();
	Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();

	for (Int_t i=0;i<nofGlobalTracks;i++) {
		// create candidate, in which we will store some parameters of reconstructed global track
    	CbmAnaJpsiCandidate cand;

		//get GlobalTrack from array
		CbmGlobalTrack* globalTrack = (CbmGlobalTrack*) fGlobalTracks->At(i);
		if (NULL == globalTrack) continue;

		// get StsTrack from global track
		Int_t stsInd = globalTrack->GetStsTrackIndex();
		if (stsInd < 0) continue;
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
		if (NULL == stsTrack) continue;

		//calculate and get track parameters
        CbmAnaJpsiUtils::CalculateAndSetTrackParamsToCandidate(&cand, stsTrack, fKFVertex);

        // RICH
        cand.fRichInd = globalTrack->GetRichRingIndex();
        if (cand.fRichInd < 0) continue;
        CbmRichRing* richRing = (CbmRichRing*) fRichRings->At(cand.fRichInd);
        if (NULL == richRing) continue;

        // TRD
        cand.fTrdInd = globalTrack->GetTrdTrackIndex();
        if (cand.fTrdInd < 0) continue;
        CbmTrdTrack* trdTrack = (CbmTrdTrack*) fTrdTracks->At(cand.fTrdInd);
        if (trdTrack == NULL) continue;

        // ToF
        cand.fTofInd = globalTrack->GetTofHitIndex();
        if (cand.fTofInd < 0) continue;
        CbmTofHit* tofHit = (CbmTofHit*) fTofHits->At(cand.fTofInd);
        if (tofHit == NULL) continue;

        //IsElectron(richRing, cand.fMomentum.Mag(), trdTrack, gTrack, &cand);

        // push candidate to the array
        // we store oanly candidate which have all local segments: STS, RICH, TRD, TOF
        fCandidates.push_back(cand);
	}
}

void CbmAnaJpsiTask::AssignMcToCandidates()
{
   int nCand = fCandidates.size();
   for (int i = 0; i < nCand; i++){
	   //reset MC information
      fCandidates[i].ResetMcParams();

      //STS
      //MCTrackId of the candidate is defined by STS track
      int stsInd = fCandidates[i].fStsInd;
      CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*) fStsTrackMatches->At(stsInd);
      if (stsMatch == NULL) continue;
      fCandidates[i].fStsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
      if (fCandidates[i].fStsMcTrackId < 0) continue;
      CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(fCandidates[i].fStsMcTrackId);
      if (mcTrack1 == NULL) continue;
      int pdg = TMath::Abs(mcTrack1->GetPdgCode());
      int motherId = mcTrack1->GetMotherId();

      // set MC signature for candidate
      if (pdg == 11 && motherId == -1) fCandidates[i].fIsMcSignalElectron = true;
      if (motherId >=0){
         CbmMCTrack* mct1 = (CbmMCTrack*) fMcTracks->At(motherId);
         int motherPdg = mct1->GetPdgCode();
         //Pi0 electron
         if (mct1 != NULL && motherPdg == 111 && pdg == 11) {
            fCandidates[i].fIsMcPi0Electron = true;
         }
         //Gamam conversion electron
         if (mct1 != NULL && motherPdg == 22 && pdg == 11){
            fCandidates[i].fIsMcGammaElectron = true;
         }
      }

      // RICH
      int richInd = fCandidates[i].fRichInd;
      CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*) fRichRingMatches->At(richInd);
      if (richMatch == NULL) continue;
      fCandidates[i].fRichMcTrackId = richMatch->GetMatchedLink().GetIndex();

      // TRD
      int trdInd = fCandidates[i].fTrdInd;
      CbmTrackMatchNew* trdMatch = (CbmTrackMatchNew*) fTrdTrackMatches->At(trdInd);
      if (trdMatch == NULL) continue;
      fCandidates[i].fTrdMcTrackId = trdMatch->GetMatchedLink().GetIndex();

      // ToF
      int tofInd = fCandidates[i].fTofInd;
      if (tofInd < 0) continue;
      CbmTofHit* tofHit = (CbmTofHit*) fTofHits->At(tofInd);
      if (tofHit == NULL) continue;
      Int_t tofPointIndex = tofHit->GetRefId();
      if (tofPointIndex < 0) continue;
      FairMCPoint* tofPoint = (FairMCPoint*) fTofPoints->At(tofPointIndex);
      if (tofPoint == NULL) continue;
      fCandidates[i].fTofMcTrackId = tofPoint->GetTrackID();
   }// candidates
}


void CbmAnaJpsiTask::DrawHist()
{
  SetDefaultDrawStyle();
  
  {
   TCanvas* c = new TCanvas("jpsi_NofMcTracksInEvent", "jpsi_NofMcTracksInEvent",  600, 600);
   //DrawH1(fhNofMcTracksInEvent);
   DrawH1(fHM->H1("fhNofMcTracksInEvent"));
  }
  
  {
   TCanvas* c = new TCanvas("jpsi_NofStsPointInEvent", "jpsi_NofStsPointInEvent",  600, 600);
   //DrawH1(fhNofStsPointInEvent);
   DrawH1(fHM->H1("fhNofStsPointInEvent"));
  }
  
  {
   TCanvas* c = new TCanvas("jpsi_NofRichPointInEvent", "jpsi_NofRichPointInEvent",  600, 600);
   //DrawH1(fhNofRichPointInEvent);
   DrawH1(fHM->H1("fhNofRichPointInEvent"));
  }
  
  {
   TCanvas* c = new TCanvas("jpsi_NofTrdPointInEvent", "jpsi_NofTrdPointInEvent",  600, 600);
   //DrawH1(fhNofTrdPointInEvent);
   DrawH1(fHM->H1("fhNofTrdPointInEvent"));
  }
  
  {
   TCanvas* c = new TCanvas("jpsi_NofTofPointInEvent", "jpsi_NofTofPointInEvent",  600, 600);
   //DrawH1(fhNofTofPointInEvent);
   DrawH1(fHM->H1("fhNofTofPointInEvent"));
  }
  
  {
   TCanvas* c = new TCanvas("jpsi_MonteCarloMomentumOfAll", "jpsi_MonteCarloMomentumOfAll",  600, 600);
   //DrawH1(fhMcMomAll);
   DrawH1(fHM->H1("fhMcMomAll"));
  }
 
  {
   TCanvas* c = new TCanvas("jpsi_PgdCode", "jpsi_PgdCode",  600, 600);
   //DrawH1(fhPdgCodeAll);
   DrawH1(fHM->H1("fhPdgCodeAll"));
  } 
  
  {
   TCanvas* c = new TCanvas("jpsi_McMomElectrons", "jpsi_McMomElectrons", 1000, 1000);
   c->Divide(2,2);
   c->cd(1);
   DrawH1(fHM->H1("fhMcMomElectrons"));
   c->cd(2);
   DrawH2(fHM->H2("fhMcPtYElectrons"));
   c->cd(3);
   TH1D* ProjX = fHM->H2("fhMcPtYElectrons")->ProjectionX();
   TH1D* ProjY = fHM->H2("fhMcPtYElectrons")->ProjectionY();
   DrawH1(ProjX);
   c->cd(4);
   DrawH1(ProjY);
  } 
  
  {
    TCanvas* c = new TCanvas("jpsi_NofStsHitInEvent","jpsi_NofStsHitInEvent", 600, 600);
    DrawH1(fHM->H1("fhNofStsHitsInEvent"));
  }
  
  {
    TCanvas* c = new TCanvas("jpsi_NofHitsInObjectInEvent","jpsi_NofHItsInObjectInEvent", 1000, 1000);
    c->Divide(2,2);
    c->cd(1);
    DrawH1(fHM->H1("fhNofStsHitsInEvent"));
    c->cd(2);
    DrawH1(fHM->H1("fhNofRichHitsInEvent"));
    c->cd(3);
    DrawH1(fHM->H1("fhNofTrdHitsInEvent"));
    c->cd(4);
    DrawH1(fHM->H1("fhNofTofHitsInEvent"));
  }
  
  {
    TCanvas* c = new TCanvas("jpsi_NofTracksOrRingsInObjectInEvent","jpsi_NofTracksOrRingsInObjectInEvent",1200,600);
    c->Divide(3,1);
    c->cd(1);
    DrawH1(fHM->H1("fhNofStsTracksInEvent"));
    c->cd(2);
    DrawH1(fHM->H1("fhNofRichRingsInEvent"));
    c->cd(3);
    DrawH1(fHM->H1("fhNofTrdTracksInEvent"));
  }
  
  {
    TCanvas* c = new TCanvas("jpsi_StsHitsXY","jpsi_StsHitsXY",600,600);
    DrawH2(fHM->H2("fhStsHitXY"));
  }
  
  {
    TCanvas* c = new TCanvas("jpsi_RichHitsXY","jpsi_RichHitsXY",600,600);
    DrawH2(fHM->H2("fhRichHitXY"));
  }  
  
  {
    TCanvas* c = new TCanvas("jpsi_TrdHitsXY","jpsi_TrdHitsXY",600,600);
    DrawH2(fHM->H2("fhTrdHitXY"));
  }
  
  {
    TCanvas* c = new TCanvas("jpsi_TofHitsXY","jpsi_TofHitsXY",600,600);
    DrawH2(fHM->H2("fhTofHitXY"));
  }  
  
  {
    TCanvas* c = new TCanvas("jpsi_AllDetektorHitsInZPosition","jpsi_AllDetektorHitsInZPosition",600,600);
    DrawH1(fHM->H1("fhAllDetektorHitsInZPosition"));
  }  
  
  {
      TCanvas* c = new TCanvas("jpsi_SignalElectronMom","jpsi_SignalElectronMom",600,600);
      DrawH1(fHM->H1("fhSignalElectronMom"));
    }

  { TCanvas* c = new TCanvas("jpsi_SignalElectronPt","jpsi_SignalElectronPt",600,600);
  DrawH1(fHM->H1("fhSignalElectronPt"));
  	}

  { TCanvas* c = new TCanvas("jpsi_SignalMotherPdgCode","jpsi_SignalMotherPdgCode",600,600);
  DrawH1(fHM->H1("fhSignalMotherPdgCode"));
  	}

  {
      TCanvas* c = new TCanvas("jpsi_GammaConvElectronMom","jpsi_GammaConvElectronMom",600,600);
      DrawH1(fHM->H1("fhGammaConvElectronMom"));
    }

  {
      TCanvas* c = new TCanvas("jpsi_GammaConvElectronPt","jpsi_GammaConvElectronPt",600,600);
      c->SetLogy();
      DrawH1(fHM->H1("fhGammaConvElectronPt"));
    }

  {
      TCanvas* c = new TCanvas("jpsi_GammaConvElectronStartVertXY","jpsi_GammaConvElectronStartVertXY",600,600);
      DrawH2(fHM->H2("fhGammaConvElectronStartVertXY"));
  }

  {
      TCanvas* c = new TCanvas("jpsi_GammaConvElectronStartVertZY","jpsi_GammaConvElectronStartVertZY",600,600);
      DrawH2(fHM->H2("fhGammaConvElectronStartVertZY"));
    }

  {
      TCanvas* c = new TCanvas("jpsi_GammaConvElectronStartVertZX","jpsi_GammaConvElectronStartVertZX",600,600);
      DrawH2(fHM->H2("fhGammaConvElectronStartVertZX"));
    }

  {
      TCanvas* c = new TCanvas("jpsi_GammaConvElectronStartVertZSqrt(X2+Y2)","jpsi_GammaConvElectronStartVertZSqrt(X2+Y2)",600,600);
      DrawH2(fHM->H2("fhGammaConvElectronStartVertZSqrt(X2+Y2)"));
    }


  {
	  TCanvas* c = new TCanvas("jpsi_RichPointSignalElectronPMTPlane","jpsi_RichPointSignalElectronPMTPlane",600,600);
	  DrawH2(fHM->H2("fhRichPointSignalElectronPMTPlane"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_pi0DalitzDecayMomentum","jpsi_pi0DalitzDecayMomentum",600,600);
	  DrawH1(fHM->H1("fhpi0DalitzDecayMomentum"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_RichPointGammaAndPi0PMTPlane","jpsi_RichPointGammaAndPi0PMTPlane",600,600);
	  DrawH2(fHM->H2("fhRichPointGammaAndPi0PMTPlane"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_RichPointGammaConversionPhotonInPETPlaneXY","jpsi_RichPointGammaConversionPhotonInPETPlaneXY",600,600);
	  DrawH2(fHM->H2("fhRichPointGammaConversionPhotonInPETPlaneXY"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_RichPointDalitzDecayInPETPlaneXY","jpsi_RichPointDalitzDecayInPETPlaneXY",600,600);
	  DrawH2(fHM->H2("fhRichPointDalitzDecayInPETPlaneXY"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_RichHitSignalElectronPMTPlane","jpsi_RichHitSignalElectronPMTPlane",600,600);
	  DrawH2(fHM->H2("fhRichHitSignalElectronPMTPlane"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_RichHitGammaConversionPhotonInPETPlaneXY","jpsi_RichHitGammaConversionPhotonInPETPlaneXY",600,600);
	  DrawH2(fHM->H2("fhRichHitGammaConversionPhotonInPETPlaneXY"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_RichHitDalitzDecayInPETPlaneXY","jpsi_RichHitDalitzDecayInPETPlaneXY",600,600);
	  DrawH2(fHM->H2("fhRichHitDalitzDecayInPETPlaneXY"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_ReconMomSignalElectron","jpsi_ReconMomSignalElectron",600,600);
	  DrawH1(fHM->H1("fhReconMomSignalElectron"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_ReconMomGammaConv","jpsi_ReconMomGammaConv",600,600);
	  DrawH1(fHM->H1("fhReconMomGammaConv"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_ReconMomDalitzDecay","jpsi_ReconMomDalitzDecay",600,600);
	  DrawH1(fHM->H1("fhReconMomDalitzDecay"));
  }

  {
	  TCanvas* c = new TCanvas("jpsi_DiffMomCandAndMcTrack","jpsi_DiffMomCandAndMcTrack",600,600);
	  c->SetLogy();
	  DrawH1(fHM->H1("fhDiffMomCandAndMcTrack"));
  }


   // fHM->WriteToFile();

}

void CbmAnaJpsiTask::McPair()
{
  Int_t nofMcTracks = fMcTracks->GetEntriesFast();
  for (Int_t i=0; i<nofMcTracks;i++)
  {
	CbmMCTrack* mctrack = (CbmMCTrack*) fMcTracks->At(i);
    Int_t motherId = mctrack->GetMotherId();
    Int_t pdgCode = mctrack->GetPdgCode();
    Double_t McMomentum = mctrack->GetP();
    Double_t motherPt = mctrack->GetPt();

    if (TMath::Abs(pdgCode) != 11 ) continue; //look for electron

    if (motherId == -1 )
    {   //analysis of signal electrons
    	fHM->H1("fhSignalElectronMom")->Fill(McMomentum);
    	fHM->H1("fhSignalElectronPt")->Fill(motherPt);
    	fHM->H1("fhSignalMotherPdgCode")->Fill(pdgCode);
    }


    if (motherId != -1) // no Signal electron
    { CbmMCTrack* motherMcTrack = (CbmMCTrack*) fMcTracks->At(motherId);
      Int_t grandmotherId = motherMcTrack->GetMotherId();
      Int_t motherPdgCode = motherMcTrack->GetPdgCode();
      Double_t motherMcMomentum = motherMcTrack->GetP();
      Double_t motherMcPt = motherMcTrack->GetPt();

      if (motherPdgCode == 22)	//gamma conversion particle
      {        //analysis of gamma conversion particle
    	fHM->H1("fhGammaConvElectronMom")->Fill(motherMcMomentum);
    	fHM->H1("fhGammaConvElectronPt")->Fill(motherMcPt);

    	TVector3 v;
    	mctrack->GetStartVertex(v);
    	fHM->H2("fhGammaConvElectronStartVertXY")->Fill(v.X(),v.Y());
    	fHM->H2("fhGammaConvElectronStartVertZY")->Fill(v.Z(),v.Y());
    	fHM->H2("fhGammaConvElectronStartVertZX")->Fill(v.Z(),v.X());
    	fHM->H2("fhGammaConvElectronStartVertZSqrt(X2+Y2)")->Fill( v.Z(),sqrt(v.Y()*v.Y()+v.X()*v.X()) );

        if (grandmotherId!=-1)
        {
    	CbmMCTrack* grandmotherMcTrack = (CbmMCTrack*) fMcTracks->At(grandmotherId);
    	Int_t grandmotherPdgCode = grandmotherMcTrack->GetPdgCode();
    	Double_t pi0DalitzDecayMomentum = grandmotherMcTrack->GetP();
    	if (grandmotherPdgCode == 111) //pi0 Dalitz decay
    	{
    		fHM->H1("fhpi0DalitzDecayMomentum")->Fill(pi0DalitzDecayMomentum);
    	}
        }

      }

    }

  }

}



void CbmAnaJpsiTask::MCPointPMT()
{
    Int_t nofRichPoints = fRichPoints->GetEntriesFast();
    for (Int_t i=0;i<nofRichPoints;i++)
    {
        // get the MC RICH point from array
        CbmRichPoint* richMcPoint = (CbmRichPoint*) fRichPoints->At(i);
        if (NULL == richMcPoint) continue;

        // get the RICH photon MC Track
        Int_t photonMcTrackId = richMcPoint->GetTrackID();
        if (photonMcTrackId==-1) continue;
        CbmMCTrack* photonMcTrack = (CbmMCTrack*) fMcTracks->At(photonMcTrackId);
        if (NULL == photonMcTrack) continue;

        // get photon mother MC Track (electron, pion etc.)
        Int_t photonMotherId = photonMcTrack->GetMotherId();
        if (photonMotherId == -1) continue;
        CbmMCTrack* photonMotherMcTrack = (CbmMCTrack*) fMcTracks->At(photonMotherId);
        if (NULL == photonMotherMcTrack) continue;
        Int_t photonMotherPdgCode = photonMotherMcTrack->GetPdgCode();

        // check that the photon was produced by electron
        if ( TMath::Abs(11) == photonMotherPdgCode )
        {
            Int_t photonGrandmotherId = photonMotherMcTrack->GetMotherId();

            // if primary signal electron
            if ( TMath::Abs(11) == photonMotherPdgCode || photonGrandmotherId == -1) {
                fHM->H2("fhRichPointSignalElectronPMTPlane")->Fill(richMcPoint->GetX(),richMcPoint->GetY());
            }

            if (photonGrandmotherId != -1) {
                //get the grandmother of the photon
            	CbmMCTrack* photonGrandmotherMcTrack = (CbmMCTrack*) fMcTracks->At(photonGrandmotherId);
                if (NULL == photonGrandmotherMcTrack) continue;
                Int_t photonGrandmotherPdgCode = photonGrandmotherMcTrack->GetPdgCode();

                // check that the grand mother of the photon is gamma or Pi0
                if (photonGrandmotherPdgCode == 111 || photonGrandmotherPdgCode == 22) {
                    fHM->H2("fhRichPointGammaAndPi0PMTPlane")->Fill(richMcPoint->GetX(),richMcPoint->GetY());
                }

                //check if grand mother of the photon is just gamma from gamma conversion
                if (photonGrandmotherPdgCode == 22)
                {
                	fHM->H2("fhRichPointGammaConversionPhotonInPETPlaneXY")->Fill(richMcPoint->GetX(),richMcPoint->GetY());

                }

                //check if grand mother of the photon is Pi0 (from Dalitz)
                if (photonGrandmotherPdgCode == 111)
                {
                	fHM->H2("fhRichPointDalitzDecayInPETPlaneXY")->Fill(richMcPoint->GetX(),richMcPoint->GetY());
                }


            }
        }
    }
}


void CbmAnaJpsiTask::RichHitPMT()
{
    Int_t nofRichHits = fRichHits->GetEntriesFast();
    for (Int_t i=0;i<nofRichHits;i++)
    {
        // get the RichHit from array
        CbmRichHit* richHit = (CbmRichHit*) fRichHits->At(i);
        if (NULL == richHit) continue;
        Int_t PointInd = richHit->GetRefId();
        if (PointInd < 0 )continue;

        // get the McRichPoint of the RichHit
        CbmRichPoint* richMcPoint = (CbmRichPoint*) fRichPoints->At(PointInd);
        if (NULL == richMcPoint) continue;

        // get the RICH photon MC Track
        Int_t photonMcTrackId = richMcPoint->GetTrackID();
        if (photonMcTrackId==-1) continue;
        CbmMCTrack* photonMcTrack = (CbmMCTrack*) fMcTracks->At(photonMcTrackId);
        if (NULL == photonMcTrack) continue;

        // get photon mother MC Track (electron, pion etc.)
        Int_t photonMotherId = photonMcTrack->GetMotherId();
        if (photonMotherId == -1) continue;
        CbmMCTrack* photonMotherMcTrack = (CbmMCTrack*) fMcTracks->At(photonMotherId);
        if (NULL == photonMotherMcTrack) continue;
        Int_t photonMotherPdgCode = photonMotherMcTrack->GetPdgCode();

        // check that the photon was produced by electron
        if ( TMath::Abs(11) == photonMotherPdgCode )
        {
           Int_t photonGrandmotherId = photonMotherMcTrack->GetMotherId();

           // if primary signal electron
           if ( TMath::Abs(11) == photonMotherPdgCode || photonGrandmotherId == -1)
           	{
        	  fHM->H2("fhRichHitSignalElectronPMTPlane")->Fill(richHit->GetX(),richHit->GetY());
           	}

           if (photonGrandmotherId != -1)
           {
             //get the grandmother of the photon
             CbmMCTrack* photonGrandmotherMcTrack = (CbmMCTrack*) fMcTracks->At(photonGrandmotherId);
             if (NULL == photonGrandmotherMcTrack) continue;
             Int_t photonGrandmotherPdgCode = photonGrandmotherMcTrack->GetPdgCode();

             //check out if grand mother of the photon is just gamma from gamma conversion
             if (photonGrandmotherPdgCode == 22)
             {
              	fHM->H2("fhRichHitGammaConversionPhotonInPETPlaneXY")->Fill(richHit->GetX(),richHit->GetY());

             }

            //check out if grand mother of the photon is Pi0 (from Dalitz)
             if (photonGrandmotherPdgCode == 111)
             {
              	fHM->H2("fhRichHitDalitzDecayInPETPlaneXY")->Fill(richHit->GetX(),richHit->GetY());
             }

           }
        }
    }
}





void CbmAnaJpsiTask::Finish()
{
  DrawHist();
}


ClassImp(CbmAnaJpsiTask)

