/**
 * \file CbmAnaConversionPhotons2.cxx
 *
 * Similar studies as in CbmAnaConversionPhotons.cxx, but with different properties
 * here mainly influence of different cuts on results
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2015
 **/

#include "CbmAnaConversionPhotons2.h"

#include "CbmMCTrack.h"
#include "FairRootManager.h"
#include "CbmRichPoint.h"
#include "CbmGlobalTrack.h"
#include "CbmStsTrack.h"
#include "CbmRichRing.h"
#include "CbmTrackMatchNew.h"
#include "CbmDrawHist.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"
//#include "CbmStsKFTrackFitter.h"
#include "../../littrack/cbm/elid/CbmLitGlobalElectronId.h"

#include <algorithm>
#include <map>

#include "TCanvas.h"


#include "CbmAnaConversionCutSettings.h"
#include "CbmAnaConversionKinematicParams.h"


using namespace std;



CbmAnaConversionPhotons2::CbmAnaConversionPhotons2()
  : fRichPoints(NULL),
    fRichRings(NULL),
    fRichRingMatches(NULL),
    fMcTracks(NULL),
    fStsTracks(NULL),
    fStsTrackMatches(NULL),
    fGlobalTracks(NULL),
    fPrimVertex(NULL),
    fKFVertex(),
    fHistoList_photons(),
    fRecoTracklist_gtIndex(),
	fRecoTracklist_mcIndex(),
	fRecoTracklist_momentum(),
	fRecoTracklist_chi(),
	fRecoTracklist_mctrack(),
    fRecoTracklist_withRICH_gtIndex(),
	fRecoTracklist_withRICH_mcIndex(),
	fRecoTracklist_withRICH_momentum(),
	fRecoTracklist_withRICH_chi(),
	fRecoTracklist_withRICH_mctrack(),
	fh2Electrons_angle_all(NULL),
	fh2Electrons_angle_combBack(NULL),
	fh2Electrons_angle_allSameG(NULL),
	fh2Electrons_angle_all_cuts(NULL),
	fh2Electrons_angle_combBack_cuts(NULL),
	fh2Electrons_angle_allSameG_cuts(NULL),
	fh2Electrons_angle_CUTcomparison(NULL),
	fh2Electrons_angle_CUTcomparison_chi(NULL),
	fh2Electrons_angle_CUTcomparison_withRICH(NULL),
	fh2Electrons_angle_CUTcomparison_withRICH_chi(NULL),
	fh2Electrons_invmass_all(NULL),
	fh2Electrons_invmass_combBack(NULL),
	fh2Electrons_invmass_allSameG(NULL),
	fh2Electrons_invmass_all_cut(NULL),
	fh2Electrons_invmass_combBack_cut(NULL),
	fh2Electrons_invmass_allSameG_cut(NULL),
	fh2Electrons_angleVSpt_all(NULL),
	fh2Electrons_angleVSpt_combBack(NULL),
	fh2Electrons_angleVSpt_allSameG(NULL),
	fh2Electrons_invmassVSpt_all(NULL),
	fh2Electrons_invmassVSpt_combBack(NULL),
	fh2Electrons_invmassVSpt_allSameG(NULL),
    timer(),
    fTime(0.)
{
}

CbmAnaConversionPhotons2::~CbmAnaConversionPhotons2()
{
}


void CbmAnaConversionPhotons2::Init()
{
	FairRootManager* ioman = FairRootManager::Instance();
	if (NULL == ioman) { Fatal("CbmAnaConversion::Init","RootManager not instantised!"); }

	fRichPoints = (TClonesArray*) ioman->GetObject("RichPoint");
	if ( NULL == fRichPoints) { Fatal("CbmAnaConversion::Init","No RichPoint array!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( NULL == fMcTracks) { Fatal("CbmAnaConversion::Init","No MCTrack array!"); }

	fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	if ( NULL == fStsTracks) { Fatal("CbmAnaConversion::Init","No StsTrack array!"); }

	fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
	if (NULL == fStsTrackMatches) { Fatal("CbmAnaConversion::Init","No StsTrackMatch array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (NULL == fGlobalTracks) { Fatal("CbmAnaConversion::Init","No GlobalTrack array!"); }

	fPrimVertex = (CbmVertex*) ioman->GetObject("PrimaryVertex");
	if (NULL == fPrimVertex) { Fatal("CbmAnaConversion::Init","No PrimaryVertex array!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if (NULL == fRichRings) { Fatal("CbmAnaConversion::Init","No RichRing array!"); }

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if (NULL == fRichRingMatches) { Fatal("CbmAnaConversion::Init","No RichRingMatch array!"); }


	InitHistos();
}


void CbmAnaConversionPhotons2::InitHistos()
{
	fHistoList_photons.clear();



	// opening angles for all photon-energies (RECO)
	fh2Electrons_angle_all		= new TH1D("fh2Electrons_angle_all", "fh2Electrons_angle_all; opening angle [deg]; #", 2001, -0.05, 200.05);
	fh2Electrons_angle_combBack	= new TH1D("fh2Electrons_angle_combBack", "fh2Electrons_angle_combBack; opening angle [deg]; #", 2001, -0.05, 200.05);
	fh2Electrons_angle_allSameG	= new TH1D("fh2Electrons_angle_allSameG", "fh2Electrons_angle_allSameG; opening angle [deg]; #", 1001, -0.05, 100.05);
	fHistoList_photons.push_back(fh2Electrons_angle_all);
	fHistoList_photons.push_back(fh2Electrons_angle_combBack);
	fHistoList_photons.push_back(fh2Electrons_angle_allSameG);
	
	
	// histogram for comparison of different opening angle cuts and their influence on signal and background amounts
	fh2Electrons_angle_CUTcomparison	= new TH1I("fh2Electrons_angle_CUTcomparison", "fh2Electrons_angle_CUTcomparison; ; #", 12, 0., 12.);
	fHistoList_photons.push_back(fh2Electrons_angle_CUTcomparison);
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(1, "true, no cut");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(2, "false, no cut");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(3, "true, cut1");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(4, "false, cut1");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(5, "true, cut2");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(6, "false, cut2");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(7, "true, cut3");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(8, "false, cut3");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(9, "true, cut4");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(10, "false, cut4");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(11, "true, cut5");
	fh2Electrons_angle_CUTcomparison->GetXaxis()->SetBinLabel(12, "false, cut5");


	fh2Electrons_angle_CUTcomparison_chi	= new TH1I("fh2Electrons_angle_CUTcomparison_chi", "fh2Electrons_angle_CUTcomparison_chi; ; #", 8, 0., 8.);
	fHistoList_photons.push_back(fh2Electrons_angle_CUTcomparison_chi);
	fh2Electrons_angle_CUTcomparison_chi->GetXaxis()->SetBinLabel(1, "true, no cut");
	fh2Electrons_angle_CUTcomparison_chi->GetXaxis()->SetBinLabel(2, "false, no cut");
	fh2Electrons_angle_CUTcomparison_chi->GetXaxis()->SetBinLabel(3, "true, cut1");
	fh2Electrons_angle_CUTcomparison_chi->GetXaxis()->SetBinLabel(4, "false, cut1");
	fh2Electrons_angle_CUTcomparison_chi->GetXaxis()->SetBinLabel(5, "true, cut2");
	fh2Electrons_angle_CUTcomparison_chi->GetXaxis()->SetBinLabel(6, "false, cut2");
	fh2Electrons_angle_CUTcomparison_chi->GetXaxis()->SetBinLabel(7, "true, cut2");
	fh2Electrons_angle_CUTcomparison_chi->GetXaxis()->SetBinLabel(8, "false, cut2");


	fh2Electrons_angle_CUTcomparison_withRICH	= new TH1I("fh2Electrons_angle_CUTcomparison_withRICH", "fh2Electrons_angle_CUTcomparison_withRICH; ; #", 12, 0., 12.);
	fHistoList_photons.push_back(fh2Electrons_angle_CUTcomparison_withRICH);
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(1, "true, no cut");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(2, "false, no cut");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(3, "true, cut1");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(4, "false, cut1");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(5, "true, cut2");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(6, "false, cut2");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(7, "true, cut3");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(8, "false, cut3");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(9, "true, cut4");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(10, "false, cut4");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(11, "true, cut5");
	fh2Electrons_angle_CUTcomparison_withRICH->GetXaxis()->SetBinLabel(12, "false, cut5");


	fh2Electrons_angle_CUTcomparison_withRICH_chi	= new TH1I("fh2Electrons_angle_CUTcomparison_withRICH_chi", "fh2Electrons_angle_CUTcomparison_withRICH_chi; ; #", 8, 0., 8.);
	fHistoList_photons.push_back(fh2Electrons_angle_CUTcomparison_withRICH_chi);
	fh2Electrons_angle_CUTcomparison_withRICH_chi->GetXaxis()->SetBinLabel(1, "true, no cut");
	fh2Electrons_angle_CUTcomparison_withRICH_chi->GetXaxis()->SetBinLabel(2, "false, no cut");
	fh2Electrons_angle_CUTcomparison_withRICH_chi->GetXaxis()->SetBinLabel(3, "true, cut1");
	fh2Electrons_angle_CUTcomparison_withRICH_chi->GetXaxis()->SetBinLabel(4, "false, cut1");
	fh2Electrons_angle_CUTcomparison_withRICH_chi->GetXaxis()->SetBinLabel(5, "true, cut2");
	fh2Electrons_angle_CUTcomparison_withRICH_chi->GetXaxis()->SetBinLabel(6, "false, cut2");
	fh2Electrons_angle_CUTcomparison_withRICH_chi->GetXaxis()->SetBinLabel(7, "true, cut2");
	fh2Electrons_angle_CUTcomparison_withRICH_chi->GetXaxis()->SetBinLabel(8, "false, cut2");

	
	
		// opening angles for all photon-energies (RECO) with application of opening angle cuts
	fh2Electrons_angle_all_cuts		= new TH1D("fh2Electrons_angle_all_cuts", "fh2Electrons_angle_all_cuts; opening angle [deg]; #", 101, -0.05, 10.05);
	fh2Electrons_angle_combBack_cuts	= new TH1D("fh2Electrons_angle_combBack_cuts", "fh2Electrons_angle_combBack_cuts; opening angle [deg]; #", 101, -0.05, 10.05);
	fh2Electrons_angle_allSameG_cuts	= new TH1D("fh2Electrons_angle_allSameG_cuts", "fh2Electrons_angle_allSameG_cuts; opening angle [deg]; #", 101, -0.05, 10.05);
	fHistoList_photons.push_back(fh2Electrons_angle_all_cuts);
	fHistoList_photons.push_back(fh2Electrons_angle_combBack_cuts);
	fHistoList_photons.push_back(fh2Electrons_angle_allSameG_cuts);
	
	
	fh2Electrons_invmass_all		= new TH1D("fh2Electrons_invmass_all", "fh2Electrons_invmass_all; invmass [GeV/c^2]; #", 5000, 0., 5.);
	fh2Electrons_invmass_combBack	= new TH1D("fh2Electrons_invmass_combBack", "fh2Electrons_invmass_combBack; invmass [GeV/c^2]; #", 5000, 0., 5.);
	fh2Electrons_invmass_allSameG	= new TH1D("fh2Electrons_invmass_allSameG", "fh2Electrons_invmass_allSameG; invmass [GeV/c^2]; #", 5000, 0., 5);
	fHistoList_photons.push_back(fh2Electrons_invmass_all);
	fHistoList_photons.push_back(fh2Electrons_invmass_combBack);
	fHistoList_photons.push_back(fh2Electrons_invmass_allSameG);
	
	fh2Electrons_invmass_all_cut		= new TH1D("fh2Electrons_invmass_all_cut", "fh2Electrons_invmass_all_cut; invmass [GeV/c^2]; #", 5000, 0., 5.);
	fh2Electrons_invmass_combBack_cut	= new TH1D("fh2Electrons_invmass_combBack_cut", "fh2Electrons_invmass_combBack_cut; invmass [GeV/c^2]; #", 5000, 0., 5.);
	fh2Electrons_invmass_allSameG_cut	= new TH1D("fh2Electrons_invmass_allSameG_cut", "fh2Electrons_invmass_allSameG_cut; invmass [GeV/c^2]; #", 5000, 0., 5);
	fHistoList_photons.push_back(fh2Electrons_invmass_all_cut);
	fHistoList_photons.push_back(fh2Electrons_invmass_combBack_cut);
	fHistoList_photons.push_back(fh2Electrons_invmass_allSameG_cut);

	

	// opening angle vs pt (reco)
	fh2Electrons_angleVSpt_all		= new TH2D("fh2Electrons_angleVSpt_all", "fh2Electrons_angleVSpt_all;pt [GeV/c];opening angle [deg]", 500, 0., 5., 400, 0., 20.);
	fh2Electrons_angleVSpt_combBack	= new TH2D("fh2Electrons_angleVSpt_combBack", "fh2Electrons_angleVSpt_combBack;pt [GeV/c];opening angle [deg]", 500, 0., 5., 400, 0., 20.);
	fh2Electrons_angleVSpt_allSameG	= new TH2D("fh2Electrons_angleVSpt_allSameG", "fh2Electrons_angleVSpt_allSameG;pt [GeV/c];opening angle [deg]", 500, 0., 5., 400, 0., 20.);
	fHistoList_photons.push_back(fh2Electrons_angleVSpt_all);
	fHistoList_photons.push_back(fh2Electrons_angleVSpt_combBack);
	fHistoList_photons.push_back(fh2Electrons_angleVSpt_allSameG);



	// invariant mass vs pt (reco)
	fh2Electrons_invmassVSpt_all		= new TH2D("fh2Electrons_invmassVSpt_all", "fh2Electrons_invmassVSpt_all;pt [GeV/c];invmass [GeV]", 500, 0., 5., 5000, 0., 5.);
	fh2Electrons_invmassVSpt_combBack	= new TH2D("fh2Electrons_invmassVSpt_combBack", "fh2Electrons_invmassVSpt_combBack;pt [GeV/c];invmass [GeV]", 500, 0., 5., 5000, 0., 5.);
	fh2Electrons_invmassVSpt_allSameG	= new TH2D("fh2Electrons_invmassVSpt_allSameG", "fh2Electrons_invmassVSpt_allSameG;pt [GeV/c];invmass [GeV]", 500, 0., 5., 5000, 0., 5.);
	fHistoList_photons.push_back(fh2Electrons_invmassVSpt_all);
	fHistoList_photons.push_back(fh2Electrons_invmassVSpt_combBack);
	fHistoList_photons.push_back(fh2Electrons_invmassVSpt_allSameG);

}







void CbmAnaConversionPhotons2::Finish()
{
	//gDirectory->cd("analysis-conversion");
	gDirectory->mkdir("Photons2");
	gDirectory->cd("Photons2");
	
	for (UInt_t i = 0; i < fHistoList_photons.size(); i++){
		fHistoList_photons[i]->Write();
	}
	gDirectory->cd("..");

}



void CbmAnaConversionPhotons2::Exec()
{
	if (fPrimVertex != NULL){
		fKFVertex = CbmKFVertex(*fPrimVertex);
	} else {
		Fatal("CbmAnaConversionPhotons2::Exec","No PrimaryVertex array!");
	}


	fRecoTracklist_gtIndex.clear();
	fRecoTracklist_mcIndex.clear();
	fRecoTracklist_momentum.clear();
	fRecoTracklist_chi.clear();
	fRecoTracklist_mctrack.clear();

	fRecoTracklist_withRICH_gtIndex.clear();
	fRecoTracklist_withRICH_mcIndex.clear();
	fRecoTracklist_withRICH_momentum.clear();
	fRecoTracklist_withRICH_chi.clear();
	fRecoTracklist_withRICH_mctrack.clear();



	// everything related to reconstructed data
	Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
	for (int iG = 0; iG < nofGlobalTracks; iG++){
		CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(iG);
		if(NULL == gTrack) continue;
		int stsInd = gTrack->GetStsTrackIndex();
		int richInd = gTrack->GetRichRingIndex();
		if (stsInd < 0) continue;
		
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
		if (stsTrack == NULL) continue;

		CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*)fStsTrackMatches->At(stsInd);
		if (stsMatch == NULL) continue;
		if(stsMatch->GetNofLinks() <= 0) continue;
		int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
		if (stsMcTrackId < 0) continue;
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
		if (mcTrack1 == NULL) continue;


		int pdg = TMath::Abs(mcTrack1->GetPdgCode());


		// calculate refitted momenta at primary vertex
		TVector3 refittedMomentum;
		CbmL1PFFitter fPFFitter;
		vector<CbmStsTrack> stsTracks;
		stsTracks.resize(1);
		stsTracks[0] = *stsTrack;
		vector<L1FieldRegion> vField;
		vector<float> chiPrim;
		fPFFitter.GetChiToVertex(stsTracks, vField, chiPrim, fKFVertex, 3e6);
		//cand.chi2sts = stsTracks[0].GetChiSq() / stsTracks[0].GetNDF();
		//cand.chi2Prim = chiPrim[0];
		const FairTrackParam* vtxTrack = stsTracks[0].GetParamFirst();
		vtxTrack->Momentum(refittedMomentum);
		float result_chi = chiPrim[0];



		// Doing refit of momenta with electron assumption
		CbmL1PFFitter fPFFitter_electron;
		vector<CbmStsTrack> stsTracks_electron;
		stsTracks_electron.resize(1);
		stsTracks_electron[0] = *stsTrack;
		vector<L1FieldRegion> vField_electron;
		vector<float> chiPrim_electron;
		vector<int> pidHypo_electron;
		pidHypo_electron.push_back(11);
		fPFFitter_electron.Fit(stsTracks_electron, pidHypo_electron); 
		fPFFitter_electron.GetChiToVertex(stsTracks_electron, vField_electron, chiPrim_electron, fKFVertex, 3e6);

		TVector3 refittedMomentum_electron;
		const FairTrackParam* vtxTrack_electron = stsTracks_electron[0].GetParamFirst();
		vtxTrack_electron->Momentum(refittedMomentum_electron);
		float result_chi_electron = chiPrim_electron[0];
		
		refittedMomentum = refittedMomentum_electron;
		result_chi = result_chi_electron;



		if(pdg == 11) {
			fRecoTracklist_gtIndex.push_back(iG);
			fRecoTracklist_mcIndex.push_back(stsMcTrackId);
			fRecoTracklist_momentum.push_back(refittedMomentum);
			fRecoTracklist_chi.push_back(result_chi);
			fRecoTracklist_mctrack.push_back(mcTrack1);
		}



		if (richInd < 0) continue;
		CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(richInd);
		if (richMatch == NULL) continue;
		int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
		if (richMcTrackId < 0) continue;
		CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
		if (mcTrack2 == NULL) continue;


		if(pdg == 11) {
			fRecoTracklist_withRICH_gtIndex.push_back(iG);
			fRecoTracklist_withRICH_mcIndex.push_back(stsMcTrackId);
			fRecoTracklist_withRICH_momentum.push_back(refittedMomentum);
			fRecoTracklist_withRICH_chi.push_back(result_chi);
			fRecoTracklist_withRICH_mctrack.push_back(mcTrack1);
		}

	}

	AnalyseElectronsReco();
	AnalyseElectronsRecoWithRICH();
}






void CbmAnaConversionPhotons2::AnalyseElectronsReco()
{
	int electronnumber = fRecoTracklist_gtIndex.size();

	cout << "CbmAnaConversionPhotons: array size in AnalyseElectronsReco(): " << electronnumber << endl;
	for(int i=0; i<electronnumber-1; i++) {
		for(int j=i+1; j<electronnumber; j++) {
			if(fRecoTracklist_mctrack[i]->GetPdgCode() + fRecoTracklist_mctrack[j]->GetPdgCode() != 0) continue;	// only 1 electron and 1 positron allowed
			
			
			CbmAnaConversionKinematicParams paramSet = CbmAnaConversionKinematicParams::KinematicParams_2particles_Reco( fRecoTracklist_momentum[i], fRecoTracklist_momentum[j] );
			Double_t OpeningAngleCut = CbmAnaConversionCutSettings::CalcOpeningAngleCut(paramSet.fPt);
			
			
			int motherID_i = fRecoTracklist_mctrack[i]->GetMotherId();
			int motherID_j = fRecoTracklist_mctrack[j]->GetMotherId();
			
			
			
			// first all possible combinations are considered
			
			fh2Electrons_angle_all->Fill(paramSet.fAngle);
			fh2Electrons_angleVSpt_all->Fill(paramSet.fPt, paramSet.fAngle);
			fh2Electrons_invmass_all->Fill(paramSet.fMinv);
			fh2Electrons_invmassVSpt_all->Fill(paramSet.fPt, paramSet.fMinv);
			
			if(paramSet.fAngle < OpeningAngleCut) {
				fh2Electrons_invmass_all_cut->Fill(paramSet.fMinv);
				fh2Electrons_angle_all_cuts->Fill(paramSet.fAngle);
			}
			
			
			
			// then all false combinations are analysed, after that "continue" of loop, i.e. break
			if(motherID_i != motherID_j) {
				fh2Electrons_angle_combBack->Fill(paramSet.fAngle);
				fh2Electrons_angleVSpt_combBack->Fill(paramSet.fPt, paramSet.fAngle);
				fh2Electrons_invmass_combBack->Fill(paramSet.fMinv);
				fh2Electrons_invmassVSpt_combBack->Fill(paramSet.fPt, paramSet.fMinv);
				
				if(paramSet.fAngle < OpeningAngleCut) {
					fh2Electrons_invmass_combBack_cut->Fill(paramSet.fMinv);
					fh2Electrons_angle_combBack_cuts->Fill(paramSet.fAngle);
				}
				
				// study of different opening angle cuts
				fh2Electrons_angle_CUTcomparison->Fill(1);	// no cuts applied
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCut(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(3);
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt1(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(5);
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt2(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(7);
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt3(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(9);
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt4(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(11);
				
				
				fh2Electrons_angle_CUTcomparison_chi->Fill(1);	// no cuts applied
				Double_t pt_e1 = fRecoTracklist_momentum[i].Perp();
				Double_t pt_e2 = fRecoTracklist_momentum[j].Perp();
				if(fRecoTracklist_chi[i] < CbmAnaConversionCutSettings::CalcChiCut(pt_e1) && fRecoTracklist_chi[j] < CbmAnaConversionCutSettings::CalcChiCut(pt_e2) ) {
					fh2Electrons_angle_CUTcomparison_chi->Fill(3);
				}
				if(fRecoTracklist_chi[i] < CbmAnaConversionCutSettings::CalcChiCutAlt1(pt_e1) && fRecoTracklist_chi[j] < CbmAnaConversionCutSettings::CalcChiCutAlt1(pt_e2) ) {
					fh2Electrons_angle_CUTcomparison_chi->Fill(5);
				}
				if(fRecoTracklist_chi[i] < CbmAnaConversionCutSettings::CalcChiCutAlt2(pt_e1) && fRecoTracklist_chi[j] < CbmAnaConversionCutSettings::CalcChiCutAlt2(pt_e2) ) {
					fh2Electrons_angle_CUTcomparison_chi->Fill(7);
				}
				
				continue;
			}
			
			
			// everything, that is not a false combination, will be further analysed
			
			fh2Electrons_angle_allSameG->Fill(paramSet.fAngle);
			fh2Electrons_angleVSpt_allSameG->Fill(paramSet.fPt, paramSet.fAngle);
			fh2Electrons_invmass_allSameG->Fill(paramSet.fMinv);
			fh2Electrons_invmassVSpt_allSameG->Fill(paramSet.fPt, paramSet.fMinv);
			
			if(paramSet.fAngle < OpeningAngleCut) {
				fh2Electrons_invmass_allSameG_cut->Fill(paramSet.fMinv);
				fh2Electrons_angle_allSameG_cuts->Fill(paramSet.fAngle);
			}
			
			
			// study of different opening angle cuts
			fh2Electrons_angle_CUTcomparison->Fill(0);	// no cuts applied
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCut(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(2);
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt1(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(4);
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt2(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(6);
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt3(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(8);
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt4(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison->Fill(10);
			
			
			fh2Electrons_angle_CUTcomparison_chi->Fill(0);	// no cuts applied
			Double_t pt_e1 = fRecoTracklist_momentum[i].Perp();
			Double_t pt_e2 = fRecoTracklist_momentum[j].Perp();
			if(fRecoTracklist_chi[i] < CbmAnaConversionCutSettings::CalcChiCut(pt_e1) && fRecoTracklist_chi[j] < CbmAnaConversionCutSettings::CalcChiCut(pt_e2) ) {
				fh2Electrons_angle_CUTcomparison_chi->Fill(2);
			}
			if(fRecoTracklist_chi[i] < CbmAnaConversionCutSettings::CalcChiCutAlt1(pt_e1) && fRecoTracklist_chi[j] < CbmAnaConversionCutSettings::CalcChiCutAlt1(pt_e2) ) {
				fh2Electrons_angle_CUTcomparison_chi->Fill(4);
			}
			if(fRecoTracklist_chi[i] < CbmAnaConversionCutSettings::CalcChiCutAlt2(pt_e1) && fRecoTracklist_chi[j] < CbmAnaConversionCutSettings::CalcChiCutAlt2(pt_e2) ) {
				fh2Electrons_angle_CUTcomparison_chi->Fill(6);
			}
		}
	}
}





void CbmAnaConversionPhotons2::AnalyseElectronsRecoWithRICH()
{
	int electronnumber = fRecoTracklist_withRICH_gtIndex.size();

	cout << "CbmAnaConversionPhotons: array size in AnalyseElectronsReco(): " << electronnumber << endl;
	for(int i=0; i<electronnumber-1; i++) {
		for(int j=i+1; j<electronnumber; j++) {
			if(fRecoTracklist_withRICH_mctrack[i]->GetPdgCode() + fRecoTracklist_withRICH_mctrack[j]->GetPdgCode() != 0) continue;	// only 1 electron and 1 positron allowed
			
			
			CbmAnaConversionKinematicParams paramSet = CbmAnaConversionKinematicParams::KinematicParams_2particles_Reco( fRecoTracklist_withRICH_momentum[i], fRecoTracklist_withRICH_momentum[j] );
			
			
			
			int motherID_i = fRecoTracklist_withRICH_mctrack[i]->GetMotherId();
			int motherID_j = fRecoTracklist_withRICH_mctrack[j]->GetMotherId();
			
			
			
			// then all false combinations are analysed, after that "continue" of loop, i.e. break
			if(motherID_i != motherID_j) {
				// study of different opening angle cuts
				fh2Electrons_angle_CUTcomparison_withRICH->Fill(1);	// no cuts applied
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCut(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(3);
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt1(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(5);
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt2(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(7);
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt3(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(9);
				if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt4(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(11);
				
				
				fh2Electrons_angle_CUTcomparison_withRICH_chi->Fill(1);	// no cuts applied
				Double_t pt_e1 = fRecoTracklist_withRICH_momentum[i].Perp();
				Double_t pt_e2 = fRecoTracklist_withRICH_momentum[j].Perp();
				if(fRecoTracklist_withRICH_chi[i] < CbmAnaConversionCutSettings::CalcChiCut(pt_e1) && fRecoTracklist_withRICH_chi[j] < CbmAnaConversionCutSettings::CalcChiCut(pt_e2) ) {
					fh2Electrons_angle_CUTcomparison_withRICH_chi->Fill(3);
				}
				if(fRecoTracklist_withRICH_chi[i] < CbmAnaConversionCutSettings::CalcChiCutAlt1(pt_e1) && fRecoTracklist_withRICH_chi[j] < CbmAnaConversionCutSettings::CalcChiCutAlt1(pt_e2) ) {
					fh2Electrons_angle_CUTcomparison_withRICH_chi->Fill(5);
				}
				if(fRecoTracklist_withRICH_chi[i] < CbmAnaConversionCutSettings::CalcChiCutAlt2(pt_e1) && fRecoTracklist_withRICH_chi[j] < CbmAnaConversionCutSettings::CalcChiCutAlt2(pt_e2) ) {
					fh2Electrons_angle_CUTcomparison_withRICH_chi->Fill(7);
				}
				
				continue;
			}
			
			
			
			// study of different opening angle cuts
			fh2Electrons_angle_CUTcomparison_withRICH->Fill(0);	// no cuts applied
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCut(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(2);
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt1(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(4);
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt2(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(6);
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt3(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(8);
			if(paramSet.fAngle < CbmAnaConversionCutSettings::CalcOpeningAngleCutAlt4(paramSet.fPt) ) fh2Electrons_angle_CUTcomparison_withRICH->Fill(10);
			
			
			fh2Electrons_angle_CUTcomparison_withRICH_chi->Fill(0);	// no cuts applied
			Double_t pt_e1 = fRecoTracklist_withRICH_momentum[i].Perp();
			Double_t pt_e2 = fRecoTracklist_withRICH_momentum[j].Perp();
			if(fRecoTracklist_withRICH_chi[i] < CbmAnaConversionCutSettings::CalcChiCut(pt_e1) && fRecoTracklist_withRICH_chi[j] < CbmAnaConversionCutSettings::CalcChiCut(pt_e2) ) {
				fh2Electrons_angle_CUTcomparison_withRICH_chi->Fill(2);
			}
			if(fRecoTracklist_withRICH_chi[i] < CbmAnaConversionCutSettings::CalcChiCutAlt1(pt_e1) && fRecoTracklist_withRICH_chi[j] < CbmAnaConversionCutSettings::CalcChiCutAlt1(pt_e2) ) {
				fh2Electrons_angle_CUTcomparison_withRICH_chi->Fill(4);
			}
			if(fRecoTracklist_withRICH_chi[i] < CbmAnaConversionCutSettings::CalcChiCutAlt2(pt_e1) && fRecoTracklist_withRICH_chi[j] < CbmAnaConversionCutSettings::CalcChiCutAlt2(pt_e2) ) {
				fh2Electrons_angle_CUTcomparison_withRICH_chi->Fill(6);
			}
		}
	}
}


