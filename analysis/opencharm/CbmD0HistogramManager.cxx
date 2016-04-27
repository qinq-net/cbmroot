// -------------------------------------------------------
// -----          CbmD0HistogramManager source file  -----
// -----          Created 09.11.2015 by P.Sitzmann   -----
// -------------------------------------------------------
#include "CbmD0HistogramManager.h"

// Includes from KF
#include "KFParticle.h"

// Includes from Cbm
#include "CbmMCTrack.h"
#include "CbmMvdPoint.h"
#include "CbmVertex.h"
#include "CbmTrack.h"
#include "CbmStsTrack.h"
#include "CbmTrackMatchNew.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"
#include "CbmKFVertex.h"
#include "CbmL1.h"
#include "CbmKF.h"
#include "CbmD0Candidate.h"

// Includes from Fair
#include "FairLogger.h"
#include "FairRootManager.h"
#include "FairMCPoint.h"

// Includes from Root
#include "TClonesArray.h"
#include "TMatrixFSym.h"

#include <stdio.h>

using namespace std;

// -------------------------------------------------------------------------
CbmD0HistogramManager::CbmD0HistogramManager()
    :FairTask("default"),
     fcutPT(),
     fcutSvZ(),
     fcutPZ(),
     fChois(),
     fnrMcEvents(),
     fnrPairEvents(),
     fnrRecoEvents(),
     fnrTrackEvents(),
     fwriteOutFile(),
     fMCFile(),
     fRecoFile(),
     fPairFile(),
     fTrackFile(),
     fmcTrackBranch(),
     fmvdPointBranch(),
     fStsPointBranch(),
     fpairBranch(),
     fListMCTracks(),
     fListMCPointsMvd(),
     fListMCPointsSts(),
     fListMCPointsRich(),
     fListMCPointsTrd(),
     fListMCPointsTof(),
     fListPairs(),
     fnrSingelEvents(),
     fKaonBranch(),
     fPionBranch(),
     fListKaons(),
     fListPions(),
     fRecoBranch(),
     fListReco(),
     fListTrackMatch(),
     fTrackMatchBranch()
{

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0HistogramManager::CbmD0HistogramManager(TString name,Float_t PTCut,
					     Float_t SvZCut, Float_t PZCut)
    :FairTask(name),
     fcutPT(),
     fcutSvZ(),
     fcutPZ(),
     fChois(),
     fnrMcEvents(),
     fnrPairEvents(),
     fnrRecoEvents(),
     fnrTrackEvents(),
     fwriteOutFile(),
     fMCFile(),
     fRecoFile(),
     fPairFile(),
     fTrackFile(),
     fmcTrackBranch(),
     fmvdPointBranch(),
     fStsPointBranch(),
     fpairBranch(),
     fListMCTracks(),
     fListMCPointsMvd(),
     fListMCPointsSts(),
     fListMCPointsRich(),
     fListMCPointsTrd(),
     fListMCPointsTof(),
     fListPairs(),
    fnrSingelEvents(),
    fKaonBranch(),
    fPionBranch(),
    fListKaons(),
    fListPions(),
    fRecoBranch(),
fListReco(),
fListTrackMatch(),
fTrackMatchBranch()
{

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0HistogramManager::~CbmD0HistogramManager()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::SetHistogramChois(TString group)
{

}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmD0HistogramManager::SetPostCuts(Float_t PTCut, Float_t SvZCut, Float_t PZCut)
{
    fcutPT = PTCut;
    fcutSvZ = SvZCut;
    fcutPZ = PZCut;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
InitStatus CbmD0HistogramManager::Init()
{

   ioman = FairRootManager::Instance();
      if (! ioman) {
	  cout << "-E- ::Init: "
	 << "RootManager not instantised!" << endl;
    return kFATAL;
    }

    InitSingel();
    InitReco();
    InitPair();
    InitMc();

    return kSUCCESS;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitAll()
{

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitReco()
{
    LOG(INFO)<<"Init RecoTrack Histogramms"<< FairLogger::endl;

    fListReco = new TClonesArray("CbmStsTrack", 100);
    fPrimVtxs = new CbmVertex();
    fListTrackMatch = new TClonesArray("CbmTrackMatchNew", 100);

    fListReco = (TClonesArray*)ioman->GetObject("StsTrack");
    fPrimVtxs =  (CbmVertex*) ioman->GetObject("PrimaryVertex");
    fListTrackMatch = (TClonesArray*) ioman->GetObject("StsTrackMatch");

    recoMomdist = new TH1F("RecoMomentumDist", "Momentumdistribution Reconstructed Tracks", 100, 0, 10);

    recoSigma = new TH2F("RecoMomentumSigma", "Differenz in reconstructed Momentum to MC all Tracks",30, 0, 3, 100, -1,1 );

    recoPionSigma = new TH2F("RecoMomentumSigmaPion", "Differenz in reconstructed Pion Momentum to MC",30, 0, 3, 100, -1, 1);
    recoKaonSigma = new TH2F("RecoMomentumSigmaKaon", "Differenz in reconstructed Kaon Momentum to MC",30, 0, 3, 100, -1, 1);

    recoPionSigmaSec = new TH2F("RecoMomentumSigmaPionSec", "Differenz in reconstructed secondary Pion Momentum to MC",30, 0, 3, 100, -1, 1);
    recoKaonSigmaSec = new TH2F("RecoMomentumSigmaKaonSec", "Differenz in reconstructed secondary Kaon Momentum to MC",30, 0, 3, 100, -1, 1);

    recoIPAll = new TH1F("RecoIPAll","Reconstructed impact parameter all tracks", 1000, 0, 5);
    recoIP4Perfect= new TH1F("RecoIP4perfect","Reconstructed impact parameter 4 MVDHit 100% matched", 1000, 0, 5);
    recoIPfirstWrong= new TH1F("RecoIPfirstWrong","Reconstructed impact parameter all tracks with wrong 1 MVD-Station Hit", 1000, 0, 5);

}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitSingel()
{
    LOG(INFO)<<"Init SingelTrack Histogramms"<< FairLogger::endl;

    fListKaons = new TClonesArray("KFParticle", 100);
    fListKaons  = (TClonesArray*)ioman->GetObject("CbmD0KaonParticles");

    fListPions = new TClonesArray("KFParticle", 100);
    fListPions = (TClonesArray*)ioman->GetObject("CbmD0PionParticles");

    momKaon = new TH1F("KaonMomentumDist", "Momentumdistribution Reconstructed Kaons", 100, 0, 30);
    momPion = new TH1F("PionMomentumDist", "Momentumdistribution Reconstructed Pions", 100, 0, 30);

    kaonPtY = new TH2F("PtYDistKaon", "Pt vs Rapidity reconstructed Kaons",1000, -3, 3, 1000, 0, 3);
    pionPtY = new TH2F("PtYDistPion", "Pt vs Rapidity reconstructed Pions",1000, -3, 3, 1000, 0, 3);

    pionIP = new TH1F("PionIPRadius", "Impact Radius Reconstructed Pions", 100, 0, 0.3);
    kaonIP = new TH1F("KaonIPRadius", "Impact Radius Reconstructed Kaons", 100, 0, 0.3);


}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitPair()
{
    LOG(INFO)<<"Init Pair Histogramms"<< FairLogger::endl;

    fListPairs = new TClonesArray("KFParticle", 100);
    fListCandidates = new TClonesArray("CbmD0Candidate", 100);
    fListPairs = (TClonesArray*)ioman->GetObject("CbmD0KFCandidate");
    fListCandidates = (TClonesArray*)ioman->GetObject("CbmD0Candidate");

    pairMomentum= new TH1F("PairMomentumDist", "Momentumdistribution Reconstructed D0", 100, 0, 30);
    massSpectra= new TH1F("MassSpectraD0","Reconstructed Mass D0", 100, 0, 3);

    ptY= new TH2F("PtYDistD0", "Pt vs Rapidity reconstructed D0",1000, -3, 3, 1000, 0, 3);
    chi2IP= new TH2F("Chi2vsIPD0"," Chi2 vs IP reconsturcted D0", 1000, 0, 1 , 1000, 0, 0.005);

    chi2RecoD0= new TH1F("Chi2RecoD0", "Chi2 of reconstructed D0", 1000, 0, 10);
    zPosDecay= new TH1F("ZPosDecay", "Z Position of D0 decay", 2000, 0, 2);
    secondaryVertexRes = new TH1F("SecondaryVertexRes", "Secondary Vertexresolution", 2000, -0.5, 0.5);
    iPRadiusD0= new TH1F("IPRadiusD0", "Impact Radius of D0", 1000, 0, 0.15);

    chi2RecoD0PostCut= new TH1F("Chi2RecoD0PostCut", "Chi2 of reconstructed D0, post cuts applied", 1000, 0, 10);
    zPosDecayPostCut= new TH1F("ZPosDecayPostCut", "Z Position of D0 decay, post cuts applied", 2000, 0, 2);
    secondaryVertexResPostCut = new TH1F("SecondaryVertexResPostCut", "Secondary Vertexresolution, post cuts applied", 2000, -0.5, 0.5);
    iPRadiusD0PostCut= new TH1F("IPRadiusD0PostCut", "Impact Radius of D0, post cuts applied", 1000, 0, 0.15);


}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitMc()
{
     LOG(INFO)<<"Init MC Histogramms"<< FairLogger::endl;

    fListMCTracks = new TClonesArray("CbmMCTrack", 1000);
    fListMCTracks = (TClonesArray*)ioman->GetObject("MCTrack");

    mcMomentum = new TH1F("MCPointMomentumDist", "Momentumdistribution MC Tracks", 100, 0, 30);        // all mc points

   mcMomentumMvd = new TH1F("MCPointMomentumDistMvd", "Momentumdistribution MC Track in MVD acceptance", 100 , 0, 30);   // MVD-MCPoints > 0
   mcMomentumMvdTrack = new TH1F("MCPointMomentumDistMvdTrackable", "Momentumdistribution MC Track in MVD trackable", 100 , 0, 30);   // MVD-Points >= 3
   mcMomentumMvdStsTrack = new TH1F("MCPointMomentumDistMvdStsTrackable", "Momentumdistribution MC Track in MVD and STS trackable", 100 , 0, 30);          // MVD + STS MC Points >= 3
   mcMomentumStsTrackMvdVertex= new TH1F("MCPointMomentumDistStsTrackMvdVertex", "Momentumdistribution MC Track in STS trackable with Vertex information from MVD", 100 , 0, 30); // STS MC Points >= 3 MVD MC Points >= 2

   mcMomentumKaonTrackable= new TH1F("MCSignalMomentumDistTrackableKaon", "Momentumdistribution of embedted Signal Trackable Kaon", 100, 0, 15);
   mcMomentumKaonTrackPID= new TH1F("MCSignalMomentumDistTrackableKaonPID", "Momentumdistribution of embedted Signal Trackable Kaon and PID", 100, 0, 15);
              
  mcMomentumPion1Trackable = new TH1F("MCSignalMomentumDistTrackablePion1", "Momentumdistribution of embedted Signal Trackable Pion 1", 100, 0, 15);
 mcMomentumPion1TrackPID = new TH1F("MCSignalMomentumDistTrackablePion1PID", "Momentumdistribution of embedted Signal Trackable Pion 1 and PID", 100, 0, 15);

  mcMomentumSignal = new TH1F("MCSignalMomentumDistTrackable", "Momentumdistribution of embedted Signal trackable ", 100, 0, 15);
  mcMomentumSignalPID = new TH1F("MCSignalMomentumDistTrackablePID", "Momentumdistribution of embedted Signal trackable and PID", 100, 0, 15);

  diffRecoSVMCSV= new TH1F("DiffRecoSVMCSV","Difference of reconstructed and mc secondary vertex",1000,-0.05,0.05);
  diffRecoSVMCSV1GeV = new TH1F("DiffRecoSVMCSV1GeV","Difference of reconstructed and mc secondary vertex[p>=1GeV]",1000,-0.05,0.05);

   chi2vsDiffVertex = new TH2F("Chi2vsDiffVertex","Chi2 of reconstructed D0 vs sec. vertexresolution", 1000 , 0, 1, 1000, 0, 0.005);

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::Register()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::Exec(Option_t* option = "")
{
    ExecSingel();
    ExecReco();
    ExecPair();
    ExecMc();
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::ExecAll()
{


}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::Finish()
{
     fwriteOutFile->cd();
     mcMomentum->Write();
     mcMomentumMvd->Write();
     mcMomentumMvdTrack->Write();
     mcMomentumMvdStsTrack->Write();
     mcMomentumStsTrackMvdVertex->Write();
     mcMomentumKaonTrackable->Write();
     mcMomentumKaonTrackPID->Write();
     mcMomentumPion1Trackable->Write();
     mcMomentumPion1TrackPID->Write();
     mcMomentumSignal->Write();
     mcMomentumSignalPID->Write();
     diffRecoSVMCSV->Write();
     diffRecoSVMCSV1GeV->Write();
     chi2vsDiffVertex->Write();
     pairMomentum->Write();
     massSpectra->Write();
     chi2RecoD0->Write();
     zPosDecay->Write();
     secondaryVertexRes->Write();
     iPRadiusD0->Write();
     ptY->Write();
     chi2IP->Write();
     recoMomdist->Write();
     recoSigma->Write();
     recoPionSigma->Write() ;
     recoKaonSigma->Write();
     recoPionSigmaSec->Write() ;
     recoKaonSigmaSec->Write();
     recoIPAll->Write();
     recoIP4Perfect->Write();
     recoIPfirstWrong->Write();
     pionIP->Write();
     kaonIP->Write();
     momKaon->Write();
     momPion->Write();
     kaonPtY->Write();
     pionPtY->Write();

     chi2RecoD0PostCut->Write();
     zPosDecayPostCut->Write();
     secondaryVertexResPostCut->Write();
     iPRadiusD0PostCut->Write();
}
// -------------------------------------------------------------------------



// -------------------------------------------------------------------------
void CbmD0HistogramManager::ExecMc()
{
    LOG(INFO) << "Starting MC Branch of CbmD0HistogramManager" << FairLogger::endl;

    CbmMCTrack* mcTrack;
    CbmMCTrack* kaon = NULL;
    CbmMCTrack* pion = NULL;
    TLorentzVector kaonLV, pion1LV, signalLV;

    for(int i = 0; i < fListMCTracks->GetEntriesFast(); i++)
       {
       mcTrack = (CbmMCTrack*)fListMCTracks->At(i);
      
       if(mcTrack->GetMotherId() == -1)
       {
       mcMomentum->Fill(mcTrack->GetP());

       if(mcTrack->GetNPoints(kMVD) > 0)
         {
	   mcMomentumMvd->Fill(mcTrack->GetP());
	   if(mcTrack->GetNPoints(kMVD)>=3)
	   {
               mcMomentumMvdTrack->Fill(mcTrack->GetP());
	   }
	   if(mcTrack->GetNPoints(kMVD) + mcTrack->GetNPoints(kSTS) >= 3)
	      {
		  mcMomentumMvdStsTrack->Fill(mcTrack->GetP());

		  if(mcTrack->GetPdgCode() == -321 && mcTrack->GetStartZ() != 0)  // Kaons from signal Trackable
		      {
		          kaon = (CbmMCTrack*)fListMCTracks->At(i);
			  mcMomentumKaonTrackable->Fill(mcTrack->GetP());
			  if(mcTrack->GetNPoints(kTOF)>0)  // Kaons from signal Trackable and TOF Pid
			  {
			    mcMomentumKaonTrackPID->Fill(mcTrack->GetP());
			  }
		      }

                  if(mcTrack->GetPdgCode() == 211 && mcTrack->GetStartZ() != 0 )  // Pion+ from signal Trackable
		      {
		          pion =  (CbmMCTrack*)fListMCTracks->At(i);
			  mcMomentumPion1Trackable->Fill(mcTrack->GetP());
			  if(mcTrack->GetNPoints(kTOF)>0)  // Pion+ from signal Trackable and TOF Pid
			  {
			      mcMomentumPion1TrackPID->Fill(mcTrack->GetP());
                          }
                      }

	      }
	   if( mcTrack->GetNPoints(kSTS) >= 3 && mcTrack->GetNPoints(kMVD) > 1 )
	      {
              mcMomentumStsTrackMvdVertex->Fill(mcTrack->GetP());
	      }

	 }

       }
       }
    if (pion != NULL && kaon != NULL)
    {
        kaon->Get4Momentum(kaonLV);
        pion->Get4Momentum(pion1LV);
        signalLV = kaonLV + pion1LV;
	mcMomentumSignal->Fill(signalLV.P());
        

        if(kaon->GetNPoints(kTOF)>0 && pion->GetNPoints(kTOF)>0)
            mcMomentumSignalPID->Fill(signalLV.P());

    }

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::ExecPair()
{
    LOG(INFO) << "Starting Pair Branch of CbmD0HistogramManager" << FairLogger::endl;


    float fvtx[3];


    KFParticle openCharm;
    CbmD0Candidate OCcandidate;

          fvtx[0] = fPrimVtxs->GetX();
          fvtx[1] = fPrimVtxs->GetY();
          fvtx[2] = fPrimVtxs->GetZ();

	  if(fListPairs->GetEntriesFast() != fListCandidates->GetEntriesFast())
	  {
	      LOG(INFO) << "Number of KF and D0 Candiates unequal, skipp this event" <<FairLogger::endl;
	  }

	for(Int_t i = 0; i < fListPairs->GetEntriesFast(); i++)
	{

	    openCharm = *((KFParticle*)fListPairs->At(i));
            OCcandidate = *((CbmD0Candidate*)fListCandidates->At(i));

	    chi2RecoD0->Fill(openCharm.GetChi2());
	    zPosDecay->Fill(openCharm.GetZ());
            secondaryVertexRes->Fill(openCharm.GetZ());


	    pairMomentum->Fill(openCharm.GetP());
	    massSpectra->Fill(openCharm.GetMass());
	    ptY->Fill(openCharm.GetY(), openCharm.GetPt());

            openCharm.TransportToPoint(&*fvtx);
	    Double_t Pair_IP  = sqrt((openCharm.GetX()*openCharm.GetX()) + (openCharm.GetY()*openCharm.GetY()) );
	    iPRadiusD0->Fill(Pair_IP);

	    chi2IP->Fill(openCharm.GetChi2(), Pair_IP);

	    if( OCcandidate.fPt1 >= fcutPT && OCcandidate.fPt2 >= fcutPT && openCharm.GetZ() >= fcutSvZ && openCharm.GetPz() >= fcutPZ )
	     {
	    chi2RecoD0PostCut->Fill(openCharm.GetChi2());
	    zPosDecayPostCut->Fill(openCharm.GetZ());
	    secondaryVertexResPostCut->Fill(openCharm.GetZ());
	    iPRadiusD0PostCut->Fill(Pair_IP);
	    }

	}


}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::ExecReco()
{
    LOG(INFO) << "Starting Reco Branch of CbmD0HistogramManager" << FairLogger::endl;


    CbmL1PFFitter fitter;
    CbmKFVertex kfvtx;
    CbmVertex* vtx = new CbmVertex();


	for(Int_t i = 0; i < fListReco->GetEntriesFast(); i++)
	{
	    CbmStsTrack* track = (CbmStsTrack*)fListReco->At(i);
	    CbmTrackMatchNew* trackMatch = (CbmTrackMatchNew*)fListTrackMatch->At(i);
	    Int_t trackMatchNr = trackMatch->GetMatchedLink().GetIndex();
            CbmMCTrack* mcTrack = NULL;
	    if(trackMatchNr > -1) mcTrack = (CbmMCTrack*)fListMCTracks->At(trackMatchNr);

	    TVector3 momentum;
	    track->GetParamFirst()->Momentum(momentum);
	    Float_t P = sqrt(momentum.X() * momentum.X() +momentum.Y() * momentum.Y() +momentum.Z() * momentum.Z());

	    recoMomdist->Fill(P);

            if(mcTrack)
	    {
	      	Float_t mcP = mcTrack->GetP();
		recoSigma->Fill( mcP, (P - mcP) / mcP);

		if(mcTrack->GetPdgCode() == -321)
		{
		    recoKaonSigma->Fill(mcP, (P - mcP) / mcP);
		    if(mcTrack->GetStartZ() != 0.0)
                        recoKaonSigmaSec->Fill(mcP, (P - mcP) / mcP);
		}
		if(mcTrack->GetPdgCode() == 211)
		{
		    recoPionSigma->Fill(mcP, (P - mcP) / mcP);
                    if(mcTrack->GetStartZ() != 0.0)
		      	recoPionSigmaSec->Fill(mcP, (P - mcP) / mcP);
		}

                vector<CbmStsTrack>	vRTracks;
                vector<L1FieldRegion> vField;

		vRTracks.resize(1);
		vRTracks[0] = *(track);
                TMatrixFSym* covMat = new TMatrixFSym(3);
		vtx->SetVertex(0, 0, 0, 0, 0, 0, *covMat);
		vector<float> ChiToPrimVtx;
  
		kfvtx = CbmKFVertex(*vtx);
		fitter.GetChiToVertex(vRTracks, vField, ChiToPrimVtx, kfvtx, 1000000000);

		Float_t impactParam = sqrt((track->GetParamFirst()->GetX() *track->GetParamFirst()->GetX()) + (track->GetParamFirst()->GetY() *track->GetParamFirst()->GetY()));
              	recoIPAll->Fill(impactParam);

		if(track->GetNofMvdHits() == 4 && trackMatch->GetTrueOverAllHitsRatio() == 1)
		{
		    recoIP4Perfect->Fill(impactParam);
		}
		if(trackMatch->GetTrueOverAllHitsRatio())
		{
		 ;  // recoIPfirstWrong->Fill(impactParam);
		}

	    }


	}




}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmD0HistogramManager::ExecSingel()
{
    LOG(INFO) << "Starting Singel Branch of CbmD0HistogramManager" << FairLogger::endl;

float fvtx[3];

         KFParticle pion, kaon;

          fvtx[0] = fPrimVtxs->GetX();
          fvtx[1] = fPrimVtxs->GetY();
          fvtx[2] = fPrimVtxs->GetZ();

	for(Int_t i = 0; i < fListPions->GetEntriesFast(); i++)
	{
	    pion = *((KFParticle*)fListPions->At(i));
            momPion->Fill(pion.GetP());
            pionPtY->Fill(pion.GetY(), pion.GetPt());

            pion.TransportToPoint(&*fvtx);
	    Double_t IP  = sqrt((pion.GetX()*pion.GetX()) + (pion.GetY()*pion.GetY()) );
            pionIP->Fill(IP);
	}

        	for(Int_t j = 0; j < fListKaons->GetEntriesFast(); j++)
	{
	    kaon = *((KFParticle*)fListKaons->At(j));
            momKaon->Fill(kaon.GetP());
	    kaonPtY->Fill(kaon.GetY(), kaon.GetPt());

            kaon.TransportToPoint(&*fvtx);
	    Double_t IP  = sqrt((kaon.GetX()*kaon.GetX()) + (kaon.GetY()*kaon.GetY()) );
            kaonIP->Fill(IP);
	}



}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
ClassImp(CbmD0HistogramManager)
