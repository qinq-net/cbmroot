/**
 * \file CbmAnaConversionRecoFull.cxx
 *
 * 
 * 
 *
 * \author Sascha Reinecke <reinecke@uni-wuppertal.de>
 * \date 2014
 **/

#include "CbmAnaConversionRecoFull.h"

// standard includes
#include <iostream>

// includes from ROOT
#include "TRandom3.h"

// included from CbmRoot
#include "FairRootManager.h"
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
#include "../../littrack/cbm/elid/CbmLitGlobalElectronId.h"


#define M2E 2.6112004954086e-7
using namespace std;



CbmAnaConversionRecoFull::CbmAnaConversionRecoFull()
  : fRichPoints(NULL),
    fRichRings(NULL),
    fRichRingMatches(NULL),
    fMcTracks(NULL),
    fStsTracks(NULL),
    fStsTrackMatches(NULL),
    fGlobalTracks(NULL),
    fPrimVertex(NULL),
    fKFVertex(),
    fHistoList_recofull(),
    fHistoList_recofull_1(),
    fHistoList_recofull_2(),
    fHistoList_recofull_3(),
    fHistoList_recofull_4(),
    fHistoList_recofull_new(),
    fhElectrons(NULL),
    electronidentifier(NULL),
    fhMomentumFits(NULL),
	fhMomentumFits_electronRich(NULL),
	fhMomentumFits_pi0reco(NULL),
    fElectrons_track(),
    fElectrons_momenta(),
	fElectrons_momentaChi(),
	fElectrons_mctrackID(),
	fElectrons_track_1(),
	fElectrons_momenta_1(),
	fElectrons_momentaChi_1(),
	fElectrons_mctrackID_1(),
	fVector_photons_pairs_1(),
	fElectrons_track_2(),
	fElectrons_momenta_2(),
	fElectrons_momentaChi_2(),
	fElectrons_mctrackID_2(),
	fVector_photons_pairs_2(),
	fElectrons_track_3(),
	fElectrons_momenta_3(),
	fElectrons_momentaChi_3(),
	fElectrons_mctrackID_3(),
	fVector_photons_pairs_3(),
	fElectrons_track_4(),
	fElectrons_momenta_4(),
	fElectrons_momentaChi_4(),
	fElectrons_mctrackID_4(),
	fVector_photons_pairs_4(),
	fElectrons_track_new(),
	fElectrons_momenta_new(),
	fElectrons_momentaChi_new(),
	fElectrons_mctrackID_new(),
	fVector_photons_pairs_new(),
    fhElectrons_invmass(NULL),
    fhElectrons_invmass_cut(NULL),
    fhElectrons_nofPerEvent(NULL),
    fhPhotons_nofPerEvent(NULL),
    fVector_photons_pairs(),
	fVector_photons_momenta(),
    fhPhotons_invmass(NULL),
	fhPhotons_invmass_cut(NULL),
	fhPhotons_invmass_cut_chi1(NULL),
	fhPhotons_invmass_cut_chi3(NULL),
	fhPhotons_invmass_cut_chi5(NULL),
	fhPhotons_invmass_cut_chi10(NULL),
	fhPhotons_invmass_cut_chi25(NULL),
	fhPhotons_invmass_cut_chi40(NULL),
	fhPhotons_invmass_cut_chi65(NULL),
	fhPhotons_invmass_cut_chi80(NULL),
	fhPhotons_invmass_cut_ptBin1(NULL),
	fhPhotons_invmass_cut_ptBin2(NULL),
	fhPhotons_invmass_cut_ptBin3(NULL),
	fhPhotons_invmass_cut_ptBin4(NULL),
	fhPhotons_invmass_vs_chi(NULL),
	fhPhotons_startvertex_vs_chi(NULL),
	fhPhotons_angleBetween(NULL),
	fhPhotons_invmass_vs_pt(NULL),
	fhPhotons_rapidity_vs_pt(NULL),
	fhPhotons_invmass_vs_openingAngle(NULL),
	fhPhotons_openingAngle_vs_momentum(NULL),
	fhPhotons_MC_motherpdg(NULL),
	fhPhotons_MC_invmass1(NULL),
	fhPhotons_MC_invmass2(NULL),
	fhPhotons_MC_invmass3(NULL),
	fhPhotons_MC_invmass4(NULL),
	fhPhotons_MC_startvertexZ(NULL),
	fhPhotons_MC_motherIdCut(NULL),
	fhPhotons_Refit_chiDiff(NULL),
	fhPhotons_Refit_momentumDiff(NULL),
	fhPhotons_Refit_chiDistribution(NULL),
	fhPhotons_RefitPion_chiDistribution(NULL),
	fElectrons_track_refit(),
	fElectrons_momenta_refit(),
	fVector_photons_pairs_refit(),
	fhPhotons_invmass_refit(NULL),
	fhPhotons_invmass_refit_cut(NULL),
	fVector_photons_pairs_direction(),
	fhPhotons_invmass_direction(NULL),
	fhPhotons_invmass_direction_cut(NULL),
	fhPhotons_boostAngle(NULL),
	fhPhotons_boostAngleMC(NULL),
	fhPhotons_boostAngleTest(NULL),
	fhPhotons_tX(NULL),
	fhPhotons_tY(NULL),
	fhElectrons_nofPerEvent_1(NULL),
	fhPhotons_nofPerEvent_1(NULL),
	fhPhotons_invmass_1(NULL),
	fhPhotons_invmass_ptBin1_1(NULL),
	fhPhotons_invmass_ptBin2_1(NULL),
	fhPhotons_invmass_ptBin3_1(NULL),
	fhPhotons_invmass_ptBin4_1(NULL),
	fhElectrons_invmass_1(NULL),
	fhPhotons_invmass_vs_pt_1(NULL),
	fhElectrons_nofPerEvent_2(NULL),
	fhPhotons_nofPerEvent_2(NULL),
	fhPhotons_invmass_2(NULL),
	fhPhotons_invmass_ptBin1_2(NULL),
	fhPhotons_invmass_ptBin2_2(NULL),
	fhPhotons_invmass_ptBin3_2(NULL),
	fhPhotons_invmass_ptBin4_2(NULL),
	fhElectrons_invmass_2(NULL),
	fhPhotons_invmass_vs_pt_2(NULL),
	fhElectrons_nofPerEvent_3(NULL),
	fhPhotons_nofPerEvent_3(NULL),
	fhPhotons_invmass_3(NULL),
	fhPhotons_invmass_ptBin1_3(NULL),
	fhPhotons_invmass_ptBin2_3(NULL),
	fhPhotons_invmass_ptBin3_3(NULL),
	fhPhotons_invmass_ptBin4_3(NULL),
	fhElectrons_invmass_3(NULL),
	fhPhotons_invmass_vs_pt_3(NULL),
	fhElectrons_nofPerEvent_4(NULL),
	fhPhotons_nofPerEvent_4(NULL),
	fhPhotons_invmass_4(NULL),
	fhPhotons_invmass_ptBin1_4(NULL),
	fhPhotons_invmass_ptBin2_4(NULL),
	fhPhotons_invmass_ptBin3_4(NULL),
	fhPhotons_invmass_ptBin4_4(NULL),
	fhElectrons_invmass_4(NULL),
	fhPhotons_invmass_vs_pt_4(NULL),
	fhElectrons_nofPerEvent_new(),
	fhPhotons_nofPerEvent_new(),
	fhPhotons_invmass_new(),
	fhPhotons_invmass_ptBin1_new(),
	fhPhotons_invmass_ptBin2_new(),
	fhPhotons_invmass_ptBin3_new(),
	fhPhotons_invmass_ptBin4_new(),
	fhElectrons_invmass_new(),
	fhPhotons_invmass_vs_pt_new(),
	fhPhotons_invmass_MCcut1_new(),
	fhPhotons_invmass_MCcut2_new(),
	fhPhotons_invmass_MCcut3_new(),
	fhPhotons_invmass_MCcut4_new(),
	timer(),
	fTime(0.)
{
}

CbmAnaConversionRecoFull::~CbmAnaConversionRecoFull()
{
}


void CbmAnaConversionRecoFull::Init()
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
	electronidentifier = new CbmLitGlobalElectronId();
	electronidentifier->Init();

}



void CbmAnaConversionRecoFull::InitHistos()
{
	fHistoList_recofull.clear();
	fHistoList_recofull_1.clear();
	fHistoList_recofull_2.clear();
	fHistoList_recofull_3.clear();
	fHistoList_recofull_4.clear();

	for(int i=1; i<=4; i++) {
		fHistoList_recofull_new[i].clear();
	}

	fhElectrons = new TH1I("fhElectrons", "fhElectrons;; #", 8, 0., 8.);
	fHistoList_recofull.push_back(fhElectrons);
	fhElectrons->GetXaxis()->SetBinLabel(1, "electrons rich");
	fhElectrons->GetXaxis()->SetBinLabel(2, "electrons trd");
	fhElectrons->GetXaxis()->SetBinLabel(3, "electrons tof");
	fhElectrons->GetXaxis()->SetBinLabel(4, "electrons rich+trd");
	fhElectrons->GetXaxis()->SetBinLabel(5, "electrons rich+tof");
	fhElectrons->GetXaxis()->SetBinLabel(6, "electrons trd+tof");
	fhElectrons->GetXaxis()->SetBinLabel(7, "electrons rich+trd+tof");
	fhElectrons->GetXaxis()->SetBinLabel(8, "electrons 2 out of 3");


	fhMomentumFits = new TH1D("fhMomentumFits", "fhMomentumFits; chi; #", 10000, 0., 1000.);
	fHistoList_recofull.push_back(fhMomentumFits);
	fhMomentumFits_electronRich = new TH1D("fhMomentumFits_electronRich", "fhMomentumFits_electronRich; chi; #", 10000, 0., 1000.);
	fHistoList_recofull.push_back(fhMomentumFits_electronRich);
	fhMomentumFits_pi0reco = new TH1D("fhMomentumFits_pi0reco", "fhMomentumFits_pi0reco; chi; #", 10000, 0., 1000.);
	fHistoList_recofull.push_back(fhMomentumFits_pi0reco);


	fhElectrons_invmass = new TH1D("fhElectrons_invmass", "fhElectrons_invmass; invariant mass; #", 600, -0.0025, 2.9975);
	fhElectrons_invmass_cut = new TH1D("fhElectrons_invmass_cut", "fhElectrons_invmass_cut; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhElectrons_invmass);
	fHistoList_recofull.push_back(fhElectrons_invmass_cut);
	
	fhElectrons_nofPerEvent = new TH1D("fhElectrons_nofPerEvent", "fhElectrons_nofPerEvent; nof electrons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull.push_back(fhElectrons_nofPerEvent);
	fhPhotons_nofPerEvent = new TH1D("fhPhotons_nofPerEvent", "fhPhotons_nofPerEvent; nof photons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull.push_back(fhPhotons_nofPerEvent);
	
	fhPhotons_invmass = new TH1D("fhPhotons_invmass", "fhPhotons_invmass; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass);
	fhPhotons_invmass_cut = new TH1D("fhPhotons_invmass_cut", "fhPhotons_invmass_cut; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut);
	
	
	fhPhotons_invmass_cut_ptBin1 = new TH1D("fhPhotons_invmass_cut_ptBin1", "fhPhotons_invmass_cut_ptBin1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_ptBin1);
	fhPhotons_invmass_cut_ptBin2 = new TH1D("fhPhotons_invmass_cut_ptBin2", "fhPhotons_invmass_cut_ptBin2; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_ptBin2);
	fhPhotons_invmass_cut_ptBin3 = new TH1D("fhPhotons_invmass_cut_ptBin3", "fhPhotons_invmass_cut_ptBin3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_ptBin3);
	fhPhotons_invmass_cut_ptBin4 = new TH1D("fhPhotons_invmass_cut_ptBin4", "fhPhotons_invmass_cut_ptBin4; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_ptBin4);
	
	
	
	fhPhotons_angleBetween = new TH1D("fhPhotons_angleBetween", "fhPhotons_angleBetween; opening angle [deg]; #", 1000, 0, 100);
	fHistoList_recofull.push_back(fhPhotons_angleBetween);


	fhPhotons_invmass_cut_chi1 = new TH1D("fhPhotons_invmass_cut_chi1", "fhPhotons_invmass_cut_chi1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi1);
	fhPhotons_invmass_cut_chi3 = new TH1D("fhPhotons_invmass_cut_chi3", "fhPhotons_invmass_cut_chi3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi3);
	fhPhotons_invmass_cut_chi5 = new TH1D("fhPhotons_invmass_cut_chi5", "fhPhotons_invmass_cut_chi5; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi5);
	fhPhotons_invmass_cut_chi10 = new TH1D("fhPhotons_invmass_cut_chi10", "fhPhotons_invmass_cut_chi10; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi10);
	fhPhotons_invmass_cut_chi25 = new TH1D("fhPhotons_invmass_cut_chi25", "fhPhotons_invmass_cut_chi25; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi25);
	fhPhotons_invmass_cut_chi40 = new TH1D("fhPhotons_invmass_cut_chi40", "fhPhotons_invmass_cut_chi40; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi40);
	fhPhotons_invmass_cut_chi65 = new TH1D("fhPhotons_invmass_cut_chi65", "fhPhotons_invmass_cut_chi65; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi65);
	fhPhotons_invmass_cut_chi80 = new TH1D("fhPhotons_invmass_cut_chi80", "fhPhotons_invmass_cut_chi80; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_cut_chi80);
	
	fhPhotons_invmass_vs_chi = new TH2D("fhPhotons_invmass_vs_chi", "fhPhotons_invmass_vs_chi; invariant mass; chi", 600, -0.0025, 2.9975, 1000, 0., 100.);
	fHistoList_recofull.push_back(fhPhotons_invmass_vs_chi);
	fhPhotons_startvertex_vs_chi = new TH2D("fhPhotons_startvertex_vs_chi", "fhPhotons_startvertex_vs_chi; startvertex; chi", 400, -5., 95., 1000, 0., 200.);
	fHistoList_recofull.push_back(fhPhotons_startvertex_vs_chi);
	fhPhotons_invmass_vs_pt = new TH2D("fhPhotons_invmass_vs_pt", "fhPhotons_invmass_vs_pt; invmass; pt", 600, -0.0025, 2.9975, 100, 0., 10.);
	fHistoList_recofull.push_back(fhPhotons_invmass_vs_pt);
	fhPhotons_rapidity_vs_pt = new TH2D("fhPhotons_rapidity_vs_pt", "fhPhotons_rapidity_vs_pt; pt [GeV]; rap [GeV]", 240, -2., 10., 210, 0., 7.);
	fHistoList_recofull.push_back(fhPhotons_rapidity_vs_pt);
	
	fhPhotons_invmass_vs_openingAngle = new TH2D("fhPhotons_invmass_vs_openingAngle", "fhPhotons_invmass_vs_openingAngle; invmass; openingAngle", 600, -0.0025, 2.9975, 100, 0., 10.);
	fHistoList_recofull.push_back(fhPhotons_invmass_vs_openingAngle);
	fhPhotons_openingAngle_vs_momentum = new TH2D("fhPhotons_openingAngle_vs_momentum", "fhPhotons_openingAngle_vs_momentum; openingAngle; momentum", 100, 0., 10., 100, 0., 10.);
	fHistoList_recofull.push_back(fhPhotons_openingAngle_vs_momentum);
	
	

	fhPhotons_MC_motherpdg = new TH1D("fhPhotons_MC_motherpdg", "fhPhotons_MC_motherpdg; pdg; #", 1000, 0., 1000.);
	fHistoList_recofull.push_back(fhPhotons_MC_motherpdg);
	fhPhotons_MC_invmass1 = new TH1D("fhPhotons_MC_invmass1", "fhPhotons_MC_minvmass1 (MC-true cut: at least gamma correct reconstructed); invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_invmass1);
	fhPhotons_MC_invmass2 = new TH1D("fhPhotons_MC_invmass2", "fhPhotons_MC_minvmass2 (MC-true cut: gamma from same mother); invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_invmass2);
	fhPhotons_MC_invmass3 = new TH1D("fhPhotons_MC_invmass3", "fhPhotons_MC_minvmass3 (MC-true cut: gamma from different mother); invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_invmass3);
	fhPhotons_MC_invmass4 = new TH1D("fhPhotons_MC_invmass4", "fhPhotons_MC_minvmass4 (MC-true cut: wrong combination of electrons); invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_invmass4);

	fhPhotons_MC_startvertexZ = new TH1D("fhPhotons_MC_startvertexZ", "fhPhotons_MC_startvertexZ; startvertexZ [cm]; #", 1000, -5., 95.);
	fHistoList_recofull.push_back(fhPhotons_MC_startvertexZ);
	
	fhPhotons_MC_motherIdCut = new TH1D("fhPhotons_MC_motherIdCut", "fhPhotons_MC_motherIdCut; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_MC_motherIdCut);

	fhPhotons_Refit_chiDiff = new TH1D("fhPhotons_Refit_chiDiff", "fhPhotons_Refit_chiDiff; difference of chi; #", 1000, -0.1, 99.9);
	fHistoList_recofull.push_back(fhPhotons_Refit_chiDiff);

	fhPhotons_Refit_momentumDiff = new TH1D("fhPhotons_Refit_momentumDiff", "fhPhotons_Refit_momentumDiff; difference of momentum mag; #", 1000, -0.1, 4.9);
	fHistoList_recofull.push_back(fhPhotons_Refit_momentumDiff);

	fhPhotons_Refit_chiDistribution = new TH1D("fhPhotons_Refit_chiDistribution", "fhPhotons_Refit_chiDistribution; chi value; #", 10000, 0., 1000.);
	fHistoList_recofull.push_back(fhPhotons_Refit_chiDistribution);

	fhPhotons_RefitPion_chiDistribution = new TH1D("fhPhotons_RefitPion_chiDistribution", "fhPhotons_RefitPion_chiDistribution; chi value; #", 10000, 0., 1000.);
	fHistoList_recofull.push_back(fhPhotons_RefitPion_chiDistribution);

	fhPhotons_invmass_refit = new TH1D("fhPhotons_invmass_refit", "fhPhotons_invmass_refit; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_refit);
	fhPhotons_invmass_refit_cut = new TH1D("fhPhotons_invmass_refit_cut", "fhPhotons_invmass_refit_cut; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_refit_cut);
	
	

	fhPhotons_invmass_direction = new TH1D("fhPhotons_invmass_direction", "fhPhotons_invmass_direction; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_direction);
	fhPhotons_invmass_direction_cut = new TH1D("fhPhotons_invmass_direction_cut", "fhPhotons_invmass_direction_cut; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull.push_back(fhPhotons_invmass_direction_cut);
	fhPhotons_boostAngle = new TH1D("fhPhotons_boostAngle", "fhPhotons_boostAngle; boostAngle; #", 7400, -370., 370.);
	fHistoList_recofull.push_back(fhPhotons_boostAngle);
	fhPhotons_boostAngleMC = new TH1D("fhPhotons_boostAngleMC", "fhPhotons_boostAngleMC; boostAngle; #", 7400, -370., 370.);
	fHistoList_recofull.push_back(fhPhotons_boostAngleMC);
	fhPhotons_boostAngleTest = new TH1D("fhPhotons_boostAngleTest", "fhPhotons_boostAngleTest; boostAngle; #", 10000, -5., 5.);
	fHistoList_recofull.push_back(fhPhotons_boostAngleTest);
	

	fhPhotons_tX = new TH1D("fhPhotons_tX", "fhPhotons_tX; tX; #", 201, -1.005, 1.005);
	fHistoList_recofull.push_back(fhPhotons_tX);
	fhPhotons_tY = new TH1D("fhPhotons_tY", "fhPhotons_tY; tY; #", 201, -1.005, 1.005);
	fHistoList_recofull.push_back(fhPhotons_tY);
	



	// histograms for index = 1 (no refit, no chicut)
	fhElectrons_nofPerEvent_1 = new TH1D("fhElectrons_nofPerEvent_1", "fhElectrons_nofPerEvent_1; nof electrons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull_1.push_back(fhElectrons_nofPerEvent_1);
	fhPhotons_nofPerEvent_1 = new TH1D("fhPhotons_nofPerEvent_1", "fhPhotons_nofPerEvent_1; nof photons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull_1.push_back(fhPhotons_nofPerEvent_1);
	
	fhPhotons_invmass_1 = new TH1D("fhPhotons_invmass_1", "fhPhotons_invmass_1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_1.push_back(fhPhotons_invmass_1);
	
	fhPhotons_invmass_ptBin1_1 = new TH1D("fhPhotons_invmass_ptBin1_1", "fhPhotons_invmass_ptBin1_1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_1.push_back(fhPhotons_invmass_ptBin1_1);
	fhPhotons_invmass_ptBin2_1 = new TH1D("fhPhotons_invmass_ptBin2_1", "fhPhotons_invmass_ptBin2_1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_1.push_back(fhPhotons_invmass_ptBin2_1);
	fhPhotons_invmass_ptBin3_1 = new TH1D("fhPhotons_invmass_ptBin3_1", "fhPhotons_invmass_ptBin3_1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_1.push_back(fhPhotons_invmass_ptBin3_1);
	fhPhotons_invmass_ptBin4_1 = new TH1D("fhPhotons_invmass_ptBin4_1", "fhPhotons_invmass_ptBin4_1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_1.push_back(fhPhotons_invmass_ptBin4_1);
	
	fhElectrons_invmass_1 = new TH1D("fhElectrons_invmass_1", "fhElectrons_invmass_1; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_1.push_back(fhElectrons_invmass_1);
	fhPhotons_invmass_vs_pt_1 = new TH2D("fhPhotons_invmass_vs_pt_1", "fhPhotons_invmass_vs_pt_1; invmass; pt", 600, -0.0025, 2.9975, 100, 0., 10.);
	fHistoList_recofull_1.push_back(fhPhotons_invmass_vs_pt_1);


	// histograms for index = 2 (no refit, with chicut)
	fhElectrons_nofPerEvent_2 = new TH1D("fhElectrons_nofPerEvent_2", "fhElectrons_nofPerEvent_2; nof electrons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull_2.push_back(fhElectrons_nofPerEvent_2);
	fhPhotons_nofPerEvent_2 = new TH1D("fhPhotons_nofPerEvent_2", "fhPhotons_nofPerEvent_2; nof photons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull_2.push_back(fhPhotons_nofPerEvent_2);
	
	fhPhotons_invmass_2 = new TH1D("fhPhotons_invmass_2", "fhPhotons_invmass_2; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_2.push_back(fhPhotons_invmass_2);
	
	fhPhotons_invmass_ptBin1_2 = new TH1D("fhPhotons_invmass_ptBin1_2", "fhPhotons_invmass_ptBin1_2; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_2.push_back(fhPhotons_invmass_ptBin1_2);
	fhPhotons_invmass_ptBin2_2 = new TH1D("fhPhotons_invmass_ptBin2_2", "fhPhotons_invmass_ptBin2_2; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_2.push_back(fhPhotons_invmass_ptBin2_2);
	fhPhotons_invmass_ptBin3_2 = new TH1D("fhPhotons_invmass_ptBin3_2", "fhPhotons_invmass_ptBin3_2; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_2.push_back(fhPhotons_invmass_ptBin3_2);
	fhPhotons_invmass_ptBin4_2 = new TH1D("fhPhotons_invmass_ptBin4_2", "fhPhotons_invmass_ptBin4_2; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_2.push_back(fhPhotons_invmass_ptBin4_2);
	
	fhElectrons_invmass_2 = new TH1D("fhElectrons_invmass_2", "fhElectrons_invmass_2; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_2.push_back(fhElectrons_invmass_2);
	fhPhotons_invmass_vs_pt_2 = new TH2D("fhPhotons_invmass_vs_pt_2", "fhPhotons_invmass_vs_pt_2; invmass; pt", 600, -0.0025, 2.9975, 100, 0., 10.);
	fHistoList_recofull_2.push_back(fhPhotons_invmass_vs_pt_2);


	// histograms for index = 3 (with refit, no chicut)
	fhElectrons_nofPerEvent_3 = new TH1D("fhElectrons_nofPerEvent_3", "fhElectrons_nofPerEvent_3; nof electrons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull_3.push_back(fhElectrons_nofPerEvent_3);
	fhPhotons_nofPerEvent_3 = new TH1D("fhPhotons_nofPerEvent_3", "fhPhotons_nofPerEvent_3; nof photons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull_3.push_back(fhPhotons_nofPerEvent_3);
	
	fhPhotons_invmass_3 = new TH1D("fhPhotons_invmass_3", "fhPhotons_invmass_3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_3.push_back(fhPhotons_invmass_3);
	
	fhPhotons_invmass_ptBin1_3 = new TH1D("fhPhotons_invmass_ptBin1_3", "fhPhotons_invmass_ptBin1_3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_3.push_back(fhPhotons_invmass_ptBin1_3);
	fhPhotons_invmass_ptBin2_3 = new TH1D("fhPhotons_invmass_ptBin2_3", "fhPhotons_invmass_ptBin2_3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_3.push_back(fhPhotons_invmass_ptBin2_3);
	fhPhotons_invmass_ptBin3_3 = new TH1D("fhPhotons_invmass_ptBin3_3", "fhPhotons_invmass_ptBin3_3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_3.push_back(fhPhotons_invmass_ptBin3_3);
	fhPhotons_invmass_ptBin4_3 = new TH1D("fhPhotons_invmass_ptBin4_3", "fhPhotons_invmass_ptBin4_3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_3.push_back(fhPhotons_invmass_ptBin4_3);
	
	fhElectrons_invmass_3 = new TH1D("fhElectrons_invmass_3", "fhElectrons_invmass_3; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_3.push_back(fhElectrons_invmass_3);
	fhPhotons_invmass_vs_pt_3 = new TH2D("fhPhotons_invmass_vs_pt_3", "fhPhotons_invmass_vs_pt_3; invmass; pt", 600, -0.0025, 2.9975, 100, 0., 10.);
	fHistoList_recofull_3.push_back(fhPhotons_invmass_vs_pt_3);


	// histograms for index = 4 (with refit, with chicut)
	fhElectrons_nofPerEvent_4 = new TH1D("fhElectrons_nofPerEvent_4", "fhElectrons_nofPerEvent_4; nof electrons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull_4.push_back(fhElectrons_nofPerEvent_4);
	fhPhotons_nofPerEvent_4 = new TH1D("fhPhotons_nofPerEvent_4", "fhPhotons_nofPerEvent_4; nof photons per event; #", 101, -0.5, 100.5);
	fHistoList_recofull_4.push_back(fhPhotons_nofPerEvent_4);
	
	fhPhotons_invmass_4 = new TH1D("fhPhotons_invmass_4", "fhPhotons_invmass_4; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_4.push_back(fhPhotons_invmass_4);
	
	fhPhotons_invmass_ptBin1_4 = new TH1D("fhPhotons_invmass_ptBin1_4", "fhPhotons_invmass_ptBin1_4; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_4.push_back(fhPhotons_invmass_ptBin1_4);
	fhPhotons_invmass_ptBin2_4 = new TH1D("fhPhotons_invmass_ptBin2_4", "fhPhotons_invmass_ptBin2_4; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_4.push_back(fhPhotons_invmass_ptBin2_4);
	fhPhotons_invmass_ptBin3_4 = new TH1D("fhPhotons_invmass_ptBin3_4", "fhPhotons_invmass_ptBin3_4; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_4.push_back(fhPhotons_invmass_ptBin3_4);
	fhPhotons_invmass_ptBin4_4 = new TH1D("fhPhotons_invmass_ptBin4_4", "fhPhotons_invmass_ptBin4_4; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_4.push_back(fhPhotons_invmass_ptBin4_4);
	
	fhElectrons_invmass_4 = new TH1D("fhElectrons_invmass_4", "fhElectrons_invmass_4; invariant mass; #", 600, -0.0025, 2.9975);
	fHistoList_recofull_4.push_back(fhElectrons_invmass_4);
	fhPhotons_invmass_vs_pt_4 = new TH2D("fhPhotons_invmass_vs_pt_4", "fhPhotons_invmass_vs_pt_4; invmass; pt", 600, -0.0025, 2.9975, 100, 0., 10.);
	fHistoList_recofull_4.push_back(fhPhotons_invmass_vs_pt_4);



	for(int i=1; i<=4; i++) {
		fhElectrons_nofPerEvent_new[i] = new TH1D(Form("fhElectrons_nofPerEvent_new_%i",i), Form("fhElectrons_nofPerEvent_new_%i; nof electrons per event; #",i), 101, -0.5, 100.5);
		fHistoList_recofull_new[i].push_back(fhElectrons_nofPerEvent_new[i]);
		fhPhotons_nofPerEvent_new[i] = new TH1D(Form("fhPhotons_nofPerEvent_new_%i",i), Form("fhPhotons_nofPerEvent_new_%i; nof photons per event; #",i), 101, -0.5, 100.5);
		fHistoList_recofull_new[i].push_back(fhPhotons_nofPerEvent_new[i]);

		fhPhotons_invmass_new[i] = new TH1D(Form("fhPhotons_invmass_new_%i",i), Form("fhPhotons_invmass_new_%i; invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_new[i]);

		fhPhotons_invmass_ptBin1_new[i] = new TH1D(Form("fhPhotons_invmass_ptBin1_new_%i",i), Form("fhPhotons_invmass_ptBin1_new_%i; invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_ptBin1_new[i]);
		fhPhotons_invmass_ptBin2_new[i] = new TH1D(Form("fhPhotons_invmass_ptBin2_new_%i",i), Form("fhPhotons_invmass_ptBin2_new_%i; invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_ptBin2_new[i]);
		fhPhotons_invmass_ptBin3_new[i] = new TH1D(Form("fhPhotons_invmass_ptBin3_new_%i",i), Form("fhPhotons_invmass_ptBin3_new_%i; invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_ptBin3_new[i]);
		fhPhotons_invmass_ptBin4_new[i] = new TH1D(Form("fhPhotons_invmass_ptBin4_new_%i",i), Form("fhPhotons_invmass_ptBin4_new_%i; invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_ptBin4_new[i]);
	
		fhElectrons_invmass_new[i] = new TH1D(Form("fhElectrons_invmass_new_%i",i), Form("fhElectrons_invmass_new_%i; invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhElectrons_invmass_new[i]);
		fhPhotons_invmass_vs_pt_new[i] = new TH2D(Form("fhPhotons_invmass_vs_pt_new_%i",i), Form("fhPhotons_invmass_vs_pt_new_%i; invmass; pt",i), 600, -0.0025, 2.9975, 100, 0., 10.);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_vs_pt_new[i]);

		fhPhotons_invmass_MCcut1_new[i] = new TH1D(Form("fhPhotons_invmass_MCcut1_new_%i",i), Form("fhPhotons_invmass_MCcut1_new_%i (MC-true cut: at least gamma correct reconstructed); invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_MCcut1_new[i]);
		fhPhotons_invmass_MCcut2_new[i] = new TH1D(Form("fhPhotons_invmass_MCcut2_new_%i",i), Form("fhPhotons_invmass_MCcut2_new_%i (MC-true cut: gamma from same mother); invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_MCcut2_new[i]);
		fhPhotons_invmass_MCcut3_new[i] = new TH1D(Form("fhPhotons_invmass_MCcut3_new_%i",i), Form("fhPhotons_invmass_MCcut3_new_%i (MC-true cut: gamma from different mother); invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_MCcut3_new[i]);
		fhPhotons_invmass_MCcut4_new[i] = new TH1D(Form("fhPhotons_invmass_MCcut4_new_%i",i), Form("fhPhotons_invmass_MCcut4_new_%i (MC-true cut: wrong combination of electrons); invariant mass; #",i), 600, -0.0025, 2.9975);
		fHistoList_recofull_new[i].push_back(fhPhotons_invmass_MCcut4_new[i]);
	}

}


void CbmAnaConversionRecoFull::Finish()
{
	gDirectory->mkdir("RecoFull");
	gDirectory->cd("RecoFull");
	
	gDirectory->mkdir("NoRefit-NoChicut");		// index 1
	gDirectory->cd("NoRefit-NoChicut");
	for (UInt_t i = 0; i < fHistoList_recofull_new[1].size(); i++){
	//for (UInt_t i = 0; i < fHistoList_recofull_1.size(); i++){
		//fHistoList_recofull_1[i]->Write();
		fHistoList_recofull_new[1][i]->Write();
	}
	gDirectory->cd("..");
	
	gDirectory->mkdir("NoRefit-WithChicut");	// index 2
	gDirectory->cd("NoRefit-WithChicut");
	for (UInt_t i = 0; i < fHistoList_recofull_new[2].size(); i++){
	//for (UInt_t i = 0; i < fHistoList_recofull_2.size(); i++){
		//fHistoList_recofull_2[i]->Write();
		fHistoList_recofull_new[2][i]->Write();
	}
	gDirectory->cd("..");
	
	gDirectory->mkdir("WithRefit-NoChicut");	// index 3
	gDirectory->cd("WithRefit-NoChicut");
	for (UInt_t i = 0; i < fHistoList_recofull_new[3].size(); i++){
	//for (UInt_t i = 0; i < fHistoList_recofull_3.size(); i++){
		//fHistoList_recofull_3[i]->Write();
		fHistoList_recofull_new[3][i]->Write();
	}
	gDirectory->cd("..");
	
	gDirectory->mkdir("WithRefit-WithChicut");	// index 4
	gDirectory->cd("WithRefit-WithChicut");
	for (UInt_t i = 0; i < fHistoList_recofull_new[4].size(); i++){
	//for (UInt_t i = 0; i < fHistoList_recofull_4.size(); i++){
		//fHistoList_recofull_4[i]->Write();
		fHistoList_recofull_new[4][i]->Write();
	}
	gDirectory->cd("..");

	for (UInt_t i = 0; i < fHistoList_recofull.size(); i++){
		fHistoList_recofull[i]->Write();
	}
	gDirectory->cd("..");


	cout << "CbmAnaConversionRecoFull: Realtime - " << fTime << endl;


}

void CbmAnaConversionRecoFull::Exec()
{
	timer.Start();


	if (fPrimVertex != NULL){
		fKFVertex = CbmKFVertex(*fPrimVertex);
	} else {
		Fatal("CbmAnaConversion::Exec","No PrimaryVertex array!");
	}


	fElectrons_track.clear();
	fElectrons_momenta.clear();
	fElectrons_momentaChi.clear();
	fElectrons_mctrackID.clear();
	fVector_photons_pairs.clear();

	fElectrons_track_1.clear();
	fElectrons_momenta_1.clear();
	fElectrons_momentaChi_1.clear();
	fElectrons_mctrackID_1.clear();
	fVector_photons_pairs_1.clear();

	fElectrons_track_2.clear();
	fElectrons_momenta_2.clear();
	fElectrons_momentaChi_2.clear();
	fElectrons_mctrackID_2.clear();
	fVector_photons_pairs_2.clear();

	fElectrons_track_3.clear();
	fElectrons_momenta_3.clear();
	fElectrons_momentaChi_3.clear();
	fElectrons_mctrackID_3.clear();
	fVector_photons_pairs_3.clear();

	fElectrons_track_4.clear();
	fElectrons_momenta_4.clear();
	fElectrons_momentaChi_4.clear();
	fElectrons_mctrackID_4.clear();
	fVector_photons_pairs_4.clear();


	for(int i=1; i<=4; i++) {
		fElectrons_track_new[i].clear();
		fElectrons_momenta_new[i].clear();
		fElectrons_momentaChi_new[i].clear();
		fElectrons_mctrackID_new[i].clear();
		fVector_photons_pairs_new[i].clear();
	}


	
	fElectrons_track_refit.clear();
	fElectrons_momenta_refit.clear();
	fVector_photons_pairs_refit.clear();

	fVector_photons_pairs_direction.clear();

	Int_t nofGT_richsts = 0;
	Int_t nofElectrons = 0;
	Int_t nofElectrons_1 = 0;
	Int_t nofElectrons_2 = 0;
	Int_t nofElectrons_3 = 0;
	Int_t nofElectrons_4 = 0;

	// everything related to reconstructed data
	Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
	cout << "CbmAnaConversionRecoFull: number of global tracks " << nofGlobalTracks << endl;
	for (int iG = 0; iG < nofGlobalTracks; iG++){
		CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(iG);
		if(NULL == gTrack) continue;
		int stsInd = gTrack->GetStsTrackIndex();
		int richInd = gTrack->GetRichRingIndex();
		if (richInd < 0) continue;
		if (stsInd < 0) continue;
		
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
		if (stsTrack == NULL) continue;

		CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*)fStsTrackMatches->At(stsInd);
		if (stsMatch == NULL) continue;
		int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
		if (stsMcTrackId < 0) continue;
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
		if (mcTrack1 == NULL) continue;

		CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(richInd);
		if (richMatch == NULL) continue;
		int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
		if (richMcTrackId < 0) continue;
		CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
		if (mcTrack2 == NULL) continue;

		//if(stsMcTrackId != richMcTrackId) continue;
		nofGT_richsts++;

//		int pdg = TMath::Abs(mcTrack1->GetPdgCode());


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

//		Double_t result_chi2ndf = stsTracks[0].GetChiSq() / stsTracks[0].GetNDF();
		float result_chi = chiPrim[0];
		fhMomentumFits->Fill(result_chi);
//		Int_t pidHypo = gTrack->GetPidHypo();
		
		// #################################################################
		// ##### up to here standard calculation of reconstructed data #####
		// #################################################################


		// indizes in the following
		// 1 = no refit, no chicut
		// 2 = no refit, with chicut
		// 3 = with refit, no chicut
		// 4 = with refit, with chicut



		Bool_t electron_rich2 = electronidentifier->IsRichElectron(iG, refittedMomentum.Mag());
		
		
		Double_t chiCut = 0;
		if(refittedMomentum.Perp() < 0.4) {
			chiCut = 31. - 70.*refittedMomentum.Perp();
		}
		if(refittedMomentum.Perp() >= 0.4) {
			chiCut = 3;
		}
		
		
		if(electron_rich2) {	// electron identification without refit of momentum
			nofElectrons_1++;
			fElectrons_track_1.push_back(gTrack);
			fElectrons_momenta_1.push_back(refittedMomentum);
			fElectrons_momentaChi_1.push_back(result_chi);
			//fElectrons_mctrackID_1.push_back(richMcTrackId);
			fElectrons_mctrackID_1.push_back(stsMcTrackId);
			
			fElectrons_track_new[1].push_back(gTrack);
			fElectrons_momenta_new[1].push_back(refittedMomentum);
			fElectrons_momentaChi_new[1].push_back(result_chi);
			//fElectrons_mctrackID_1.push_back(richMcTrackId);
			fElectrons_mctrackID_new[1].push_back(stsMcTrackId);
			
			
			// using the old method
			fElectrons_track.push_back(gTrack);
			fElectrons_momenta.push_back(refittedMomentum);
			fElectrons_momentaChi.push_back(result_chi);
			//fElectrons_mctrackID_1.push_back(richMcTrackId);
			fElectrons_mctrackID.push_back(stsMcTrackId);
			
			if(result_chi <= chiCut) {
				nofElectrons_2++;
				fElectrons_track_2.push_back(gTrack);
				fElectrons_momenta_2.push_back(refittedMomentum);
				fElectrons_momentaChi_2.push_back(result_chi);
				//fElectrons_mctrackID_2.push_back(richMcTrackId);
				fElectrons_mctrackID_2.push_back(stsMcTrackId);
			
				fElectrons_track_new[2].push_back(gTrack);
				fElectrons_momenta_new[2].push_back(refittedMomentum);
				fElectrons_momentaChi_new[2].push_back(result_chi);
				//fElectrons_mctrackID_1.push_back(richMcTrackId);
				fElectrons_mctrackID_new[2].push_back(stsMcTrackId);
			}
		}



		// Doing refit of momenta

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

		//cout << "refitted momenta!" << endl;



		Bool_t electron_rich = electronidentifier->IsRichElectron(iG, refittedMomentum.Mag());
		//Bool_t electron_rich = (pdg == 11);
		Bool_t electron_trd = electronidentifier->IsTrdElectron(iG, refittedMomentum.Mag());
		Bool_t electron_tof = electronidentifier->IsTofElectron(iG, refittedMomentum.Mag());



		if(electron_rich) {	// electron identification with refit of momentum
			nofElectrons_3++;
			fElectrons_track_3.push_back(gTrack);
			fElectrons_momenta_3.push_back(refittedMomentum);
			fElectrons_momentaChi_3.push_back(result_chi);
			//fElectrons_mctrackID_3.push_back(richMcTrackId);
			fElectrons_mctrackID_3.push_back(stsMcTrackId);
			
			fElectrons_track_new[3].push_back(gTrack);
			fElectrons_momenta_new[3].push_back(refittedMomentum);
			fElectrons_momentaChi_new[3].push_back(result_chi);
			//fElectrons_mctrackID_1.push_back(richMcTrackId);
			fElectrons_mctrackID_new[3].push_back(stsMcTrackId);
			
			if(result_chi <= chiCut) {
				nofElectrons_4++;
				fElectrons_track_4.push_back(gTrack);
				fElectrons_momenta_4.push_back(refittedMomentum);
				fElectrons_momentaChi_4.push_back(result_chi);
				//fElectrons_mctrackID_4.push_back(richMcTrackId);
				fElectrons_mctrackID_4.push_back(stsMcTrackId);
			
				fElectrons_track_new[4].push_back(gTrack);
				fElectrons_momenta_new[4].push_back(refittedMomentum);
				fElectrons_momentaChi_new[4].push_back(result_chi);
				//fElectrons_mctrackID_1.push_back(richMcTrackId);
				fElectrons_mctrackID_new[4].push_back(stsMcTrackId);
			}
			
			fhMomentumFits_electronRich->Fill(result_chi);
			TVector3 startvertex;
			mcTrack2->GetStartVertex(startvertex);
			fhPhotons_startvertex_vs_chi->Fill(startvertex.Z(), result_chi);
		}





		if(electron_rich) fhElectrons->Fill(0);
		if(electron_trd) fhElectrons->Fill(1);
		if(electron_tof) fhElectrons->Fill(2);
		if(electron_rich && electron_trd) fhElectrons->Fill(3);
		if(electron_rich && electron_tof) fhElectrons->Fill(4);
		if(electron_trd && electron_tof) fhElectrons->Fill(5);
		if(electron_rich && electron_trd && electron_tof) fhElectrons->Fill(6);
		if( (electron_rich && electron_trd) || (electron_rich && electron_tof) || (electron_trd && electron_tof) ) fhElectrons->Fill(7);
	}


	fhElectrons_nofPerEvent->Fill(nofElectrons);
	fhElectrons_nofPerEvent_1->Fill(nofElectrons_1);
	fhElectrons_nofPerEvent_2->Fill(nofElectrons_2);
	fhElectrons_nofPerEvent_3->Fill(nofElectrons_3);
	fhElectrons_nofPerEvent_4->Fill(nofElectrons_4);

	cout << "CbmAnaConversionRecoFull: number of global tracks in STS and RICH " << nofGT_richsts << endl;

	CombineElectrons();
	CombinePhotons();

	cout << "CbmAnaConversionRecoFull: combining for each case!" << endl;
	
	for(int i=1; i<=4; i++) {
		CombineElectrons(fElectrons_track_new[i], fElectrons_momenta_new[i], fElectrons_momentaChi_new[i], fElectrons_mctrackID_new[i], fVector_photons_pairs_new[i], i);
		CombinePhotons(fElectrons_track_new[i], fElectrons_momenta_new[i], fElectrons_momentaChi_new[i], fElectrons_mctrackID_new[i], fVector_photons_pairs_new[i], i);
	}
	
	CombineElectrons(fElectrons_track_1, fElectrons_momenta_1, fElectrons_momentaChi_1, fElectrons_mctrackID_1, fVector_photons_pairs_1, 1);
	CombinePhotons(fElectrons_track_1, fElectrons_momenta_1, fElectrons_momentaChi_1, fElectrons_mctrackID_1, fVector_photons_pairs_1, 1);
	
	CombineElectrons(fElectrons_track_2, fElectrons_momenta_2, fElectrons_momentaChi_2, fElectrons_mctrackID_2, fVector_photons_pairs_2, 2);
	CombinePhotons(fElectrons_track_2, fElectrons_momenta_2, fElectrons_momentaChi_2, fElectrons_mctrackID_2, fVector_photons_pairs_2, 2);
	
	CombineElectrons(fElectrons_track_3, fElectrons_momenta_3, fElectrons_momentaChi_3, fElectrons_mctrackID_3, fVector_photons_pairs_3, 3);
	CombinePhotons(fElectrons_track_3, fElectrons_momenta_3, fElectrons_momentaChi_3, fElectrons_mctrackID_3, fVector_photons_pairs_3, 3);
	
	CombineElectrons(fElectrons_track_4, fElectrons_momenta_4, fElectrons_momentaChi_4, fElectrons_mctrackID_4, fVector_photons_pairs_4, 4);
	CombinePhotons(fElectrons_track_4, fElectrons_momenta_4, fElectrons_momentaChi_4, fElectrons_mctrackID_4, fVector_photons_pairs_4, 4);

	//CombineElectronsRefit();
	//CombinePhotonsRefit();

	//CombinePhotonsDirection();

	timer.Stop();
	fTime += timer.RealTime();
}



//void CbmAnaConversionRecoFull::CombineElectrons(	vector<CbmGlobalTrack*> gtrack, vector<TVector3> momenta, vector<float> momentaChi, vector<int> mctrackID)
void CbmAnaConversionRecoFull::CombineElectrons(vector<CbmGlobalTrack*> gtrack, vector<TVector3> momenta, vector<float>, vector<int>, vector< vector<int> > reconstructedPhotons, Int_t index)
{
	Int_t nof = momenta.size();
	cout << "CbmAnaConversionRecoFull: " << index << ": CombineElectrons, nof - " << nof << endl;
	Int_t nofPhotons = 0;
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t check1 = (gtrack[a]->GetParamLast()->GetQp() > 0);	// positive or negative charge (qp = charge over momentum ratio)
				Int_t check2 = (gtrack[b]->GetParamLast()->GetQp() > 0);
				Int_t test = check1 + check2;
				if(test != 1) continue;		// need one electron and one positron
				//if(fElectrons_momentaChi[a] > 10 || fElectrons_momentaChi[b] > 10) continue;
				
				CbmLmvmKinematicParams params1 = CalculateKinematicParamsReco(momenta[a], momenta[b]);
				
				// standard fixed opening angle cut
				//Double_t openingAngleCut = 1;
				
				// opening angle cut depending on pt of e+e- pair
				Double_t openingAngleCut = 1.5 - 0.5 * params1.fPt;
				
				Double_t invMassCut = 0.03;
				
				Int_t IsPhoton_openingAngle1	= (params1.fAngle < openingAngleCut);
				Int_t IsPhoton_invMass1			= (params1.fMinv < invMassCut);
				
/*
				Double_t tXa = gtrack[a]->GetParamLast()->GetTx();
				Double_t tYa = gtrack[a]->GetParamLast()->GetTy();
				Double_t tXb = gtrack[b]->GetParamLast()->GetTx();
				Double_t tYb = gtrack[b]->GetParamLast()->GetTy();
*/
				
				if(IsPhoton_openingAngle1 && IsPhoton_invMass1) {
					nofPhotons++;
					vector<int> pair; // = {a, b};
					pair.push_back(a);
					pair.push_back(b);
					reconstructedPhotons.push_back(pair);
					//fhElectrons_invmass_cut->Fill(params1.fMinv);
					
					if(index == 1) {
						fhElectrons_invmass_1->Fill(params1.fMinv);
						fVector_photons_pairs_1.push_back(pair);
						//fVector_photons_pairs.push_back(pair);
					}
					if(index == 2) {
						fhElectrons_invmass_2->Fill(params1.fMinv);
						fVector_photons_pairs_2.push_back(pair);
					}
					if(index == 3) {
						fhElectrons_invmass_3->Fill(params1.fMinv);
						fVector_photons_pairs_3.push_back(pair);
					}
					if(index == 4) {
						fhElectrons_invmass_4->Fill(params1.fMinv);
						fVector_photons_pairs_4.push_back(pair);
					}
					
					fhElectrons_invmass_new[index]->Fill(params1.fMinv);
					fVector_photons_pairs_new[index].push_back(pair);
					
					
					//fVector_photons_momenta.push_back(params1.momPair);
					
					//fhPhotons_tX->Fill(tXa);
					//fhPhotons_tX->Fill(tXb);
					//fhPhotons_tY->Fill(tYa);
					//fhPhotons_tY->Fill(tYb);
					
					TVector3 momentumE1f;
					TVector3 momentumE2f;
					gtrack[a]->GetParamFirst()->Momentum(momentumE1f);
					gtrack[b]->GetParamFirst()->Momentum(momentumE2f);
					TVector3 momentumE1l;
					TVector3 momentumE2l;
					gtrack[a]->GetParamLast()->Momentum(momentumE1l);
					gtrack[b]->GetParamLast()->Momentum(momentumE2l);
					
					//Double_t energyE1 = TMath::Sqrt(momentumE1.Mag2() + M2E);
					//TLorentzVector lorVecE1(momentumE1, energyE1);
					//Double_t energyE2 = TMath::Sqrt(momentumE2.Mag2() + M2E);
					//TLorentzVector lorVecE2(momentumE2, energyE2);
					//TLorentzVector g = lorVecE1 + lorVecE2;
					
					//lorVecE1.Boost(g.BoostVector() );
					//lorVecE2.Boost(g.BoostVector() );
					
					TVector3 normal1 = momentumE1f.Cross(momentumE1l);
					TVector3 normal2 = momentumE2f.Cross(momentumE2l);
					Double_t normalAngle = normal1.Angle(normal2);
					Double_t theta = 180.*normalAngle/TMath::Pi();
					
					//Double_t boostAngle = lorVecE1.Angle(lorVecE2.Vect());
					
					//fhPhotons_boostAngle->Fill(normalAngle);

					
					
					//if( TMath::Abs(tXa - tXb) < 0.5 || TMath::Abs(tYa - tYb) < 0.5 ) {
					if( theta < 30 ) {
						fVector_photons_pairs_direction.push_back(pair);
					}
				}
			}
		}
	}
	if(index == 1) fhPhotons_nofPerEvent_1->Fill(nofPhotons);
	if(index == 2) fhPhotons_nofPerEvent_2->Fill(nofPhotons);
	if(index == 3) fhPhotons_nofPerEvent_3->Fill(nofPhotons);
	if(index == 4) fhPhotons_nofPerEvent_4->Fill(nofPhotons);
	fhPhotons_nofPerEvent_new[index]->Fill(nofPhotons);
	//fhPhotons_nofPerEvent->Fill(nofPhotons);
	cout << "CbmAnaConversionRecoFull: CombineElectrons: Crosscheck - nof reconstructed photons: " << nofPhotons << endl;
}


Double_t CbmAnaConversionRecoFull::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
// calculation of invariant mass from four electrons/positrons
{
    Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
    TLorentzVector lorVec1(part1, energy1);

    Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
    TLorentzVector lorVec2(part2, energy2);

    Double_t energy3 = TMath::Sqrt(part3.Mag2() + M2E);
    TLorentzVector lorVec3(part3, energy3);

    Double_t energy4 = TMath::Sqrt(part4.Mag2() + M2E);
    TLorentzVector lorVec4(part4, energy4);
    
    TLorentzVector sum;
    sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;    

	return sum.Mag();
}



Double_t CbmAnaConversionRecoFull::Pt_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
{
    Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
    TLorentzVector lorVec1(part1, energy1);

    Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
    TLorentzVector lorVec2(part2, energy2);

    Double_t energy3 = TMath::Sqrt(part3.Mag2() + M2E);
    TLorentzVector lorVec3(part3, energy3);

    Double_t energy4 = TMath::Sqrt(part4.Mag2() + M2E);
    TLorentzVector lorVec4(part4, energy4);
    
    TLorentzVector sum;
    sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;    

	Double_t perp = sum.Perp();
	Double_t pt = TMath::Sqrt(sum.X() * sum.X() + sum.Y() * sum.Y() );
	
	cout << "CbmAnaConversionRecoFull::Pt_4particlesRECO: perp/pt = " << perp << " / " << pt << endl;

	return perp;
}



Double_t CbmAnaConversionRecoFull::Rap_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
{
    Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
    TLorentzVector lorVec1(part1, energy1);

    Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
    TLorentzVector lorVec2(part2, energy2);

    Double_t energy3 = TMath::Sqrt(part3.Mag2() + M2E);
    TLorentzVector lorVec3(part3, energy3);

    Double_t energy4 = TMath::Sqrt(part4.Mag2() + M2E);
    TLorentzVector lorVec4(part4, energy4);
    
    TLorentzVector sum;
    sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;    

	
    TVector3 momPair = part1 + part2 + part3 + part4;
    Double_t energyPair = energy1 + energy2 + energy3 + energy4;
    Double_t pzPair = momPair.Pz();
    Double_t yPair = 0.5*TMath::Log((energyPair+pzPair)/(energyPair-pzPair));
	
	cout << "CbmAnaConversionRecoFull::Rap_4particlesRECO: yPair = " << yPair << endl;

	return yPair;
}




CbmLmvmKinematicParams CbmAnaConversionRecoFull::CalculateKinematicParamsReco(const TVector3 electron1, const TVector3 electron2)
{
	CbmLmvmKinematicParams params;

    Double_t energyP = TMath::Sqrt(electron1.Mag2() + M2E);
    TLorentzVector lorVecP(electron1, energyP);

    Double_t energyM = TMath::Sqrt(electron2.Mag2() + M2E);
    TLorentzVector lorVecM(electron2, energyM);

    TVector3 momPair = electron1 + electron2;
    Double_t energyPair = energyP + energyM;
    Double_t ptPair = momPair.Perp();
    Double_t pzPair = momPair.Pz();
    Double_t yPair = 0.5*TMath::Log((energyPair+pzPair)/(energyPair-pzPair));
    Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
    Double_t theta = 180.*anglePair/TMath::Pi();
    Double_t minv = 2.*TMath::Sin(anglePair / 2.)*TMath::Sqrt(electron1.Mag()*electron2.Mag());

    params.fMomentumMag = momPair.Mag();
    params.fPt = ptPair;
    params.fRapidity = yPair;
    params.fMinv = minv;
    params.fAngle = theta;
    return params;
}



CbmLmvmKinematicParams CbmAnaConversionRecoFull::CalculateKinematicParams_4particles(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
{
	CbmLmvmKinematicParams params;

    Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
    TLorentzVector lorVec1(part1, energy1);

    Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
    TLorentzVector lorVec2(part2, energy2);

    Double_t energy3 = TMath::Sqrt(part3.Mag2() + M2E);
    TLorentzVector lorVec3(part3, energy3);

    Double_t energy4 = TMath::Sqrt(part4.Mag2() + M2E);
    TLorentzVector lorVec4(part4, energy4);
    
    TLorentzVector sum;
    sum = lorVec1 + lorVec2 + lorVec3 + lorVec4; 

    TVector3 momPair = part1 + part2 + part3 + part4;
    Double_t energyPair = energy1 + energy2 + energy3 + energy4;
    Double_t ptPair = momPair.Perp();
    Double_t pzPair = momPair.Pz();
    Double_t yPair = 0.5*TMath::Log((energyPair+pzPair)/(energyPair-pzPair));
//    Double_t anglePair = 0;
//    Double_t theta = 180.*anglePair/TMath::Pi();
    Double_t minv = sum.Mag();

    params.fMomentumMag = momPair.Mag();
    params.fPt = ptPair;
    params.fRapidity = yPair;
    params.fMinv = minv;
    params.fAngle = 0;
    return params;
}





void CbmAnaConversionRecoFull::CombinePhotons(vector<CbmGlobalTrack*> gtrack, vector<TVector3> momenta, vector<float> momentaChi, vector<int>, vector< vector<int> > reconstructedPhotons, Int_t index)
// index: kind of electron cut: 1 = no refit, no chicut; 2 = no refit, with chicut; 3 = with refit, no chicut; 4 = with refit, with chicut
{
	if(index == 1) reconstructedPhotons = fVector_photons_pairs_1;
	if(index == 2) reconstructedPhotons = fVector_photons_pairs_2;
	if(index == 3) reconstructedPhotons = fVector_photons_pairs_3;
	if(index == 4) reconstructedPhotons = fVector_photons_pairs_4;
	
	reconstructedPhotons = fVector_photons_pairs_new[index];
	
	Int_t nof = reconstructedPhotons.size();
	cout << "CbmAnaConversionRecoFull: " << index << ": CombinePhotons, nof - " << nof << endl;
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				cout << "CbmAnaConversionRecoFull: calculating invariant mass!" << endl;
				Int_t electron11 = reconstructedPhotons[a][0];
				Int_t electron12 = reconstructedPhotons[a][1];
				Int_t electron21 = reconstructedPhotons[b][0];
				Int_t electron22 = reconstructedPhotons[b][1];
			
				Double_t invmass = Invmass_4particlesRECO(momenta[electron11], momenta[electron12], momenta[electron21], momenta[electron22]);
				
				if(index == 1) fhPhotons_invmass_1->Fill(invmass);
				if(index == 2) fhPhotons_invmass_2->Fill(invmass);
				if(index == 3) fhPhotons_invmass_3->Fill(invmass);
				if(index == 4) fhPhotons_invmass_4->Fill(invmass);
				fhPhotons_invmass_new[index]->Fill(invmass);
				
				Double_t pt = Pt_4particlesRECO(momenta[electron11], momenta[electron12], momenta[electron21], momenta[electron22]);
				Double_t rap = Rap_4particlesRECO(momenta[electron11], momenta[electron12], momenta[electron21], momenta[electron22]);
				
				Double_t opening_angle = OpeningAngleBetweenPhotons(momenta, reconstructedPhotons[a], reconstructedPhotons[b]);
				if(index == 1) fhPhotons_angleBetween->Fill(opening_angle);
				
				
				CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(momenta[electron11], momenta[electron12], momenta[electron21], momenta[electron22]);
				
				
				TVector3 momentumE1;
				TVector3 momentumE2;
				gtrack[a]->GetParamLast()->Momentum(momentumE1);
				gtrack[b]->GetParamLast()->Momentum(momentumE2);
					
				Double_t energyE11 = TMath::Sqrt(momenta[electron11].Mag2() + M2E);
				TLorentzVector lorVecE11(momenta[electron11], energyE11);
				Double_t energyE12 = TMath::Sqrt(momenta[electron12].Mag2() + M2E);
				TLorentzVector lorVecE12(momenta[electron12], energyE12);
				Double_t energyE21 = TMath::Sqrt(momenta[electron21].Mag2() + M2E);
				TLorentzVector lorVecE21(momenta[electron21], energyE21);
				Double_t energyE22 = TMath::Sqrt(momenta[electron22].Mag2() + M2E);
				TLorentzVector lorVecE22(momenta[electron22], energyE22);
				TLorentzVector g1 = lorVecE11 + lorVecE12;
				TLorentzVector g2 = lorVecE21 + lorVecE22;
				TLorentzVector pi = lorVecE11 + lorVecE12 + lorVecE21 + lorVecE22;
					
				g1.Boost(-pi.BoostVector() );
				g2.Boost(-pi.BoostVector() );
					
				Double_t boostAngle = g1.Angle(g2.Vect());
				Double_t theta = 180.*boostAngle/TMath::Pi();
				
				if(index == 1) fhPhotons_boostAngleTest->Fill(boostAngle);
					
				//if(opening_angle < 10) {
				if(true) {
					if(false) {
					fhPhotons_boostAngle->Fill(theta);
				
					fhPhotons_invmass_cut->Fill(invmass);
					fhPhotons_invmass_vs_pt->Fill(invmass, pt);
					fhPhotons_rapidity_vs_pt->Fill(pt, rap);
					fhPhotons_invmass_vs_openingAngle->Fill(invmass, opening_angle);
					fhPhotons_openingAngle_vs_momentum->Fill(opening_angle, params1.fMomentumMag);
					
					fhMomentumFits_pi0reco->Fill(momentaChi[electron11]);
					fhMomentumFits_pi0reco->Fill(momentaChi[electron12]);
					fhMomentumFits_pi0reco->Fill(momentaChi[electron21]);
					fhMomentumFits_pi0reco->Fill(momentaChi[electron22]);
					
					fhPhotons_invmass_vs_chi->Fill(invmass, momentaChi[electron11]);
					fhPhotons_invmass_vs_chi->Fill(invmass, momentaChi[electron12]);
					fhPhotons_invmass_vs_chi->Fill(invmass, momentaChi[electron21]);
					fhPhotons_invmass_vs_chi->Fill(invmass, momentaChi[electron22]);
					}
					
					if(index == 1) {
						if(pt <= 1) 			fhPhotons_invmass_ptBin1_1->Fill(invmass);
						if(pt > 1 && pt <= 2)	fhPhotons_invmass_ptBin2_1->Fill(invmass);
						if(pt > 2 && pt <= 3)	fhPhotons_invmass_ptBin3_1->Fill(invmass);
						if(pt > 3 && pt <= 4)	fhPhotons_invmass_ptBin4_1->Fill(invmass);
						fhPhotons_invmass_vs_pt_1->Fill(invmass, pt);
					}
					if(index == 2) {
						if(pt <= 1) 			fhPhotons_invmass_ptBin1_2->Fill(invmass);
						if(pt > 1 && pt <= 2)	fhPhotons_invmass_ptBin2_2->Fill(invmass);
						if(pt > 2 && pt <= 3)	fhPhotons_invmass_ptBin3_2->Fill(invmass);
						if(pt > 3 && pt <= 4)	fhPhotons_invmass_ptBin4_2->Fill(invmass);
						fhPhotons_invmass_vs_pt_2->Fill(invmass, pt);
					}
					if(index == 3) {
						if(pt <= 1) 			fhPhotons_invmass_ptBin1_3->Fill(invmass);
						if(pt > 1 && pt <= 2)	fhPhotons_invmass_ptBin2_3->Fill(invmass);
						if(pt > 2 && pt <= 3)	fhPhotons_invmass_ptBin3_3->Fill(invmass);
						if(pt > 3 && pt <= 4)	fhPhotons_invmass_ptBin4_3->Fill(invmass);
						fhPhotons_invmass_vs_pt_3->Fill(invmass, pt);
					}
					if(index == 4) {
						if(pt <= 1) 			fhPhotons_invmass_ptBin1_4->Fill(invmass);
						if(pt > 1 && pt <= 2)	fhPhotons_invmass_ptBin2_4->Fill(invmass);
						if(pt > 2 && pt <= 3)	fhPhotons_invmass_ptBin3_4->Fill(invmass);
						if(pt > 3 && pt <= 4)	fhPhotons_invmass_ptBin4_4->Fill(invmass);
						fhPhotons_invmass_vs_pt_4->Fill(invmass, pt);
					}
					
					if(pt <= 1) 			fhPhotons_invmass_ptBin1_new[index]->Fill(invmass);
					if(pt > 1 && pt <= 2)	fhPhotons_invmass_ptBin2_new[index]->Fill(invmass);
					if(pt > 2 && pt <= 3)	fhPhotons_invmass_ptBin3_new[index]->Fill(invmass);
					if(pt > 3 && pt <= 4)	fhPhotons_invmass_ptBin4_new[index]->Fill(invmass);
					fhPhotons_invmass_vs_pt_new[index]->Fill(invmass, pt);
					
					/*
					Double_t chicut = 1.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi1->Fill(invmass);
					}
					chicut = 3.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi3->Fill(invmass);
					}
					chicut = 5.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi5->Fill(invmass);
					}
					chicut = 10.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi10->Fill(invmass);
					}
					chicut = 25.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi25->Fill(invmass);
					}
					chicut = 40.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi40->Fill(invmass);
					}
					chicut = 65.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi65->Fill(invmass);
					}
					chicut = 80.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi80->Fill(invmass);
					}
					*/
					
					
					// CROSSCHECK WITH MC-TRUE DATA!
					CbmMCTrack* mctrack11 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron11]);
					CbmMCTrack* mctrack12 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron12]);
					CbmMCTrack* mctrack21 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron21]);
					CbmMCTrack* mctrack22 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron22]);
					
					Int_t motherId11 = mctrack11->GetMotherId();
					Int_t motherId12 = mctrack12->GetMotherId();
					Int_t motherId21 = mctrack21->GetMotherId();
					Int_t motherId22 = mctrack22->GetMotherId();
					
					TVector3 startvertex11;
					mctrack11->GetStartVertex(startvertex11);
					TVector3 startvertex12;
					mctrack12->GetStartVertex(startvertex12);
					TVector3 startvertex21;
					mctrack21->GetStartVertex(startvertex21);
					TVector3 startvertex22;
					mctrack22->GetStartVertex(startvertex22);
					
					//fhPhotons_MC_startvertexZ->Fill(startvertex11.Z());
					//fhPhotons_MC_startvertexZ->Fill(startvertex12.Z());
					//fhPhotons_MC_startvertexZ->Fill(startvertex21.Z());
					//fhPhotons_MC_startvertexZ->Fill(startvertex22.Z());
					
					if(motherId11 == motherId12 && motherId21 == motherId22) {
						fhPhotons_invmass_MCcut1_new[index]->Fill(invmass);
						if(motherId11 != -1 && motherId21 != -1) {
							CbmMCTrack* mothermctrack11 = (CbmMCTrack*)fMcTracks->At(motherId11);
							CbmMCTrack* mothermctrack21 = (CbmMCTrack*)fMcTracks->At(motherId21);
							
							Int_t grandmotherId11 = mothermctrack11->GetMotherId();
							Int_t grandmotherId21 = mothermctrack21->GetMotherId();
							
							if(grandmotherId11 == grandmotherId21) {
								fhPhotons_invmass_MCcut2_new[index]->Fill(invmass);
								if(grandmotherId11 == -1) continue;
								CbmMCTrack* pi0mctrack11 = (CbmMCTrack*)fMcTracks->At(grandmotherId11);
								if(pi0mctrack11->GetMotherId() == -1) {
									//fhPhotons_MC_motherIdCut->Fill(invmass);
								}
							}
							if(grandmotherId11 != grandmotherId21) {
								fhPhotons_invmass_MCcut3_new[index]->Fill(invmass);
							}
						}
					}
					
					if(motherId11 != motherId12 || motherId21 != motherId22) {
						fhPhotons_invmass_MCcut4_new[index]->Fill(invmass);
					}
					
					
					/*
					if(motherId11 != -1 && motherId12 != -1 && motherId21 != -1 && motherId22 != -1) {
						CbmMCTrack* mothermctrack11 = (CbmMCTrack*)fMcTracks->At(motherId11);
						CbmMCTrack* mothermctrack12 = (CbmMCTrack*)fMcTracks->At(motherId12);
						CbmMCTrack* mothermctrack21 = (CbmMCTrack*)fMcTracks->At(motherId21);
						CbmMCTrack* mothermctrack22 = (CbmMCTrack*)fMcTracks->At(motherId22);
					
						fhPhotons_MC_motherpdg->Fill(mothermctrack11->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack12->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack21->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack22->GetPdgCode());
					}
					
					
					
					TLorentzVector lorVecE11c;
					TLorentzVector lorVecE12c;
					TLorentzVector lorVecE21c;
					TLorentzVector lorVecE22c;
					
					mctrack11->Get4Momentum(lorVecE11c);
					mctrack12->Get4Momentum(lorVecE12c);
					mctrack21->Get4Momentum(lorVecE21c);
					mctrack22->Get4Momentum(lorVecE22c);
					
					TLorentzVector g1c = lorVecE11c + lorVecE12c;
					TLorentzVector g2c = lorVecE21c + lorVecE22c;
					TLorentzVector pic = lorVecE11c + lorVecE12c + lorVecE21c + lorVecE22c;
					
					g1c.Boost(-pic.BoostVector() );
					g2c.Boost(-pic.BoostVector() );
					
					Double_t boostAnglec = g1c.Angle(g2c.Vect());
					Double_t thetac = 180.*boostAnglec/TMath::Pi();
					fhPhotons_boostAngleMC->Fill(thetac);
					
					
					*/
				}
			}
		}
	}
}





Double_t CbmAnaConversionRecoFull::OpeningAngleBetweenPhotons(vector<TVector3> momenta, vector<int> photon1, vector<int> photon2)
{
	Double_t energy1 = TMath::Sqrt(momenta[photon1[0]].Mag2() + M2E);
	TLorentzVector lorVec1(momenta[photon1[0]], energy1);
	
	Double_t energy2 = TMath::Sqrt(momenta[photon1[1]].Mag2() + M2E);
	TLorentzVector lorVec2(momenta[photon1[1]], energy2);
	
	Double_t energy3 = TMath::Sqrt(momenta[photon2[0]].Mag2() + M2E);
	TLorentzVector lorVec3(momenta[photon2[0]], energy3);
	
	Double_t energy4 = TMath::Sqrt(momenta[photon2[1]].Mag2() + M2E);
	TLorentzVector lorVec4(momenta[photon2[1]], energy4);

	TLorentzVector lorPhoton1 = lorVec1 + lorVec2;
	TLorentzVector lorPhoton2 = lorVec3 + lorVec4;

	Double_t angleBetweenPhotons = lorPhoton1.Angle(lorPhoton2.Vect());
	Double_t theta = 180.*angleBetweenPhotons/TMath::Pi();

	return theta;
}





Double_t CbmAnaConversionRecoFull::OpeningAngleBetweenPhotons2(vector<int> photon1, vector<int> photon2)
{
	Double_t energy1 = TMath::Sqrt(fElectrons_momenta[photon1[0]].Mag2() + M2E);
	TLorentzVector lorVec1(fElectrons_momenta[photon1[0]], energy1);
	
	Double_t energy2 = TMath::Sqrt(fElectrons_momenta[photon1[1]].Mag2() + M2E);
	TLorentzVector lorVec2(fElectrons_momenta[photon1[1]], energy2);
	
	Double_t energy3 = TMath::Sqrt(fElectrons_momenta[photon2[0]].Mag2() + M2E);
	TLorentzVector lorVec3(fElectrons_momenta[photon2[0]], energy3);
	
	Double_t energy4 = TMath::Sqrt(fElectrons_momenta[photon2[1]].Mag2() + M2E);
	TLorentzVector lorVec4(fElectrons_momenta[photon2[1]], energy4);

	TLorentzVector lorPhoton1 = lorVec1 + lorVec2;
	TLorentzVector lorPhoton2 = lorVec3 + lorVec4;

	Double_t angleBetweenPhotons = lorPhoton1.Angle(lorPhoton2.Vect());
	Double_t theta = 180.*angleBetweenPhotons/TMath::Pi();

	return theta;
}





/*
void CbmAnaConversionRecoFull::CombineElectronsRefit()
{
	Int_t nof = fElectrons_momenta_refit.size();
	cout << "CbmAnaConversionRecoFull: CombineElectronsRefit, nof - " << nof << endl;
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t check1 = (fElectrons_track_refit[a]->GetParamLast()->GetQp() > 0);	// positive or negative charge (qp = charge over momentum ratio)
				Int_t check2 = (fElectrons_track_refit[b]->GetParamLast()->GetQp() > 0);
				Int_t test = check1 + check2;
				if(test != 1) continue;		// need one electron and one positron
				//if(fElectrons_momentaChi[a] > 10 || fElectrons_momentaChi[b] > 10) continue;
				
				CbmLmvmKinematicParams params1 = CalculateKinematicParamsReco(fElectrons_momenta_refit[a], fElectrons_momenta_refit[b]);
				
				Double_t openingAngleCut = 1;
				Double_t invMassCut = 0.03;
				
				Int_t IsPhoton_openingAngle1	= (params1.fAngle < openingAngleCut);
				Int_t IsPhoton_invMass1			= (params1.fMinv < invMassCut);
				
				if(IsPhoton_openingAngle1 && IsPhoton_invMass1) {
					vector<int> pair; // = {a, b};
					pair.push_back(a);
					pair.push_back(b);
					fVector_photons_pairs_refit.push_back(pair);
				}
			}
		}
	}
}




void CbmAnaConversionRecoFull::CombinePhotonsRefit()
{
	Int_t nof = fVector_photons_pairs_refit.size();
	cout << "CbmAnaConversionRecoFull: CombinePhotonsRefit, nof - " << nof << endl;
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t electron11 = fVector_photons_pairs_refit[a][0];
				Int_t electron12 = fVector_photons_pairs_refit[a][1];
				Int_t electron21 = fVector_photons_pairs_refit[b][0];
				Int_t electron22 = fVector_photons_pairs_refit[b][1];
			
				Double_t invmass = Invmass_4particlesRECO(fElectrons_momenta_refit[electron11], fElectrons_momenta_refit[electron12], fElectrons_momenta_refit[electron21], fElectrons_momenta_refit[electron22]);
				fhPhotons_invmass_refit->Fill(invmass);
				
				Double_t opening_angle = OpeningAngleBetweenPhotonsRefit(fVector_photons_pairs_refit[a], fVector_photons_pairs_refit[b]);
				//fhPhotons_angleBetween->Fill(opening_angle);
				
				if(opening_angle < 8) {
					fhPhotons_invmass_refit_cut->Fill(invmass);
					
					//Double_t chicut = 1.0;
					//if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
					//	fhPhotons_invmass_cut_chi1->Fill(invmass);
					//}
				}
			}
		}
	}
}




Double_t CbmAnaConversionRecoFull::OpeningAngleBetweenPhotonsRefit(vector<int> photon1, vector<int> photon2)
{
	Double_t energy1 = TMath::Sqrt(fElectrons_momenta_refit[photon1[0]].Mag2() + M2E);
	TLorentzVector lorVec1(fElectrons_momenta_refit[photon1[0]], energy1);
	
	Double_t energy2 = TMath::Sqrt(fElectrons_momenta_refit[photon1[1]].Mag2() + M2E);
	TLorentzVector lorVec2(fElectrons_momenta_refit[photon1[1]], energy2);
	
	Double_t energy3 = TMath::Sqrt(fElectrons_momenta_refit[photon2[0]].Mag2() + M2E);
	TLorentzVector lorVec3(fElectrons_momenta_refit[photon2[0]], energy3);
	
	Double_t energy4 = TMath::Sqrt(fElectrons_momenta_refit[photon2[1]].Mag2() + M2E);
	TLorentzVector lorVec4(fElectrons_momenta_refit[photon2[1]], energy4);

	TLorentzVector lorPhoton1 = lorVec1 + lorVec2;
	TLorentzVector lorPhoton2 = lorVec3 + lorVec4;

	Double_t angleBetweenPhotons = lorPhoton1.Angle(lorPhoton2.Vect());
	Double_t theta = 180.*angleBetweenPhotons/TMath::Pi();

	return theta;
}



void CbmAnaConversionRecoFull::CombinePhotonsDirection()
{
	Int_t nof = fVector_photons_pairs_direction.size();
	cout << "CbmAnaConversionRecoFull: CombinePhotonsDirection, nof - " << nof << endl;
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t electron11 = fVector_photons_pairs_direction[a][0];
				Int_t electron12 = fVector_photons_pairs_direction[a][1];
				Int_t electron21 = fVector_photons_pairs_direction[b][0];
				Int_t electron22 = fVector_photons_pairs_direction[b][1];
			
				Double_t invmass = Invmass_4particlesRECO(fElectrons_momenta[electron11], fElectrons_momenta[electron12], fElectrons_momenta[electron21], fElectrons_momenta[electron22]);
				fhPhotons_invmass_direction->Fill(invmass);
				
				Double_t opening_angle = OpeningAngleBetweenPhotonsRefit(fVector_photons_pairs_direction[a], fVector_photons_pairs_direction[b]);
				//fhPhotons_angleBetween->Fill(opening_angle);
				
				if(opening_angle < 8) {
					fhPhotons_invmass_direction_cut->Fill(invmass);
					
					//Double_t chicut = 1.0;
					//if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
					//	fhPhotons_invmass_cut_chi1->Fill(invmass);
					//}
				}
			}
		}
	}
}
*/


/*
void CbmAnaConversionRecoFull::CombineElectrons()
{
	Int_t nof = fElectrons_momenta.size();
	if(nof >= 4) {
		for(int a=0; a<nof-3; a++) {
			for(int b=a; b<nof-2; b++) {
				for(int c=b; c<nof-1; c++) {
					for(int d=c; d<nof; d++) {
						Int_t check1 = (fElectrons_track[a]->GetParamLast()->GetQp() > 0);	// positive or negative charge (qp = charge over momentum ratio)
						Int_t check2 = (fElectrons_track[b]->GetParamLast()->GetQp() > 0);
						Int_t check3 = (fElectrons_track[c]->GetParamLast()->GetQp() > 0);
						Int_t check4 = (fElectrons_track[d]->GetParamLast()->GetQp() > 0);
						Int_t test = check1 + check2 + check3 + check4;
						if(test != 2) continue;		// need two electrons and two positrons
						
						
						Double_t invmass = Invmass_4particlesRECO(fElectrons_momenta[a], fElectrons_momenta[b], fElectrons_momenta[c], fElectrons_momenta[d]);
						fhElectrons_invmass->Fill(invmass);
						
						
						CbmLmvmKinematicParams params1 = CalculateKinematicParamsReco(fElectrons_momenta[a], fElectrons_momenta[b]);
						CbmLmvmKinematicParams params2 = CalculateKinematicParamsReco(fElectrons_momenta[a], fElectrons_momenta[c]);
						CbmLmvmKinematicParams params3 = CalculateKinematicParamsReco(fElectrons_momenta[a], fElectrons_momenta[d]);
						CbmLmvmKinematicParams params4 = CalculateKinematicParamsReco(fElectrons_momenta[b], fElectrons_momenta[c]);
						CbmLmvmKinematicParams params5 = CalculateKinematicParamsReco(fElectrons_momenta[b], fElectrons_momenta[d]);
						CbmLmvmKinematicParams params6 = CalculateKinematicParamsReco(fElectrons_momenta[c], fElectrons_momenta[d]);
						
						Double_t openingAngleCut = 1;
						Int_t IsPhoton_openingAngle1 = (params1.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle2 = (params2.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle3 = (params3.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle4 = (params4.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle5 = (params5.fAngle < openingAngleCut);
						Int_t IsPhoton_openingAngle6 = (params6.fAngle < openingAngleCut);
						
						Double_t invMassCut = 0.03;
						Int_t IsPhoton_invMass1 = (params1.fMinv < invMassCut);
						Int_t IsPhoton_invMass2 = (params2.fMinv < invMassCut);
						Int_t IsPhoton_invMass3 = (params3.fMinv < invMassCut);
						Int_t IsPhoton_invMass4 = (params4.fMinv < invMassCut);
						Int_t IsPhoton_invMass5 = (params5.fMinv < invMassCut);
						Int_t IsPhoton_invMass6 = (params6.fMinv < invMassCut);
						
						if(IsPhoton_openingAngle1 && IsPhoton_openingAngle6 && IsPhoton_invMass1 && IsPhoton_invMass6 && (check1 + check2 == 1) && (check3 + check4 == 1)) {
							fhElectrons_invmass_cut->Fill(invmass);
						}
						if(IsPhoton_openingAngle2 && IsPhoton_openingAngle5 && IsPhoton_invMass2 && IsPhoton_invMass5 && (check1 + check3 == 1) && (check2 + check4 == 1)) {
							fhElectrons_invmass_cut->Fill(invmass);
						}
						if(IsPhoton_openingAngle3 && IsPhoton_openingAngle4 && IsPhoton_invMass3 && IsPhoton_invMass4 && (check1 + check4 == 1) && (check2 + check3 == 1)) {
							fhElectrons_invmass_cut->Fill(invmass);
						}
					}
				}
			}
		}
	}
}
*/


void CbmAnaConversionRecoFull::CombineElectrons()
{
	Int_t nof = fElectrons_momenta.size();
	cout << "CbmAnaConversionRecoFull: CombineElectrons, nof - " << nof << endl;
	Int_t nofPhotons = 0;
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t check1 = (fElectrons_track[a]->GetParamLast()->GetQp() > 0);	// positive or negative charge (qp = charge over momentum ratio)
				Int_t check2 = (fElectrons_track[b]->GetParamLast()->GetQp() > 0);
				Int_t test = check1 + check2;
				if(test != 1) continue;		// need one electron and one positron
				//if(fElectrons_momentaChi[a] > 10 || fElectrons_momentaChi[b] > 10) continue;
				
				CbmLmvmKinematicParams params1 = CalculateKinematicParamsReco(fElectrons_momenta[a], fElectrons_momenta[b]);
				
				// standard fixed opening angle cut
				//Double_t openingAngleCut = 1;
				
				// opening angle cut depending on pt of e+e- pair
				Double_t openingAngleCut = 1.5 - 0.5 * params1.fPt;
				//Double_t openingAngleCut = 1.2;
				
				Double_t invMassCut = 0.03;
				
				Int_t IsPhoton_openingAngle1	= (params1.fAngle < openingAngleCut);
				Int_t IsPhoton_invMass1			= (params1.fMinv < invMassCut);
				
				Double_t tXa = fElectrons_track[a]->GetParamLast()->GetTx();
				Double_t tYa = fElectrons_track[a]->GetParamLast()->GetTy();
				Double_t tXb = fElectrons_track[b]->GetParamLast()->GetTx();
				Double_t tYb = fElectrons_track[b]->GetParamLast()->GetTy();
				
				fhElectrons_invmass->Fill(params1.fMinv);
				
				if(IsPhoton_openingAngle1 && IsPhoton_invMass1) {
					nofPhotons++;
					vector<int> pair; // = {a, b};
					pair.push_back(a);
					pair.push_back(b);
					fVector_photons_pairs.push_back(pair);
					fhElectrons_invmass_cut->Fill(params1.fMinv);
					//fVector_photons_momenta.push_back(params1.momPair);
					
					fhPhotons_tX->Fill(tXa);
					fhPhotons_tX->Fill(tXb);
					fhPhotons_tY->Fill(tYa);
					fhPhotons_tY->Fill(tYb);
					
					TVector3 momentumE1f;
					TVector3 momentumE2f;
					fElectrons_track[a]->GetParamFirst()->Momentum(momentumE1f);
					fElectrons_track[b]->GetParamFirst()->Momentum(momentumE2f);
					TVector3 momentumE1l;
					TVector3 momentumE2l;
					fElectrons_track[a]->GetParamLast()->Momentum(momentumE1l);
					fElectrons_track[b]->GetParamLast()->Momentum(momentumE2l);
					
					//Double_t energyE1 = TMath::Sqrt(momentumE1.Mag2() + M2E);
					//TLorentzVector lorVecE1(momentumE1, energyE1);
					//Double_t energyE2 = TMath::Sqrt(momentumE2.Mag2() + M2E);
					//TLorentzVector lorVecE2(momentumE2, energyE2);
					//TLorentzVector g = lorVecE1 + lorVecE2;
					
					//lorVecE1.Boost(g.BoostVector() );
					//lorVecE2.Boost(g.BoostVector() );
					
					TVector3 normal1 = momentumE1f.Cross(momentumE1l);
					TVector3 normal2 = momentumE2f.Cross(momentumE2l);
					Double_t normalAngle = normal1.Angle(normal2);
					Double_t theta = 180.*normalAngle/TMath::Pi();
					
					//Double_t boostAngle = lorVecE1.Angle(lorVecE2.Vect());
					
					//fhPhotons_boostAngle->Fill(normalAngle);

					
					
					//if( TMath::Abs(tXa - tXb) < 0.5 || TMath::Abs(tYa - tYb) < 0.5 ) {
					if( theta < 30 ) {
						fVector_photons_pairs_direction.push_back(pair);
					}
				}
			}
		}
	}
	fhPhotons_nofPerEvent->Fill(nofPhotons);
}






void CbmAnaConversionRecoFull::CombinePhotons()
{
	Int_t nof = fVector_photons_pairs.size();
	cout << "CbmAnaConversionRecoFull: CombinePhotons, nof - " << nof << endl;
	if(nof >= 2) {
		for(int a=0; a<nof-1; a++) {
			for(int b=a+1; b<nof; b++) {
				Int_t electron11 = fVector_photons_pairs[a][0];
				Int_t electron12 = fVector_photons_pairs[a][1];
				Int_t electron21 = fVector_photons_pairs[b][0];
				Int_t electron22 = fVector_photons_pairs[b][1];
			
				Double_t invmass = Invmass_4particlesRECO(fElectrons_momenta[electron11], fElectrons_momenta[electron12], fElectrons_momenta[electron21], fElectrons_momenta[electron22]);
				fhPhotons_invmass->Fill(invmass);
				
				Double_t pt = Pt_4particlesRECO(fElectrons_momenta[electron11], fElectrons_momenta[electron12], fElectrons_momenta[electron21], fElectrons_momenta[electron22]);
				Double_t rap = Rap_4particlesRECO(fElectrons_momenta[electron11], fElectrons_momenta[electron12], fElectrons_momenta[electron21], fElectrons_momenta[electron22]);
				
				Double_t opening_angle = OpeningAngleBetweenPhotons2(fVector_photons_pairs[a], fVector_photons_pairs[b]);
				fhPhotons_angleBetween->Fill(opening_angle);
				
				
				CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(fElectrons_momenta[electron11], fElectrons_momenta[electron12], fElectrons_momenta[electron21], fElectrons_momenta[electron22]);
				
				
				TVector3 momentumE1;
				TVector3 momentumE2;
				fElectrons_track[a]->GetParamLast()->Momentum(momentumE1);
				fElectrons_track[b]->GetParamLast()->Momentum(momentumE2);
					
				Double_t energyE11 = TMath::Sqrt(fElectrons_momenta[electron11].Mag2() + M2E);
				TLorentzVector lorVecE11(fElectrons_momenta[electron11], energyE11);
				Double_t energyE12 = TMath::Sqrt(fElectrons_momenta[electron12].Mag2() + M2E);
				TLorentzVector lorVecE12(fElectrons_momenta[electron12], energyE12);
				Double_t energyE21 = TMath::Sqrt(fElectrons_momenta[electron21].Mag2() + M2E);
				TLorentzVector lorVecE21(fElectrons_momenta[electron21], energyE21);
				Double_t energyE22 = TMath::Sqrt(fElectrons_momenta[electron22].Mag2() + M2E);
				TLorentzVector lorVecE22(fElectrons_momenta[electron22], energyE22);
				TLorentzVector g1 = lorVecE11 + lorVecE12;
				TLorentzVector g2 = lorVecE21 + lorVecE22;
				TLorentzVector pi = lorVecE11 + lorVecE12 + lorVecE21 + lorVecE22;
					
				g1.Boost(-pi.BoostVector() );
				g2.Boost(-pi.BoostVector() );
					
				Double_t boostAngle = g1.Angle(g2.Vect());
				Double_t theta = 180.*boostAngle/TMath::Pi();
				
				fhPhotons_boostAngleTest->Fill(boostAngle);
					
				//if(opening_angle < 10) {
				if(true) {
					fhPhotons_boostAngle->Fill(theta);
				
					fhPhotons_invmass_cut->Fill(invmass);
					fhPhotons_invmass_vs_pt->Fill(invmass, pt);
					fhPhotons_rapidity_vs_pt->Fill(pt, rap);
					fhPhotons_invmass_vs_openingAngle->Fill(invmass, opening_angle);
					fhPhotons_openingAngle_vs_momentum->Fill(opening_angle, params1.fMomentumMag);
					
					fhMomentumFits_pi0reco->Fill(fElectrons_momentaChi[electron11]);
					fhMomentumFits_pi0reco->Fill(fElectrons_momentaChi[electron12]);
					fhMomentumFits_pi0reco->Fill(fElectrons_momentaChi[electron21]);
					fhMomentumFits_pi0reco->Fill(fElectrons_momentaChi[electron22]);
					
					fhPhotons_invmass_vs_chi->Fill(invmass, fElectrons_momentaChi[electron11]);
					fhPhotons_invmass_vs_chi->Fill(invmass, fElectrons_momentaChi[electron12]);
					fhPhotons_invmass_vs_chi->Fill(invmass, fElectrons_momentaChi[electron21]);
					fhPhotons_invmass_vs_chi->Fill(invmass, fElectrons_momentaChi[electron22]);
					
					if(pt <= 1) fhPhotons_invmass_cut_ptBin1->Fill(invmass);
					if(pt > 1 && pt <= 2) fhPhotons_invmass_cut_ptBin2->Fill(invmass);
					if(pt > 2 && pt <= 3) fhPhotons_invmass_cut_ptBin3->Fill(invmass);
					if(pt > 3 && pt <= 4) fhPhotons_invmass_cut_ptBin4->Fill(invmass);
					
					Double_t chicut = 1.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi1->Fill(invmass);
					}
					chicut = 3.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi3->Fill(invmass);
					}
					chicut = 5.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi5->Fill(invmass);
					}
					chicut = 10.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi10->Fill(invmass);
					}
					chicut = 25.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi25->Fill(invmass);
					}
					chicut = 40.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi40->Fill(invmass);
					}
					chicut = 65.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi65->Fill(invmass);
					}
					chicut = 80.0;
					if(fElectrons_momentaChi[electron11] < chicut && fElectrons_momentaChi[electron12] < chicut && fElectrons_momentaChi[electron21] < chicut && fElectrons_momentaChi[electron22] < chicut) {
						fhPhotons_invmass_cut_chi80->Fill(invmass);
					}
					
					
					
					// CROSSCHECK WITH MC-TRUE DATA!
					CbmMCTrack* mctrack11 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron11]);
					CbmMCTrack* mctrack12 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron12]);
					CbmMCTrack* mctrack21 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron21]);
					CbmMCTrack* mctrack22 = (CbmMCTrack*)fMcTracks->At(fElectrons_mctrackID[electron22]);
					
					Int_t motherId11 = mctrack11->GetMotherId();
					Int_t motherId12 = mctrack12->GetMotherId();
					Int_t motherId21 = mctrack21->GetMotherId();
					Int_t motherId22 = mctrack22->GetMotherId();
					
					TVector3 startvertex11;
					mctrack11->GetStartVertex(startvertex11);
					TVector3 startvertex12;
					mctrack12->GetStartVertex(startvertex12);
					TVector3 startvertex21;
					mctrack21->GetStartVertex(startvertex21);
					TVector3 startvertex22;
					mctrack22->GetStartVertex(startvertex22);
					
					fhPhotons_MC_startvertexZ->Fill(startvertex11.Z());
					fhPhotons_MC_startvertexZ->Fill(startvertex12.Z());
					fhPhotons_MC_startvertexZ->Fill(startvertex21.Z());
					fhPhotons_MC_startvertexZ->Fill(startvertex22.Z());
					
					if(motherId11 == motherId12 && motherId21 == motherId22) {
						fhPhotons_MC_invmass1->Fill(invmass);
						if(motherId11 != -1 && motherId21 != -1) {
							CbmMCTrack* mothermctrack11 = (CbmMCTrack*)fMcTracks->At(motherId11);
							CbmMCTrack* mothermctrack21 = (CbmMCTrack*)fMcTracks->At(motherId21);
							
							Int_t grandmotherId11 = mothermctrack11->GetMotherId();
							Int_t grandmotherId21 = mothermctrack21->GetMotherId();
							
							if(grandmotherId11 == grandmotherId21) {
								fhPhotons_MC_invmass2->Fill(invmass);
								if(grandmotherId11 == -1) continue;
								CbmMCTrack* pi0mctrack11 = (CbmMCTrack*)fMcTracks->At(grandmotherId11);
								if(pi0mctrack11->GetMotherId() == -1) {
									fhPhotons_MC_motherIdCut->Fill(invmass);
								}
							}
							if(grandmotherId11 != grandmotherId21) {
								fhPhotons_MC_invmass3->Fill(invmass);
							}
						}
					}
					
					if(motherId11 != motherId12 || motherId21 != motherId22) {
						fhPhotons_MC_invmass4->Fill(invmass);
					}
					
					
					
					if(motherId11 != -1 && motherId12 != -1 && motherId21 != -1 && motherId22 != -1) {
						CbmMCTrack* mothermctrack11 = (CbmMCTrack*)fMcTracks->At(motherId11);
						CbmMCTrack* mothermctrack12 = (CbmMCTrack*)fMcTracks->At(motherId12);
						CbmMCTrack* mothermctrack21 = (CbmMCTrack*)fMcTracks->At(motherId21);
						CbmMCTrack* mothermctrack22 = (CbmMCTrack*)fMcTracks->At(motherId22);
					
						fhPhotons_MC_motherpdg->Fill(mothermctrack11->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack12->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack21->GetPdgCode());
						fhPhotons_MC_motherpdg->Fill(mothermctrack22->GetPdgCode());
					}
					
					
					
					TLorentzVector lorVecE11c;
					TLorentzVector lorVecE12c;
					TLorentzVector lorVecE21c;
					TLorentzVector lorVecE22c;
					
					mctrack11->Get4Momentum(lorVecE11c);
					mctrack12->Get4Momentum(lorVecE12c);
					mctrack21->Get4Momentum(lorVecE21c);
					mctrack22->Get4Momentum(lorVecE22c);
					
					TLorentzVector g1c = lorVecE11c + lorVecE12c;
					TLorentzVector g2c = lorVecE21c + lorVecE22c;
					TLorentzVector pic = lorVecE11c + lorVecE12c + lorVecE21c + lorVecE22c;
					
					g1c.Boost(-pic.BoostVector() );
					g2c.Boost(-pic.BoostVector() );
					
					Double_t boostAnglec = g1c.Angle(g2c.Vect());
					Double_t thetac = 180.*boostAnglec/TMath::Pi();
					fhPhotons_boostAngleMC->Fill(thetac);
					
					
					
				}
			}
		}
	}
}


