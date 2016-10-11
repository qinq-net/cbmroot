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
#include "CbmD0TrackCandidate.h"
#include "CbmGlobalTrack.h"

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
     ioman(NULL),
     fEventNr(-1),
     fchi2Singel(-1.1),
     fcutPT(-1.1),
     fcutSvZ(-1.1),
     fcutPZ(-1.1),
     fChois(),
     fnrMcEvents(-1),
     fnrPairEvents(-1),
     fnrRecoEvents(-1),
     fnrTrackEvents(-1),
     fnrSingelEvents(-1),
     fwriteOutFile(NULL),
     fListTrackMatch(NULL),
     fListMCTracks(NULL),
     fListMCPointsMvd(NULL),
     fListMCPointsSts(NULL),
     fListMCPointsRich(NULL),
     fListMCPointsTrd(NULL),
     fListMCPointsTof(NULL),
     fListPairs(NULL),
     fListCandidates(NULL),
     fListKaons(NULL),
     fListPions(NULL),
     fListReco(NULL),
     fListGlobal(NULL),
     fKaonTrackArray(NULL),
     fPionTrackArray(NULL),
     fPrimVtxs(NULL),
     mcMomentum(NULL),
     mcMomentumMvd(NULL),
     mcMomentumMvdTrack(NULL),
     mcMomentumMvdStsTrack(NULL),
     mcMomentumMvdStsTrackPID(NULL),
     mcMomentumStsTrackMvdVertex(NULL),
     mcMomentumKaonTrackable(NULL),
     mcMomentumKaonTrackPID(NULL),
     mcMomentumPion1Trackable(NULL),
     mcMomentumPion1TrackPID(NULL),
     mcMomentumSignal(NULL),
     mcMomentumSignalPID(NULL),
     diffRecoSVMCSV(NULL),
     diffRecoSVMCSV1GeV(NULL),
     chi2vsDiffVertex(NULL),
     pairMomentum(NULL),
     massSpectra(NULL),
     massSpectraSVZ3H(NULL),
     massSpectraChi23H(NULL),
     massSpectraSVZ4H(NULL),
     massSpectraChi24H(NULL),
     chi2RecoD0(NULL),
     zPosDecay(NULL),
     secondaryVertexRes(NULL),
     secondaryVertexResmin3MVD(NULL),
     secondaryVertexRes3MVD(NULL),
     secondaryVertexRes4MVD(NULL),
     iPRadiusD0(NULL),
     chi2RecoD0PostCut(NULL),
     zPosDecayPostCut(NULL),
     secondaryVertexResPostCut(NULL),
     secondaryVertexResPostCut3H(NULL),
     secondaryVertexResPostCutex3H(NULL),
     secondaryVertexResPostCut4H(NULL),
     secVertexResChiCut(NULL),
     secVertexResChiCutSVZcut(NULL),
     secVertexResChiCutSVZcut4MVD(NULL),
     iPRadiusD0PostCut(NULL),
     chi2singelTrackBadD0(NULL),
     iPsingelTrackBadD0(NULL),
     nrMvdHitsBadD0(NULL),
     ptY(NULL),
     chi2IP(NULL),
     recoMomdist(NULL),
     recoMomdistAll(NULL),
     recoMomdistmin3Hit(NULL),
     recoMomdistmin4Hit(NULL),
     recoMomdistSecVCut(NULL),
     recoMomdistSecVCutmin4Hit(NULL),
     recoSigma(NULL),
     recoPionSigma(NULL),
     recoKaonSigma(NULL),
     recoPionSigmaSec(NULL),
     recoKaonSigmaSec(NULL),
     recoIPAll(NULL),
     recoIP4Perfect(NULL),
     recoIPfirstWrong(NULL),
     pionIP(NULL),
     kaonIP(NULL),
     momKaon(NULL),
     momPion(NULL),
     chi2NDFKaon(NULL),
     chi2NDFPion(NULL),
     kaonPtY(NULL),
     pionPtY()
{

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0HistogramManager::CbmD0HistogramManager(TString name,Float_t PTCut,
					     Float_t SvZCut, Float_t PZCut)
    :FairTask(name),
     ioman(NULL),
     fEventNr(-1),
     fchi2Singel(-1.1),
     fcutPT(PTCut),
     fcutSvZ(SvZCut),
     fcutPZ(PZCut),
     fChois(),
     fnrMcEvents(-1),
     fnrPairEvents(-1),
     fnrRecoEvents(-1),
     fnrTrackEvents(-1),
     fnrSingelEvents(-1),
     fwriteOutFile(NULL),
     fListTrackMatch(NULL),
     fListMCTracks(NULL),
     fListMCPointsMvd(NULL),
     fListMCPointsSts(NULL),
     fListMCPointsRich(NULL),
     fListMCPointsTrd(NULL),
     fListMCPointsTof(NULL),
     fListPairs(NULL),
     fListCandidates(NULL),
     fListKaons(NULL),
     fListPions(NULL),
     fListReco(NULL),
     fListGlobal(NULL),
     fKaonTrackArray(NULL),
     fPionTrackArray(NULL),
     fPrimVtxs(NULL),
     mcMomentum(NULL),
     mcMomentumMvd(NULL),
     mcMomentumMvdTrack(NULL),
     mcMomentumMvdStsTrack(NULL),
     mcMomentumMvdStsTrackPID(NULL),
     mcMomentumStsTrackMvdVertex(NULL),
     mcMomentumKaonTrackable(NULL),
     mcMomentumKaonTrackPID(NULL),
     mcMomentumPion1Trackable(NULL),
     mcMomentumPion1TrackPID(NULL),
     mcMomentumSignal(NULL),
     mcMomentumSignalPID(NULL),
     diffRecoSVMCSV(NULL),
     diffRecoSVMCSV1GeV(NULL),
     chi2vsDiffVertex(NULL),
     pairMomentum(NULL),
     massSpectra(NULL),
     massSpectraSVZ3H(NULL),
     massSpectraChi23H(NULL),
     massSpectraSVZ4H(NULL),
     massSpectraChi24H(NULL),
     chi2RecoD0(NULL),
     zPosDecay(NULL),
     secondaryVertexRes(NULL),
     secondaryVertexResmin3MVD(NULL),
     secondaryVertexRes3MVD(NULL),
     secondaryVertexRes4MVD(NULL),
     iPRadiusD0(NULL),
     chi2RecoD0PostCut(NULL),
     zPosDecayPostCut(NULL),
     secondaryVertexResPostCut(NULL),
     secondaryVertexResPostCut3H(NULL),
     secondaryVertexResPostCutex3H(NULL),
     secondaryVertexResPostCut4H(NULL),
     secVertexResChiCut(NULL),
     secVertexResChiCutSVZcut(NULL),
     secVertexResChiCutSVZcut4MVD(NULL),
     iPRadiusD0PostCut(NULL),
     chi2singelTrackBadD0(NULL),
     iPsingelTrackBadD0(NULL),
     nrMvdHitsBadD0(NULL),
     ptY(NULL),
     chi2IP(NULL),
     recoMomdist(NULL),
     recoMomdistAll(NULL),
     recoMomdistmin3Hit(NULL),
     recoMomdistmin4Hit(NULL),
     recoMomdistSecVCut(NULL),
     recoMomdistSecVCutmin4Hit(NULL),
     recoSigma(NULL),
     recoPionSigma(NULL),
     recoKaonSigma(NULL),
     recoPionSigmaSec(NULL),
     recoKaonSigmaSec(NULL),
     recoIPAll(NULL),
     recoIP4Perfect(NULL),
     recoIPfirstWrong(NULL),
     pionIP(NULL),
     kaonIP(NULL),
     momKaon(NULL),
     momPion(NULL),
     chi2NDFKaon(NULL),
     chi2NDFPion(NULL),
     kaonPtY(NULL),
     pionPtY()
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
void CbmD0HistogramManager::SetHistogramChois(TString /*group*/)
{

}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmD0HistogramManager::SetPostCuts(Float_t Chi2SingelCut, Float_t PTCut, Float_t SvZCut, Float_t PZCut)
{
    fchi2Singel = Chi2SingelCut;
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

    fEventNr = 0;

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

    fListGlobal = new TClonesArray("CbmGlobalTrack", 100);
    fListReco = new TClonesArray("CbmStsTrack", 100);
    fPrimVtxs = new CbmVertex();
    fListTrackMatch = new TClonesArray("CbmTrackMatchNew", 100);

    fListGlobal = (TClonesArray*)ioman->GetObject("GlobalTrack");
    fListReco = (TClonesArray*)ioman->GetObject("StsTrack");
    fPrimVtxs =  (CbmVertex*) ioman->GetObject("PrimaryVertex");
    fListTrackMatch = (TClonesArray*) ioman->GetObject("StsTrackMatch");

    recoMomdist = new TH1F("RecoMomentumDist", "Momentumdistribution Reconstructed Kaon and Pion Tracks", 100, 0, 10);
    recoMomdistAll = new TH1F("RecoMomentumDistAll", "Momentumdistribution Reconstructed all Tracks", 100, 0, 10);
    recoMomdistmin3Hit = new TH1F("RecoMomentumDistmin3Hit", "Momentumdistribution Reconstructed Tracks with min 3 Hits", 100, 0, 10);
    recoMomdistmin4Hit = new TH1F("RecoMomentumDistmin4Hit", "Momentumdistribution Reconstructed Tracks with min 4 Hits", 100, 0, 10);
    recoMomdistSecVCut = new TH1F("RecoMomentumDistSecVCut", "Momentumdistribution Reconstructed Tracks after secondary vertex cut", 100, 0, 10);
    recoMomdistSecVCutmin4Hit = new TH1F("RecoMomentumDistSecVCutmin4Hit", "Momentumdistribution Reconstructed Tracks after secondary vertex cut and min 4 Hits", 100, 0, 10);


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

    fKaonTrackArray     = new TClonesArray("CbmD0TrackCandidate", 100);
    fKaonTrackArray     = (TClonesArray*) ioman->GetObject("CbmD0KaonTrackArray");

    fPionTrackArray     = new TClonesArray("CbmD0TrackCandidate", 100);
    fPionTrackArray     = (TClonesArray*) ioman->GetObject("CbmD0PionTrackArray");

    momKaon = new TH1F("KaonMomentumDist", "Momentumdistribution Reconstructed Kaons", 100, 0, 30);
    momPion = new TH1F("PionMomentumDist", "Momentumdistribution Reconstructed Pions", 100, 0, 30);

    kaonPtY = new TH2F("PtYDistKaon", "Pt vs Rapidity reconstructed Kaons",1000, -3, 3, 1000, 0, 3);
    pionPtY = new TH2F("PtYDistPion", "Pt vs Rapidity reconstructed Pions",1000, -3, 3, 1000, 0, 3);

    pionIP = new TH1F("PionIPRadius", "Impact Radius Reconstructed Pions", 100, 0, 0.3);
    kaonIP = new TH1F("KaonIPRadius", "Impact Radius Reconstructed Kaons", 100, 0, 0.3);

    chi2NDFKaon = new TH1F("Chi2NDFKaon", "Chi squar over NDF of reconstructed Kaons", 100, 0, 20);
    chi2NDFPion = new TH1F("Chi2NDFPion", "Chi squar over NDF of reconstructed Pions", 100, 0, 20);


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
    massSpectraSVZ3H= new TH1F("MassSpectraD0SVZ3H","Reconstructed Mass D0, with SVZ and min 3 MVD Hit cut", 100, 0, 3);
    massSpectraChi23H= new TH1F("MassSpectraD0Chi23H","Reconstructed Mass D0, SVZ, chi2/ndf and min 3 MVD Hit cut", 100, 0, 3);
    massSpectraSVZ4H= new TH1F("MassSpectraD0SVZ4H","Reconstructed Mass D0, with SVZ and 4 MVD Hit cut", 100, 0, 3);
    massSpectraChi24H= new TH1F("MassSpectraD0Chi24H","Reconstructed Mass D0, SVZ, chi2/ndf and 4 MVD Hit cut", 100, 0, 3);


    ptY= new TH2F("PtYDistD0", "Pt vs Rapidity reconstructed D0",1000, -3, 3, 1000, 0, 3);
    chi2IP= new TH2F("Chi2vsIPD0"," Chi2 vs IP reconsturcted D0", 1000, 0, 1 , 1000, 0, 0.005);

    chi2RecoD0= new TH1F("Chi2RecoD0", "Chi2 of reconstructed D0", 1000, 0, 10);
    zPosDecay= new TH1F("ZPosDecay", "Z Position of D0 decay", 2000, 0, 2);
    secondaryVertexRes = new TH1F("SecondaryVertexRes", "Secondary Vertexresolution", 2000, -0.5, 0.5);
    secondaryVertexResmin3MVD = new TH1F("SecondaryVertexResmin3H", "Secondary Vertexresolution minimum 3 MVD hits", 2000, -0.5, 0.5);
    secondaryVertexRes3MVD = new TH1F("SecondaryVertexResex3H", "Secondary Vertexresolution exectly 3 MVD hits", 2000, -0.5, 0.5);
    secondaryVertexRes4MVD = new TH1F("SecondaryVertexRes4H", "Secondary Vertexresolution 4 MVD hits", 2000, -0.5, 0.5);
    iPRadiusD0= new TH1F("IPRadiusD0", "Impact Radius of D0", 1000, 0, 0.15);

    chi2RecoD0PostCut= new TH1F("Chi2RecoD0PostCut", "Chi2 of reconstructed D0, post cuts applied", 1000, 0, 10);
    zPosDecayPostCut= new TH1F("ZPosDecayPostCut", "Z Position of D0 decay, post cuts applied", 2000, 0, 2);

    secondaryVertexResPostCut = new TH1F("SecondaryVertexResPostCut", "Secondary Vertexresolution, SVZ cut applied", 2000, -0.5, 0.5);
    secondaryVertexResPostCut3H = new TH1F("SecondaryVertexResPostCut3H", "Secondary Vertexresolution, SVZ and min 3 MVD hit cut applied", 2000, -0.5, 0.5);
    secondaryVertexResPostCutex3H = new TH1F("SecondaryVertexResPostCutex3H", "Secondary Vertexresolution, SVZ and 3 MVD hit cut applied", 2000, -0.5, 0.5);
    secondaryVertexResPostCut4H = new TH1F("SecondaryVertexResPostCut4H", "Secondary Vertexresolution, SVZ and 4 MVD hit cut applied", 2000, -0.5, 0.5);

    secVertexResChiCut= new TH1F("SecVertexResChiCut", "Secondary Vertexresolution, Chi2 over NDF cut on kaon and pion Candidate", 2000, -0.5, 0.5);
    secVertexResChiCutSVZcut= new TH1F("SecVertexResChiCutSVZcut", "Secondary Vertexresolution, Chi2 over NDF cut on kaon and pion Candidate, SVZ cut", 2000, -0.5, 0.5);
    secVertexResChiCutSVZcut4MVD= new TH1F("SecVertexResChiCutSVZcut4MVD", "Secondary Vertexresolution, Chi2 over NDF cut on kaon and pion Candidate, SVZ and 4 MVD cut", 2000, -0.5, 0.5);

    iPRadiusD0PostCut= new TH1F("IPRadiusD0PostCut", "Impact Radius of D0, post cuts applied", 1000, 0, 0.15);

    nrMvdHitsBadD0= new TH1F("NrMvdHitsBadD0", "Nr of mvd hits in bad reconstructed D0", 20, 0, 5);
    iPsingelTrackBadD0= new TH1F("IPsingelTrackBadD0", "IP of tracks in bad reconstructed D0", 1000, 0,0.3);
    chi2singelTrackBadD0= new TH1F("chi2singelTrackBadD0", "Chi2 of tracks in bad reconstructed D0", 1000, 0, 10);

}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
void CbmD0HistogramManager::InitMc()
{
     LOG(INFO)<<"Init MC Histogramms"<< FairLogger::endl;

    fListMCTracks = new TClonesArray("CbmMCTrack", 1000);
    fListMCTracks = (TClonesArray*)ioman->GetObject("MCTrack");

    mcMomentum = new TH1F("MCPointMomentumDist", "Momentumdistribution MC Tracks", 100, 0, 10);        // alkonsl mc points

   mcMomentumMvd = new TH1F("MCPointMomentumDistMvd", "Momentumdistribution MC Track in MVD acceptance", 100 , 0, 10);   // MVD-MCPoints > 0
   mcMomentumMvdTrack = new TH1F("MCPointMomentumDistMvdTrackable", "Momentumdistribution MC Track in MVD trackable", 100 , 0, 10);   // MVD-Points >= 3
   mcMomentumMvdStsTrack = new TH1F("MCPointMomentumDistMvdStsTrackable", "Momentumdistribution MC Track in MVD and STS trackable", 100 , 0, 10);          // MVD + STS MC Points >= 3
   mcMomentumMvdStsTrackPID = new TH1F("MCPointMomentumDistMvdStsTrackablePID", "Momentumdistribution MC Track in MVD and STS trackable, Pions and Kaons only", 100 , 0, 10);          // MVD + STS MC Points >= 3
   mcMomentumStsTrackMvdVertex= new TH1F("MCPointMomentumDistStsTrackMvdVertex", "Momentumdistribution MC Track in STS trackable with Vertex information from MVD", 100 , 0, 10); // STS MC Points >= 3 MVD MC Points >= 2

   mcMomentumKaonTrackable= new TH1F("MCSignalMomentumDistTrackableKaon", "Momentumdistribution of embedted Signal Trackable Kaon", 100, 0, 10);
   mcMomentumKaonTrackPID= new TH1F("MCSignalMomentumDistTrackableKaonPID", "Momentumdistribution of embedted Signal Trackable Kaon and PID", 100, 0, 10);
              
  mcMomentumPion1Trackable = new TH1F("MCSignalMomentumDistTrackablePion1", "Momentumdistribution of embedted Signal Trackable Pion 1", 100, 0, 10);
 mcMomentumPion1TrackPID = new TH1F("MCSignalMomentumDistTrackablePion1PID", "Momentumdistribution of embedted Signal Trackable Pion 1 and PID", 100, 0, 10);

  mcMomentumSignal = new TH1F("MCSignalMomentumDistTrackable", "Momentumdistribution of embedted Signal trackable ", 100, 0, 10);
  mcMomentumSignalPID = new TH1F("MCSignalMomentumDistTrackablePID", "Momentumdistribution of embedted Signal trackable and PID", 100, 0, 10);

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
void CbmD0HistogramManager::Exec(Option_t*)
{
    LOG(INFO)<<"Executing Event Nr "<<fEventNr << FairLogger::endl;
    fEventNr++;

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
     mcMomentumMvdStsTrackPID->Write();

     diffRecoSVMCSV->Write();
     diffRecoSVMCSV1GeV->Write();
     chi2vsDiffVertex->Write();
     pairMomentum->Write();


     massSpectra->Write();
     massSpectraSVZ3H->Write();
     massSpectraChi23H->Write();
     massSpectraSVZ4H->Write();
     massSpectraChi24H->Write();

     chi2RecoD0->Write();
     zPosDecay->Write();


     secondaryVertexRes->Write();
     secondaryVertexResmin3MVD->Write();
     secondaryVertexRes3MVD->Write();
     secondaryVertexRes4MVD->Write();
     secondaryVertexResPostCut->Write();
     secondaryVertexResPostCut3H->Write();
     secondaryVertexResPostCut4H->Write();

     iPRadiusD0->Write();
     ptY->Write();
     chi2IP->Write();

     recoMomdistAll->Write();
     recoMomdist->Write();
     recoMomdistmin3Hit->Write();
     recoMomdistmin4Hit->Write();
     recoMomdistSecVCut->Write();
     recoMomdistSecVCutmin4Hit->Write();

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
     chi2NDFKaon->Write();
     chi2NDFPion->Write();

     chi2RecoD0PostCut->Write();
     zPosDecayPostCut->Write();

     iPRadiusD0PostCut->Write();
     chi2singelTrackBadD0->Write();
     iPsingelTrackBadD0->Write();
     nrMvdHitsBadD0->Write();

     secVertexResChiCut->Write();
     secVertexResChiCutSVZcut->Write();
     secVertexResChiCutSVZcut4MVD->Write();
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

    //   if(mcTrack->GetNPoints(kMVD) > 0)
     //    {
	     mcMomentumMvd->Fill(mcTrack->GetP());
	 }
        if(mcTrack->GetNPoints(kMVD)>=3)
         {
             mcMomentumMvdTrack->Fill(mcTrack->GetP());
         }
        if((mcTrack->GetNPoints(kMVD) + mcTrack->GetNPoints(kSTS)) >= 3)
         {
           mcMomentumMvdStsTrack->Fill(mcTrack->GetP());
           if(mcTrack->GetPdgCode() == -321 || mcTrack->GetPdgCode() == 211) mcMomentumMvdStsTrackPID->Fill(mcTrack->GetP());

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



      // }
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


    KFParticle openCharm, kaon, pion;
    CbmStsTrack kaonTrack, pionTrack;
    CbmD0Candidate OCcandidate;
    CbmD0TrackCandidate kaonCand;
    CbmD0TrackCandidate pionCand;
    CbmGlobalTrack* globalTrackKaon;
    CbmGlobalTrack* globalTrackPion;
    Int_t kaonGlobID, pionGlobID;


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

	    kaonCand = *((CbmD0TrackCandidate*)fKaonTrackArray->At(OCcandidate.ftrack1));
            kaon = *((KFParticle*)fListKaons->At(OCcandidate.ftrack1));
	    kaonGlobID = kaonCand.GetTrackIndex();

	    pionCand = *((CbmD0TrackCandidate*)fPionTrackArray->At(OCcandidate.ftrack2));
            pion = *((KFParticle*)fListPions->At(OCcandidate.ftrack2));
	    pionGlobID = pionCand.GetTrackIndex();

	    globalTrackKaon = (CbmGlobalTrack*) fListGlobal->At(kaonGlobID);
	    globalTrackPion = (CbmGlobalTrack*) fListGlobal->At(pionGlobID);

            if(globalTrackKaon->GetStsTrackIndex()!= -1)
              kaonTrack    =*((CbmStsTrack*) fListReco->At(globalTrackKaon->GetStsTrackIndex()));
            else continue;

            if(globalTrackPion->GetStsTrackIndex()!= -1)
              pionTrack    =*((CbmStsTrack*) fListReco->At(globalTrackPion->GetStsTrackIndex()));
	    else continue;

            Float_t ip1 =  GetIPRadius(globalTrackKaon);
            Float_t ip2 =  GetIPRadius(globalTrackPion);

	    chi2RecoD0->Fill(openCharm.GetChi2());
	    zPosDecay->Fill(openCharm.GetZ());
            secondaryVertexRes->Fill(openCharm.GetZ());
     
            if(kaonTrack.GetNofMvdHits()>2 && pionTrack.GetNofMvdHits()>2) secondaryVertexResmin3MVD->Fill(openCharm.GetZ());
            if(kaonTrack.GetNofMvdHits()==3 && pionTrack.GetNofMvdHits()==3)secondaryVertexRes3MVD->Fill(openCharm.GetZ());
            if(kaonTrack.GetNofMvdHits()>3 && pionTrack.GetNofMvdHits()>3) secondaryVertexRes4MVD->Fill(openCharm.GetZ());


	    pairMomentum->Fill(openCharm.GetP());
	    massSpectra->Fill(openCharm.GetMass());
	    ptY->Fill(openCharm.GetY(), openCharm.GetPt());

            openCharm.TransportToPoint(&*fvtx);
	    Double_t Pair_IP  = sqrt((openCharm.GetX()*openCharm.GetX()) + (openCharm.GetY()*openCharm.GetY()) );
	    iPRadiusD0->Fill(Pair_IP);

	    chi2IP->Fill(openCharm.GetChi2(), Pair_IP);

	    if(openCharm.GetZ()>fcutSvZ )
	    {
                chi2RecoD0PostCut->Fill(openCharm.GetChi2());
		iPRadiusD0PostCut->Fill(Pair_IP);

		chi2singelTrackBadD0->Fill(kaonTrack.GetChiSq());
		chi2singelTrackBadD0->Fill(pionTrack.GetChiSq());

		iPsingelTrackBadD0->Fill(ip1);
		iPsingelTrackBadD0->Fill(ip2);

		nrMvdHitsBadD0->Fill(kaonTrack.GetNofMvdHits());
		nrMvdHitsBadD0->Fill(pionTrack.GetNofMvdHits());

		recoMomdistSecVCut->Fill(kaon.GetP());
		recoMomdistSecVCut->Fill(pion.GetP());

		secondaryVertexResPostCut->Fill(openCharm.GetZ());          

               if(kaonTrack.GetNofMvdHits()>2 && pionTrack.GetNofMvdHits()>2)
	       {
		   secondaryVertexResPostCut3H->Fill(openCharm.GetZ());
		   massSpectraSVZ3H->Fill(openCharm.GetMass());
	       }
                if(kaonTrack.GetNofMvdHits()==3 && pionTrack.GetNofMvdHits()==3) secondaryVertexResPostCutex3H->Fill(openCharm.GetZ());
		if(kaonTrack.GetNofMvdHits()>3 && pionTrack.GetNofMvdHits()>3)
		{
		    secondaryVertexResPostCut4H->Fill(openCharm.GetZ());
	            recoMomdistSecVCutmin4Hit->Fill(kaon.GetP());
		    recoMomdistSecVCutmin4Hit->Fill(pion.GetP());
                    massSpectraSVZ4H->Fill(openCharm.GetMass());
		}



	    }

	    if((kaon.GetChi2()/kaon.GetNDF()) < fchi2Singel && (pion.GetChi2()/pion.GetNDF()) < fchi2Singel)
	    {
		secVertexResChiCut->Fill(openCharm.GetZ());
                 if(openCharm.GetZ()>fcutSvZ )
	            {
			secVertexResChiCutSVZcut->Fill(openCharm.GetZ());
                       if(kaonTrack.GetNofMvdHits()>2 && pionTrack.GetNofMvdHits()>2)  massSpectraChi23H->Fill(openCharm.GetMass());
     

                if(kaonTrack.GetNofMvdHits()>3 && pionTrack.GetNofMvdHits()>3)
		{
		    secVertexResChiCutSVZcut4MVD->Fill(openCharm.GetZ());
                    massSpectraChi24H->Fill(openCharm.GetMass());
		}
		    }

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
    Int_t trackMatchNr;
    CbmTrackMatchNew* trackMatch;
    CbmStsTrack* track;
    CbmMCTrack* mcTrack = NULL;


	for(Int_t i = 0; i < fListReco->GetEntriesFast(); i++)
	{
	    CbmGlobalTrack* glTrack = (CbmGlobalTrack*)fListGlobal->At(i);

	    if(glTrack->GetStsTrackIndex()!= -1)
	    {
		track = (CbmStsTrack*)fListReco->At(glTrack->GetStsTrackIndex());
	        trackMatch = (CbmTrackMatchNew*)fListTrackMatch->At(glTrack->GetStsTrackIndex());
	        trackMatchNr = trackMatch->GetMatchedLink().GetIndex();
		if(trackMatchNr > -1)
		    mcTrack = (CbmMCTrack*)fListMCTracks->At(trackMatchNr);

	    }
	    else continue;

	    TVector3 momentum;
	    track->GetParamFirst()->Momentum(momentum);
	    Float_t P = sqrt(momentum.X() * momentum.X() +momentum.Y() * momentum.Y() +momentum.Z() * momentum.Z());

	    recoMomdistAll->Fill(P);

            if(mcTrack)
	    {
	      	Float_t mcP = mcTrack->GetP();
		recoSigma->Fill( mcP, (P - mcP) / mcP);

		if(mcTrack->GetPdgCode() == -321)
		{
                    recoMomdist->Fill(P);
                    if(track->GetNofMvdHits()>2) recoMomdistmin3Hit->Fill(P);
		    if(track->GetNofMvdHits()>3) recoMomdistmin4Hit->Fill(P);

		    recoKaonSigma->Fill(mcP, (P - mcP) / mcP);
		    if(mcTrack->GetStartZ() != 0.0)
                        recoKaonSigmaSec->Fill(mcP, (P - mcP) / mcP);
		}
		if(mcTrack->GetPdgCode() == 211)
		{
		    recoMomdist->Fill(P);
                    if(track->GetNofMvdHits()>2) recoMomdistmin3Hit->Fill(P);
		    if(track->GetNofMvdHits()>3) recoMomdistmin4Hit->Fill(P);

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
            chi2NDFPion->Fill(pion.GetChi2()/pion.GetNDF());

            pion.TransportToPoint(&*fvtx);
	    Double_t IP  = sqrt((pion.GetX()*pion.GetX()) + (pion.GetY()*pion.GetY()) );
            pionIP->Fill(IP);
	}

        	for(Int_t j = 0; j < fListKaons->GetEntriesFast(); j++)
	{
	    kaon = *((KFParticle*)fListKaons->At(j));
            momKaon->Fill(kaon.GetP());
	    kaonPtY->Fill(kaon.GetY(), kaon.GetPt());
            chi2NDFKaon->Fill(kaon.GetChi2()/ kaon.GetNDF());

            kaon.TransportToPoint(&*fvtx);
	    Double_t IP  = sqrt((kaon.GetX()*kaon.GetX()) + (kaon.GetY()*kaon.GetY()) );
            kaonIP->Fill(IP);
	}



}
// -------------------------------------------------------------------------

Float_t CbmD0HistogramManager::GetIPRadius(CbmGlobalTrack* track)
{


    Float_t impactParam = sqrt((track->GetParamVertex()->GetX() *track->GetParamVertex()->GetX()) + (track->GetParamVertex()->GetY() *track->GetParamVertex()->GetY()));

    return impactParam;

}

// -------------------------------------------------------------------------
ClassImp(CbmD0HistogramManager)
