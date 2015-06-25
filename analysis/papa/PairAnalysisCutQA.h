#ifndef PAIRANALYSISCUTQA_H
#define PAIRANALYSISCUTQA_H

/* Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//#################################################################
//#                                                               #
//#             Class PairAnalysisCutQA                             #
//#              PairAnalysis Group of cuts                         #
//#                                                               #
//#  Authors:                                                     #
//#   Julian    Book,     Uni Ffm / Julian.Book@cern.ch           #
//#                                                               #
//#################################################################

#include <TNamed.h>
#include <TH1I.h>
#include <TH2I.h>
#include <TList.h>
#include <TObjArray.h>

#include "AnalysisFilter.h"

class TCollection;

class PairAnalysisCutQA : public TNamed {
  
public:
  enum { kEvent=0, kTrack, kPair, kNtypes };

  PairAnalysisCutQA();
  PairAnalysisCutQA(const char*name, const char* title);
  
  virtual ~PairAnalysisCutQA();

  //Analysis cuts interface
  //
  void Init();
  void AddTrackFilter(     AnalysisFilter *trkFilter);
  /*  void AddPrePairFilter(   AnalysisFilter *prePairFilter);*/
  /* void AddPrePairLegFilter(AnalysisFilter *prePairLegFilter); */
  void AddPairFilter(      AnalysisFilter *pairFilter);
  void AddEventFilter(     AnalysisFilter *eventFilter);

  //  void Fill(AnalysisCuts *cut);
  void Fill(UInt_t mask, TObject *obj);
  void FillAll(TObject *obj);// { fCutQA->Fill(0); }

  const TObjArray * GetQAHistArray() const { return &fQAHistArray; }


private:

  TObjArray fQAHistArray;              //-> array of QA histograms
  TH1I *fCutQA[kNtypes];               // qa histogram
  TH2I *fPdgCutQA[kNtypes];            // qa histogram for PDG codes
  Int_t fNCuts[kNtypes];               // number of cuts
  const char* fCutNames[20][kNtypes];  // cut names
  const char* fTypeKeys[kNtypes];      // type names


  UInt_t GetObjIndex(TObject *obj);    // return object index

  PairAnalysisCutQA(const PairAnalysisCutQA &);
  PairAnalysisCutQA &operator=(const PairAnalysisCutQA &);
  
  ClassDef(PairAnalysisCutQA,2) //Group of cuts
};

#endif
