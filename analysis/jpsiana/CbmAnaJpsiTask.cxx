
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
#include "CbmAnaJpsiCuts.h"



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
	  fJpsiCandidates(NULL),
	  fPrimVertex(NULL),
	  fKFVertex(),
	  fCandidates(),
	  fElIdAnn(NULL),
	  fHM(NULL),
	  fCuts()
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

   fJpsiCandidates = new TClonesArray("CbmAnaJpsiCandidate");
   ioman->Register("JpsiCandidates","Jpsi", fJpsiCandidates, kTRUE);

   InitHist();

   if (fCuts.fUseRichAnn){
      fElIdAnn = new CbmRichElectronIdAnn();
      fElIdAnn->Init();
   }

   return kSUCCESS;
}


void CbmAnaJpsiTask::CreateAnalysisStepsH1(
      const string& name,
      const string& axisX,
      const string& axisY,
      double nBins,
      double min,
      double max
      )
{
   for (Int_t i = 0; i < CbmAnaJpsiHist::fNofAnaSteps; i++)
   {
      string hname = name + "_"+ CbmAnaJpsiHist::fAnaSteps[i];
      fHM->Create1<TH1D>(hname, hname+";"+axisX+";"+axisY, nBins, min, max);
   }
}

void CbmAnaJpsiTask::CreateAnalysisStepsH2(
      const string& name,
      const string& axisX,
      const string& axisY,
      const string& axisZ,
      double nBinsX,
      double minX,
      double maxX,
      double nBinsY,
      double minY,
      double maxY)
{
   for (Int_t i = 0; i < CbmAnaJpsiHist::fNofAnaSteps; i++)
   {
      string hname = name + "_"+ CbmAnaJpsiHist::fAnaSteps[i];
      fHM->Create2<TH2D>(hname, hname+";"+axisX+";"+axisY+";"+axisZ, nBinsX, minX, maxX, nBinsY, minY, maxY);
   }
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

void CbmAnaJpsiTask::CreateSourceTypesH2(
      const string& name,
      const string& axisX,
      const string& axisY,
      const string& axisZ,
      double nBinsX,
      double minX,
      double maxX,
      double nBinsY,
      double minY,
      double maxY)
{
   string hname = "";
   for (Int_t i = 0; i < CbmAnaJpsiHist::fNofSourceTypes; i++){
      hname = name + "_"+ CbmAnaJpsiHist::fSourceTypes[i];
      fHM->Create2<TH2D>(hname, hname+";"+axisX+";"+axisY+";"+axisZ, nBinsX, minX, maxX, nBinsY, minY, maxY);
   }
}

void CbmAnaJpsiTask::InitHist()
{
	fHM = new CbmHistManager();

	// Event number counter
	fHM->Create1<TH1D>("fh_event_number", "fh_event_number;a.u.;Number of events", 1, 0, 1.);

   //RICH PMT plane XY
   CreateSourceTypesH2("fh_rich_pmt_xy","X [cm]","Y [cm]","Hits/cm^{2}/event", 220, -110, 110, 400, -200, 200);

   //distributions of ID and analysis cuts
   CreateSourceTypesH1("fh_track_chi2prim","#chi^{2}_{prim}", "Yield", 200, 0., 20.);
   CreateSourceTypesH1("fh_track_mom","P [GeV/c]", "Yield", 160, 0., 16.);
   CreateSourceTypesH1("fh_track_chi2sts","#chi^{2}_{STS}", "Yield", 80, 0., 8.);
   CreateSourceTypesH1("fh_track_rapidity","Rapidity", "Yield", 40, 0., 4.);
   CreateSourceTypesH1("fh_track_pt","P_{t} [GeV/c]", "Yield", 40, 0., 4.);
   CreateSourceTypesH1("fh_track_rich_ann","RICH ANN output", "Yield", 120, -1.2, 1.2);
   CreateSourceTypesH1("fh_track_trd_ann","TRD ANN output", "Yield", 120, -1.2, 1.2);
   CreateSourceTypesH2("fh_track_tof_m2","P [GeV/c]", "m^{2} [GeV/c^{2}]^{2}","Yield", 900, 0., 9., 360, -0.4, 1.4);

   //vertex of the secondary electrons from gamma conversion
   fHM->Create2<TH2D>("fh_vertex_el_gamma_xz","fh_vertex_el_gamma_xz;Z [cm];X [cm];Counter per event", 200, -10., 190., 400, -130.,130.);
   fHM->Create2<TH2D>("fh_vertex_el_gamma_yz","fh_vertex_el_gamma_yz;Z [cm];Y [cm];Counter per event", 200, -10., 190., 400, -130., 130.);
   fHM->Create2<TH2D>("fh_vertex_el_gamma_xy","fh_vertex_el_gamma_xy;X [cm];Y [cm];Counter per event",  400, -130.,130., 400, -130., 130.);
   fHM->Create2<TH2D>("fh_vertex_el_gamma_rz","fh_vertex_el_gamma_rz;Z [cm];#sqrt{X^{2}+Y^{2}} [cm];Counter per event",  300, -10., 190., 300, 0., 150.);

   // Number of BG and signal tracks after each cut
   fHM->Create1<TH1D>("fh_nof_bg_tracks","fh_nof_bg_tracks;Analysis steps;Number of BG tracks/event", CbmAnaJpsiHist::fNofAnaSteps, 0., CbmAnaJpsiHist::fNofAnaSteps);
   fHM->Create1<TH1D>("fh_nof_el_tracks","fh_nof_el_tracks;Analysis steps;Number of signal tracks/event", CbmAnaJpsiHist::fNofAnaSteps, 0., CbmAnaJpsiHist::fNofAnaSteps);
   fHM->Create2<TH2D>("fh_source_tracks","fh_source_tracks;Analysis steps;Particle", CbmAnaJpsiHist::fNofAnaSteps, 0., CbmAnaJpsiHist::fNofAnaSteps, 7, 0., 7.);

   //Create invariant mass histograms
   CreateAnalysisStepsH1("fh_signal_minv", "M_{ee} [GeV/c^{2}]", "Yield", 1800, 2.0 , 3.8);
   CreateAnalysisStepsH1("fh_bg_minv", "M_{ee} [GeV/c^{2}]", "Yield", 4000, 0. , 4.);
   CreateAnalysisStepsH1("fh_pi0_minv", "M_{ee} [GeV/c^{2}]", "Yield", 600, 0. , 0.6);
   CreateAnalysisStepsH1("fh_gamma_minv", "M_{ee} [GeV/c^{2}]", "Yield", 800, 0. , 0.8);

   // minv for true matched and mismatched tracks
   CreateAnalysisStepsH1("fh_bg_truematch_minv", "M_{ee} [GeV/c^{2}]", "Yield", 4000, 0. , 4.);
   CreateAnalysisStepsH1("fh_bg_truematch_el_minv", "M_{ee} [GeV/c^{2}]", "Yield", 4000, 0. , 4.);
   CreateAnalysisStepsH1("fh_bg_truematch_notel_minv", "M_{ee} [GeV/c^{2}]", "Yield", 4000, 0. , 4.);
   CreateAnalysisStepsH1("fh_bg_mismatch_minv", "M_{ee} [GeV/c^{2}]", "Yield", 4000, 0. , 4.);

   //Invariant mass vs. Mc Pt
   CreateAnalysisStepsH2("fh_signal_minv_pt","M_{ee} [GeV/c^{2}]", "P_{t} [GeV/c]", "Yield", 100, 0., 4., 20, 0., 2.);

   // Momentum distribution of the signal
   CreateAnalysisStepsH1("fh_signal_mom", "P [GeV/c]", "Particles per event", 250, 0., 25.);
   //Pt/y distibution of the signal
   CreateAnalysisStepsH2("fh_signal_pty","Rapidity", "P_{t} [GeV/c]", "Particles per event", 40, 0., 4., 20, 0., 2.);

   //Number of mismatches after each cut
   fHM->Create1<TH1D>("fh_nof_mismatches","fh_nof_mismatches;Analysis steps;Tracks/event", CbmAnaJpsiHist::fNofAnaSteps, 0., CbmAnaJpsiHist::fNofAnaSteps);
   fHM->Create1<TH1D>("fh_nof_mismatches_rich","fh_nof_mismatches_rich;Analysis steps;Tracks/event", CbmAnaJpsiHist::fNofAnaSteps, 0., CbmAnaJpsiHist::fNofAnaSteps);
   fHM->Create1<TH1D>("fh_nof_mismatches_trd","fh_nof_mismatches_trd;Analysis steps;Tracks/event", CbmAnaJpsiHist::fNofAnaSteps, 0., CbmAnaJpsiHist::fNofAnaSteps);
   fHM->Create1<TH1D>("fh_nof_mismatches_tof","fh_nof_mismatches_tof;Analysis steps;Tracks/event", CbmAnaJpsiHist::fNofAnaSteps, 0., CbmAnaJpsiHist::fNofAnaSteps);
}


void CbmAnaJpsiTask::Exec(
      Option_t* option)
{
	fJpsiCandidates->Clear();

   fHM->H1("fh_event_number")->Fill(0.5);

   fEventNum++;
   cout << "CbmAnaJpsiTask, event No. " <<  fEventNum << endl;
   
   if (fPrimVertex != NULL){
      fKFVertex = CbmKFVertex(*fPrimVertex);
   } else {
      Fatal("CbmAnaJpsiTask::Exec","No PrimaryVertex array!");
   }

  if (fPrimVertex != NULL){
     fKFVertex = CbmKFVertex(*fPrimVertex);
  } else {
     Fatal("CbmAnaDielectronTask::Exec","No PrimaryVertex array!");
  }

  MCPairs();

  RichPmtXY();

  FillCandidates();

  AssignMcToCandidates();

  DifferenceSignalAndBg();

  SingleParticleAcceptance();

  PairMcAndAcceptance();

  SignalAndBgReco();

  CopyCandidatesToOutputArray();

}

void CbmAnaJpsiTask::MCPairs()
{
    Int_t nMcTracks = fMcTracks->GetEntries();
    for (Int_t i = 0; i < nMcTracks; i++){
        CbmMCTrack* mctrack = (CbmMCTrack*) fMcTracks->At(i);
        Bool_t isMcGammaElectron = CbmAnaJpsiUtils::IsMcGammaElectron(mctrack, fMcTracks);
        if (isMcGammaElectron) {
			TVector3 v;
			mctrack->GetStartVertex(v);
			fHM->H2("fh_vertex_el_gamma_xz")->Fill(v.Z(),v.X());
			fHM->H2("fh_vertex_el_gamma_yz")->Fill(v.Z(),v.Y());
			fHM->H2("fh_vertex_el_gamma_xy")->Fill(v.X(),v.Y());
			fHM->H2("fh_vertex_el_gamma_rz")->Fill( v.Z(), sqrt(v.X()*v.X()+v.Y()*v.Y()) );
        }
    } // nMcTracks
} //MC Pairs

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

        IsElectron(richRing, cand.fMomentum.Mag(), trdTrack, globalTrack, &cand);

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
      fCandidates[i].fStsMcMotherId = mcTrack1->GetMotherId();
      fCandidates[i].fMcPdg = TMath::Abs(mcTrack1->GetPdgCode());
      fCandidates[i].fIsMcSignalElectron = CbmAnaJpsiUtils::IsMcSignalElectron(mcTrack1);
      fCandidates[i].fIsMcGammaElectron =  CbmAnaJpsiUtils::IsMcGammaElectron(mcTrack1, fMcTracks);
      fCandidates[i].fIsMcPi0Electron =  CbmAnaJpsiUtils::IsMcPi0Electron(mcTrack1, fMcTracks);

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
            fHM->H1("fh_track_chi2prim_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fChi2Prim);
            fHM->H1("fh_track_mom_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fMomentum.Mag());
            fHM->H1("fh_track_chi2sts_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fChi2sts);
            fHM->H1("fh_track_rapidity_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fRapidity);
            fHM->H1("fh_track_pt_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fMomentum.Perp());
            fHM->H1("fh_track_rich_ann_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fRichAnn);
            fHM->H1("fh_track_trd_ann_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fTrdAnn);
            fHM->H1("fh_track_tof_m2_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill(fCandidates[i].fMomentum.Mag(), fCandidates[i].fMass2);
        } else {
        	fHM->H1("fh_track_chi2prim_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fChi2Prim);
        	fHM->H1("fh_track_mom_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fMomentum.Mag());
        	fHM->H1("fh_track_chi2sts_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fChi2sts);
        	fHM->H1("fh_track_rapidity_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fRapidity);
        	fHM->H1("fh_track_pt_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fMomentum.Perp());
        	fHM->H1("fh_track_rich_ann_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fRichAnn);
            fHM->H1("fh_track_trd_ann_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fTrdAnn);
            fHM->H1("fh_track_tof_m2_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill(fCandidates[i].fMomentum.Mag(), fCandidates[i].fMass2);
        }
        if (fCandidates[i].fIsMcGammaElectron){
        	fHM->H1("fh_track_chi2prim_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fChi2Prim);
        	fHM->H1("fh_track_mom_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fMomentum.Mag());
        	fHM->H1("fh_track_chi2sts_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fChi2sts);
        	fHM->H1("fh_track_rapidity_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fRapidity);
        	fHM->H1("fh_track_pt_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fMomentum.Perp());
        	fHM->H1("fh_track_rich_ann_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fRichAnn);
            fHM->H1("fh_track_trd_ann_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fTrdAnn);
            fHM->H1("fh_track_tof_m2_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(fCandidates[i].fMomentum.Mag(), fCandidates[i].fMass2);
        }
        if (fCandidates[i].fIsMcPi0Electron) {
        	fHM->H1("fh_track_chi2prim_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fChi2Prim);
        	fHM->H1("fh_track_mom_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fMomentum.Mag());
        	fHM->H1("fh_track_chi2sts_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fChi2sts);
        	fHM->H1("fh_track_rapidity_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fRapidity);
        	fHM->H1("fh_track_pt_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fMomentum.Perp());
        	fHM->H1("fh_track_rich_ann_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fRichAnn);
            fHM->H1("fh_track_trd_ann_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fTrdAnn);
            fHM->H1("fh_track_tof_m2_" + CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(fCandidates[i].fMomentum.Mag(), fCandidates[i].fMass2);
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

void CbmAnaJpsiTask::SingleParticleAcceptance()
{
    Int_t nMcTracks = fMcTracks->GetEntries();
    for (Int_t i = 0; i < nMcTracks; i++) {
        CbmMCTrack* mctrack = (CbmMCTrack*) fMcTracks->At(i);
        if (CbmAnaJpsiUtils::IsMcSignalElectron(mctrack)){
        	fHM->H1("fh_nof_el_tracks")->Fill(kJpsiMc + 0.5);
        	if ( IsMcTrackAccepted(i) ) {
        		 fHM->H1("fh_nof_el_tracks")->Fill(kJpsiAcc + 0.5);
        	}
        }
    }
}

void CbmAnaJpsiTask::PairMcAndAcceptance()
{
	Int_t nMcTracks = fMcTracks->GetEntries();
	for (Int_t iP = 0; iP < nMcTracks; iP++) {
		CbmMCTrack* mctrackP = (CbmMCTrack*) fMcTracks->At(iP);
		Bool_t isMcSignalElectronP = CbmAnaJpsiUtils::IsMcSignalElectron(mctrackP);
		if (!isMcSignalElectronP) continue;
		Bool_t isAccP = IsMcTrackAccepted(iP);
		for (Int_t iM = 0; iM < nMcTracks; iM++) {
			if (iP == iM) continue;
			CbmMCTrack* mctrackM = (CbmMCTrack*) fMcTracks->At(iM);
			Bool_t isMcSignalElectronM = CbmAnaJpsiUtils::IsMcSignalElectron(mctrackM);
			if (!isMcSignalElectronM) continue;
			Bool_t isAccM = IsMcTrackAccepted(iM);
			CbmAnaJpsiKinematicParams p = CbmAnaJpsiKinematicParams::KinematicParamsWithMcTracks(mctrackP,mctrackM);

			if (isMcSignalElectronM && isMcSignalElectronP) {
				fHM->H2("fh_signal_pty_"+CbmAnaJpsiHist::fAnaSteps[kJpsiMc])->Fill(p.fRapidity, p.fPt);
				fHM->H1("fh_signal_mom_"+CbmAnaJpsiHist::fAnaSteps[kJpsiMc])->Fill(p.fMomentumMag);
				fHM->H1("fh_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[kJpsiMc])->Fill(p.fMinv);
				fHM->H2("fh_signal_minv_pt_"+CbmAnaJpsiHist::fAnaSteps[kJpsiMc])->Fill(p.fMinv, p.fPt);
				//accepted e+/-
				if (isAccP && isAccM) {
					//no FillPairHists because we have not declared KinematicParamCand jet
					fHM->H2("fh_signal_pty_"+CbmAnaJpsiHist::fAnaSteps[kJpsiAcc])->Fill(p.fRapidity, p.fPt);
					fHM->H1("fh_signal_mom_"+CbmAnaJpsiHist::fAnaSteps[kJpsiAcc])->Fill(p.fMomentumMag);
					fHM->H1("fh_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[kJpsiAcc])->Fill(p.fMinv);
					fHM->H2("fh_signal_minv_pt_"+CbmAnaJpsiHist::fAnaSteps[kJpsiAcc])->Fill(p.fMinv, p.fPt);
				}
			}
		}
	}
} // PairsAcceptance


void CbmAnaJpsiTask::TrackSource(
		CbmAnaJpsiCandidate* cand,
    	CbmAnaJpsiAnalysisSteps step,
		Int_t pdg)
{
	int binNum = (double) step + 0.5;
	Double_t mom = cand->fMomentum.Mag();
	Double_t pt = cand->fMomentum.Perp();


    if (cand->fIsMcSignalElectron) {
		fHM->H1("fh_nof_el_tracks")->Fill(binNum);
	} else {
		fHM->H1("fh_nof_bg_tracks")->Fill(binNum);

		if (IsMismatch(cand)) fHM->H1("fh_nof_mismatches")->Fill(binNum);
		if (cand->fStsMcTrackId != cand->fRichMcTrackId) fHM->H1("fh_nof_mismatches_rich")->Fill(binNum);
		if (cand->fStsMcTrackId != cand->fTrdMcTrackId) fHM->H1("fh_nof_mismatches_trd")->Fill(binNum);
		if (cand->fStsMcTrackId != cand->fTofMcTrackId) fHM->H1("fh_nof_mismatches_tof")->Fill(binNum);


  	    if (cand->fIsMcGammaElectron) {
  	  		fHM->H2("fh_source_tracks")->Fill(binNum, 0.5);
  	    } else if (cand->fIsMcPi0Electron) {
  	    	fHM->H2("fh_source_tracks")->Fill(binNum, 1.5);
  	    } else if (pdg == 211 || pdg ==-211) { //Pi+-
	    	fHM->H2("fh_source_tracks")->Fill(binNum, 2.5);
  	  	} else if (pdg == 2212) { //P
	    	fHM->H2("fh_source_tracks")->Fill(binNum, 3.5);
  	  	} else if (pdg == 321 || pdg == -321) {
    		fHM->H2("fh_source_tracks")->Fill(binNum, 4.5);
  	  	} else if ( (pdg == 11 || pdg == -11) && !cand->fIsMcGammaElectron && !cand->fIsMcPi0Electron && !cand->fIsMcSignalElectron) {
    		fHM->H2("fh_source_tracks")->Fill(binNum, 5.5);
  	    } else{
	    	fHM->H2("fh_source_tracks")->Fill(binNum, 6.5);
  	  	}
	}//Signal or not
}//TrackSource



void CbmAnaJpsiTask::FillPairHists(
	  CbmAnaJpsiCandidate* candP,
      CbmAnaJpsiCandidate* candM,
	  CbmAnaJpsiKinematicParams* parMc,
	  CbmAnaJpsiKinematicParams* parRec,
      CbmAnaJpsiAnalysisSteps step)
{
	Bool_t isSignal = candP->fIsMcSignalElectron && candM->fIsMcSignalElectron;
	Bool_t isPi0 = (candP->fIsMcPi0Electron && candM->fIsMcPi0Electron && candP->fStsMcMotherId == candM->fStsMcMotherId);
	Bool_t isGamma = (candP->fIsMcGammaElectron && candM->fIsMcGammaElectron && candP->fStsMcMotherId == candM->fStsMcMotherId);
    Bool_t isBG = (!isGamma) && (!isPi0) && (!(candP->fIsMcSignalElectron || candM->fIsMcSignalElectron));
	Bool_t isMismatch = (IsMismatch(candP) || IsMismatch(candM));

	if (isSignal) fHM->H2("fh_signal_pty_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parMc->fRapidity, parMc->fPt);
	if (isSignal) fHM->H1("fh_signal_mom_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parMc->fMomentumMag);
	if (isSignal) fHM->H1("fh_signal_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv);
	if (isSignal) fHM->H2("fh_signal_minv_pt_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv, parMc->fPt);
	if (isBG) fHM->H1("fh_bg_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv);
	//PairSource(candP, candM, step, parRec);
	if (isPi0) fHM->H1("fh_pi0_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv);
	if (isGamma) fHM->H1("fh_gamma_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv);
	if (isBG && isMismatch) fHM->H1("fh_bg_mismatch_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv);
	if (isBG && !isMismatch) {
		fHM->H1("fh_bg_truematch_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv);
		if (candP->fMcPdg == 11 && candM->fMcPdg == 11) fHM->H1("fh_bg_truematch_el_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv);
		if (candP->fMcPdg != 11 || candM->fMcPdg != 11) fHM->H1("fh_bg_truematch_notel_minv_"+CbmAnaJpsiHist::fAnaSteps[step])->Fill(parRec->fMinv);
	}
}


void CbmAnaJpsiTask::SignalAndBgReco()
{
	Int_t ncand = fCandidates.size();

	for (Int_t i=0;i<ncand;i++) {
		Int_t pdg = 0;
		if (fCandidates[i].fStsMcTrackId > 0) {
			CbmMCTrack* mcTrack = (CbmMCTrack*) fMcTracks->At(fCandidates[i].fStsMcTrackId);
			if (NULL != mcTrack) pdg = mcTrack->GetPdgCode();
		}

	     Bool_t isChi2Prim = (fCandidates[i].fChi2Prim < fCuts.fChiPrimCut);
	     Bool_t isEl = (fCandidates[i].fIsElectron);
	     Bool_t isPtCut = (fCandidates[i].fMomentum.Perp() > fCuts.fPtCut);

	     TrackSource(&fCandidates[i], kJpsiReco, pdg);
	     if (isChi2Prim) TrackSource(&fCandidates[i], kJpsiChi2Prim, pdg);
	     if (isChi2Prim && isEl) TrackSource(&fCandidates[i], kJpsiElId, pdg);
	     if (isChi2Prim && isEl && isPtCut) TrackSource(&fCandidates[i], kJpsiPtCut, pdg);
	}

	for (Int_t iP=0;iP<ncand;iP++)
	{
		if (fCandidates[iP].fCharge < 0) continue;
		CbmMCTrack* mctrackP = NULL;
		if (fCandidates[iP].fStsMcTrackId >=0) mctrackP = (CbmMCTrack*) fMcTracks->At(fCandidates[iP].fStsMcTrackId);

		for (Int_t iM=0;iM<ncand;iM++)
		{
			if (fCandidates[iM].fCharge > 0) continue;
			CbmMCTrack* mctrackM = NULL;
			if (fCandidates[iM].fStsMcTrackId >=0) mctrackM = (CbmMCTrack*) fMcTracks->At(fCandidates[iM].fStsMcTrackId);
			if (iM == iP ) continue;

			CbmAnaJpsiKinematicParams pMC;
			if (mctrackP != NULL && mctrackM != NULL) pMC = CbmAnaJpsiKinematicParams::KinematicParamsWithMcTracks(mctrackP, mctrackM);

			CbmAnaJpsiKinematicParams pRec = CbmAnaJpsiKinematicParams::KinematicParamsWithCandidates(&fCandidates[iP],&fCandidates[iM]);

		    Bool_t isChiPrimary = (fCandidates[iP].fChi2Prim < fCuts.fChiPrimCut && fCandidates[iM].fChi2Prim < fCuts.fChiPrimCut);
		    Bool_t isEl = (fCandidates[iP].fIsElectron && fCandidates[iM].fIsElectron);
		    Bool_t isPtCut = (fCandidates[iP].fMomentum.Perp() > fCuts.fPtCut && fCandidates[iM].fMomentum.Perp() > fCuts.fPtCut);

		    FillPairHists(&fCandidates[iP], &fCandidates[iM], &pMC, &pRec, kJpsiReco);
		    if (isChiPrimary)
		    {
		    	FillPairHists(&fCandidates[iP], &fCandidates[iM], &pMC, &pRec, kJpsiChi2Prim);
		    }
		    if (isChiPrimary && isEl)
		    {
		    	FillPairHists(&fCandidates[iP], &fCandidates[iM], &pMC, &pRec, kJpsiElId);
		    }
		    if (isChiPrimary && isEl && isPtCut)
		    {
		    	FillPairHists(&fCandidates[iP], &fCandidates[iM], &pMC, &pRec, kJpsiPtCut);
		    }
		}//iM
	}//iP
}

void CbmAnaJpsiTask::IsElectron(
		CbmRichRing* ring,
		Double_t momentum,
		CbmTrdTrack* trdTrack,
	    CbmGlobalTrack * gTrack,
		CbmAnaJpsiCandidate* cand)
{
	Bool_t richEl = IsRichElectron(ring, momentum, cand);
	Bool_t trdEl = (trdTrack != NULL)?IsTrdElectron(trdTrack, cand):true;
	Double_t annRich = cand->fRichAnn;
	Double_t annTrd = cand->fTrdAnn;
	Bool_t tofEl = IsTofElectron(gTrack, momentum, cand);
	//Bool_t momCut = (fCuts.fMomentumCut > 0.)?(momentum < fCuts.fMomentumCut):true;

	if (richEl && trdEl && tofEl)
	{
	     cand->fIsElectron = true;
	} else
	{
	     cand->fIsElectron = false;
	}

} // IsElectron



Bool_t CbmAnaJpsiTask::IsRichElectron(CbmRichRing* ring,
		Double_t momentum,
		CbmAnaJpsiCandidate* cand)
{
	if (fCuts.fUseRichAnn == false)
	{
		Bool_t axisA= ring->GetAaxis();
		Bool_t axisB= ring->GetBaxis();
		Double_t dist= ring->GetDistance();
		if ( fabs(axisA-fCuts.fMeanA) < fCuts.fRmsCoeff*fCuts.fRmsA &&fabs(axisB-fCuts.fMeanB) < fCuts.fRmsCoeff*fCuts.fRmsB && dist < fCuts.fDistCut){
		            return true;
		        } else {
		            return false;
		        }
	 } else
	 {
	    Double_t ann = fElIdAnn->DoSelect(ring, momentum);
	    cand->fRichAnn = ann;
	    if (ann > fCuts.fRichAnnCut) return true;
	    else  return false;
	 }
}//IsRichElectron



Bool_t CbmAnaJpsiTask::IsTrdElectron(
      CbmTrdTrack* trdTrack,
	  CbmAnaJpsiCandidate* cand)
{
    Double_t ann = trdTrack->GetPidANN();
    cand->fTrdAnn = ann;
    if (ann > fCuts.fTrdAnnCut) return true;
    else return false;

}


Bool_t CbmAnaJpsiTask::IsTofElectron(
      CbmGlobalTrack* gTrack,
      Double_t momentum,
	  CbmAnaJpsiCandidate* cand)
{
	Double_t trackLength = gTrack->GetLength() / 100. ;

	//calculate Time Of Flight of TOF Hit
	Int_t tofIndex = gTrack->GetTofHitIndex();
	CbmTofHit* tofHit = (CbmTofHit*) fTofHits->At(tofIndex);
	if (tofHit == NULL)
	{
		cand->fMass2 = 100.;
		return false;
	}

	Double_t time= 0.2998 * tofHit->GetTime(); // time in ns -> transfrom to ct in m

    // Calculate mass squared
	Double_t mass2 = TMath::Power(momentum,2.)* (TMath::Power(time/ trackLength, 2) - 1);
	cand->fMass2 = mass2;

	if (momentum >= 1.)
	{
		if (mass2 < (0.029*momentum-0.0409))	//(0.013*momentum - 0.003))
			{
		       return true;
		    }
	} else
		{
			if (mass2 < 0.02)	//0.01)
			{
		      return true; //fTofM2
		    }
		}
	return false;
}


Bool_t CbmAnaJpsiTask::IsMismatch(
		CbmAnaJpsiCandidate* cand)
{
	if (cand->fStsMcTrackId == cand->fRichMcTrackId && cand->fStsMcTrackId == cand->fTrdMcTrackId &&
	       cand->fStsMcTrackId == cand->fTofMcTrackId && cand->fStsMcTrackId !=-1) return false;
	return true;
}

void CbmAnaJpsiTask::RichPmtXY() {
	Int_t nofRichHits = fRichHits->GetEntriesFast();
	for (Int_t i = 0; i < nofRichHits; i++) {
		// get the RichHit from array
		CbmRichHit* richHit = (CbmRichHit*) fRichHits->At(i);
		if (NULL == richHit) continue;
		Int_t PointInd = richHit->GetRefId();
		if (PointInd < 0) continue;

		// get the McRichPoint of the RichHit
		CbmRichPoint* richMcPoint = (CbmRichPoint*) fRichPoints->At(PointInd);
		if (NULL == richMcPoint) continue;

		// get the RICH photon MC Track
		Int_t photonMcTrackId = richMcPoint->GetTrackID();
		if (photonMcTrackId == -1) continue;
		CbmMCTrack* photonMcTrack = (CbmMCTrack*) fMcTracks->At(photonMcTrackId);
		if (NULL == photonMcTrack) continue;

		// get photon mother MC Track (electron, pion etc.)
		Int_t photonMotherId = photonMcTrack->GetMotherId();
		if (photonMotherId == -1) continue;
		CbmMCTrack* photonMotherMcTrack = (CbmMCTrack*) fMcTracks->At(photonMotherId);
		if (NULL == photonMotherMcTrack) continue;

		Bool_t isMcSignalElectron = CbmAnaJpsiUtils::IsMcSignalElectron(photonMotherMcTrack);
		Bool_t isMcGammaElectron = CbmAnaJpsiUtils::IsMcGammaElectron(photonMotherMcTrack, fMcTracks);
		Bool_t isMcPi0Electron = CbmAnaJpsiUtils::IsMcPi0Electron(photonMotherMcTrack, fMcTracks);
		if (isMcSignalElectron) {
			fHM->H2("fh_rich_pmt_xy_" + CbmAnaJpsiHist::fSourceTypes[kJpsiSignal])->Fill( richHit->GetX(), richHit->GetY());
		} else {
			fHM->H2("fh_rich_pmt_xy_" + CbmAnaJpsiHist::fSourceTypes[kJpsiBg])->Fill( richHit->GetX(), richHit->GetY());
		}
		if (isMcGammaElectron) fHM->H2("fh_rich_pmt_xy_" + CbmAnaJpsiHist::fSourceTypes[kJpsiGamma])->Fill(richHit->GetX(), richHit->GetY());
		if (isMcPi0Electron) fHM->H2("fh_rich_pmt_xy_"+ CbmAnaJpsiHist::fSourceTypes[kJpsiPi0])->Fill(richHit->GetX(), richHit->GetY());
	}
}

void CbmAnaJpsiTask::CopyCandidatesToOutputArray()
{
	for (Int_t i = 0; i < fCandidates.size(); i++) {
		new ((*fJpsiCandidates)[i]) CbmAnaJpsiCandidate(fCandidates[i]);
	}
}


void CbmAnaJpsiTask::Finish()
{
	fHM->WriteToFile();
}


ClassImp(CbmAnaJpsiTask)

