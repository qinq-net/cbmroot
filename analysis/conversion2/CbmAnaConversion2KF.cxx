/**
 * \file CbmAnaConversion2KF.cxx
 *
 * This class only extract some results from the KFParticle package,
 * which are relevant for the conversion analysis.
 *
 * \author ??
 * \date ??
 *
 **/

#include "CbmAnaConversion2KF.h"


// included from CbmRoot
#include "FairRootManager.h"
#include "CbmKFParticleFinder.h"
#include "CbmKFParticleFinderQA.h"
#include "KFParticleTopoReconstructor.h"
#include "KFTopoPerformance.h"
#include "CbmStsTrack.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"
#include "TDatabasePDG.h"
#include "CbmGlobalTrack.h"
#include "CbmStsKFTrackFitter.h"
#include "CbmRichRing.h"

#include "KFMCParticle.h"
#include "KFPartMatch.h"
#include "KFParticleFinder.h"
#include "CbmKF.h"
#include "CbmKFParticle.h"

#include "CbmAnaConversion2BG.h"


#define M2E 2.6112004954086e-7

using namespace std;

CbmAnaConversion2KF::CbmAnaConversion2KF()
 : fKFMcParticles(nullptr),
   fMcTracks(nullptr),
   fStsTracks(nullptr),
   fStsTrackMatches(nullptr),
   fGlobalTracks(nullptr),
   fRichRings(nullptr),
   fRichRingMatches(nullptr),
   fKFparticle(nullptr),
   fKFparticleFinderQA(nullptr),
   fKFtopo(nullptr),
   fKFtopoPerf(nullptr),
   fHistoList_kfparticle(),
   frefmomentum(),
   fmcvector(),


// mixing
   fHistoList_mixing(),
   EMT_Event(),
   EMT_pair_momenta(),
   EMT_OA(),
   EMT_InvMass(),
   EMT_NofRings(),
   gr_EMT_InvMass_all(nullptr),
   gr_EMT_InvMass_oneInRICH(nullptr),
   gr_EMT_InvMass_twoInRICH(nullptr),
   gr_EMT_InvMass_zeroInRICH(nullptr),
   gr_EMT_InvMass_onetwoInRICH(nullptr),
   EMT_Z(),
   gr_EMT_InvMass_all_target(nullptr),
   gr_EMT_InvMass_oneInRICH_target(nullptr),
   gr_EMT_InvMass_twoInRICH_target(nullptr),
   gr_EMT_InvMass_zeroInRICH_target(nullptr),
   gr_EMT_InvMass_onetwoInRICH_target(nullptr),
   gr_EMT_InvMass_all_outside_target(nullptr),
   gr_EMT_InvMass_oneInRICH_outside_target(nullptr),
   gr_EMT_InvMass_twoInRICH_outside_target(nullptr),
   gr_EMT_InvMass_zeroInRICH_outside_target(nullptr),
   gr_EMT_InvMass_onetwoInRICH_outside_target(nullptr),

// multiplicity
   fHistoList_multiplicity(),
   fHistoList_multiplicity_all(),
   fHistoList_multiplicity_one(),
   fHistoList_multiplicity_two(),
   fHistoList_multiplicity_zero(),
   fHistoList_multiplicity_onetwo(),
   MultiplicityInv_all(nullptr),
   MultiplicityInv_one(nullptr),
   MultiplicityInv_two(nullptr),
   MultiplicityInv_zero(nullptr),
   MultiplicityInv_onetwo(nullptr),

   Multiplicity_chargedParticles_Inv_all(nullptr),
   Multiplicity_chargedParticles_Inv_one(nullptr),
   Multiplicity_chargedParticles_Inv_two(nullptr),
   Multiplicity_chargedParticles_Inv_zero(nullptr),
   Multiplicity_chargedParticles_Inv_onetwo(nullptr),

   EMT_Event_multi_all(),
   EMT_pair_momenta_multi_all(),
   EMT_OA_multi_all(),
   EMT_InvMass_multi_all(),
   EMT_Event_multi_one(),
   EMT_pair_momenta_multi_one(),
   EMT_OA_multi_one(),
   EMT_InvMass_multi_one(),
   EMT_Event_multi_two(),
   EMT_pair_momenta_multi_two(),
   EMT_OA_multi_two(),
   EMT_InvMass_multi_two(),
   EMT_Event_multi_zero(),
   EMT_pair_momenta_multi_zero(),
   EMT_OA_multi_zero(),
   EMT_InvMass_multi_zero(),
   EMT_Event_multi_onetwo(),
   EMT_pair_momenta_multi_onetwo(),
   EMT_OA_multi_onetwo(),
   EMT_InvMass_multi_onetwo(),

   EMT_multi_all(), 
   EMT_multi_one(), 
   EMT_multi_two(),
   EMT_multi_zero(),
   EMT_multi_onetwo(), 

	gr_EMTMulti_InvMass_all_m1(nullptr),
	gr_EMTMulti_InvMass_all_m2(nullptr),
	gr_EMTMulti_InvMass_all_m3(nullptr),
	gr_EMTMulti_InvMass_all_m4(nullptr), 
	gr_EMTMulti_InvMass_all_m5(nullptr), 
	gr_EMTMulti_InvMass_all_m6(nullptr), 
	gr_EMTMulti_InvMass_all_m7(nullptr), 
	gr_EMTMulti_InvMass_all_m8(nullptr), 
	gr_EMTMulti_InvMass_all_m9(nullptr), 
	gr_EMTMulti_InvMass_all_m10(nullptr),
	gr_EMTMulti_InvMass_all_m11(nullptr),
	gr_EMTMulti_InvMass_all_m12(nullptr),
	gr_EMTMulti_InvMass_all_m13(nullptr),
	gr_EMTMulti_InvMass_all_m14(nullptr), 
	gr_EMTMulti_InvMass_all_m15(nullptr), 
	gr_EMTMulti_InvMass_all_m16(nullptr), 
	gr_EMTMulti_InvMass_all_m17(nullptr), 
	gr_EMTMulti_InvMass_all_m18(nullptr), 
	gr_EMTMulti_InvMass_all_m19(nullptr), 
	gr_EMTMulti_InvMass_all_m20(nullptr),
 
	gr_EMTMulti_InvMass_one_m1(nullptr),
	gr_EMTMulti_InvMass_one_m2(nullptr),
	gr_EMTMulti_InvMass_one_m3(nullptr),
	gr_EMTMulti_InvMass_one_m4(nullptr),
	gr_EMTMulti_InvMass_one_m5(nullptr),
	gr_EMTMulti_InvMass_one_m6(nullptr),
	gr_EMTMulti_InvMass_one_m7(nullptr),
	gr_EMTMulti_InvMass_one_m8(nullptr),
	gr_EMTMulti_InvMass_one_m9(nullptr),
	gr_EMTMulti_InvMass_one_m10(nullptr),
	gr_EMTMulti_InvMass_one_m11(nullptr),
	gr_EMTMulti_InvMass_one_m12(nullptr),
	gr_EMTMulti_InvMass_one_m13(nullptr),
	gr_EMTMulti_InvMass_one_m14(nullptr),
	gr_EMTMulti_InvMass_one_m15(nullptr),
	gr_EMTMulti_InvMass_one_m16(nullptr),
	gr_EMTMulti_InvMass_one_m17(nullptr),
	gr_EMTMulti_InvMass_one_m18(nullptr),
	gr_EMTMulti_InvMass_one_m19(nullptr),
	gr_EMTMulti_InvMass_one_m20(nullptr),

	gr_EMTMulti_InvMass_two_m1(nullptr),
	gr_EMTMulti_InvMass_two_m2(nullptr),
	gr_EMTMulti_InvMass_two_m3(nullptr),
	gr_EMTMulti_InvMass_two_m4(nullptr),
	gr_EMTMulti_InvMass_two_m5(nullptr),
	gr_EMTMulti_InvMass_two_m6(nullptr),
	gr_EMTMulti_InvMass_two_m7(nullptr),
	gr_EMTMulti_InvMass_two_m8(nullptr),
	gr_EMTMulti_InvMass_two_m9(nullptr),
	gr_EMTMulti_InvMass_two_m10(nullptr),
	gr_EMTMulti_InvMass_two_m11(nullptr),
	gr_EMTMulti_InvMass_two_m12(nullptr),
	gr_EMTMulti_InvMass_two_m13(nullptr),
	gr_EMTMulti_InvMass_two_m14(nullptr),
	gr_EMTMulti_InvMass_two_m15(nullptr),
	gr_EMTMulti_InvMass_two_m16(nullptr),
	gr_EMTMulti_InvMass_two_m17(nullptr),
	gr_EMTMulti_InvMass_two_m18(nullptr),
	gr_EMTMulti_InvMass_two_m19(nullptr),
	gr_EMTMulti_InvMass_two_m20(nullptr),

	gr_EMTMulti_InvMass_zero_m1(nullptr),
	gr_EMTMulti_InvMass_zero_m2(nullptr),
	gr_EMTMulti_InvMass_zero_m3(nullptr),
	gr_EMTMulti_InvMass_zero_m4(nullptr),
	gr_EMTMulti_InvMass_zero_m5(nullptr),
	gr_EMTMulti_InvMass_zero_m6(nullptr),
	gr_EMTMulti_InvMass_zero_m7(nullptr),
	gr_EMTMulti_InvMass_zero_m8(nullptr),
	gr_EMTMulti_InvMass_zero_m9(nullptr),
	gr_EMTMulti_InvMass_zero_m10(nullptr),
	gr_EMTMulti_InvMass_zero_m11(nullptr),
	gr_EMTMulti_InvMass_zero_m12(nullptr),
	gr_EMTMulti_InvMass_zero_m13(nullptr),
	gr_EMTMulti_InvMass_zero_m14(nullptr),
	gr_EMTMulti_InvMass_zero_m15(nullptr),
	gr_EMTMulti_InvMass_zero_m16(nullptr),
	gr_EMTMulti_InvMass_zero_m17(nullptr),
	gr_EMTMulti_InvMass_zero_m18(nullptr),
	gr_EMTMulti_InvMass_zero_m19(nullptr),
	gr_EMTMulti_InvMass_zero_m20(nullptr),

	gr_EMTMulti_InvMass_onetwo_m1(nullptr),
	gr_EMTMulti_InvMass_onetwo_m2(nullptr),
	gr_EMTMulti_InvMass_onetwo_m3(nullptr),
	gr_EMTMulti_InvMass_onetwo_m4(nullptr),
	gr_EMTMulti_InvMass_onetwo_m5(nullptr),
	gr_EMTMulti_InvMass_onetwo_m6(nullptr),
	gr_EMTMulti_InvMass_onetwo_m7(nullptr),
	gr_EMTMulti_InvMass_onetwo_m8(nullptr),
	gr_EMTMulti_InvMass_onetwo_m9(nullptr),
	gr_EMTMulti_InvMass_onetwo_m10(nullptr),
	gr_EMTMulti_InvMass_onetwo_m11(nullptr),
	gr_EMTMulti_InvMass_onetwo_m12(nullptr),
	gr_EMTMulti_InvMass_onetwo_m13(nullptr),
	gr_EMTMulti_InvMass_onetwo_m14(nullptr),
	gr_EMTMulti_InvMass_onetwo_m15(nullptr),
	gr_EMTMulti_InvMass_onetwo_m16(nullptr),
	gr_EMTMulti_InvMass_onetwo_m17(nullptr),
	gr_EMTMulti_InvMass_onetwo_m18(nullptr),
	gr_EMTMulti_InvMass_onetwo_m19(nullptr),
	gr_EMTMulti_InvMass_onetwo_m20(nullptr),


// 0-2
   GammasAll(),
   fHistoList_all(),
   fGammaInvMassMC_All(nullptr),
   fGammaInvMassReco_All(nullptr),
   fGammaOpeningAngleMC_All(nullptr),
   fGammaOpeningAngleReco_All(nullptr),
   fPdg_All(nullptr),
   fPi0InvMassTrueKFV_All(nullptr),
   fPi0InvMassRecoKFV_All(nullptr),
   fdx_All(nullptr),
   fdy_All(nullptr),
   fdz_All(nullptr),
   fD_All(nullptr),
   fP_mc_All(nullptr),
   fP_reco_All(nullptr),
   fPt_mc_All(nullptr),
   fPt_reco_All(nullptr),
   fPi0_pt_vs_rap_All(nullptr),
   fHistoList_all_target(),
   fHistoList_all_mvd(),
   fHistoList_all_sts(),
   fGammaInvMassReco_All_target(nullptr),
   fGammaOpeningAngleReco_All_target(nullptr),
   fPi0InvMassRecoKFV_All_target(nullptr),
   fGammaInvMassReco_All_mvd(nullptr),
   fGammaOpeningAngleReco_All_mvd(nullptr),
   fPi0InvMassRecoKFV_All_mvd(nullptr),
   fGammaInvMassReco_All_sts(nullptr),
   fGammaOpeningAngleReco_All_sts(nullptr),
   fPi0InvMassRecoKFV_All_sts(nullptr),
   fHistoList_all_outside(),
   fPi0InvMassRecoKFV_All_In_Rich_outside_target(nullptr),
// 1
   GammasOneLeptonInRICH(),
   fHistoList_oneInRich(),
   fGammaInvMassMC_One_In_Rich(nullptr),
   fGammaInvMassReco_One_In_Rich(nullptr),
   fGammaOpeningAngleMC_One_In_Rich(nullptr),
   fGammaOpeningAngleReco_One_In_Rich(nullptr),
   fPi0InvMassTrueKFV_One_In_Rich(nullptr),
   fPi0InvMassRecoKFV_One_In_Rich(nullptr),
   fPdg_One_In_Rich(nullptr),
   fdx_One_In_Rich(nullptr),
   fdy_One_In_Rich(nullptr),
   fdz_One_In_Rich(nullptr),
   fD_One_In_Rich(nullptr),
   fP_mc_One_In_Rich(nullptr),
   fP_reco_One_In_Rich(nullptr),
   fPt_mc_One_In_Rich(nullptr),
   fPt_reco_One_In_Rich(nullptr),
   fPi0_pt_vs_rap_One_In_Rich(nullptr),
   fHistoList_oneInRich_target(),
   fHistoList_oneInRich_mvd(),
   fHistoList_oneInRich_sts(),
   fGammaInvMassReco_One_In_Rich_target(nullptr),
   fGammaOpeningAngleReco_One_In_Rich_target(nullptr),
   fPi0InvMassRecoKFV_One_In_Rich_target(nullptr),
   fGammaInvMassReco_One_In_Rich_mvd(nullptr),
   fGammaOpeningAngleReco_One_In_Rich_mvd(nullptr),
   fPi0InvMassRecoKFV_One_In_Rich_mvd(nullptr),
   fGammaInvMassReco_One_In_Rich_sts(nullptr),
   fGammaOpeningAngleReco_One_In_Rich_sts(nullptr),
   fPi0InvMassRecoKFV_One_In_Rich_sts(nullptr),
   fHistoList_one_outside(),
   fPi0InvMassRecoKFV_One_In_Rich_outside_target(nullptr),
// 2
   GammasTwoLeptonInRICH(),
   fHistoList_twoInRich(),
   fGammaInvMassMC_Two_In_Rich(nullptr),
   fGammaInvMassReco_Two_In_Rich(nullptr),
   fGammaOpeningAngleMC_Two_In_Rich(nullptr),
   fGammaOpeningAngleReco_Two_In_Rich(nullptr),
   fPi0InvMassTrueKFV_Two_In_Rich(nullptr),
   fPi0InvMassRecoKFV_Two_In_Rich(nullptr),
   fPdg_Two_In_Rich(nullptr),
   fdx_Two_In_Rich(nullptr),
   fdy_Two_In_Rich(nullptr),
   fdz_Two_In_Rich(nullptr),
   fD_Two_In_Rich(nullptr),
   fP_mc_Two_In_Rich(nullptr),
   fP_reco_Two_In_Rich(nullptr),
   fPt_mc_Two_In_Rich(nullptr),
   fPt_reco_Two_In_Rich(nullptr),
   fPi0_pt_vs_rap_Two_In_Rich(nullptr),
   fHistoList_twoInRich_target(),
   fHistoList_twoInRich_mvd(),
   fHistoList_twoInRich_sts(),
   fGammaInvMassReco_Two_In_Rich_target(nullptr),
   fGammaOpeningAngleReco_Two_In_Rich_target(nullptr),
   fPi0InvMassRecoKFV_Two_In_Rich_target(nullptr),
   fGammaInvMassReco_Two_In_Rich_mvd(nullptr),
   fGammaOpeningAngleReco_Two_In_Rich_mvd(nullptr),
   fPi0InvMassRecoKFV_Two_In_Rich_mvd(nullptr),
   fGammaInvMassReco_Two_In_Rich_sts(nullptr),
   fGammaOpeningAngleReco_Two_In_Rich_sts(nullptr),
   fPi0InvMassRecoKFV_Two_In_Rich_sts(nullptr),
   fHistoList_two_outside(),
   fPi0InvMassRecoKFV_Two_In_Rich_outside_target(nullptr),
// 0
   GammasZeroLeptonInRICH(),
   fHistoList_zeroInRich(),
   fGammaInvMassMC_Zero_In_Rich(nullptr),
   fGammaInvMassReco_Zero_In_Rich(nullptr),
   fGammaOpeningAngleMC_Zero_In_Rich(nullptr),
   fGammaOpeningAngleReco_Zero_In_Rich(nullptr),
   fPi0InvMassTrueKFV_Zero_In_Rich(nullptr),
   fPi0InvMassRecoKFV_Zero_In_Rich(nullptr),
   fPdg_Zero_In_Rich(nullptr),
   fdx_Zero_In_Rich(nullptr),
   fdy_Zero_In_Rich(nullptr),
   fdz_Zero_In_Rich(nullptr),
   fD_Zero_In_Rich(nullptr),
   fP_mc_Zero_In_Rich(nullptr),
   fP_reco_Zero_In_Rich(nullptr),
   fPt_mc_Zero_In_Rich(nullptr),
   fPt_reco_Zero_In_Rich(nullptr),
   fPi0_pt_vs_rap_Zero_In_Rich(nullptr),
   fHistoList_zeroInRich_target(),
   fHistoList_zeroInRich_mvd(),
   fHistoList_zeroInRich_sts(),
   fGammaInvMassReco_Zero_In_Rich_target(nullptr),
   fGammaOpeningAngleReco_Zero_In_Rich_target(nullptr),
   fPi0InvMassRecoKFV_Zero_In_Rich_target(nullptr),
   fGammaInvMassReco_Zero_In_Rich_mvd(nullptr),
   fGammaOpeningAngleReco_Zero_In_Rich_mvd(nullptr),
   fPi0InvMassRecoKFV_Zero_In_Rich_mvd(nullptr),
   fGammaInvMassReco_Zero_In_Rich_sts(nullptr),
   fGammaOpeningAngleReco_Zero_In_Rich_sts(nullptr),
   fPi0InvMassRecoKFV_Zero_In_Rich_sts(nullptr),
   fHistoList_zero_outside(),
   fPi0InvMassRecoKFV_Zero_In_Rich_outside_target(nullptr),
// 1-2
   GammasOneTwoLeptonInRICH(),
   fHistoList_onetwoInRich(),
   fGammaInvMassMC_OneTwo_In_Rich(nullptr),
   fGammaInvMassReco_OneTwo_In_Rich(nullptr),
   fGammaOpeningAngleMC_OneTwo_In_Rich(nullptr),
   fGammaOpeningAngleReco_OneTwo_In_Rich(nullptr),
   fPi0InvMassTrueKFV_OneTwo_In_Rich(nullptr),
   fPi0InvMassRecoKFV_OneTwo_In_Rich(nullptr),
   fPdg_OneTwo_In_Rich(nullptr),
   fdx_OneTwo_In_Rich(nullptr),
   fdy_OneTwo_In_Rich(nullptr),
   fdz_OneTwo_In_Rich(nullptr),
   fD_OneTwo_In_Rich(nullptr),
   fP_mc_OneTwo_In_Rich(nullptr),
   fP_reco_OneTwo_In_Rich(nullptr),
   fPt_mc_OneTwo_In_Rich(nullptr),
   fPt_reco_OneTwo_In_Rich(nullptr),
   fPi0_pt_vs_rap_OneTwo_In_Rich(nullptr),
   fHistoList_onetwoInRich_target(),
   fHistoList_onetwoInRich_mvd(),
   fHistoList_onetwoInRich_sts(),
   fGammaInvMassReco_OneTwo_In_Rich_target(nullptr),
   fGammaOpeningAngleReco_OneTwo_In_Rich_target(nullptr),
   fPi0InvMassRecoKFV_OneTwo_In_Rich_target(nullptr),
   fGammaInvMassReco_OneTwo_In_Rich_mvd(nullptr),
   fGammaOpeningAngleReco_OneTwo_In_Rich_mvd(nullptr),
   fPi0InvMassRecoKFV_OneTwo_In_Rich_mvd(nullptr),
   fGammaInvMassReco_OneTwo_In_Rich_sts(nullptr),
   fGammaOpeningAngleReco_OneTwo_In_Rich_sts(nullptr),
   fPi0InvMassRecoKFV_OneTwo_In_Rich_sts(nullptr),
   fHistoList_onetwo_outside(),
   fPi0InvMassRecoKFV_OneTwo_In_Rich_outside_target(nullptr),

   fHistoList_MC(),
   fMC_start_vertex(nullptr),
   fMC_Pt(nullptr),
   fMC_P(nullptr),
   fPdgCodesMC(nullptr),

   fHistoList_CheckForCuts(),
   CheckForCuts_OA_MC(nullptr),
   CheckForCuts_OA_Reco(nullptr),
   CheckForCuts_InvMass_MC(nullptr),
   CheckForCuts_InvMass_Reco(nullptr),
   CheckForCuts_OA_MC_from_one_pi0(nullptr),
   CheckForCuts_OA_Reco_from_one_pi0(nullptr),
   CheckForCuts_InvMass_MC_from_one_pi0(nullptr),
   CheckForCuts_InvMass_Reco_from_one_pi0(nullptr),
   CheckForCuts_z_vs_InvMass_MC_from_one_pi0(nullptr),
   CheckForCuts_z_vs_OA_MC_from_one_pi0(nullptr),
   CheckForCuts_z_vs_InvMass_Reco_from_one_pi0(nullptr),
   CheckForCuts_z_vs_OA_Reco_from_one_pi0(nullptr),
   CheckForCuts_InvMass_Reco_from_one_pi0_less20cm(nullptr),
   CheckForCuts_OA_Reco_from_one_pi0_less20cm(nullptr),
   CheckForCuts_InvMass_Reco_from_one_pi0_between20cm_40cm(nullptr),
   CheckForCuts_OA_Reco_from_one_pi0_between20cm_40cm(nullptr),
   CheckForCuts_InvMass_Reco_from_one_pi0_more40cm(nullptr),
   CheckForCuts_OA_Reco_from_one_pi0_more40cm(nullptr),

    fAnaBG(nullptr),
    DoBGAnalysis(0),
    fHistoList_bg_all(),
    BG1_all(nullptr),
    BG2_all(nullptr),
    BG3_all(nullptr),
    BG4_all(nullptr),
    BG5_all(nullptr),
    BG6_all(nullptr),
    BG7_all(nullptr),
    BG8_all(nullptr),
    BG9_all(nullptr),
    BG10_all(nullptr),
    fHistoList_bg_one(),
    BG1_one(nullptr),
    BG2_one(nullptr),
    BG3_one(nullptr),
    BG4_one(nullptr),
    BG5_one(nullptr),
    BG6_one(nullptr),
    BG7_one(nullptr),
    BG8_one(nullptr),
    BG9_one(nullptr),
    BG10_one(nullptr),
    fHistoList_bg_two(),
    BG1_two(nullptr),
    BG2_two(nullptr),
    BG3_two(nullptr),
    BG4_two(nullptr),
    BG5_two(nullptr),
    BG6_two(nullptr),
    BG7_two(nullptr),
    BG8_two(nullptr),
    BG9_two(nullptr),
    BG10_two(nullptr),
    fHistoList_bg_zero(),
    BG1_zero(nullptr),
    BG2_zero(nullptr),
    BG3_zero(nullptr),
    BG4_zero(nullptr),
    BG5_zero(nullptr),
    BG6_zero(nullptr),
    BG7_zero(nullptr),
    BG8_zero(nullptr),
    BG9_zero(nullptr),
    BG10_zero(nullptr),
    fHistoList_bg_onetwo(),
    BG1_onetwo(nullptr),
    BG2_onetwo(nullptr),
    BG3_onetwo(nullptr),
    BG4_onetwo(nullptr),
    BG5_onetwo(nullptr),
    BG6_onetwo(nullptr),
    BG7_onetwo(nullptr),
    BG8_onetwo(nullptr),
    BG9_onetwo(nullptr),
    BG10_onetwo(nullptr),

    PdgCase8_all(nullptr),
    PdgCase8_one(nullptr),
    PdgCase8_two(nullptr),
    PdgCase8_zero(nullptr),
    PdgCase8_onetwo(nullptr),
    testsameMIDcase8_all(nullptr),
    testsameGRIDcase8_all(nullptr),
    testsameMIDcase8_one(nullptr),
    testsameGRIDcase8_one(nullptr),
    testsameMIDcase8_two(nullptr),
    testsameGRIDcase8_two(nullptr),
    testsameMIDcase8_zero(nullptr),
    testsameGRIDcase8_zero(nullptr),
    testsameMIDcase8_onetwo(nullptr),
    testsameGRIDcase8_onetwo(nullptr),

    PdgCase8mothers_all(nullptr),
    PdgCase8mothers_one(nullptr),
    PdgCase8mothers_two(nullptr),
    PdgCase8mothers_zero(nullptr),
    PdgCase8mothers_onetwo(nullptr),

    case8GRIDInvMassGamma_all(nullptr),
    case8GRIDOAGamma_all(nullptr),
    case8GRIDInvMassGamma_one(nullptr),
    case8GRIDOAGamma_one(nullptr),
    case8GRIDInvMassGamma_two(nullptr),
    case8GRIDOAGamma_two(nullptr), 
    case8GRIDInvMassGamma_zero(nullptr),
    case8GRIDOAGamma_zero(nullptr),
    case8GRIDInvMassGamma_onetwo(nullptr),
    case8GRIDOAGamma_onetwo(nullptr),

 
    fHistoList_pt(),
    fHistoList_pt_all(),
    Pt1_all(nullptr),
    Pt2_all(nullptr),
    Pt3_all(nullptr),
    Pt4_all(nullptr),
    Pt5_all(nullptr),
    Pt6_all(nullptr),
    Pt7_all(nullptr),
    Pt8_all(nullptr),
    Pt9_all(nullptr),
    Pt10_all(nullptr),
    Pt11_all(nullptr),
    Pt12_all(nullptr),
    fHistoList_pt_one(),
    Pt1_one(nullptr),
    Pt2_one(nullptr),
    Pt3_one(nullptr),
    Pt4_one(nullptr),
    Pt5_one(nullptr),
    Pt6_one(nullptr),
    Pt7_one(nullptr),
    Pt8_one(nullptr),
    Pt9_one(nullptr),
    Pt10_one(nullptr),
    Pt11_one(nullptr),
    Pt12_one(nullptr),
    fHistoList_pt_two(),
    Pt1_two(nullptr),
    Pt2_two(nullptr),
    Pt3_two(nullptr),
    Pt4_two(nullptr),
    Pt5_two(nullptr),
    Pt6_two(nullptr),
    Pt7_two(nullptr),
    Pt8_two(nullptr),
    Pt9_two(nullptr),
    Pt10_two(nullptr),
    Pt11_two(nullptr),
    Pt12_two(nullptr),
    fHistoList_pt_zero(),
    Pt1_zero(nullptr),
    Pt2_zero(nullptr),
    Pt3_zero(nullptr),
    Pt4_zero(nullptr),
    Pt5_zero(nullptr),
    Pt6_zero(nullptr),
    Pt7_zero(nullptr),
    Pt8_zero(nullptr),
    Pt9_zero(nullptr),
    Pt10_zero(nullptr),
    Pt11_zero(nullptr),
    Pt12_zero(nullptr),
    fHistoList_pt_onetwo(),
    Pt1_onetwo(nullptr),
    Pt2_onetwo(nullptr),
    Pt3_onetwo(nullptr),
    Pt4_onetwo(nullptr),
    Pt5_onetwo(nullptr),
    Pt6_onetwo(nullptr),
    Pt7_onetwo(nullptr),
    Pt8_onetwo(nullptr),
    Pt9_onetwo(nullptr),
    Pt10_onetwo(nullptr),
    Pt11_onetwo(nullptr),
    Pt12_onetwo(nullptr),

    Case1ZYPos_all(nullptr),
    Case1ZYPos_one(nullptr),
    Case1ZYPos_two(nullptr),
    Case1ZYPos_zero(nullptr),
    Case1ZYPos_onetwo(nullptr),
   timer(),
   fTime(0.)
{
}

CbmAnaConversion2KF::~CbmAnaConversion2KF()
{
}

void CbmAnaConversion2KF::SetKF(CbmKFParticleFinder* kfparticle, CbmKFParticleFinderQA* kfparticleQA)
{
	fKFparticle = kfparticle;
	fKFparticleFinderQA = kfparticleQA;
	if(fKFparticle) {
		cout << "CbmAnaConversion2KF: kf works" << endl;
	}
	else {
		cout << "CbmAnaConversion2KF: kf works not" << endl;
	}
}

void CbmAnaConversion2KF::Init()
{
	FairRootManager* ioman = FairRootManager::Instance();
	if (nullptr == ioman) { Fatal("CbmAnaConversion2KF::Init","RootManager not instantised!"); }

	fKFMcParticles = (TClonesArray*) ioman->GetObject("KFMCParticles");
	if ( nullptr == fKFMcParticles) { Fatal("CbmAnaConversion2KF::Init","No KFMCParticles array!"); }

	fMcTracks = (TClonesArray*) ioman->GetObject("MCTrack");
	if ( nullptr == fMcTracks) { Fatal("CbmAnaConversion2KF::Init","No MCTrack array!"); }

	fStsTracks = (TClonesArray*) ioman->GetObject("StsTrack");
	if ( nullptr == fStsTracks) { Fatal("CbmAnaConversion2KF::Init","No StsTrack array!"); }

	fStsTrackMatches = (TClonesArray*) ioman->GetObject("StsTrackMatch");
	if (nullptr == fStsTrackMatches) { Fatal("CbmAnaConversion2KF::Init","No StsTrackMatch array!"); }

	fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
	if (nullptr == fGlobalTracks) { Fatal("CbmAnaConversion2KF::Init","No GlobalTrack array!"); }

	fRichRings = (TClonesArray*) ioman->GetObject("RichRing");
	if (nullptr == fRichRings) { Fatal("CbmAnaConversion2KF::Init","No RichRing array!"); }

	fRichRingMatches = (TClonesArray*) ioman->GetObject("RichRingMatch");
	if (nullptr == fRichRingMatches) { Fatal("CbmAnaConversion2KF::Init","No RichRingMatch array!"); }

	fKFtopo = fKFparticle->GetTopoReconstructor();

	cout << " CbmAnaConversion2KF::Init() " << endl;
	InitHistos();

	DoBGAnalysis = 1;

	if(DoBGAnalysis) {
		fAnaBG = new CbmAnaConversion2BG();
		fAnaBG->Init();
	}

}


void CbmAnaConversion2KF::InitHistos()
{

	fPdgCodesMC = new TH1D("fPdgCodesMC", "fPdgCodesMC;pdg code;#", 1000, 0, 1000);
	fHistoList_MC.push_back(fPdgCodesMC);

	fMC_start_vertex = new TH2D("fMC_start_vertex","fMC_start_vertex; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_MC.push_back(fMC_start_vertex);

	fMC_Pt = new TH1D("fMC_Pt","fMC_Pt; P_{t} in GeV/c", 200, 0, 10);
	fHistoList_MC.push_back(fMC_Pt);

	fMC_P = new TH1D("fMC_P","fMC_P; P in GeV/c", 200, 0, 10);
	fHistoList_MC.push_back(fMC_P);

	CheckForCuts_OA_MC = new TH1D("CheckForCuts_OA_MC","CheckForCuts_OA_MC; angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_OA_MC);

	CheckForCuts_OA_Reco = new TH1D("CheckForCuts_OA_Reco","CheckForCuts_OA_Reco; angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_OA_Reco);

	CheckForCuts_InvMass_MC = new TH1D("CheckForCuts_InvMass_MC","CheckForCuts_InvMass_MC; invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_InvMass_MC);

	CheckForCuts_InvMass_Reco = new TH1D("CheckForCuts_InvMass_Reco","CheckForCuts_InvMass_Reco; invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_InvMass_Reco);

	CheckForCuts_OA_MC_from_one_pi0 = new TH1D("CheckForCuts_OA_MC_from_one_pi0","CheckForCuts_OA_MC_from_one_pi0; angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_OA_MC_from_one_pi0);

	CheckForCuts_OA_Reco_from_one_pi0 = new TH1D("CheckForCuts_OA_Reco_from_one_pi0","CheckForCuts_OA_Reco_from_one_pi0; angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_OA_Reco_from_one_pi0);

	CheckForCuts_InvMass_MC_from_one_pi0 = new TH1D("CheckForCuts_InvMass_MC_from_one_pi0","CheckForCuts_InvMass_MC_from_one_pi0; invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_InvMass_MC_from_one_pi0);

	CheckForCuts_InvMass_Reco_from_one_pi0 = new TH1D("CheckForCuts_InvMass_Reco_from_one_pi0","CheckForCuts_InvMass_Reco_from_one_pi0; invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_InvMass_Reco_from_one_pi0);

	CheckForCuts_z_vs_OA_MC_from_one_pi0 = new TH2D("CheckForCuts_z_vs_OA_MC_from_one_pi0","CheckForCuts_z_vs_OA_MC_from_one_pi0 ;Z [cm]; #theta in deg; Nof", 1000, -1, 99, 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_z_vs_OA_MC_from_one_pi0);

	CheckForCuts_z_vs_OA_Reco_from_one_pi0 = new TH2D("CheckForCuts_z_vs_OA_Reco_from_one_pi0","CheckForCuts_z_vs_OA_Reco_from_one_pi0 ;Z [cm]; #theta in deg; Nof", 1000, -1, 99, 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_z_vs_OA_Reco_from_one_pi0);

	CheckForCuts_z_vs_InvMass_MC_from_one_pi0 = new TH2D("CheckForCuts_z_vs_InvMass_MC_from_one_pi0", "CheckForCuts_z_vs_InvMass_MC_from_one_pi0 ;Z [cm]; invariant mass in GeV/c^{2}; Nof", 1000, -1, 99, 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_z_vs_InvMass_MC_from_one_pi0);

	CheckForCuts_z_vs_InvMass_Reco_from_one_pi0 = new TH2D("CheckForCuts_z_vs_InvMass_Reco_from_one_pi0","CheckForCuts_z_vs_InvMass_Reco_from_one_pi0 ;Z [cm]; invariant mass in GeV/c^{2}; Nof", 1000, -1, 99, 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_z_vs_InvMass_Reco_from_one_pi0);

	CheckForCuts_InvMass_Reco_from_one_pi0_less20cm = new TH1D("CheckForCuts_InvMass_Reco_from_one_pi0_less20cm","CheckForCuts_InvMass_Reco_from_one_pi0_less20cm; invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_InvMass_Reco_from_one_pi0_less20cm);

	CheckForCuts_OA_Reco_from_one_pi0_less20cm = new TH1D("CheckForCuts_OA_Reco_from_one_pi0_less20cm","CheckForCuts_OA_Reco_from_one_pi0_less20cm; angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_OA_Reco_from_one_pi0_less20cm);

	CheckForCuts_InvMass_Reco_from_one_pi0_between20cm_40cm = new TH1D("CheckForCuts_InvMass_Reco_from_one_pi0_between20cm_40cm","CheckForCuts_InvMass_Reco_from_one_pi0_between20cm_40cm; invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_InvMass_Reco_from_one_pi0_between20cm_40cm);

	CheckForCuts_OA_Reco_from_one_pi0_between20cm_40cm = new TH1D("CheckForCuts_OA_Reco_from_one_pi0_between20cm_40cm","CheckForCuts_OA_Reco_from_one_pi0_between20cm_40cm; angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_OA_Reco_from_one_pi0_between20cm_40cm);

	CheckForCuts_InvMass_Reco_from_one_pi0_more40cm = new TH1D("CheckForCuts_InvMass_Reco_from_one_pi0_more40cm","CheckForCuts_InvMass_Reco_from_one_pi0_more40cm; invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_CheckForCuts.push_back(CheckForCuts_InvMass_Reco_from_one_pi0_more40cm);

	CheckForCuts_OA_Reco_from_one_pi0_more40cm = new TH1D("CheckForCuts_OA_Reco_from_one_pi0_more40cm","CheckForCuts_OA_Reco_from_one_pi0_more40cm; angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_CheckForCuts.push_back(CheckForCuts_OA_Reco_from_one_pi0_more40cm);

	// Event Mixing
	gr_EMT_InvMass_all = new TH1D("gr_EMT_InvMass_all","gr_EMT_InvMass_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_mixing.push_back(gr_EMT_InvMass_all);

	gr_EMT_InvMass_oneInRICH = new TH1D("gr_EMT_InvMass_oneInRICH","gr_EMT_InvMass_oneInRICH;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_mixing.push_back(gr_EMT_InvMass_oneInRICH);

	gr_EMT_InvMass_twoInRICH = new TH1D("gr_EMT_InvMass_twoInRICH","gr_EMT_InvMass_twoInRICH;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_mixing.push_back(gr_EMT_InvMass_twoInRICH);

	gr_EMT_InvMass_zeroInRICH = new TH1D("gr_EMT_InvMass_zeroInRICH","gr_EMT_InvMass_zeroInRICH;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_mixing.push_back(gr_EMT_InvMass_zeroInRICH);

	gr_EMT_InvMass_onetwoInRICH = new TH1D("gr_EMT_InvMass_onetwoInRICH","gr_EMT_InvMass_onetwoInRICH;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_mixing.push_back(gr_EMT_InvMass_onetwoInRICH);

	gr_EMT_InvMass_all_target = new TH1D("gr_EMT_InvMass_all_target","gr_EMT_InvMass_all_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_target.push_back(gr_EMT_InvMass_all_target);
	gr_EMT_InvMass_oneInRICH_target = new TH1D("gr_EMT_InvMass_oneInRICH_target","gr_EMT_InvMass_oneInRICH_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich_target.push_back(gr_EMT_InvMass_oneInRICH_target);
	gr_EMT_InvMass_twoInRICH_target = new TH1D("gr_EMT_InvMass_twoInRICH_target","gr_EMT_InvMass_twoInRICH_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich_target.push_back(gr_EMT_InvMass_twoInRICH_target);
	gr_EMT_InvMass_zeroInRICH_target = new TH1D("gr_EMT_InvMass_zeroInRICH_target","gr_EMT_InvMass_zeroInRICH_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich_target.push_back(gr_EMT_InvMass_zeroInRICH_target);
	gr_EMT_InvMass_onetwoInRICH_target = new TH1D("gr_EMT_InvMass_onetwoInRICH_target","gr_EMT_InvMass_onetwoInRICH_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich_target.push_back(gr_EMT_InvMass_onetwoInRICH_target);
	gr_EMT_InvMass_all_outside_target = new TH1D("gr_EMT_InvMass_all_outside_target","gr_EMT_InvMass_all_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_outside.push_back(gr_EMT_InvMass_all_outside_target);
	gr_EMT_InvMass_oneInRICH_outside_target = new TH1D("gr_EMT_InvMass_oneInRICH_outside_target","gr_EMT_InvMass_oneInRICH_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_one_outside.push_back(gr_EMT_InvMass_oneInRICH_outside_target);
	gr_EMT_InvMass_twoInRICH_outside_target = new TH1D("gr_EMT_InvMass_twoInRICH_outside_target","gr_EMT_InvMass_twoInRICH_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_two_outside.push_back(gr_EMT_InvMass_twoInRICH_outside_target);
	gr_EMT_InvMass_zeroInRICH_outside_target = new TH1D("gr_EMT_InvMass_zeroInRICH_outside_target","gr_EMT_InvMass_zeroInRICH_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zero_outside.push_back(gr_EMT_InvMass_zeroInRICH_outside_target);
	gr_EMT_InvMass_onetwoInRICH_outside_target = new TH1D("gr_EMT_InvMass_onetwoInRICH_outside_target","gr_EMT_InvMass_onetwoInRICH_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwo_outside.push_back(gr_EMT_InvMass_onetwoInRICH_outside_target);

	// Multiplicity
	MultiplicityInv_all = new TH2D("MultiplicityInv_all","MultiplicityInv_all;Nof gammas in event;invariant mass in GeV/c^{2};#", 400, 0, 30, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(MultiplicityInv_all);

	MultiplicityInv_one = new TH2D("MultiplicityInv_one","MultiplicityInv_one;Nof gammas in event;invariant mass in GeV/c^{2};#", 400, 0, 30, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(MultiplicityInv_one);

	MultiplicityInv_two = new TH2D("MultiplicityInv_two","MultiplicityInv_two;Nof gammas in event;invariant mass in GeV/c^{2};#", 400, 0, 30, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(MultiplicityInv_two);

	MultiplicityInv_zero = new TH2D("MultiplicityInv_zero","MultiplicityInv_zero;Nof gammas in event;invariant mass in GeV/c^{2};#", 400, 0, 30, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(MultiplicityInv_zero);

	MultiplicityInv_onetwo = new TH2D("MultiplicityInv_onetwo","MultiplicityInv_onetwo;Nof gammas in event;invariant mass in GeV/c^{2};#", 400, 0, 30, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(MultiplicityInv_onetwo);

	Multiplicity_chargedParticles_Inv_all = new TH2D("Multiplicity_chargedParticles_Inv_all","Multiplicity_chargedParticles_Inv_all;Nof charged particles in event;invariant mass in GeV/c^{2};#", 1000, 0, 1000, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(Multiplicity_chargedParticles_Inv_all);

	Multiplicity_chargedParticles_Inv_one = new TH2D("Multiplicity_chargedParticles_Inv_one","Multiplicity_chargedParticles_Inv_one;Nof charged particles in event;invariant mass in GeV/c^{2};#", 1000, 0, 1000, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(Multiplicity_chargedParticles_Inv_one);

	Multiplicity_chargedParticles_Inv_two = new TH2D("Multiplicity_chargedParticles_Inv_two","Multiplicity_chargedParticles_Inv_two;Nof charged particles in event;invariant mass in GeV/c^{2};#", 1000, 0, 1000, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(Multiplicity_chargedParticles_Inv_two);

	Multiplicity_chargedParticles_Inv_zero = new TH2D("Multiplicity_chargedParticles_Inv_zero","Multiplicity_chargedParticles_Inv_zero;Nof charged particles in event;invariant mass in GeV/c^{2};#", 1000, 0, 1000, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(Multiplicity_chargedParticles_Inv_zero);

	Multiplicity_chargedParticles_Inv_onetwo = new TH2D("Multiplicity_chargedParticles_Inv_onetwo","Multiplicity_chargedParticles_Inv_onetwo;Nof charged particles in event;invariant mass in GeV/c^{2};#", 1000, 0, 1000, 1000, 0, 3.0);   
	fHistoList_multiplicity.push_back(Multiplicity_chargedParticles_Inv_onetwo);


	gr_EMTMulti_InvMass_all_m1 = new TH1D("gr_EMTMulti_InvMass_all_m1","gr_EMTMulti_InvMass_all_m1;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m1);
	gr_EMTMulti_InvMass_all_m2 = new TH1D("gr_EMTMulti_InvMass_all_m2","gr_EMTMulti_InvMass_all_m2;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m2);
	gr_EMTMulti_InvMass_all_m3 = new TH1D("gr_EMTMulti_InvMass_all_m3","gr_EMTMulti_InvMass_all_m3;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m3);
	gr_EMTMulti_InvMass_all_m4 = new TH1D("gr_EMTMulti_InvMass_all_m4","gr_EMTMulti_InvMass_all_m4;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m4);
	gr_EMTMulti_InvMass_all_m5 = new TH1D("gr_EMTMulti_InvMass_all_m5","gr_EMTMulti_InvMass_all_m5;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m5);
	gr_EMTMulti_InvMass_all_m6 = new TH1D("gr_EMTMulti_InvMass_all_m6","gr_EMTMulti_InvMass_all_m6;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m6);
	gr_EMTMulti_InvMass_all_m7 = new TH1D("gr_EMTMulti_InvMass_all_m7","gr_EMTMulti_InvMass_all_m7;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m7);
	gr_EMTMulti_InvMass_all_m8 = new TH1D("gr_EMTMulti_InvMass_all_m8","gr_EMTMulti_InvMass_all_m8;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m8);
	gr_EMTMulti_InvMass_all_m9 = new TH1D("gr_EMTMulti_InvMass_all_m9","gr_EMTMulti_InvMass_all_m9;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m9);
	gr_EMTMulti_InvMass_all_m10 = new TH1D("gr_EMTMulti_InvMass_all_m10","gr_EMTMulti_InvMass_all_m10;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m10);
	gr_EMTMulti_InvMass_all_m11 = new TH1D("gr_EMTMulti_InvMass_all_m11","gr_EMTMulti_InvMass_all_m11;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m11);
	gr_EMTMulti_InvMass_all_m12 = new TH1D("gr_EMTMulti_InvMass_all_m12","gr_EMTMulti_InvMass_all_m12;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m12);
	gr_EMTMulti_InvMass_all_m13 = new TH1D("gr_EMTMulti_InvMass_all_m13","gr_EMTMulti_InvMass_all_m13;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m13);
	gr_EMTMulti_InvMass_all_m14 = new TH1D("gr_EMTMulti_InvMass_all_m14","gr_EMTMulti_InvMass_all_m14;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m14);
	gr_EMTMulti_InvMass_all_m15 = new TH1D("gr_EMTMulti_InvMass_all_m15","gr_EMTMulti_InvMass_all_m15;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m15);
	gr_EMTMulti_InvMass_all_m16 = new TH1D("gr_EMTMulti_InvMass_all_m16","gr_EMTMulti_InvMass_all_m16;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m16);
	gr_EMTMulti_InvMass_all_m17 = new TH1D("gr_EMTMulti_InvMass_all_m17","gr_EMTMulti_InvMass_all_m17;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m17);
	gr_EMTMulti_InvMass_all_m18 = new TH1D("gr_EMTMulti_InvMass_all_m18","gr_EMTMulti_InvMass_all_m18;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m18);
	gr_EMTMulti_InvMass_all_m19 = new TH1D("gr_EMTMulti_InvMass_all_m19","gr_EMTMulti_InvMass_all_m19;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m19);
	gr_EMTMulti_InvMass_all_m20 = new TH1D("gr_EMTMulti_InvMass_all_m20","gr_EMTMulti_InvMass_all_m20;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_all.push_back(gr_EMTMulti_InvMass_all_m20);

	gr_EMTMulti_InvMass_one_m1 = new TH1D("gr_EMTMulti_InvMass_one_m1","gr_EMTMulti_InvMass_one_m1;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m1);
	gr_EMTMulti_InvMass_one_m2 = new TH1D("gr_EMTMulti_InvMass_one_m2","gr_EMTMulti_InvMass_one_m2;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m2);
	gr_EMTMulti_InvMass_one_m3 = new TH1D("gr_EMTMulti_InvMass_one_m3","gr_EMTMulti_InvMass_one_m3;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m3);
	gr_EMTMulti_InvMass_one_m4 = new TH1D("gr_EMTMulti_InvMass_one_m4","gr_EMTMulti_InvMass_one_m4;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m4);
	gr_EMTMulti_InvMass_one_m5 = new TH1D("gr_EMTMulti_InvMass_one_m5","gr_EMTMulti_InvMass_one_m5;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m5);
	gr_EMTMulti_InvMass_one_m6 = new TH1D("gr_EMTMulti_InvMass_one_m6","gr_EMTMulti_InvMass_one_m6;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m6);
	gr_EMTMulti_InvMass_one_m7 = new TH1D("gr_EMTMulti_InvMass_one_m7","gr_EMTMulti_InvMass_one_m7;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m7);
	gr_EMTMulti_InvMass_one_m8 = new TH1D("gr_EMTMulti_InvMass_one_m8","gr_EMTMulti_InvMass_one_m8;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m8);
	gr_EMTMulti_InvMass_one_m9 = new TH1D("gr_EMTMulti_InvMass_one_m9","gr_EMTMulti_InvMass_one_m9;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m9);
	gr_EMTMulti_InvMass_one_m10 = new TH1D("gr_EMTMulti_InvMass_one_m10","gr_EMTMulti_InvMass_one_m10;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m10);
	gr_EMTMulti_InvMass_one_m11 = new TH1D("gr_EMTMulti_InvMass_one_m11","gr_EMTMulti_InvMass_one_m11;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m11);
	gr_EMTMulti_InvMass_one_m12 = new TH1D("gr_EMTMulti_InvMass_one_m12","gr_EMTMulti_InvMass_one_m12;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m12);
	gr_EMTMulti_InvMass_one_m13 = new TH1D("gr_EMTMulti_InvMass_one_m13","gr_EMTMulti_InvMass_one_m13;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m13);
	gr_EMTMulti_InvMass_one_m14 = new TH1D("gr_EMTMulti_InvMass_one_m14","gr_EMTMulti_InvMass_one_m14;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m14);
	gr_EMTMulti_InvMass_one_m15 = new TH1D("gr_EMTMulti_InvMass_one_m15","gr_EMTMulti_InvMass_one_m15;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m15);
	gr_EMTMulti_InvMass_one_m16 = new TH1D("gr_EMTMulti_InvMass_one_m16","gr_EMTMulti_InvMass_one_m16;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m16);
	gr_EMTMulti_InvMass_one_m17 = new TH1D("gr_EMTMulti_InvMass_one_m17","gr_EMTMulti_InvMass_one_m17;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m17);
	gr_EMTMulti_InvMass_one_m18 = new TH1D("gr_EMTMulti_InvMass_one_m18","gr_EMTMulti_InvMass_one_m18;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m18);
	gr_EMTMulti_InvMass_one_m19 = new TH1D("gr_EMTMulti_InvMass_one_m19","gr_EMTMulti_InvMass_one_m19;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m19);
	gr_EMTMulti_InvMass_one_m20 = new TH1D("gr_EMTMulti_InvMass_one_m20","gr_EMTMulti_InvMass_one_m20;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_one.push_back(gr_EMTMulti_InvMass_one_m20);

	gr_EMTMulti_InvMass_two_m1 = new TH1D("gr_EMTMulti_InvMass_two_m1","gr_EMTMulti_InvMass_two_m1;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m1);
	gr_EMTMulti_InvMass_two_m2 = new TH1D("gr_EMTMulti_InvMass_two_m2","gr_EMTMulti_InvMass_two_m2;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m2);
	gr_EMTMulti_InvMass_two_m3 = new TH1D("gr_EMTMulti_InvMass_two_m3","gr_EMTMulti_InvMass_two_m3;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m3);
	gr_EMTMulti_InvMass_two_m4 = new TH1D("gr_EMTMulti_InvMass_two_m4","gr_EMTMulti_InvMass_two_m4;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m4);
	gr_EMTMulti_InvMass_two_m5 = new TH1D("gr_EMTMulti_InvMass_two_m5","gr_EMTMulti_InvMass_two_m5;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m5);
	gr_EMTMulti_InvMass_two_m6 = new TH1D("gr_EMTMulti_InvMass_two_m6","gr_EMTMulti_InvMass_two_m6;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m6);
	gr_EMTMulti_InvMass_two_m7 = new TH1D("gr_EMTMulti_InvMass_two_m7","gr_EMTMulti_InvMass_two_m7;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m7);
	gr_EMTMulti_InvMass_two_m8 = new TH1D("gr_EMTMulti_InvMass_two_m8","gr_EMTMulti_InvMass_two_m8;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m8);
	gr_EMTMulti_InvMass_two_m9 = new TH1D("gr_EMTMulti_InvMass_two_m9","gr_EMTMulti_InvMass_two_m9;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m9);
	gr_EMTMulti_InvMass_two_m10 = new TH1D("gr_EMTMulti_InvMass_two_m10","gr_EMTMulti_InvMass_two_m10;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m10);
	gr_EMTMulti_InvMass_two_m11 = new TH1D("gr_EMTMulti_InvMass_two_m11","gr_EMTMulti_InvMass_two_m11;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m11);
	gr_EMTMulti_InvMass_two_m12 = new TH1D("gr_EMTMulti_InvMass_two_m12","gr_EMTMulti_InvMass_two_m12;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m12);
	gr_EMTMulti_InvMass_two_m13 = new TH1D("gr_EMTMulti_InvMass_two_m13","gr_EMTMulti_InvMass_two_m13;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m13);
	gr_EMTMulti_InvMass_two_m14 = new TH1D("gr_EMTMulti_InvMass_two_m14","gr_EMTMulti_InvMass_two_m14;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m14);
	gr_EMTMulti_InvMass_two_m15 = new TH1D("gr_EMTMulti_InvMass_two_m15","gr_EMTMulti_InvMass_two_m15;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m15);
	gr_EMTMulti_InvMass_two_m16 = new TH1D("gr_EMTMulti_InvMass_two_m16","gr_EMTMulti_InvMass_two_m16;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m16);
	gr_EMTMulti_InvMass_two_m17 = new TH1D("gr_EMTMulti_InvMass_two_m17","gr_EMTMulti_InvMass_two_m17;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m17);
	gr_EMTMulti_InvMass_two_m18 = new TH1D("gr_EMTMulti_InvMass_two_m18","gr_EMTMulti_InvMass_two_m18;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m18);
	gr_EMTMulti_InvMass_two_m19 = new TH1D("gr_EMTMulti_InvMass_two_m19","gr_EMTMulti_InvMass_two_m19;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m19);
	gr_EMTMulti_InvMass_two_m20 = new TH1D("gr_EMTMulti_InvMass_two_m20","gr_EMTMulti_InvMass_two_m20;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_two.push_back(gr_EMTMulti_InvMass_two_m20);

	gr_EMTMulti_InvMass_zero_m1 = new TH1D("gr_EMTMulti_InvMass_zero_m1","gr_EMTMulti_InvMass_zero_m1;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m1);
	gr_EMTMulti_InvMass_zero_m2 = new TH1D("gr_EMTMulti_InvMass_zero_m2","gr_EMTMulti_InvMass_zero_m2;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m2);
	gr_EMTMulti_InvMass_zero_m3 = new TH1D("gr_EMTMulti_InvMass_zero_m3","gr_EMTMulti_InvMass_zero_m3;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m3);
	gr_EMTMulti_InvMass_zero_m4 = new TH1D("gr_EMTMulti_InvMass_zero_m4","gr_EMTMulti_InvMass_zero_m4;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m4);
	gr_EMTMulti_InvMass_zero_m5 = new TH1D("gr_EMTMulti_InvMass_zero_m5","gr_EMTMulti_InvMass_zero_m5;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m5);
	gr_EMTMulti_InvMass_zero_m6 = new TH1D("gr_EMTMulti_InvMass_zero_m6","gr_EMTMulti_InvMass_zero_m6;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m6);
	gr_EMTMulti_InvMass_zero_m7 = new TH1D("gr_EMTMulti_InvMass_zero_m7","gr_EMTMulti_InvMass_zero_m7;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m7);
	gr_EMTMulti_InvMass_zero_m8 = new TH1D("gr_EMTMulti_InvMass_zero_m8","gr_EMTMulti_InvMass_zero_m8;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m8);
	gr_EMTMulti_InvMass_zero_m9 = new TH1D("gr_EMTMulti_InvMass_zero_m9","gr_EMTMulti_InvMass_zero_m9;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m9);
	gr_EMTMulti_InvMass_zero_m10 = new TH1D("gr_EMTMulti_InvMass_zero_m10","gr_EMTMulti_InvMass_zero_m10;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m10);
	gr_EMTMulti_InvMass_zero_m11 = new TH1D("gr_EMTMulti_InvMass_zero_m11","gr_EMTMulti_InvMass_zero_m11;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m11);
	gr_EMTMulti_InvMass_zero_m12 = new TH1D("gr_EMTMulti_InvMass_zero_m12","gr_EMTMulti_InvMass_zero_m12;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m12);
	gr_EMTMulti_InvMass_zero_m13 = new TH1D("gr_EMTMulti_InvMass_zero_m13","gr_EMTMulti_InvMass_zero_m13;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m13);
	gr_EMTMulti_InvMass_zero_m14 = new TH1D("gr_EMTMulti_InvMass_zero_m14","gr_EMTMulti_InvMass_zero_m14;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m14);
	gr_EMTMulti_InvMass_zero_m15 = new TH1D("gr_EMTMulti_InvMass_zero_m15","gr_EMTMulti_InvMass_zero_m15;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m15);
	gr_EMTMulti_InvMass_zero_m16 = new TH1D("gr_EMTMulti_InvMass_zero_m16","gr_EMTMulti_InvMass_zero_m16;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m16);
	gr_EMTMulti_InvMass_zero_m17 = new TH1D("gr_EMTMulti_InvMass_zero_m17","gr_EMTMulti_InvMass_zero_m17;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m17);
	gr_EMTMulti_InvMass_zero_m18 = new TH1D("gr_EMTMulti_InvMass_zero_m18","gr_EMTMulti_InvMass_zero_m18;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m18);
	gr_EMTMulti_InvMass_zero_m19 = new TH1D("gr_EMTMulti_InvMass_zero_m19","gr_EMTMulti_InvMass_zero_m19;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m19);
	gr_EMTMulti_InvMass_zero_m20 = new TH1D("gr_EMTMulti_InvMass_zero_m20","gr_EMTMulti_InvMass_zero_m20;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_zero.push_back(gr_EMTMulti_InvMass_zero_m20);

	gr_EMTMulti_InvMass_onetwo_m1 = new TH1D("gr_EMTMulti_InvMass_onetwo_m1","gr_EMTMulti_InvMass_onetwo_m1;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m1);
	gr_EMTMulti_InvMass_onetwo_m2 = new TH1D("gr_EMTMulti_InvMass_onetwo_m2","gr_EMTMulti_InvMass_onetwo_m2;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m2);
	gr_EMTMulti_InvMass_onetwo_m3 = new TH1D("gr_EMTMulti_InvMass_onetwo_m3","gr_EMTMulti_InvMass_onetwo_m3;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m3);
	gr_EMTMulti_InvMass_onetwo_m4 = new TH1D("gr_EMTMulti_InvMass_onetwo_m4","gr_EMTMulti_InvMass_onetwo_m4;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m4);
	gr_EMTMulti_InvMass_onetwo_m5 = new TH1D("gr_EMTMulti_InvMass_onetwo_m5","gr_EMTMulti_InvMass_onetwo_m5;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m5);
	gr_EMTMulti_InvMass_onetwo_m6 = new TH1D("gr_EMTMulti_InvMass_onetwo_m6","gr_EMTMulti_InvMass_onetwo_m6;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m6);
	gr_EMTMulti_InvMass_onetwo_m7 = new TH1D("gr_EMTMulti_InvMass_onetwo_m7","gr_EMTMulti_InvMass_onetwo_m7;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m7);
	gr_EMTMulti_InvMass_onetwo_m8 = new TH1D("gr_EMTMulti_InvMass_onetwo_m8","gr_EMTMulti_InvMass_onetwo_m8;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m8);
	gr_EMTMulti_InvMass_onetwo_m9 = new TH1D("gr_EMTMulti_InvMass_onetwo_m9","gr_EMTMulti_InvMass_onetwo_m9;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m9);
	gr_EMTMulti_InvMass_onetwo_m10 = new TH1D("gr_EMTMulti_InvMass_onetwo_m10","gr_EMTMulti_InvMass_onetwo_m10;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m10);
	gr_EMTMulti_InvMass_onetwo_m11 = new TH1D("gr_EMTMulti_InvMass_onetwo_m11","gr_EMTMulti_InvMass_onetwo_m11;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m11);
	gr_EMTMulti_InvMass_onetwo_m12 = new TH1D("gr_EMTMulti_InvMass_onetwo_m12","gr_EMTMulti_InvMass_onetwo_m12;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m12);
	gr_EMTMulti_InvMass_onetwo_m13 = new TH1D("gr_EMTMulti_InvMass_onetwo_m13","gr_EMTMulti_InvMass_onetwo_m13;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m13);
	gr_EMTMulti_InvMass_onetwo_m14 = new TH1D("gr_EMTMulti_InvMass_onetwo_m14","gr_EMTMulti_InvMass_onetwo_m14;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m14);
	gr_EMTMulti_InvMass_onetwo_m15 = new TH1D("gr_EMTMulti_InvMass_onetwo_m15","gr_EMTMulti_InvMass_onetwo_m15;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m15);
	gr_EMTMulti_InvMass_onetwo_m16 = new TH1D("gr_EMTMulti_InvMass_onetwo_m16","gr_EMTMulti_InvMass_onetwo_m16;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m16);
	gr_EMTMulti_InvMass_onetwo_m17 = new TH1D("gr_EMTMulti_InvMass_onetwo_m17","gr_EMTMulti_InvMass_onetwo_m17;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m17);
	gr_EMTMulti_InvMass_onetwo_m18 = new TH1D("gr_EMTMulti_InvMass_onetwo_m18","gr_EMTMulti_InvMass_onetwo_m18;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m18);
	gr_EMTMulti_InvMass_onetwo_m19 = new TH1D("gr_EMTMulti_InvMass_onetwo_m19","gr_EMTMulti_InvMass_onetwo_m19;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m9);
	gr_EMTMulti_InvMass_onetwo_m20 = new TH1D("gr_EMTMulti_InvMass_onetwo_m20","gr_EMTMulti_InvMass_onetwo_m20;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_multiplicity_onetwo.push_back(gr_EMTMulti_InvMass_onetwo_m20);


	//0-2. Could be, that no tracks in RICH at all (not sure. Maksym used UseRICHRvspPID). Combined from all charged particles. Both have track in STS.    **maybe cut on opening angle
	fGammaInvMassMC_All = new TH1D("fGammaInvMassMC_All","fGammaInvMassMC_All;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all.push_back(fGammaInvMassMC_All);

	fGammaInvMassReco_All = new TH1D("fGammaInvMassReco_All","fGammaInvMassReco_All;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all.push_back(fGammaInvMassReco_All);

	fGammaOpeningAngleMC_All = new TH1D("fGammaOpeningAngleMC_All","fGammaOpeningAngleMC_All;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_all.push_back(fGammaOpeningAngleMC_All);

	fGammaOpeningAngleReco_All = new TH1D("fGammaOpeningAngleReco_All","fGammaOpeningAngleReco_All;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_all.push_back(fGammaOpeningAngleReco_All);

	fPdg_All = new TH1D("fPdg_All","fPdg_All ;Id ;#", 4001, 0, 1000);
	fHistoList_all.push_back(fPdg_All);

	fPi0InvMassTrueKFV_All = new TH1D("fPi0InvMassTrueKFV_All","fPi0InvMassTrueKFV_All;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all.push_back(fPi0InvMassTrueKFV_All);

	fPi0InvMassRecoKFV_All = new TH1D("fPi0InvMassRecoKFV_All","fPi0InvMassRecoKFV_All;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all.push_back(fPi0InvMassRecoKFV_All);

	fdx_All = new TH1D("fdX_All","fdX_All ;dx [cm] ;#", 400, -10, 10);
	fHistoList_all.push_back(fdx_All);

	fdy_All = new TH1D("fdy_All","fdy_All ;dy [cm] ;#", 400, -10, 10);
	fHistoList_all.push_back(fdy_All);

	fdz_All = new TH1D("fdz_All","fdz_All ;dz [cm] ;#", 400, -10, 10);
	fHistoList_all.push_back(fdz_All);

	fD_All = new TH1D("fD_All","fD_All ;D [cm] ;#", 400, 0, 40);
	fHistoList_all.push_back(fD_All);

	fP_mc_All = new TH1D("fP_mc_All","fP_mc_All ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_all.push_back(fP_mc_All);

	fP_reco_All = new TH1D("fP_reco_All","fP_reco_All ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_all.push_back(fP_reco_All);

	fPt_mc_All = new TH1D("fPt_mc_All","fPt_mc_All ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_all.push_back(fPt_mc_All);

	fPt_reco_All = new TH1D("fPt_reco_All","fPt_reco_All ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_all.push_back(fPt_reco_All);

	fPi0_pt_vs_rap_All = new TH2D("fPi0_pt_vs_rap_All", "fPi0_pt_vs_rap_All;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);  
	fHistoList_all.push_back(fPi0_pt_vs_rap_All);

	// 0-2  target
	fGammaInvMassReco_All_target = new TH1D("fGammaInvMassReco_All_target","fGammaInvMassReco_All_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_target.push_back(fGammaInvMassReco_All_target);
	fGammaOpeningAngleReco_All_target = new TH1D("fGammaOpeningAngleReco_All_target","fGammaOpeningAngleReco_All_target;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_all_target.push_back(fGammaOpeningAngleReco_All_target);
	fPi0InvMassRecoKFV_All_target = new TH1D("fPi0InvMassRecoKFV_All_target","fPi0InvMassRecoKFV_All_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_target.push_back(fPi0InvMassRecoKFV_All_target);
	// 0-2  mvd
	fGammaInvMassReco_All_mvd = new TH1D("fGammaInvMassReco_All_mvd","fGammaInvMassReco_All_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_mvd.push_back(fGammaInvMassReco_All_mvd);
	fGammaOpeningAngleReco_All_mvd = new TH1D("fGammaOpeningAngleReco_All_mvd","fGammaOpeningAngleReco_All_mvd;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_all_mvd.push_back(fGammaOpeningAngleReco_All_mvd);
	fPi0InvMassRecoKFV_All_mvd = new TH1D("fPi0InvMassRecoKFV_All_mvd","fPi0InvMassRecoKFV_All_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_mvd.push_back(fPi0InvMassRecoKFV_All_mvd);
	// 0-2  sts
	fGammaInvMassReco_All_sts = new TH1D("fGammaInvMassReco_All_sts","fGammaInvMassReco_All_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_sts.push_back(fGammaInvMassReco_All_sts);
	fGammaOpeningAngleReco_All_sts = new TH1D("fGammaOpeningAngleReco_All_sts","fGammaOpeningAngleReco_All_sts;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_all_sts.push_back(fGammaOpeningAngleReco_All_sts);
	fPi0InvMassRecoKFV_All_sts = new TH1D("fPi0InvMassRecoKFV_All_sts","fPi0InvMassRecoKFV_All_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_sts.push_back(fPi0InvMassRecoKFV_All_sts);
	// 0-2 outside the target
	fPi0InvMassRecoKFV_All_In_Rich_outside_target = new TH1D("fPi0InvMassRecoKFV_All_In_Rich_outside_target","fPi0InvMassRecoKFV_All_In_Rich_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_all_outside.push_back(fPi0InvMassRecoKFV_All_In_Rich_outside_target);



	//1. One particle from gamma registered in RICH like electron/positron. Second particle may be any. Both have track in STS.    **maybe cut on opening angle
	fGammaInvMassMC_One_In_Rich = new TH1D("fGammaInvMassMC_One_In_Rich","fGammaInvMassMC_One_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich.push_back(fGammaInvMassMC_One_In_Rich);

	fGammaInvMassReco_One_In_Rich = new TH1D("fGammaInvMassReco_One_In_Rich","fGammaInvMassReco_One_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich.push_back(fGammaInvMassReco_One_In_Rich);

	fGammaOpeningAngleMC_One_In_Rich = new TH1D("fGammaOpeningAngleMC_One_In_Rich","fGammaOpeningAngleMC_One_In_Rich;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_oneInRich.push_back(fGammaOpeningAngleMC_One_In_Rich);

	fGammaOpeningAngleReco_One_In_Rich = new TH1D("fGammaOpeningAngleReco_One_In_Rich","fGammaOpeningAngleReco_One_In_Rich;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_oneInRich.push_back(fGammaOpeningAngleReco_One_In_Rich);

	fPdg_One_In_Rich = new TH1D("fPdg_One_In_Rich","fPdg_One_In_Rich ;Id ;#", 4001, 0, 1000);
	fHistoList_oneInRich.push_back(fPdg_One_In_Rich);

	fPi0InvMassTrueKFV_One_In_Rich = new TH1D("fPi0InvMassTrueKFV_One_In_Rich","fPi0InvMassTrueKFV_One_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich.push_back(fPi0InvMassTrueKFV_One_In_Rich);

	fPi0InvMassRecoKFV_One_In_Rich = new TH1D("fPi0InvMassRecoKFV_One_In_Rich","fPi0InvMassRecoKFV_One_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich.push_back(fPi0InvMassRecoKFV_One_In_Rich);

	fdx_One_In_Rich = new TH1D("fdX_One_In_Rich","fdX_One_In_Rich ;dx [cm] ;#", 400, -10, 10);
	fHistoList_oneInRich.push_back(fdx_One_In_Rich);

	fdy_One_In_Rich = new TH1D("fdy_One_In_Rich","fdy_One_In_Rich ;dy [cm] ;#", 400, -10, 10);
	fHistoList_oneInRich.push_back(fdy_One_In_Rich);

	fdz_One_In_Rich = new TH1D("fdz_One_In_Rich","fdz_One_In_Rich ;dz [cm] ;#", 400, -10, 10);
	fHistoList_oneInRich.push_back(fdz_One_In_Rich);

	fD_One_In_Rich = new TH1D("fD_One_In_Rich","fD_One_In_Rich ;D [cm] ;#", 400, 0, 40);
	fHistoList_oneInRich.push_back(fD_One_In_Rich);

	fP_mc_One_In_Rich = new TH1D("fP_mc_One_In_Rich","fP_mc_One_In_Rich ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_oneInRich.push_back(fP_mc_One_In_Rich);

	fP_reco_One_In_Rich = new TH1D("fP_reco_One_In_Rich","fP_reco_One_In_Rich ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_oneInRich.push_back(fP_reco_One_In_Rich);

	fPt_mc_One_In_Rich = new TH1D("fPt_mc_One_In_Rich","fPt_mc_One_In_Rich ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_oneInRich.push_back(fPt_mc_One_In_Rich);

	fPt_reco_One_In_Rich = new TH1D("fPt_reco_One_In_Rich","fPt_reco_One_In_Rich ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_oneInRich.push_back(fPt_reco_One_In_Rich);

	fPi0_pt_vs_rap_One_In_Rich = new TH2D("fPi0_pt_vs_rap_One_In_Rich", "fPi0_pt_vs_rap_One_In_Rich;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);
	fHistoList_oneInRich.push_back(fPi0_pt_vs_rap_One_In_Rich);

	// 1  target
	fGammaInvMassReco_One_In_Rich_target = new TH1D("fGammaInvMassReco_One_In_Rich_target","fGammaInvMassReco_One_In_Rich_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich_target.push_back(fGammaInvMassReco_One_In_Rich_target);
	fGammaOpeningAngleReco_One_In_Rich_target = new TH1D("fGammaOpeningAngleReco_One_In_Rich_target","fGammaOpeningAngleReco_One_In_Rich_target;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_oneInRich_target.push_back(fGammaOpeningAngleReco_One_In_Rich_target);
	fPi0InvMassRecoKFV_One_In_Rich_target = new TH1D("fPi0InvMassRecoKFV_One_In_Rich_target","fPi0InvMassRecoKFV_One_In_Rich_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich_target.push_back(fPi0InvMassRecoKFV_One_In_Rich_target);
	// 1  mvd
	fGammaInvMassReco_One_In_Rich_mvd = new TH1D("fGammaInvMassReco_One_In_Rich_mvd","fGammaInvMassReco_One_In_Rich_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich_mvd.push_back(fGammaInvMassReco_One_In_Rich_mvd);
	fGammaOpeningAngleReco_One_In_Rich_mvd = new TH1D("fGammaOpeningAngleReco_One_In_Rich_mvd","fGammaOpeningAngleReco_One_In_Rich_mvd;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_oneInRich_mvd.push_back(fGammaOpeningAngleReco_One_In_Rich_mvd);
	fPi0InvMassRecoKFV_One_In_Rich_mvd = new TH1D("fPi0InvMassRecoKFV_One_In_Rich_mvd","fPi0InvMassRecoKFV_One_In_Rich_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich_mvd.push_back(fPi0InvMassRecoKFV_One_In_Rich_mvd);
	// 1  sts
	fGammaInvMassReco_One_In_Rich_sts = new TH1D("fGammaInvMassReco_One_In_Rich_sts","fGammaInvMassReco_One_In_Rich_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich_sts.push_back(fGammaInvMassReco_One_In_Rich_sts);
	fGammaOpeningAngleReco_One_In_Rich_sts = new TH1D("fGammaOpeningAngleReco_One_In_Rich_sts","fGammaOpeningAngleReco_One_In_Rich_sts;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_oneInRich_sts.push_back(fGammaOpeningAngleReco_One_In_Rich_sts);
	fPi0InvMassRecoKFV_One_In_Rich_sts = new TH1D("fPi0InvMassRecoKFV_One_In_Rich_sts","fPi0InvMassRecoKFV_One_In_Rich_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_oneInRich_sts.push_back(fPi0InvMassRecoKFV_One_In_Rich_sts);
	// 1 outside the target
	fPi0InvMassRecoKFV_One_In_Rich_outside_target = new TH1D("fPi0InvMassRecoKFV_One_In_Rich_outside_target","fPi0InvMassRecoKFV_One_In_Rich_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_one_outside.push_back(fPi0InvMassRecoKFV_One_In_Rich_outside_target);


	//2. Two particles from gamma registered in RICH like electron/positron. Both have track in STS.    **maybe cut on opening angle
	fGammaInvMassMC_Two_In_Rich = new TH1D("fGammaInvMassMC_Two_In_Rich","fGammaInvMassMC_Two_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich.push_back(fGammaInvMassMC_Two_In_Rich);

	fGammaInvMassReco_Two_In_Rich = new TH1D("fGammaInvMassReco_Two_In_Rich","fGammaInvMassReco_Two_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich.push_back(fGammaInvMassReco_Two_In_Rich);

	fGammaOpeningAngleMC_Two_In_Rich = new TH1D("fGammaOpeningAngleMC_Two_In_Rich","fGammaOpeningAngleMC_Two_In_Rich (between e+e-);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_twoInRich.push_back(fGammaOpeningAngleMC_Two_In_Rich);

	fGammaOpeningAngleReco_Two_In_Rich = new TH1D("fGammaOpeningAngleReco_Two_In_Rich","fGammaOpeningAngleReco_Two_In_Rich (between e+e-);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_twoInRich.push_back(fGammaOpeningAngleReco_Two_In_Rich);

	fPdg_Two_In_Rich = new TH1D("fPdg_Two_In_Rich","fPdg_Two_In_Rich ;Id ;#", 4001, 0, 1000);
	fHistoList_twoInRich.push_back(fPdg_Two_In_Rich);

	fPi0InvMassTrueKFV_Two_In_Rich = new TH1D("fPi0InvMassTrueKFV_Two_In_Rich","fPi0InvMassTrueKFV_Two_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich.push_back(fPi0InvMassTrueKFV_Two_In_Rich);

	fPi0InvMassRecoKFV_Two_In_Rich = new TH1D("fPi0InvMassRecoKFV_Two_In_Rich","fPi0InvMassRecoKFV_Two_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich.push_back(fPi0InvMassRecoKFV_Two_In_Rich);

	fdx_Two_In_Rich = new TH1D("fdX_Two_In_Rich","fdX_Two_In_Rich ;dx [cm] ;#", 400, -10, 10);
	fHistoList_twoInRich.push_back(fdx_Two_In_Rich);

	fdy_Two_In_Rich = new TH1D("fdy_Two_In_Rich","fdy_Two_In_Rich ;dy [cm] ;#", 400, -10, 10);
	fHistoList_twoInRich.push_back(fdy_Two_In_Rich);

	fdz_Two_In_Rich = new TH1D("fdz_Two_In_Rich","fdz_Two_In_Rich ;dz [cm] ;#", 400, -10, 10);
	fHistoList_twoInRich.push_back(fdz_Two_In_Rich);

	fD_Two_In_Rich = new TH1D("fD_Two_In_Rich","fD_Two_In_Rich ;D [cm] ;#", 400, 0, 40);
	fHistoList_twoInRich.push_back(fD_Two_In_Rich);

	fP_mc_Two_In_Rich = new TH1D("fP_mc_Two_In_Rich","fP_mc_Two_In_Rich ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_twoInRich.push_back(fP_mc_Two_In_Rich);

	fP_reco_Two_In_Rich = new TH1D("fP_reco_Two_In_Rich","fP_reco_Two_In_Rich ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_twoInRich.push_back(fP_reco_Two_In_Rich);

	fPt_mc_Two_In_Rich = new TH1D("fPt_mc_Two_In_Rich","fPt_mc_Two_In_Rich ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_twoInRich.push_back(fPt_mc_Two_In_Rich);

	fPt_reco_Two_In_Rich = new TH1D("fPt_reco_Two_In_Rich","fPt_reco_Two_In_Rich ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_twoInRich.push_back(fPt_reco_Two_In_Rich);

	fPi0_pt_vs_rap_Two_In_Rich = new TH2D("fPi0_pt_vs_rap_Two_In_Rich", "fPi0_pt_vs_rap_Two_In_Rich;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);
	fHistoList_twoInRich.push_back(fPi0_pt_vs_rap_Two_In_Rich);

	// 2  target
	fGammaInvMassReco_Two_In_Rich_target = new TH1D("fGammaInvMassReco_Two_In_Rich_target","fGammaInvMassReco_Two_In_Rich_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich_target.push_back(fGammaInvMassReco_Two_In_Rich_target);
	fGammaOpeningAngleReco_Two_In_Rich_target = new TH1D("fGammaOpeningAngleReco_Two_In_Rich_target","fGammaOpeningAngleReco_Two_In_Rich_target;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_twoInRich_target.push_back(fGammaOpeningAngleReco_Two_In_Rich_target);
	fPi0InvMassRecoKFV_Two_In_Rich_target = new TH1D("fPi0InvMassRecoKFV_Two_In_Rich_target","fPi0InvMassRecoKFV_Two_In_Rich_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich_target.push_back(fPi0InvMassRecoKFV_Two_In_Rich_target);
	// 2  mvd
	fGammaInvMassReco_Two_In_Rich_mvd = new TH1D("fGammaInvMassReco_Two_In_Rich_mvd","fGammaInvMassReco_Two_In_Rich_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich_mvd.push_back(fGammaInvMassReco_Two_In_Rich_mvd);
	fGammaOpeningAngleReco_Two_In_Rich_mvd = new TH1D("fGammaOpeningAngleReco_Two_In_Rich_mvd","fGammaOpeningAngleReco_Two_In_Rich_mvd;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_twoInRich_mvd.push_back(fGammaOpeningAngleReco_Two_In_Rich_mvd);
	fPi0InvMassRecoKFV_Two_In_Rich_mvd = new TH1D("fPi0InvMassRecoKFV_Two_In_Rich_mvd","fPi0InvMassRecoKFV_Two_In_Rich_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich_mvd.push_back(fPi0InvMassRecoKFV_Two_In_Rich_mvd);
	// 2  sts
	fGammaInvMassReco_Two_In_Rich_sts = new TH1D("fGammaInvMassReco_Two_In_Rich_sts","fGammaInvMassReco_Two_In_Rich_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich_sts.push_back(fGammaInvMassReco_Two_In_Rich_sts);
	fGammaOpeningAngleReco_Two_In_Rich_sts = new TH1D("fGammaOpeningAngleReco_Two_In_Rich_sts","fGammaOpeningAngleReco_Two_In_Rich_sts;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_twoInRich_sts.push_back(fGammaOpeningAngleReco_Two_In_Rich_sts);
	fPi0InvMassRecoKFV_Two_In_Rich_sts = new TH1D("fPi0InvMassRecoKFV_Two_In_Rich_sts","fPi0InvMassRecoKFV_Two_In_Rich_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_twoInRich_sts.push_back(fPi0InvMassRecoKFV_Two_In_Rich_sts);
	// 2 outside the target
	fPi0InvMassRecoKFV_Two_In_Rich_outside_target = new TH1D("fPi0InvMassRecoKFV_Two_In_Rich_outside_target","fPi0InvMassRecoKFV_Two_In_Rich_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_two_outside.push_back(fPi0InvMassRecoKFV_Two_In_Rich_outside_target);


	// 0 
	fGammaInvMassMC_Zero_In_Rich = new TH1D("fGammaInvMassMC_Zero_In_Rich","fGammaInvMassMC_Zero_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich.push_back(fGammaInvMassMC_Zero_In_Rich);

	fGammaInvMassReco_Zero_In_Rich = new TH1D("fGammaInvMassReco_Zero_In_Rich","fGammaInvMassReco_Zero_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich.push_back(fGammaInvMassReco_Zero_In_Rich);

	fGammaOpeningAngleMC_Zero_In_Rich = new TH1D("fGammaOpeningAngleMC_Zero_In_Rich","fGammaOpeningAngleMC_Zero_In_Rich (between e+e-);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_zeroInRich.push_back(fGammaOpeningAngleMC_Zero_In_Rich);

	fGammaOpeningAngleReco_Zero_In_Rich = new TH1D("fGammaOpeningAngleReco_Zero_In_Rich","fGammaOpeningAngleReco_Zero_In_Rich (between e+e-);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_zeroInRich.push_back(fGammaOpeningAngleReco_Zero_In_Rich);

	fPdg_Zero_In_Rich = new TH1D("fPdg_Zero_In_Rich","fPdg_Zero_In_Rich ;Id ;#", 4001, 0, 1000);
	fHistoList_zeroInRich.push_back(fPdg_Zero_In_Rich);

	fPi0InvMassTrueKFV_Zero_In_Rich = new TH1D("fPi0InvMassTrueKFV_Zero_In_Rich","fPi0InvMassTrueKFV_Zero_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich.push_back(fPi0InvMassTrueKFV_Zero_In_Rich);

	fPi0InvMassRecoKFV_Zero_In_Rich = new TH1D("fPi0InvMassRecoKFV_Zero_In_Rich","fPi0InvMassRecoKFV_Zero_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich.push_back(fPi0InvMassRecoKFV_Zero_In_Rich);

	fdx_Zero_In_Rich = new TH1D("fdX_Zero_In_Rich","fdX_Zero_In_Rich ;dx [cm] ;#", 400, -10, 10);
	fHistoList_zeroInRich.push_back(fdx_Zero_In_Rich);

	fdy_Zero_In_Rich = new TH1D("fdy_Zero_In_Rich","fdy_Zero_In_Rich ;dy [cm] ;#", 400, -10, 10);
	fHistoList_zeroInRich.push_back(fdy_Zero_In_Rich);

	fdz_Zero_In_Rich = new TH1D("fdz_Zero_In_Rich","fdz_Zero_In_Rich ;dz [cm] ;#", 400, -10, 10);
	fHistoList_zeroInRich.push_back(fdz_Zero_In_Rich);

	fD_Zero_In_Rich = new TH1D("fD_Zero_In_Rich","fD_Zero_In_Rich ;D [cm] ;#", 400, 0, 40);
	fHistoList_zeroInRich.push_back(fD_Zero_In_Rich);

	fP_mc_Zero_In_Rich = new TH1D("fP_mc_Zero_In_Rich","fP_mc_Zero_In_Rich ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_zeroInRich.push_back(fP_mc_Zero_In_Rich);

	fP_reco_Zero_In_Rich = new TH1D("fP_reco_Zero_In_Rich","fP_reco_Zero_In_Rich ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_zeroInRich.push_back(fP_reco_Zero_In_Rich);

	fPt_mc_Zero_In_Rich = new TH1D("fPt_mc_Zero_In_Rich","fPt_mc_Zero_In_Rich ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_zeroInRich.push_back(fPt_mc_Zero_In_Rich);

	fPt_reco_Zero_In_Rich = new TH1D("fPt_reco_Zero_In_Rich","fPt_reco_Zero_In_Rich ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_zeroInRich.push_back(fPt_reco_Zero_In_Rich);

	fPi0_pt_vs_rap_Zero_In_Rich = new TH2D("fPi0_pt_vs_rap_Zero_In_Rich", "fPi0_pt_vs_rap_Zero_In_Rich;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);
	fHistoList_zeroInRich.push_back(fPi0_pt_vs_rap_Zero_In_Rich);

	// 0  target
	fGammaInvMassReco_Zero_In_Rich_target = new TH1D("fGammaInvMassReco_Zero_In_Rich_target","fGammaInvMassReco_Zero_In_Rich_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich_target.push_back(fGammaInvMassReco_Zero_In_Rich_target);
	fGammaOpeningAngleReco_Zero_In_Rich_target = new TH1D("fGammaOpeningAngleReco_Zero_In_Rich_target","fGammaOpeningAngleReco_Zero_In_Rich_target;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_zeroInRich_target.push_back(fGammaOpeningAngleReco_Zero_In_Rich_target);
	fPi0InvMassRecoKFV_Zero_In_Rich_target = new TH1D("fPi0InvMassRecoKFV_Zero_In_Rich_target","fPi0InvMassRecoKFV_Zero_In_Rich_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich_target.push_back(fPi0InvMassRecoKFV_Zero_In_Rich_target);
	// 0  mvd
	fGammaInvMassReco_Zero_In_Rich_mvd = new TH1D("fGammaInvMassReco_Zero_In_Rich_mvd","fGammaInvMassReco_Zero_In_Rich_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich_mvd.push_back(fGammaInvMassReco_Zero_In_Rich_mvd);
	fGammaOpeningAngleReco_Zero_In_Rich_mvd = new TH1D("fGammaOpeningAngleReco_Zero_In_Rich_mvd","fGammaOpeningAngleReco_Zero_In_Rich_mvd;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_zeroInRich_mvd.push_back(fGammaOpeningAngleReco_Zero_In_Rich_mvd);
	fPi0InvMassRecoKFV_Zero_In_Rich_mvd = new TH1D("fPi0InvMassRecoKFV_Zero_In_Rich_mvd","fPi0InvMassRecoKFV_Zero_In_Rich_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich_mvd.push_back(fPi0InvMassRecoKFV_Zero_In_Rich_mvd);
	// 0  sts
	fGammaInvMassReco_Zero_In_Rich_sts = new TH1D("fGammaInvMassReco_Zero_In_Rich_sts","fGammaInvMassReco_Zero_In_Rich_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich_sts.push_back(fGammaInvMassReco_Zero_In_Rich_sts);
	fGammaOpeningAngleReco_Zero_In_Rich_sts = new TH1D("fGammaOpeningAngleReco_Zero_In_Rich_sts","fGammaOpeningAngleReco_Zero_In_Rich_sts;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_zeroInRich_sts.push_back(fGammaOpeningAngleReco_Zero_In_Rich_sts);
	fPi0InvMassRecoKFV_Zero_In_Rich_sts = new TH1D("fPi0InvMassRecoKFV_Zero_In_Rich_sts","fPi0InvMassRecoKFV_Zero_In_Rich_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zeroInRich_sts.push_back(fPi0InvMassRecoKFV_Zero_In_Rich_sts);
	// 0 outside the target
	fPi0InvMassRecoKFV_Zero_In_Rich_outside_target = new TH1D("fPi0InvMassRecoKFV_Zero_In_Rich_outside_target","fPi0InvMassRecoKFV_Zero_In_Rich_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_zero_outside.push_back(fPi0InvMassRecoKFV_Zero_In_Rich_outside_target);


	// 1-2 
	fGammaInvMassMC_OneTwo_In_Rich = new TH1D("fGammaInvMassMC_OneTwo_In_Rich","fGammaInvMassMC_OneTwo_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich.push_back(fGammaInvMassMC_OneTwo_In_Rich);

	fGammaInvMassReco_OneTwo_In_Rich = new TH1D("fGammaInvMassReco_OneTwo_In_Rich","fGammaInvMassReco_OneTwo_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich.push_back(fGammaInvMassReco_OneTwo_In_Rich);

	fGammaOpeningAngleMC_OneTwo_In_Rich = new TH1D("fGammaOpeningAngleMC_OneTwo_In_Rich","fGammaOpeningAngleMC_OneTwo_In_Rich (between e+e-);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_onetwoInRich.push_back(fGammaOpeningAngleMC_OneTwo_In_Rich);

	fGammaOpeningAngleReco_OneTwo_In_Rich = new TH1D("fGammaOpeningAngleReco_OneTwo_In_Rich","fGammaOpeningAngleReco_OneTwo_In_Rich (between e+e-);angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_onetwoInRich.push_back(fGammaOpeningAngleReco_OneTwo_In_Rich);

	fPdg_OneTwo_In_Rich = new TH1D("fPdg_OneTwo_In_Rich","fPdg_OneTwo_In_Rich ;Id ;#", 4001, 0, 1000);
	fHistoList_onetwoInRich.push_back(fPdg_OneTwo_In_Rich);

	fPi0InvMassTrueKFV_OneTwo_In_Rich = new TH1D("fPi0InvMassTrueKFV_OneTwo_In_Rich","fPi0InvMassTrueKFV_OneTwo_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich.push_back(fPi0InvMassTrueKFV_OneTwo_In_Rich);

	fPi0InvMassRecoKFV_OneTwo_In_Rich = new TH1D("fPi0InvMassRecoKFV_OneTwo_In_Rich","fPi0InvMassRecoKFV_OneTwo_In_Rich;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich.push_back(fPi0InvMassRecoKFV_OneTwo_In_Rich);

	fdx_OneTwo_In_Rich = new TH1D("fdX_OneTwo_In_Rich","fdX_OneTwo_In_Rich ;dx [cm] ;#", 400, -10, 10);
	fHistoList_onetwoInRich.push_back(fdx_OneTwo_In_Rich);

	fdy_OneTwo_In_Rich = new TH1D("fdy_OneTwo_In_Rich","fdy_OneTwo_In_Rich ;dy [cm] ;#", 400, -10, 10);
	fHistoList_onetwoInRich.push_back(fdy_OneTwo_In_Rich);

	fdz_OneTwo_In_Rich = new TH1D("fdz_OneTwo_In_Rich","fdz_OneTwo_In_Rich ;dz [cm] ;#", 400, -10, 10);
	fHistoList_onetwoInRich.push_back(fdz_OneTwo_In_Rich);

	fD_OneTwo_In_Rich = new TH1D("fD_OneTwo_In_Rich","fD_OneTwo_In_Rich ;D [cm] ;#", 400, 0, 40);
	fHistoList_onetwoInRich.push_back(fD_OneTwo_In_Rich);

	fP_mc_OneTwo_In_Rich = new TH1D("fP_mc_OneTwo_In_Rich","fP_mc_OneTwo_In_Rich ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_onetwoInRich.push_back(fP_mc_OneTwo_In_Rich);

	fP_reco_OneTwo_In_Rich = new TH1D("fP_reco_OneTwo_In_Rich","fP_reco_OneTwo_In_Rich ;P in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_onetwoInRich.push_back(fP_reco_OneTwo_In_Rich);

	fPt_mc_OneTwo_In_Rich = new TH1D("fPt_mc_OneTwo_In_Rich","fPt_mc_OneTwo_In_Rich ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_onetwoInRich.push_back(fPt_mc_OneTwo_In_Rich);

	fPt_reco_OneTwo_In_Rich = new TH1D("fPt_reco_OneTwo_In_Rich","fPt_reco_OneTwo_In_Rich ;Pt in GeV/c^{2} ;#", 200, 0, 10);
	fHistoList_onetwoInRich.push_back(fPt_reco_OneTwo_In_Rich);

	fPi0_pt_vs_rap_OneTwo_In_Rich = new TH2D("fPi0_pt_vs_rap_OneTwo_In_Rich", "fPi0_pt_vs_rap_OneTwo_In_Rich;pt [GeV]; rap [GeV]", 120, -2., 10., 90, -2., 7.);
	fHistoList_onetwoInRich.push_back(fPi0_pt_vs_rap_OneTwo_In_Rich);

	// 1-2  target
	fGammaInvMassReco_OneTwo_In_Rich_target = new TH1D("fGammaInvMassReco_OneTwo_In_Rich_target","fGammaInvMassReco_OneTwo_In_Rich_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich_target.push_back(fGammaInvMassReco_OneTwo_In_Rich_target);
	fGammaOpeningAngleReco_OneTwo_In_Rich_target = new TH1D("fGammaOpeningAngleReco_OneTwo_In_Rich_target","fGammaOpeningAngleReco_OneTwo_In_Rich_target;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_onetwoInRich_target.push_back(fGammaOpeningAngleReco_OneTwo_In_Rich_target);
	fPi0InvMassRecoKFV_OneTwo_In_Rich_target = new TH1D("fPi0InvMassRecoKFV_OneTwo_In_Rich_target","fPi0InvMassRecoKFV_OneTwo_In_Rich_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich_target.push_back(fPi0InvMassRecoKFV_OneTwo_In_Rich_target);
	// 1-2  mvd
	fGammaInvMassReco_OneTwo_In_Rich_mvd = new TH1D("fGammaInvMassReco_OneTwo_In_Rich_mvd","fGammaInvMassReco_OneTwo_In_Rich_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich_mvd.push_back(fGammaInvMassReco_OneTwo_In_Rich_mvd);
	fGammaOpeningAngleReco_OneTwo_In_Rich_mvd = new TH1D("fGammaOpeningAngleReco_OneTwo_In_Rich_mvd","fGammaOpeningAngleReco_OneTwo_In_Rich_mvd;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_onetwoInRich_mvd.push_back(fGammaOpeningAngleReco_OneTwo_In_Rich_mvd);
	fPi0InvMassRecoKFV_OneTwo_In_Rich_mvd = new TH1D("fPi0InvMassRecoKFV_OneTwo_In_Rich_mvd","fPi0InvMassRecoKFV_OneTwo_In_Rich_mvd;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich_mvd.push_back(fPi0InvMassRecoKFV_OneTwo_In_Rich_mvd);
	// 1-2  sts
	fGammaInvMassReco_OneTwo_In_Rich_sts = new TH1D("fGammaInvMassReco_OneTwo_In_Rich_sts","fGammaInvMassReco_OneTwo_In_Rich_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich_sts.push_back(fGammaInvMassReco_OneTwo_In_Rich_sts);
	fGammaOpeningAngleReco_OneTwo_In_Rich_sts = new TH1D("fGammaOpeningAngleReco_OneTwo_In_Rich_sts","fGammaOpeningAngleReco_OneTwo_In_Rich_sts;angle [deg];#", 1010, -0.1, 100.9);
	fHistoList_onetwoInRich_sts.push_back(fGammaOpeningAngleReco_OneTwo_In_Rich_sts);
	fPi0InvMassRecoKFV_OneTwo_In_Rich_sts = new TH1D("fPi0InvMassRecoKFV_OneTwo_In_Rich_sts","fPi0InvMassRecoKFV_OneTwo_In_Rich_sts;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwoInRich_sts.push_back(fPi0InvMassRecoKFV_OneTwo_In_Rich_sts);
	// 1-2 outside the target
	fPi0InvMassRecoKFV_OneTwo_In_Rich_outside_target = new TH1D("fPi0InvMassRecoKFV_OneTwo_In_Rich_outside_target","fPi0InvMassRecoKFV_OneTwo_In_Rich_outside_target;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_onetwo_outside.push_back(fPi0InvMassRecoKFV_OneTwo_In_Rich_outside_target);


	// Background explanation
	BG1_all = new TH1D("BG1_all","BG1_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG1_all);
	BG2_all = new TH1D("BG2_all","BG2_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG2_all);
	BG3_all = new TH1D("BG3_all","BG3_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG3_all);
	BG4_all = new TH1D("BG4_all","BG4_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG4_all);
	BG5_all = new TH1D("BG5_all","BG5_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG5_all);
	BG6_all = new TH1D("BG6_all","BG6_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG6_all);
	BG7_all = new TH1D("BG7_all","BG7_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG7_all);
	BG8_all = new TH1D("BG8_all","BG8_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG8_all);
	BG9_all = new TH1D("BG9_all","BG9_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG9_all);
	BG10_all = new TH1D("BG10_all","BG10_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(BG10_all);

	BG1_one = new TH1D("BG1_one","BG1_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG1_one);
	BG2_one = new TH1D("BG2_one","BG2_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG2_one);
	BG3_one = new TH1D("BG3_one","BG3_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG3_one);
	BG4_one = new TH1D("BG4_one","BG4_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG4_one);
	BG5_one = new TH1D("BG5_one","BG5_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG5_one);
	BG6_one = new TH1D("BG6_one","BG6_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG6_one);
	BG7_one = new TH1D("BG7_one","BG7_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG7_one);
	BG8_one = new TH1D("BG8_one","BG8_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG8_one);
	BG9_one = new TH1D("BG9_one","BG9_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG9_one);
	BG10_one = new TH1D("BG10_one","BG10_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_one.push_back(BG10_one);

	BG1_two = new TH1D("BG1_two","BG1_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG1_two);
	BG2_two = new TH1D("BG2_two","BG2_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG2_two);
	BG3_two = new TH1D("BG3_two","BG3_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG3_two);
	BG4_two = new TH1D("BG4_two","BG4_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG4_two);
	BG5_two = new TH1D("BG5_two","BG5_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG5_two);
	BG6_two = new TH1D("BG6_two","BG6_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG6_two);
	BG7_two = new TH1D("BG7_two","BG7_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG7_two);
	BG8_two = new TH1D("BG8_two","BG8_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG8_two);
	BG9_two = new TH1D("BG9_two","BG9_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG9_two);
	BG10_two = new TH1D("BG10_two","BG10_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_two.push_back(BG10_two);

	BG1_zero = new TH1D("BG1_zero","BG1_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG1_zero);
	BG2_zero = new TH1D("BG2_zero","BG2_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG2_zero);
	BG3_zero = new TH1D("BG3_zero","BG3_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG3_zero);
	BG4_zero = new TH1D("BG4_zero","BG4_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG4_zero);
	BG5_zero = new TH1D("BG5_zero","BG5_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG5_zero);
	BG6_zero = new TH1D("BG6_zero","BG6_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG6_zero);
	BG7_zero = new TH1D("BG7_zero","BG7_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG7_zero);
	BG8_zero = new TH1D("BG8_zero","BG8_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG8_zero);
	BG9_zero = new TH1D("BG9_zero","BG9_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG9_zero);
	BG10_zero = new TH1D("BG10_zero","BG10_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_zero.push_back(BG10_zero);

	BG1_onetwo = new TH1D("BG1_onetwo","BG1_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG1_onetwo);
	BG2_onetwo = new TH1D("BG2_onetwo","BG2_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG2_onetwo);
	BG3_onetwo = new TH1D("BG3_onetwo","BG3_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG3_onetwo);
	BG4_onetwo = new TH1D("BG4_onetwo","BG4_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG4_onetwo);
	BG5_onetwo = new TH1D("BG5_onetwo","BG5_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG5_onetwo);
	BG6_onetwo = new TH1D("BG6_onetwo","BG6_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG6_onetwo);
	BG7_onetwo = new TH1D("BG7_onetwo","BG7_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG7_onetwo);
	BG8_onetwo = new TH1D("BG8_onetwo","BG8_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG8_onetwo);
	BG9_onetwo = new TH1D("BG9_onetwo","BG9_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG9_onetwo);
	BG10_onetwo = new TH1D("BG10_onetwo","BG10_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_onetwo.push_back(BG10_onetwo);

	PdgCase8_all = new TH1D("PdgCase8_all","PdgCase8_all ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_all.push_back(PdgCase8_all);
	PdgCase8_one = new TH1D("PdgCase8_one","PdgCase8_one ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_one.push_back(PdgCase8_one);
	PdgCase8_two = new TH1D("PdgCase8_two","PdgCase8_two ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_two.push_back(PdgCase8_two);
	PdgCase8_zero = new TH1D("PdgCase8_zero","PdgCase8_zero ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_zero.push_back(PdgCase8_zero);
	PdgCase8_onetwo = new TH1D("PdgCase8_onetwo","PdgCase8_onetwo ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_onetwo.push_back(PdgCase8_onetwo);

	PdgCase8mothers_all = new TH1D("PdgCase8mothers_all","PdgCase8mothers_all ;Id ;#", 5000, -2500, 2500);
	PdgCase8mothers_one = new TH1D("PdgCase8mothers_one","PdgCase8mothers_one ;Id ;#", 5000, -2500, 2500);
	PdgCase8mothers_two = new TH1D("PdgCase8mothers_two","PdgCase8mothers_two ;Id ;#", 5000, -2500, 2500);
	PdgCase8mothers_zero = new TH1D("PdgCase8mothers_zero","PdgCase8mothers_zero ;Id ;#", 5000, -2500, 2500);
	PdgCase8mothers_onetwo = new TH1D("PdgCase8mothers_onetwo","PdgCase8mothers_onetwo ;Id ;#", 5000, -2500, 2500);
	fHistoList_bg_all.push_back(PdgCase8mothers_all);
	fHistoList_bg_one.push_back(PdgCase8mothers_one);
	fHistoList_bg_two.push_back(PdgCase8mothers_two);
	fHistoList_bg_zero.push_back(PdgCase8mothers_zero);
	fHistoList_bg_onetwo.push_back(PdgCase8mothers_onetwo);

	testsameMIDcase8_all = new TH1D("testsameMIDcase8_all","testsameMIDcase8_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_all = new TH1D("testsameGRIDcase8_all","testsameGRIDcase8_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	testsameMIDcase8_one = new TH1D("testsameMIDcase8_one","testsameMIDcase8_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_one = new TH1D("testsameGRIDcase8_one","testsameGRIDcase8_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	testsameMIDcase8_two = new TH1D("testsameMIDcase8_two","testsameMIDcase8_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_two = new TH1D("testsameGRIDcase8_two","testsameGRIDcase8_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	testsameMIDcase8_zero = new TH1D("testsameMIDcase8_zero","testsameMIDcase8_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_zero = new TH1D("testsameGRIDcase8_zero","testsameGRIDcase8_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	testsameMIDcase8_onetwo = new TH1D("testsameMIDcase8_onetwo","testsameMIDcase8_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	testsameGRIDcase8_onetwo = new TH1D("testsameGRIDcase8_onetwo","testsameGRIDcase8_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_bg_all.push_back(testsameMIDcase8_all);
	fHistoList_bg_all.push_back(testsameGRIDcase8_all);
	fHistoList_bg_one.push_back(testsameMIDcase8_one);
	fHistoList_bg_one.push_back(testsameGRIDcase8_one);
	fHistoList_bg_two.push_back(testsameMIDcase8_two);
	fHistoList_bg_two.push_back(testsameGRIDcase8_two);
	fHistoList_bg_zero.push_back(testsameMIDcase8_zero);
	fHistoList_bg_zero.push_back(testsameGRIDcase8_zero);
	fHistoList_bg_onetwo.push_back(testsameMIDcase8_onetwo);
	fHistoList_bg_onetwo.push_back(testsameGRIDcase8_onetwo);

	case8GRIDInvMassGamma_all = new TH1D("case8GRIDInvMassGamma_all","case8GRIDInvMassGamma_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_all = new TH1D("case8GRIDOAGamma_all","case8GRIDOAGamma_all;angle [deg];#", 1010, -0.1, 100.9); 
	case8GRIDInvMassGamma_one = new TH1D("case8GRIDInvMassGamma_one","case8GRIDInvMassGamma_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_one = new TH1D("case8GRIDOAGamma_one","case8GRIDOAGamma_one;angle [deg];#", 1010, -0.1, 100.9); 
	case8GRIDInvMassGamma_two = new TH1D("case8GRIDInvMassGamma_two","case8GRIDInvMassGamma_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_two = new TH1D("case8GRIDOAGamma_two","case8GRIDOAGamma_two;angle [deg];#", 1010, -0.1, 100.9); 
	case8GRIDInvMassGamma_zero = new TH1D("case8GRIDInvMassGamma_zero","case8GRIDInvMassGamma_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_zero = new TH1D("case8GRIDOAGamma_zero","case8GRIDOAGamma_zero;angle [deg];#", 1010, -0.1, 100.9); 
	case8GRIDInvMassGamma_onetwo = new TH1D("case8GRIDInvMassGamma_onetwo","case8GRIDInvMassGamma_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0); 
	case8GRIDOAGamma_onetwo = new TH1D("case8GRIDOAGamma_onetwo","case8GRIDOAGamma_onetwo;angle [deg];#", 1010, -0.1, 100.9); 

	fHistoList_bg_all.push_back(case8GRIDInvMassGamma_all);
	fHistoList_bg_all.push_back(case8GRIDOAGamma_all);
	fHistoList_bg_one.push_back(case8GRIDInvMassGamma_one);
	fHistoList_bg_one.push_back(case8GRIDOAGamma_one);
	fHistoList_bg_two.push_back(case8GRIDInvMassGamma_two);
	fHistoList_bg_two.push_back(case8GRIDOAGamma_two);
	fHistoList_bg_zero.push_back(case8GRIDInvMassGamma_zero);
	fHistoList_bg_zero.push_back(case8GRIDOAGamma_zero);
	fHistoList_bg_onetwo.push_back(case8GRIDInvMassGamma_onetwo);
	fHistoList_bg_onetwo.push_back(case8GRIDOAGamma_onetwo);


	// Separate by Pt
	Pt1_all = new TH1D("Pt1_all","Pt1_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt1_all);
	Pt2_all = new TH1D("Pt2_all","Pt2_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt2_all);
	Pt3_all = new TH1D("Pt3_all","Pt3_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt3_all);
	Pt4_all = new TH1D("Pt4_all","Pt4_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt4_all);
	Pt5_all = new TH1D("Pt5_all","Pt5_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt5_all);
	Pt6_all = new TH1D("Pt6_all","Pt6_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt6_all);
	Pt7_all = new TH1D("Pt7_all","Pt7_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt7_all);
	Pt8_all = new TH1D("Pt8_all","Pt8_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt8_all);
	Pt9_all = new TH1D("Pt9_all","Pt9_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt9_all);
	Pt10_all = new TH1D("Pt10_all","Pt10_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt10_all);
	Pt11_all = new TH1D("Pt11_all","Pt11_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt11_all);
	Pt12_all = new TH1D("Pt12_all","Pt12_all;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_all.push_back(Pt12_all);

	Pt1_one = new TH1D("Pt1_one","Pt1_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt1_one);
	Pt2_one = new TH1D("Pt2_one","Pt2_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt2_one);
	Pt3_one = new TH1D("Pt3_one","Pt3_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt3_one);
	Pt4_one = new TH1D("Pt4_one","Pt4_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt4_one);
	Pt5_one = new TH1D("Pt5_one","Pt5_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt5_one);
	Pt6_one = new TH1D("Pt6_one","Pt6_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt6_one);
	Pt7_one = new TH1D("Pt7_one","Pt7_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt7_one);
	Pt8_one = new TH1D("Pt8_one","Pt8_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt8_one);
	Pt9_one = new TH1D("Pt9_one","Pt9_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt9_one);
	Pt10_one = new TH1D("Pt10_one","Pt10_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt10_one);
	Pt11_one = new TH1D("Pt11_one","Pt11_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt11_one);
	Pt12_one = new TH1D("Pt12_one","Pt12_one;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_one.push_back(Pt12_one);

	Pt1_two = new TH1D("Pt1_two","Pt1_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt1_two);
	Pt2_two = new TH1D("Pt2_two","Pt2_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt2_two);
	Pt3_two = new TH1D("Pt3_two","Pt3_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt3_two);
	Pt4_two = new TH1D("Pt4_two","Pt4_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt4_two);
	Pt5_two = new TH1D("Pt5_two","Pt5_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt5_two);
	Pt6_two = new TH1D("Pt6_two","Pt6_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt6_two);
	Pt7_two = new TH1D("Pt7_two","Pt7_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt7_two);
	Pt8_two = new TH1D("Pt8_two","Pt8_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt8_two);
	Pt9_two = new TH1D("Pt9_two","Pt9_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt9_two);
	Pt10_two = new TH1D("Pt10_two","Pt10_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt10_two);
	Pt11_two = new TH1D("Pt11_two","Pt11_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt11_two);
	Pt12_two = new TH1D("Pt12_two","Pt12_two;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_two.push_back(Pt12_two);

	Pt1_zero = new TH1D("Pt1_zero","Pt1_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt1_zero);
	Pt2_zero = new TH1D("Pt2_zero","Pt2_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt2_zero);
	Pt3_zero = new TH1D("Pt3_zero","Pt3_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt3_zero);
	Pt4_zero = new TH1D("Pt4_zero","Pt4_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt4_zero);
	Pt5_zero = new TH1D("Pt5_zero","Pt5_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt5_zero);
	Pt6_zero = new TH1D("Pt6_zero","Pt6_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt6_zero);
	Pt7_zero = new TH1D("Pt7_zero","Pt7_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt7_zero);
	Pt8_zero = new TH1D("Pt8_zero","Pt8_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt8_zero);
	Pt9_zero = new TH1D("Pt9_zero","Pt9_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt9_zero);
	Pt10_zero = new TH1D("Pt10_zero","Pt10_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt10_zero);
	Pt11_zero = new TH1D("Pt11_zero","Pt11_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt11_zero);
	Pt12_zero = new TH1D("Pt12_zero","Pt12_zero;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_zero.push_back(Pt12_zero);

	Pt1_onetwo = new TH1D("Pt1_onetwo","Pt1_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt1_onetwo);
	Pt2_onetwo = new TH1D("Pt2_onetwo","Pt2_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt2_onetwo);
	Pt3_onetwo = new TH1D("Pt3_onetwo","Pt3_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt3_onetwo);
	Pt4_onetwo = new TH1D("Pt4_onetwo","Pt4_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt4_onetwo);
	Pt5_onetwo = new TH1D("Pt5_onetwo","Pt5_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt5_onetwo);
	Pt6_onetwo = new TH1D("Pt6_onetwo","Pt6_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt6_onetwo);
	Pt7_onetwo = new TH1D("Pt7_onetwo","Pt7_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt7_onetwo);
	Pt8_onetwo = new TH1D("Pt8_onetwo","Pt8_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt8_onetwo);
	Pt9_onetwo = new TH1D("Pt9_onetwo","Pt9_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt9_onetwo);
	Pt10_onetwo = new TH1D("Pt10_onetwo","Pt10_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt10_onetwo);
	Pt11_onetwo = new TH1D("Pt11_onetwo","Pt11_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt11_onetwo);
	Pt12_onetwo = new TH1D("Pt12_onetwo","Pt12_onetwo;invariant mass in GeV/c^{2};#", 1000, 0, 3.0);
	fHistoList_pt_onetwo.push_back(Pt12_onetwo);

	fHistoList_pt.push_back(fPi0InvMassRecoKFV_All); 
	fHistoList_pt.push_back(fPi0InvMassRecoKFV_One_In_Rich);
	fHistoList_pt.push_back(fPi0InvMassRecoKFV_Two_In_Rich);
	fHistoList_pt.push_back(fPi0InvMassRecoKFV_Zero_In_Rich);
	fHistoList_pt.push_back(fPi0InvMassRecoKFV_OneTwo_In_Rich);


	Case1ZYPos_all = new TH2D("Case1ZYPos_all","Case1ZYPos_all ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_all.push_back(Case1ZYPos_all);
	Case1ZYPos_one = new TH2D("Case1ZYPos_one","Case1ZYPos_one ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_one.push_back(Case1ZYPos_one);
	Case1ZYPos_two = new TH2D("Case1ZYPos_two","Case1ZYPos_two ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_two.push_back(Case1ZYPos_two);
	Case1ZYPos_zero = new TH2D("Case1ZYPos_zero","Case1ZYPos_zero ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_zero.push_back(Case1ZYPos_zero);
	Case1ZYPos_onetwo = new TH2D("Case1ZYPos_onetwo","Case1ZYPos_onetwo ; z[cm]; y[cm]", 400, -1, 200, 200, -50, 50);
	fHistoList_bg_onetwo.push_back(Case1ZYPos_onetwo);


	cout << " CbmAnaConversion2KF::InitHistos() " << endl;

}







void CbmAnaConversion2KF::Finish()
{
	gDirectory->mkdir("KF");
	gDirectory->cd("KF");



	gDirectory->mkdir("MC");
	gDirectory->cd("MC");
	for (UInt_t i = 0; i < fHistoList_MC.size(); i++){
		fHistoList_MC[i]->Write();
	}
	gDirectory->cd("..");



	gDirectory->mkdir("CheckCuts");
	gDirectory->cd("CheckCuts");
	for (UInt_t i = 0; i < fHistoList_CheckForCuts.size(); i++){
		fHistoList_CheckForCuts[i]->Write();
	}
	gDirectory->cd("..");



	gDirectory->mkdir("EventMixing");
	gDirectory->cd("EventMixing");
	for (UInt_t i = 0; i < fHistoList_mixing.size(); i++){
		fHistoList_mixing[i]->Write();
	}
	gDirectory->cd("..");

	gDirectory->mkdir("multiplicity");
	gDirectory->cd("multiplicity");
	gDirectory->mkdir("all");
	gDirectory->cd("all");
	for (UInt_t i = 0; i < fHistoList_multiplicity_all.size(); i++){
		fHistoList_multiplicity_all[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("one");
	gDirectory->cd("one");
	for (UInt_t i = 0; i < fHistoList_multiplicity_one.size(); i++){
		fHistoList_multiplicity_one[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("two");
	gDirectory->cd("two");
	for (UInt_t i = 0; i < fHistoList_multiplicity_two.size(); i++){
		fHistoList_multiplicity_two[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("zero");
	gDirectory->cd("zero");
	for (UInt_t i = 0; i < fHistoList_multiplicity_zero.size(); i++){
		fHistoList_multiplicity_zero[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("onetwo");
	gDirectory->cd("onetwo");
	for (UInt_t i = 0; i < fHistoList_multiplicity_onetwo.size(); i++){
		fHistoList_multiplicity_onetwo[i]->Write();
	}
	gDirectory->cd("..");
	for (UInt_t i = 0; i < fHistoList_multiplicity.size(); i++){
		fHistoList_multiplicity[i]->Write();
	}
	gDirectory->cd("..");


	gDirectory->mkdir("BG");
	gDirectory->cd("BG");
	gDirectory->mkdir("BG_all");
	gDirectory->cd("BG_all");
	for (UInt_t i = 0; i < fHistoList_bg_all.size(); i++){
		fHistoList_bg_all[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("BG_one");
	gDirectory->cd("BG_one");
	for (UInt_t i = 0; i < fHistoList_bg_one.size(); i++){
		fHistoList_bg_one[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("BG_two");
	gDirectory->cd("BG_two");
	for (UInt_t i = 0; i < fHistoList_bg_two.size(); i++){
		fHistoList_bg_two[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("BG_zero");
	gDirectory->cd("BG_zero");
	for (UInt_t i = 0; i < fHistoList_bg_zero.size(); i++){
		fHistoList_bg_zero[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("BG_onetwo");
	gDirectory->cd("BG_onetwo");
	for (UInt_t i = 0; i < fHistoList_bg_onetwo.size(); i++){
		fHistoList_bg_onetwo[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->cd("..");

	gDirectory->mkdir("Pt");
	gDirectory->cd("Pt");
	gDirectory->mkdir("Pt_all");
	gDirectory->cd("Pt_all");
	for (UInt_t i = 0; i < fHistoList_pt_all.size(); i++){
		fHistoList_pt_all[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("Pt_one");
	gDirectory->cd("Pt_one");
	for (UInt_t i = 0; i < fHistoList_pt_one.size(); i++){
		fHistoList_pt_one[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("Pt_two");
	gDirectory->cd("Pt_two");
	for (UInt_t i = 0; i < fHistoList_pt_two.size(); i++){
		fHistoList_pt_two[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("Pt_zero");
	gDirectory->cd("Pt_zero");
	for (UInt_t i = 0; i < fHistoList_pt_zero.size(); i++){
		fHistoList_pt_zero[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("Pt_onetwo");
	gDirectory->cd("Pt_onetwo");
	for (UInt_t i = 0; i < fHistoList_pt_onetwo.size(); i++){
		fHistoList_pt_onetwo[i]->Write();
	}
	gDirectory->cd("..");
	for (UInt_t i = 0; i < fHistoList_pt.size(); i++){
		fHistoList_pt[i]->Write();
	}
	gDirectory->cd("..");


	gDirectory->mkdir("g -> All");
	gDirectory->cd("g -> All");
	gDirectory->mkdir("target(<4.5cm)");
	gDirectory->cd("target(<4.5cm)");
	for (UInt_t i = 0; i < fHistoList_all_target.size(); i++){
		fHistoList_all_target[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("MVD(4.5cm-20cm)");
	gDirectory->cd("MVD(4.5cm-20cm)");
	for (UInt_t i = 0; i < fHistoList_all_mvd.size(); i++){
		fHistoList_all_mvd[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("STS(20cm-70cm)");
	gDirectory->cd("STS(20cm-70cm)");
	for (UInt_t i = 0; i < fHistoList_all_sts.size(); i++){
		fHistoList_all_sts[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("outside the target(>4.5cm)");
	gDirectory->cd("outside the target(>4.5cm)");
	for (UInt_t i = 0; i < fHistoList_all_outside.size(); i++){
		fHistoList_all_outside[i]->Write();
	}
	gDirectory->cd("..");
	for (UInt_t i = 0; i < fHistoList_all.size(); i++){
		fHistoList_all[i]->Write();
	}
	gDirectory->cd("..");



	gDirectory->mkdir("g -> One");
	gDirectory->cd("g -> One");
	gDirectory->mkdir("target(<4.5cm)");
	gDirectory->cd("target(<4.5cm)");
	for (UInt_t i = 0; i < fHistoList_oneInRich_target.size(); i++){
		fHistoList_oneInRich_target[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("MVD(4.5cm-20cm)");
	gDirectory->cd("MVD(4.5cm-20cm)");
	for (UInt_t i = 0; i < fHistoList_oneInRich_mvd.size(); i++){
		fHistoList_oneInRich_mvd[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("STS(20cm-70cm)");
	gDirectory->cd("STS(20cm-70cm)");
	for (UInt_t i = 0; i < fHistoList_oneInRich_sts.size(); i++){
		fHistoList_oneInRich_sts[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("outside the target(>4.5cm)");
	gDirectory->cd("outside the target(>4.5cm)");
	for (UInt_t i = 0; i < fHistoList_one_outside.size(); i++){
		fHistoList_one_outside[i]->Write();
	}
	gDirectory->cd("..");
	for (UInt_t i = 0; i < fHistoList_oneInRich.size(); i++){
		fHistoList_oneInRich[i]->Write();
	}
	gDirectory->cd("..");



	gDirectory->mkdir("g -> Two");
	gDirectory->cd("g -> Two");
	gDirectory->mkdir("target(<4.5cm)");
	gDirectory->cd("target(<4.5cm)");
	for (UInt_t i = 0; i < fHistoList_twoInRich_target.size(); i++){
		fHistoList_twoInRich_target[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("MVD(4.5cm-20cm)");
	gDirectory->cd("MVD(4.5cm-20cm)");
	for (UInt_t i = 0; i < fHistoList_twoInRich_mvd.size(); i++){
		fHistoList_twoInRich_mvd[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("STS(20cm-70cm)");
	gDirectory->cd("STS(20cm-70cm)");
	for (UInt_t i = 0; i < fHistoList_twoInRich_sts.size(); i++){
		fHistoList_twoInRich_sts[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("outside the target(>4.5cm)");
	gDirectory->cd("outside the target(>4.5cm)");
	for (UInt_t i = 0; i < fHistoList_two_outside.size(); i++){
		fHistoList_two_outside[i]->Write();
	}
	gDirectory->cd("..");
	for (UInt_t i = 0; i < fHistoList_twoInRich.size(); i++){
		fHistoList_twoInRich[i]->Write();
	}
	gDirectory->cd("..");



	gDirectory->mkdir("g -> Zero");
	gDirectory->cd("g -> Zero");
	gDirectory->mkdir("target(<4.5cm)");
	gDirectory->cd("target(<4.5cm)");
	for (UInt_t i = 0; i < fHistoList_zeroInRich_target.size(); i++){
		fHistoList_zeroInRich_target[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("MVD(4.5cm-20cm)");
	gDirectory->cd("MVD(4.5cm-20cm)");
	for (UInt_t i = 0; i < fHistoList_zeroInRich_mvd.size(); i++){
		fHistoList_zeroInRich_mvd[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("STS(20cm-70cm)");
	gDirectory->cd("STS(20cm-70cm)");
	for (UInt_t i = 0; i < fHistoList_zeroInRich_sts.size(); i++){
		fHistoList_zeroInRich_sts[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("outside the target(>4.5cm)");
	gDirectory->cd("outside the target(>4.5cm)");
	for (UInt_t i = 0; i < fHistoList_zero_outside.size(); i++){
		fHistoList_zero_outside[i]->Write();
	}
	gDirectory->cd("..");
	for (UInt_t i = 0; i < fHistoList_zeroInRich.size(); i++){
		fHistoList_zeroInRich[i]->Write();
	}
	gDirectory->cd("..");



	gDirectory->mkdir("g -> OneTwo");
	gDirectory->cd("g -> OneTwo");
	gDirectory->mkdir("target(<4.5cm)");
	gDirectory->cd("target(<4.5cm)");
	for (UInt_t i = 0; i < fHistoList_onetwoInRich_target.size(); i++){
		fHistoList_onetwoInRich_target[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("MVD(4.5cm-20cm)");
	gDirectory->cd("MVD(4.5cm-20cm)");
	for (UInt_t i = 0; i < fHistoList_onetwoInRich_mvd.size(); i++){
		fHistoList_onetwoInRich_mvd[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("STS(20cm-70cm)");
	gDirectory->cd("STS(20cm-70cm)");
	for (UInt_t i = 0; i < fHistoList_onetwoInRich_sts.size(); i++){
		fHistoList_onetwoInRich_sts[i]->Write();
	}
	gDirectory->cd("..");
	gDirectory->mkdir("outside the target(>4.5cm)");
	gDirectory->cd("outside the target(>4.5cm)");
	for (UInt_t i = 0; i < fHistoList_onetwo_outside.size(); i++){
		fHistoList_onetwo_outside[i]->Write();
	}
	gDirectory->cd("..");
	for (UInt_t i = 0; i < fHistoList_onetwoInRich.size(); i++){
		fHistoList_onetwoInRich[i]->Write();
	}
	gDirectory->cd("..");




	for (UInt_t i = 0; i < fHistoList_kfparticle.size(); i++){
		fHistoList_kfparticle[i]->Write();
	}
	gDirectory->cd("..");

//	cout << "CbmAnaConversion2KF: Realtime - " << fTime << endl;
}




void CbmAnaConversion2KF::Exec(int fEventNum, double OpeningAngleCut, double GammaInvMassCut)
{
	int Event = fEventNum;
	double AngleCut = OpeningAngleCut;
	double InvMassCut = GammaInvMassCut;

	timer.Start();
//	cout << " " << endl;
	cout << "CbmAnaConversion2KF::Exec()   Event: " << Event << endl;
//	cout << " num of global tracks = " << fGlobalTracks->GetEntriesFast() << endl;

	test(Event, AngleCut, InvMassCut);

	timer.Stop();
	fTime += timer.RealTime();

//	cout << " " << endl;
}


void CbmAnaConversion2KF::test(int Event, double AngleCut, double InvMassCut) 
{


	DoMC();

	int nofparticles = fKFMcParticles->GetEntriesFast();
	for(int i=0; i<nofparticles; i++) {
		KFMCParticle* mcTrack1 = (KFMCParticle*) fKFMcParticles->At(i);
		CbmMCTrack* mctrack = (CbmMCTrack*)fMcTracks->At(i);
		if (mcTrack1 == nullptr) continue;
		if (mctrack == nullptr) continue;
		int pdg = mcTrack1->GetPDG();
		int motherId = mcTrack1->GetMotherId();
	}

	vector<KFParticle> particlevector;
	particlevector = fKFtopo->GetParticles();


	vector<KFParticle> allgammas;
	allgammas.clear();
	for (int vv=0; vv<particlevector.size(); vv++){
		if (particlevector[vv].KFParticleBase::GetPDG() == 22){ // particle is gamma
		if (particlevector[vv].KFParticleBase::NDaughters() != 2 ) continue; // check - if gamma has two particles
			allgammas.push_back(particlevector[vv]);
		}
	}
	
   

	GammasAll.clear();
	GammasOneLeptonInRICH.clear();
	GammasTwoLeptonInRICH.clear();
	GammasZeroLeptonInRICH.clear();
	GammasOneTwoLeptonInRICH.clear();

	FindGammas(allgammas, particlevector, AngleCut, Event, InvMassCut);

	FindPi0(GammasAll, particlevector, fPi0InvMassRecoKFV_All, fPi0InvMassTrueKFV_All, fPi0_pt_vs_rap_All, fPi0InvMassRecoKFV_All_target, fPi0InvMassRecoKFV_All_mvd, fPi0InvMassRecoKFV_All_sts, BG1_all, BG2_all, BG3_all, BG4_all, BG5_all, BG6_all, BG7_all, BG8_all, BG9_all, BG10_all, MultiplicityInv_all, Multiplicity_chargedParticles_Inv_all, PdgCase8_all, fPi0InvMassRecoKFV_All_In_Rich_outside_target,  testsameMIDcase8_all, testsameGRIDcase8_all, PdgCase8mothers_all, case8GRIDInvMassGamma_all, case8GRIDOAGamma_all, fHistoList_pt_all, Case1ZYPos_all);

	FindPi0(GammasOneLeptonInRICH, particlevector, fPi0InvMassRecoKFV_One_In_Rich, fPi0InvMassTrueKFV_One_In_Rich, fPi0_pt_vs_rap_One_In_Rich, fPi0InvMassRecoKFV_One_In_Rich_target, fPi0InvMassRecoKFV_One_In_Rich_mvd, fPi0InvMassRecoKFV_One_In_Rich_sts, BG1_one, BG2_one, BG3_one, BG4_one, BG5_one, BG6_one, BG7_one, BG8_one, BG9_one, BG10_one, MultiplicityInv_one, Multiplicity_chargedParticles_Inv_one, PdgCase8_one, fPi0InvMassRecoKFV_One_In_Rich_outside_target, testsameMIDcase8_one, testsameGRIDcase8_one, PdgCase8mothers_one, case8GRIDInvMassGamma_one, case8GRIDOAGamma_one, fHistoList_pt_one, Case1ZYPos_one);

	FindPi0(GammasTwoLeptonInRICH, particlevector, fPi0InvMassRecoKFV_Two_In_Rich, fPi0InvMassTrueKFV_Two_In_Rich, fPi0_pt_vs_rap_Two_In_Rich, fPi0InvMassRecoKFV_Two_In_Rich_target, fPi0InvMassRecoKFV_Two_In_Rich_mvd, fPi0InvMassRecoKFV_Two_In_Rich_sts, BG1_two, BG2_two, BG3_two, BG4_two, BG5_two, BG6_two, BG7_two, BG8_two, BG9_two, BG10_two, MultiplicityInv_two, Multiplicity_chargedParticles_Inv_two, PdgCase8_two, fPi0InvMassRecoKFV_Two_In_Rich_outside_target, testsameMIDcase8_two, testsameGRIDcase8_two, PdgCase8mothers_two, case8GRIDInvMassGamma_two, case8GRIDOAGamma_two, fHistoList_pt_two, Case1ZYPos_two);

	FindPi0(GammasZeroLeptonInRICH, particlevector, fPi0InvMassRecoKFV_Zero_In_Rich, fPi0InvMassTrueKFV_Zero_In_Rich, fPi0_pt_vs_rap_Zero_In_Rich, fPi0InvMassRecoKFV_Zero_In_Rich_target, fPi0InvMassRecoKFV_Zero_In_Rich_mvd, fPi0InvMassRecoKFV_Zero_In_Rich_sts, BG1_zero, BG2_zero, BG3_zero, BG4_zero, BG5_zero, BG6_zero, BG7_zero, BG8_zero, BG9_zero, BG10_zero, MultiplicityInv_zero, Multiplicity_chargedParticles_Inv_zero, PdgCase8_zero, fPi0InvMassRecoKFV_Zero_In_Rich_outside_target, testsameMIDcase8_zero, testsameGRIDcase8_zero, PdgCase8mothers_zero, case8GRIDInvMassGamma_zero, case8GRIDOAGamma_zero, fHistoList_pt_zero, Case1ZYPos_zero);

	FindPi0(GammasOneTwoLeptonInRICH, particlevector, fPi0InvMassRecoKFV_OneTwo_In_Rich, fPi0InvMassTrueKFV_OneTwo_In_Rich, fPi0_pt_vs_rap_OneTwo_In_Rich, fPi0InvMassRecoKFV_OneTwo_In_Rich_target, fPi0InvMassRecoKFV_OneTwo_In_Rich_mvd, fPi0InvMassRecoKFV_OneTwo_In_Rich_sts, BG1_onetwo, BG2_onetwo, BG3_onetwo, BG4_onetwo, BG5_onetwo, BG6_onetwo, BG7_onetwo, BG8_onetwo, BG9_onetwo, BG10_onetwo, MultiplicityInv_onetwo, Multiplicity_chargedParticles_Inv_onetwo, PdgCase8_onetwo, fPi0InvMassRecoKFV_OneTwo_In_Rich_outside_target, testsameMIDcase8_onetwo, testsameGRIDcase8_onetwo, PdgCase8mothers_onetwo, case8GRIDInvMassGamma_onetwo, case8GRIDOAGamma_onetwo, fHistoList_pt_onetwo, Case1ZYPos_onetwo);


	if(Event%499 == 0) {
		MixedEvent(AngleCut);
		EMT_Event.clear();
		EMT_pair_momenta.clear();
		EMT_OA.clear();
		EMT_InvMass.clear();
		EMT_NofRings.clear();  
		EMT_Z.clear();  
	}

	if(Event%999 == 0) {
		MixedEventMulti();
		EMT_Event_multi_all.clear();
		EMT_pair_momenta_multi_all.clear();
		EMT_OA_multi_all.clear();
		EMT_InvMass_multi_all.clear();
		EMT_Event_multi_one.clear();
		EMT_pair_momenta_multi_one.clear();
		EMT_OA_multi_one.clear();
		EMT_InvMass_multi_one.clear();
		EMT_Event_multi_two.clear();
		EMT_pair_momenta_multi_two.clear();
		EMT_OA_multi_two.clear();
		EMT_InvMass_multi_two.clear();
		EMT_Event_multi_zero.clear();
		EMT_pair_momenta_multi_zero.clear();
		EMT_OA_multi_zero.clear();
		EMT_InvMass_multi_zero.clear();
		EMT_Event_multi_onetwo.clear();
		EMT_pair_momenta_multi_onetwo.clear();
		EMT_OA_multi_onetwo.clear();
		EMT_InvMass_multi_onetwo.clear();
		EMT_multi_all.clear(); 
		EMT_multi_one.clear(); 
		EMT_multi_two.clear(); 
		EMT_multi_zero.clear(); 
		EMT_multi_onetwo.clear(); 
	}

	// pi0, which maksym accept
	cout << endl;
	vector<vector<KFParticle>> primpi0;
	primpi0 = fKFtopo->GetKFParticleFinder()->GetPrimaryPi0();
	vector<KFParticle> primpi0inside;
	primpi0inside = primpi0.at(0);
	for (int tt=0; tt<primpi0inside.size(); tt++){
		cout << "\t ***********primpi0->GetPt = " << primpi0inside[tt].GetPt() << "; ->GetMass = " << primpi0inside[tt].GetMass() << "; ->GetX = " << primpi0inside[tt].GetX() 
		<< "; ->GetY = " << primpi0inside[tt].GetY() << "; ->GetZ = " << primpi0inside[tt].GetZ() << "; ->GetE = " << primpi0inside[tt].GetE() << endl;
	}

}


void CbmAnaConversion2KF::FindGammas(vector<KFParticle> allgammas, vector<KFParticle> particlevector, double AngleCut, int Event, double InvMassCut)
{
	for (int tt=0; tt<allgammas.size(); tt++){
		frefmomentum.clear();
		fmcvector.clear();
		std::vector<int> electronIds = allgammas[tt].KFParticleBase::DaughterIds();
		std::vector<int> grDaughter0 = particlevector[electronIds.at(0)].KFParticleBase::DaughterIds();
		std::vector<int> grDaughter1 = particlevector[electronIds.at(1)].KFParticleBase::DaughterIds();
		if(grDaughter0.size() != 1 || grDaughter1.size() != 1) continue; // check that it made only two particles   --> use STS track

		// STS ind
		CbmStsTrack* stsTrack0 = (CbmStsTrack*) fStsTracks->At(grDaughter0.at(0));
		CbmStsTrack* stsTrack1 = (CbmStsTrack*) fStsTracks->At(grDaughter1.at(0));
		if (stsTrack0 == nullptr || stsTrack1 == nullptr) continue;
		CbmTrackMatchNew* stsMatch0 = (CbmTrackMatchNew*) fStsTrackMatches->At(grDaughter0.at(0));
		CbmTrackMatchNew* stsMatch1 = (CbmTrackMatchNew*) fStsTrackMatches->At(grDaughter1.at(0));
		if (stsMatch0 == nullptr || stsMatch1 == nullptr) continue;
		if (stsMatch0->GetNofLinks() <= 0 || stsMatch1->GetNofLinks() <= 0) continue;
		int stsMcTrackId0 = stsMatch0->GetMatchedLink().GetIndex();
		int stsMcTrackId1 = stsMatch1->GetMatchedLink().GetIndex();
		if (stsMcTrackId0 < 0 || stsMcTrackId1 < 0) continue;
		CbmMCTrack* mcTrack0 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId0);
		CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId1);
		if (mcTrack0 == nullptr || mcTrack1 == nullptr) continue;


		// RICH ind 
		Int_t ngTracks = fGlobalTracks->GetEntriesFast();
		int richInd0 = 99999;
		int richInd1 = 99999;
		for (Int_t iTr = 0; iTr < ngTracks; iTr++) {
			CbmGlobalTrack* gTrack = (CbmGlobalTrack*) fGlobalTracks->At(iTr);
			if(nullptr == gTrack) continue;
			int stsInd = gTrack->GetStsTrackIndex();
			if (stsInd < 0 ) continue; 
			if (stsInd == grDaughter0.at(0)){
				if(gTrack->GetRichRingIndex() > -1) richInd0 = gTrack->GetRichRingIndex();
			}
			if (stsInd == grDaughter1.at(0)){ 
				if(gTrack->GetRichRingIndex() > -1) richInd1 = gTrack->GetRichRingIndex();
			}
		}
		int richcheck = 0;
		int richcheck0 = 0;
		int richcheck1 = 0;
		for (int loo = 0; loo < 2; loo++){
			if (loo == 0 && richInd0 != 99999){ 
				CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(richInd0);
				if (richMatch == nullptr) continue;
				int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
				if (richMcTrackId < 0) continue;
				if(stsMcTrackId0 != richMcTrackId) continue; // check that global track was matched correctly for STS and RICH together
				CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
				if (mcTrack2 == nullptr) continue;
				int pdgRICH = mcTrack2->GetPdgCode();
				if (TMath::Abs(pdgRICH) == 11 ) richcheck++; richcheck0++;
			}
			if (loo == 1 && richInd1 != 99999){ 
				CbmTrackMatchNew* richMatch  = (CbmTrackMatchNew*)fRichRingMatches->At(richInd1);
				if (richMatch == nullptr) continue;
				int richMcTrackId = richMatch->GetMatchedLink().GetIndex();
				if (richMcTrackId < 0) continue;
				if(stsMcTrackId1 != richMcTrackId) continue; // check that global track was matched correctly for STS and RICH together
				CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(richMcTrackId);
				if (mcTrack2 == nullptr) continue;
				int pdgRICH = mcTrack2->GetPdgCode();
				if (TMath::Abs(pdgRICH) == 11 ) richcheck++; richcheck1++;
			}
		}
		
		TVector3 refmomentum0;
		TVector3 refmomentum1;
		refmomentum0 = FitToVertexSTS(stsTrack0, allgammas[tt].GetX(), allgammas[tt].GetY(), allgammas[tt].GetZ());
		refmomentum1 = FitToVertexSTS(stsTrack1, allgammas[tt].GetX(), allgammas[tt].GetY(), allgammas[tt].GetZ());
		frefmomentum.push_back(refmomentum0);
		frefmomentum.push_back(refmomentum1);
		fmcvector.push_back(mcTrack0);
		fmcvector.push_back(mcTrack1);
		Double_t invmassTrue = Invmass_2particles(fmcvector.at(0), fmcvector.at(1));
		Double_t invmassReco = Invmass_2particlesRECO(frefmomentum.at(0), frefmomentum.at(1));
		Double_t opening_angle_mc = CalculateOpeningAngleMC(fmcvector.at(0), fmcvector.at(1));
		Double_t opening_angle_refitted = CalculateOpeningAngleReco(frefmomentum.at(0), frefmomentum.at(1));

/*		cout << "MY: px = " << refmomentum0.X() << "; py = " << refmomentum0.Y() << "; pz = " << refmomentum0.Z() << endl;
		cout << "MAX: px = " << particlevector[electronIds.at(0)].KFParticleBase::GetPx() << "; py = " << particlevector[electronIds.at(0)].KFParticleBase::GetPy() << "; pz = " << particlevector[electronIds.at(0)].KFParticleBase::GetPz() << endl;
		cout << endl;
		cout << "MY: px = " << refmomentum1.X() << "; py = " << refmomentum1.Y() << "; pz = " << refmomentum1.Z() << endl;
		cout << "MAX: px = " << particlevector[electronIds.at(1)].KFParticleBase::GetPx() << "; py = " << particlevector[electronIds.at(1)].KFParticleBase::GetPy() << "; pz = " << particlevector[electronIds.at(1)].KFParticleBase::GetPz() << endl;*/

		 // graphs for understanding cuts:
		if ( TMath::Abs(mcTrack0->GetPdgCode()) == 11 && TMath::Abs(mcTrack1->GetPdgCode()) == 11 && (mcTrack0->GetPdgCode()+mcTrack1->GetPdgCode()) == 0){
			CheckForCuts_OA_MC->Fill(opening_angle_mc);
			CheckForCuts_OA_Reco->Fill(opening_angle_refitted);
			CheckForCuts_InvMass_MC->Fill(invmassTrue);
			CheckForCuts_InvMass_Reco->Fill(invmassReco);
				if (mcTrack0->GetMotherId() == -1 || mcTrack1->GetMotherId() == -1) continue;
				CbmMCTrack* mcTrackMotherSTS0 = (CbmMCTrack*) fMcTracks->At(mcTrack0->GetMotherId());
				CbmMCTrack* mcTrackMotherSTS1 = (CbmMCTrack*) fMcTracks->At(mcTrack1->GetMotherId());
				if (mcTrackMotherSTS0 == nullptr || mcTrackMotherSTS1 == nullptr) continue;
				if (mcTrackMotherSTS0->GetPdgCode() != 22 && mcTrackMotherSTS1->GetPdgCode() != 22) continue; // electrons/positrons from gamma
				if (mcTrackMotherSTS0->GetMotherId() == -1) continue;
				if (mcTrackMotherSTS0->GetMotherId() != mcTrackMotherSTS1->GetMotherId()) continue; // from one gamma
				CbmMCTrack* mcTrackMotherOfGammaSTS0 = (CbmMCTrack*) fMcTracks->At(mcTrackMotherSTS0->GetMotherId()); // pi0 
				if (mcTrackMotherOfGammaSTS0->GetPdgCode() != 111) continue;
				CheckForCuts_OA_MC_from_one_pi0->Fill(opening_angle_mc);
				CheckForCuts_OA_Reco_from_one_pi0->Fill(opening_angle_refitted);
				CheckForCuts_InvMass_MC_from_one_pi0->Fill(invmassTrue);
				CheckForCuts_InvMass_Reco_from_one_pi0->Fill(invmassReco);
				CheckForCuts_z_vs_InvMass_MC_from_one_pi0->Fill(mcTrack0->GetStartZ(), invmassTrue);
				CheckForCuts_z_vs_OA_MC_from_one_pi0->Fill(mcTrack0->GetStartZ(), opening_angle_mc);
				CheckForCuts_z_vs_InvMass_Reco_from_one_pi0->Fill(allgammas[tt].GetZ(), invmassReco);
				CheckForCuts_z_vs_OA_Reco_from_one_pi0->Fill(allgammas[tt].GetZ(), opening_angle_refitted);
				if (allgammas[tt].GetZ() <= 20) {
					CheckForCuts_InvMass_Reco_from_one_pi0_less20cm->Fill(invmassReco);
					CheckForCuts_OA_Reco_from_one_pi0_less20cm->Fill(opening_angle_refitted);
				}
				if (allgammas[tt].GetZ() <= 40 && allgammas[tt].GetZ() > 20) {
					CheckForCuts_InvMass_Reco_from_one_pi0_between20cm_40cm->Fill(invmassReco);
					CheckForCuts_OA_Reco_from_one_pi0_between20cm_40cm->Fill(opening_angle_refitted);
				}
				if (allgammas[tt].GetZ() > 40) {
					CheckForCuts_InvMass_Reco_from_one_pi0_more40cm->Fill(invmassReco);
					CheckForCuts_OA_Reco_from_one_pi0_more40cm->Fill(opening_angle_refitted);
				}
		}



		// cuts
		if (TMath::Abs(opening_angle_refitted) > AngleCut) continue;
		if (TMath::Abs(invmassReco) > InvMassCut) continue;


		// for event mixing
		EMT_Event.push_back(Event);  //event number
		EMT_pair_momenta.push_back(frefmomentum);  //event number
		EMT_OA.push_back(opening_angle_refitted);  //event number
		EMT_InvMass.push_back(invmassReco);  //event number
		EMT_NofRings.push_back(richcheck);  //event number
		EMT_Z.push_back(allgammas[tt].GetZ());



		double dx0 = allgammas[tt].GetX()-mcTrack0->GetStartX();
		double dy0 = allgammas[tt].GetY()-mcTrack0->GetStartY();
		double dz0 = allgammas[tt].GetZ()-mcTrack0->GetStartZ();
		double dx1 = allgammas[tt].GetX()-mcTrack1->GetStartX();
		double dy1 = allgammas[tt].GetY()-mcTrack1->GetStartY();
		double dz1 = allgammas[tt].GetZ()-mcTrack1->GetStartZ();
		double Dist0 = sqrt((dx0)*(dx0)+(dy0)*(dy0)+(dz0)*(dz0));
		double Dist1 = sqrt((dx1)*(dx1)+(dy1)*(dy1)+(dz1)*(dz1));

		// everything (RICH == 0, RICH == 1, RICH == 2) together
		if(richcheck == 0 || richcheck == 1 || richcheck == 2 ){
			// for event mixing multi
			EMT_Event_multi_all.push_back(Event);  //event number
			EMT_pair_momenta_multi_all.push_back(frefmomentum);  //event number
			EMT_OA_multi_all.push_back(opening_angle_refitted);  //event number
			EMT_InvMass_multi_all.push_back(invmassReco);  //event number

			GammasAll.push_back(allgammas[tt]);
			fGammaInvMassMC_All->Fill(invmassTrue);
			fGammaInvMassReco_All->Fill(invmassReco);
			fGammaOpeningAngleMC_All->Fill(opening_angle_mc);
			fGammaOpeningAngleReco_All->Fill(opening_angle_refitted);
			fPdg_All->Fill(TMath::Abs(mcTrack0->GetPdgCode()));
			fPdg_All->Fill(TMath::Abs(mcTrack1->GetPdgCode()));
			fdx_All->Fill(dx0);
			fdx_All->Fill(dx1);
			fdy_All->Fill(dy0);
			fdy_All->Fill(dy1);
			fdz_All->Fill(dz0);
			fdz_All->Fill(dz1);
			fD_All->Fill(Dist0);
			fD_All->Fill(Dist1);
			fP_mc_All->Fill(mcTrack0->GetP());
			fP_mc_All->Fill(mcTrack1->GetP());
			fPt_mc_All->Fill(mcTrack0->GetPt());
			fPt_mc_All->Fill(mcTrack1->GetPt());
			fP_reco_All->Fill(refmomentum0.Mag());
			fP_reco_All->Fill(refmomentum1.Mag());
			fPt_reco_All->Fill(refmomentum0.Perp());
			fPt_reco_All->Fill(refmomentum1.Perp());
			if(allgammas[tt].GetZ() < 4.5){
				fGammaInvMassReco_All_target->Fill(invmassReco);
				fGammaOpeningAngleReco_All_target->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 4.5 && allgammas[tt].GetZ() < 20){
				fGammaInvMassReco_All_mvd->Fill(invmassReco);
				fGammaOpeningAngleReco_All_mvd->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 20 && allgammas[tt].GetZ() < 70){
				fGammaInvMassReco_All_sts->Fill(invmassReco);
				fGammaOpeningAngleReco_All_sts->Fill(opening_angle_refitted);
			}
		}

		// only cases, when RICH == 1 
		if(richcheck == 1 ){
			// for event mixing multi
			EMT_Event_multi_one.push_back(Event);  //event number
			EMT_pair_momenta_multi_one.push_back(frefmomentum);  //event number
			EMT_OA_multi_one.push_back(opening_angle_refitted);  //event number
			EMT_InvMass_multi_one.push_back(invmassReco);  //event number

			GammasOneLeptonInRICH.push_back(allgammas[tt]);
			fGammaInvMassMC_One_In_Rich->Fill(invmassTrue);
			fGammaInvMassReco_One_In_Rich->Fill(invmassReco);
			fGammaOpeningAngleMC_One_In_Rich->Fill(opening_angle_mc);
			fGammaOpeningAngleReco_One_In_Rich->Fill(opening_angle_refitted);
			fPdg_One_In_Rich->Fill(TMath::Abs(mcTrack0->GetPdgCode()));
			fPdg_One_In_Rich->Fill(TMath::Abs(mcTrack1->GetPdgCode()));
			fdx_One_In_Rich->Fill(dx0);
			fdx_One_In_Rich->Fill(dx1);
			fdy_One_In_Rich->Fill(dy0);
			fdy_One_In_Rich->Fill(dy1);
			fdz_One_In_Rich->Fill(dz0);
			fdz_One_In_Rich->Fill(dz1);
			fD_One_In_Rich->Fill(Dist0);
			fD_One_In_Rich->Fill(Dist1);
			fP_mc_One_In_Rich->Fill(mcTrack0->GetP());
			fP_mc_One_In_Rich->Fill(mcTrack1->GetP());
			fPt_mc_One_In_Rich->Fill(mcTrack0->GetPt());
			fPt_mc_One_In_Rich->Fill(mcTrack1->GetPt());
			fP_reco_One_In_Rich->Fill(refmomentum0.Mag());
			fP_reco_One_In_Rich->Fill(refmomentum1.Mag());
			fPt_reco_One_In_Rich->Fill(refmomentum0.Perp());
			fPt_reco_One_In_Rich->Fill(refmomentum1.Perp());
			if(allgammas[tt].GetZ() < 4.5){
				fGammaInvMassReco_One_In_Rich_target->Fill(invmassReco);
				fGammaOpeningAngleReco_One_In_Rich_target->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 4.5 && allgammas[tt].GetZ() < 20){
				fGammaInvMassReco_One_In_Rich_mvd->Fill(invmassReco);
				fGammaOpeningAngleReco_One_In_Rich_mvd->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 20 && allgammas[tt].GetZ() < 70){
				fGammaInvMassReco_One_In_Rich_sts->Fill(invmassReco);
				fGammaOpeningAngleReco_One_In_Rich_sts->Fill(opening_angle_refitted);
			}
		}

		// only cases, when RICH == 2 
		if(richcheck == 2 ){
			// for event mixing multi
			EMT_Event_multi_two.push_back(Event);  //event number
			EMT_pair_momenta_multi_two.push_back(frefmomentum);  //event number
			EMT_OA_multi_two.push_back(opening_angle_refitted);  //event number
			EMT_InvMass_multi_two.push_back(invmassReco);  //event number

			GammasTwoLeptonInRICH.push_back(allgammas[tt]);
			fGammaInvMassMC_Two_In_Rich->Fill(invmassTrue);
			fGammaInvMassReco_Two_In_Rich->Fill(invmassReco);
			fGammaOpeningAngleMC_Two_In_Rich->Fill(opening_angle_mc);
			fGammaOpeningAngleReco_Two_In_Rich->Fill(opening_angle_refitted);
			fPdg_Two_In_Rich->Fill(TMath::Abs(mcTrack0->GetPdgCode()));
			fPdg_Two_In_Rich->Fill(TMath::Abs(mcTrack1->GetPdgCode()));
			fdx_Two_In_Rich->Fill(dx0);
			fdx_Two_In_Rich->Fill(dx1);
			fdy_Two_In_Rich->Fill(dy0);
			fdy_Two_In_Rich->Fill(dy1);
			fdz_Two_In_Rich->Fill(dz0);
			fdz_Two_In_Rich->Fill(dz1);
			fD_Two_In_Rich->Fill(Dist0);
			fD_Two_In_Rich->Fill(Dist1);
			fP_mc_Two_In_Rich->Fill(mcTrack0->GetP());
			fP_mc_Two_In_Rich->Fill(mcTrack1->GetP());
			fPt_mc_Two_In_Rich->Fill(mcTrack0->GetPt());
			fPt_mc_Two_In_Rich->Fill(mcTrack1->GetPt());
			fP_reco_Two_In_Rich->Fill(refmomentum0.Mag());
			fP_reco_Two_In_Rich->Fill(refmomentum1.Mag());
			fPt_reco_Two_In_Rich->Fill(refmomentum0.Perp());
			fPt_reco_Two_In_Rich->Fill(refmomentum1.Perp());
			if(allgammas[tt].GetZ() < 4.5){
				fGammaInvMassReco_Two_In_Rich_target->Fill(invmassReco);
				fGammaOpeningAngleReco_Two_In_Rich_target->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 4.5 && allgammas[tt].GetZ() < 20){
				fGammaInvMassReco_Two_In_Rich_mvd->Fill(invmassReco);
				fGammaOpeningAngleReco_Two_In_Rich_mvd->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 20 && allgammas[tt].GetZ() < 70){
				fGammaInvMassReco_Two_In_Rich_sts->Fill(invmassReco);
				fGammaOpeningAngleReco_Two_In_Rich_sts->Fill(opening_angle_refitted);
			}
		}

		// only cases, when RICH == 0 
		if(richcheck == 0 ){
			// for event mixing multi
			EMT_Event_multi_zero.push_back(Event);  //event number
			EMT_pair_momenta_multi_zero.push_back(frefmomentum);  //event number
			EMT_OA_multi_zero.push_back(opening_angle_refitted);  //event number
			EMT_InvMass_multi_zero.push_back(invmassReco);  //event number

			GammasZeroLeptonInRICH.push_back(allgammas[tt]);
			fGammaInvMassMC_Zero_In_Rich->Fill(invmassTrue);
			fGammaInvMassReco_Zero_In_Rich->Fill(invmassReco);
			fGammaOpeningAngleMC_Zero_In_Rich->Fill(opening_angle_mc);
			fGammaOpeningAngleReco_Zero_In_Rich->Fill(opening_angle_refitted);
			fPdg_Zero_In_Rich->Fill(TMath::Abs(mcTrack0->GetPdgCode()));
			fPdg_Zero_In_Rich->Fill(TMath::Abs(mcTrack1->GetPdgCode()));
			fdx_Zero_In_Rich->Fill(dx0);
			fdx_Zero_In_Rich->Fill(dx1);
			fdy_Zero_In_Rich->Fill(dy0);
			fdy_Zero_In_Rich->Fill(dy1);
			fdz_Zero_In_Rich->Fill(dz0);
			fdz_Zero_In_Rich->Fill(dz1);
			fD_Zero_In_Rich->Fill(Dist0);
			fD_Zero_In_Rich->Fill(Dist1);
			fP_mc_Zero_In_Rich->Fill(mcTrack0->GetP());
			fP_mc_Zero_In_Rich->Fill(mcTrack1->GetP());
			fPt_mc_Zero_In_Rich->Fill(mcTrack0->GetPt());
			fPt_mc_Zero_In_Rich->Fill(mcTrack1->GetPt());
			fP_reco_Zero_In_Rich->Fill(refmomentum0.Mag());
			fP_reco_Zero_In_Rich->Fill(refmomentum1.Mag());
			fPt_reco_Zero_In_Rich->Fill(refmomentum0.Perp());
			fPt_reco_Zero_In_Rich->Fill(refmomentum1.Perp());
			if(allgammas[tt].GetZ() < 4.5){
				fGammaInvMassReco_Zero_In_Rich_target->Fill(invmassReco);
				fGammaOpeningAngleReco_Zero_In_Rich_target->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 4.5 && allgammas[tt].GetZ() < 20){
				fGammaInvMassReco_Zero_In_Rich_mvd->Fill(invmassReco);
				fGammaOpeningAngleReco_Zero_In_Rich_mvd->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 20 && allgammas[tt].GetZ() < 70){
				fGammaInvMassReco_Zero_In_Rich_sts->Fill(invmassReco);
				fGammaOpeningAngleReco_Zero_In_Rich_sts->Fill(opening_angle_refitted);
			}
		}

		// cases, when RICH == 1 or RICH == 2 together
		if(richcheck == 1 || richcheck == 2 ){
			// for event mixing multi
			EMT_Event_multi_onetwo.push_back(Event);  //event number
			EMT_pair_momenta_multi_onetwo.push_back(frefmomentum);  //event number
			EMT_OA_multi_onetwo.push_back(opening_angle_refitted);  //event number
			EMT_InvMass_multi_onetwo.push_back(invmassReco);  //event number

			GammasOneTwoLeptonInRICH.push_back(allgammas[tt]);
			fGammaInvMassMC_OneTwo_In_Rich->Fill(invmassTrue);
			fGammaInvMassReco_OneTwo_In_Rich->Fill(invmassReco);
			fGammaOpeningAngleMC_OneTwo_In_Rich->Fill(opening_angle_mc);
			fGammaOpeningAngleReco_OneTwo_In_Rich->Fill(opening_angle_refitted);
			fPdg_OneTwo_In_Rich->Fill(TMath::Abs(mcTrack0->GetPdgCode()));
			fPdg_OneTwo_In_Rich->Fill(TMath::Abs(mcTrack1->GetPdgCode()));
			fdx_OneTwo_In_Rich->Fill(dx0);
			fdx_OneTwo_In_Rich->Fill(dx1);
			fdy_OneTwo_In_Rich->Fill(dy0);
			fdy_OneTwo_In_Rich->Fill(dy1);
			fdz_OneTwo_In_Rich->Fill(dz0);
			fdz_OneTwo_In_Rich->Fill(dz1);
			fD_OneTwo_In_Rich->Fill(Dist0);
			fD_OneTwo_In_Rich->Fill(Dist1);
			fP_mc_OneTwo_In_Rich->Fill(mcTrack0->GetP());
			fP_mc_OneTwo_In_Rich->Fill(mcTrack1->GetP());
			fPt_mc_OneTwo_In_Rich->Fill(mcTrack0->GetPt());
			fPt_mc_OneTwo_In_Rich->Fill(mcTrack1->GetPt());
			fP_reco_OneTwo_In_Rich->Fill(refmomentum0.Mag());
			fP_reco_OneTwo_In_Rich->Fill(refmomentum1.Mag());
			fPt_reco_OneTwo_In_Rich->Fill(refmomentum0.Perp());
			fPt_reco_OneTwo_In_Rich->Fill(refmomentum1.Perp());
			if(allgammas[tt].GetZ() < 4.5){
				fGammaInvMassReco_OneTwo_In_Rich_target->Fill(invmassReco);
				fGammaOpeningAngleReco_OneTwo_In_Rich_target->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 4.5 && allgammas[tt].GetZ() < 20){
				fGammaInvMassReco_OneTwo_In_Rich_mvd->Fill(invmassReco);
				fGammaOpeningAngleReco_OneTwo_In_Rich_mvd->Fill(opening_angle_refitted);
			}
			if(allgammas[tt].GetZ() > 20 && allgammas[tt].GetZ() < 70){
				fGammaInvMassReco_OneTwo_In_Rich_sts->Fill(invmassReco);
				fGammaOpeningAngleReco_OneTwo_In_Rich_sts->Fill(opening_angle_refitted);
			}
		}

	}
	
	cout << "number of gammas with 0-2 electron identified in RICH = " << GammasAll.size() << endl;
	cout << "number of gammas with 1-2 electron identified in RICH = " << GammasOneTwoLeptonInRICH.size() << endl;
	cout << "number of gammas with  0  electron identified in RICH = " << GammasZeroLeptonInRICH.size() << endl;
	cout << "number of gammas with  1  electron identified in RICH = " << GammasOneLeptonInRICH.size() << endl;
	cout << "number of gammas with  2  electron identified in RICH = " << GammasTwoLeptonInRICH.size() << endl;

	for (int kk = 0; kk < GammasAll.size(); kk++){
		EMT_multi_all.push_back(GammasAll.size());
	}
	for (int kk = 0; kk < GammasOneLeptonInRICH.size(); kk++){
		EMT_multi_one.push_back(GammasOneLeptonInRICH.size());
	}
	for (int kk = 0; kk < GammasTwoLeptonInRICH.size(); kk++){
		EMT_multi_two.push_back(GammasTwoLeptonInRICH.size());
	}
	for (int kk = 0; kk < GammasZeroLeptonInRICH.size(); kk++){
		EMT_multi_zero.push_back(GammasZeroLeptonInRICH.size());
	}
	for (int kk = 0; kk < GammasOneTwoLeptonInRICH.size(); kk++){
		EMT_multi_onetwo.push_back(GammasOneTwoLeptonInRICH.size());
	}

}



TVector3 CbmAnaConversion2KF::FitToVertexSTS(CbmStsTrack* stsTrack, double x, double y, double z)
{
	CbmVertex* vtx = new CbmVertex();
	TMatrixFSym* covMat = new TMatrixFSym(3);
	vtx->SetVertex(x, y, z, 0, 0, 0, *covMat);
	CbmStsKFTrackFitter fitter;
	FairTrackParam neu_track;
	fitter.FitToVertex(stsTrack, vtx, &neu_track);

	TVector3 position;
	TVector3 momentum;

	neu_track.Position(position);
	neu_track.Momentum(momentum);

	return momentum;
}


Double_t CbmAnaConversion2KF::Invmass_2particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2)
{
	TLorentzVector lorVec1;
	mctrack1->Get4Momentum(lorVec1);
    
	TLorentzVector lorVec2;
	mctrack2->Get4Momentum(lorVec2); 

	TLorentzVector sum;
	sum = lorVec1 + lorVec2; 

	return sum.Mag();
}

Double_t CbmAnaConversion2KF::Invmass_2particlesRECO(const TVector3 part1, const TVector3 part2)
{
	Double_t energy1 = TMath::Sqrt(part1.Mag2() + M2E);
	TLorentzVector lorVec1(part1, energy1);

	Double_t energy2 = TMath::Sqrt(part2.Mag2() + M2E);
	TLorentzVector lorVec2(part2, energy2);

	TLorentzVector sum;
	sum = lorVec1 + lorVec2;    

	return sum.Mag();
}

Double_t CbmAnaConversion2KF::CalculateOpeningAngleReco(TVector3 electron1, TVector3 electron2)
{
	Double_t energyP = TMath::Sqrt(electron1.Mag2() + M2E);
	TLorentzVector lorVecP(electron1, energyP);

	Double_t energyM = TMath::Sqrt(electron2.Mag2() + M2E);
	TLorentzVector lorVecM(electron2, energyM);

	Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
	Double_t theta = 180.*anglePair/TMath::Pi();

	return theta;
}


Double_t CbmAnaConversion2KF::CalculateOpeningAngleMC(CbmMCTrack* mctrack1, CbmMCTrack* mctrack2)
{
	TVector3 electron1;
	mctrack1->GetMomentum(electron1);
	Double_t energyP = TMath::Sqrt(electron1.Mag2() + M2E);
	TLorentzVector lorVecP(electron1, energyP);

	TVector3 electron2;
	mctrack2->GetMomentum(electron2);
	Double_t energyM = TMath::Sqrt(electron2.Mag2() + M2E);
	TLorentzVector lorVecM(electron2, energyM);

	Double_t anglePair = lorVecM.Angle(lorVecP.Vect());
	Double_t theta = 180.*anglePair/TMath::Pi();

	return theta;
}

Double_t CbmAnaConversion2KF::Invmass_4particles(const CbmMCTrack* mctrack1, const CbmMCTrack* mctrack2, const CbmMCTrack* mctrack3, const CbmMCTrack* mctrack4)
{
	TLorentzVector lorVec1;
	mctrack1->Get4Momentum(lorVec1);
    
	TLorentzVector lorVec2;
	mctrack2->Get4Momentum(lorVec2);
    
	TLorentzVector lorVec3;
	mctrack3->Get4Momentum(lorVec3);
    
	TLorentzVector lorVec4;
	mctrack4->Get4Momentum(lorVec4);

	TLorentzVector sum;
	sum = lorVec1 + lorVec2 + lorVec3 + lorVec4;    

	return sum.Mag();
}

Double_t CbmAnaConversion2KF::Invmass_4particlesRECO(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
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


void CbmAnaConversion2KF::FindPi0(vector<KFParticle> normalgammas, vector<KFParticle> particlevector, TH1D* histoReco, TH1D* histoTrue, TH2D* ptVSrap, TH1D* less1cm, TH1D* zw1and20cm, TH1D* zw20and70cm, TH1D* Case1, TH1D* Case2, TH1D* Case3, TH1D* Case4, TH1D* Case5, TH1D* Case6, TH1D* Case7, TH1D* Case8, TH1D* Case9, TH1D* Case10, TH2D* histoRecoCheck, TH2D* charged, TH1D* PdgCase8, TH1D* more1cm, TH1D* testsameMIDcase8, TH1D* testsameGRIDcase8, TH1D* PdgCase8mothers, TH1D* case8GRIDInvMassGamma, TH1D* case8GRIDOAGamma, vector<TH1*> Pt_histos, TH2D* Case1ZYPos)
{
	// combine all gamma in pi0 --> calculate inv mass for gammas and pi0 // not the case, when one particle used twice for different gammas
	if(normalgammas.size() > 1){
		for (int gamma1=0; gamma1<normalgammas.size()-1; gamma1++){
			for (int gamma2=gamma1+1; gamma2<normalgammas.size(); gamma2++){
				std::vector<int> gamma1electronIds = normalgammas[gamma1].KFParticleBase::DaughterIds();
				std::vector<int> gamma2electronIds = normalgammas[gamma2].KFParticleBase::DaughterIds();
				
				// 4 electrons from gammas
				std::vector<int> electronIdsConstr1 = particlevector[gamma1electronIds.at(0)].KFParticleBase::DaughterIds();
				std::vector<int> electronIdsConstr2 = particlevector[gamma1electronIds.at(1)].KFParticleBase::DaughterIds();
				std::vector<int> electronIdsConstr3 = particlevector[gamma2electronIds.at(0)].KFParticleBase::DaughterIds();
				std::vector<int> electronIdsConstr4 = particlevector[gamma2electronIds.at(1)].KFParticleBase::DaughterIds();
				if(electronIdsConstr1.size() != 1 || electronIdsConstr2.size() != 1 || electronIdsConstr3.size() != 1 || electronIdsConstr4.size() != 1) continue; //electrons constructed from itself
				if(electronIdsConstr1.at(0) == electronIdsConstr3.at(0) || electronIdsConstr1.at(0) == electronIdsConstr4.at(0) || 
				   electronIdsConstr2.at(0) == electronIdsConstr3.at(0) || electronIdsConstr2.at(0) == electronIdsConstr4.at(0)) continue; // not the case, when one particle used twice for different gammas
				
				//take STS tracks
				CbmStsTrack* stsTrack1 = (CbmStsTrack*) fStsTracks->At(electronIdsConstr1.at(0));
				CbmStsTrack* stsTrack2 = (CbmStsTrack*) fStsTracks->At(electronIdsConstr2.at(0));
				CbmStsTrack* stsTrack3 = (CbmStsTrack*) fStsTracks->At(electronIdsConstr3.at(0));
				CbmStsTrack* stsTrack4 = (CbmStsTrack*) fStsTracks->At(electronIdsConstr4.at(0));
				if (stsTrack1 == nullptr || stsTrack2 == nullptr || stsTrack3 == nullptr || stsTrack4 == nullptr) continue;
				
				CbmTrackMatchNew* stsMatch1 = (CbmTrackMatchNew*) fStsTrackMatches->At(electronIdsConstr1.at(0));
				CbmTrackMatchNew* stsMatch2 = (CbmTrackMatchNew*) fStsTrackMatches->At(electronIdsConstr2.at(0));
				CbmTrackMatchNew* stsMatch3 = (CbmTrackMatchNew*) fStsTrackMatches->At(electronIdsConstr3.at(0));
				CbmTrackMatchNew* stsMatch4 = (CbmTrackMatchNew*) fStsTrackMatches->At(electronIdsConstr4.at(0));
				if (stsMatch1 == nullptr || stsMatch2 == nullptr || stsMatch3 == nullptr || stsMatch4 == nullptr) continue;
				if (stsMatch1->GetNofLinks() <= 0 || stsMatch2->GetNofLinks() <= 0 || stsMatch3->GetNofLinks() <= 0 || stsMatch4->GetNofLinks() <= 0) continue;
				
				int stsMcTrackId1 = stsMatch1->GetMatchedLink().GetIndex();
				int stsMcTrackId2 = stsMatch2->GetMatchedLink().GetIndex();
				int stsMcTrackId3 = stsMatch3->GetMatchedLink().GetIndex();
				int stsMcTrackId4 = stsMatch4->GetMatchedLink().GetIndex();
				if (stsMcTrackId1 < 0 || stsMcTrackId2 < 0 || stsMcTrackId3 < 0 || stsMcTrackId4 < 0) continue;
				
				// take MC true data
				CbmMCTrack* mcTrack1 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId1);
				CbmMCTrack* mcTrack2 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId2);
				CbmMCTrack* mcTrack3 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId3);
				CbmMCTrack* mcTrack4 = (CbmMCTrack*) fMcTracks->At(stsMcTrackId4);
				if (mcTrack1 == nullptr || mcTrack2 == nullptr || mcTrack3 == nullptr || mcTrack4 == nullptr) continue;
				
				// fit STS track to point from KFParticle
				TVector3 refmomentum1 = FitToVertexSTS(stsTrack1, normalgammas[gamma1].GetX(), normalgammas[gamma1].GetY(), normalgammas[gamma1].GetZ());
				TVector3 refmomentum2 = FitToVertexSTS(stsTrack2, normalgammas[gamma1].GetX(), normalgammas[gamma1].GetY(), normalgammas[gamma1].GetZ());
				TVector3 refmomentum3 = FitToVertexSTS(stsTrack3, normalgammas[gamma2].GetX(), normalgammas[gamma2].GetY(), normalgammas[gamma2].GetZ());
				TVector3 refmomentum4 = FitToVertexSTS(stsTrack4, normalgammas[gamma2].GetX(), normalgammas[gamma2].GetY(), normalgammas[gamma2].GetZ());
				
				Double_t invmassTruePi0 = Invmass_4particles(mcTrack1, mcTrack2, mcTrack3, mcTrack4);	// true MC values
				Double_t invmassRecoPi0 = Invmass_4particlesRECO(refmomentum1, refmomentum2, refmomentum3, refmomentum4);
				CbmLmvmKinematicParams params = CalculateKinematicParams_4particles(refmomentum1, refmomentum2, refmomentum3, refmomentum4);


				Double_t OA3 = CalculateOpeningAngleReco(refmomentum1, refmomentum3);
				Double_t OA4 = CalculateOpeningAngleReco(refmomentum2, refmomentum4);
				Double_t OA5 = CalculateOpeningAngleReco(refmomentum1, refmomentum4);
				Double_t OA6 = CalculateOpeningAngleReco(refmomentum2, refmomentum3);
//				if ((OA3+OA4 < 6 || OA5+OA6 < 6) && (normalgammas[gamma1].GetZ() < 4.5 && normalgammas[gamma2].GetZ() < 4.5)) continue;

				
				// cuts
				//if (TMath::Abs(invmassTruePi0) < 0.12 || TMath::Abs(invmassTruePi0) > 0.15) continue;
				
				histoReco->Fill(invmassRecoPi0);
				histoTrue->Fill(invmassTruePi0);

				histoRecoCheck->Fill(normalgammas.size(), invmassRecoPi0);
				charged->Fill(fGlobalTracks->GetEntriesFast(), invmassRecoPi0);

				if(normalgammas[gamma1].GetZ() < 4.5 && normalgammas[gamma2].GetZ() < 4.5){
					less1cm->Fill(invmassRecoPi0);
				}
				if(normalgammas[gamma1].GetZ() > 4.5 && normalgammas[gamma1].GetZ() < 20 && normalgammas[gamma2].GetZ() > 4.5 && normalgammas[gamma2].GetZ() < 20){
					zw1and20cm->Fill(invmassRecoPi0);
				}
				if(normalgammas[gamma1].GetZ() > 20 && normalgammas[gamma1].GetZ() < 70 && normalgammas[gamma2].GetZ() > 20 && normalgammas[gamma2].GetZ() < 70){
					zw20and70cm->Fill(invmassRecoPi0);
				}
				if(normalgammas[gamma1].GetZ() > 4.5 && normalgammas[gamma2].GetZ() > 4.5){
					more1cm->Fill(invmassRecoPi0);
				}

				// separate by Pt
				if (params.fPt <= 0.2) Pt_histos[0]->Fill(invmassRecoPi0);
				if (params.fPt > 0.2 && params.fPt <= 0.4) Pt_histos[1]->Fill(invmassRecoPi0);
				if (params.fPt > 0.4 && params.fPt <= 0.6) Pt_histos[2]->Fill(invmassRecoPi0);
				if (params.fPt > 0.6 && params.fPt <= 0.8) Pt_histos[3]->Fill(invmassRecoPi0);
				if (params.fPt > 0.8 && params.fPt <= 1.0) Pt_histos[4]->Fill(invmassRecoPi0);
				if (params.fPt > 1.0 && params.fPt <= 1.2) Pt_histos[5]->Fill(invmassRecoPi0);
				if (params.fPt > 1.2 && params.fPt <= 1.4) Pt_histos[6]->Fill(invmassRecoPi0);
				if (params.fPt > 1.4 && params.fPt <= 1.6) Pt_histos[7]->Fill(invmassRecoPi0);
				if (params.fPt > 1.6 && params.fPt <= 1.8) Pt_histos[8]->Fill(invmassRecoPi0);
				if (params.fPt > 1.8 && params.fPt <= 2.0) Pt_histos[9]->Fill(invmassRecoPi0);
				if (params.fPt > 2.0 && params.fPt <= 2.2) Pt_histos[10]->Fill(invmassRecoPi0);
				if (params.fPt > 2.2) Pt_histos[11]->Fill(invmassRecoPi0);


				if(DoBGAnalysis) {
					fAnaBG->Exec(mcTrack1, mcTrack2, mcTrack3, mcTrack4, refmomentum1, refmomentum2, refmomentum3, refmomentum4, invmassRecoPi0, Case1, Case2, Case3, Case4, Case5, Case6, Case7, Case8, Case9, Case10, PdgCase8, testsameMIDcase8, testsameGRIDcase8, PdgCase8mothers, case8GRIDInvMassGamma, case8GRIDOAGamma, Case1ZYPos);
				}


				CbmLmvmKinematicParams params1 = CalculateKinematicParams_4particles(refmomentum1, refmomentum2, refmomentum3, refmomentum4);

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


void CbmAnaConversion2KF::DoMC()
{
	// ========================================================================================
	///////   START - Analyse MC tracks
	Int_t nofMcTracks = fMcTracks->GetEntriesFast();
	for (int i = 0; i < nofMcTracks; i++) {
		CbmMCTrack* mctrack = (CbmMCTrack*)fMcTracks->At(i);
		if (mctrack == nullptr) continue;

		fPdgCodesMC->Fill(TMath::Abs(mctrack->GetPdgCode()));
		if (TMath::Abs(mctrack->GetPdgCode() ) == 11) { 
			int motherId = mctrack->GetMotherId();
			if (motherId == -1) continue;
			CbmMCTrack* mother = (CbmMCTrack*) fMcTracks->At(motherId);
			if(nullptr == mother) continue; 
			int grandmotherId = mother->GetMotherId();
			if(grandmotherId == -1) continue;
			CbmMCTrack* grandmother = (CbmMCTrack*) fMcTracks->At(grandmotherId);
			if (nullptr == grandmother) continue;
			int mcGrandmotherPdg = grandmother->GetPdgCode();
			if(mcGrandmotherPdg == 111){
				TVector3 v;
				mctrack->GetStartVertex(v);
				fMC_start_vertex->Fill(mctrack->GetStartZ(), mctrack->GetStartY() );
				fMC_Pt->Fill(mctrack->GetPt());
				fMC_P->Fill(mctrack->GetP());
			}
		}
	}
	///////   START - Analyse MC tracks (END)
	// ========================================================================================
}

CbmLmvmKinematicParams CbmAnaConversion2KF::CalculateKinematicParams_4particles(const TVector3 part1, const TVector3 part2, const TVector3 part3, const TVector3 part4)
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

void CbmAnaConversion2KF::MixedEvent(double AngleCut)
// combines photons from two different events, taken from each time 200 events
{
	Int_t nof = EMT_Event.size();
	cout << "MixedEvent - nof entries " << nof << endl;
	cout << "MixedEvent - EMT_NofRings size " << EMT_NofRings.size() << endl;
	for(Int_t a = 0; a < nof-1; a++) {
		for(Int_t b = a+1; b < nof; b++) {
			if(EMT_Event[a] == EMT_Event[b]) continue;		// to make sure that the photons are from two different events
			if (TMath::Abs(EMT_OA[a]) > AngleCut || TMath::Abs(EMT_OA[b]) > AngleCut) continue; // check cut on Opening Angle
			TVector3 e11 = EMT_pair_momenta[a][0];
			TVector3 e12 = EMT_pair_momenta[a][1];
			TVector3 e21 = EMT_pair_momenta[b][0];
			TVector3 e22 = EMT_pair_momenta[b][1];
			CbmLmvmKinematicParams params = CalculateKinematicParams_4particles(e11, e12, e21, e22);
			gr_EMT_InvMass_all->Fill(params.fMinv);
			if (EMT_NofRings[a] == 1 && EMT_NofRings[b] == 1) gr_EMT_InvMass_oneInRICH->Fill(params.fMinv);
			if (EMT_NofRings[a] == 2 && EMT_NofRings[b] == 2) gr_EMT_InvMass_twoInRICH->Fill(params.fMinv);
			if (EMT_NofRings[a] == 0 && EMT_NofRings[b] == 0) gr_EMT_InvMass_zeroInRICH->Fill(params.fMinv);
			if ( (EMT_NofRings[a] == 1 || EMT_NofRings[a] == 2) && (EMT_NofRings[b] == 1 || EMT_NofRings[b] == 2) ) gr_EMT_InvMass_onetwoInRICH->Fill(params.fMinv);

			if (EMT_Z[a] < 4.5 && EMT_Z[b] < 4.5){
				gr_EMT_InvMass_all_target->Fill(params.fMinv);
				if (EMT_NofRings[a] == 1 && EMT_NofRings[b] == 1) gr_EMT_InvMass_oneInRICH_target->Fill(params.fMinv);
				if (EMT_NofRings[a] == 2 && EMT_NofRings[b] == 2) gr_EMT_InvMass_twoInRICH_target->Fill(params.fMinv);
				if (EMT_NofRings[a] == 0 && EMT_NofRings[b] == 0) gr_EMT_InvMass_zeroInRICH_target->Fill(params.fMinv);
				if ( (EMT_NofRings[a] == 1 || EMT_NofRings[a] == 2) && (EMT_NofRings[b] == 1 || EMT_NofRings[b] == 2) ) gr_EMT_InvMass_onetwoInRICH_target->Fill(params.fMinv);
			}
			if (EMT_Z[a] > 4.5 && EMT_Z[b] > 4.5){
				gr_EMT_InvMass_all_outside_target->Fill(params.fMinv);
				if (EMT_NofRings[a] == 1 && EMT_NofRings[b] == 1) gr_EMT_InvMass_oneInRICH_outside_target->Fill(params.fMinv);
				if (EMT_NofRings[a] == 2 && EMT_NofRings[b] == 2) gr_EMT_InvMass_twoInRICH_outside_target->Fill(params.fMinv);
				if (EMT_NofRings[a] == 0 && EMT_NofRings[b] == 0) gr_EMT_InvMass_zeroInRICH_outside_target->Fill(params.fMinv);
				if ( (EMT_NofRings[a] == 1 || EMT_NofRings[a] == 2) && (EMT_NofRings[b] == 1 || EMT_NofRings[b] == 2) ) gr_EMT_InvMass_onetwoInRICH_outside_target->Fill(params.fMinv);
			}

		}
	}
}

void CbmAnaConversion2KF::MixedEventMulti()
// combines photons from two different events, taken from each time 200 events
{
	// all 
	Int_t nof_all = EMT_Event_multi_all.size();
	cout << "MixedEvent - nof entries all " << nof_all << endl;
	for(Int_t a = 0; a < nof_all-1; a++) {
		for(Int_t b = a+1; b < nof_all; b++) {
			if (EMT_Event_multi_all[a] == EMT_Event_multi_all[b]) continue;		// to make sure that the photons are from two different events
			if (EMT_multi_all[a] != EMT_multi_all[b]) continue;		// check same multiplicity
			TVector3 e11 = EMT_pair_momenta_multi_all[a][0];
			TVector3 e12 = EMT_pair_momenta_multi_all[a][1];
			TVector3 e21 = EMT_pair_momenta_multi_all[b][0];
			TVector3 e22 = EMT_pair_momenta_multi_all[b][1];
			CbmLmvmKinematicParams params = CalculateKinematicParams_4particles(e11, e12, e21, e22);

			if (EMT_multi_all[a] == 1) gr_EMTMulti_InvMass_all_m1->Fill(params.fMinv);
			if (EMT_multi_all[a] == 2) gr_EMTMulti_InvMass_all_m2->Fill(params.fMinv);
			if (EMT_multi_all[a] == 3) gr_EMTMulti_InvMass_all_m3->Fill(params.fMinv);
			if (EMT_multi_all[a] == 4) gr_EMTMulti_InvMass_all_m4->Fill(params.fMinv);
			if (EMT_multi_all[a] == 5) gr_EMTMulti_InvMass_all_m5->Fill(params.fMinv);
			if (EMT_multi_all[a] == 6) gr_EMTMulti_InvMass_all_m6->Fill(params.fMinv);
			if (EMT_multi_all[a] == 7) gr_EMTMulti_InvMass_all_m7->Fill(params.fMinv);
			if (EMT_multi_all[a] == 8) gr_EMTMulti_InvMass_all_m8->Fill(params.fMinv);
			if (EMT_multi_all[a] == 9) gr_EMTMulti_InvMass_all_m9->Fill(params.fMinv);
			if (EMT_multi_all[a] == 10) gr_EMTMulti_InvMass_all_m10->Fill(params.fMinv);
			if (EMT_multi_all[a] == 11) gr_EMTMulti_InvMass_all_m11->Fill(params.fMinv);
			if (EMT_multi_all[a] == 12) gr_EMTMulti_InvMass_all_m12->Fill(params.fMinv);
			if (EMT_multi_all[a] == 13) gr_EMTMulti_InvMass_all_m13->Fill(params.fMinv);
			if (EMT_multi_all[a] == 14) gr_EMTMulti_InvMass_all_m14->Fill(params.fMinv);
			if (EMT_multi_all[a] == 15) gr_EMTMulti_InvMass_all_m15->Fill(params.fMinv);
			if (EMT_multi_all[a] == 16) gr_EMTMulti_InvMass_all_m16->Fill(params.fMinv);
			if (EMT_multi_all[a] == 17) gr_EMTMulti_InvMass_all_m17->Fill(params.fMinv);
			if (EMT_multi_all[a] == 18) gr_EMTMulti_InvMass_all_m18->Fill(params.fMinv);
			if (EMT_multi_all[a] == 19) gr_EMTMulti_InvMass_all_m19->Fill(params.fMinv);
			if (EMT_multi_all[a] == 20) gr_EMTMulti_InvMass_all_m20->Fill(params.fMinv);
		}
	}

	// one 
	Int_t nof_one = EMT_Event_multi_one.size();
	cout << "MixedEvent - nof entries one " << nof_one << endl;
	for(Int_t a = 0; a < nof_one-1; a++) {
		for(Int_t b = a+1; b < nof_one; b++) {
			if (EMT_Event_multi_one[a] == EMT_Event_multi_one[b]) continue;		// to make sure that the photons are from two different events
			if (EMT_multi_one[a] != EMT_multi_one[b]) continue;		// check same multiplicity
			TVector3 e11 = EMT_pair_momenta_multi_one[a][0];
			TVector3 e12 = EMT_pair_momenta_multi_one[a][1];
			TVector3 e21 = EMT_pair_momenta_multi_one[b][0];
			TVector3 e22 = EMT_pair_momenta_multi_one[b][1];
			CbmLmvmKinematicParams params = CalculateKinematicParams_4particles(e11, e12, e21, e22);

			if (EMT_multi_one[a] == 1) gr_EMTMulti_InvMass_one_m1->Fill(params.fMinv);
			if (EMT_multi_one[a] == 2) gr_EMTMulti_InvMass_one_m2->Fill(params.fMinv);
			if (EMT_multi_one[a] == 3) gr_EMTMulti_InvMass_one_m3->Fill(params.fMinv);
			if (EMT_multi_one[a] == 4) gr_EMTMulti_InvMass_one_m4->Fill(params.fMinv);
			if (EMT_multi_one[a] == 5) gr_EMTMulti_InvMass_one_m5->Fill(params.fMinv);
			if (EMT_multi_one[a] == 6) gr_EMTMulti_InvMass_one_m6->Fill(params.fMinv);
			if (EMT_multi_one[a] == 7) gr_EMTMulti_InvMass_one_m7->Fill(params.fMinv);
			if (EMT_multi_one[a] == 8) gr_EMTMulti_InvMass_one_m8->Fill(params.fMinv);
			if (EMT_multi_one[a] == 9) gr_EMTMulti_InvMass_one_m9->Fill(params.fMinv);
			if (EMT_multi_one[a] == 10) gr_EMTMulti_InvMass_one_m10->Fill(params.fMinv);
			if (EMT_multi_one[a] == 11) gr_EMTMulti_InvMass_one_m11->Fill(params.fMinv);
			if (EMT_multi_one[a] == 12) gr_EMTMulti_InvMass_one_m12->Fill(params.fMinv);
			if (EMT_multi_one[a] == 13) gr_EMTMulti_InvMass_one_m13->Fill(params.fMinv);
			if (EMT_multi_one[a] == 14) gr_EMTMulti_InvMass_one_m14->Fill(params.fMinv);
			if (EMT_multi_one[a] == 15) gr_EMTMulti_InvMass_one_m15->Fill(params.fMinv);
			if (EMT_multi_one[a] == 16) gr_EMTMulti_InvMass_one_m16->Fill(params.fMinv);
			if (EMT_multi_one[a] == 17) gr_EMTMulti_InvMass_one_m17->Fill(params.fMinv);
			if (EMT_multi_one[a] == 18) gr_EMTMulti_InvMass_one_m18->Fill(params.fMinv);
			if (EMT_multi_one[a] == 19) gr_EMTMulti_InvMass_one_m19->Fill(params.fMinv);
			if (EMT_multi_one[a] == 20) gr_EMTMulti_InvMass_one_m20->Fill(params.fMinv);
		}
	}

	// two 
	Int_t nof_two = EMT_Event_multi_two.size();
	cout << "MixedEvent - nof entries two " << nof_two << endl;
	for(Int_t a = 0; a < nof_two-1; a++) {
		for(Int_t b = a+1; b < nof_two; b++) {
			if (EMT_Event_multi_two[a] == EMT_Event_multi_two[b]) continue;		// to make sure that the photons are from two different events
			if (EMT_multi_two[a] != EMT_multi_two[b]) continue;		// check same multiplicity
			TVector3 e11 = EMT_pair_momenta_multi_two[a][0];
			TVector3 e12 = EMT_pair_momenta_multi_two[a][1];
			TVector3 e21 = EMT_pair_momenta_multi_two[b][0];
			TVector3 e22 = EMT_pair_momenta_multi_two[b][1];
			CbmLmvmKinematicParams params = CalculateKinematicParams_4particles(e11, e12, e21, e22);

			if (EMT_multi_two[a] == 1) gr_EMTMulti_InvMass_two_m1->Fill(params.fMinv);
			if (EMT_multi_two[a] == 2) gr_EMTMulti_InvMass_two_m2->Fill(params.fMinv);
			if (EMT_multi_two[a] == 3) gr_EMTMulti_InvMass_two_m3->Fill(params.fMinv);
			if (EMT_multi_two[a] == 4) gr_EMTMulti_InvMass_two_m4->Fill(params.fMinv);
			if (EMT_multi_two[a] == 5) gr_EMTMulti_InvMass_two_m5->Fill(params.fMinv);
			if (EMT_multi_two[a] == 6) gr_EMTMulti_InvMass_two_m6->Fill(params.fMinv);
			if (EMT_multi_two[a] == 7) gr_EMTMulti_InvMass_two_m7->Fill(params.fMinv);
			if (EMT_multi_two[a] == 8) gr_EMTMulti_InvMass_two_m8->Fill(params.fMinv);
			if (EMT_multi_two[a] == 9) gr_EMTMulti_InvMass_two_m9->Fill(params.fMinv);
			if (EMT_multi_two[a] == 10) gr_EMTMulti_InvMass_two_m10->Fill(params.fMinv);
			if (EMT_multi_two[a] == 11) gr_EMTMulti_InvMass_two_m11->Fill(params.fMinv);
			if (EMT_multi_two[a] == 12) gr_EMTMulti_InvMass_two_m12->Fill(params.fMinv);
			if (EMT_multi_two[a] == 13) gr_EMTMulti_InvMass_two_m13->Fill(params.fMinv);
			if (EMT_multi_two[a] == 14) gr_EMTMulti_InvMass_two_m14->Fill(params.fMinv);
			if (EMT_multi_two[a] == 15) gr_EMTMulti_InvMass_two_m15->Fill(params.fMinv);
			if (EMT_multi_two[a] == 16) gr_EMTMulti_InvMass_two_m16->Fill(params.fMinv);
			if (EMT_multi_two[a] == 17) gr_EMTMulti_InvMass_two_m17->Fill(params.fMinv);
			if (EMT_multi_two[a] == 18) gr_EMTMulti_InvMass_two_m18->Fill(params.fMinv);
			if (EMT_multi_two[a] == 19) gr_EMTMulti_InvMass_two_m19->Fill(params.fMinv);
			if (EMT_multi_two[a] == 20) gr_EMTMulti_InvMass_two_m20->Fill(params.fMinv);
		}
	}

	// zero 
	Int_t nof_zero = EMT_Event_multi_zero.size();
	cout << "MixedEvent - nof entries zero " << nof_zero << endl;
	for(Int_t a = 0; a < nof_zero-1; a++) {
		for(Int_t b = a+1; b < nof_zero; b++) {
			if (EMT_Event_multi_zero[a] == EMT_Event_multi_zero[b]) continue;		// to make sure that the photons are from two different events
			if (EMT_multi_zero[a] != EMT_multi_zero[b]) continue;		// check same multiplicity
			TVector3 e11 = EMT_pair_momenta_multi_zero[a][0];
			TVector3 e12 = EMT_pair_momenta_multi_zero[a][1];
			TVector3 e21 = EMT_pair_momenta_multi_zero[b][0];
			TVector3 e22 = EMT_pair_momenta_multi_zero[b][1];
			CbmLmvmKinematicParams params = CalculateKinematicParams_4particles(e11, e12, e21, e22);

			if (EMT_multi_zero[a] == 1) gr_EMTMulti_InvMass_zero_m1->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 2) gr_EMTMulti_InvMass_zero_m2->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 3) gr_EMTMulti_InvMass_zero_m3->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 4) gr_EMTMulti_InvMass_zero_m4->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 5) gr_EMTMulti_InvMass_zero_m5->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 6) gr_EMTMulti_InvMass_zero_m6->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 7) gr_EMTMulti_InvMass_zero_m7->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 8) gr_EMTMulti_InvMass_zero_m8->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 9) gr_EMTMulti_InvMass_zero_m9->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 10) gr_EMTMulti_InvMass_zero_m10->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 11) gr_EMTMulti_InvMass_zero_m11->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 12) gr_EMTMulti_InvMass_zero_m12->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 13) gr_EMTMulti_InvMass_zero_m13->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 14) gr_EMTMulti_InvMass_zero_m14->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 15) gr_EMTMulti_InvMass_zero_m15->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 16) gr_EMTMulti_InvMass_zero_m16->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 17) gr_EMTMulti_InvMass_zero_m17->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 18) gr_EMTMulti_InvMass_zero_m18->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 19) gr_EMTMulti_InvMass_zero_m19->Fill(params.fMinv);
			if (EMT_multi_zero[a] == 20) gr_EMTMulti_InvMass_zero_m20->Fill(params.fMinv);
		}
	}

	// onetwo 
	Int_t nof_onetwo = EMT_Event_multi_onetwo.size();
	cout << "MixedEvent - nof entries onetwo " << nof_onetwo << endl;
	for(Int_t a = 0; a < nof_onetwo-1; a++) {
		for(Int_t b = a+1; b < nof_onetwo; b++) {
			if (EMT_Event_multi_onetwo[a] == EMT_Event_multi_onetwo[b]) continue;		// to make sure that the photons are from two different events
			if (EMT_multi_onetwo[a] != EMT_multi_onetwo[b]) continue;		// check same multiplicity
			TVector3 e11 = EMT_pair_momenta_multi_onetwo[a][0];
			TVector3 e12 = EMT_pair_momenta_multi_onetwo[a][1];
			TVector3 e21 = EMT_pair_momenta_multi_onetwo[b][0];
			TVector3 e22 = EMT_pair_momenta_multi_onetwo[b][1];
			CbmLmvmKinematicParams params = CalculateKinematicParams_4particles(e11, e12, e21, e22);

			if (EMT_multi_onetwo[a] == 1) gr_EMTMulti_InvMass_onetwo_m1->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 2) gr_EMTMulti_InvMass_onetwo_m2->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 3) gr_EMTMulti_InvMass_onetwo_m3->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 4) gr_EMTMulti_InvMass_onetwo_m4->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 5) gr_EMTMulti_InvMass_onetwo_m5->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 6) gr_EMTMulti_InvMass_onetwo_m6->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 7) gr_EMTMulti_InvMass_onetwo_m7->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 8) gr_EMTMulti_InvMass_onetwo_m8->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 9) gr_EMTMulti_InvMass_onetwo_m9->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 10) gr_EMTMulti_InvMass_onetwo_m10->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 11) gr_EMTMulti_InvMass_onetwo_m11->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 12) gr_EMTMulti_InvMass_onetwo_m12->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 13) gr_EMTMulti_InvMass_onetwo_m13->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 14) gr_EMTMulti_InvMass_onetwo_m14->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 15) gr_EMTMulti_InvMass_onetwo_m15->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 16) gr_EMTMulti_InvMass_onetwo_m16->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 17) gr_EMTMulti_InvMass_onetwo_m17->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 18) gr_EMTMulti_InvMass_onetwo_m18->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 19) gr_EMTMulti_InvMass_onetwo_m19->Fill(params.fMinv);
			if (EMT_multi_onetwo[a] == 20) gr_EMTMulti_InvMass_onetwo_m20->Fill(params.fMinv);
		}
	}

}


