/**
 * \file CbmAnaConversion2Manual.cxx
 *
 * 
 * 
 *
 * \author ??
 * \date ??
 **/

#include "CbmAnaConversion2Manual.h"


// included from CbmRoot
#include "FairRootManager.h"
#include "CbmStsTrack.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"
#include "TDatabasePDG.h"
#include "CbmGlobalTrack.h"
#include "CbmStsKFTrackFitter.h"
#include "CbmRichRing.h"
#include "CbmL1PFFitter.h"
#include "L1Field.h"

#include "CbmAnaConversion2BG.h"

#define M2E 2.6112004954086e-7

using namespace std;



CbmAnaConversion2Manual::CbmAnaConversion2Manual()
 : fMcTracks(nullptr),
   fStsTracks(nullptr),
   fStsTrackMatches(nullptr),
   fGlobalTracks(nullptr),
   fRichRings(nullptr),
   fRichRingMatches(nullptr),
   fPrimVertex(nullptr),
   fKFVertex(),
   VMomenta_minus(),
   VMCtracks_minus(),
   VMomenta_plus(),
   VMCtracks_plus(),
   VRings_plus(),
   VRings_minus(),
   frefmomenta(),
   EMT_Event(),
   EMT_pair_momenta(),
   EMT_OA(),
   EMT_InvMass(),
   EMT_NofRings(),
   fHistoList_man_all(),
   fHistoList_man_one(),
   fHistoList_man_two(),
   fHistoList_man_zero(),
   fHistoList_man_onetwo(),
   EMT_InvMass_all(nullptr),
   EMT_InvMass_oneInRICH(nullptr),
   EMT_InvMass_twoInRICH(nullptr),
   EMT_InvMass_zeroInRICH(nullptr),
   EMT_InvMass_onetwoInRICH(nullptr),
   fGammaInvMassReco_all(nullptr),
   fGammaOpeningAngleReco_all(nullptr),
   fPdg_all(nullptr),
   fPi0InvMassReco_all(nullptr),
   fP_reco_all(nullptr),
   fPt_reco_all(nullptr),
   fPi0_pt_vs_rap_all(nullptr),
   fGammaInvMassReco_one(nullptr),
   fGammaOpeningAngleReco_one(nullptr),
   fPdg_one(nullptr),
   fPi0InvMassReco_one(nullptr),
   fP_reco_one(nullptr),
   fPt_reco_one(nullptr),
   fPi0_pt_vs_rap_one(nullptr),
   fGammaInvMassReco_two(nullptr),
   fGammaOpeningAngleReco_two(nullptr),
   fPdg_two(nullptr),
   fPi0InvMassReco_two(nullptr),
   fP_reco_two(nullptr),
   fPt_reco_two(nullptr),
   fPi0_pt_vs_rap_two(nullptr),
   fGammaInvMassReco_zero(nullptr),
   fGammaOpeningAngleReco_zero(nullptr),
   fPdg_zero(nullptr),
   fPi0InvMassReco_zero(nullptr),
   fP_reco_zero(nullptr),
   fPt_reco_zero(nullptr),
   fPi0_pt_vs_rap_zero(nullptr),
   fGammaInvMassReco_onetwo(nullptr),
   fGammaOpeningAngleReco_onetwo(nullptr),
   fPdg_onetwo(nullptr),
   fPi0InvMassReco_onetwo(nullptr),
   fP_reco_onetwo(nullptr),
   fPt_reco_onetwo(nullptr),
   fPi0_pt_vs_rap_onetwo(nullptr),

  
   Gammas_all(),
   Gammas_one(),
   Gammas_two(),
   Gammas_zero(),
   Gammas_onetwo(),
   VstsIndex_minus(),
   VstsIndex_plus(),
   frefId(),
   Gammas_stsIndex_all(),
   Gammas_stsIndex_one(),
   Gammas_stsIndex_two(),
   Gammas_stsIndex_zero(),
   Gammas_stsIndex_onetwo(),
   fMCtracks(),
   Gammas_MC_all(),
   Gammas_MC_one(),
   Gammas_MC_two(),
   Gammas_MC_zero(),
   Gammas_MC_onetwo(),
   fHistoList_man(),
   fAnaBGInManual(nullptr),
   DoBGAnalysisInManual(0),
   fHistoList_bg_InM_all(),
   BG1_InM_all(nullptr),
   BG2_InM_all(nullptr),
   BG3_InM_all(nullptr),
   BG4_InM_all(nullptr),
   BG5_InM_all(nullptr),
   BG6_InM_all(nullptr),
   BG7_InM_all(nullptr),
   BG8_InM_all(nullptr),
   BG9_InM_all(nullptr),
   BG10_InM_all(nullptr),
   fHistoList_bg_InM_one(),
   BG1_InM_one(nullptr),
   BG2_InM_one(nullptr),
   BG3_InM_one(nullptr),
   BG4_InM_one(nullptr),
   BG5_InM_one(nullptr),
   BG6_InM_one(nullptr),
   BG7_InM_one(nullptr),
   BG8_InM_one(nullptr),
   BG9_InM_one(nullptr),
   BG10_InM_one(nullptr),
   fHistoList_bg_InM_two(),
   BG1_InM_two(nullptr),
   BG2_InM_two(nullptr),
   BG3_InM_two(nullptr),
   BG4_InM_two(nullptr),
   BG5_InM_two(nullptr),
   BG6_InM_two(nullptr),
   BG7_InM_two(nullptr),
   BG8_InM_two(nullptr),
   BG9_InM_two(nullptr),
   BG10_InM_two(nullptr),
   fHistoList_bg_InM_zero(),
   BG1_InM_zero(nullptr),
   BG2_InM_zero(nullptr),
   BG3_InM_zero(nullptr),
   BG4_InM_zero(nullptr),
   BG5_InM_zero(nullptr),
   BG6_InM_zero(nullptr),
   BG7_InM_zero(nullptr),
   BG8_InM_zero(nullptr),
   BG9_InM_zero(nullptr),
   BG10_InM_zero(nullptr),
   fHistoList_bg_InM_onetwo(),
   BG1_InM_onetwo(nullptr),
   BG2_InM_onetwo(nullptr),
   BG3_InM_onetwo(nullptr),
   BG4_InM_onetwo(nullptr),
   BG5_InM_onetwo(nullptr),
   BG6_InM_onetwo(nullptr),
   BG7_InM_onetwo(nullptr),
   BG8_InM_onetwo(nullptr),
   BG9_InM_onetwo(nullptr),
   BG10_InM_onetwo(nullptr),
   PdgCase8_InM_all(nullptr),
   PdgCase8_InM_one(nullptr),
   PdgCase8_InM_two(nullptr),
   PdgCase8_InM_zero(nullptr),
   PdgCase8_InM_onetwo(nullptr),
   testsameMIDcase8_InM_all(nullptr),
   testsameGRIDcase8_InM_all(nullptr),
   testsameMIDcase8_InM_one(nullptr),
   testsameGRIDcase8_InM_one(nullptr),
   testsameMIDcase8_InM_two(nullptr),
   testsameGRIDcase8_InM_two(nullptr),
   testsameMIDcase8_InM_zero(nullptr),
   testsameGRIDcase8_InM_zero(nullptr),
   testsameMIDcase8_InM_onetwo(nullptr),
   testsameGRIDcase8_InM_onetwo(nullptr),
   PdgCase8mothers_InM_all(nullptr),
   PdgCase8mothers_InM_one(nullptr),
   PdgCase8mothers_InM_two(nullptr),
   PdgCase8mothers_InM_zero(nullptr),
   PdgCase8mothers_InM_onetwo(nullptr),
   case8GRIDInvMassGamma_InM_all(nullptr),
   case8GRIDOAGamma_InM_all(nullptr),
   case8GRIDInvMassGamma_InM_one(nullptr),
   case8GRIDOAGamma_InM_one(nullptr),
   case8GRIDInvMassGamma_InM_two(nullptr),
   case8GRIDOAGamma_InM_two(nullptr),
   case8GRIDInvMassGamma_InM_zero(nullptr), 
   case8GRIDOAGamma_InM_zero(nullptr),
   case8GRIDInvMassGamma_InM_onetwo(nullptr),
   case8GRIDOAGamma_InM_onetwo(nullptr),
   Case1ZYPos_InM_all(nullptr),
   Case1ZYPos_InM_one(nullptr),
   Case1ZYPos_InM_two(nullptr),
   Case1ZYPos_InM_zero(nullptr),
   Case1ZYPos_InM_onetwo(nullptr)
{
}

CbmAnaConversion2Manual::~CbmAnaConversion2Manual()
{
}

void CbmAnaConversion2Manual::Init()
{
	FairRootManager* ioman = FairRootManager::Instance();
	if (nullptr == ioman) { Fatal("CbmAnaConversion2Manual::Init","RootManager not instantised!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( nullptr == fMcTracks) { Fatal("CbmAnaConversion2Manual::Init","No MCTrack array!"); }

	fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	if ( nullptr == fStsTracks) { Fatal("CbmAnaConversion2Manual::Init","No StsTrack array!"); }

	fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
	if (nullptr == fStsTrackMatches) { Fatal("CbmAnaConversion2Manual::Init","No StsTrackMatch array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (nullptr == fGlobalTracks) { Fatal("CbmAnaConversion2Manual::Init","No GlobalTrack array!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if (nullptr == fRichRings) { Fatal("CbmAnaConversion2Manual::Init","No RichRing array!"); }

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if (nullptr == fRichRingMatches) { Fatal("CbmAnaConversion2Manual::Init","No RichRingMatch array!"); }

	fPrimVertex = (CbmVertex*) ioman->GetObject("PrimaryVertex");
	if (nullptr == fPrimVertex) { Fatal("CbmAnaConversion2Manual::Init","No PrimaryVertex array!"); }

	cout << " CbmAnaConversion2Manual::Init() " << endl;
	InitHistos();

	DoBGAnalysisInManual = 1;

	if(DoBGAnalysisInManual) {
		fAnaBGInManual = new CbmAnaConversion2BG();
		fAnaBGInManual->Init();
	}

}


void CbmAnaConversion2Manual::InitHistos()
{
	// Mixing
	EMT_InvMass_all = new TH1D("EMT_InvMass_all","EMT_InvMass_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man.push_back(EMT_InvMass_all);

	EMT_InvMass_oneInRICH = new TH1D("EMT_InvMass_oneInRICH","EMT_InvMass_oneInRICH;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man.push_back(EMT_InvMass_oneInRICH);

	EMT_InvMass_twoInRICH = new TH1D("EMT_InvMass_twoInRICH","EMT_InvMass_twoInRICH;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man.push_back(EMT_InvMass_twoInRICH);

	EMT_InvMass_zeroInRICH = new TH1D("EMT_InvMass_zeroInRICH","EMT_InvMass_zeroInRICH;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man.push_back(EMT_InvMass_zeroInRICH);

	EMT_InvMass_onetwoInRICH = new TH1D("EMT_InvMass_onetwoInRICH","EMT_InvMass_onetwoInRICH;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man.push_back(EMT_InvMass_onetwoInRICH);

//0,1,2
	fGammaInvMassReco_all = new TH1D("fGammaInvMassReco_all","fGammaInvMassReco_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_all.push_back(fGammaInvMassReco_all);

	fGammaOpeningAngleReco_all = new TH1D("fGammaOpeningAngleReco_all","fGammaOpeningAngleReco_all;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_man_all.push_back(fGammaOpeningAngleReco_all);

	fPdg_all = new TH1D("fPdg_all","fPdg_all ;Id ;#", 4001, 0, 1000);
	fHistoList_man_all.push_back(fPdg_all);

	fPi0InvMassReco_all = new TH1D("fPi0InvMassReco_all","fPi0InvMassReco_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_all.push_back(fPi0InvMassReco_all);

	fP_reco_all = new TH1D("fP_reco_all","fP_reco_all ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_all.push_back(fP_reco_all);

	fPt_reco_all = new TH1D("fPt_reco_all","fPt_reco_all ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_all.push_back(fPt_reco_all);

	fPi0_pt_vs_rap_all = new TH2D("fPi0_pt_vs_rap_all", "fPi0_pt_vs_rap_all;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);  
	fHistoList_man_all.push_back(fPi0_pt_vs_rap_all);

//1
	fGammaInvMassReco_one = new TH1D("fGammaInvMassReco_one","fGammaInvMassReco_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_one.push_back(fGammaInvMassReco_one);

	fGammaOpeningAngleReco_one = new TH1D("fGammaOpeningAngleReco_one","fGammaOpeningAngleReco_one;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_man_one.push_back(fGammaOpeningAngleReco_one);

	fPdg_one = new TH1D("fPdg_one","fPdg_one ;Id ;#", 4001, 0, 1000);
	fHistoList_man_one.push_back(fPdg_one);

	fPi0InvMassReco_one = new TH1D("fPi0InvMassReco_one","fPi0InvMassReco_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_one.push_back(fPi0InvMassReco_one);

	fP_reco_one = new TH1D("fP_reco_one","fP_reco_one ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_one.push_back(fP_reco_one);

	fPt_reco_one = new TH1D("fPt_reco_one","fPt_reco_one ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_one.push_back(fPt_reco_one);

	fPi0_pt_vs_rap_one = new TH2D("fPi0_pt_vs_rap_one", "fPi0_pt_vs_rap_one;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);  
	fHistoList_man_one.push_back(fPi0_pt_vs_rap_one);

//2
	fGammaInvMassReco_two = new TH1D("fGammaInvMassReco_two","fGammaInvMassReco_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_two.push_back(fGammaInvMassReco_two);

	fGammaOpeningAngleReco_two = new TH1D("fGammaOpeningAngleReco_two","fGammaOpeningAngleReco_two;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_man_two.push_back(fGammaOpeningAngleReco_two);

	fPdg_two = new TH1D("fPdg_two","fPdg_two ;Id ;#", 4001, 0, 1000);
	fHistoList_man_two.push_back(fPdg_two);

	fPi0InvMassReco_two = new TH1D("fPi0InvMassReco_two","fPi0InvMassReco_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_two.push_back(fPi0InvMassReco_two);

	fP_reco_two = new TH1D("fP_reco_two","fP_reco_two ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_two.push_back(fP_reco_two);

	fPt_reco_two = new TH1D("fPt_reco_two","fPt_reco_two ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_two.push_back(fPt_reco_two);

	fPi0_pt_vs_rap_two = new TH2D("fPi0_pt_vs_rap_two", "fPi0_pt_vs_rap_two;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);  
	fHistoList_man_two.push_back(fPi0_pt_vs_rap_two);

//0
	fGammaInvMassReco_zero = new TH1D("fGammaInvMassReco_zero","fGammaInvMassReco_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_zero.push_back(fGammaInvMassReco_zero);

	fGammaOpeningAngleReco_zero = new TH1D("fGammaOpeningAngleReco_zero","fGammaOpeningAngleReco_zero;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_man_zero.push_back(fGammaOpeningAngleReco_zero);

	fPdg_zero = new TH1D("fPdg_zero","fPdg_zero ;Id ;#", 4001, 0, 1000);
	fHistoList_man_zero.push_back(fPdg_zero);

	fPi0InvMassReco_zero = new TH1D("fPi0InvMassReco_zero","fPi0InvMassReco_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_zero.push_back(fPi0InvMassReco_zero);

	fP_reco_zero = new TH1D("fP_reco_zero","fP_reco_zero ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_zero.push_back(fP_reco_zero);

	fPt_reco_zero = new TH1D("fPt_reco_zero","fPt_reco_zero ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_zero.push_back(fPt_reco_zero);

	fPi0_pt_vs_rap_zero = new TH2D("fPi0_pt_vs_rap_zero", "fPi0_pt_vs_rap_zero;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);  
	fHistoList_man_zero.push_back(fPi0_pt_vs_rap_zero);

//1,2
	fGammaInvMassReco_onetwo = new TH1D("fGammaInvMassReco_onetwo","fGammaInvMassReco_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_onetwo.push_back(fGammaInvMassReco_onetwo);

	fGammaOpeningAngleReco_onetwo = new TH1D("fGammaOpeningAngleReco_onetwo","fGammaOpeningAngleReco_onetwo;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_man_onetwo.push_back(fGammaOpeningAngleReco_onetwo);

	fPdg_onetwo = new TH1D("fPdg_onetwo","fPdg_onetwo ;Id ;#", 4001, 0, 1000);
	fHistoList_man_onetwo.push_back(fPdg_onetwo);

	fPi0InvMassReco_onetwo = new TH1D("fPi0InvMassReco_onetwo","fPi0InvMassReco_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_man_onetwo.push_back(fPi0InvMassReco_onetwo);

	fP_reco_onetwo = new TH1D("fP_reco_onetwo","fP_reco_onetwo ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_onetwo.push_back(fP_reco_onetwo);

	fPt_reco_onetwo = new TH1D("fPt_reco_onetwo","fPt_reco_onetwo ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_man_onetwo.push_back(fPt_reco_onetwo);

	fPi0_pt_vs_rap_onetwo = new TH2D("fPi0_pt_vs_rap_onetwo", "fPi0_pt_vs_rap_onetwo;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);  
	fHistoList_man_onetwo.push_back(fPi0_pt_vs_rap_onetwo);


	BG1_InM_all = new TH1D("BG1_InM_all","BG1_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG1_InM_all);
	BG2_InM_all = new TH1D("BG2_InM_all","BG2_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG2_InM_all);
	BG3_InM_all = new TH1D("BG3_InM_all","BG3_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG3_InM_all);
	BG4_InM_all = new TH1D("BG4_InM_all","BG4_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG4_InM_all);
	BG5_InM_all = new TH1D("BG5_InM_all","BG5_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG5_InM_all);
	BG6_InM_all = new TH1D("BG6_InM_all","BG6_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG6_InM_all);
	BG7_InM_all = new TH1D("BG7_InM_all","BG7_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG7_InM_all);
	BG8_InM_all = new TH1D("BG8_InM_all","BG8_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG8_InM_all);
	BG9_InM_all = new TH1D("BG9_InM_all","BG9_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG9_InM_all);
	BG10_InM_all = new TH1D("BG10_InM_all","BG10_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(BG10_InM_all);

	BG1_InM_one = new TH1D("BG1_InM_one","BG1_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG1_InM_one);
	BG2_InM_one = new TH1D("BG2_InM_one","BG2_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG2_InM_one);
	BG3_InM_one = new TH1D("BG3_InM_one","BG3_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG3_InM_one);
	BG4_InM_one = new TH1D("BG4_InM_one","BG4_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG4_InM_one);
	BG5_InM_one = new TH1D("BG5_InM_one","BG5_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG5_InM_one);
	BG6_InM_one = new TH1D("BG6_InM_one","BG6_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG6_InM_one);
	BG7_InM_one = new TH1D("BG7_InM_one","BG7_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG7_InM_one);
	BG8_InM_one = new TH1D("BG8_InM_one","BG8_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG8_InM_one);
	BG9_InM_one = new TH1D("BG9_InM_one","BG9_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG9_InM_one);
	BG10_InM_one = new TH1D("BG10_InM_one","BG10_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_one.push_back(BG10_InM_one);

	BG1_InM_two = new TH1D("BG1_InM_two","BG1_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG1_InM_two);
	BG2_InM_two = new TH1D("BG2_InM_two","BG2_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG2_InM_two);
	BG3_InM_two = new TH1D("BG3_InM_two","BG3_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG3_InM_two);
	BG4_InM_two = new TH1D("BG4_InM_two","BG4_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG4_InM_two);
	BG5_InM_two = new TH1D("BG5_InM_two","BG5_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG5_InM_two);
	BG6_InM_two = new TH1D("BG6_InM_two","BG6_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG6_InM_two);
	BG7_InM_two = new TH1D("BG7_InM_two","BG7_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG7_InM_two);
	BG8_InM_two = new TH1D("BG8_InM_two","BG8_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG8_InM_two);
	BG9_InM_two = new TH1D("BG9_InM_two","BG9_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG9_InM_two);
	BG10_InM_two = new TH1D("BG10_InM_two","BG10_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_two.push_back(BG10_InM_two);

	BG1_InM_zero = new TH1D("BG1_InM_zero","BG1_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG1_InM_zero);
	BG2_InM_zero = new TH1D("BG2_InM_zero","BG2_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG2_InM_zero);
	BG3_InM_zero = new TH1D("BG3_InM_zero","BG3_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG3_InM_zero);
	BG4_InM_zero = new TH1D("BG4_InM_zero","BG4_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG4_InM_zero);
	BG5_InM_zero = new TH1D("BG5_InM_zero","BG5_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG5_InM_zero);
	BG6_InM_zero = new TH1D("BG6_InM_zero","BG6_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG6_InM_zero);
	BG7_InM_zero = new TH1D("BG7_InM_zero","BG7_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG7_InM_zero);
	BG8_InM_zero = new TH1D("BG8_InM_zero","BG8_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG8_InM_zero);
	BG9_InM_zero = new TH1D("BG9_InM_zero","BG9_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG9_InM_zero);
	BG10_InM_zero = new TH1D("BG10_InM_zero","BG10_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_zero.push_back(BG10_InM_zero);

	BG1_InM_onetwo = new TH1D("BG1_InM_onetwo","BG1_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG1_InM_onetwo);
	BG2_InM_onetwo = new TH1D("BG2_InM_onetwo","BG2_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG2_InM_onetwo);
	BG3_InM_onetwo = new TH1D("BG3_InM_onetwo","BG3_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG3_InM_onetwo);
	BG4_InM_onetwo = new TH1D("BG4_InM_onetwo","BG4_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG4_InM_onetwo);
	BG5_InM_onetwo = new TH1D("BG5_InM_onetwo","BG5_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG5_InM_onetwo);
	BG6_InM_onetwo = new TH1D("BG6_InM_onetwo","BG6_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG6_InM_onetwo);
	BG7_InM_onetwo = new TH1D("BG7_InM_onetwo","BG7_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG7_InM_onetwo);
	BG8_InM_onetwo = new TH1D("BG8_InM_onetwo","BG8_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG8_InM_onetwo);
	BG9_InM_onetwo = new TH1D("BG9_InM_onetwo","BG9_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG9_InM_onetwo);
	BG10_InM_onetwo = new TH1D("BG10_InM_onetwo","BG10_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_onetwo.push_back(BG10_InM_onetwo);

	PdgCase8_InM_all = new TH1D("PdgCase8_InM_all","PdgCase8_InM_all ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_InM_all.push_back(PdgCase8_InM_all);
	PdgCase8_InM_one = new TH1D("PdgCase8_InM_one","PdgCase8_InM_one ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_InM_one.push_back(PdgCase8_InM_one);
	PdgCase8_InM_two = new TH1D("PdgCase8_InM_two","PdgCase8_InM_two ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_InM_two.push_back(PdgCase8_InM_two);
	PdgCase8_InM_zero = new TH1D("PdgCase8_InM_zero","PdgCase8_InM_zero ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_InM_zero.push_back(PdgCase8_InM_zero);
	PdgCase8_InM_onetwo = new TH1D("PdgCase8_InM_onetwo","PdgCase8_InM_onetwo ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_InM_onetwo.push_back(PdgCase8_InM_onetwo);

	PdgCase8mothers_InM_all = new TH1D("PdgCase8mothers_InM_all","PdgCase8mothers_InM_all ;Id ;#", 5000, -2500, 2500);
	PdgCase8mothers_InM_one = new TH1D("PdgCase8mothers_InM_one","PdgCase8mothers_InM_one ;Id ;#", 5000, -2500, 2500);
	PdgCase8mothers_InM_two = new TH1D("PdgCase8mothers_InM_two","PdgCase8mothers_InM_two ;Id ;#", 5000, -2500, 2500);
	PdgCase8mothers_InM_zero = new TH1D("PdgCase8mothers_InM_zero","PdgCase8mothers_InM_zero ;Id ;#", 5000, -2500, 2500);
	PdgCase8mothers_InM_onetwo = new TH1D("PdgCase8mothers_InM_onetwo","PdgCase8mothers_InM_onetwo ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_InM_all.push_back(PdgCase8mothers_InM_all);
	fHistoList_bg_InM_one.push_back(PdgCase8mothers_InM_one);
	fHistoList_bg_InM_two.push_back(PdgCase8mothers_InM_two);
	fHistoList_bg_InM_zero.push_back(PdgCase8mothers_InM_zero);
	fHistoList_bg_InM_onetwo.push_back(PdgCase8mothers_InM_onetwo);

	testsameMIDcase8_InM_all = new TH1D("testsameMIDcase8_InM_all","testsameMIDcase8_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_InM_all = new TH1D("testsameGRIDcase8_InM_all","testsameGRIDcase8_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	testsameMIDcase8_InM_one = new TH1D("testsameMIDcase8_InM_one","testsameMIDcase8_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_InM_one = new TH1D("testsameGRIDcase8_InM_one","testsameGRIDcase8_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	testsameMIDcase8_InM_two = new TH1D("testsameMIDcase8_InM_two","testsameMIDcase8_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_InM_two = new TH1D("testsameGRIDcase8_InM_two","testsameGRIDcase8_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	testsameMIDcase8_InM_zero = new TH1D("testsameMIDcase8_InM_zero","testsameMIDcase8_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_InM_zero = new TH1D("testsameGRIDcase8_InM_zero","testsameGRIDcase8_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	testsameMIDcase8_InM_onetwo = new TH1D("testsameMIDcase8_InM_onetwo","testsameMIDcase8_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_InM_onetwo = new TH1D("testsameGRIDcase8_InM_onetwo","testsameGRIDcase8_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_InM_all.push_back(testsameMIDcase8_InM_all);
	fHistoList_bg_InM_all.push_back(testsameGRIDcase8_InM_all);
	fHistoList_bg_InM_one.push_back(testsameMIDcase8_InM_one);
	fHistoList_bg_InM_one.push_back(testsameGRIDcase8_InM_one);
	fHistoList_bg_InM_two.push_back(testsameMIDcase8_InM_two);
	fHistoList_bg_InM_two.push_back(testsameGRIDcase8_InM_two);
	fHistoList_bg_InM_zero.push_back(testsameMIDcase8_InM_zero);
	fHistoList_bg_InM_zero.push_back(testsameGRIDcase8_InM_zero);
	fHistoList_bg_InM_onetwo.push_back(testsameMIDcase8_InM_onetwo);
	fHistoList_bg_InM_onetwo.push_back(testsameGRIDcase8_InM_onetwo);

	case8GRIDInvMassGamma_InM_all = new TH1D("case8GRIDInvMassGamma_InM_all","case8GRIDInvMassGamma_InM_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_InM_all = new TH1D("case8GRIDOAGamma_InM_all","case8GRIDOAGamma_InM_all;angle [deg];#", 1010, -0.1, 100.9); 
	case8GRIDInvMassGamma_InM_one = new TH1D("case8GRIDInvMassGamma_InM_one","case8GRIDInvMassGamma_InM_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_InM_one = new TH1D("case8GRIDOAGamma_InM_one","case8GRIDOAGamma_InM_one;angle [deg];#", 1010, -0.1, 100.9); 
	case8GRIDInvMassGamma_InM_two = new TH1D("case8GRIDInvMassGamma_InM_two","case8GRIDInvMassGamma_InM_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_InM_two = new TH1D("case8GRIDOAGamma_InM_two","case8GRIDOAGamma_InM_two;angle [deg];#", 1010, -0.1, 100.9); 
	case8GRIDInvMassGamma_InM_zero = new TH1D("case8GRIDInvMassGamma_InM_zero","case8GRIDInvMassGamma_InM_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_InM_zero = new TH1D("case8GRIDOAGamma_InM_zero","case8GRIDOAGamma_InM_zero;angle [deg];#", 1010, -0.1, 100.9); 
	case8GRIDInvMassGamma_InM_onetwo = new TH1D("case8GRIDInvMassGamma_InM_onetwo","case8GRIDInvMassGamma_InM_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_InM_onetwo = new TH1D("case8GRIDOAGamma_InM_onetwo","case8GRIDOAGamma_InM_onetwo;angle [deg];#", 1010, -0.1, 100.9); 

	fHistoList_bg_InM_all.push_back(case8GRIDInvMassGamma_InM_all);
	fHistoList_bg_InM_all.push_back(case8GRIDOAGamma_InM_all);
	fHistoList_bg_InM_one.push_back(case8GRIDInvMassGamma_InM_one);
	fHistoList_bg_InM_one.push_back(case8GRIDOAGamma_InM_one);
	fHistoList_bg_InM_two.push_back(case8GRIDInvMassGamma_InM_two);
	fHistoList_bg_InM_two.push_back(case8GRIDOAGamma_InM_two);
	fHistoList_bg_InM_zero.push_back(case8GRIDInvMassGamma_InM_zero);
	fHistoList_bg_InM_zero.push_back(case8GRIDOAGamma_InM_zero);
	fHistoList_bg_InM_onetwo.push_back(case8GRIDInvMassGamma_InM_onetwo);
	fHistoList_bg_InM_onetwo.push_back(case8GRIDOAGamma_InM_onetwo);

	Case1ZYPos_InM_all = new TH2D("Case1ZYPos_InM_all","Case1ZYPos_InM_all ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_InM_all.push_back(Case1ZYPos_InM_all);
	Case1ZYPos_InM_one = new TH2D("Case1ZYPos_InM_one","Case1ZYPos_InM_one ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_InM_one.push_back(Case1ZYPos_InM_one);
	Case1ZYPos_InM_two = new TH2D("Case1ZYPos_InM_two","Case1ZYPos_InM_two ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_InM_two.push_back(Case1ZYPos_InM_two);
	Case1ZYPos_InM_zero = new TH2D("Case1ZYPos_InM_zero","Case1ZYPos_InM_zero ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_InM_zero.push_back(Case1ZYPos_InM_zero);
	Case1ZYPos_InM_onetwo = new TH2D("Case1ZYPos_InM_onetwo","Case1ZYPos_InM_onetwo ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_InM_onetwo.push_back(Case1ZYPos_InM_onetwo);


	cout << " CbmAnaConversion2Manual::InitHistos() " << endl;

}


void CbmAnaConversion2Manual::Finish()
{
	gDirectory->mkdir("Manual");
	gDirectory->cd("Manual");

	gDirectory->mkdir("BG_InManual");
	gDirectory->cd("BG_InManual");
	gDirectory->mkdir("BG_InM_all");
	gDirectory->cd("BG_InM_all");
	for (UInt_t i = 0; i < fHistoList_bg_InM_all.size(); i++){
		fHistoList_bg_InM_all[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("BG_InM_one");
	gDirectory->cd("BG_InM_one");
	for (UInt_t i = 0; i < fHistoList_bg_InM_one.size(); i++){
		fHistoList_bg_InM_one[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("BG_InM_two");
	gDirectory->cd("BG_InM_two");
	for (UInt_t i = 0; i < fHistoList_bg_InM_two.size(); i++){
		fHistoList_bg_InM_two[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("BG_InM_zero");
	gDirectory->cd("BG_InM_zero");
	for (UInt_t i = 0; i < fHistoList_bg_InM_zero.size(); i++){
		fHistoList_bg_InM_zero[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("BG_InM_onetwo");
	gDirectory->cd("BG_InM_onetwo");
	for (UInt_t i = 0; i < fHistoList_bg_InM_onetwo.size(); i++){
		fHistoList_bg_InM_onetwo[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->cd("..");

	gDirectory->mkdir("all");
	gDirectory->cd("all");
	for (UInt_t i = 0; i < fHistoList_man_all.size(); i++){
		fHistoList_man_all[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("one");
	gDirectory->cd("one");
	for (UInt_t i = 0; i < fHistoList_man_one.size(); i++){
		fHistoList_man_one[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("two");
	gDirectory->cd("two");
	for (UInt_t i = 0; i < fHistoList_man_two.size(); i++){
		fHistoList_man_two[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("zero");
	gDirectory->cd("zero");
	for (UInt_t i = 0; i < fHistoList_man_zero.size(); i++){
		fHistoList_man_zero[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("onetwo");
	gDirectory->cd("onetwo");
	for (UInt_t i = 0; i < fHistoList_man_onetwo.size(); i++){
		fHistoList_man_onetwo[i]->Write();
	}
	gDirectory->cd("..");
	for (UInt_t i = 0; i < fHistoList_man.size(); i++){
		fHistoList_man[i]->Write();
	}
	gDirectory->cd("..");

}


void CbmAnaConversion2Manual::Exec(int fEventNum, double OpeningAngleCut, double GammaInvMassCut)
{
	int Event = fEventNum;
	double AngleCut = OpeningAngleCut;
	double InvMassCut = GammaInvMassCut;
	cout << "CbmAnaConversion2Manual::Exec()   Event: " << Event << endl;

	if (fPrimVertex != nullptr){
		fKFVertex = CbmKFVertex(*fPrimVertex);
	} else {
		Fatal("CbmAnaConversion::Exec","No PrimaryVertex array!");
	}

	VMCtracks_minus.clear();
	VMomenta_minus.clear();
	VRings_minus.clear();
	VstsIndex_minus.clear();
	VMCtracks_plus.clear();
	VMomenta_plus.clear();
	VRings_plus.clear();
	VstsIndex_plus.clear();
	Gammas_all.clear();
	Gammas_one.clear();
	Gammas_two.clear();
	Gammas_zero.clear();
	Gammas_onetwo.clear();
	Gammas_stsIndex_all.clear();
	Gammas_stsIndex_one.clear();
	Gammas_stsIndex_two.clear();
	Gammas_stsIndex_zero.clear();
	Gammas_stsIndex_onetwo.clear();
	Gammas_MC_all.clear();
	Gammas_MC_one.clear();
	Gammas_MC_two.clear();
	Gammas_MC_zero.clear();
	Gammas_MC_onetwo.clear();


	Int_t ngTracks = fGlobalTracks->GetEntriesFast();
	for (Int_t i = 0; i < ngTracks; i++) {
		CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(i);
		if(nullptr == gTrack) continue;
		int stsInd = gTrack->GetStsTrackIndex();
		int richInd = gTrack->GetRichRingIndex();

		if (stsInd < 0) continue;
		CbmStsTrack* stsTrack = (CbmStsTrack*) fStsTracks->At(stsInd);
		if (stsTrack == nullptr) continue;
		CbmTrackMatchNew* stsMatch  = (CbmTrackMatchNew*) fStsTrackMatches->At(stsInd);
		if (stsMatch == nullptr) continue;
		if(stsMatch->GetNofLinks() <= 0) continue;
		int stsMcTrackId = stsMatch->GetMatchedLink().GetIndex();
		if (stsMcTrackId < 0) continue;
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId);
		if (mcTrack1 == nullptr) continue;

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
		double charge = vtxTrack_electron->GetQp()*refittedMomentum_electron.Mag();
		float result_chi_electron = chiPrim_electron[0];
		float result_ndf_electron = stsTracks_electron[0].GetNDF();
		TVector3 refittedPos_electron;
		vtxTrack_electron->Position(refittedPos_electron);

		if (result_chi_electron > 3 || isnan(result_chi_electron) == 1 ) continue;
//		cout << "result_chi_electron = " << result_chi_electron << "; charge = " << charge << endl;
//		cout << "X = " << refittedPos_electron.X() << "; Y = " << refittedPos_electron.Y() << "; Z = " << refittedPos_electron.Z() << endl;

		int InRich = 0;
 
		if (richInd > 0){
			CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(richInd);
			if (richMatch != nullptr){
				int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
				if (richMcTrackId > 0){
					if(stsMcTrackId == richMcTrackId){ // check that global track was matched correctly for STS and RICH together
						CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
						if (mcTrack2 != nullptr){
							int pdgRICH = mcTrack2->GetPdgCode();
							if (TMath::Abs(pdgRICH) == 11 ) InRich++;
						}
					}
				}
			}
		}

		if ( charge < 0){
			VMCtracks_minus.push_back(mcTrack1);
			VMomenta_minus.push_back(refittedMomentum_electron);
			VRings_minus.push_back(InRich);
			VstsIndex_minus.push_back(stsInd);
		}
		if ( charge > 0){
			VMCtracks_plus.push_back(mcTrack1);
			VMomenta_plus.push_back(refittedMomentum_electron);
			VRings_plus.push_back(InRich);
			VstsIndex_plus.push_back(stsInd);
		}

	}


	FindGammas(AngleCut, InvMassCut, Event, VMomenta_minus, VMCtracks_minus, VMomenta_plus, VMCtracks_plus, VRings_minus, VRings_plus, VstsIndex_minus, VstsIndex_plus, fGammaInvMassReco_all, fGammaOpeningAngleReco_all, fPdg_all, fP_reco_all, fPt_reco_all, fGammaInvMassReco_one, fGammaOpeningAngleReco_one, fPdg_one, fP_reco_one, fPt_reco_one, fGammaInvMassReco_two, fGammaOpeningAngleReco_two, fPdg_two, fP_reco_two, fPt_reco_two, fGammaInvMassReco_zero, fGammaOpeningAngleReco_zero, fPdg_zero, fP_reco_zero, fPt_reco_zero, fGammaInvMassReco_onetwo, fGammaOpeningAngleReco_onetwo, fPdg_onetwo, fP_reco_onetwo, fPt_reco_onetwo);

	FindPi0(Gammas_all, Gammas_stsIndex_all, Gammas_MC_all, fPi0InvMassReco_all, fPi0_pt_vs_rap_all, BG1_InM_all, BG2_InM_all, BG3_InM_all, BG4_InM_all, BG5_InM_all, BG6_InM_all, BG7_InM_all, BG8_InM_all, BG9_InM_all, BG10_InM_all, PdgCase8_InM_all, testsameMIDcase8_InM_all, testsameGRIDcase8_InM_all, PdgCase8mothers_InM_all, case8GRIDInvMassGamma_InM_all, case8GRIDOAGamma_InM_all, Case1ZYPos_InM_all);

	FindPi0(Gammas_one, Gammas_stsIndex_one, Gammas_MC_one, fPi0InvMassReco_one, fPi0_pt_vs_rap_one, BG1_InM_one, BG2_InM_one, BG3_InM_one, BG4_InM_one, BG5_InM_one, BG6_InM_one, BG7_InM_one, BG8_InM_one, BG9_InM_one, BG10_InM_one, PdgCase8_InM_one, testsameMIDcase8_InM_one, testsameGRIDcase8_InM_one, PdgCase8mothers_InM_one, case8GRIDInvMassGamma_InM_one, case8GRIDOAGamma_InM_one, Case1ZYPos_InM_one);

	FindPi0(Gammas_two, Gammas_stsIndex_two, Gammas_MC_two, fPi0InvMassReco_two, fPi0_pt_vs_rap_two, BG1_InM_two, BG2_InM_two, BG3_InM_two, BG4_InM_two, BG5_InM_two, BG6_InM_two, BG7_InM_two, BG8_InM_two, BG9_InM_two, BG10_InM_two, PdgCase8_InM_two, testsameMIDcase8_InM_two, testsameGRIDcase8_InM_two, PdgCase8mothers_InM_two, case8GRIDInvMassGamma_InM_two, case8GRIDOAGamma_InM_two, Case1ZYPos_InM_two);

	FindPi0(Gammas_zero, Gammas_stsIndex_zero, Gammas_MC_zero, fPi0InvMassReco_zero, fPi0_pt_vs_rap_zero, BG1_InM_zero, BG2_InM_zero, BG3_InM_zero, BG4_InM_zero, BG5_InM_zero, BG6_InM_zero, BG7_InM_zero, BG8_InM_zero, BG9_InM_zero, BG10_InM_zero, PdgCase8_InM_zero, testsameMIDcase8_InM_zero, testsameGRIDcase8_InM_zero, PdgCase8mothers_InM_zero, case8GRIDInvMassGamma_InM_zero, case8GRIDOAGamma_InM_zero, Case1ZYPos_InM_zero);

	FindPi0(Gammas_onetwo, Gammas_stsIndex_onetwo, Gammas_MC_onetwo, fPi0InvMassReco_onetwo, fPi0_pt_vs_rap_onetwo, BG1_InM_onetwo, BG2_InM_onetwo, BG3_InM_onetwo, BG4_InM_onetwo, BG5_InM_onetwo, BG6_InM_onetwo, BG7_InM_onetwo, BG8_InM_onetwo, BG9_InM_onetwo, BG10_InM_onetwo, PdgCase8_InM_onetwo, testsameMIDcase8_InM_onetwo, testsameGRIDcase8_InM_onetwo, PdgCase8mothers_InM_onetwo, case8GRIDInvMassGamma_InM_onetwo, case8GRIDOAGamma_InM_onetwo, Case1ZYPos_InM_onetwo);


	if(Event%499 == 0) {
		Mixing();
		EMT_Event.clear();
		EMT_pair_momenta.clear();
		EMT_OA.clear();
		EMT_InvMass.clear();
		EMT_NofRings.clear();  
	}

}


void CbmAnaConversion2Manual::FindGammas(double AngleCut, double InvMassCut, int Event, vector<TVector3> Momenta_minus, vector<CbmMCTrack*> MCtracks_minus, vector<TVector3> Momenta_plus, vector<CbmMCTrack*> MCtracks_plus, std::vector<int> Rings_minus, std::vector<int> Rings_plus, std::vector<int> stsIndex_minus, std::vector<int> stsIndex_plus, TH1D* GammaInvMassReco_all, TH1D* GammaOpeningAngleReco_all, TH1D* Pdg_all, TH1D* P_reco_all, TH1D* Pt_reco_all, TH1D* GammaInvMassReco_one, TH1D* GammaOpeningAngleReco_one, TH1D* Pdg_one, TH1D* P_reco_one, TH1D* Pt_reco_one, TH1D* GammaInvMassReco_two, TH1D* GammaOpeningAngleReco_two, TH1D* Pdg_two, TH1D* P_reco_two, TH1D* Pt_reco_two, TH1D* GammaInvMassReco_zero, TH1D* GammaOpeningAngleReco_zero, TH1D* Pdg_zero, TH1D* P_reco_zero, TH1D* Pt_reco_zero, TH1D* GammaInvMassReco_onetwo, TH1D* GammaOpeningAngleReco_onetwo, TH1D* Pdg_onetwo, TH1D* P_reco_onetwo, TH1D* Pt_reco_onetwo)
{
	for (int i=0; i<Momenta_minus.size(); i++){
		for (int j=0; j<Momenta_plus.size(); j++){

			TVector3 part1 = Momenta_minus[i];
			TVector3 part2 = Momenta_plus[j];
			CbmMCTrack* part1MC = MCtracks_minus[i];
			CbmMCTrack* part2MC = MCtracks_plus[j];
			int richcheck = Rings_minus[i] + Rings_plus[j];

			Double_t InvmassReco = Invmass_2particlesRECO(part1, part2);
			Double_t OpeningAngle = CalculateOpeningAngleReco(part1, part2);

			// cuts
			if (TMath::Abs(OpeningAngle) > AngleCut) continue;
			if (TMath::Abs(InvmassReco) > InvMassCut) continue;

			frefmomenta.clear();
			frefmomenta.push_back(part1);
			frefmomenta.push_back(part2);
			frefId.clear();
			frefId.push_back(stsIndex_minus[i]);
			frefId.push_back(stsIndex_plus[j]);
			fMCtracks.clear();
			fMCtracks.push_back(part1MC);
			fMCtracks.push_back(part2MC);

			// for event mixing
			EMT_Event.push_back(Event);  //event number
			EMT_pair_momenta.push_back(frefmomenta);  //event number
			EMT_OA.push_back(OpeningAngle);  //event number
			EMT_InvMass.push_back(InvmassReco);  //event number
			EMT_NofRings.push_back(richcheck);  //event number

			// everything (RICH == 0, RICH == 1, RICH == 2) together
			if(richcheck == 0 || richcheck == 1 || richcheck == 2 ){
				Gammas_all.push_back(frefmomenta);
				Gammas_stsIndex_all.push_back(frefId);
				Gammas_MC_all.push_back(fMCtracks);
				GammaInvMassReco_all->Fill(InvmassReco);
				GammaOpeningAngleReco_all->Fill(OpeningAngle);
				Pdg_all->Fill(TMath::Abs(part1MC->GetPdgCode()));
				Pdg_all->Fill(TMath::Abs(part2MC->GetPdgCode()));
				P_reco_all->Fill(part1.Mag());
				P_reco_all->Fill(part2.Mag());
				Pt_reco_all->Fill(part1.Perp());
				Pt_reco_all->Fill(part2.Perp());
			}

			// only cases, when RICH == 1 
			if(richcheck == 1 ){
				Gammas_one.push_back(frefmomenta);
				Gammas_stsIndex_one.push_back(frefId);
				Gammas_MC_one.push_back(fMCtracks);
				GammaInvMassReco_one->Fill(InvmassReco);
				GammaOpeningAngleReco_one->Fill(OpeningAngle);
				Pdg_one->Fill(TMath::Abs(part1MC->GetPdgCode()));
				Pdg_one->Fill(TMath::Abs(part2MC->GetPdgCode()));
				P_reco_one->Fill(part1.Mag());
				P_reco_one->Fill(part2.Mag());
				Pt_reco_one->Fill(part1.Perp());
				Pt_reco_one->Fill(part2.Perp());
			}

			// only cases, when RICH == 2 
			if(richcheck == 2 ){
				Gammas_two.push_back(frefmomenta);
				Gammas_stsIndex_two.push_back(frefId);
				Gammas_MC_two.push_back(fMCtracks);
				GammaInvMassReco_two->Fill(InvmassReco);
				GammaOpeningAngleReco_two->Fill(OpeningAngle);
				Pdg_two->Fill(TMath::Abs(part1MC->GetPdgCode()));
				Pdg_two->Fill(TMath::Abs(part2MC->GetPdgCode()));
				P_reco_two->Fill(part1.Mag());
				P_reco_two->Fill(part2.Mag());
				Pt_reco_two->Fill(part1.Perp());
				Pt_reco_two->Fill(part2.Perp());
			}

			// only cases, when RICH == 0 
			if(richcheck == 0 ){
				Gammas_zero.push_back(frefmomenta);
				Gammas_stsIndex_zero.push_back(frefId);
				Gammas_MC_zero.push_back(fMCtracks);
				GammaInvMassReco_zero->Fill(InvmassReco);
				GammaOpeningAngleReco_zero->Fill(OpeningAngle);
				Pdg_zero->Fill(TMath::Abs(part1MC->GetPdgCode()));
				Pdg_zero->Fill(TMath::Abs(part2MC->GetPdgCode()));
				P_reco_zero->Fill(part1.Mag());
				P_reco_zero->Fill(part2.Mag());
				Pt_reco_zero->Fill(part1.Perp());
				Pt_reco_zero->Fill(part2.Perp());
			}

			// cases, when RICH == 1 or RICH == 2 together
			if(richcheck == 1 || richcheck == 2 ){
				Gammas_onetwo.push_back(frefmomenta);
				Gammas_stsIndex_onetwo.push_back(frefId);
				Gammas_MC_onetwo.push_back(fMCtracks);
				GammaInvMassReco_onetwo->Fill(InvmassReco);
				GammaOpeningAngleReco_onetwo->Fill(OpeningAngle);
				Pdg_onetwo->Fill(TMath::Abs(part1MC->GetPdgCode()));
				Pdg_onetwo->Fill(TMath::Abs(part2MC->GetPdgCode()));
				P_reco_onetwo->Fill(part1.Mag());
				P_reco_onetwo->Fill(part2.Mag());
				Pt_reco_onetwo->Fill(part1.Perp());
				Pt_reco_onetwo->Fill(part2.Perp());
			}
		}
	}
	
	cout << "number of gammas with 0-2 electron identified in RICH = " << Gammas_all.size() << endl;
	cout << "number of gammas with 1-2 electron identified in RICH = " << Gammas_one.size() << endl;
	cout << "number of gammas with  0  electron identified in RICH = " << Gammas_two.size() << endl;
	cout << "number of gammas with  1  electron identified in RICH = " << Gammas_zero.size() << endl;
	cout << "number of gammas with  2  electron identified in RICH = " << Gammas_onetwo.size() << endl;

}


void CbmAnaConversion2Manual::FindPi0(std::vector< std::vector<TVector3> > normalgammas, std::vector< std::vector<int> > Gammas_stsIndex, std::vector< std::vector<CbmMCTrack*> > MCtracks, TH1D* histoReco, TH2D* ptVSrap, TH1D* Case1, TH1D* Case2, TH1D* Case3, TH1D* Case4, TH1D* Case5, TH1D* Case6, TH1D* Case7, TH1D* Case8, TH1D* Case9, TH1D* Case10,  TH1D* PdgCase8, TH1D* testsameMIDcase8, TH1D* testsameGRIDcase8, TH1D* PdgCase8mothers, TH1D* case8GRIDInvMassGamma, TH1D* case8GRIDOAGamma, TH2D* Case1ZYPos)
{
	// combine all gamma in pi0 --> calculate inv mass for gammas and pi0 // not the case, when one particle used twice for different gammas
	if(normalgammas.size() > 1){
		for (int gamma1=0; gamma1<normalgammas.size()-1; gamma1++){
			for (int gamma2=gamma1+1; gamma2<normalgammas.size(); gamma2++){
				// 4 reconstructed particles from gammas
				TVector3 e11 = normalgammas[gamma1][0];
				TVector3 e12 = normalgammas[gamma1][1];
				TVector3 e21 = normalgammas[gamma2][0];
				TVector3 e22 = normalgammas[gamma2][1];

				// MC true data for this particles
				CbmMCTrack* mcTrack1 = MCtracks[gamma1][0];
				CbmMCTrack* mcTrack2 = MCtracks[gamma1][1];
				CbmMCTrack* mcTrack3 = MCtracks[gamma2][0];
				CbmMCTrack* mcTrack4 = MCtracks[gamma2][1];

				if(Gammas_stsIndex[gamma1][0] == Gammas_stsIndex[gamma2][0] || Gammas_stsIndex[gamma1][0] == Gammas_stsIndex[gamma2][1] || 
				  Gammas_stsIndex[gamma1][1] == Gammas_stsIndex[gamma2][0] || Gammas_stsIndex[gamma1][1] == Gammas_stsIndex[gamma2][1]) continue; // particles not used twice --> different

				CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(e11, e12, e21, e22);

				histoReco->Fill(params1.fMinv);


				if(DoBGAnalysisInManual) {
					fAnaBGInManual->Exec(mcTrack1, mcTrack2, mcTrack3, mcTrack4, e11, e12, e21, e22, params1.fMinv, Case1, Case2, Case3, Case4, Case5, Case6, Case7, Case8, Case9, Case10, PdgCase8, testsameMIDcase8, testsameGRIDcase8, PdgCase8mothers, case8GRIDInvMassGamma, case8GRIDOAGamma, Case1ZYPos);
				}


				if( TMath::Abs(mcTrack1->GetPdgCode()) != 11 || TMath::Abs(mcTrack2->GetPdgCode()) != 11 || TMath::Abs(mcTrack3->GetPdgCode()) != 11 || TMath::Abs(mcTrack4->GetPdgCode()) != 11) continue;
				if(mcTrack1->GetPdgCode() + mcTrack2->GetPdgCode() != 0) continue;
				if(mcTrack3->GetPdgCode() + mcTrack4->GetPdgCode() != 0) continue;
				int motherId1 = mcTrack1->GetMotherId();
				int motherId2 = mcTrack2->GetMotherId();
				int motherId3 = mcTrack3->GetMotherId();
				int motherId4 = mcTrack4->GetMotherId();
				if (motherId1 == -1 || motherId2 == -1 || motherId3 == -1 || motherId4 == -1) continue;
				if (motherId1 != motherId2 || motherId3 != motherId4) continue;
				CbmMCTrack* mother1 = (CbmMCTrack*) fMcTracks->At(motherId1);
				CbmMCTrack* mother2 = (CbmMCTrack*) fMcTracks->At(motherId2);
				CbmMCTrack* mother3 = (CbmMCTrack*) fMcTracks->At(motherId3);
				CbmMCTrack* mother4 = (CbmMCTrack*) fMcTracks->At(motherId4);
				if (nullptr == mother1 || nullptr == mother2 || nullptr == mother3 || nullptr == mother4) continue;
				int mcMotherPdg1 = mother1->GetPdgCode();
				int mcMotherPdg2 = mother2->GetPdgCode();
				int mcMotherPdg3 = mother3->GetPdgCode();
				int mcMotherPdg4 = mother4->GetPdgCode();
				int grandmotherId1 = mother1->GetMotherId();
				int grandmotherId2 = mother2->GetMotherId();
				int grandmotherId3 = mother3->GetMotherId();
				int grandmotherId4 = mother4->GetMotherId();

				if( mcMotherPdg1 == 22 && mcMotherPdg2 == 22 && mcMotherPdg3 == 111 && mcMotherPdg4 == 111){
					if (grandmotherId1 != grandmotherId2) continue;
					if (grandmotherId1 != motherId3) continue;
					CbmMCTrack* grandmother1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
					if (nullptr == grandmother1 ) continue;
					int mcGrandMotherPdg1 = grandmother1->GetPdgCode();
					if (mcGrandMotherPdg1 != 111 ) continue;
					//cout << "Dalitz case" << endl;
					//cout << "motherids: " << motherId1 << "/" << motherId2 << "/" << motherId3 << "/" << motherId4 << "\t " << endl;
					//cout << "grandmotherid: " << grandmotherId1 << "/" << grandmotherId2 << "/" << grandmotherId3 << "/" << grandmotherId4 << "    grandmotherpdg: " << mcGrandMotherPdg1 << endl;
					ptVSrap->Fill(params1.fPt, params1.fRapidity);
				}

				if( mcMotherPdg1 == 111 && mcMotherPdg2 == 111 && mcMotherPdg3 == 22 && mcMotherPdg4 == 22){
					if (grandmotherId3 != grandmotherId4) continue;
					if (grandmotherId3 != motherId1) continue;
					CbmMCTrack* grandmother3 = (CbmMCTrack*) fMcTracks->At(grandmotherId3);
					if (nullptr == grandmother3 ) continue;
					int mcGrandMotherPdg3 = grandmother3->GetPdgCode();
					if (mcGrandMotherPdg3 != 111 ) continue;
					//cout << "Dalitz case" << endl;
					//cout << "motherids: " << motherId1 << "/" << motherId2 << "/" << motherId3 << "/" << motherId4 << "\t " << endl;
					//cout << "grandmotherid: " << grandmotherId1 << "/" << grandmotherId2 << "/" << grandmotherId3 << "/" << grandmotherId4 << "    grandmotherpdg: " << mcGrandMotherPdg3 << endl;
					ptVSrap->Fill(params1.fPt, params1.fRapidity);
				}

				if( mcMotherPdg1 == 22 && mcMotherPdg2 == 22 && mcMotherPdg3 == 22 && mcMotherPdg4 == 22){
					if (grandmotherId1 != grandmotherId2 || grandmotherId3 != grandmotherId4 || grandmotherId1 != grandmotherId3) continue;
					CbmMCTrack* grandmother1 = (CbmMCTrack*) fMcTracks->At(grandmotherId1);
					if (nullptr == grandmother1) continue;
					int mcGrandMotherPdg1 = grandmother1->GetPdgCode();
					if (mcGrandMotherPdg1 != 111 ) continue;
					//cout << "Double conversion case " << endl;
					//cout << "motherids: " << motherId1 << "/" << motherId2 << "/" << motherId3 << "/" << motherId4 << "\t " << endl;
					//cout << "grandmotherid: " << grandmotherId1 << "/" << grandmotherId2 << "/" << grandmotherId3 << "/" << grandmotherId4 << "    grandmotherpdg: " << mcGrandMotherPdg1 << endl;
					ptVSrap->Fill(params1.fPt, params1.fRapidity);
				}

			}
		}
	}
}




Double_t CbmAnaConversion2Manual::Invmass_2particlesRECO(const TVector3 part1, const TVector3 part2)
{
	Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
	TLorentzVector lorVec1(part1, energy1);

	Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
	TLorentzVector lorVec2(part2, energy2);

	TLorentzVector sum;
	sum = lorVec1 + lorVec2;    

	return sum.Mag();
}

Double_t CbmAnaConversion2Manual::CalculateOpeningAngleReco(TVector3 electron1, TVector3 electron2)
{
	Double_t energyP = TMath::Sqrt(electron1.Mag2() + M2E);
	TLorentzVector lorVecP(electron1, energyP);

	Double_t energyM = TMath::Sqrt(electron2.Mag2() + M2E);
	TLorentzVector lorVecM(electron2, energyM);

	Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
	Double_t theta = 180.*anglePair/TMath::Pi();

	return theta;
}

CbmLmvmKinematicParams CbmAnaConversion2Manual::CalculateKinematicParams_4particles(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
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
	Double_t minv = sum.Mag();

	params.fMomentumMag = momPair.Mag();
	params.fPt = ptPair;
	params.fRapidity = yPair;
	params.fMinv = minv;
	return params;
}

Double_t CbmAnaConversion2Manual::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
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


void CbmAnaConversion2Manual::Mixing()
// combines photons from two different events, taken from each time 200 events
{
	Int_t nof = EMT_Event.size();
	cout << "Mixing in Manual - nof entries " << nof << endl;
	for(Int_t a = 0; a < nof-1; a++) {
		for(Int_t b = a+1; b < nof; b++) {
			if(EMT_Event[a] == EMT_Event[b]) continue;		// to make sure that the photons are from two different events
			if (TMath::Abs(EMT_OA[a]) > 1 || TMath::Abs(EMT_OA[b]) > 1) continue; // check cut on Opening Angle
			TVector3 e11 = EMT_pair_momenta[a][0];
			TVector3 e12 = EMT_pair_momenta[a][1];
			TVector3 e21 = EMT_pair_momenta[b][0];
			TVector3 e22 = EMT_pair_momenta[b][1];
			CbmLmvmKinematicParams params = CalculateKinematicParams_4particles(e11, e12, e21, e22);
			EMT_InvMass_all->Fill(params.fMinv);
			if (EMT_NofRings[a] == 1 && EMT_NofRings[b] == 1) EMT_InvMass_oneInRICH->Fill(params.fMinv);
			if (EMT_NofRings[a] == 2 && EMT_NofRings[b] == 2) EMT_InvMass_twoInRICH->Fill(params.fMinv);
			if (EMT_NofRings[a] == 0 && EMT_NofRings[b] == 0) EMT_InvMass_zeroInRICH->Fill(params.fMinv);
			if ( (EMT_NofRings[a] == 1 || EMT_NofRings[a] == 2) && (EMT_NofRings[b] == 1 || EMT_NofRings[b] == 2) ) EMT_InvMass_onetwoInRICH->Fill(params.fMinv);
		}
	}
}
