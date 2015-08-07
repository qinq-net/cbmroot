///////////////////////////////////////////////////////////////////////////
//                Dielectron SignalBase                                  //
//                                                                       //
//                                                                       //
/*
Base class for signal extraction from a histogram or an array of histograms
The histogram is assumed to be an inv. mass spectrum,
the array of histograms is assumed to be an array with inv. mass histograms
resulting from single and mixed events, as defined in PairAnalysis.cxx

*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////


#include <TVectorT.h>
#include <TPaveText.h>
#include <TF1.h>
#include <TH1.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TDatabasePDG.h>
#include <TList.h>

#include "PairAnalysisSignalFunc.h"
#include "PairAnalysisSignalBase.h"

ClassImp(PairAnalysisSignalBase)

TH1F* PairAnalysisSignalBase::fgHistSimPM=0x0;
TObject* PairAnalysisSignalBase::fgPeakShape=0x0;
const char* PairAnalysisSignalBase::fgkValueNames[7] = {
  "S",
  "B",
  "S/#sqrt{S+B}",
  "S/B",
  "Mass",
  "MassWidth",
  "ChiSqNDFmatch"};
const char* PairAnalysisSignalBase::fgkBackgroundMethodNames[10] = {
  "FittedMC",
  "Fitted",
  "like-sign",
  "like-sign (arithm.)",
  "like-sign #times R(#Deltam)",
  "like-sign (arithm.) #times R(#Deltam)",
  "fitted like-sign",
  "mixed event",
  "fitted mixed event",
  "track rotation" };

PairAnalysisSignalBase::PairAnalysisSignalBase() :
  TNamed(),
  fArrHists(0x0),
  fHistSignal(0),
  fHistSB(0),
  fHistSign(0),
  fHistBackground(0),
  fHistDataPM(0),
  fHistDataPP(0),
  fHistDataMM(0),
  fHistDataME(0),
  fHistRfactor(0),
  fHistSignalMC(0),
  fValues(7),
  fErrors(7),
  fIntMin(0),
  fIntMax(0),
  fFitMin(0),
  fFitMax(0),
  fRebin(1),
  fMethod(kLikeSign),
  fScaleMin(0.),
  fScaleMax(0.),
  fScaleMin2(0.),
  fScaleMax2(0.),
  fNiterTR(1),
  fScaleFactor(1.),
  fMixingCorr(kFALSE),
  fPeakMethod(kBinCounting),
  fProcessed(kFALSE),
  fPeakIsTF1(kFALSE),
  fPOIpdg(443)
{
  //
  // Default Constructor
  //
}

//______________________________________________
PairAnalysisSignalBase::PairAnalysisSignalBase(const char* name, const char* title) :
  TNamed(name, title),
  fArrHists(0x0),
  fHistSignal(0),
  fHistSB(0),
  fHistSign(0),
  fHistBackground(0),
  fHistDataPM(0),
  fHistDataPP(0),
  fHistDataMM(0),
  fHistDataME(0),
  fHistRfactor(0),
  fHistSignalMC(0),
  fValues(7),
  fErrors(7),
  fIntMin(0),
  fIntMax(0),
  fFitMin(0),
  fFitMax(0),
  fRebin(1),
  fMethod(kLikeSign),
  fScaleMin(0.),
  fScaleMax(0.),
  fScaleMin2(0.),
  fScaleMax2(0.),
  fNiterTR(1),
  fScaleFactor(1.),
  fMixingCorr(kFALSE),
  fPeakMethod(kBinCounting),
  fProcessed(kFALSE),
  fPeakIsTF1(kFALSE),
  fPOIpdg(443)
{
  //
  // Named Constructor
  //
}

//______________________________________________
PairAnalysisSignalBase::PairAnalysisSignalBase(const PairAnalysisSignalBase &c) :
  TNamed(c.GetName(), c.GetTitle()),
  fArrHists(c.fArrHists),
  fHistSignal(c.GetSignalHistogram()),
  fHistSB(c.GetSoverBHistogram()),
  fHistSign(c.GetSignificanceHistogram()),
  fHistBackground(c.GetBackgroundHistogram()),
  fHistDataPM(c.GetUnlikeSignHistogram()),
  fHistDataPP(0x0),
  fHistDataMM(0x0),
  fHistDataME(0x0),
  fHistRfactor(c.GetRfactorHistogram()),
  fHistSignalMC(c.GetMCSignalShape()),
  fValues(c.GetValues()),
  fErrors(c.GetErrors()),
  fIntMin(c.GetIntegralMin()),
  fIntMax(c.GetIntegralMax()),
  fFitMin(c.GetFitMin()),
  fFitMax(c.GetFitMax()),
  fRebin(c.GetRebin()),
  fMethod(c.GetMethod()),
  fScaleMin(c.GetScaleMin()),
  fScaleMax(c.GetScaleMax()),
  fScaleMin2(c.GetScaleMin2()),
  fScaleMax2(c.GetScaleMax2()),
  fNiterTR(1),
  fScaleFactor(c.GetScaleFactor()),
  fMixingCorr(kFALSE),
  fPeakMethod(c.GetExtractionMethod()),
  fProcessed(kFALSE),
  fPeakIsTF1(kFALSE),
  fPOIpdg(c.GetParticleOfInterest())
{
  //
  // Copy Constructor
  //
}

//______________________________________________
PairAnalysisSignalBase::~PairAnalysisSignalBase()
{
  //
  // Default Destructor
  //
  if (fArrHists)       delete fArrHists;
  if (fHistSignal)     delete fHistSignal;
  if (fHistSB)         delete fHistSB;
  if (fHistSign)       delete fHistSign;
  if (fHistBackground) delete fHistBackground;
  if (fHistDataPP)     delete fHistDataPP;
  if (fHistDataPM)     delete fHistDataPM;
  if (fHistDataMM)     delete fHistDataMM;
  if (fHistDataME)     delete fHistDataME;
  if (fHistRfactor)    delete fHistRfactor;
  if (fHistSignalMC)   delete fHistSignalMC;
}

//______________________________________________
TPaveText* PairAnalysisSignalBase::DrawStats(Double_t x1/*=0.*/, Double_t y1/*=0.*/, Double_t x2/*=0.*/, Double_t y2/*=0.*/)
{
  //
  // Draw extracted values in a TPaveText
  // with the corners x1,y2,x2,y2
  //
  if (TMath::Abs(x1)<1e-20&&TMath::Abs(x2)<1e-20){
    x1=.6;
    x2=.9;
    y1=.7;
    y2=.9;
  }

  TPaveText *t=new TPaveText(x1,y1,x2,y2,"brNDC");
  // t->SetFillColor(kWhite);
  // t->SetBorderSize(1);
  t->SetTextAlign(12);
  t->AddText(Form("%.2f < %s < %.2f GeV/c^{2}", fIntMin, "m_{inv}", fIntMax));
  t->AddText(Form("%s: %.4g #pm %.4g", fgkValueNames[0], fValues(0), fErrors(0)));
  t->AddText(Form("%s: %.4g #pm %.4g", fgkValueNames[1],  fValues(1), fErrors(1)));
  t->AddText(Form("%s: %.1f #pm %.1f", fgkValueNames[2],  fValues(2), fErrors(2)));
  Int_t smallS2B=(fValues(3)<0.1?1:100);
  t->AddText(Form("%s: %.2f #pm %.2f%s", fgkValueNames[3],  fValues(3)*100/smallS2B, fErrors(3)*100/smallS2B, smallS2B>1?"":"%"));
  if(fValues(4)>0)
    t->AddText(Form("Mass: %.2f #pm %.2f GeV/c^{2}", fValues(4), fErrors(4)));
  if(fValues(5)>0)
    t->AddText(Form("Mass res.: %.1f #pm %.1f MeV/c^{2}", 1000*fValues(5), 1000*fErrors(5)));
  if(fValues(6)>0)
    t->AddText(Form("Match chi2/ndf.: %.1f", fValues(6)));
  t->Draw();

  return t;
}

//______________________________________________
void PairAnalysisSignalBase::Print(Option_t */*option*/) const
{
  //
  // Print the statistics
  //
  printf("Signal : %.5g #pm %.5g (+-%.3g%%)\n",fValues(0), fErrors(0), 100*fErrors(0)/fValues(0));
  printf("Backgnd: %.5g #pm %.5g (+-%.3g%%)\n",fValues(1), fErrors(1), 100*fErrors(1)/fValues(1));
  printf("Signif : %.5g #pm %.5g (+-%.3g%%)\n",fValues(2), fErrors(2), 100*fErrors(2)/fValues(2));
  printf("SoB    : %.5g #pm %.5g (+-%.3g%%)\n",fValues(3), fErrors(3), 100*fErrors(3)/fValues(3));
  if(fValues(4)>0)
    printf("Mass: %.5g #pm %.5g\n", fValues(4), fErrors(4));
  if(fValues(5)>0)
    printf("Mass res.: %.5g #pm %.5g\n", fValues(5), fErrors(5));
  if(fValues(6)>0)
    printf("Match chi2/ndf: %.5g \n", fValues(6));
  printf("Scale  : %.5g \n",fScaleFactor);
  printf("Mass int.: %.5g - %.5g \n",fIntMin,fIntMax);
}

//______________________________________________
Double_t PairAnalysisSignalBase::ScaleHistograms(TH1* histRaw, TH1* histBackground, Double_t intMin, Double_t intMax)
{
  //
  // scale histBackground to match the integral of histRaw in the interval intMin, intMax
  //

  //protect using over and underflow bins in normalisation calculation
  if (intMin<histRaw->GetXaxis()->GetXmin())        intMin=histRaw->GetXaxis()->GetXmin();
  if (intMin<histBackground->GetXaxis()->GetXmin()) intMin=histBackground->GetXaxis()->GetXmin();

  if (intMax>histRaw->GetXaxis()->GetXmax())
    intMax=histRaw->GetXaxis()->GetXmax()-histRaw->GetBinWidth(histRaw->GetNbinsX())/2.;
  if (intMax>histBackground->GetXaxis()->GetXmax())
    intMax=histBackground->GetXaxis()->GetXmax()-histBackground->GetBinWidth(histBackground->GetNbinsX())/2.;

  Double_t intRaw      = histRaw       ->Integral(histRaw->FindBin(intMin),       histRaw->FindBin(intMax));
  Double_t intBack     = histBackground->Integral(histBackground->FindBin(intMin),histBackground->FindBin(intMax));
  fScaleFactor         = intBack>0?intRaw/intBack:1.;
  // scale
  histBackground->Sumw2();
  histBackground->Scale(fScaleFactor);

  return fScaleFactor;
}

//______________________________________________
Double_t PairAnalysisSignalBase::ScaleHistograms(TH1* histRaw, TH1* histBackground, Double_t intMin, Double_t intMax, Double_t intMin2, Double_t intMax2)
{
  //
  // scale histBackground to match the integral of histRaw in the interval intMin, intMax and intMin2, intMax2
  //

  if(TMath::Abs(intMin2-intMax2)<0.00001) return (ScaleHistograms(histRaw, histBackground, intMin, intMax));

  //protect using over and underflow bins in normalisation calculation
  if (intMin<histRaw->GetXaxis()->GetXmin())        intMin=histRaw->GetXaxis()->GetXmin();
  if (intMin<histBackground->GetXaxis()->GetXmin()) intMin=histBackground->GetXaxis()->GetXmin();

  if (intMax2>histRaw->GetXaxis()->GetXmax())
    intMax2=histRaw->GetXaxis()->GetXmax()-histRaw->GetBinWidth(histRaw->GetNbinsX())/2.;
  if (intMax2>histBackground->GetXaxis()->GetXmax())
    intMax2=histBackground->GetXaxis()->GetXmax()-histBackground->GetBinWidth(histBackground->GetNbinsX())/2.;

  Double_t intRaw      = histRaw       ->Integral(histRaw->FindBin(intMin),       histRaw->FindBin(intMax));
  Double_t intBack     = histBackground->Integral(histBackground->FindBin(intMin),histBackground->FindBin(intMax));
  intRaw              += histRaw       ->Integral(histRaw->FindBin(intMin2),       histRaw->FindBin(intMax2));
  intBack             += histBackground->Integral(histBackground->FindBin(intMin2),histBackground->FindBin(intMax2));

  fScaleFactor = intBack>0?intRaw/intBack:1.;
  // scale
  histBackground->Sumw2();
  histBackground->Scale(fScaleFactor);

  return fScaleFactor;
}

//______________________________________________
TH1* PairAnalysisSignalBase::MergeObjects(TH1* obj1, TH1* obj2, Double_t val) {
  //
  // function to merge all TH1 inherited objects
  // (needed because TProfile::Add with negative 'val' does not what is needed)
  // what about TProfile2D (inherits from TH2D) ?
  //
  if(!obj1) return obj2;
  if(!obj2) return obj1;

  TString key=Form("%s_%s",obj1->GetName(),"Signal");
  if(obj1->IsA()==TProfile2D::Class() && obj2->IsA()==TProfile2D::Class() ) {
    if(val>=0.) {
      //summation
      ((TProfile2D*)obj1)->Add( ((TProfile2D*)obj2), val);
      return (TH1*)obj1->Clone(key.Data()); //TOCHECK: clone needed??
    }
    else return 0x0;
  }
  else if(obj1->IsA()==TProfile::Class() && obj2->IsA()==TProfile::Class() ) {
    if(val>=0.) {
      //summation
      ((TProfile*)obj1)->Add( ((TProfile*)obj2), val);
      return (TH1*)obj1->Clone(key.Data()); //TOCHECK: clone needed??
    }
    else  {
      // ONLY 1D subtraction
      TH1D *histSign = new TH1D(key.Data(), "",
       				obj1->GetXaxis()->GetNbins(),
				obj1->GetXaxis()->GetXmin(), obj1->GetXaxis()->GetXmax());
      histSign->Sumw2();
      histSign->SetDirectory(0);

      for(Int_t i=0; i<=obj1->GetNbinsX(); i++) {
     	histSign->SetBinContent(i, obj1->GetBinContent(i)-obj2->GetBinContent(i));
     	histSign->SetBinError(i, TMath::Sqrt(obj1->GetBinError(i)*obj1->GetBinError(i) -
					     obj2->GetBinError(i)*obj2->GetBinError(i))
			      );
      }
      return histSign;
    }

    /*
      TList listH;
    TString listHargs;
    listHargs.Form("((TCollection*)0x%lx)", (ULong_t)&listH);
    Int_t error = 0;
    listH.Add(obj2);
    obj1->Execute("Merge", listHargs.Data(), &error);
    */
  }
  else { // Histograms
    if(val<0.) { // subtraction
      TH1 *histSign = (TH1*) obj1->Clone(key.Data());
      histSign->Add(obj2, val);
      return histSign;
    }
    else { // merge
      obj1->Add(obj2, val);
      return obj1;
    }
  }

}

//______________________________________________
TObject* PairAnalysisSignalBase::DescribePeakShape(ESignalExtractionMethod method, Bool_t replaceValErr,  TH1F *mcShape) {
  //
  // Describe the extracted peak by the selected method and overwrite signal etc if needed
  //

  if(replaceValErr) fPeakMethod=method;
  Double_t data=0.;
  Double_t mc=0.;
  Double_t massPOI=TDatabasePDG::Instance()->GetParticle(fPOIpdg)->Mass();
  Double_t nPOI     = fHistSignal->GetBinContent(fHistSignal->FindBin(massPOI));
  Double_t binWidth = fHistSignal->GetBinWidth(  fHistSignal->FindBin(massPOI));
  TF1 *fit=0x0;
  Int_t fitResult=0;
  Int_t parMass =-1;
  Int_t parSigma=-1;
  PairAnalysisSignalFunc fct;// = 0;//new PairAnalysisSignalFunc();

  // do the scaling/fitting
  switch(method) {
  case kBinCounting: /*nothing needs to be done*/ 
    break;

  case kMCScaledMax:
    if(!mcShape) { Error("DescribePeakShape","No MC histogram passed. Returning."); return 0x0; }
    data = fHistSignal->GetBinContent(fHistSignal->FindBin(massPOI));
    mc   = mcShape->GetBinContent(fHistSignal->FindBin(massPOI));
    mcShape->Scale(data / mc );
    break;

  case kMCScaledInt:
    if(!mcShape) { Error("DescribePeakShape","No MC histogram passed. Returning."); return 0x0; }
    if(mcShape->GetBinWidth(1)!=fHistSignal->GetBinWidth(1)) 
      printf(" WARNING: MC and signal histogram have different bin widths. \n");
    data = fHistSignal->Integral(fHistSignal->FindBin(fIntMin),fHistSignal->FindBin(fIntMax));
    mc   = mcShape->Integral(mcShape->FindBin(fIntMin),mcShape->FindBin(fIntMax));
    mcShape->Scale(data / mc );
    break;

  case kMCFitted:
    if(!mcShape && !fgHistSimPM) {
      printf(" ERROR: No MC histogram passed or set. Returning. \n"); return 0x0; }
    if(!fgHistSimPM) fgHistSimPM=mcShape;
    if(fgHistSimPM->GetBinWidth(1)!=fHistSignal->GetBinWidth(1)) fgHistSimPM->Rebin(fRebin);
    //    fit = new TF1("fitMC",PairAnalysisSignalFunc::PeakFunMC,fFitMin,fFitMax,1);
    fit = new TF1("fitMC",&fct,&PairAnalysisSignalFunc::PeakFunMC,fFitMin,fFitMax,1);
    fit->SetParNames("N");
    fitResult = fHistSignal->Fit(fit,"RNI0Q");
    break;

  case kCrystalBall:
    fit = new TF1("fitCB",&fct,&PairAnalysisSignalFunc::PeakFunCB,fFitMin,fFitMax,5);
    fit->SetParNames("alpha","n","meanx","sigma","N");
    //  fit->SetParameters(-.2,5.,gMjpsi,.06,20);
    //  fit->SetParameters(1.,3.6,gMjpsi,.08,700);
    fit->SetParameters(0.4, 4.0, massPOI, 0.025, 1.3*nPOI);
    fit->SetParLimits(0, 0.0,           1.           );
    fit->SetParLimits(1, 0.01,          10.          );
    fit->SetParLimits(2, massPOI-0.02,  massPOI+0.02 );
    fit->SetParLimits(3, 0.001,          0.2         );
    fit->SetParLimits(4, 0.2*nPOI,      2.0*nPOI     );
    parMass=2;
    parSigma=3;
    fitResult = fHistSignal->Fit(fit,"RNI0Q");
    break;

  case kGaus:
    fit = new TF1("fitGaus",&fct,&PairAnalysisSignalFunc::PeakFunGaus,fFitMin,fFitMax,3);
    //fit = new TF1("fitGaus","gaus",fFitMin,fFitMax);
    fit->SetParNames("N","meanx","sigma");
    fit->SetParameters(1.3*nPOI, massPOI, 0.025);
    fit->SetParLimits(0, 0.2*nPOI,      2.0*nPOI     );
    fit->SetParLimits(1, massPOI-0.02, massPOI+0.02);
    fit->SetParLimits(2, 0.001,         1.           );
    parMass=1;
    parSigma=2;
    fitResult = fHistSignal->Fit(fit,"RNI0Q");
    break;

  }
  // warning in case of fit issues
  if(fitResult!=0)   Warning("DescripePeakShape","fit has error/issue (%d)",fitResult);

  // store chi2/ndf of the fit
  if(fit) fValues(6) = (fit->GetNDF() ? fit->GetChisquare()/fit->GetNDF() : -1.);

  // overwrite values and errors if requested
  if(replaceValErr) {
    switch(method) {
    case kBinCounting:
      fValues(0) = fHistSignal->IntegralAndError(fHistSignal->FindBin(fIntMin), fHistSignal->FindBin(fIntMax), fErrors(0));
      SetSignificanceAndSOB();
      break;
    case kMCScaledMax:
    case kMCScaledInt:
      fValues(0) = mcShape->IntegralAndError(mcShape->FindBin(fIntMin), mcShape->FindBin(fIntMax), fErrors(0));
      SetSignificanceAndSOB();
      break;

    case kMCFitted:
    case kCrystalBall:
    case kGaus:
      fValues(0) = fit->Integral(fIntMin, fIntMax)/binWidth;
      fErrors(0) = fit->IntegralError(fIntMin, fIntMax)/binWidth;
      SetSignificanceAndSOB();
      break;
    }

    // set mass position and width
    if(parMass>=0) {
      fValues(4) = fit->GetParameter(parMass);
      fErrors(4) = fit->GetParError(parMass);
    }
    if(parSigma>=0) {
      fValues(5) = fit->GetParameter(parSigma);
      fErrors(5) = fit->GetParError(parSigma);
    }
    else {
      // calculate FWHM
      SetFWHM();
    }
  }

  // set the peak method obj
  switch(method) {
  case kBinCounting:
    if(replaceValErr) fgPeakShape=(TH1F*)fHistSignal->Clone("BinCount");
    //    delete fct;
    return (TH1F*)fHistSignal->Clone("BinCountReturn");
    break;
  case kMCScaledMax:
  case kMCScaledInt:
    if(replaceValErr) fgPeakShape=mcShape;
    //    delete fct;
    return mcShape;
    break;
  case kMCFitted:
  case kCrystalBall:
  case kGaus:
    if(fgHistSimPM) fit->SetName(Form("mcShapeFunc-%s",fgHistSimPM->GetName()));
    if(replaceValErr) fgPeakShape=fit;
    //    delete fct;
    return fit;
    break;
  }

  // printf("true integration range: %f %f \n",
  //  	 fHistSignal->GetBinLowEdge(fHistSignal->FindBin(fIntMin)),
  // 	 fHistSignal->GetBinLowEdge(fHistSignal->FindBin(fIntMax))+fHistSignal->GetBinWidth(fHistSignal->FindBin(fIntMax)));
  //  delete fct;
  if(replaceValErr && fgPeakShape->IsA()==TF1::Class()) fPeakIsTF1=kTRUE;
  return fgPeakShape;

}
