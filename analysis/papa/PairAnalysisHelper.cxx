///////////////////////////////////////////////////////////////////////////
//                                                                       //
// helper functions wrapped in a namespace.
//
//
// Authors: 
//  * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
//  Julian Book <Julian.Book@cern.ch>
//
//                                                                       //
///////////////////////////////////////////////////////////////////////////


#include <TError.h>
#include <TPad.h>
#include <TMath.h>
#include <TObjString.h>
#include <TObjArray.h>
#include <TVectorD.h>
#include <TH1.h>
#include <TF1.h>
#include <TRandom.h>
#include <TProfile.h>
#include <TFormula.h>
#include <TDatabasePDG.h>
#include <TParticlePDG.h>
#include <TMCProcess.h>
#include <THashList.h>
#include <TGraph.h>

#include <CbmDetectorList.h>

#include "PairAnalysisVarManager.h"
#include "PairAnalysisHelper.h"

//_____________________________________________________________________________
TVectorD* PairAnalysisHelper::MakeLogBinning(Int_t nbinsX, Double_t xmin, Double_t xmax)
{
  //
  // Make logarithmic binning
  // the user has to delete the array afterwards!!!
  //
  
  //check limits
  if (xmin<1e-20 || xmax<1e-20){
    Error("PairAnalysisHelper::MakeLogBinning","For Log binning xmin and xmax must be > 1e-20. Using linear binning instead!");
    return PairAnalysisHelper::MakeLinBinning(nbinsX, xmin, xmax);
  }
  if (xmax<xmin){
    Double_t tmp=xmin;
    xmin=xmax;
    xmax=tmp;
  }
  TVectorD *binLim=new TVectorD(nbinsX+1);
  Double_t first=xmin;
  Double_t last=xmax;
  Double_t expMax=TMath::Log(last/first);
  for (Int_t i=0; i<nbinsX+1; ++i){
    (*binLim)[i]=first*TMath::Exp(expMax/nbinsX*(Double_t)i);
  }
  return binLim;
}

//_____________________________________________________________________________
TVectorD* PairAnalysisHelper::MakeLinBinning(Int_t nbinsX, Double_t xmin, Double_t xmax)
{
  //
  // Make linear binning
  // the user has to delete the array afterwards!!!
  //
  if (xmax<xmin){
    Double_t tmp=xmin;
    xmin=xmax;
    xmax=tmp;
  }
  TVectorD *binLim=new TVectorD(nbinsX+1);
  Double_t first=xmin;
  Double_t last=xmax;
  Double_t binWidth=(last-first)/nbinsX;
  for (Int_t i=0; i<nbinsX+1; ++i){
    (*binLim)[i]=first+binWidth*(Double_t)i;
  }
  return binLim;
}

//_____________________________________________________________________________
TVectorD* PairAnalysisHelper::MakeArbitraryBinning(const char* bins)
{
  //
  // Make arbitrary binning, bins separated by a ','
  //
  TString limits(bins);
  if (limits.IsNull()){
    Error("PairAnalysisHelper::MakeArbitraryBinning","Bin Limit string is empty, cannot add the variable");
    return 0x0;
  }
  
  TObjArray *arr=limits.Tokenize(",");
  Int_t nLimits=arr->GetEntries();
  if (nLimits<2){
    Error("PairAnalysisHelper::MakeArbitraryBinning","Need at leas 2 bin limits, cannot add the variable");
    delete arr;
    return 0x0;
  }
  
  TVectorD *binLimits=new TVectorD(nLimits);
  for (Int_t iLim=0; iLim<nLimits; ++iLim){
    (*binLimits)[iLim]=(static_cast<TObjString*>(arr->At(iLim)))->GetString().Atof();
  }
  
  delete arr;
  return binLimits;
}

//_____________________________________________________________________________
TVectorD* PairAnalysisHelper::MakeGausBinning(Int_t nbinsX, Double_t mean, Double_t sigma)
{
  //
  // Make gaussian binning
  // the user has to delete the array afterwards!!!
  //

  TVectorD *binLim=new TVectorD(nbinsX+1);

  TF1 g("g","gaus",mean-5*sigma,mean+5*sigma);
  g.SetParameters(1,mean,sigma);
  Double_t sum=g.Integral(mean-5*sigma,mean+5*sigma);
  //printf("full integral gaussian: %f \n",sum);

  TF1 g2("g2","gaus(0)/[3]",mean-5*sigma,mean+5*sigma);
  g2.SetParameters(1,mean,sigma,sum); /// normalize with sum

  //  Double_t *params=g2.GetParameters();

  Double_t epsilon=sigma/10000;  // step size
  Double_t xt=mean-5*sigma;      // bin limit
  Double_t pint=0.0;             // previous integral 

  Int_t bin=0;                   // current entry
  Double_t lim=epsilon;          // requested integral values (start,..., end values)

  // calculate intergral until you found all limits
  while( (xt+=epsilon)<=mean+5*sigma) {

    // current integral
    //    Double_t cint = g2.Integral(mean-5*sigma,xt,params,epsilon); //fast, but NOT root 6 (Integral(min,max,epsilon)) compatible
    Double_t cint = g2.Integral(mean-5*sigma,xt);//,params,epsilon);   //slow
    //    printf(" integral to %f:  %f , search limit: %f \n",xt,cint,lim);

    /// condition for bin limit
    if(cint>=lim && pint<lim) {
      //      printf(" %d bin found for %f with requested integral %f, actual integral is %f \n",bin,xt,lim,cint);
      /// set value to vector
      (*binLim)[bin]=xt;

      /// next bin and limit
      bin++;
      lim=bin*(1./nbinsX);
      /// fix end integral value
      if(bin==nbinsX) lim=1.-epsilon;
    }

    /// store prevoius integral value
    pint=cint;
  }

  //  binLim->Print();
  return binLim;

}

//_____________________________________________________________________________
TArrayD *PairAnalysisHelper::MakeStatBinLimits(TH1* h, Double_t stat)
{
  //
  // get bin limits for stat. error less than 'stat'
  //
  if(!h || stat>1.) return 0x0;

  Double_t cont = 0.0;
  Double_t err  = 0.0;
  Double_t from = h->GetBinLowEdge(1);
  Double_t to   = 0.0;

  TArrayD  *vBins  = new TArrayD(1+1);
  vBins->AddAt(from, 0);
  Int_t     vEle   = 1;

  for(Int_t i=1; i<=h->GetNbinsX(); i++) {
    if(h->GetBinContent(i)==0.0 && h->GetBinError(i)==0.) continue;

    to=h->GetBinLowEdge(i+1);
    cont+=h->GetBinContent(i);
    err+=(h->GetBinError(i)*h->GetBinError(i));
    vBins->AddAt(to, vEle);

    //    Printf("cont %f err %f(%f) sum of -> rel err %f%% (current: %f%%)",
    //       h->GetBinContent(i), h->GetBinError(i), TMath::Sqrt(h->GetBinContent(i)),h->GetBinError(i)/h->GetBinContent(i)*100, TMath::Sqrt(err)/cont*100);

    // check for new bin                                                                                                                                                                                                                                            
    if(TMath::Sqrt(err)/cont <= stat) {
      //Printf("bin from %f to %f with err %f%%",from,to,TMath::Sqrt(err)/cont*100);
      err=0.0;
      cont=0.0;
      vEle++;
      from=to;
      vBins->Set(vEle+1);
    }

  }

  vBins->AddAt(h->GetXaxis()->GetXmax(), vBins->GetSize()-1);

  //for(Int_t i=0;i<vBins->GetSize();i++)  Printf("%d %f",i,vBins->At(i));
  return vBins;
}

//_____________________________________________________________________________
TVectorD* PairAnalysisHelper::MakePdgBinning()
{
  //
  // Make arbitrary binning using defined PDG codes
  //

  // array of pdgcodes stored in TDatabasePDG
  TDatabasePDG *pdg = new TDatabasePDG();//::Instance();
  pdg->ReadPDGTable();
  TIter next(pdg->ParticleList() );
  TGraph gr;
  TParticlePDG *p;
  Int_t i=0;
  while ((p = (TParticlePDG *)next())) {
    if(TMath::Abs(p->PdgCode()) < 1e+6) {
      // printf("%s -> %d \n",p->GetName(),p->PdgCode());
      gr.SetPoint(i++, p->PdgCode(),1.);
    }
  }
  gr.Sort();
  TVectorD *vec = new TVectorD(gr.GetN(), gr.GetX());
  //  vec->Print();
  delete pdg;
  return vec;

}

//_____________________________________________________________________________
Double_t PairAnalysisHelper::EvalFormula(TFormula *form, const Double_t *values)
{
  //
  // evaluate return value for formula with current parameter values
  //
  Double_t params[10] = {0.};
  //put parameter values into array using variables stored as the parameters
  for(Int_t ip=0; ip<form->GetNpar(); ip++) params[ip]=values[(UInt_t)form->GetParameter(ip)];
  return (form->EvalPar(0x0,params));
}

//_____________________________________________________________________________
TString PairAnalysisHelper::GetFormulaTitle(TFormula *form)
{
  //
  // evaluate the formula in a readable way (for axis etc)
  //
  // TODO: add units, switch to TMathText, get ride of obsolete parentheses
  TString tform(form->GetExpFormula());
  // TMathText
  //  tform.ReplaceAll("*","\\cdot");   // multiplication sign
  //  tform.ReplaceAll("TMath::","\\"); // get ride of TMath::
  // TLatex
  tform.ReplaceAll("*","#upoint");   // multiplication sign
  //  tform.ReplaceAll("Sqrt","sqrt"); // sqrt sign
  tform.ReplaceAll("TMath::",""); // get ride of TMath::
  tform.ReplaceAll("()","");        // remove function parenthesis
  tform.ToLower();                  // lower cases (e.g. Cos -> cos)
  // replace parameter variables with proper labels
  for(Int_t j=0;j<form->GetNpar();j++)
    tform.ReplaceAll(Form("[%d]",j),
		     PairAnalysisVarManager::GetValueLabel((UInt_t)form->GetParameter(j))
		     );
  return (tform);
}

//_____________________________________________________________________________
TString PairAnalysisHelper::GetFormulaName(TFormula *form)
{
  //
  // build formula key with parameter names
  //
  TString tform("f(");
  for(Int_t j=0;j<form->GetNpar();j++) {
    tform+=PairAnalysisVarManager::GetValueName((UInt_t)form->GetParameter(j));
    if(j!=form->GetNpar()-1) tform+=",";
  }
  tform+=")";
  return (tform);
}

//_____________________________________________________________________________
TFormula *PairAnalysisHelper::GetFormula(const char *name, const char* formula)
{
  //
  // build a TFormula object
  //
  TString check(formula);
  if(check.IsNull()) return 0x0;
  TFormula *form = new TFormula(name,formula);
  // compile function
  if(form->Compile()) return 0x0;
  //set parameter/variable identifier
  for(Int_t i=0; i<form->GetNpar(); i++) {
    form->SetParName(  i, PairAnalysisVarManager::GetValueName(form->GetParameter(i)) );
    //    fUsedVars->SetBitNumber((Int_t)form->GetParameter(i),kTRUE);
  }
  return form;

}

//_____________________________________________________________________________
void PairAnalysisHelper::SetPDGBinLabels( TH1 *hist) {
  //
  // build formula key with parameter names
  //
  TDatabasePDG *pdg = TDatabasePDG::Instance();
  TAxis *xaxis = hist->GetXaxis();
  for(Int_t i=1; i<hist->GetNbinsX()+1; i++) {
    // printf("bin %d: low edge: %.0f --> %s \n",i,xaxis->GetBinLowEdge(i), pdg->GetParticle(xaxis->GetBinLowEdge(i))->GetName());
    xaxis->SetBinLabel(i,pdg->GetParticle((Int_t)xaxis->GetBinLowEdge(i))->GetName());
  }
}

//_____________________________________________________________________________
TString PairAnalysisHelper::GetPDGlabel(Int_t pdg)
{
  //
  // return the label in latex format corresponding to pdg code
  //

  TString name=TDatabasePDG::Instance()->GetParticle(pdg)->GetName();
  name.ReplaceAll("dd_1_bar","primary");
  name.ReplaceAll("proton","p");
  // correct greek letters
  if(name.Contains("delta",TString::kIgnoreCase) ||
     name.Contains("sigma",TString::kIgnoreCase) ||
     name.Contains("xi",TString::kIgnoreCase) ||
     name.Contains("lambda",TString::kIgnoreCase) ||
     name.Contains("omega",TString::kIgnoreCase) ||
     name.Contains("eta",TString::kIgnoreCase) ||
     name.Contains("tau",TString::kIgnoreCase) ||
     name.Contains("phi",TString::kIgnoreCase) ||
     name.Contains("eta",TString::kIgnoreCase) ||
     name.Contains("upsilon",TString::kIgnoreCase) ||
     name.Contains("pi",TString::kIgnoreCase) ||
     name.Contains("rho",TString::kIgnoreCase) ) name.Prepend("#");
  // correct anti particles
  if(name.Contains("_bar")) { name.ReplaceAll("_bar","}"); name.Prepend("#bar{"); }
  if(name.Contains("anti")) { name.ReplaceAll("anti","#bar{"); name.Append("}"); }
    // correct indices
  name.ReplaceAll("+","^{+}");       name.ReplaceAll("-","^{-}");         name.ReplaceAll("0","^{0}");
  name.ReplaceAll("_s","_{s}");       name.ReplaceAll("_c","_{c}");       name.ReplaceAll("_b","_{b}");
  name.ReplaceAll("_1","_{1}");
  // specials
  name.ReplaceAll("/psi","/#psi");
  //  Printf(" %d = %s",pdg,name.Data());
  return name;
}

//_____________________________________________________________________________
void PairAnalysisHelper::SetGEANTBinLabels( TH1 *hist) {
  //
  // build formula key with parameter names
  //
  TAxis *xaxis = hist->GetXaxis();
  for(Int_t i=1; i<hist->GetNbinsX()+1; i++) {
    xaxis->SetBinLabel(i,TMCProcessName[i-1]);
  }
  xaxis->LabelsOption("v"); // vertical labels
  if(gPad) {
    xaxis->SetTitleOffset(3.6);
    gPad->SetTopMargin(0.01);
    gPad->SetBottomMargin(0.4);
  }
}


//_____________________________________________________________________________
TString PairAnalysisHelper::GetDetName(DetectorId det)
{
  //
  // get detector name
  //
  TString name="";
  CbmDetectorList::GetSystemNameCaps(det, name);
  return (name);
}

//_____________________________________________________________________________
Double_t PairAnalysisHelper::GetContentMinimum(TH1 *h, Bool_t inclErr) {
  //
  // get minimum bin content of histogram (having entries)
  //
  Int_t bin, binx, biny, binz;
  Int_t xfirst  = h->GetXaxis()->GetFirst();
  Int_t xlast   = h->GetXaxis()->GetLast();
  Int_t yfirst  = h->GetYaxis()->GetFirst();
  Int_t ylast   = h->GetYaxis()->GetLast();
  Int_t zfirst  = h->GetZaxis()->GetFirst();
  Int_t zlast   = h->GetZaxis()->GetLast();
  Double_t minimum = FLT_MAX, value=0., error=0.;
  for (binz=zfirst;binz<=zlast;binz++) {
    for (biny=yfirst;biny<=ylast;biny++) {
      for (binx=xfirst;binx<=xlast;binx++) {
	bin = h->GetBin(binx,biny,binz);
	value = h->GetBinContent(bin);
	error = h->GetBinError(bin);
	//	Printf(" \t hist%s bin%d value%f error%f \n",h->GetTitle(),bin,value,error);
	if(gPad->GetLogy() && (value-error)<= 0.) continue;
	if(error>value*0.9) continue;
	if(inclErr) value -= h->GetBinError(bin);
	if (value < minimum && TMath::Abs(h->GetBinError(bin)-1.e-15) > 1.e-15) {
	  minimum = value;
	}
      }
    }
  }
  //  Printf(" RETURN VALUE: hist%s %f \n",h->GetTitle(),minimum);
  return minimum;

}

Double_t PairAnalysisHelper::GetContentMaximum(TH1 *h, Bool_t inclErr)
{
  //
  // get maximum bin content+error of histogram (having entries)
  //
  Int_t bin, binx, biny, binz;
  Int_t xfirst  = h->GetXaxis()->GetFirst();
  Int_t xlast   = h->GetXaxis()->GetLast();
  Int_t yfirst  = h->GetYaxis()->GetFirst();
  Int_t ylast   = h->GetYaxis()->GetLast();
  Int_t zfirst  = h->GetZaxis()->GetFirst();
  Int_t zlast   = h->GetZaxis()->GetLast();
  Double_t maximum = -1.*FLT_MAX, value=0., error=0.;
  for (binz=zfirst;binz<=zlast;binz++) {
    for (biny=yfirst;biny<=ylast;biny++) {
      for (binx=xfirst;binx<=xlast;binx++) {
	bin = h->GetBin(binx,biny,binz);
	value = h->GetBinContent(bin);
	error = h->GetBinError(bin);
	if(inclErr) value += h->GetBinError(bin);
	if (value > maximum && TMath::Abs(error-1.e-15) > 1.e-15) {
	  maximum = value;
	}
      }
    }
  }
  return maximum;
}

Double_t PairAnalysisHelper::GetQuantile(TH1* h1, Double_t p/*=0.5*/) {
  //
  // calculates the quantile of the bin contents, p=0.5 -> Median
  // useful functionallity for plotting 2D distibutions with some extreme outliers
  //
  if(p<0.0 || p>1.) return -1.;
  Int_t nbinsX = h1->GetNbinsX();
  Int_t nbinsY = h1->GetNbinsY();
  Int_t nbinsZ = h1->GetNbinsZ();
  Int_t nbins = (nbinsX*(nbinsY?nbinsY:1)*(nbinsZ?nbinsZ:1));
  Int_t xbin=-1;
  Int_t ybin=-1;
  Int_t zbin=-1;
  Double_t val[nbins];
  Int_t idx[nbins];
  Int_t nfilled = 0;
  for(Int_t i=1; i<=nbins; i++) {
    h1->GetBinXYZ(i,xbin,ybin,zbin);
    if(xbin<h1->GetXaxis()->GetFirst() || xbin>h1->GetXaxis()->GetLast()) continue;
    if(ybin<h1->GetYaxis()->GetFirst() || ybin>h1->GetYaxis()->GetLast()) continue;
    Double_t con = h1->GetBinContent(i);
    Double_t err = h1->GetBinError(i);
    if(err!=0.0) {
      //      printf("bin%d %.f+-%.f \n",i,con,err);
      val[nfilled]=con+(h1->GetDimension()<2 ? err : 0.0); // w or w/o err?
      nfilled++;
    }
  }
  if(nfilled==0) return -1.;
  TMath::Sort(nfilled,val,idx,kFALSE); // kFALSE=increasing numbers
  Int_t pos = (Int_t) ((Double_t)nfilled*p);
  //for(int i=0; i<nfilled; i++) cout << i << " " << idx[i] << " " << val[idx[i]] << endl;
  //printf("nfilled %d quantile %f pos %d: %f \n",nfilled,p,pos,val[idx[pos]]);
  return val[idx[pos]];
}


TObject* PairAnalysisHelper::FindObjectByTitle(TObjArray *arrhist, TString ref)
{
  //
  // shortcut to find a certain pair type object in array
  //
  for(Int_t i=0; i<arrhist->GetEntriesFast(); i++) {
    if( !ref.CompareTo(arrhist->UncheckedAt(i)->GetTitle()) ) {
      return arrhist->UncheckedAt(i);
    }
  }
  return 0x0;

  //  return ( arrhist->FindObject(Form("Pair.%s",PairAnalysis::PairClassName(type))) );
  //  TString ref=Form("Pair.%s",PairAnalysis::PairClassName(type));

}

