///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                      PairAnalysisSignalExt                             //
//                                                                       //
/*

  Class used for extracting the signal from an invariant mass spectrum.
  Used invariant mass spectra are provided via an array of histograms. There are serveral method
  to estimate the background and to extract the raw yield from the background subtracted spectra.

  Example usage:

  PairAnalysisSignalExt *sig = new PairAnalysisSignalExt();


  1) invariant mass input spectra

  Assuming a PairAnalysisHistos as data format (check class for more details)
  PairAnalysisHistos *histo = new PairAnalysisHistos();
  histos->ReadFromFile("path/to/the/output/file.root", "ConfigName");
  histos->SetCutClass("TRD");
  TObjArray *arrHists = histos->DrawSame("pM",   "Can Leg goff");

  2) background estimation

  2.1) set the method for the background estimation (methods can be found in PairAnalysisSignalBase)
  sig->SetMethod(PairAnalysisSignalBase::kLikeSign);
  2.2) rebin the spectras if needed
  //  sig->SetRebin(2);
  2.3) normalize the backgound spectum to the opposite-sign spectrum in the desired range(s)
  sig->SetScaleBackgroundToRaw(minScale, maxScale);
  //  sig->SetScaleBackgroundToRaw(minScale, maxScale, minScale2, maxScale2);


  3) configure the signal extraction

  3.1) set the method for the signal extraction (methods can be found in PairAnalysisSignalBase)
  depending on the method serveral inputs are needed (e.g. MC shape, PDG code of the particle of interest)
  //  sig->SetParticleOfInterest(443); //default is jpsi
  //  sig->SetMCSignalShape(signalMC);
  sig->SetIntegralRange(minInt, maxInt);  // range for bin counting (signal integration)
  sig->SetExtractionMethod(PairAnalysisSignalBase::BinCounting); // this is the default


  4) start the processing

  sig->Process(arrHists);
  sig->Print(""); // print values and errors extracted

  5) access the spectra, get values and draw

  5.1) draw histograms with statistics, legend, etc
  // see options (logy,nomc,..) under PairAnalysisSignalExt::Draw
  sig->Draw("can leg stat");

  5.1) optional access the output
  TH1F *hsign = (TH1F*) sig->GetUnlikeSignHistogram();  // same as the input (rebinned)
  TH1F *hbgrd = (TH1F*) sig->GetBackgroundHistogram();  // scaled input      (rebinned)
  TH1F *hextr = (TH1F*) sig->GetSignalHistogram();      // after backgound extraction (rebinned)
  TObject *oPeak = (TObject*) sig->GetPeakShape();      // can be a TF1 or TH1 depending on the extraction method
  TH1F *hrfac = (TH1F*) sig->GetRfactorHistogram();     // if like-sign correction was activated, o.w. 0x0
  5.2) access the extracted values and errors
  sig->GetValues();     or GetErrors();                 // yield extraction

*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////
#include <TROOT.h>
#include <TStyle.h>

#include <TCollection.h>
#include <TF1.h>
#include <TH1.h>
#include <TH2F.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TString.h>
#include <TLine.h>
#include <TGaxis.h>

#include "PairAnalysisSignalExt.h"
#include "PairAnalysis.h"
#include "PairAnalysisSignalMC.h"

#include "PairAnalysisStyler.h"

ClassImp(PairAnalysisSignalExt)

PairAnalysisSignalExt::PairAnalysisSignalExt() :
  PairAnalysisSignalBase()
{
  //
  // Default Constructor
  //
}

//______________________________________________
PairAnalysisSignalExt::PairAnalysisSignalExt(const char* name, const char* title) :
  PairAnalysisSignalBase(name, title)
{
  //
  // Named Constructor
  //
}

//______________________________________________
PairAnalysisSignalExt::PairAnalysisSignalExt(const PairAnalysisSignalExt &c) :
  PairAnalysisSignalBase(c)
{
  //
  // Copy Constructor
  //
}

//______________________________________________
PairAnalysisSignalExt::~PairAnalysisSignalExt()
{
  //
  // Default Destructor
  //
}

//______________________________________________
void PairAnalysisSignalExt::Process(TObjArray* const arrhist)
{
  //
  // signal subtraction. support like-sign subtraction and event mixing method
  //
  fArrHists = arrhist;
  fArrHists->SetOwner(kFALSE);

  // calculate optimal binning if configured
  if(fRebinStat<1. && fBinLimits==0x0) {
    fBinLimits = PairAnalysisHelper::MakeStatBinLimits( (TH1*)FindObject(arrhist,PairAnalysis::kSEPM) , fRebinStat);
  }


  // clean up spectra
  if(fProcessed && 0) { //TODO: not needed??
    if(fHistDataPP)     delete fHistDataPP;     fHistDataPP=0x0;
    if(fHistDataPM)     delete fHistDataPM;     fHistDataPM=0x0;
    if(fHistDataMM)     delete fHistDataMM;     fHistDataMM=0x0;

    if(fHistDataME)     delete fHistDataME;     fHistDataME=0x0;
    if(fHistRfactor)    delete fHistRfactor;    fHistRfactor=0x0;

    if(fHistMixPP)      delete fHistMixPP;      fHistMixPP=0x0;
    if(fHistMixPM)      delete fHistMixPM;      fHistMixPM=0x0;
    if(fHistMixMM)      delete fHistMixMM;      fHistMixMM=0x0;
    if(fHistMixMP)      delete fHistMixMP;      fHistMixMP=0x0;

    if(fHistDataTR)     delete fHistDataTR;     fHistDataTR=0x0;

    if(fHistSignal)     delete fHistSignal;     fHistSignal=0x0;
    if(fHistBackground) delete fHistBackground; fHistBackground=0x0;
    if(fHistSB)         delete fHistSB;         fHistSB=0x0;
    if(fHistSign)       delete fHistSign;       fHistSign=0x0;
    if(fHistCocktail)   delete fHistCocktail;   fHistCocktail=0x0;
  }

  //// get histograms and rebin
  // SE ++
  fHistDataPP = (TH1*)FindObject(arrhist,PairAnalysis::kSEPP);
  if(fHistDataPP) {
    if(fBinLimits)     {
      fHistDataPP = fHistDataPP->Rebin(fBinLimits->GetSize()-1,"histPP",fBinLimits->GetArray());   
      fHistDataPP->Scale(1.,"width");
    }
    else               fHistDataPP->Clone("histPP");
    if(fHistDataPP->GetDefaultSumw2()) fHistDataPP->Sumw2();
    fHistDataPP->SetDirectory(0);
    if(fRebin>1)       fHistDataPP->Rebin(fRebin);
  }
  // SE +-
  fHistDataPM = (TH1*)FindObject(arrhist,PairAnalysis::kSEPM);
  if(fHistDataPM) {
    if(fBinLimits)     {
      fHistDataPM = fHistDataPM->Rebin(fBinLimits->GetSize()-1,"histPM",fBinLimits->GetArray());
      fHistDataPM->Scale(1.,"width");
    }
    else               fHistDataPM->Clone("histPM");
    if(fHistDataPM->GetDefaultSumw2()) fHistDataPM->Sumw2();
    fHistDataPM->SetDirectory(0);
    if(fRebin>1)       fHistDataPM->Rebin(fRebin);
    fHistDataPM->SetYTitle( (fBinLimits?"dN/dm":"Counts") );
  }
  // SE --
  fHistDataMM = (TH1*)FindObject(arrhist,PairAnalysis::kSEMM);
  if(fHistDataMM) {
    if(fBinLimits)     {
      fHistDataMM = fHistDataMM->Rebin(fBinLimits->GetSize()-1,"histMM",fBinLimits->GetArray());
      fHistDataMM->Scale(1.,"width");
    }
    else               fHistDataMM->Clone("histMM");
    if(fHistDataMM->GetDefaultSumw2()) fHistDataMM->Sumw2();
    fHistDataMM->SetDirectory(0);
    if(fRebin>1)       fHistDataMM->Rebin(fRebin);
  }
  // ME ++
  fHistMixPP = (TH1*)FindObject(arrhist,PairAnalysis::kMEPP);
  if(fHistMixPP) {
    if(fBinLimits)     {
      fHistMixPP = fHistMixPP->Rebin(fBinLimits->GetSize()-1,"mixPP",fBinLimits->GetArray());
      fHistMixPP->Scale(1.,"width");
    }
    else               fHistMixPP->Clone("mixPP");
    if(fHistMixPP->GetDefaultSumw2()) fHistMixPP->Sumw2();
    fHistMixPP->SetDirectory(0);
    if(fRebin>1)       fHistMixPP->Rebin(fRebin);
  }
  // ME +-
  fHistMixPM = (TH1*)FindObject(arrhist,PairAnalysis::kMEPM);
  if(fHistMixPM) {
    if(fBinLimits)     {
      fHistMixPM = fHistMixPM->Rebin(fBinLimits->GetSize()-1,"mixPM",fBinLimits->GetArray());
      fHistMixPM->Scale(1.,"width");
    }
    else               fHistMixPM->Clone("mixPM");
    if(fHistMixPM->GetDefaultSumw2()) fHistMixPM->Sumw2();
    fHistMixPM->SetDirectory(0);
    if(fRebin>1)       fHistMixPM->Rebin(fRebin);
  }
  // ME -+
  fHistMixMP = (TH1*)FindObject(arrhist,PairAnalysis::kMEMP);
  if(fHistMixMP) {
    if(fBinLimits)     {
      fHistMixMP = fHistMixMP->Rebin(fBinLimits->GetSize()-1,"mixMP",fBinLimits->GetArray());
      fHistMixMP->Scale(1.,"width");
    }
    else               fHistMixMP->Clone("mixMP");
    if(fHistMixMP->GetDefaultSumw2()) fHistMixMP->Sumw2();
    fHistMixMP->SetDirectory(0);
    if(fRebin>1)       fHistMixMP->Rebin(fRebin);
  }
  if(fHistMixPM && fHistMixMP) fHistMixPM->Add( fHistMixMP ); // merge ME +- and -+
  // ME --
  fHistMixMM = (TH1*)FindObject(arrhist,PairAnalysis::kMEMM);
  if(fHistMixMM) {
    if(fBinLimits)     {
      fHistMixMM = fHistMixMM->Rebin(fBinLimits->GetSize()-1,"mixMM",fBinLimits->GetArray());
      fHistMixMM->Scale(1.,"width");
    }
    else               fHistMixMM->Clone("mixMM");
    if(fHistMixMM->GetDefaultSumw2()) fHistMixMM->Sumw2();
    fHistMixMM->SetDirectory(0);
    if(fRebin>1)       fHistMixMM->Rebin(fRebin);
  }
  // TR +-
  fHistDataTR = (TH1*)FindObject(arrhist,PairAnalysis::kSEPMRot);
  if(fHistDataTR) {
    if(fBinLimits)     {
      fHistDataTR = fHistDataTR->Rebin(fBinLimits->GetSize()-1,"histTR",fBinLimits->GetArray());
      fHistDataTR->Scale(1.,"width");
    }
    else               fHistDataTR->Clone("histTR");
    if(fHistDataTR->GetDefaultSumw2()) fHistDataTR->Sumw2();
    fHistDataTR->SetDirectory(0);
    if(fRebin>1)       fHistDataTR->Rebin(fRebin);
  }

  // init histograms for R-factor, subtracted signal, background
  fHistSignal = new TH1D("HistSignal", "Substracted signal",
			 fHistDataPM->GetXaxis()->GetNbins(),
			 fHistDataPM->GetXaxis()->GetXbins()->GetArray());
  fHistSignal->SetXTitle(fHistDataPM->GetXaxis()->GetTitle());
  fHistSignal->SetYTitle(fHistDataPM->GetYaxis()->GetTitle());
  if(fHistSignal->GetDefaultSumw2()) fHistSignal->Sumw2();
  fHistSignal->SetDirectory(0);
  fHistBackground = new TH1D("HistBackground", "Background contribution",
			     fHistDataPM->GetXaxis()->GetNbins(),
			     fHistDataPM->GetXaxis()->GetXbins()->GetArray());
  if(fHistBackground->GetDefaultSumw2()) fHistBackground->Sumw2();
  fHistBackground->SetDirectory(0);
  fHistRfactor = new TH1D("HistRfactor", "Rfactor;;N^{mix}_{+-}/2#sqrt{N^{mix}_{++} N^{mix}_{--}}",
                          fHistDataPM->GetXaxis()->GetNbins(),
			  fHistDataPM->GetXaxis()->GetXbins()->GetArray());
  if(fHistRfactor->GetDefaultSumw2()) fHistRfactor->Sumw2();
  fHistRfactor->SetDirectory(0);

  // cocktail
  if(fArrCocktail && fArrCocktail->GetEntriesFast()) {
    printf("rebin %d cocktail histograms\n",fArrCocktail->GetEntriesFast());
    fHistCocktail = new TH1D("HistCocktail", "Cocktail contribution",
			     fHistDataPM->GetXaxis()->GetNbins(),
			     fHistDataPM->GetXaxis()->GetXbins()->GetArray());
    if(fHistCocktail->GetDefaultSumw2()) fHistCocktail->Sumw2();
    fHistCocktail->SetDirectory(0);

    // loop over all ingredients
    for(Int_t i=0; i<fArrCocktail->GetEntriesFast(); i++) {
      TH1* htmp = static_cast<TH1*>(fArrCocktail->UncheckedAt(i));
      if(fBinLimits)     {
	htmp = htmp->Rebin(fBinLimits->GetSize()-1,htmp->GetTitle(),fBinLimits->GetArray());
	if(htmp->GetDefaultSumw2()) htmp->Sumw2();
	htmp->SetDirectory(0);
	htmp->Scale(1.,"width");
	fArrCocktail->AddAt(htmp,i);
      }
      //      else               htmp->Clone("histTR");
      // if(fHistDataTR->GetDefaultSumw2()) fHistDataTR->Sumw2();
      // fHistDataTR->SetDirectory(0);
      if(fRebin>1)       htmp->Rebin(fRebin);
      fHistCocktail->Add(htmp,+1.);
    }
  }

  // process method
  switch ( fMethod ){
    case kLikeSign :
    case kLikeSignArithm :
    case kLikeSignRcorr:
    case kLikeSignArithmRcorr:
      ProcessLS(arrhist);    // process like-sign subtraction method
      break;

    case kEventMixing :
      ProcessEM(arrhist);    // process event mixing method
      break;

  case kRotation:
      ProcessTR(arrhist);
      break;

  case kCocktail:
    fCocktailSubtr=kTRUE;
    ProcessCocktail(arrhist);
    break;

    default :
      Warning("Process","Subtraction method not supported. Please check SetMethod() function.");
  }


  // set S/B and Significance histos
  if(!fPeakIsTF1) fHistSB = (TH1*)fgPeakShape->Clone("histSB");
  else {
    fHistSB =(TH1*)fHistSignal->Clone("histSB");
    fHistSB->Reset("CEIS");
    fHistSB->Eval((TF1*)fgPeakShape);
  }
  fHistSB->Divide(fHistBackground);
  fHistSB->SetYTitle("S/B");

  //significance
  fHistSign = (TH1*)fHistSignal->Clone("histSB");
  fHistSign->Reset("CEIS");
  Double_t s=0.; Double_t b=0.;
  for(Int_t i=1; i<=fHistSign->GetNbinsX(); i++) {

    if(!fPeakIsTF1)  s=static_cast<TH1*>(fgPeakShape)->GetBinContent(i);
    else             s=static_cast<TF1*>(fgPeakShape)->Eval(fHistSign->GetBinCenter(i));
    b=fHistBackground->GetBinContent(i);

    if(s+b<1.e-6) continue;
    fHistSign->SetBinContent(i,s/TMath::Sqrt(s+b));
  }
  fHistSign->SetYTitle("S/#sqrt{S+B}");
  //  fErrors(2) = ((s+b)>0 ? fValues(2)*TMath::Sqrt(be*be + TMath::Power(se*(s+2*b)/s, 2)) / 2 / (s+b) : 0);

}

//______________________________________________
void PairAnalysisSignalExt::ProcessLS(TObjArray* const arrhist)
{
  //
  // signal subtraction
  //

  // protections
  if(!fHistDataPP || !fHistDataMM) return;

  // fill background histogram
  for(Int_t ibin=1; ibin<=fHistDataPM->GetXaxis()->GetNbins(); ibin++) {
    Float_t pp  = fHistDataPP->GetBinContent(ibin);
    Float_t ppe = fHistDataPP->GetBinError(ibin);
    Float_t mm  = fHistDataMM->GetBinContent(ibin);
    Float_t mme = fHistDataMM->GetBinError(ibin);

    Float_t background = 2*TMath::Sqrt(pp*mm);
    // do not use it since LS could be weighted err!=sqrt(entries)
    // Float_t ebackground = TMath::Sqrt(mm+pp);
    Float_t ebackground = TMath::Sqrt(mm/pp*ppe*ppe + pp/mm*mme*mme);
    // Arithmetic mean instead of geometric
    if (fMethod==kLikeSignArithm || fMethod==kLikeSignArithmRcorr ){
      background=(pp+mm);
      //      ebackground=TMath::Sqrt(pp+mm);
      ebackground=TMath::Sqrt(ppe*ppe+mme*mme);
      if (TMath::Abs(ebackground)<1e-30) ebackground=1;
    }
    // set contents
    fHistBackground->SetBinContent(ibin, background);
    fHistBackground->SetBinError(ibin, ebackground);
  }

  //correct LS spectrum bin-by-bin with R factor obtained in mixed events
  if(fMixingCorr || fMethod==kLikeSignRcorr || fMethod==kLikeSignArithmRcorr) {

    // protections
    if(!fHistMixPM || fHistMixPP || !fHistMixMM) return;

    // fill R-factor histogram
    for(Int_t ibin=1; ibin<=fHistMixPM->GetXaxis()->GetNbins(); ibin++) {
      Float_t pp  = fHistMixPP->GetBinContent(ibin);
      Float_t ppe = fHistMixPP->GetBinError(ibin);
      Float_t mm  = fHistMixMM->GetBinContent(ibin);
      Float_t mme = fHistMixMM->GetBinError(ibin);
      Float_t pm  = fHistMixPM->GetBinContent(ibin);
      Float_t pme = fHistMixPM->GetBinError(ibin);

      Float_t background = 2*TMath::Sqrt(pp*mm);
      // do not use it since ME could be weighted err!=sqrt(entries)
      // Float_t ebackground = TMath::Sqrt(mm+pp);
      Float_t ebackground = TMath::Sqrt(mm/pp*ppe*ppe + pp/mm*mme*mme);
      //Arithmetic mean instead of geometric
      if (fMethod==kLikeSignArithm){
        background=(pp+mm);
        ebackground=TMath::Sqrt(ppe*ppe+mme*mme);
        if (TMath::Abs(ebackground)<1e-30) ebackground=1;
      }

      Float_t rcon = 1.0;
      Float_t rerr = 0.0;
      if(background>0.0) {
        rcon = pm/background;
        rerr = TMath::Sqrt((1./background)*(1./background) * pme*pme +
                           (pm/(background*background))*(pm/(background*background)) * ebackground*ebackground);
      }
      fHistRfactor->SetBinContent(ibin, rcon);
      fHistRfactor->SetBinError(ibin, rerr);
    }
    // correction
    fHistBackground->Multiply(fHistRfactor);
  }

  //scale histograms to match integral between fScaleMin and fScaleMax
  // or if fScaleMax <  fScaleMin use fScaleMin as scale factor
  if (fScaleMax>fScaleMin && fScaleMax2>fScaleMin2)
    fScaleFactor = ScaleHistograms(fHistDataPM,fHistBackground,fScaleMin,fScaleMax,fScaleMin2,fScaleMax2);
  else if (fScaleMax>fScaleMin)
    fScaleFactor = ScaleHistograms(fHistDataPM,fHistBackground,fScaleMin,fScaleMax);
  else if (fScaleMin>0.){
    fScaleFactor   = fScaleMin;
    fHistBackground->Scale(fScaleFactor);
  }

  //subtract background
  fHistSignal->Add( fHistDataPM);
  fHistSignal->Add( fHistBackground, -1);

  //subtract cocktail (if added)
  if(fCocktailSubtr) fHistSignal->Add( fHistCocktail, -1);

  // background
  fValues(1) = fHistBackground->IntegralAndError(fHistBackground->FindBin(fIntMin),
						 fHistBackground->FindBin(fIntMax), 
						 fErrors(1));

  // signal depending on peak description method
  DescribePeakShape(fPeakMethod, kTRUE, fgHistSimPM);
  //printf("%f  %f\n",fValues(0),fValues(1));
  //  SetSignificanceAndSOB();

  fProcessed = kTRUE;
}

//______________________________________________
void PairAnalysisSignalExt::ProcessEM(TObjArray* const arrhist)
{
  //
  // event mixing of +- and -+
  //

  if( !fHistMixPM ) {
    Error("ProcessEM","Mixed event histogram missing");
    return;
  }


  // fill background histogram
  for(Int_t ibin=1; ibin<=fHistDataPM->GetXaxis()->GetNbins(); ibin++) {
    Float_t pm  = fHistMixPM->GetBinContent(ibin);
    Float_t pme = fHistMixPM->GetBinError(ibin);

    Float_t background  = pm;
    Float_t ebackground = TMath::Sqrt(pme*pme);
    //TODO:      Float_t ebackground = TMath::Power(pm, 3./4);

    // set contents
    fHistBackground->SetBinContent(ibin, background);
    fHistBackground->SetBinError(ibin, ebackground);
  }

  //scale histograms to match integral between fScaleMin and fScaleMax
  // or if fScaleMax <  fScaleMin use fScaleMin as scale factor
  if (fScaleMax>fScaleMin && fScaleMax2>fScaleMin2)
    fScaleFactor = ScaleHistograms(fHistDataPM,fHistBackground,fScaleMin,fScaleMax,fScaleMin2,fScaleMax2);
  else if (fScaleMax>fScaleMin)
    fScaleFactor = ScaleHistograms(fHistDataPM,fHistBackground,fScaleMin,fScaleMax);
  else if (fScaleMin>0.){
    fScaleFactor=fScaleMin;
    fHistBackground->Scale(fScaleFactor);
  }

  //subtract background
  fHistSignal->Add( fHistDataPM);
  fHistSignal->Add( fHistBackground, -1);
  //  fHistSignal = MergeObjects(fHistDataPM,fHistBackground,-1.);

  //subtract cocktail (if added)
  if(fCocktailSubtr) fHistSignal->Add( fHistCocktail, -1);

  // background
  fValues(1) = fHistBackground->IntegralAndError(fHistBackground->FindBin(fIntMin),
                                                 fHistBackground->FindBin(fIntMax),
                                                 fErrors(1));

  // signal depending on peak description method
  DescribePeakShape(fPeakMethod, kTRUE, fgHistSimPM);

  fProcessed = kTRUE;
}

//______________________________________________
void PairAnalysisSignalExt::ProcessTR(TObjArray* const arrhist)
{
  //
  // signal subtraction
  //

  if (!fHistDataTR) {
    Error("ProcessTR","Track rotation histogram missing");
    return;
  }

  // fill background histogram
  for(Int_t ibin=1; ibin<=fHistDataPM->GetXaxis()->GetNbins(); ibin++) {
    Float_t pm  = fHistDataTR->GetBinContent(ibin);
    Float_t pme = fHistDataTR->GetBinError(ibin);

    Float_t background  = pm;
    Float_t ebackground = TMath::Sqrt(pme*pme);

    // set contents
    fHistBackground->SetBinContent(ibin, background);
    fHistBackground->SetBinError(ibin, ebackground);
  }

  // scale according to number of interations used in track rotation
  fHistBackground->Scale(1./fNiterTR);

  //scale histograms to match integral between fScaleMin and fScaleMax
  // or if fScaleMax <  fScaleMin use fScaleMin as scale factor
  if (fScaleMax>fScaleMin && fScaleMax2>fScaleMin2)
    fScaleFactor = ScaleHistograms(fHistDataPM,fHistBackground,fScaleMin,fScaleMax,fScaleMin2,fScaleMax2);
  else if (fScaleMax>fScaleMin)
    fScaleFactor = ScaleHistograms(fHistDataPM,fHistBackground,fScaleMin,fScaleMax);
  else if (fScaleMin>0.) {
    fScaleFactor=fScaleMin;
    fHistBackground->Scale(fScaleFactor);
  }

  //subtract background
  fHistSignal->Add( fHistDataPM);
  fHistSignal->Add( fHistBackground, -1);

  //subtract cocktail (if added)
  if(fCocktailSubtr) fHistSignal->Add( fHistCocktail, -1);

  // background
  fValues(1) = fHistBackground->IntegralAndError(fHistBackground->FindBin(fIntMin),
                                                 fHistBackground->FindBin(fIntMax),
                                                 fErrors(1));

  // signal depending on peak description method
  DescribePeakShape(fPeakMethod, kTRUE, fgHistSimPM);

  fProcessed = kTRUE;

}

//______________________________________________
void PairAnalysisSignalExt::ProcessCocktail(TObjArray* const arrhist)
{
  //
  // signal subtraction
  //

  if (!fArrCocktail) {
    Error("ProcessCocktail","Cocktail histograms missing");
    return;
  }

  // fill background histogram
  if(fArrCocktail) fHistBackground=fHistCocktail;

  //scale histograms to match integral between fScaleMin and fScaleMax
  // or if fScaleMax <  fScaleMin use fScaleMin as scale factor
  if (fScaleMax>fScaleMin && fScaleMax2>fScaleMin2)
    fScaleFactor = ScaleHistograms(fHistDataPM,fHistBackground,fScaleMin,fScaleMax,fScaleMin2,fScaleMax2);
  else if (fScaleMax>fScaleMin)
    fScaleFactor = ScaleHistograms(fHistDataPM,fHistBackground,fScaleMin,fScaleMax);
  else if (fScaleMin>0.) {
    fScaleFactor=fScaleMin;
    fHistBackground->Scale(fScaleFactor);
  }

  //subtract background
  fHistSignal->Add( fHistDataPM);
  fHistSignal->Add( fHistBackground, -1);

  // background
  fValues(1) = fHistBackground->IntegralAndError(fHistBackground->FindBin(fIntMin),
						 fHistBackground->FindBin(fIntMax),
                                                  fErrors(1));

  // signal depending on peak description method
  DescribePeakShape(fPeakMethod, kTRUE, fgHistSimPM);

  fProcessed = kTRUE;

}

//______________________________________________
void PairAnalysisSignalExt::Draw(const Option_t* option)
{
  //
  // Draw the fitted function
  //
  Info("Draw", "Signal extraction results for '%s'",fgkBackgroundMethodNames[fMethod]);
  TString optString(option);
  optString.ToLower();
  optString.ReplaceAll(" ","");
  Bool_t optTask     =optString.Contains("same");      optString.ReplaceAll("same","");
  Bool_t optNoMCtrue =optString.Contains("nomctrue");  optString.ReplaceAll("nomctrue","");
  Bool_t optNoMC     =optString.Contains("nomc");      optString.ReplaceAll("nomc","");
  Bool_t optLeg      =optString.Contains("leg");       optString.ReplaceAll("leg","");
  Bool_t optCan      =optString.Contains("can");       optString.ReplaceAll("can","");
  //  Bool_t optMeta     =optString.Contains("meta");      optString.ReplaceAll("meta","");
  //  Bool_t optEvt      =optString.Contains("events");    optString.ReplaceAll("events","");
  //  Bool_t optSel      =optString.Contains("sel");       optString.ReplaceAll("sel","");
  Bool_t optStat     =optString.Contains("stat");      optString.ReplaceAll("stat","");
  Bool_t optSB       =optString.Contains("sb");        optString.ReplaceAll("sb","");
  Bool_t optSgn      =optString.Contains("sgn");       optString.ReplaceAll("sgn","");
  Bool_t optOnlyRaw  =optString.Contains("onlyraw");   optString.ReplaceAll("onlyraw","");
  Bool_t optOnlySig  =optString.Contains("onlysig");   optString.ReplaceAll("onlysig","");
  Bool_t optOnlyMC   =optString.Contains("onlymc");    optString.ReplaceAll("onlymc","");
  Bool_t optCocktail =optString.Contains("cocktail");  optString.ReplaceAll("cocktail","");

  // load style
  PairAnalysisStyler::LoadStyle();

  // add canvas
  TCanvas *c=0;
  if (optCan){
    c=(TCanvas*)gROOT->FindObject(Form("cSignalExtraction"));
    if (!c) c=new TCanvas(Form("cSignalExtraction"),Form("SignalExtraction"));
    //    c->Clear();
    c->cd();
  }

  Int_t nobj=0;
  TObject *obj;
  // count number of drawn objects in pad
  TList *prim = gPad->GetListOfPrimitives();
  for(Int_t io=0; io<prim->GetSize(); io++) {
    obj=prim->At(io);
    if(obj->InheritsFrom(TH1::Class()) && obj!=prim->At(io+1)) nobj++;
  }

  // add or get legend
  TLegend *leg=0;
  if ( (optLeg && optTask && !nobj) || (optLeg && !optTask) ) {
    leg=new TLegend(.75,.3,
		    1.-gPad->GetTopMargin()-gStyle->GetTickLength("X"),
		    1.-gPad->GetRightMargin()-gStyle->GetTickLength("Y"),
		    fArrHists->GetName(),"nbNDC");
    if(optTask) leg->SetHeader("");
  }
  else if(optLeg && nobj) {
    leg=(TLegend*)prim->FindObject("TPave");
  }

  // logaritmic style
  if(optString.Contains("logx")) gPad->SetLogx();
  if(optString.Contains("logy")) gPad->SetLogy();
  if(optString.Contains("logz")) gPad->SetLogz();
  optString.ReplaceAll("logx","");
  optString.ReplaceAll("logy","");
  optString.ReplaceAll("logz","");

  Int_t i=nobj; // TOD: obsolete?

  TString ytitle = fHistDataPM->GetYaxis()->GetTitle();
  // add bin width to y-axis title
  if( !(fHistDataPM->GetXaxis()->IsVariableBinSize()) ) {
    ytitle+=Form("/%.0f MeV/#it{c}^{2}",fHistDataPM->GetBinWidth(1)*1e3);
  }
  fHistDataPM->GetYaxis()->SetTitle(ytitle.Data());
  fHistSignal->GetYaxis()->SetTitle(ytitle.Data());

  // styling
  fHistDataPM->SetNameTitle("unlike-sign","unlike-sign");
  fHistDataPM->UseCurrentStyle();
  PairAnalysisStyler::Style(fHistDataPM,PairAnalysisStyler::kRaw);
  if(fPlotMin!=fPlotMax) fHistDataPM->SetAxisRange(fPlotMin,fPlotMax, "X");

  fHistBackground->SetNameTitle(fgkBackgroundMethodNames[fMethod],fgkBackgroundMethodNames[fMethod]);
  fHistBackground->UseCurrentStyle();
  PairAnalysisStyler::Style(fHistBackground,PairAnalysisStyler::kBgrd);

  fHistSignal->SetNameTitle("signal","signal");
  fHistSignal->UseCurrentStyle();
  PairAnalysisStyler::Style(fHistSignal,PairAnalysisStyler::kSig);
  if(fPlotMin!=fPlotMax) fHistSignal->SetAxisRange(fPlotMin,fPlotMax, "X");

  if(fHistCocktail) {
    fHistCocktail->SetNameTitle("cocktail","cocktail");
    fHistCocktail->UseCurrentStyle();
    PairAnalysisStyler::Style(fHistCocktail,PairAnalysisStyler::kCocktail);
    if(fPlotMin!=fPlotMax) fHistCocktail->SetAxisRange(fPlotMin,fPlotMax, "X");
  }

  if(optSB) {
    fHistSB->SetNameTitle("signal","signal");
    fHistSB->UseCurrentStyle();
    PairAnalysisStyler::Style(fHistSB,PairAnalysisStyler::kSig);
    if(fPlotMin!=fPlotMax) fHistSB->SetAxisRange(fPlotMin,fPlotMax, "X");
  }
  else if(optSgn) {
    fHistSign->SetNameTitle("signal","signal");
    fHistSign->UseCurrentStyle();
    PairAnalysisStyler::Style(fHistSign,PairAnalysisStyler::kSig);
    if(fPlotMin!=fPlotMax) fHistSign->SetAxisRange(fPlotMin,fPlotMax, "X");
  }

  // fHistRfactor->UseCurrentStyle();
  // fHistRfactor->SetTitle("");
  PairAnalysisStyler::Style(fHistRfactor,3);

  fgPeakShape->UseCurrentStyle();
  //  fgPeakShape->SetTitle("");
  PairAnalysisStyler::Style(fgPeakShape,PairAnalysisStyler::kFit);

  // draw stuff
  if(c)  c->cd(1);
  Info("Draw","Start plotting stuff with option -%s-",optString.Data());
  TString drawOpt=(optString.IsNull()?"EP":optString);
  // draw spectra
  if(!optOnlyMC) {

    if(optSB && !optOnlyRaw)       { fHistSB->Draw(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++; }
    else if(optSgn && !optOnlyRaw) { fHistSign->Draw(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++; }
    else if(optOnlySig) { 
      drawOpt=(optString.IsNull()?"EP":optString);
      fHistSignal->DrawCopy(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++;
      drawOpt=(optString.IsNull()?"L same":optString);
      if(dynamic_cast<TF1*>(fgPeakShape)) { static_cast<TF1*>(fgPeakShape)->DrawCopy(drawOpt); i++; }
    }
    else {
      fHistDataPM->DrawCopy(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++;
      if(fMethod==kRotation || fMethod==kEventMixing || fMethod==kCocktail)  drawOpt=(optString.IsNull()?"HIST":optString);
      fHistBackground->DrawCopy(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++;
      drawOpt=(optString.IsNull()?"EP":optString);
      if(!optOnlyRaw) {
	drawOpt=(optString.IsNull()?"EP":optString);
	fHistSignal->DrawCopy(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++;
	drawOpt=(optString.IsNull()?"L same":optString);
	if(dynamic_cast<TF1*>(fgPeakShape)) { static_cast<TF1*>(fgPeakShape)->DrawCopy(drawOpt); i++; }
      }
    }
    // add cocktail
    if(optCocktail && fHistCocktail) {
      drawOpt=(optString.IsNull()?"HIST":optString);
      fHistCocktail->DrawCopy((drawOpt+"same").Data()); i++;
    }

  }

  // draw MC signals
  if(!optNoMC) {
    if(!optSgn) {
      TIter nextObj(fArrHists);
      TH1 *hmc;
      Int_t isty=0;
      while ( (hmc = dynamic_cast<TH1*>(nextObj())) ) {
	TString key=hmc->GetTitle(); //hmc->GetName();
	TString tit=hmc->GetTitle();
	if(key.CountChar('_')!=1) continue; // only reconstr. MC signals
	// remove cocktail subtracted signals
	if     (optOnlySig &&  fCocktailSubtr && FindObjectByTitle(fArrCocktail,key)) continue;
	else if(optOnlySig && !fCocktailSubtr && !FindObjectByTitle(fArrCocktail,key)) continue;
	PairAnalysisStyler::Style(hmc,isty++);
	// check if rebinning is necessary
	if(fHistSignal->GetNbinsX()!=hmc->GetNbinsX()) {
	  if(fBinLimits)     {
	    hmc = hmc->Rebin(fBinLimits->GetSize()-1,key.Data(),fBinLimits->GetArray());
	    hmc->SetTitle(tit.Data());
	    hmc->Scale(1.,"width");
	  }
	  if(fRebin>1)       hmc->Rebin(fRebin);
	}
	if(optSB) hmc->Divide(fHistSignal);
	if(fPlotMin!=fPlotMax) hmc->SetAxisRange(fPlotMin,fPlotMax, "X");
	hmc->Draw(i>0?"HISTsame":"HIST");
	i++;
      }
    }
  }

  // axis maximum
  Double_t max=-1e10;
  Double_t min=+1e10;
  //TListIter nextObj(gPad->GetListOfPrimitives(),kIterForward);
  TListIter nextObj(gPad->GetListOfPrimitives(),kIterBackward);
  //  TObject *obj;
  while ((obj = nextObj())) {
    if(obj->InheritsFrom(TH1::Class())) {
      TH1 *hobj = static_cast<TH1*>(obj);
      max=TMath::Max(max,PairAnalysisHelper::GetContentMaximum(hobj)); //hobj->GetMaximum();
      hobj->SetMaximum(max*1.1);
      if( gPad->GetLogy() && PairAnalysisHelper::GetContentMinimum(hobj)<0.) continue;
      min=TMath::Min(min,PairAnalysisHelper::GetContentMinimum(hobj));//hobj->GetBinContent(hobj->GetMinimumBin()));
      hobj->SetMinimum( min*(min<0.?1.1:0.9) ); //TODO: doesnt work, why?? Negative values?

      //      Printf("after %s max%f \t min%f",hobj->GetTitle(),max,min);
      // automatically set log option
      if(gPad->GetLogy() && max/(min>0.?min:1.) > TMath::Power(10.,TGaxis::GetMaxDigits())) {
	hobj->GetYaxis()->SetMoreLogLabels(kFALSE);
	hobj->GetYaxis()->SetNoExponent(kFALSE);
      }
      //      if(gPad->GetLogy()) break;
    }
  }

  // add legend entries
  TListIter nextObjFwd(gPad->GetListOfPrimitives(),kIterForward);
  if(optLeg && leg) {
    nextObjFwd.Reset();
    Int_t ileg=0;
    while ((obj = nextObjFwd())) {
      if(obj->InheritsFrom(TH1::Class())) {
       	TH1 *hleg = static_cast<TH1*>(obj);
	if(nobj && ileg<nobj)  { ileg++; continue; }

	TString histClass=hleg->GetTitle(); //hleg->GetName();
	histClass.ReplaceAll("Pair.", "");
	histClass.ReplaceAll("Pair_", "");
	// change default signal names to titles
	for(Int_t isig=0; isig<PairAnalysisSignalMC::kNSignals; isig++) {
	  histClass.ReplaceAll(PairAnalysisSignalMC::fgkSignals[isig][0],PairAnalysisSignalMC::fgkSignals[isig][1]);
	}
	// remove trailing and leading spaces
	histClass.Remove(TString::kBoth,' ');
	// histClass.Remove(TString::kBoth,'.');
	// histClass.Remove(TString::kBoth,'_');

	// modify legend option
	TString legOpt = hleg->GetDrawOption();
	legOpt.ToLower();
	legOpt+="l";
	legOpt.ReplaceAll("hist","");
	legOpt.ReplaceAll("same","");
	legOpt.ReplaceAll("scat","");
	legOpt.ReplaceAll("col","");
	legOpt.ReplaceAll("z","");
	legOpt.ReplaceAll("e","");
	//	Printf("hist %s legopt %s",histClass.Data(),legOpt.Data());
	if(ileg==nobj && optTask) leg->AddEntry((TObject*)0x0,fArrHists->GetName(),"");
	leg->AddEntry(hleg,histClass.Data(),legOpt.Data());
	ileg++;
      }
    }
  }

  // legend
  if (leg) {
    leg->SetMargin(0.075);
    leg->SetFillStyle(0);
    leg->SetTextSize(0.02); // TODO: replaced by global legend textsize for root > v5-34-26
    PairAnalysisStyler::SetLegendCoordinates(leg);
    leg->Draw();
  }

  // draw statistics box
  if(optStat) DrawStats(0.7, gPad->GetBottomMargin()+gStyle->GetTickLength("Y"),
			1.-gPad->GetRightMargin()-gStyle->GetTickLength("X"),
			gPad->GetBottomMargin()+gStyle->GetTickLength("Y")+ 5*0.025);


  // styling
  gPad->RedrawAxis();

}

