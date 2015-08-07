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
  //  fErrors(2) = ((s+b)>0 ? fValues(2)*TMath::Sqrt(be*be + TMath::Power(se*(s+2*b)/s, 2)) / 2 / (s+b) : 0);

}

//______________________________________________
void PairAnalysisSignalExt::ProcessLS(TObjArray* const arrhist)
{
  //
  // signal subtraction
  //
  if(fHistRfactor)    delete fHistRfactor;    fHistRfactor=0x0;
  if(fHistDataPM)     delete fHistDataPM;     fHistDataPM=0x0;
  if(fHistDataME)     delete fHistDataME;     fHistDataME=0x0;
  if(fHistSignal)     delete fHistSignal;     fHistSignal=0x0;
  if(fHistBackground) delete fHistBackground; fHistBackground=0x0;

  fHistDataPP = (TH1*)FindObject(arrhist,PairAnalysis::kSEPP)->Clone("histPP");  // ++    SE
  fHistDataPM = (TH1*)FindObject(arrhist,PairAnalysis::kSEPM)->Clone("histPM");  // +-    SE
  fHistDataMM = (TH1*)FindObject(arrhist,PairAnalysis::kSEMM)->Clone("histMM");  // --    SE
  fHistDataPP->Sumw2();
  fHistDataPM->Sumw2();
  fHistDataMM->Sumw2();
  fHistDataPP->SetDirectory(0);
  fHistDataPM->SetDirectory(0);
  fHistDataMM->SetDirectory(0);

  // rebin the histograms
  if (fRebin>1) {
    fHistDataPP->Rebin(fRebin);
    fHistDataPM->Rebin(fRebin);
    fHistDataMM->Rebin(fRebin);
  }
  // init histograms for R-factor, subtracted signal, background
  fHistRfactor = new TH1D("HistRfactor", "Rfactor;;N^{mix}_{+-}/2#sqrt{N^{mix}_{++} N^{mix}_{--}}",
                          fHistDataPM->GetXaxis()->GetNbins(),
                          fHistDataPM->GetXaxis()->GetXmin(), fHistDataPM->GetXaxis()->GetXmax());
  fHistRfactor->Sumw2();
  fHistRfactor->SetDirectory(0);
  fHistSignal = new TH1D("HistSignal", "Like-Sign substracted signal",
			 fHistDataPM->GetXaxis()->GetNbins(),
			 fHistDataPM->GetXaxis()->GetXmin(), fHistDataPM->GetXaxis()->GetXmax());
  fHistSignal->SetDirectory(0);
  fHistBackground = new TH1D("HistBackground", "Like-sign contribution",
			     fHistDataPM->GetXaxis()->GetNbins(),
			     fHistDataPM->GetXaxis()->GetXmin(), fHistDataPM->GetXaxis()->GetXmax());
  fHistBackground->SetDirectory(0);

  // fill background histogram
  for(Int_t ibin=1; ibin<=fHistDataPM->GetXaxis()->GetNbins(); ibin++) {
    Float_t pp = fHistDataPP->GetBinContent(ibin);
    Float_t mm = fHistDataMM->GetBinContent(ibin);

    Float_t background = 2*TMath::Sqrt(pp*mm);
    Float_t ebackground = TMath::Sqrt(mm+pp);
    // Arithmetic mean instead of geometric
    if (fMethod==kLikeSignArithm || fMethod==kLikeSignArithmRcorr ){
      background=(pp+mm);
      ebackground=TMath::Sqrt(pp+mm);
      if (TMath::Abs(ebackground)<1e-30) ebackground=1;
    }
    // set contents
    fHistBackground->SetBinContent(ibin, background);
    fHistBackground->SetBinError(ibin, ebackground);
  }

  //correct LS spectrum bin-by-bin with R factor obtained in mixed events
  if(fMixingCorr || fMethod==kLikeSignRcorr || fMethod==kLikeSignArithmRcorr) {

    TH1* histMixPP = (TH1*)FindObject(arrhist,PairAnalysis::kMEPP)->Clone("mixPP");  // ++    ME
    TH1* histMixMM = (TH1*)FindObject(arrhist,PairAnalysis::kMEMM)->Clone("mixMM");  // --    ME


    TH1* histMixPM = 0x0;
    if(FindObject(arrhist,PairAnalysis::kMEMP)) {
      histMixPM = (TH1*)FindObject(arrhist,PairAnalysis::kMEMP)->Clone("mixMP");  // -+    ME
      histMixPM->Sumw2();
    }

    // merge ME +- with -+ if needed
    TH1 *htmp=(TH1*)FindObject(arrhist,PairAnalysis::kMEPM);
    if (!histMixPM && htmp) histMixPM  = (TH1*)htmp->Clone("mixPM");                 // +-    ME
    else if(htmp)           histMixPM->Add(htmp);
    //    }

    // protection
    if (!histMixPM){
      Error("ProcessME", "For R-factor correction the mixed event histograms are required. No +- histogram found");
      delete histMixPP;
      delete histMixMM;
      return;
    }

    // rebin the histograms
    if (fRebin>1) {
      histMixPP->Rebin(fRebin);
      histMixMM->Rebin(fRebin);
      histMixPM->Rebin(fRebin);
    }

    // fill R-factor histogram
    for(Int_t ibin=1; ibin<=histMixPM->GetXaxis()->GetNbins(); ibin++) {
      Float_t pp  = histMixPP->GetBinContent(ibin);
      Float_t ppe = histMixPP->GetBinError(ibin);
      Float_t mm  = histMixMM->GetBinContent(ibin);
      Float_t mme = histMixMM->GetBinError(ibin);
      Float_t pm  = histMixPM->GetBinContent(ibin);
      Float_t pme = histMixPM->GetBinError(ibin);

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
    fHistBackground->Multiply(fHistRfactor);

    // clean up
    if (histMixPP) delete histMixPP;
    if (histMixMM) delete histMixMM;
    if (histMixPM) delete histMixPM;
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

  if (!FindObject(arrhist,PairAnalysis::kSEPM) ||
      !( FindObject(arrhist,PairAnalysis::kMEMP) || FindObject(arrhist,PairAnalysis::kMEPM) ) ) {
    Error("ProcessEM","Either OS or mixed histogram missing");
    return;
  }

  if(fHistRfactor)    delete fHistRfactor;    fHistRfactor=0x0;
  if(fHistDataPM)     delete fHistDataPM;     fHistDataPM=0x0;
  if(fHistDataME)     delete fHistDataME;     fHistDataME=0x0;
  if(fHistSignal)     delete fHistSignal;     fHistSignal=0x0;
  if(fHistBackground) delete fHistBackground; fHistBackground=0x0;

  // init histograms, subtracted signal, background
  fHistDataPM = (TH1*)FindObject(arrhist,PairAnalysis::kSEPM)->Clone("histPM");  // +-    SE
  fHistDataPM->Sumw2();
  fHistDataPM->SetDirectory(0x0);
  // get ME distributions
  if (FindObject(arrhist,PairAnalysis::kMEMP)){
    fHistDataME   = (TH1*)FindObject(arrhist,PairAnalysis::kMEMP)->Clone("histMPME");  // -+    ME
  }
  if (FindObject(arrhist,PairAnalysis::kMEPM)){
    TH1 *htmp=(TH1*)FindObject(arrhist,PairAnalysis::kMEPM);
    if (!fHistDataME) fHistDataME   = (TH1*)htmp->Clone("histMPME");  // -+    ME
    else MergeObjects(fHistDataME,htmp);
  }
  fHistDataME->SetDirectory(0x0); // added

  fHistBackground = (TH1*)fHistDataME->Clone("ME_Background");
  fHistBackground->SetDirectory(0x0);
  fHistBackground->Sumw2();

  // rebin the histograms
  if (fRebin>1) {
    fHistDataPM->Rebin(fRebin);
    fHistDataME->Rebin(fRebin);
    fHistBackground->Rebin(fRebin);
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
  fHistSignal = MergeObjects(fHistDataPM,fHistBackground,-1.);

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

  if (!FindObject(arrhist,PairAnalysis::kSEPM) || !FindObject(arrhist,PairAnalysis::kSEPMRot) ){
    Error("ProcessTR","Either OS or rotation histogram missing");
    return;
  }

  if(fHistRfactor)    delete fHistRfactor;    fHistRfactor=0x0;
  if(fHistDataPM)     delete fHistDataPM;     fHistDataPM=0x0;
  if(fHistDataME)     delete fHistDataME;     fHistDataME=0x0;
  if(fHistSignal)     delete fHistSignal;     fHistSignal=0x0;
  if(fHistBackground) delete fHistBackground; fHistBackground=0x0;

  // init histograms for subtracted signal, background
  fHistDataPM = (TH1*)FindObject(arrhist,PairAnalysis::kSEPM)->Clone("histPM");  // +-    SE
  fHistDataPM->Sumw2();
  fHistDataPM->SetDirectory(0x0);
  fHistBackground = (TH1*)FindObject(arrhist,PairAnalysis::kSEPMRot)->Clone("histRotation");
  fHistBackground->Sumw2();
  fHistBackground->SetDirectory(0x0);

  // scale according to number of interations used in track rotation
  fHistBackground->Scale(1./fNiterTR);

  // rebin the histograms
  if (fRebin>1) {
    fHistDataPM->Rebin(fRebin);
    fHistBackground->Rebin(fRebin);
  }

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

  // signal
  fHistSignal=(TH1*)fHistDataPM->Clone("histSignal");
  fHistSignal->Add(fHistBackground,-1.);
  fHistSignal->SetDirectory(0x0);

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
  if(TMath::Abs(fHistDataPM->GetBinWidth(1)-fHistDataPM->GetBinWidth(fHistDataPM->GetNbinsX()-2))<1.e-6) {
    ytitle+=Form("/%.0f MeV/#it{c}^{2}",fHistDataPM->GetBinWidth(1)*1e3);
  }
  fHistDataPM->GetYaxis()->SetTitle(ytitle.Data());

  // styling
  fHistDataPM->SetName("unlike-sign");
  fHistDataPM->UseCurrentStyle();
  fHistDataPM->SetTitle("");
  PairAnalysisStyler::Style(fHistDataPM,PairAnalysisStyler::kRaw);

  fHistBackground->SetName(fgkBackgroundMethodNames[fMethod]);
  fHistBackground->UseCurrentStyle();
  fHistBackground->SetTitle("");
  PairAnalysisStyler::Style(fHistBackground,PairAnalysisStyler::kBgrd);

  fHistSignal->SetName("signal");
  fHistSignal->UseCurrentStyle();
  fHistSignal->SetTitle("");
  PairAnalysisStyler::Style(fHistSignal,PairAnalysisStyler::kSig);

  if(optSB) {
    fHistSB->SetName("signal");
    fHistSB->UseCurrentStyle();
    fHistSB->SetTitle("");
    PairAnalysisStyler::Style(fHistSB,PairAnalysisStyler::kSig);
  }
  else if(optSgn) {
    fHistSign->SetName("signal");
    fHistSign->UseCurrentStyle();
    fHistSign->SetTitle("");
    PairAnalysisStyler::Style(fHistSign,PairAnalysisStyler::kSig);
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
  if(optSB && !optOnlyRaw)       { fHistSB->Draw(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++; }
  else if(optSgn && !optOnlyRaw) { fHistSign->Draw(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++; }
  else {
    fHistDataPM->DrawCopy(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++;
    if(fMethod==kRotation || fMethod==kEventMixing)  drawOpt=(optString.IsNull()?"HIST":optString);
    fHistBackground->DrawCopy(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++;
    drawOpt=(optString.IsNull()?"EP":optString);
    if(!optOnlyRaw) {
      drawOpt=(optString.IsNull()?"EP":optString);
      fHistSignal->DrawCopy(i>0?(drawOpt+"same").Data():drawOpt.Data()); i++;
      drawOpt=(optString.IsNull()?"L same":optString);
      if(dynamic_cast<TF1*>(fgPeakShape)) { static_cast<TF1*>(fgPeakShape)->DrawCopy(drawOpt); i++; }
    }
  }

  // draw MC signals
  if(!optNoMC) {
    if(!optSgn) {
      TIter nextObj(fArrHists);
      TH1 *hmc;
      Int_t isty=0;
      while ( (hmc = dynamic_cast<TH1*>(nextObj())) ) {
	TString key=hmc->GetName();
	if(key.CountChar('_')!=1) continue; // only reconstr. MC signals
	PairAnalysisStyler::Style(hmc,isty++);
	if(optSB) hmc->Divide(fHistSignal);
	hmc->Draw("HISTsame");
	i++;
      }
    }
  }

  // axis maximum
  Double_t max=-1e10;
  Double_t min=+1e10;
  TListIter nextObj(gPad->GetListOfPrimitives(),kIterForward);
  //  TObject *obj;
  while ((obj = nextObj())) {
    if(obj->InheritsFrom(TH1::Class())) {
      max=TMath::Max(max,static_cast<TH1*>(obj)->GetMaximum());
      min=TMath::Min(min,static_cast<TH1*>(obj)->GetMinimum());
      // automatically set log option
      if(gPad->GetLogy() && max/(min>0.?min:1.) > TMath::Power(10.,TGaxis::GetMaxDigits())) {
	static_cast<TH1*>(obj)->GetYaxis()->SetMoreLogLabels(kFALSE);
	static_cast<TH1*>(obj)->GetYaxis()->SetNoExponent(kFALSE);
      }
      if(gPad->GetLogy()) break;
      static_cast<TH1*>(obj)->SetMaximum(max*1.1);
      static_cast<TH1*>(obj)->SetMinimum(min*1.1); //TODO: doesnt work, why??
    }
  }

  // add legend entries
  if(optLeg && leg) {
    nextObj.Reset();
    Int_t ileg=0;
    while ((obj = nextObj())) {
      if(obj->InheritsFrom(TH1::Class())) {
       	TH1 *hleg = static_cast<TH1*>(obj);
	if(nobj && ileg<nobj)  { ileg++; continue; }

	TString histClass=hleg->GetName();
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
	if(ileg==nobj) leg->AddEntry((TObject*)0x0,fArrHists->GetName(),"");
	leg->AddEntry(hleg,histClass.Data(),legOpt.Data());
	ileg++;
      }
    }
  }

  // legend
  if (leg) {
    leg->SetFillStyle(0);
    leg->SetTextSize(0.02);
    Int_t nleg = ((TList*)leg->GetListOfPrimitives())->GetSize();
    leg->SetY1(leg->GetY2()-nleg*.025); //i*.05
    leg->Draw();
  }

  // draw statistics box
  if(optStat) DrawStats(0.7, gPad->GetBottomMargin()+gStyle->GetTickLength("Y"),
			1.-gPad->GetRightMargin()-gStyle->GetTickLength("X"),
			gPad->GetBottomMargin()+gStyle->GetTickLength("Y")+ 5*0.025);


  // styling
  gPad->RedrawAxis();

}

