#ifndef PAIRANALYSISSPECTRUM_H
#define PAIRANALYSISSPECTRUM_H
//#############################################################
//#                                                           #
//#         Class PairAnalysisSpectrum                        #
//#  Authors:                                                 #
//#   Julian    Book,     Uni Ffm / Julian.Book@cern.ch       #
//#                                                           #
//#############################################################


//#include <TTree.h>
#include <TVectorT.h>
#include <TString.h>
#include <TObjArray.h>

#include "PairAnalysisHF.h"
#include "PairAnalysisHistos.h"

#include "PairAnalysisFunction.h"
#include "PairAnalysisSignalExt.h"

#include "PairAnalysis.h"

class TList;
class TObjArray;
class TFormula;
class TTree;
class TH1F;

class Extraction : public TObject {
public:
  TString  setup   = ""; // indentifier key
  Int_t    setupId = -1; // identifier idx
  Double_t var     = 0.; // value of variable
  Double_t varE    = 0.; // error of variable
  Double_t s       = 0.; // raw signal
  Double_t sE      = 0.; // raw signal error
  Double_t b       = 0.; // background
  Double_t bE      = 0.; // background error
  Double_t sb      = 0.; //SB
  Double_t sbE     = 0.; //SBError
  Double_t sgn     = 0.; //Significance
  Double_t sgnE    = 0.; //SignificanceError
  TH1F *HistSignal = NULL; //SignalHistogram
  ClassDef(Extraction,1)
};
ClassImp(Extraction)

class PairAnalysisSpectrum : public PairAnalysisFunction {

public:

  enum ESystMethod { kBarlow=0, kSystMax, kSystRMS };

  PairAnalysisSpectrum();
  PairAnalysisSpectrum(const char*name, const char* title);

  virtual ~PairAnalysisSpectrum();

  // General Setter
  void SetVariable(    TString varType, TVectorD *const binLimits)  { fVar=varType; fVarBinning=binLimits; }
  void SetSpectrumCalc(const char *form, Double_t *params=0x0, Double_t *paramsE=0x0);
  void SetSystMethod(ESystMethod mthd)  { fSystMthd=mthd; }

  // Input
  void AddInput( TObjArray *raw, TObjArray *mc,  TString identifier);

  void AddCorrInput(PairAnalysisHF *hf)           { fCorrInput.Add(hf);  }
  void AddExtractor(PairAnalysisSignalExt *sig)   { fExtractor.Add((PairAnalysisSignalExt*)sig->Clone()); }

  // Spectrum
  virtual void Draw(const char* varexp, const char* selection, Option_t* option = "");  // copy from ttree
  virtual void Draw(const Option_t* option)    { Draw(option, "", ""); }
  void DrawSystRawYields();

  // Processing
  //  virtual void Print(Option_t *option="") const;
  void Init();
  void Process();

  // output
  virtual void Write();

private:

  // settings
  Int_t     fIdx = 0;                // index
  TString   fVar = "";               // variable looked at
  TVectorD *fVarBinning = NULL;      // variable binning
  //  Char_t    fDivision[10];            // test

  // calculation
  TFormula   *fCalculation = NULL;  // formula for scaling, etc.
  Double_t   *fParams = NULL;       // parameters
  Double_t   *fParamsE = NULL;      // parameter errors
  ESystMethod fSystMthd = kSystMax; // systematic method TODO: implement

  // input
  TString    fInputKeys[100];       // keys to identify the extraction
  TList      fRawInput;             // list of input objects for signals (HF, Ntuple, THnSparse)
  TList      fMCInput;              // list of input objects for mc (HF, Ntuple, THnSparse)
  TList      fCorrInput;            // list of input objects for corrections (HF, Ntuple, THnSparse)
  TList      fExtractor;            // list of input objects for signal extraction objects (Ext,Func)
  //  TObjArray      fExtractor;            // list of input objects for signal extraction objects (Ext,Func)

  // output
  TTree      *fTree;                // tree output
  TList      *fResults = NULL;      // final list with spectra
  TObjArray  *fExtractions = NULL;  // final canvases
  Extraction *fExt = NULL;          // extraction
  Double_t   fTest = 0.;            // test

  PairAnalysisSpectrum(const PairAnalysisSpectrum &c);
  PairAnalysisSpectrum &operator=(const PairAnalysisSpectrum &c);

  ClassDef(PairAnalysisSpectrum,1) // base and abstract class for signal extraction
};

#endif
