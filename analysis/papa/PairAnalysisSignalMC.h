#ifndef PAIRANALYSISSIGNALMC_H
#define PAIRANALYSISSIGNALMC_H

/* Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

#include <TNamed.h>
#include <TMCProcess.h>
#include <TSpline.h>
//#include <TGraph.h>
//#include <TF1.h>


//__________________________________________________________________
class PairAnalysisSignalMC : public TNamed {
  
 public:
  enum EBranchRelation {kUndefined=0, kSame, kDifferent};
  enum ESource {kDontCare=0, kPrimary, kFinalState, kDirect, kSecondary, kNoCocktail, kSecondaryFromWeakDecay, kSecondaryFromMaterial};
  enum EJpsiRadiativ {kAll=0, kIsRadiative, kIsNotRadiative};
  enum EDalitz {      kWhoCares=0, kIsDalitz, kIsNotDalitz};
  enum EDefinedSignal {kInclJpsi=0, kNonRadJpsi, kRadJpsi, kPsi2S,
		       kConversion,
		       kRho0, kOmegaDalitz, kOmega, kPhi,
		       kEtaDalitz, kEta,
		       kPi0Dalitz, kPi0Gamma, kPi0,
		       kInclElePM,
		       kDeltaElectron,
		       kPrimElectron, kPrimMuon, kPrimPion, kPrimKaon, kPrimProton,
		       kDeuteron, kTriton, kHe3, kAlpha,
		       kNSignals};
  
  PairAnalysisSignalMC();
  PairAnalysisSignalMC(const Char_t* name, const Char_t* title);
  PairAnalysisSignalMC(EDefinedSignal defaultSignal);
  virtual ~PairAnalysisSignalMC();
  
  void SetLegPDGs(Int_t pdg1, Int_t pdg2, Bool_t exclude1=kFALSE, Bool_t exclude2=kFALSE)
    {fLeg1 = pdg1; fLeg2 = pdg2; fLeg1Exclude=exclude1; fLeg2Exclude=exclude2;}
  void SetMotherPDGs(Int_t pdg1, Int_t pdg2, Bool_t exclude1=kFALSE, Bool_t exclude2=kFALSE)
    {fMother1 = pdg1; fMother2 = pdg2; fMother1Exclude=exclude1; fMother2Exclude=exclude2;}
  void SetGrandMotherPDGs(Int_t pdg1, Int_t pdg2, Bool_t exclude1=kFALSE, Bool_t exclude2=kFALSE)
    {fGrandMother1 = pdg1; fGrandMother2 = pdg2; fGrandMother1Exclude=exclude1; fGrandMother2Exclude=exclude2;}
  void SetGreatGrandMotherPDGs(Int_t pdg1, Int_t pdg2, Bool_t exclude1=kFALSE, Bool_t exclude2=kFALSE)
    {fGreatGrandMother1 = pdg1; fGreatGrandMother2 = pdg2; fGreatGrandMother1Exclude=exclude1; fGreatGrandMother2Exclude=exclude2;}
  void SetLegSources(ESource s1, ESource s2)                       {fLeg1Source = s1;                      fLeg2Source = s2;}
  void SetMotherSources(ESource s1, ESource s2)                    {fMother1Source = s1;                   fMother2Source = s2;}
  void SetGrandMotherSources(ESource s1, ESource s2)               {fGrandMother1Source = s1;              fGrandMother2Source = s2;}
  void SetCheckBothChargesLegs(Bool_t flag1, Bool_t flag2)         {fCheckBothChargesLeg1 = flag1;         fCheckBothChargesLeg2 = flag2;}
  void SetCheckBothChargesMothers(Bool_t flag1, Bool_t flag2)      {fCheckBothChargesMother1 = flag1;      fCheckBothChargesMother2 = flag2;}
  void SetCheckBothChargesGrandMothers(Bool_t flag1, Bool_t flag2) {fCheckBothChargesGrandMother1 = flag1; fCheckBothChargesGrandMother2 = flag2;}
  void SetMothersRelation(EBranchRelation relation)                {fMothersRelation = relation;}
  void SetGEANTProcess(TMCProcess processID, Bool_t check=kTRUE)  {fGEANTProcess = processID; fCheckGEANTProcess=check;}
  //  void SetWeight(TSpline3 *wghts, UInt_t var)                      { if(&fWeights) delete &fWeights; fWeights = *wghts; fType=var; }
  void SetWeight(TSpline3 *wghts, UInt_t var)                      { if(fWeights) delete fWeights; fWeights = wghts; fType=var; }
  void SetWeight(Double_t wght)                                    {fWeight=wght;}
  void SetFillPureMCStep(Bool_t fill=kTRUE)                        {fFillPureMCStep = fill;}
  void SetIsSingleParticle(Bool_t fill=kTRUE)                      {fIsSingleParticle = fill;}

  Int_t GetLegPDG(Int_t branch)                        const {return (branch==1 ? fLeg1 : fLeg2);}
  Int_t GetMotherPDG(Int_t branch)                     const {return (branch==1 ? fMother1 : fMother2);}
  Int_t GetGrandMotherPDG(Int_t branch)                const {return (branch==1 ? fGrandMother1 : fGrandMother2);}
  Int_t GetGreatGrandMotherPDG(Int_t branch)                const {return (branch==1 ? fGreatGrandMother1 : fGreatGrandMother2);}
  Bool_t GetLegPDGexclude(Int_t branch)                const {return (branch==1 ? fLeg1Exclude : fLeg2Exclude);}
  Bool_t GetMotherPDGexclude(Int_t branch)             const {return (branch==1 ? fMother1Exclude : fMother2Exclude);}
  Bool_t GetGrandMotherPDGexclude(Int_t branch)        const {return (branch==1 ? fGrandMother1Exclude : fGrandMother2Exclude);}
  Bool_t GetGreatGrandMotherPDGexclude(Int_t branch)        const {return (branch==1 ? fGreatGrandMother1Exclude : fGreatGrandMother2Exclude);}
  ESource GetLegSource(Int_t branch)                   const {return (branch==1 ? fLeg1Source : fLeg2Source);}
  ESource GetMotherSource(Int_t branch)                const {return (branch==1 ? fMother1Source : fMother2Source);}
  ESource GetGrandMotherSource(Int_t branch)           const {return (branch==1 ? fGrandMother1Source : fGrandMother2Source);}
  Bool_t GetCheckBothChargesLegs(Int_t branch)         const {return (branch==1 ? fCheckBothChargesLeg1 : fCheckBothChargesLeg2);}
  Bool_t GetCheckBothChargesMothers(Int_t branch)      const {return (branch==1 ? fCheckBothChargesMother1 : fCheckBothChargesMother2);}
  Bool_t GetCheckBothChargesGrandMothers(Int_t branch) const {return (branch==1 ? fCheckBothChargesGrandMother1 : fCheckBothChargesGrandMother2);}
  Bool_t GetCheckBothChargesGreatGrandMothers(Int_t branch) const {return (branch==1 ? fCheckBothChargesGreatGrandMother1 : fCheckBothChargesGreatGrandMother2);}
  EBranchRelation GetMothersRelation()                 const {return fMothersRelation;}
  TMCProcess GetGEANTProcess()                         const {return fGEANTProcess;}
  Bool_t GetCheckGEANTProcess()                        const {return fCheckGEANTProcess;}
  Double_t GetWeight()                                 const {return fWeight;}
  //  Double_t GetWeight(Double_t *const values)           const {return (&fWeights ? fWeights.Eval(values[fType]) : fWeight); }
  //  Double_t GetWeight(Double_t *const values)           const {return (fType>0 ? fWeights->Eval(values[fType]) : fWeight); }
  // TODO: workaround
  Double_t GetWeight(Double_t *const values)           const;// {return (fType>0 ? fWeights->Eval(values[fType]) : fWeight); }
  Bool_t GetFillPureMCStep()                           const {return fFillPureMCStep;}
  Bool_t IsSingleParticle()                            const {return fIsSingleParticle; }

  void SetJpsiRadiative(EJpsiRadiativ rad) { fJpsiRadiative=rad;    }
  EJpsiRadiativ GetJpsiRadiative() const   { return fJpsiRadiative; }

  void SetIsDalitz(EDalitz dal, Int_t pdg=0) { fDalitz=dal; fDalitzPdg=pdg;   }
  EDalitz GetDalitz() const   { return fDalitz; }
  Int_t   GetDalitzPdg()   const   { return fDalitzPdg; }

  static const char* fgkSignals[kNSignals][2];  //default signal names+titles

  void Print(Option_t * option = "") const;

 private:
  // PDG codes for legs, mothers and grand-mothers
  Int_t fLeg1;                        // leg 1 PDG
  Int_t fLeg2;                        // leg 2 PDG
  Int_t fMother1;                     // mother 1 PDG
  Int_t fMother2;                     // mother 2 PDG
  Int_t fGrandMother1;                // grandmother 1 PDG
  Int_t fGrandMother2;                // grandmother 2 PDG
  Int_t fGreatGrandMother1;           // great grandmother 1 PDG
  Int_t fGreatGrandMother2;           // great grandmother 2 PDG

  // Toggle on/off the use of the PDG codes as inclusion or exclusion
  // Example: if fLeg1=211 and fLeg1Exclude=kTRUE than all codes will be accepted for leg 1 with
  //          the exception of 211 (pions)
  Bool_t fLeg1Exclude;                // leg 1 
  Bool_t fLeg2Exclude;                // leg 2 
  Bool_t fMother1Exclude;             // mother 1 
  Bool_t fMother2Exclude;             // mother 2 
  Bool_t fGrandMother1Exclude;        // grandmother 1 
  Bool_t fGrandMother2Exclude;        // grandmother 2 
  Bool_t fGreatGrandMother1Exclude;   // great grandmother 1 
  Bool_t fGreatGrandMother2Exclude;   // great grandmother 2 
    
  // Particle sources
  ESource fLeg1Source;                // leg 1 source
  ESource fLeg2Source;                // leg 2 source
  ESource fMother1Source;             // mother 1 source
  ESource fMother2Source;             // mother 2 source
  ESource fGrandMother1Source;        // grandmother 1 source
  ESource fGrandMother2Source;        // grandmother 2 source

  // Flaggs whether to check both charges of a given PDG code
  Bool_t fCheckBothChargesLeg1;         // check both charges of the legs pdg
  Bool_t fCheckBothChargesLeg2;         //                leg2
  Bool_t fCheckBothChargesMother1;      //                mother 1
  Bool_t fCheckBothChargesMother2;      //                mother 2
  Bool_t fCheckBothChargesGrandMother1; //              grand mother 1
  Bool_t fCheckBothChargesGrandMother2; //              grand mother 2
  Bool_t fCheckBothChargesGreatGrandMother1; //         great grand mother 1
  Bool_t fCheckBothChargesGreatGrandMother2; //         great grand mother 2
  Bool_t fCheckGEANTProcess;            //              GEANT process

  EBranchRelation fMothersRelation;   // mother 1&2 relation (same, different or whatever)
  TMCProcess fGEANTProcess;           // GEANT process ID (see roots TMCProcess)
  Double_t fWeight;                   // weighting factor
  //  TSpline3   fWeights;                   //> weighting factors
  TSpline3   *fWeights;                   //> weighting factors
  UInt_t   fType;                   // lookup variable for weighting factors
  EJpsiRadiativ fJpsiRadiative;       // check for J/psi radiative decay

  // dalitz decays
  EDalitz fDalitz;                      // check for dalitz decay
  Int_t fDalitzPdg;                     // dalitz PDG

  Bool_t fFillPureMCStep;             // check and fill the pure MC step
  Bool_t fIsSingleParticle;           // single particle MC signal such as e,pi,K,p

  PairAnalysisSignalMC(const PairAnalysisSignalMC &c);
  PairAnalysisSignalMC &operator=(const PairAnalysisSignalMC &c);
  
  ClassDef(PairAnalysisSignalMC,5); // MC signal defintions
};

#endif
