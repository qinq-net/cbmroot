/**
 * \file CbmAnaConversion2KF.h
 *
 * \author ??
 * \date ??
 **/
 
 
#ifndef CBM_ANA_CONVERSION2_KF
#define CBM_ANA_CONVERSION2_KF

// included from ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TClonesArray.h>
#include <TStopwatch.h>

// included from CbmRoot
#include "CbmStsTrack.h"
#include "CbmMCTrack.h"
#include "CbmVertex.h"
#include "CbmKFParticleFinder.h"
#include "CbmKFParticleFinderQA.h"
#include "CbmKFParticle.h"
#include "KFParticle.h"
#include "KFPartMatch.h"
#include "KFTopoPerformance.h"
#include "CbmLmvmKinematicParams.h"

#include "CbmAnaConversion2BG.h"




using namespace std;


class CbmAnaConversion2KF
{

public:
	CbmAnaConversion2KF();
	virtual ~CbmAnaConversion2KF();

	void Init();
	void InitHistos();
	void Finish();
	void Exec(int fEventNum, double OpeningAngleCut, double GammaInvMassCut);

	void SetKF(CbmKFParticleFinder* kfparticle, CbmKFParticleFinderQA* kfparticleQA);
	void test(int Event, double AngleCut, double InvMassCut);
	TVector3 FitToVertexSTS(CbmStsTrack* stsTrack, double x, double y, double z);
	Double_t Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2);
	Double_t Invmass_2particlesRECO(const TVector3 part1, const TVector3 part2);
	Double_t CalculateOpeningAngleReco(TVector3 electron1, TVector3 electron2);
	Double_t CalculateOpeningAngleMC(CbmMCTrack* mctrack1, CbmMCTrack* mctrack2);
	Double_t Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4);
	Double_t Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	void FindPi0(vector<KFParticle> normalgammas, vector<KFParticle> particlevector, TH1D* histoReco, TH1D* histoTrue, TH2D* ptVSrap, TH1D* less1cm, TH1D* zw1and20cm, TH1D* zw20and70cm, TH1D* Case1, TH1D* Case2, TH1D* Case3, TH1D* Case4, TH1D* Case5, TH1D* Case6, TH1D* Case7, TH1D* Case8, TH1D* Case9, TH1D* Case10, TH2D* histoRecoCheck, TH2D* charged, TH1D* PdgCase8, TH1D* more1cm, TH1D* testsameMIDcase8, TH1D* testsameGRIDcase8, TH1D* PdgCase8mothers, TH1D* case8GRIDInvMassGamma, TH1D* case8GRIDOAGamma, vector<TH1*> Pt_histos, TH2D* Case1ZYPos);
	void FindGammas(vector<KFParticle> allgammas, vector<KFParticle> particlevector, double AngleCut, int Event, double InvMassCut);
	void DoMC();
	CbmLmvmKinematicParams CalculateKinematicParams_4particles(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4);
	void MixedEvent(double AngleCut);
	void MixedEventMulti();


private:
	TClonesArray* fKFMcParticles;
	TClonesArray* fMcTracks;
	TClonesArray* fStsTracks;
	TClonesArray* fStsTrackMatches;
	TClonesArray* fGlobalTracks;
	TClonesArray* fRichRings;
	TClonesArray* fRichRingMatches;
	CbmKFParticleFinder* fKFparticle;
	CbmKFParticleFinderQA* fKFparticleFinderQA;
	const KFParticleTopoReconstructor* fKFtopo;
	KFTopoPerformance* fKFtopoPerf;
	vector<TH1*> fHistoList_kfparticle;	// list of all histograms containing results from KFParticle package
	vector<TVector3> frefmomentum;
	vector<CbmMCTrack*> fmcvector;

	// event mixing
	vector<TH1*> fHistoList_mixing;
	std::vector< std::vector<TVector3> >	EMT_pair_momenta;
	std::vector<int> EMT_Event;
	std::vector<Double_t> EMT_OA;
	std::vector<Double_t> EMT_InvMass;
	std::vector<int> EMT_NofRings;
	TH1D * gr_EMT_InvMass_all;
	TH1D * gr_EMT_InvMass_oneInRICH;
	TH1D * gr_EMT_InvMass_twoInRICH;
	TH1D * gr_EMT_InvMass_zeroInRICH;
	TH1D * gr_EMT_InvMass_onetwoInRICH;
	std::vector<Double_t> EMT_Z;
	TH1D * gr_EMT_InvMass_all_target;
	TH1D * gr_EMT_InvMass_oneInRICH_target;
	TH1D * gr_EMT_InvMass_twoInRICH_target;
	TH1D * gr_EMT_InvMass_zeroInRICH_target;
	TH1D * gr_EMT_InvMass_onetwoInRICH_target;
	TH1D * gr_EMT_InvMass_all_outside_target;
	TH1D * gr_EMT_InvMass_oneInRICH_outside_target;
	TH1D * gr_EMT_InvMass_twoInRICH_outside_target;
	TH1D * gr_EMT_InvMass_zeroInRICH_outside_target;
	TH1D * gr_EMT_InvMass_onetwoInRICH_outside_target;

	// multiplicity
	vector<TH1*> fHistoList_multiplicity;
	vector<TH1*> fHistoList_multiplicity_all;
	vector<TH1*> fHistoList_multiplicity_one;
	vector<TH1*> fHistoList_multiplicity_two;
	vector<TH1*> fHistoList_multiplicity_zero;
	vector<TH1*> fHistoList_multiplicity_onetwo;
	TH2D * MultiplicityInv_all;
	TH2D * MultiplicityInv_one;
	TH2D * MultiplicityInv_two;
	TH2D * MultiplicityInv_zero;
	TH2D * MultiplicityInv_onetwo;

	TH2D * Multiplicity_chargedParticles_Inv_all;
	TH2D * Multiplicity_chargedParticles_Inv_one;
	TH2D * Multiplicity_chargedParticles_Inv_two;
	TH2D * Multiplicity_chargedParticles_Inv_zero;
	TH2D * Multiplicity_chargedParticles_Inv_onetwo;

	std::vector<int> EMT_Event_multi_all;  
	std::vector< std::vector<TVector3> > EMT_pair_momenta_multi_all;  
	std::vector<Double_t> EMT_OA_multi_all; 
	std::vector<Double_t> EMT_InvMass_multi_all; 

	std::vector<int> EMT_Event_multi_one; 
	std::vector< std::vector<TVector3> > EMT_pair_momenta_multi_one;  
	std::vector<Double_t> EMT_OA_multi_one; 
	std::vector<Double_t> EMT_InvMass_multi_one; 

	std::vector<int> EMT_Event_multi_two;  
	std::vector< std::vector<TVector3> > EMT_pair_momenta_multi_two; 
	std::vector<Double_t> EMT_OA_multi_two; 
	std::vector<Double_t> EMT_InvMass_multi_two; 

	std::vector<int> EMT_Event_multi_zero; 
	std::vector< std::vector<TVector3> > EMT_pair_momenta_multi_zero;  
	std::vector<Double_t> EMT_OA_multi_zero; 
	std::vector<Double_t> EMT_InvMass_multi_zero; 

	std::vector<int> EMT_Event_multi_onetwo;  
	std::vector< std::vector<TVector3> > EMT_pair_momenta_multi_onetwo; 
	std::vector<Double_t> EMT_OA_multi_onetwo; 
	std::vector<Double_t> EMT_InvMass_multi_onetwo; 

	std::vector<int> EMT_multi_all; 
	std::vector<int> EMT_multi_one; 
	std::vector<int> EMT_multi_two;
	std::vector<int> EMT_multi_zero;
	std::vector<int> EMT_multi_onetwo;

	TH1D * gr_EMTMulti_InvMass_all_m1;
	TH1D * gr_EMTMulti_InvMass_all_m2;
	TH1D * gr_EMTMulti_InvMass_all_m3;
	TH1D * gr_EMTMulti_InvMass_all_m4;
	TH1D * gr_EMTMulti_InvMass_all_m5;
	TH1D * gr_EMTMulti_InvMass_all_m6;
	TH1D * gr_EMTMulti_InvMass_all_m7;
	TH1D * gr_EMTMulti_InvMass_all_m8;
	TH1D * gr_EMTMulti_InvMass_all_m9;
	TH1D * gr_EMTMulti_InvMass_all_m10;
	TH1D * gr_EMTMulti_InvMass_all_m11;
	TH1D * gr_EMTMulti_InvMass_all_m12;
	TH1D * gr_EMTMulti_InvMass_all_m13;
	TH1D * gr_EMTMulti_InvMass_all_m14;
	TH1D * gr_EMTMulti_InvMass_all_m15;
	TH1D * gr_EMTMulti_InvMass_all_m16;
	TH1D * gr_EMTMulti_InvMass_all_m17;
	TH1D * gr_EMTMulti_InvMass_all_m18;
	TH1D * gr_EMTMulti_InvMass_all_m19;
	TH1D * gr_EMTMulti_InvMass_all_m20;
 
	TH1D * gr_EMTMulti_InvMass_one_m1;
	TH1D * gr_EMTMulti_InvMass_one_m2;
	TH1D * gr_EMTMulti_InvMass_one_m3;
	TH1D * gr_EMTMulti_InvMass_one_m4;
	TH1D * gr_EMTMulti_InvMass_one_m5; 
	TH1D * gr_EMTMulti_InvMass_one_m6;
	TH1D * gr_EMTMulti_InvMass_one_m7;
	TH1D * gr_EMTMulti_InvMass_one_m8;
	TH1D * gr_EMTMulti_InvMass_one_m9;
	TH1D * gr_EMTMulti_InvMass_one_m10;
	TH1D * gr_EMTMulti_InvMass_one_m11;
	TH1D * gr_EMTMulti_InvMass_one_m12;
	TH1D * gr_EMTMulti_InvMass_one_m13;
	TH1D * gr_EMTMulti_InvMass_one_m14;
	TH1D * gr_EMTMulti_InvMass_one_m15; 
	TH1D * gr_EMTMulti_InvMass_one_m16;
	TH1D * gr_EMTMulti_InvMass_one_m17;
	TH1D * gr_EMTMulti_InvMass_one_m18;
	TH1D * gr_EMTMulti_InvMass_one_m19;
	TH1D * gr_EMTMulti_InvMass_one_m20;

	TH1D * gr_EMTMulti_InvMass_two_m1;
	TH1D * gr_EMTMulti_InvMass_two_m2;
	TH1D * gr_EMTMulti_InvMass_two_m3;
	TH1D * gr_EMTMulti_InvMass_two_m4;
	TH1D * gr_EMTMulti_InvMass_two_m5;
	TH1D * gr_EMTMulti_InvMass_two_m6;
	TH1D * gr_EMTMulti_InvMass_two_m7;
	TH1D * gr_EMTMulti_InvMass_two_m8;
	TH1D * gr_EMTMulti_InvMass_two_m9;
	TH1D * gr_EMTMulti_InvMass_two_m10;
	TH1D * gr_EMTMulti_InvMass_two_m11;
	TH1D * gr_EMTMulti_InvMass_two_m12;
	TH1D * gr_EMTMulti_InvMass_two_m13;
	TH1D * gr_EMTMulti_InvMass_two_m14;
	TH1D * gr_EMTMulti_InvMass_two_m15;
	TH1D * gr_EMTMulti_InvMass_two_m16;
	TH1D * gr_EMTMulti_InvMass_two_m17;
	TH1D * gr_EMTMulti_InvMass_two_m18;
	TH1D * gr_EMTMulti_InvMass_two_m19;
	TH1D * gr_EMTMulti_InvMass_two_m20;

	TH1D * gr_EMTMulti_InvMass_zero_m1;
	TH1D * gr_EMTMulti_InvMass_zero_m2;
	TH1D * gr_EMTMulti_InvMass_zero_m3;
	TH1D * gr_EMTMulti_InvMass_zero_m4;
	TH1D * gr_EMTMulti_InvMass_zero_m5;
	TH1D * gr_EMTMulti_InvMass_zero_m6;
	TH1D * gr_EMTMulti_InvMass_zero_m7;
	TH1D * gr_EMTMulti_InvMass_zero_m8;
	TH1D * gr_EMTMulti_InvMass_zero_m9;
	TH1D * gr_EMTMulti_InvMass_zero_m10;
	TH1D * gr_EMTMulti_InvMass_zero_m11;
	TH1D * gr_EMTMulti_InvMass_zero_m12;
	TH1D * gr_EMTMulti_InvMass_zero_m13;
	TH1D * gr_EMTMulti_InvMass_zero_m14;
	TH1D * gr_EMTMulti_InvMass_zero_m15;
	TH1D * gr_EMTMulti_InvMass_zero_m16;
	TH1D * gr_EMTMulti_InvMass_zero_m17;
	TH1D * gr_EMTMulti_InvMass_zero_m18;
	TH1D * gr_EMTMulti_InvMass_zero_m19;
	TH1D * gr_EMTMulti_InvMass_zero_m20;

	TH1D * gr_EMTMulti_InvMass_onetwo_m1;
	TH1D * gr_EMTMulti_InvMass_onetwo_m2;
	TH1D * gr_EMTMulti_InvMass_onetwo_m3;
	TH1D * gr_EMTMulti_InvMass_onetwo_m4;
	TH1D * gr_EMTMulti_InvMass_onetwo_m5;
	TH1D * gr_EMTMulti_InvMass_onetwo_m6;
	TH1D * gr_EMTMulti_InvMass_onetwo_m7;
	TH1D * gr_EMTMulti_InvMass_onetwo_m8;
	TH1D * gr_EMTMulti_InvMass_onetwo_m9;
	TH1D * gr_EMTMulti_InvMass_onetwo_m10;
	TH1D * gr_EMTMulti_InvMass_onetwo_m11;
	TH1D * gr_EMTMulti_InvMass_onetwo_m12;
	TH1D * gr_EMTMulti_InvMass_onetwo_m13;
	TH1D * gr_EMTMulti_InvMass_onetwo_m14;
	TH1D * gr_EMTMulti_InvMass_onetwo_m15;
	TH1D * gr_EMTMulti_InvMass_onetwo_m16;
	TH1D * gr_EMTMulti_InvMass_onetwo_m17;
	TH1D * gr_EMTMulti_InvMass_onetwo_m18;
	TH1D * gr_EMTMulti_InvMass_onetwo_m19;
	TH1D * gr_EMTMulti_InvMass_onetwo_m20;

//  0-2
	vector<KFParticle> GammasAll;
	vector<TH1*> fHistoList_all;
	TH1D * fGammaInvMassMC_All;
	TH1D * fGammaInvMassReco_All;
	TH1D * fGammaOpeningAngleMC_All;
	TH1D * fGammaOpeningAngleReco_All;
	TH1D * fPdg_All;
	TH1D * fPi0InvMassTrueKFV_All;
	TH1D * fPi0InvMassRecoKFV_All;
	TH1D * fdx_All;
	TH1D * fdy_All;
	TH1D * fdz_All;
	TH1D * fD_All;
	TH1D * fP_mc_All;
	TH1D * fP_reco_All;
	TH1D * fPt_mc_All;
	TH1D * fPt_reco_All;
	TH2D * fPi0_pt_vs_rap_All;
	vector<TH1*> fHistoList_all_target;
	vector<TH1*> fHistoList_all_mvd;
	vector<TH1*> fHistoList_all_sts;
	TH1D * fGammaInvMassReco_All_target;
	TH1D * fGammaOpeningAngleReco_All_target;
	TH1D * fPi0InvMassRecoKFV_All_target;
	TH1D * fGammaInvMassReco_All_mvd;
	TH1D * fGammaOpeningAngleReco_All_mvd;
	TH1D * fPi0InvMassRecoKFV_All_mvd;
	TH1D * fGammaInvMassReco_All_sts;
	TH1D * fGammaOpeningAngleReco_All_sts;
	TH1D * fPi0InvMassRecoKFV_All_sts;
	vector<TH1*> fHistoList_all_outside;
	TH1D * fPi0InvMassRecoKFV_All_In_Rich_outside_target;

//  1
	vector<KFParticle> GammasOneLeptonInRICH;
	vector<TH1*> fHistoList_oneInRich;
	TH1D * fGammaInvMassMC_One_In_Rich;
	TH1D * fGammaInvMassReco_One_In_Rich;
	TH1D * fGammaOpeningAngleMC_One_In_Rich;
	TH1D * fGammaOpeningAngleReco_One_In_Rich;
	TH1D * fPi0InvMassTrueKFV_One_In_Rich;
	TH1D * fPi0InvMassRecoKFV_One_In_Rich;
	TH1D * fPdg_One_In_Rich;
	TH1D * fdx_One_In_Rich;
	TH1D * fdy_One_In_Rich;
	TH1D * fdz_One_In_Rich;
	TH1D * fD_One_In_Rich;
	TH1D * fP_mc_One_In_Rich;
	TH1D * fP_reco_One_In_Rich;
	TH1D * fPt_mc_One_In_Rich;
	TH1D * fPt_reco_One_In_Rich;
	TH2D * fPi0_pt_vs_rap_One_In_Rich;
	vector<TH1*> fHistoList_oneInRich_target;
	vector<TH1*> fHistoList_oneInRich_mvd;
	vector<TH1*> fHistoList_oneInRich_sts;
	TH1D * fGammaInvMassReco_One_In_Rich_target;
	TH1D * fGammaOpeningAngleReco_One_In_Rich_target;
	TH1D * fPi0InvMassRecoKFV_One_In_Rich_target;
	TH1D * fGammaInvMassReco_One_In_Rich_mvd;
	TH1D * fGammaOpeningAngleReco_One_In_Rich_mvd;
	TH1D * fPi0InvMassRecoKFV_One_In_Rich_mvd;
	TH1D * fGammaInvMassReco_One_In_Rich_sts;
	TH1D * fGammaOpeningAngleReco_One_In_Rich_sts;
	TH1D * fPi0InvMassRecoKFV_One_In_Rich_sts;
	vector<TH1*> fHistoList_one_outside;
	TH1D * fPi0InvMassRecoKFV_One_In_Rich_outside_target;

//  2
	vector<KFParticle> GammasTwoLeptonInRICH;
	vector<TH1*> fHistoList_twoInRich;
	TH1D * fGammaInvMassMC_Two_In_Rich;
	TH1D * fGammaInvMassReco_Two_In_Rich;
	TH1D * fGammaOpeningAngleMC_Two_In_Rich;
	TH1D * fGammaOpeningAngleReco_Two_In_Rich;
	TH1D * fPi0InvMassTrueKFV_Two_In_Rich;
	TH1D * fPi0InvMassRecoKFV_Two_In_Rich;
	TH1D * fPdg_Two_In_Rich;
	TH1D * fdx_Two_In_Rich;
	TH1D * fdy_Two_In_Rich;
	TH1D * fdz_Two_In_Rich;
	TH1D * fD_Two_In_Rich;
	TH1D * fP_mc_Two_In_Rich;
	TH1D * fP_reco_Two_In_Rich;
	TH1D * fPt_mc_Two_In_Rich;
	TH1D * fPt_reco_Two_In_Rich;
	TH2D * fPi0_pt_vs_rap_Two_In_Rich;
	vector<TH1*> fHistoList_twoInRich_target;
	vector<TH1*> fHistoList_twoInRich_mvd;
	vector<TH1*> fHistoList_twoInRich_sts;
	TH1D * fGammaInvMassReco_Two_In_Rich_target;
	TH1D * fGammaOpeningAngleReco_Two_In_Rich_target;
	TH1D * fPi0InvMassRecoKFV_Two_In_Rich_target;
	TH1D * fGammaInvMassReco_Two_In_Rich_mvd;
	TH1D * fGammaOpeningAngleReco_Two_In_Rich_mvd;
	TH1D * fPi0InvMassRecoKFV_Two_In_Rich_mvd;
	TH1D * fGammaInvMassReco_Two_In_Rich_sts;
	TH1D * fGammaOpeningAngleReco_Two_In_Rich_sts;
	TH1D * fPi0InvMassRecoKFV_Two_In_Rich_sts;
	vector<TH1*> fHistoList_two_outside;
	TH1D * fPi0InvMassRecoKFV_Two_In_Rich_outside_target;

//  0
	vector<KFParticle> GammasZeroLeptonInRICH;
	vector<TH1*> fHistoList_zeroInRich;
	TH1D * fGammaInvMassMC_Zero_In_Rich;
	TH1D * fGammaInvMassReco_Zero_In_Rich;
	TH1D * fGammaOpeningAngleMC_Zero_In_Rich;
	TH1D * fGammaOpeningAngleReco_Zero_In_Rich;
	TH1D * fPi0InvMassTrueKFV_Zero_In_Rich;
	TH1D * fPi0InvMassRecoKFV_Zero_In_Rich;
	TH1D * fPdg_Zero_In_Rich;
	TH1D * fdx_Zero_In_Rich;
	TH1D * fdy_Zero_In_Rich;
	TH1D * fdz_Zero_In_Rich;
	TH1D * fD_Zero_In_Rich;
	TH1D * fP_mc_Zero_In_Rich;
	TH1D * fP_reco_Zero_In_Rich;
	TH1D * fPt_mc_Zero_In_Rich;
	TH1D * fPt_reco_Zero_In_Rich;
	TH2D * fPi0_pt_vs_rap_Zero_In_Rich;
	vector<TH1*> fHistoList_zeroInRich_target;
	vector<TH1*> fHistoList_zeroInRich_mvd;
	vector<TH1*> fHistoList_zeroInRich_sts;
	TH1D * fGammaInvMassReco_Zero_In_Rich_target;
	TH1D * fGammaOpeningAngleReco_Zero_In_Rich_target;
	TH1D * fPi0InvMassRecoKFV_Zero_In_Rich_target;
	TH1D * fGammaInvMassReco_Zero_In_Rich_mvd;
	TH1D * fGammaOpeningAngleReco_Zero_In_Rich_mvd;
	TH1D * fPi0InvMassRecoKFV_Zero_In_Rich_mvd;
	TH1D * fGammaInvMassReco_Zero_In_Rich_sts;
	TH1D * fGammaOpeningAngleReco_Zero_In_Rich_sts;
	TH1D * fPi0InvMassRecoKFV_Zero_In_Rich_sts;
	vector<TH1*> fHistoList_zero_outside;
	TH1D * fPi0InvMassRecoKFV_Zero_In_Rich_outside_target;

//  1-2
	vector<KFParticle> GammasOneTwoLeptonInRICH;
	vector<TH1*> fHistoList_onetwoInRich;
	TH1D * fGammaInvMassMC_OneTwo_In_Rich;
	TH1D * fGammaInvMassReco_OneTwo_In_Rich;
	TH1D * fGammaOpeningAngleMC_OneTwo_In_Rich;
	TH1D * fGammaOpeningAngleReco_OneTwo_In_Rich;
	TH1D * fPi0InvMassTrueKFV_OneTwo_In_Rich;
	TH1D * fPi0InvMassRecoKFV_OneTwo_In_Rich;
	TH1D * fPdg_OneTwo_In_Rich;
	TH1D * fdx_OneTwo_In_Rich;
	TH1D * fdy_OneTwo_In_Rich;
	TH1D * fdz_OneTwo_In_Rich;
	TH1D * fD_OneTwo_In_Rich;
	TH1D * fP_mc_OneTwo_In_Rich;
	TH1D * fP_reco_OneTwo_In_Rich;
	TH1D * fPt_mc_OneTwo_In_Rich;
	TH1D * fPt_reco_OneTwo_In_Rich;
	TH2D * fPi0_pt_vs_rap_OneTwo_In_Rich;
	vector<TH1*> fHistoList_onetwoInRich_target;
	vector<TH1*> fHistoList_onetwoInRich_mvd;
	vector<TH1*> fHistoList_onetwoInRich_sts;
	TH1D * fGammaInvMassReco_OneTwo_In_Rich_target;
	TH1D * fGammaOpeningAngleReco_OneTwo_In_Rich_target;
	TH1D * fPi0InvMassRecoKFV_OneTwo_In_Rich_target;
	TH1D * fGammaInvMassReco_OneTwo_In_Rich_mvd;
	TH1D * fGammaOpeningAngleReco_OneTwo_In_Rich_mvd;
	TH1D * fPi0InvMassRecoKFV_OneTwo_In_Rich_mvd;
	TH1D * fGammaInvMassReco_OneTwo_In_Rich_sts;
	TH1D * fGammaOpeningAngleReco_OneTwo_In_Rich_sts;
	TH1D * fPi0InvMassRecoKFV_OneTwo_In_Rich_sts;
	vector<TH1*> fHistoList_onetwo_outside;
	TH1D * fPi0InvMassRecoKFV_OneTwo_In_Rich_outside_target;


//  MC
	vector<TH1*> fHistoList_MC;
	TH2D * fMC_start_vertex;
	TH1D * fMC_Pt;
	TH1D * fMC_P;
	TH1D * fPdgCodesMC;

//  check cuts
	vector<TH1*> fHistoList_CheckForCuts;
	TH1D * CheckForCuts_OA_MC;
	TH1D * CheckForCuts_OA_Reco;
	TH1D * CheckForCuts_InvMass_MC;
	TH1D * CheckForCuts_InvMass_Reco;
	TH1D * CheckForCuts_OA_MC_from_one_pi0;
	TH1D * CheckForCuts_OA_Reco_from_one_pi0;
	TH1D * CheckForCuts_InvMass_MC_from_one_pi0;
	TH1D * CheckForCuts_InvMass_Reco_from_one_pi0;
	TH2D * CheckForCuts_z_vs_InvMass_MC_from_one_pi0;
	TH2D * CheckForCuts_z_vs_OA_MC_from_one_pi0;
	TH2D * CheckForCuts_z_vs_InvMass_Reco_from_one_pi0;
	TH2D * CheckForCuts_z_vs_OA_Reco_from_one_pi0;
	TH1D * CheckForCuts_InvMass_Reco_from_one_pi0_less20cm;
	TH1D * CheckForCuts_OA_Reco_from_one_pi0_less20cm;
	TH1D * CheckForCuts_InvMass_Reco_from_one_pi0_between20cm_40cm;
	TH1D * CheckForCuts_OA_Reco_from_one_pi0_between20cm_40cm;
	TH1D * CheckForCuts_InvMass_Reco_from_one_pi0_more40cm;
	TH1D * CheckForCuts_OA_Reco_from_one_pi0_more40cm;

	// bg
	CbmAnaConversion2BG *fAnaBG;
	int DoBGAnalysis;
	vector<TH1*> fHistoList_bg_all;
	TH1D * BG1_all;
	TH1D * BG2_all;
	TH1D * BG3_all;
	TH1D * BG4_all;
	TH1D * BG5_all;
	TH1D * BG6_all;
	TH1D * BG7_all;
	TH1D * BG8_all;
	TH1D * BG9_all;
	TH1D * BG10_all;
	vector<TH1*> fHistoList_bg_one;
	TH1D * BG1_one;
	TH1D * BG2_one;
	TH1D * BG3_one;
	TH1D * BG4_one;
	TH1D * BG5_one;
	TH1D * BG6_one;
	TH1D * BG7_one;
	TH1D * BG8_one;
	TH1D * BG9_one;
	TH1D * BG10_one;
	vector<TH1*> fHistoList_bg_two;
	TH1D * BG1_two;
	TH1D * BG2_two;
	TH1D * BG3_two;
	TH1D * BG4_two;
	TH1D * BG5_two;
	TH1D * BG6_two;
	TH1D * BG7_two;
	TH1D * BG8_two;
	TH1D * BG9_two;
	TH1D * BG10_two;
	vector<TH1*> fHistoList_bg_zero;
	TH1D * BG1_zero;
	TH1D * BG2_zero;
	TH1D * BG3_zero;
	TH1D * BG4_zero;
	TH1D * BG5_zero;
	TH1D * BG6_zero;
	TH1D * BG7_zero;
	TH1D * BG8_zero;
	TH1D * BG9_zero;
	TH1D * BG10_zero;
	vector<TH1*> fHistoList_bg_onetwo;
	TH1D * BG1_onetwo;
	TH1D * BG2_onetwo;
	TH1D * BG3_onetwo;
	TH1D * BG4_onetwo;
	TH1D * BG5_onetwo;
	TH1D * BG6_onetwo;
	TH1D * BG7_onetwo;
	TH1D * BG8_onetwo;
	TH1D * BG9_onetwo;
	TH1D * BG10_onetwo;

	TH1D * PdgCase8_all;
	TH1D * PdgCase8_one;
	TH1D * PdgCase8_two;
	TH1D * PdgCase8_zero;
	TH1D * PdgCase8_onetwo;
	TH1D * testsameMIDcase8_all; 
	TH1D * testsameGRIDcase8_all;
	TH1D * testsameMIDcase8_one; 
	TH1D * testsameGRIDcase8_one;
	TH1D * testsameMIDcase8_two; 
	TH1D * testsameGRIDcase8_two;
	TH1D * testsameMIDcase8_zero; 
	TH1D * testsameGRIDcase8_zero;
	TH1D * testsameMIDcase8_onetwo; 
	TH1D * testsameGRIDcase8_onetwo;

	TH1D * PdgCase8mothers_all;
	TH1D * PdgCase8mothers_one;
	TH1D * PdgCase8mothers_two;
	TH1D * PdgCase8mothers_zero;
	TH1D * PdgCase8mothers_onetwo;

	TH1D * case8GRIDInvMassGamma_all; 
	TH1D * case8GRIDOAGamma_all; 
	TH1D * case8GRIDInvMassGamma_one; 
	TH1D * case8GRIDOAGamma_one; 
	TH1D * case8GRIDInvMassGamma_two; 
	TH1D * case8GRIDOAGamma_two; 
	TH1D * case8GRIDInvMassGamma_zero; 
	TH1D * case8GRIDOAGamma_zero; 
	TH1D * case8GRIDInvMassGamma_onetwo; 
	TH1D * case8GRIDOAGamma_onetwo; 

	vector<TH1*> fHistoList_pt;
	vector<TH1*> fHistoList_pt_all;
	TH1D * Pt1_all;
	TH1D * Pt2_all;
	TH1D * Pt3_all;
	TH1D * Pt4_all;
	TH1D * Pt5_all;
	TH1D * Pt6_all;
	TH1D * Pt7_all;
	TH1D * Pt8_all;
	TH1D * Pt9_all;
	TH1D * Pt10_all;
	TH1D * Pt11_all;
	TH1D * Pt12_all;
	vector<TH1*> fHistoList_pt_one;
	TH1D * Pt1_one;
	TH1D * Pt2_one;
	TH1D * Pt3_one;
	TH1D * Pt4_one;
	TH1D * Pt5_one;
	TH1D * Pt6_one;
	TH1D * Pt7_one;
	TH1D * Pt8_one;
	TH1D * Pt9_one;
	TH1D * Pt10_one;
	TH1D * Pt11_one;
	TH1D * Pt12_one;
	vector<TH1*> fHistoList_pt_two;
	TH1D * Pt1_two;
	TH1D * Pt2_two;
	TH1D * Pt3_two;
	TH1D * Pt4_two;
	TH1D * Pt5_two;
	TH1D * Pt6_two;
	TH1D * Pt7_two;
	TH1D * Pt8_two;
	TH1D * Pt9_two;
	TH1D * Pt10_two;
	TH1D * Pt11_two;
	TH1D * Pt12_two;
	vector<TH1*> fHistoList_pt_zero;
	TH1D * Pt1_zero;
	TH1D * Pt2_zero;
	TH1D * Pt3_zero;
	TH1D * Pt4_zero;
	TH1D * Pt5_zero;
	TH1D * Pt6_zero;
	TH1D * Pt7_zero;
	TH1D * Pt8_zero;
	TH1D * Pt9_zero;
	TH1D * Pt10_zero;
	TH1D * Pt11_zero;
	TH1D * Pt12_zero;
	vector<TH1*> fHistoList_pt_onetwo;
	TH1D * Pt1_onetwo;
	TH1D * Pt2_onetwo;
	TH1D * Pt3_onetwo;
	TH1D * Pt4_onetwo;
	TH1D * Pt5_onetwo;
	TH1D * Pt6_onetwo;
	TH1D * Pt7_onetwo;
	TH1D * Pt8_onetwo;
	TH1D * Pt9_onetwo;
	TH1D * Pt10_onetwo;
	TH1D * Pt11_onetwo;
	TH1D * Pt12_onetwo;

	TH2D * Case1ZYPos_all;
	TH2D * Case1ZYPos_one;
	TH2D * Case1ZYPos_two;
	TH2D * Case1ZYPos_zero;
	TH2D * Case1ZYPos_onetwo;


	// timer
	TStopwatch timer;
	Double_t fTime;

	CbmAnaConversion2KF(const CbmAnaConversion2KF&);
	CbmAnaConversion2KF operator=(const CbmAnaConversion2KF&);

	ClassDef(CbmAnaConversion2KF,1)
};

#endif
