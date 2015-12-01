#ifndef PAIRANALYSISSPECIALCUTS_H
#define PAIRANALYSISSPECIALCUTS_H

//#############################################################
//#                                                           #
//#         Class PairAnalysisSpecialCuts                     #
//#         Provide cuts for using objects                    #
//#                                                           #
//#  Authors:                                                 #
//#   Julian    Book,     Uni Ffm / Julian.Book@cern.ch       #
//#                                                           #
//#############################################################

#include <Rtypes.h>
#include <TBits.h>

#include "AnalysisCuts.h"
#include "PairAnalysisVarManager.h"

class TGraph;
class THnBase;
class PairAnalysisSpecialCuts : public AnalysisCuts {
public:
  // Whether all cut criteria have to be fulfilled of just any
  enum CutType { kAll=0, kAny };
  enum { kNMaxCuts=10 };

  PairAnalysisSpecialCuts();
  PairAnalysisSpecialCuts(const char* name, const char* title);
  virtual ~PairAnalysisSpecialCuts();
  //TODO: make copy constructor and assignment operator public
  void AddCut(PairAnalysisVarManager::ValueTypes type, const char *formulaMin, const char *formulaMax, Bool_t excludeRange=kFALSE);
  void AddCut(const char *formula,                     const char *formulaMin, const char *formulaMax, Bool_t excludeRange=kFALSE);

  void AddCut(PairAnalysisVarManager::ValueTypes type, TGraph *const graphMin, TGraph * const graphMax, Bool_t excludeRange=kFALSE);
  void AddCut(const char *formula,                     TGraph *const graphMin, TGraph * const graphMax, Bool_t excludeRange=kFALSE);

  void AddCut(PairAnalysisVarManager::ValueTypes type, THnBase *const histMin, THnBase * const histMax, Bool_t excludeRange=kFALSE);
  void AddCut(const char *formula,                     THnBase *const histMin, THnBase * const histMax, Bool_t excludeRange=kFALSE);

  // setters
  void    SetCutOnMCtruth(Bool_t mc=kTRUE) { fCutOnMCtruth=mc; }
  void    SetCutType(CutType type)         { fCutType=type;    }

  // getters
  Bool_t  GetCutOnMCtruth() const { return fCutOnMCtruth; }
  CutType GetCutType()      const { return fCutType;      }

  Int_t GetNCuts() { return fNActiveCuts; }

  //
  //Analysis cuts interface
  //
  virtual Bool_t IsSelected(TObject* track);
  virtual Bool_t IsSelected(TList*   /* list */ ) {return kFALSE;}

  //   virtual Bool_t IsSelected(TObject* track, TObject */*event*/=0);
  //   virtual Long64_t Merge(TCollection* /* list */)      { return 0; }

  //
  // Cut information
  //
  virtual UInt_t GetSelectedCutsMask() const { return fSelectedCutsMask; }

  virtual void Print(const Option_t* option = "") const;


 private:

  TBits     *fUsedVars;            // list of used variables
  UShort_t  fActiveCuts[PairAnalysisSpecialCuts::kNMaxCuts];       // list of activated cuts
  UShort_t  fNActiveCuts;                      // number of acive cuts
  UInt_t    fActiveCutsMask;                   // mask of active cuts

  UInt_t   fSelectedCutsMask;                 // Maks of selected cuts, is available after calling IsSelected

  Bool_t   fCutOnMCtruth;                     // whether to cut on the MC truth of the particle

  CutType  fCutType;                          // type of the cut: any, all

  Bool_t fCutExclude[PairAnalysisSpecialCuts::kNMaxCuts];         // inverse cut logic?
  TObject *fCutMin[PairAnalysisSpecialCuts::kNMaxCuts];          // use object as upper cut
  TObject *fCutMax[PairAnalysisSpecialCuts::kNMaxCuts];          // use object as upper cut
  TFormula *fVarFormula[PairAnalysisSpecialCuts::kNMaxCuts];     // use a formula for the variable

  PairAnalysisSpecialCuts(const PairAnalysisSpecialCuts &c);
  PairAnalysisSpecialCuts &operator=(const PairAnalysisSpecialCuts &c);

  ClassDef(PairAnalysisSpecialCuts,1)         //Cut class providing cuts to all infomation available for the AliVParticle interface
};


#endif

