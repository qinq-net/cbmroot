/**
 * \file CbmAnaConversion2Manual.h
 *
 * \author ??
 * \date ??
 **/
 
 
#ifndef CBM_ANA_CONVERSION2_MANUAL
#define CBM_ANA_CONVERSION2_MANUAL

// included from ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TClonesArray.h>
#include <TStopwatch.h>

// included from CbmRoot
#include "CbmStsTrack.h"
#include "CbmMCTrack.h"
#include "CbmLmvmKinematicParams.h"
#include "CbmKFVertex.h"

#include "CbmAnaConversion2BG.h"




using namespace std;


class CbmAnaConversion2Manual
{

public:
	CbmAnaConversion2Manual();
	CbmAnaConversion2Manual(const CbmAnaConversion2Manual&) = delete;
	CbmAnaConversion2Manual operator=(const CbmAnaConversion2Manual&) = delete;

	virtual ~CbmAnaConversion2Manual();

	void Init();
	void InitHistos();
	void Finish();
	void Exec(int fEventNum, double OpeningAngleCut, double GammaInvMassCut);

	void FindGammas(double AngleCut, double InvMassCut, int Event, vector<TVector3> Momenta_minus, vector<CbmMCTrack*> MCtracks_minus, vector<TVector3> Momenta_plus, vector<CbmMCTrack*> MCtracks_plus, std::vector<int> VRings_minus, std::vector<int> VRings_plus, std::vector<int> stsIndex_minus, std::vector<int> stsIndex_plus, TH1D* GammaInvMassReco_all, TH1D* GammaOpeningAngleReco_all, TH1D* Pdg_all, TH1D* P_reco_all, TH1D* Pt_reco_all, TH1D* GammaInvMassReco_one, TH1D* GammaOpeningAngleReco_one, TH1D* Pdg_one, TH1D* P_reco_one, TH1D* Pt_reco_one, TH1D* GammaInvMassReco_two, TH1D* GammaOpeningAngleReco_two, TH1D* Pdg_two, TH1D* P_reco_two, TH1D* Pt_reco_two, TH1D* GammaInvMassReco_zero, TH1D* GammaOpeningAngleReco_zero, TH1D* Pdg_zero, TH1D* P_reco_zero, TH1D* Pt_reco_zero, TH1D* GammaInvMassReco_onetwo, TH1D* GammaOpeningAngleReco_onetwo, TH1D* Pdg_onetwo, TH1D* P_reco_onetwo, TH1D* Pt_reco_onetwo);
	Double_t Invmass_2particlesRECO(const TVector3 part1, const TVector3 part2);
	Double_t CalculateOpeningAngleReco(TVector3 electron1, TVector3 electron2);
	void FindPi0(std::vector< std::vector<TVector3> > normalgammas, std::vector< std::vector<int> > Gammas_stsIndex, std::vector< std::vector<CbmMCTrack*> > MCtracks, TH1D* histoReco, TH2D* ptVSrap, TH1D* Case1, TH1D* Case2, TH1D* Case3, TH1D* Case4, TH1D* Case5, TH1D* Case6, TH1D* Case7, TH1D* Case8, TH1D* Case9, TH1D* Case10,  TH1D* PdgCase8, TH1D* testsameMIDcase8, TH1D* testsameGRIDcase8, TH1D* PdgCase8mothers, TH1D* case8GRIDInvMassGamma, TH1D* case8GRIDOAGamma, TH2D* Case1ZYPos);
	CbmLmvmKinematicParams CalculateKinematicParams_4particles(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	Double_t Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	void Mixing();


private:
	TClonesArray* fMcTracks;
	TClonesArray* fStsTracks;
	TClonesArray* fStsTrackMatches;
	TClonesArray* fGlobalTracks;
	TClonesArray* fRichRings;
	TClonesArray* fRichRingMatches;

	CbmVertex *fPrimVertex;
	CbmKFVertex fKFVertex;

	vector<TVector3> VMomenta_minus;
	vector<CbmMCTrack*> VMCtracks_minus;
	vector<TVector3> VMomenta_plus;
	vector<CbmMCTrack*> VMCtracks_plus;
	std::vector<int> VRings_plus;
	std::vector<int> VRings_minus;
	vector<TVector3> frefmomenta;
	std::vector<int> VstsIndex_minus;
	std::vector<int> VstsIndex_plus;

	std::vector<int> EMT_Event;
	std::vector< std::vector<TVector3> >	EMT_pair_momenta;
	std::vector<Double_t> EMT_OA;
	std::vector<Double_t> EMT_InvMass;
	std::vector<int> EMT_NofRings;

	vector<TH1*> fHistoList_man_all;
	vector<TH1*> fHistoList_man_one;
	vector<TH1*> fHistoList_man_two;
	vector<TH1*> fHistoList_man_zero;
	vector<TH1*> fHistoList_man_onetwo;

//Mixing
	vector<TH1*> fHistoList_man;
	TH1D * EMT_InvMass_all;
	TH1D * EMT_InvMass_oneInRICH;
	TH1D * EMT_InvMass_twoInRICH;
	TH1D * EMT_InvMass_zeroInRICH;
	TH1D * EMT_InvMass_onetwoInRICH;


//0,1,2
	TH1D * fGammaInvMassReco_all;
	TH1D * fGammaOpeningAngleReco_all;
	TH1D * fPdg_all;
	TH1D * fPi0InvMassReco_all;
	TH1D * fP_reco_all;
	TH1D * fPt_reco_all;
	TH2D * fPi0_pt_vs_rap_all;

//1
	TH1D * fGammaInvMassReco_one;
	TH1D * fGammaOpeningAngleReco_one;
	TH1D * fPdg_one;
	TH1D * fPi0InvMassReco_one;
	TH1D * fP_reco_one;
	TH1D * fPt_reco_one;
	TH2D * fPi0_pt_vs_rap_one;

//2
	TH1D * fGammaInvMassReco_two;
	TH1D * fGammaOpeningAngleReco_two;
	TH1D * fPdg_two;
	TH1D * fPi0InvMassReco_two;
	TH1D * fP_reco_two;
	TH1D * fPt_reco_two;
	TH2D * fPi0_pt_vs_rap_two;

//0
	TH1D * fGammaInvMassReco_zero;
	TH1D * fGammaOpeningAngleReco_zero;
	TH1D * fPdg_zero;
	TH1D * fPi0InvMassReco_zero;
	TH1D * fP_reco_zero;
	TH1D * fPt_reco_zero;
	TH2D * fPi0_pt_vs_rap_zero;

//1,2
	TH1D * fGammaInvMassReco_onetwo;
	TH1D * fGammaOpeningAngleReco_onetwo;
	TH1D * fPdg_onetwo;
	TH1D * fPi0InvMassReco_onetwo;
	TH1D * fP_reco_onetwo;
	TH1D * fPt_reco_onetwo;
	TH2D * fPi0_pt_vs_rap_onetwo;

	std::vector< std::vector<TVector3> > Gammas_all;
	std::vector< std::vector<TVector3> > Gammas_one;
	std::vector< std::vector<TVector3> > Gammas_two;
	std::vector< std::vector<TVector3> > Gammas_zero;
	std::vector< std::vector<TVector3> > Gammas_onetwo;

	std::vector<int> frefId;
	std::vector< std::vector<int> > Gammas_stsIndex_all;
	std::vector< std::vector<int> > Gammas_stsIndex_one;
	std::vector< std::vector<int> > Gammas_stsIndex_two;
	std::vector< std::vector<int> > Gammas_stsIndex_zero;
	std::vector< std::vector<int> > Gammas_stsIndex_onetwo;

	std::vector<CbmMCTrack*> fMCtracks;
	std::vector< std::vector<CbmMCTrack*> > Gammas_MC_all;
	std::vector< std::vector<CbmMCTrack*> > Gammas_MC_one;
	std::vector< std::vector<CbmMCTrack*> > Gammas_MC_two;
	std::vector< std::vector<CbmMCTrack*> > Gammas_MC_zero;
	std::vector< std::vector<CbmMCTrack*> > Gammas_MC_onetwo;

	CbmAnaConversion2BG *fAnaBGInManual;
	int DoBGAnalysisInManual;

	vector<TH1*> fHistoList_bg_InM_all;
	TH1D * BG1_InM_all;
	TH1D * BG2_InM_all;
	TH1D * BG3_InM_all;
	TH1D * BG4_InM_all;
	TH1D * BG5_InM_all;
	TH1D * BG6_InM_all;
	TH1D * BG7_InM_all;
	TH1D * BG8_InM_all;
	TH1D * BG9_InM_all;
	TH1D * BG10_InM_all;
	vector<TH1*> fHistoList_bg_InM_one;
	TH1D * BG1_InM_one;
	TH1D * BG2_InM_one;
	TH1D * BG3_InM_one;
	TH1D * BG4_InM_one;
	TH1D * BG5_InM_one;
	TH1D * BG6_InM_one;
	TH1D * BG7_InM_one;
	TH1D * BG8_InM_one;
	TH1D * BG9_InM_one;
	TH1D * BG10_InM_one;
	vector<TH1*> fHistoList_bg_InM_two;
	TH1D * BG1_InM_two;
	TH1D * BG2_InM_two;
	TH1D * BG3_InM_two;
	TH1D * BG4_InM_two;
	TH1D * BG5_InM_two;
	TH1D * BG6_InM_two;
	TH1D * BG7_InM_two;
	TH1D * BG8_InM_two;
	TH1D * BG9_InM_two;
	TH1D * BG10_InM_two;
	vector<TH1*> fHistoList_bg_InM_zero;
	TH1D * BG1_InM_zero;
	TH1D * BG2_InM_zero;
	TH1D * BG3_InM_zero;
	TH1D * BG4_InM_zero;
	TH1D * BG5_InM_zero;
	TH1D * BG6_InM_zero;
	TH1D * BG7_InM_zero;
	TH1D * BG8_InM_zero;
	TH1D * BG9_InM_zero;
	TH1D * BG10_InM_zero;
	vector<TH1*> fHistoList_bg_InM_onetwo;
	TH1D * BG1_InM_onetwo;
	TH1D * BG2_InM_onetwo;
	TH1D * BG3_InM_onetwo;
	TH1D * BG4_InM_onetwo;
	TH1D * BG5_InM_onetwo;
	TH1D * BG6_InM_onetwo;
	TH1D * BG7_InM_onetwo;
	TH1D * BG8_InM_onetwo;
	TH1D * BG9_InM_onetwo;
	TH1D * BG10_InM_onetwo;

	TH1D * PdgCase8_InM_all;
	TH1D * PdgCase8_InM_one;
	TH1D * PdgCase8_InM_two;
	TH1D * PdgCase8_InM_zero;
	TH1D * PdgCase8_InM_onetwo;
	TH1D * testsameMIDcase8_InM_all; 
	TH1D * testsameGRIDcase8_InM_all;
	TH1D * testsameMIDcase8_InM_one; 
	TH1D * testsameGRIDcase8_InM_one;
	TH1D * testsameMIDcase8_InM_two; 
	TH1D * testsameGRIDcase8_InM_two;
	TH1D * testsameMIDcase8_InM_zero; 
	TH1D * testsameGRIDcase8_InM_zero;
	TH1D * testsameMIDcase8_InM_onetwo; 
	TH1D * testsameGRIDcase8_InM_onetwo;

	TH1D * PdgCase8mothers_InM_all;
	TH1D * PdgCase8mothers_InM_one;
	TH1D * PdgCase8mothers_InM_two;
	TH1D * PdgCase8mothers_InM_zero;
	TH1D * PdgCase8mothers_InM_onetwo;

	TH1D * case8GRIDInvMassGamma_InM_all; 
	TH1D * case8GRIDOAGamma_InM_all; 
	TH1D * case8GRIDInvMassGamma_InM_one; 
	TH1D * case8GRIDOAGamma_InM_one; 
	TH1D * case8GRIDInvMassGamma_InM_two; 
	TH1D * case8GRIDOAGamma_InM_two; 
	TH1D * case8GRIDInvMassGamma_InM_zero; 
	TH1D * case8GRIDOAGamma_InM_zero; 
	TH1D * case8GRIDInvMassGamma_InM_onetwo; 
	TH1D * case8GRIDOAGamma_InM_onetwo; 

	TH2D * Case1ZYPos_InM_all;
	TH2D * Case1ZYPos_InM_one;
	TH2D * Case1ZYPos_InM_two;
	TH2D * Case1ZYPos_InM_zero;
	TH2D * Case1ZYPos_InM_onetwo;


	ClassDef(CbmAnaConversion2Manual,1)
};

#endif
