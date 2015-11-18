/* Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 * based on the ALICE-papa package                                  */

///////////////////////////////////////////////////////////////////////////
//                PairAnalysis Variables Manager class                     //
//                                                                       //
/*

*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include "PairAnalysisVarManager.h"

ClassImp(PairAnalysisVarManager)

const char* PairAnalysisVarManager::fgkParticleNames[PairAnalysisVarManager::kNMaxValuesMC][3] = {
  {"None",                   "",                                                   ""},
// Constant information
  {"MEL",                    "m_{e}",                                              "(GeV/#it{c^{2}})"},
  {"MMU",                    "m_{#mu}",                                            "(GeV/#it{c^{2}})"},
  {"MPI",                    "m_{#pi}",                                            "(GeV/#it{c^{2}})"},
  {"MKA",                    "m_{K}",                                              "(GeV/#it{c^{2}})"},
  {"MPR",                    "m_{p}",                                              "(GeV/#it{c^{2}})"},
// Hit specific variables
  {"PosX",                   "x_{hit}",                                            "(cm)"},
  {"PosY",                   "y_{hit}",                                            "(cm)"},
  {"PosZ",                   "z_{hit}",                                            "(cm)"},
  {"LinksMC",                "N_{links}^{MC}",                                     ""},
  {"Eloss",                  "d#it{E}/d#it{x} + TR",                           "(keV#upointcm^{2}/g)"},
  {"ElossdEdx",              "d#it{E}/d#it{x}",                                "(keV#upointcm^{2}/g)"},
  {"ElossTR",                "TR",                                             "(keV#upointcm^{2}/g)"},
  {"NPhotons",               "N_{photons}^{RICH}",                                 "(a.u.)"},
  {"PmtId",                  "ID_{Pmt}^{RICH}",                                    "(a.u.)"},
  {"Beta",                   "#beta^{TOF}",                                        ""},
  {"TOFPidDeltaBetaEL",      "#beta^{TOF}-#beta^{TOF}_{e}",                        ""},
  {"TOFPidDeltaBetaMU",      "#beta^{TOF}-#beta^{TOF}_{#mu}",                      ""},
  {"TOFPidDeltaBetaPI",      "#beta^{TOF}-#beta^{TOF}_{#pi}",                      ""},
  {"TOFPidDeltaBetaKA",      "#beta^{TOF}-#beta^{TOF}_{K}",                        ""},
  {"TOFPidDeltaBetaPR",      "#beta^{TOF}-#beta^{TOF}_{p}",                        ""},
  {"MassSq",                 "m^{2} (TOF)",                                        ""},
// Particle specific variables
  {"Px",                     "#it{p}_{x}",                                         "(GeV/#it{c})"},
  {"Py",                     "#it{p}_{y}",                                         "(GeV/#it{c})"},
  {"Pz",                     "#it{p}_{z}",                                         "(GeV/#it{c})"},
  {"Pt",                     "#it{p}_{T}",                                         "(GeV/#it{c})"},
  {"PtSq",                   "#it{p}_{T}^{2}",                                     "((GeV/#it{c})^{2})"},
  {"P",                      "#it{p}",                                             "(GeV/#it{c})"},
  {"Xv",                     "x_{vtx}",                                            "(cm)"},
  {"Yv",                     "y_{vtx}",                                            "(cm)"},
  {"Zv",                     "z_{vtx}",                                            "(cm)"},
  {"OneOverPt",              "1/^{}#it{p}_{T}",                                    "(GeV/#it{c})^{-1}"},
  {"Phi",                    "#phi",                                               "(rad.)"},
  {"Theta",                  "#theta",                                             "(rad.)"},
  {"Eta",                    "#eta",                                               ""},
  {"Y",                      "#it{y}",                                             ""},
  {"Ylab",                   "#it{y}_{lab}",                                       ""},
  {"E",                      "E",                                                  "(GeV)"},
  {"M",                      "m_{inv}",                                            "(GeV/#it{c^{2}})"},
  {"Charge",                 "q",                                                  "(e)"},
  {"Chi2NDFtoVtx",           "#chi^{2}/^{}N_{df}^{track#leftrightarrowvtx}",      "(#sigma)"},
  {"ImpactParXY",            "#it{dca}_{xy}",                                      "(cm)"},
  //  {"ImpactParZ",             "#it{dca}_{z}",                                       "(cm)"},
  {"InclAngle",              "#alpha_{incl}",                                      "(rad.)"},
// Track specific variables
  // global track
  {"TrackLength",            "#it{l}_{track}",                                     "(cm)"},
  {"TrackChi2NDF",           "#chi^{2}/^{}N_{df}^{glbl}",                          ""},
  {"Pin",                    "#it{p}^{in}",                                        "(GeV/#it{c})"},
  {"Ptin",                   "#it{p}_{T}^{in}",                                    "(GeV/#it{c})"},
  {"Pout",                   "#it{p}^{out}",                                       "(GeV/#it{c})"},
  {"Ptout",                  "#it{p}_{T}^{out}",                                   "(GeV/#it{c})"},
  // trd track information
  {"TRDSignal",              "TRD d#it{E}/d#it{x} + TR",                           "(keV#upointcm^{2}/g})"},
  {"TRDPidWkn",              "PID_{e}^{TRD} (Wkn)",                                ""},
  {"TRDPidANN",              "PID_{e}^{TRD} (ANN)",                                ""},
  {"TRDPidLikeEL",           "P(PID_{e}^{TRD})",                                   ""},
  {"TRDPidLikePI",           "P(PID_{#pi}^{TRD})",                                 ""},
  {"TRDPidLikeKA",           "P(PID_{K}^{TRD})",                                   ""},
  {"TRDPidLikePR",           "P(PID_{p}^{TRD})",                                   ""},
  {"TRDPidLikeMU",           "P(PID_{#muon}^{TRD})",                               ""},
  {"TRDHits",                "N_{hit}^{TRD}",                                      ""},
  {"TRDChi2NDF",             "#chi^{2}/^{}N_{df}^{TRD}",                           ""},
  {"TRDPin",                 "#it{p}^{TRDin}",                                     "(GeV/#it{c})"},
  {"TRDPtin",                "#it{p}_{T}^{TRDin}",                                 "(GeV/#it{c})"},
  {"TRDPout",                "#it{p}^{TRDout}",                                    "(GeV/#it{c})"},
  {"TRDPtout",               "#it{p}_{T}^{TRDout}",                                "(GeV/#it{c})"},
  // sts track information
  {"MVDHits",                "N_{hit}^{MVD}",                                      ""},
  {"MVDFirstHitPosZ",        "z_{first hit}^{MVD}",                                "(cm)"},
  {"ImpactParZ",             "#it{dca}_{z}/#Delta#it{dca}_{z}",                    "#sigma"},
  {"STSHits",                "N_{hit}^{STS}",                                      ""},
  {"STSChi2NDF",             "#chi^{2}/^{}N_{df}^{STS}",                           ""},
  {"STSPin",                 "#it{p}^{STSin}",                                     "(GeV/#it{c})"},
  {"STSPtin",                "#it{p}_{T}^{STSin}",                                 "(GeV/#it{c})"},
  {"STSPout",                "#it{p}^{STSout}",                                    "(GeV/#it{c})"},
  {"STSPtout",               "#it{p}_{T}^{STSout}",                                "(GeV/#it{c})"},
  {"STSXv",                  "x_{STS}",                                            "(cm)"},
  {"STSYv",                  "y_{STS}",                                            "(cm)"},
  {"STSZv",                  "z_{STS}",                                            "(cm)"},
  {"STSChi2NDFtoVtx",        "#chi^{2}/^{}N_{df}^{STS#leftrightarrowvtx}",         "(#sigma)"},
  {"STSFirstHitPosZ",        "z_{first hit}^{STS}",                                "(cm)"},
  //RICH track specific information
  {"RICHPidANN",             "PID_{e}^{RICH} (ANN)",                               ""},
  {"RICHHitsOnRing",         "N_{hit on Ring}^{RICH}",                             ""},
  {"RICHHits",               "N_{hit}^{RICH}",                                     ""},
  {"RICHChi2NDF",            "#chi^{2}/^{}N_{df}^{RICH}",                          ""},
  {"RICHRadius",             "r_{}^{RICH}",                                        "(cm)"},
  //TOF track specific information
  {"TOFHits",                "N_{hit}^{TOF}",                                      ""},
  // MUCH track information
  {"MUCHHits",               "N_{hit}^{MUCH}",                                     ""},
  {"MUCHHitsPixel",          "N_{pixel hit}^{MUCH}",                               ""},
  {"MUCHHitsStraw",          "N_{strip hit}^{MUCH}",                               ""},
  {"MUCHChi2NDF",            "#chi^{2}/^{}N_{df}^{MUCH}",                          ""},
  // technical information
  {"RndmTrack",               "P",                                                  ""},

// Pair specific variables
  {"Chi2NDF",                "#chi^{2}/^{}ndf",                                    ""},
  {"DecayLength",            "l_{decay}",                                          "(cm)"},
  {"R",                      "d(#vec{x}_{vtx},#vec{x}_{part.})",                   "(cm)"},
  {"OpeningAngle",           "#varphi",                                            "(rad.)"},
  {"CosPointingAngle",       "cos(#theta)",                                        "(rad.)"},
  {"ArmAlpha",               "#alpha^{arm}",                                       ""},
  {"ArmPt",                  "#it{p}_{T}^{arm}",                                   "(GeV/#it{c})"},
  {"ThetaHE",                "cos(#theta_{HE})",                                   ""},
  {"PhiHE",                  "#phi_{HE}",                                          "(rad.)"},
  {"ThetaSqHE",              "cos^{2}(#theta_{HE})",                               ""},
  {"Cos2PhiHE",              "cos(2#phi_{HE})",                                    ""},
  {"CosTilPhiHE",            "cos(#phi_{HE})",                                     ""},
  {"ThetaCS",                "cos(#theta_{CS})",                                   ""},
  {"PhiCS",                  "#phi_{CS}",                                          "(rad.)"},
  {"ThetaSqCS",              "cos^{2}(#theta_{CS})",                               ""},
  {"Cos2PhiCS",              "cos(2#phi_{CS})",                                    ""},
  {"CosTilPhiCS",            "cos(#phi_{CS})",                                     ""},
  {"PsiPair",                "#Psi^{pair}",                                        "(rad.)"},
  {"PhivPair",               "#Phi_{v}^{pair}",                                    "(rad.)"},

  {"LegDistance",            "d^{legs}",                                           "(cm)"},
  {"LegDistanceXY",          "d^{legs}_{xy}",                                      "(cm)"},
  {"DeltaEta",               "#Delta #eta",                                        ""},
  {"DeltaPhi",               "#Delta #phi",                                        ""},
  {"LegsP",                  "#sqrt{#it{p}^{leg1}#upoint#it{p}^{leg2}}",           "(GeV/#it{c})"},
  {"Merr",                   "m_{inv} error",                                      "(GeV/#it{c}^{2})"},
  {"DCA",                    "#it{dca}",                                           "(cm)"},
  {"PairType",               "PairType",                                           ""},
  {"PseudoProperTime",       "#tau",                                               "(#mus)"}, //TODO: check unit
  {"PseudoProperTimeErr",    "#tau error",                                         "(#mus)"},
  {"PseudoProperTimeResolution", "(#tau-#tau^{MC truth})",                         "(#mus)"},
  {"PseudoProperTimePull",   "#frac{(#tau-#tau^{MC truth})}{#tau error}",          ""},
  {"TRDpidEffPair",          "#varepsilon^{TRD pair}(PID)",                           ""},
  {"MomAsymDau1",            "#it{p}^{leg1}/#it{p}^{pair}",                        ""},
  {"MomAsymDau2",            "#it{p}^{leg2}/#it{p}^{pair}",                        ""},
  {"EffPair",                "A#times#varepsilon",                                 ""},
  {"OneOverEffPair",         "(A#times#varepsilon)^{-1}",                          ""},
  {"OneOverEffPairSq",       "(A#times#varepsilon)^{-2}",                          ""},
  {"RndmPair",               "P",                                                  ""},
  {"Pairs",                  "pairs/event",                                        ""},

// Event specific variables
  {"XvPrim",                 "x_{prim.vtx}",                                       "(cm)"},
  {"YvPrim",                 "y_{prim.vtx}",                                       "(cm)"},
  {"ZvPrim",                 "z_{prim.vtx}",                                       "(cm)"},
  {"VtxChi",                 "#chi^{2}_{prim.vtx}",                                ""},
  {"VtxNDF",                 "ndf",                                                ""},
  {"XRes",                   "#Delta x_{prim.vtx}",                                "(cm)"},
  {"YRes",                   "#Delta y_{prim.vtx}",                                "(cm)"},
  {"ZRes",                   "#Delta z_{prim.vtx}",                                "(cm)"},
  {"PhiMaxPt",               "#phi(#it{p}_{T}^{lead})",                            "(rad.)"},
  {"MaxPt",                  "#it{p}_{T}^{lead}",                                  "(GeV/#it{c})"},
  {"NTrk",                   "N_{trk}",                                            ""},
  {"Tracks",                 "tracks/per event",                                   ""},
  {"NVtxContrib",            "N_{vtx. contrib.}",                                  ""},

  {"RefMult",                "N_{trk}^{ref}",                                      ""},
  {"RefMultTPConly",         "N_{trk}^{TPConly}",                                  ""},
  {"Nch",                    "N_{ch} #cbar_{#||{#eta}<1.6}",                       ""},
  {"Nch05",                  "N_{ch} #cbar_{#||{#eta}<0.5}",                       ""},
  {"Nch10",                  "N_{ch} #cbar_{#||{#eta}<1.0}",                       ""},
  {"Centrality",             "centrality_{V0M}",                                  "(%)"},
  {"TriggerInclONL",         "online trigger bit (inclusive)",                     ""},
  {"TriggerInclOFF",         "offline trigger bit (inclusive)",                    ""},
  {"TriggerExclOFF",         "offline trigger bit (exclusive)",                    ""},
  {"Nevents",                "N_{evt}",                                            ""},
  {"RunNumber",              "run",                                                ""},
  {"Ybeam",                  "#it{y}_{beam}",                                      ""},
  {"Ebeam",                  "#sqrt{s}",                                           "(#it{A}GeV)"},
  {"MixingBin",              "mixing bin",                                         ""},
  {"TotalTRDHits",           "N_{tot.hit}^{TRD}",                                  ""},


// MC information
// Hit specific variables
  {"PosXMC",                   "x_{hit}^{MC}",                                            "(cm)"},
  {"PosYMC",                   "y_{hit}^{MC}",                                            "(cm)"},
  {"PosZMC",                   "z_{hit}^{MC}",                                            "(cm)"},
  {"ElossMC",                  "d#it{E}/d#it{x}^{MC}",                                    "(keV#upointcm^{2}/g})"},
// Particle specific variables
  {"PxMC",                     "#it{p}_{x}^{MC}",                                         "(GeV/#it{c})"},
  {"PyMC",                     "#it{p}_{y}^{MC}",                                         "(GeV/#it{c})"},
  {"PzMC",                     "#it{p}_{z}^{MC}",                                         "(GeV/#it{c})"},
  {"PtMC",                     "#it{p}_{T}^{MC}",                                         "(GeV/#it{c})"},
  {"PtSqMC",                   "#it{p}_{T}^{2} (MC)",                                     "((GeV/#it{c})^{2})"},
  {"PMC",                      "#it{p}^{MC}",                                             "(GeV/#it{c})"},
  {"XvMC",                     "x_{vtx}^{MC}",                                            "(cm)"},
  {"YvMC",                     "y_{vtx}^{MC}",                                            "(cm)"},
  {"ZvMC",                     "z_{vtx}^{MC}",                                            "(cm)"},
  {"PhivMC",                   "#phi_{vtx}^{MC}",                                         "(rad.)"},
  {"ThetavMC",                 "#theta_{vtx}^{MC}"                                        "(rad.)"},
  {"OneOverPtMC",              "1/^{}#it{p}_{T}^{MC}",                                    "(GeV/#it{c})^{-1}"},
  {"PhiMC",                    "#phi^{MC}",                                               "(rad.)"},
  {"ThetaMC",                  "#theta^{MC}",                                             "(rad.)"},
  {"EtaMC",                    "#eta^{MC}",                                               ""},
  {"YMC",                      "#it{y}^{MC}",                                             ""},
  {"YlabMC",                   "#it{y}_{lab}^{MC}",                                       ""},
  {"EMC",                      "E^{MC}",                                                  "(GeV)"},
  {"MMC",                      "m_{inv}^{MC}",                                            "(GeV/#it{c^{2}})"},
  {"ChargeMC",                 "q^{MC}",                                                  "(e)"},
  {"PdgCode",                  "PDG code",                                           ""},
  {"PdgCodeMother",            "mothers PDG code",                                   ""},
  {"PdgCodeGrandMother",       "grand mothers PDG code",                             ""},
  {"GeantId",                  "Geant process",                                     ""},
  {"Weight",                   "N#times#it{BR}",                                     ""},


// Track specific variables
  {"TRDHitsMC",                "N_{hit}^{TRD} (MC)",                                      ""},
  {"MVDHitsMC",                "N_{hit}^{MVD} (MC)",                                      ""},
  {"STSHitsMC",                "N_{hit}^{STS} (MC)",                                      ""},
  {"TOFHitsMC",                "N_{hit}^{TOF} (MC)",                                      ""},
  {"MUCHHitsMC",               "N_{hit}^{MUCH} (MC)",                                      ""},
  {"RICHHitsMC",               "N_{hit}^{RICH} (MC)",                                     ""},
  {"TRDMCPoints",              "N_{diff.MCtrk}^{TRD}",                                      ""},
  {"RICHMCPoints",             "N_{MC,pts}^{RICH}",                                      ""},
  {"TRDTrueHits",              "N_{true,hit}^{TRD}",                                      ""},
  {"TRDDistHits",              "N_{distorted,hit}^{TRD}",                                 ""},
  {"TRDFakeHits",              "N_{fake,hit}^{TRD}",                                      ""},
  {"TRDDistortion",            "distortion^{TRD}",                                 ""},
  {"STSTrueHits",              "N_{true,hit}^{STS}",                                      ""},
  {"STSFakeHits",              "N_{fake,hit}^{STS}",                                      ""},
  {"TRDisMC",                  "MC status TRD",                                    ""},
  {"MVDisMC",                  "MC status MVD",                                    ""},
  {"STSisMC",                  "MC status STS",                                    ""},
  {"MUCHisMC",                 "MC status MUCH",                                    ""},
  {"RICHisMC",                 "MC status RICH",                                    ""},
  {"TOFisMC",                  "MC status TOF",                                    ""},
  {"RICHhasProj",              "projection status RICH",                                    ""},

// Pair specific MC variables
  {"OpeningAngleMC",           "#varphi^{MC}",                                     "(rad.)"},

// Event specific MCvariables
  {"NTrkMC",                   "N_{trk}^{MC}",                                       ""},
  {"STSMatches",               "N_{trk.matches}^{STS}",                              ""},
  {"TRDMatches",               "N_{trk.matches}^{TRD}",                              ""},
  {"VageMatches",              "N_{vage.matches}^{STS}",                             ""},
  {"TotalTRDHitsMC",           "N_{tot.hit}^{TRD} (MC)",                             ""},
  {"ImpactParam",              "#it{b}",                                             "(fm)"},
  {"NPrimMC",                  "N_{prim}^{MC}",                                      ""},

};

PairAnalysisEvent* PairAnalysisVarManager::fgEvent           = 0x0;
CbmKFVertex*    PairAnalysisVarManager::fgKFVertex         = 0x0;
//CbmStsKFTrackFitter* PairAnalysisVarManager::fgKFFitter    = 0x0;
//CbmL1PFFitter*  PairAnalysisVarManager::fgL1Fitter    = 0x0;
TBits*          PairAnalysisVarManager::fgFillMap          = 0x0;
Int_t           PairAnalysisVarManager::fgCurrentRun = -1;
Double_t        PairAnalysisVarManager::fgData[PairAnalysisVarManager::kNMaxValuesMC] = {0.};
TFormula*       PairAnalysisVarManager::fgFormula[kNMaxValuesMC] = {0x0};
CbmRichElectronIdAnn* PairAnalysisVarManager::fgRichElIdAnn=0x0;

//________________________________________________________________
PairAnalysisVarManager::PairAnalysisVarManager() :
  TNamed("PairAnalysisVarManager","PairAnalysisVarManager")
{
  //
  // Default constructor
  //
  for(Int_t i=1; i<kNMaxValuesMC; ++i) {
   fgFormula[i]=0x0;
  }

  gRandom->SetSeed();
}

//________________________________________________________________
PairAnalysisVarManager::PairAnalysisVarManager(const char* name, const char* title) :
  TNamed(name,title)
{
  //
  // Named constructor
  //
  for(Int_t i=1; i<kNMaxValuesMC; ++i) {
    fgFormula[i]=0x0;
  }

  gRandom->SetSeed();
}

//________________________________________________________________
PairAnalysisVarManager::~PairAnalysisVarManager()
{
  //
  // Default destructor
  //
  for(Int_t i=1; i<kNMaxValuesMC; ++i) {
    if(fgFormula[i]) delete fgFormula[i];
  }

}

//________________________________________________________________
UInt_t PairAnalysisVarManager::GetValueType(const char* valname)
{
  //
  // Get value type by value name
  //

  TString name(valname);
  for(UInt_t i=0; i<PairAnalysisVarManager::kNMaxValuesMC; i++) {
    if(!name.CompareTo(fgkParticleNames[i][0])) return i;
  }
  return -1;
}

//________________________________________________________________
UInt_t PairAnalysisVarManager::GetValueTypeMC(UInt_t var)
{
  //
  // Get MC value type by standard value name
  //
  // protection against PairAnalysisHistos::kNoAutoFill, kNoProfile, kNoWeights, kNone
  if(var==997 || var==998 ||var==999 || var==1000000000) return var;

  TString valname=GetValueName(var);
  TString name=valname + "MC";

  //  name+="MC";
  for(UInt_t i=0; i<PairAnalysisVarManager::kNMaxValuesMC; i++) {
    if(!name.CompareTo(fgkParticleNames[i][0])) return i;
  }
  return GetValueType(valname);
}

/*
Double_t PairAnalysisVarManager::GetChiToVertex(const CbmStsTrack *track, CbmVertex *vertex)
{
  //
  // Get  vertex chi/ndf between ststrack and vertex
  //

  // KFFitter
  return fgKFFitter->GetChiToVertex(track,vertex);

  // L1PFFitter
  vector<CbmStsTrack> stsTracks;
  stsTracks.resize(1);
  stsTracks[0] = *track;
  vector<L1FieldRegion> vField;
  vector<float> chiPrim;
  fgL1Fitter->GetChiToVertex(stsTracks, vField, chiPrim, fgKFVertex, 3.e+6);
  printf("L1fitter: %f\n", chiPrim[0]);
  return chiPrim[0];

}
*/
