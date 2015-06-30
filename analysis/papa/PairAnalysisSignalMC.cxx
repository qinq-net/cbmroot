/*************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 **************************************************************************/

///////////////////////////////////////////////////////////////////////////
//                PairAnalysis MC signal description container             //
//                                                                       //
//                                                                       //
/*
 * A container to describe the decay of a two body process
 * 
 * 
 * 
 * 
 * 
 */
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include "PairAnalysisSignalMC.h"
#include "PairAnalysisHelper.h"

ClassImp(PairAnalysisSignalMC)

const char* PairAnalysisSignalMC::fgkSignals[kNSignals][2]= {  //default signal names+titles
  {"InclJpsi",       "J/#psi#rightarrow e^{+}e^{-}"},
  {"Conversion",     "#gamma#rightarrow e^{+}e^{-}"},
  {"Rho0",           "#rho^{0}"},
  {"Omega",          "#omega"},
  {"OmegaDalitz",    "#omega^{Dalitz}"},
  {"Phi",            "#phi"},
  {"Eta",            "#eta"},
  {"Pi0",            "#pi^{0}"},
  {"InclElePM",      "e^{+}e^{-} (incl.)"},
  {"DeltaElectron",  "#delta rays"},
  {"PrimElectron",   "e (prim.)"},
  {"PrimMuon",       "#mu (prim.)"},
  {"PrimPion",       "#pi (prim.)"},
  {"PrimKaon",       "K (prim.)"},
  {"PrimProton",     "p (prim.)"},
  {"Deuteron",       "d"},
  {"Triton",         "t"},
  {"Helion",         "^{3}He"},
  {"Alpha",          "^{4}He"}
};

//_________________________________________________________________________
PairAnalysisSignalMC::PairAnalysisSignalMC() :
  TNamed("PairAnalysisSignalMC", "PairAnalysisSignalMC"),
  fLeg1(0),
  fLeg2(0),
  fMother1(0),
  fMother2(0),
  fGrandMother1(0),
  fGrandMother2(0),
  fLeg1Exclude(kFALSE),      
  fLeg2Exclude(kFALSE),         
  fMother1Exclude(kFALSE),       
  fMother2Exclude(kFALSE),   
  fGrandMother1Exclude(kFALSE),
  fGrandMother2Exclude(kFALSE),
  fLeg1Source(kDontCare),
  fLeg2Source(kDontCare),
  fMother1Source(kDontCare),
  fMother2Source(kDontCare),
  fGrandMother1Source(kDontCare),
  fGrandMother2Source(kDontCare),
  fCheckBothChargesLeg1(kFALSE),
  fCheckBothChargesLeg2(kFALSE),
  fCheckBothChargesMother1(kFALSE),
  fCheckBothChargesMother2(kFALSE),
  fCheckBothChargesGrandMother1(kFALSE),
  fCheckBothChargesGrandMother2(kFALSE),
  fCheckGEANTProcess(kFALSE),
  fMothersRelation(kUndefined),
  fGEANTProcess(kPPrimary),
  fWeight(1.),
  fJpsiRadiative(kAll),
  fDalitz(kWhoCares),
  fDalitzPdg(0),
  fFillPureMCStep(kFALSE),
  fIsSingleParticle(kFALSE)
{

  //
  // Default constructor
  //
}


//_________________________________________________________________________
PairAnalysisSignalMC::PairAnalysisSignalMC(const Char_t* name, const Char_t* title) :
  TNamed(name, title),
  fLeg1(0),
  fLeg2(0),
  fMother1(0),
  fMother2(0),
  fGrandMother1(0),
  fGrandMother2(0),
  fLeg1Exclude(kFALSE),      
  fLeg2Exclude(kFALSE),         
  fMother1Exclude(kFALSE),       
  fMother2Exclude(kFALSE),   
  fGrandMother1Exclude(kFALSE),
  fGrandMother2Exclude(kFALSE),
  fLeg1Source(kDontCare),
  fLeg2Source(kDontCare),
  fMother1Source(kDontCare),
  fMother2Source(kDontCare),
  fGrandMother1Source(kDontCare),
  fGrandMother2Source(kDontCare),
  fCheckBothChargesLeg1(kFALSE),
  fCheckBothChargesLeg2(kFALSE),
  fCheckBothChargesMother1(kFALSE),
  fCheckBothChargesMother2(kFALSE),
  fCheckBothChargesGrandMother1(kFALSE),
  fCheckBothChargesGrandMother2(kFALSE),
  fCheckGEANTProcess(kFALSE),
  fMothersRelation(kUndefined),
  fGEANTProcess(kPPrimary),
  fWeight(1.),
  fJpsiRadiative(kAll),
  fDalitz(kWhoCares),
  fDalitzPdg(0),
  fFillPureMCStep(kFALSE),
  fIsSingleParticle(kFALSE)
{

  //
  // Named constructor
  //
}

//_________________________________________________________________________
PairAnalysisSignalMC::PairAnalysisSignalMC(EDefinedSignal defaultSignal) :
  TNamed(),
  fLeg1(0),
  fLeg2(0),
  fMother1(0),
  fMother2(0),
  fGrandMother1(0),
  fGrandMother2(0),
  fLeg1Exclude(kFALSE),      
  fLeg2Exclude(kFALSE),         
  fMother1Exclude(kFALSE),       
  fMother2Exclude(kFALSE),   
  fGrandMother1Exclude(kFALSE),
  fGrandMother2Exclude(kFALSE),
  fLeg1Source(kDontCare),
  fLeg2Source(kDontCare),
  fMother1Source(kDontCare),
  fMother2Source(kDontCare),
  fGrandMother1Source(kDontCare),
  fGrandMother2Source(kDontCare),
  fCheckBothChargesLeg1(kFALSE),
  fCheckBothChargesLeg2(kFALSE),
  fCheckBothChargesMother1(kFALSE),
  fCheckBothChargesMother2(kFALSE),
  fCheckBothChargesGrandMother1(kFALSE),
  fCheckBothChargesGrandMother2(kFALSE),
  fCheckGEANTProcess(kFALSE),
  fMothersRelation(kUndefined),
  fGEANTProcess(kPPrimary),
  fWeight(1.),
  fJpsiRadiative(kAll),
  fDalitz(kWhoCares),
  fDalitzPdg(0),
  fFillPureMCStep(kFALSE),
  fIsSingleParticle(kFALSE)
{

  //
  // Constructor with default signals
  //
  TString title="";
  switch(defaultSignal) {
  case kInclJpsi:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;  fLeg2=-11; fCheckBothChargesLeg1=kTRUE; fCheckBothChargesLeg2=kTRUE;
    fMother1=443; fMother2=443; fCheckBothChargesMother1=kTRUE; fCheckBothChargesMother2=kTRUE;
    fMothersRelation=kSame;
    break;
  case kConversion:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;    fLeg2=-11;   fCheckBothChargesLeg1=kTRUE;    fCheckBothChargesLeg2=kTRUE;
    fMother1=22; fMother2=22; fCheckBothChargesMother1=kTRUE; fCheckBothChargesMother2=kTRUE;
    fMother1Source=kSecondary; fMother2Source=kSecondary;
    fMothersRelation=kSame;
    SetGEANTProcess(kPPair);
    break;
  case kRho0:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;    fLeg2=-11;   fCheckBothChargesLeg1=kTRUE;    fCheckBothChargesLeg2=kTRUE;
    fMother1=113; fMother2=113; fCheckBothChargesMother1=kTRUE; fCheckBothChargesMother2=kTRUE;
    fMothersRelation=kSame;
    SetGEANTProcess(kPPrimary); //pluto
    break;
  case kOmega:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;    fLeg2=-11;   fCheckBothChargesLeg1=kTRUE;    fCheckBothChargesLeg2=kTRUE;
    fMother1=223; fMother2=223; fCheckBothChargesMother1=kTRUE; fCheckBothChargesMother2=kTRUE;
    fMothersRelation=kSame;
    fDalitz=kIsNotDalitz; fDalitzPdg=111;
    SetGEANTProcess(kPPrimary); //pluto
    break;
  case kOmegaDalitz:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;    fLeg2=-11;   fCheckBothChargesLeg1=kTRUE;    fCheckBothChargesLeg2=kTRUE;
    fMother1=223; fMother2=223;
    fMothersRelation=kSame;
    fDalitz=kIsDalitz; fDalitzPdg=111;
    SetGEANTProcess(kPPrimary); //pluto
    break;
  case kPhi:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;    fLeg2=-11;   fCheckBothChargesLeg1=kTRUE;    fCheckBothChargesLeg2=kTRUE;
    fMother1=333; fMother2=333; fCheckBothChargesMother1=kTRUE; fCheckBothChargesMother2=kTRUE;
    fMothersRelation=kSame;
    SetGEANTProcess(kPPrimary); //pluto
    break;
  case kEta:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;    fLeg2=-11;   fCheckBothChargesLeg1=kTRUE;    fCheckBothChargesLeg2=kTRUE;
    fMother1=221; fMother2=221; fCheckBothChargesMother1=kTRUE; fCheckBothChargesMother2=kTRUE;
    fMothersRelation=kSame;
    //    SetGEANTProcess(kPUserDefined);
    break;
  case kPi0:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;    fLeg2=-11;   fCheckBothChargesLeg1=kTRUE;    fCheckBothChargesLeg2=kTRUE;
    fMother1=111; fMother2=111; fCheckBothChargesMother1=kTRUE; fCheckBothChargesMother2=kTRUE;
    fMothersRelation=kSame;
    //    SetGEANTProcess(kPUserDefined);
    break;
  case kInclElePM:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;    fLeg2=-11;   fCheckBothChargesLeg1=kTRUE;    fCheckBothChargesLeg2=kTRUE;
    fMother1Source=kPrimary; fMother2Source=kPrimary;
    //    fMothersRelation=kSame;
    break;
  case kDeltaElectron: //single delta ray electrons
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    SetGEANTProcess(kPDeltaRay);
    fIsSingleParticle=kTRUE;
    //    fMother1Source=kPrimary; fMother2Source=kDontCare;
    break;
  case kPrimElectron: //single electrons
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=11;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  case kPrimMuon: //single muoons
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=13;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  case kPrimPion: //single pions
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=211;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  case kPrimKaon: //single kaons
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=321;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  case kPrimProton: //single protons
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=2212;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  case kDeuteron:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=1000010020;;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    //SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  case kTriton:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=1000010030;;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    //SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  case kHe3:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=1000020030;;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    //SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  case kAlpha:
    SetNameTitle(fgkSignals[defaultSignal][0],fgkSignals[defaultSignal][1]);
    fLeg1=1000020040;;  fLeg2=1; fCheckBothChargesLeg1=kTRUE;
    //fMother1Source=kPrimary;
    //SetGEANTProcess(kPPrimary);
    fIsSingleParticle=kTRUE;
    break;
  default:
    Error("PairAnalysisSignalMC","Signal NOT predefined! Either request it or configure on your own.");
    return;
    break;
  }
  //  Info("PairAnalysisSignalMC","Use predefined signal selected: %s\n",GetName());

  // TODO: add print command showing the signal configuration
  // e.g.: (add charge, source/origin, geant process)
  // grandMothersPDG1 -> mothersPDG1 -> daughtersPDG1
  //                     | relation
  // grandMothersPDG2 -> mothersPDG2 -> !(daughtersPDG2)

}



//_________________________________________________________________________
PairAnalysisSignalMC::~PairAnalysisSignalMC() {
  //
  //  Destructor
  //
}
