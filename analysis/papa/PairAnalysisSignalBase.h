#ifndef PAIRANALYSISSIGNALBASE_H
#define PAIRANALYSISSIGNALBASE_H

/* Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//#############################################################
//#                                                           # 
//#         Class PairAnalysisSignalBase                       #
//#  Authors:                                                 #
//#   Julian    Book,     Uni Ffm / Julian.Book@cern.ch       #
//#                                                           #
//#############################################################


#include <TNamed.h>
#include <TVectorT.h>
#include <TMath.h>
#include <TH1F.h>
#include <TF1.h>

#include "PairAnalysis.h"

class TObjArray;
class TPaveText;

class PairAnalysisSignalBase : public TNamed {
public:
  enum EBackgroundMethod {
    kFittedMC = 0,
    kFitted,
    kLikeSign,
    kLikeSignArithm,
    kLikeSignRcorr,
    kLikeSignArithmRcorr,
    kLikeSignFit,
    kEventMixing,
    kEventMixingFit,
    kRotation
  };

  enum ESignalExtractionMethod {
    kBinCounting = 0,
    kMCScaledMax,
    kMCScaledInt,
    kMCFitted,
    kCrystalBall,
    kGaus
  };

  PairAnalysisSignalBase();
  PairAnalysisSignalBase(const char*name, const char* title);
  
  virtual ~PairAnalysisSignalBase();

  // Setter
  // signal
  void SetMCSignalShape(TH1F* hist)                        { fgHistSimPM=hist; fHistSignalMC=hist; }
  void SetParticleOfInterest(Int_t pdgcode)                { fPOIpdg=pdgcode; }
  void SetIntegralRange(Double_t min, Double_t max)        { fIntMin=min; fIntMax=max; }
  void SetFitRange(Double_t min, Double_t max)             { fFitMin=min; fFitMax=max; }
  void SetRebin(Int_t factor)                              { fRebin=factor; }
  void SetExtractionMethod(ESignalExtractionMethod method) { fPeakMethod=method; }
  void SetMixingCorrection(Bool_t mixcorr=kTRUE)           { fMixingCorr=mixcorr; }
  // background
  void SetMethod(EBackgroundMethod method)                 { fMethod = method;}
  void SetNTrackRotations(Int_t iterations)                { fNiterTR =iterations; }
  void SetScaleBackgroundToRaw(Double_t intMin, Double_t intMax) { fScaleMin=intMin; fScaleMax=intMax; }
  void SetScaleBackgroundToRaw(Double_t intMin, Double_t intMax, Double_t intMin2, Double_t intMax2) { fScaleMin=intMin; fScaleMax=intMax; fScaleMin2=intMin2; fScaleMax2=intMax2; }

  // Getter
  Int_t GetParticleOfInterest()      const { return fPOIpdg; }
  Double_t GetIntegralMin()          const { return fIntMin; }
  Double_t GetIntegralMax()          const { return fIntMax; }
  Double_t GetFitMin()               const { return fFitMin; }
  Double_t GetFitMax()               const { return fFitMax; }
  Int_t GetRebin()                   const { return fRebin;  }
  ESignalExtractionMethod GetExtractionMethod() const      { return fPeakMethod; }
  EBackgroundMethod GetMethod()      const { return fMethod; }
  Double_t GetScaleMin()             const { return fScaleMin;   }
  Double_t GetScaleMax()             const { return fScaleMax;   }
  Double_t GetScaleMin2()            const { return fScaleMin2;  }
  Double_t GetScaleMax2()            const { return fScaleMax2;  }
  // values of results
  Double_t GetScaleFactor()          const { return fScaleFactor; }
  const TVectorD& GetValues()        const { return fValues; }
  const TVectorD& GetErrors()        const { return fErrors; }
  Double_t GetSignal()               const { return fValues(0); }
  Double_t GetSignalError()          const { return fErrors(0); }
  Double_t GetBackground()           const { return fValues(1); }
  Double_t GetBackgroundError()      const { return fErrors(1); }
  Double_t GetSignificance()         const { return fValues(2); }
  Double_t GetSignificanceError()    const { return fErrors(2); }
  Double_t GetSB()                   const { return fValues(3); }
  Double_t GetSBError()              const { return fErrors(3); }
  Double_t GetMass()                 const { return fValues(4); }
  Double_t GetMassError()            const { return fErrors(4); }
  Double_t GetMassWidth()            const { return fValues(5); }
  Double_t GetMassWidthError()       const { return fErrors(5); }
  Double_t GetMatchChi2NDF()         const { return fValues(6); }
  Double_t GetMatchChi2NDFError()    const { return fErrors(6); }
  static const char* GetValueName(Int_t i) { return (i>=0&&i<7)?fgkValueNames[i]:""; }
  // objects
  TH1* GetMCSignalShape()            const { return fHistSignalMC; }
  TH1* GetSignalHistogram()          const { return fHistSignal; }
  TH1* GetSoverBHistogram()          const { return fHistSB; }
  TH1* GetSignificanceHistogram()    const { return fHistSign; }
  TH1* GetBackgroundHistogram()      const { return fHistBackground; }
  TH1* GetUnlikeSignHistogram()      const { return fHistDataPM; }
  TH1* GetRfactorHistogram()         const { return fHistRfactor; }
  TObject* GetPeakShape()            const { return fgPeakShape; }



  TObject* DescribePeakShape(ESignalExtractionMethod method=kMCFitted, Bool_t replaceValErr=kFALSE,  TH1F *mcShape=0x0);
  Double_t ScaleHistograms(TH1* histRaw, TH1* histBackground, Double_t intMin, Double_t intMax);
  Double_t ScaleHistograms(TH1* histRaw, TH1* histBackground, Double_t intMin, Double_t intMax, Double_t intMin2, Double_t intMax2);

  static TH1* MergeObjects(TH1* obj1, TH1* obj2, Double_t val=+1.);
  virtual void Print(Option_t *option="") const;

  /**
  This function needs to be implemented by the signal extraction classes.
  Here all the work should be done.

  The signal extraction is done on the mass spectra.
  The TObjArray should contain the Inv. Mass spectra of the 8 possible combinations
  for single and mixed events defined in PairAnalysis.cxx
  */
  virtual void Process(TObjArray * const /*arrhist*/) = 0;

protected:
  TObjArray *fArrHists;              // array of input histograms
  TH1 *fHistSignal;                  // histogram of pure signal
  TH1 *fHistSB;                      // histogram of signal to bgrd
  TH1 *fHistSign;                    // histogram of significance
  TH1 *fHistBackground;              // histogram of background (fitted=0, like-sign=1, event mixing=2)
  TH1 *fHistDataPM;                  // histogram of selected +- pair candidates
  TH1 *fHistDataPP;                  // histogram of selected ++ pair candidates
  TH1 *fHistDataMM;                  // histogram of selected -- pair candidates
  TH1 *fHistDataME;                  // histogram of selected +- pair candidates from mixed event
  TH1 *fHistRfactor;                 // histogram of R factors
  TH1 *fHistSignalMC;                // histogram of signal MC shape

  TVectorD fValues;                  // values
  TVectorD fErrors;                  // value errors

  Double_t fIntMin;                  // signal extraction range min
  Double_t fIntMax;                  // signal extraction range max
  Double_t fFitMin;                  // fit range lowest inv. mass
  Double_t fFitMax;                  // fit range highest inv. mass

  Int_t fRebin;                      // histogram rebin factor
  EBackgroundMethod fMethod;         // method for background substraction
  Double_t fScaleMin;                // min for scaling of raw and background histogram
  Double_t fScaleMax;                // max for scaling of raw and background histogram
  Double_t fScaleMin2;               // min for scaling of raw and background histogram
  Double_t fScaleMax2;               // max for scaling of raw and background histogram
  Int_t    fNiterTR;                 // track rotation scale factor according to number of rotations
  Double_t fScaleFactor;             // scale factor of raw to background histogram scaling
  Bool_t fMixingCorr;                // switch for bin by bin correction with R factor

  ESignalExtractionMethod fPeakMethod; // method for peak description and signal extraction
  static TObject *fgPeakShape;       // histogram or function used to describe the extracted signal
  Bool_t fPeakIsTF1;                 // flag

  Bool_t fProcessed;                 // flag
  Int_t  fPOIpdg;                    // pdg code particle of interest
  static TH1F* fgHistSimPM;          // simulated peak shape

  void SetSignificanceAndSOB();      // calculate the significance and S/B
  void SetFWHM();                    // calculate the fwhm
  static const char* fgkValueNames[7]; //value names
  static const char* fgkBackgroundMethodNames[10]; // background estimator names
  TPaveText* DrawStats(Double_t x1=0., Double_t y1=0., Double_t x2=0., Double_t y2=0.);
  TObject* FindObject(TObjArray *arrhist, PairAnalysis::EPairType type);

  PairAnalysisSignalBase(const PairAnalysisSignalBase &c);
  PairAnalysisSignalBase &operator=(const PairAnalysisSignalBase &c);

  ClassDef(PairAnalysisSignalBase,1) // base and abstract class for signal extraction
};

inline TObject* PairAnalysisSignalBase::FindObject(TObjArray *arrhist, PairAnalysis::EPairType type)
{
  //
  // shortcut to find a certain pair type object in array
  //
  return ( arrhist->FindObject(Form("Pair.%s",PairAnalysis::PairClassName(type))) );
}

inline void PairAnalysisSignalBase::SetSignificanceAndSOB()
{
  //
  // Calculate S/B and significance
  //
  // Signal/Background
  fValues(3) = (fValues(1)>0 ? fValues(0)/fValues(1) : 0);
  Float_t epsSig = (fValues(0)>0 ? fErrors(0)/fValues(0) : 0);
  Float_t epsBknd = (fValues(1)>0 ? fErrors(1)/fValues(1) : 0);
  fErrors(3) = fValues(3)*TMath::Sqrt(epsSig*epsSig + epsBknd*epsBknd);
  // Significance
  fValues(2) = ((fValues(0)+fValues(1))>0 ? fValues(0)/TMath::Sqrt(fValues(0)+fValues(1)) : 0);
  Float_t s = (fValues(0)>0?fValues(0):0); Float_t b = fValues(1);
  Float_t se = fErrors(0); Float_t be = fErrors(1);
  // fErrors(2) = ((s+b)>0 ? TMath::Sqrt((s*(s+2*b)*(s+2*b)+b*s*s)/(4*TMath::Power(s+b,3))) : 0); // old implementation
  fErrors(2) = ((s+b)>0 ? fValues(2)*TMath::Sqrt(be*be + TMath::Power(se*(s+2*b)/s, 2)) / 2 / (s+b) : 0);
}

inline void PairAnalysisSignalBase::SetFWHM()
{
  // calculate the fwhm
  if(!fgPeakShape) return;

  // case for TF1
  if(fgPeakShape->IsA() == TF1::Class()) {
    TF1* fit  = (TF1*) fgPeakShape->Clone("fit");
    TF1* pfit = (TF1*) fit->Clone("pfit");
    TF1* mfit = (TF1*) fit->Clone("mfit");
    for (Int_t i=0; i<fit->GetNpar(); i++) {
      pfit->SetParameter(i,fit->GetParameter(i) + fit->GetParError(i));
      mfit->SetParameter(i,fit->GetParameter(i) - fit->GetParError(i));
    }
    Double_t maxX   = fit->GetMaximumX();
    Double_t maxY   = fit->GetHistogram()->GetMaximum();
    Double_t xAxMin = fit->GetXmin();
    Double_t xAxMax = fit->GetXmax();
    // fwhms 
    Double_t fwhmMin  = fit->GetX(.5*maxY, xAxMin, maxX);
    Double_t fwhmMax  = fit->GetX(.5*maxY, maxX, xAxMax);
    Double_t pfwhmMin = pfit->GetX(.5*maxY, xAxMin, maxX);
    Double_t pfwhmMax = pfit->GetX(.5*maxY, maxX, xAxMax);
    Double_t mfwhmMin = mfit->GetX(.5*maxY, xAxMin, maxX);
    Double_t mfwhmMax = mfit->GetX(.5*maxY, maxX, xAxMax);
    Double_t pError = TMath::Abs( (fwhmMax-fwhmMin) - (pfwhmMax-pfwhmMin) );
    Double_t mError = TMath::Abs( (fwhmMax-fwhmMin) - (mfwhmMax-mfwhmMin) );
    fValues(5) = (fwhmMax-fwhmMin);
    fErrors(5) = (pError>=mError ? pError : mError);
    delete fit;
    delete pfit;
    delete mfit;
  }
  else if(fgPeakShape->IsA() == TH1F::Class()) {
    // th1 calculation
    TH1F *hist = (TH1F*) fgPeakShape->Clone("hist");
    Int_t bin1 = hist->FindFirstBinAbove(hist->GetMaximum()/2);
    Int_t bin2 = hist->FindLastBinAbove(hist->GetMaximum()/2);
    fValues(5) = hist->GetBinCenter(bin2) - hist->GetBinCenter(bin1);
    fErrors(5) = 0.0; // not defined
    delete hist;
  }

}

#endif
