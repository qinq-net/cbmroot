#ifndef PAIRANALYSISFUNCTION_H
#define PAIRANALYSISFUNCTION_H

//#############################################################
//#                                                           #
//#         Class PairAnalysisFunction                        #
//#                                                           #
//#  Authors:                                                 #
//#   Julian    Book,     Uni Ffm / Julian.Book@cern.ch       #
//#                                                           #
//#############################################################

#include <TNamed.h>
#include <TString.h>
#include <TVectorT.h>
#include <TMath.h>
#include <TH1F.h>
#include <TF1.h>

class PairAnalysisFunction : public TNamed {

 public:
  PairAnalysisFunction();
  PairAnalysisFunction(const char*name, const char* title);
  /* PairAnalysisFunction(const PairAnalysisFunction &c); */
  /* PairAnalysisFunction &operator=(const PairAnalysisFunction &c); */

  virtual ~PairAnalysisFunction();

  //  virtual void Fit(Option_t *opt);

  // Setter
  void SetFitRange(Double_t min, Double_t max)   { fFitMin=min; fFitMax=max;  }
  void SetUseIntegral(Bool_t flag=kTRUE)         { fUseIntegral = flag; }
  void SetFitOption(const char* opt) {
    fFitOpt=opt;    fFitOpt.ToLower();
    if(!fFitOpt.Contains("s")) fFitOpt += "s";
  }

  void SetMCSignalShape(TH1F* hist) { fgHistSimPM=hist; }
  void SetFunctions(TF1 * const combined, TF1 * const sig=0, TF1 * const back=0, Int_t parM=1, Int_t parMres=2);
  void SetDefaults(Int_t type);

  void CombineFunc(TF1 * const peak=0, TF1 * const bgnd=0);

  // predefined peak functions
  Double_t PeakFunMC(const Double_t *x, const Double_t *par); // peak function from a mc histo
  Double_t PeakFunCB(const Double_t *x, const Double_t *par); // crystal ball function
  Double_t PeakFunGaus(const Double_t *x, const Double_t *par); // gaussian

  // Getter
  TF1*  GetSignalFunction()     const { return fFuncSignal;        }
  TF1*  GetBackgroundFunction() const { return fFuncBackground;    }
  TF1*  GetCombinedFunction()   const { return fFuncSigBack;       }

  Int_t GetDof()                const { return fDof;               }
  Double_t GetChi2Dof()         const { return fChi2Dof;           }
  Double_t GetFitMin()          const { return fFitMin;            }
    Double_t GetFitMax()        const { return fFitMax;            }
  
  //virtual void Draw(const Option_t* option = "");
  //  virtual void Print(Option_t *option="") const;


protected:

  static TH1F* fgHistSimPM;          // simulated peak shape

  Double_t PeakBgndFun(const Double_t *x, const Double_t *par); // combine any bgrd and any peak function

  TF1 *fFuncSignal;                // Function for the signal description
  TF1 *fFuncBackground;            // Function for the background description
  TF1 *fFuncSigBack;               // Combined function signal plus background

  Double_t fFitMin;                  // fit range lowest inv. mass
  Double_t fFitMax;                  // fit range highest inv. mass

  Int_t fParMass;                  // the index of the parameter corresponding to the resonance mass
  Int_t fParMassWidth;             // the index of the parameter corresponding to the resonance mass width

  TString fFitOpt;             // fit option used
  Bool_t fUseIntegral;         // use the integral of the fitted functions to extract signal and background

  Int_t    fDof;                   // degrees of freedom
  Double_t fChi2Dof;               // chi2/dof of the fitted inv mass spectra

  Int_t    fNparPeak;              // number of parameters for peak function
  Int_t    fNparBgnd;              // number of parameters for background function

  PairAnalysisFunction(const PairAnalysisFunction &c);
  PairAnalysisFunction &operator=(const PairAnalysisFunction &c);

  ClassDef(PairAnalysisFunction,1)         // class for signal extraction using a combined bgrd+signal fit

};

#endif
