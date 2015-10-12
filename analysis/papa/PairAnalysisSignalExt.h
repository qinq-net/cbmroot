#ifndef PAIRANALYSISSIGNALEXT_H
#define PAIRANALYSISSIGNALEXT_H

/* Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. */

//#############################################################
//#                                                           # 
//#           Class PairAnalysisSignalExt                    #
//#                                                           #
//#  Authors:                                                 #
//#   Julian    Book,     Uni Ffm / Julian.Book@cern.ch       #
//#                                                           #
//#############################################################

#include <TVectorT.h>
#include <TString.h>
#include <TH1.h>

#include "PairAnalysisSignalBase.h"

class PairAnalysisSignalExt : public PairAnalysisSignalBase {

public:
 
  PairAnalysisSignalExt();
  PairAnalysisSignalExt(const char*name, const char* title);

  virtual ~PairAnalysisSignalExt();

  virtual void Process(TObjArray* const arrhist);
  void ProcessLS();        // like-sign method
  void ProcessEM();        // event mixing method
  void ProcessTR();        // track rotation method
  void ProcessCocktail();  // cocktail method

  virtual void Draw(const Option_t* option = "");

 
  PairAnalysisSignalExt(const PairAnalysisSignalExt &c);

private:

  //  PairAnalysisSignalExt(const PairAnalysisSignalExt &c);
  PairAnalysisSignalExt &operator=(const PairAnalysisSignalExt &c);

  ClassDef(PairAnalysisSignalExt,1)    // class for signal extraction using LS, ME or ROT
};

#endif
