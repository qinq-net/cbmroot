
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
#include "CbmAnaJpsiHist.h"
#include "CbmTrackMatchNew.h"
#include "CbmAnaJpsiKinematicParams.h"


using namespace std;

CbmAnaJpsiTask::CbmAnaJpsiTask()
    : FairTask("CbmAnaJpsiTask"),
	  fEventNum(0),
	  fMcTracks(NULL),
	  fStsPoints(NULL),
	  fStsHits(NULL),
	  fStsTracks(NULL),
	  fStsTrackMatches(NULL),
	  fRichPoints(NULL),
	  fRichHits(NULL),
	  fRichRings(NULL),
	  fRichRingMatches(NULL),
	  fTrdPoints(NULL),
	  fTrdHits(NULL),
	  fTrdTracks(NULL),
	  fTrdTrackMatches(NULL),
	  fTofPoints(NULL),
	  fTofHits(NULL),
	  fGlobalTracks(NULL),
	  fPrimVertex(NULL),
	  fKFVertex(),
	  fCandidates(),
	  fHM(NULL)
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

void CbmAnaJpsiTask::CreateSourceTypesH1(
      const string& name,
      const string& axisX,
      const string& axisY,
      double nBins,
      double min,
      double max)
{
   for (Int_t i = 0; i < CbmAnaJpsiHist::fNofSourceTypes; i++){
      string hname = name + "_"+ CbmAnaJpsiHist::fSourceTypes[i];
      fHM->Create1<TH1D>(hname, hname+";"+axisX+";"+axisY, nBins, min, max);
   }
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
   fHM->Create2<TH2D>("fhGammaConvElectronStartVertZSqrt(X2+Y2)","fhGammaConvElectronStartVertZSqrt(X2+Y2);Z[cm];Sqrt(#X^{2}+#Y^{2})[cm];Entries",1100,0,1100,600,0,600);

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
   CreateSourceTypesH1("fhChi2PrimEl","#chi^{2}_{prim}", "Yield", 200, 0., 20.);
   CreateSourceTypesH1("fhMomEl","P [GeV/c]", "Yield", 160, 0., 16.);
   CreateSourceTypesH1("fhChi2StsEl","#chi^{2}_{STS}", "Yield", 80, 0., 8.);
   CreateSourceTypesH1("fhRapidityEl","Rapidity", "Yield", 40, 0., 4.);

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

  DifferenceSignalAndBg();

  PairMcAndAcceptance();
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
		cand.fStsInd = globalTrack->GetStsTrackIndex();
		if (cand.fStsInd < 0) continue;
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(cand.fStsInd);
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
        // we store only candidate which have all local segments: STS, RICH, TRD, TOF
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
         //Gamma conversion electron
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

void CbmAnaJpsiTask::DifferenceSignalAndBg()
{
    Int_t nCand = fCandidates.size();
    for (Int_t i = 0; i < nCand; i++){
        if (fCandidates[i].fIsMcSignalElectron){
            fHM->H1("fhChi2PrimEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fChi2Prim);
            fHM->H1("fhMomEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fMomentum.Mag());
            fHM->H1("fhChi2StsEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fChi2sts);
            fHM->H1("fhRapidityEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fRapidity);
        } else {
        	fHM->H1("fhChi2PrimEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fChi2Prim);
        	fHM->H1("fhMomEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fMomentum.Mag());
        	fHM->H1("fhChi2StsEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fChi2sts);
        	fHM->H1("fhRapidityEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fRapidity);
        }
        if (fCandidates[i].fIsMcGammaElectron){
        	fHM->H1("fhChi2PrimEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fChi2Prim);
        	fHM->H1("fhMomEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fMomentum.Mag());
        	fHM->H1("fhChi2StsEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fChi2sts);
        	fHM->H1("fhRapidityEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fRapidity);
        }
        if (fCandidates[i].fIsMcPi0Electron) {
        	fHM->H1("fhChi2PrimEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fChi2Prim);
        	fHM->H1("fhMomEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fMomentum.Mag());
        	fHM->H1("fhChi2StsEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fChi2sts);
        	fHM->H1("fhRapidityEl_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fRapidity);
        }
    } // loop over candidates
}

Bool_t CbmAnaJpsiTask::IsMcTrackAccepted(
		Int_t mcTrackInd)
{
	CbmMCTrack* tr = (CbmMCTrack*) fMcTracks->At(mcTrackInd);
	if (tr == NULL) return false;
	//Int_t nRichPoints = fNofHitsInRingMap[mcTrackInd];
	return (tr->GetNPoints(kMVD) + tr->GetNPoints(kSTS) >= 4);// && nRichPoints >= 7 && tr->GetNPoints(kTRD) >= 2 && tr->GetNPoints(kTOF) > 0) ;
}

void CbmAnaJpsiTask::PairMcAndAcceptance()
{
	Int_t nMcTracks = fMcTracks->GetEntries();
	for (Int_t iP = 0; iP < nMcTracks; iP++) {
		CbmMCTrack* mctrackP = (CbmMCTrack*) fMcTracks->At(iP);
		Int_t motherIdP = mctrackP->GetMotherId();
		Int_t pdgP = mctrackP->GetPdgCode();
		if ( pdgP != 11 ) continue;
		Bool_t isAccP = IsMcTrackAccepted(iP);
		for (Int_t iM = 0; iM < nMcTracks; iM++) {
			if (iP == iM) continue;
			CbmMCTrack* mctrackM = (CbmMCTrack*) fMcTracks->At(iM);
			Int_t motherIdM = mctrackM->GetMotherId();
			Int_t pdgM = mctrackM->GetPdgCode();
			if ( pdgM != -11 ) continue;
			Bool_t isAccM = IsMcTrackAccepted(iM);
			CbmAnaJpsiKinematicParams p = CbmAnaJpsiKinematicParams::KinematicParamsWithMcTracks(mctrackP,mctrackM);

			// e+/- from signal
			if (motherIdM == -1 && pdgM == -11 && motherIdP == -1 && pdgP == 11) {
				// 2D histo p.fRapidity, p.fPt
				// 1D histo p.fMomentumMag
				// 1D histo p.fMinv

				//accepted e+/-
				if (isAccP && isAccM) {
					// 2D histo p.fRapidity, p.fPt
					// 1D histo p.fMomentumMag
					// 1D histo p.fMinv
				}
			}
		}//iM
	}//iP
} // PairsAcceptance


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
	fHM->WriteToFile();
}


ClassImp(CbmAnaJpsiTask)

