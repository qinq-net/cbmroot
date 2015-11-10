/*************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 **************************************************************************/

///////////////////////////////////////////////////////////////////////////
//
// Generic Histogram container with support for groups and filling of groups
// by passing a vector of data
//
// Authors:
//   Julian Book   <Julian.Book@cern.ch>
/*

  TOADD: reserved words, MC signals

  Histograms such as THxF, TProfile(2D,3D) and n-dimensonal objects such as
  (THn, THnSparse) can be added via the various functions:
  - UserHistogram
  - UserProfile + different error option (Mean,Rms,.. see: TProfile::BuildOptions(..))
  - UserSparse
  In addition histograms can be filled with weights.

  Different kind of binnings can be passed (linear, arbitrary, logarithmic)
  either via the PairAnalysisHelper functionalities (recommended):
  - PairAnalysisHelper::MakeLinearBinning(nbins,low,high)
  - PairAnalysisHelper::MakeLogBinning(nbins,low,high)
  - PairAnalysisHelper::MakeArbitraryBinning("1.,4.,10.,..")
  or via one of the provided functions and switchers therein.

  The 'name' and 'title;titleX;titleY;...' arguments of the objects are
  recommended to be left free (i.e. ""), they are only kept for backward
  compatibility. Then all axis labels, units and names and object names
  are set in a consistent way and it is ensured that they are unique.

  Variables are added via the enumerator of PairAnalysisVarManager. Any
  computation of variables (max=10) are implemented according to TFormula
  using the TMath functions and the passage of an array of variables, e.g.:

  UInt_t vars[]={ PairAnalysisVarManager::kRefMult, PairAnalysisVarManager::kRefMultTPConly };
  histos->UserHistogram("Event","test","", PairAnalysisHelper::MakeLinBinning(100,1.e-1,1.e+1),
  "[0]*[1]/TMath::Pi()+TMath::Cos([1])", varEpm);



*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include <typeinfo>

#include <TH1.h>
#include <TH1F.h>
#include <TH1D.h>
#include <TH2.h>
#include <TH3.h>
#include <THnBase.h>
#include <THn.h>
#include <THnSparse.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TProfile3D.h>
#include <TCollection.h>
#include <THashList.h>
#include <TString.h>
#include <TObjString.h>
#include <TObjArray.h>
#include <TFile.h>
#include <TError.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TROOT.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TLatex.h>
#include <TKey.h>
#include <TAxis.h>
#include <TGaxis.h>
#include <TVirtualPS.h>
#include <TVectorD.h>
#include <TFormula.h>

#include "PairAnalysis.h"
#include "PairAnalysisHelper.h"
#include "PairAnalysisSignalMC.h"
#include "PairAnalysisStyler.h"
#include "PairAnalysisVarManager.h"
#include "PairAnalysisHistos.h"
#include "PairAnalysisMetaData.h"

ClassImp(PairAnalysisHistos)


PairAnalysisHistos::PairAnalysisHistos() :
//   TCollection(),
TNamed("PairAnalysisHistos","PairAnalysis Histogram Container"),
  fHistoList(),
  fMetaData(0x0),
  fList(0x0),
  fUsedVars(new TBits(PairAnalysisVarManager::kNMaxValues)),
  fReservedWords(new TString),
  fPrecision(kFloat)
{
  //
  // Default constructor
  //
  fHistoList.SetOwner(kTRUE);
  fHistoList.SetName("PairAnalysis_Histos");
  PairAnalysisVarManager::InitFormulas();
  PairAnalysisStyler::LoadStyle();
}

//_____________________________________________________________________________
PairAnalysisHistos::PairAnalysisHistos(const char* name, const char* title) :
  //   TCollection(),
  TNamed(name, title),
  fHistoList(),
  fMetaData(0x0),
  fList(0x0),
  fUsedVars(new TBits(PairAnalysisVarManager::kNMaxValues)),
  fReservedWords(new TString),
  fPrecision(kFloat)
{
  //
  // TNamed constructor
  //
  fHistoList.SetOwner(kTRUE);
  fHistoList.SetName(name);
  PairAnalysisVarManager::InitFormulas();
  PairAnalysisStyler::LoadStyle();
}

//_____________________________________________________________________________
PairAnalysisHistos::~PairAnalysisHistos()
{
  //
  // Destructor
  //
  fHistoList.Clear();
  if (fUsedVars) delete fUsedVars;
  if (fList) fList->Clear();
  if (fMetaData) delete fMetaData;
  delete fReservedWords;
}

//_____________________________________________________________________________
void PairAnalysisHistos::UserHistogram(const char* histClass, Int_t ndim, TObjArray *limits, UInt_t *vars, UInt_t valTypeW)
{
  //
  // Histogram creation n>3 dimension only with non-linear binning
  //

  Bool_t isOk=kTRUE;
  isOk&=(ndim<21 && ndim>3);
  if(!isOk) { Warning("UserHistogram","Array sizes should be between 3 and 20. Not adding Histogram to '%s'.", histClass); return; }
  isOk&=(ndim==limits->GetEntriesFast());
  if(!isOk) return;

  // set automatic histo name
  TString name;
  for(Int_t iv=0; iv < ndim; iv++)
    name+=Form("%s_",PairAnalysisVarManager::GetValueName(vars[iv]));
  name.Resize(name.Length()-1);

  isOk&=IsHistogramOk(histClass,name);

  THnD *hist;
  Int_t bins[ndim];
  if (isOk) {
    // get number of bins
    for(Int_t idim=0 ;idim<ndim; idim++) {
      TVectorD *vec = (TVectorD*) limits->At(idim);
      bins[idim]=vec->GetNrows()-1;
    }

    hist=new THnD(name.Data(),"", ndim, bins, 0x0, 0x0);

    // set binning
    for(Int_t idim=0 ;idim<ndim; idim++) {
      TVectorD *vec = (TVectorD*) limits->At(idim);
      hist->SetBinEdges(idim,vec->GetMatrixArray());
    }

    // store variales in axes
    StoreVariables(hist, vars);
    hist->SetUniqueID(valTypeW); // store weighting variable

    // store which variables are used
    for(Int_t i=0; i<ndim; i++)   fUsedVars->SetBitNumber(vars[i],kTRUE);
    fUsedVars->SetBitNumber(valTypeW,kTRUE);

    Bool_t isReserved=fReservedWords->Contains(histClass);
    if (isReserved)
      UserHistogramReservedWords(histClass, hist);
    else
      UserHistogram(histClass, hist);

  }
}

//_____________________________________________________________________________
void PairAnalysisHistos::AddSparse(const char* histClass, Int_t ndim, TObjArray *limits, UInt_t *vars, UInt_t valTypeW)
{
  //
  // THnSparse creation with non-linear binning
  //

  Bool_t isOk=kTRUE;
  isOk&=(ndim==limits->GetEntriesFast());
  if(!isOk) return;

  // set automatic histo name
  TString name;
  for(Int_t iv=0; iv < ndim; iv++)
    name+=Form("%s_",PairAnalysisVarManager::GetValueName(vars[iv]));
  name.Resize(name.Length()-1);

  isOk&=IsHistogramOk(histClass,name);

  THnSparseD *hist;
  Int_t bins[ndim];
  if (isOk) {
    // get number of bins
    for(Int_t idim=0 ;idim<ndim; idim++) {
      TVectorD *vec = (TVectorD*) limits->At(idim);
      bins[idim]=vec->GetNrows()-1;
    }

    hist=new THnSparseD(name.Data(),"", ndim, bins, 0x0, 0x0);

    // set binning
    for(Int_t idim=0 ;idim<ndim; idim++) {
      TVectorD *vec = (TVectorD*) limits->At(idim);
      hist->SetBinEdges(idim,vec->GetMatrixArray());
    }

    // store variales in axes
    StoreVariables(hist, vars);
    hist->SetUniqueID(valTypeW); // store weighting variable

    // store which variables are used
    for(Int_t i=0; i<ndim; i++)   fUsedVars->SetBitNumber(vars[i],kTRUE);
    fUsedVars->SetBitNumber(valTypeW,kTRUE);

    Bool_t isReserved=fReservedWords->Contains(histClass);
    if (isReserved)
      UserHistogramReservedWords(histClass, hist);
    else
      UserHistogram(histClass, hist);

  }
}

//_____________________________________________________________________________
TString PairAnalysisHistos::UserHistogram(const char* histClass, TObject* hist)
{
  //
  // Add any type of user histogram
  //

  //special case for the calss Pair. where histograms will be created for all pair classes
  Bool_t isReserved=fReservedWords->Contains(histClass);
  if (isReserved) {
    UserHistogramReservedWords(histClass, hist);
    return hist->GetName();
  }

  // if (!IsHistogramOk(histClass,hist->GetName())) return hist->GetName();
  // THashList *classTable=(THashList*)fHistoList.FindObject(histClass);
  //  //  hist->SetDirectory(0);

  // store variables axis
  UInt_t valType[20] = {0};
  // extract variables from axis
  FillVarArray(hist, valType);
  // change to mc truth variables if available
  TString hclass=histClass;
  if(hclass.Contains("MCtruth")) {
    for(Int_t i=0;i<2;i++) {
      //      Printf("SWITCH TO MC: before: %d %s ---->",valType[i],PairAnalysisVarManager::GetValueName(valType[i]));
      valType[i] = PairAnalysisVarManager::GetValueTypeMC(valType[i]);
      // if theres no corresponding MCtruth variable, skip adding this histogram
      //      if(valType[i] < PairAnalysisVarManager::kNMaxValues && valType[i]>0) return hist->GetName();
      if(valType[i] < PairAnalysisVarManager::kPairMax && valType[i]>0) return hist->GetName();
      // request filling of mc variable
      fUsedVars->SetBitNumber(valType[i],kTRUE);
      //      Printf("after: %d %s",valType[i],PairAnalysisVarManager::GetValueName(valType[i]));
    }
    StoreVariables(hist, valType);
    hist->SetUniqueID(valType[19]); // store weighting variable
    // check for formulas
    if(hist->InheritsFrom(TH1::Class())) {
      TIter next(((TH1*)hist)->GetListOfFunctions());
      TFormula *f=0;
      while ( (f=dynamic_cast<TFormula*>(next()) ) ) {
	for(Int_t i=0; i<f->GetNpar(); i++) {
	  Int_t parMC=PairAnalysisVarManager::GetValueTypeMC(f->GetParameter(i));
	  // if theres none corresponding MCtruth variable, skip adding this histogram
	  if(parMC < PairAnalysisVarManager::kNMaxValues) return hist->GetName();
	  f->SetParameter(   i, parMC );
	  f->SetParName(     i, PairAnalysisVarManager::GetValueName(parMC)   );
	  fUsedVars->SetBitNumber(parMC,kTRUE);
	}
      }
      // change histogram key according to mctruth information
      //    Printf("SWITCH TO MC NAME: before: %s ---->",hist->GetName());
      AdaptNameTitle((TH1*)hist, histClass);
      //    Printf("after:  %s",hist->GetName());
    }
  }
  else {
    StoreVariables(hist, valType);
    hist->SetUniqueID(valType[19]); // store weighting variable
  }

  // add histogram to class
  if (!IsHistogramOk(histClass,hist->GetName())) return hist->GetName();
  THashList *classTable=(THashList*)fHistoList.FindObject(histClass);
  //  hist->SetDirectory(0);
  if(classTable) classTable->Add(hist);
  return hist->GetName();
}

//_____________________________________________________________________________
TH1 *PairAnalysisHistos::GetTHist(const char* histClass, const char *name, const char* title,
				const TVectorD * const binsX, const TVectorD * const binsY, const TVectorD * const binsZ
				)
{
  //
  // retrieve n-dimensional Hist depending on arguments
  //
  Bool_t isOk=kTRUE;
  isOk&=IsHistogramOk(histClass,name);
  isOk&=(binsX!=0x0);
  if (!isOk) return 0x0;
  switch(fPrecision) {
  case kFloat:
    if(!binsY)      return (new TH1F(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray()) );
    else if(!binsZ) return (new TH2F(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray(),
				     binsY->GetNrows()-1,binsY->GetMatrixArray())            );
    else            return (new TH3F(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray(),
				     binsY->GetNrows()-1,binsY->GetMatrixArray(),
				     binsZ->GetNrows()-1,binsZ->GetMatrixArray())            );
    break;
  case kDouble:
    if(!binsY)      return (new TH1D(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray()) );
    else if(!binsZ) return (new TH2D(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray(),
				     binsY->GetNrows()-1,binsY->GetMatrixArray())            );
    else            return (new TH3D(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray(),
				     binsY->GetNrows()-1,binsY->GetMatrixArray(),
				     binsZ->GetNrows()-1,binsZ->GetMatrixArray())            );
    break;
  }
}

//_____________________________________________________________________________
TH1 *PairAnalysisHistos::GetTProf(const char* histClass, const char *name, const char* title,
				const TVectorD * const binsX, const TVectorD * const binsY, const TVectorD * const binsZ,
				TString option)
{
  //
  // retrieve n-dimensional profile histogram with error options depending on arguments
  //
  Bool_t isOk=kTRUE;
  isOk&=IsHistogramOk(histClass,name);
  isOk&=(binsX!=0x0);
  if (!isOk) return 0x0;
  // parse error option
  TString opt=""; Double_t pmin=0., pmax=0.;
  if(!option.IsNull()) {
    TObjArray *arr=option.Tokenize(";");
    arr->SetOwner();
    opt=((TObjString*)arr->At(0))->GetString();
    if(arr->GetEntriesFast()>1) pmin=(((TObjString*)arr->At(1))->GetString()).Atof();
    if(arr->GetEntriesFast()>2) pmax=(((TObjString*)arr->At(2))->GetString()).Atof();
    delete arr;
  }
  // build profile with error options and return it
  TH1 *prof=0x0;
  if(!binsY)      return (new TProfile(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray(),pmin,pmax,opt.Data()) );
  else if(!binsZ) {
    prof = new TProfile2D(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray(),
			  binsY->GetNrows()-1,binsY->GetMatrixArray());
    ((TProfile2D*)prof)->BuildOptions(pmin,pmax,opt.Data());
    return prof;
  }
  else            {
    prof = new TProfile3D(name,title,binsX->GetNrows()-1,binsX->GetMatrixArray(),
			  binsY->GetNrows()-1,binsY->GetMatrixArray(),
			  binsZ->GetNrows()-1,binsZ->GetMatrixArray());
    ((TProfile3D*)prof)->BuildOptions(pmin,pmax,opt.Data());
    return prof;
  }

}

//_____________________________________________________________________________
TFormula *PairAnalysisHistos::GetFormula(const char *name, const char* formula)
{
  //
  // build a TFormula object
  //
  TFormula *form = new TFormula(name,formula);
  // compile function
  if(form->Compile()) return 0x0;
  //set parameter/variable identifier
  for(Int_t i=0; i<form->GetNpar(); i++) {
    form->SetParName(  i, PairAnalysisVarManager::GetValueName(form->GetParameter(i)) );
    fUsedVars->SetBitNumber((Int_t)form->GetParameter(i),kTRUE);
  }
  return form;

}

//_____________________________________________________________________________
void PairAnalysisHistos::AddClass(const char* histClass)
{
  //
  // Add a class of histograms
  // Several classes can be added by separating them by a ';' e.g. 'class1;class2;class3'
  //
  TString hists(histClass);
  TObjArray *arr=hists.Tokenize(";");
  TIter next(arr);
  TObject *o=0;
  while ( (o=next()) ){
    if (fHistoList.FindObject(o->GetName())){
      Warning("AddClass","Cannot create class '%s' it already exists.",histClass);
      continue;
    }
    if (fReservedWords->Contains(o->GetName())){
      Error("AddClass","Pair is a reserved word, please use another name");
      continue;
    }
    THashList *table=new THashList;
    table->SetOwner(kTRUE);
    table->SetName(o->GetName());
    fHistoList.Add(table);
  }
  delete arr;

}

//_____________________________________________________________________________
void PairAnalysisHistos::FillClass(const char* histClass, const Double_t *values)
{
  //
  // Fill class 'histClass' (by name)
  //

  THashList *classTable=(THashList*)fHistoList.FindObject(histClass);
  if (!classTable){
    //    Warning("FillClass","Cannot fill class '%s' its not defined.",histClass);
    return;
  }

  TIter nextHist(classTable);
  TObject *obj=0;
  while ( (obj=(TObject*)nextHist()) )  FillValues(obj, values);

  return;
}

//_____________________________________________________________________________
void PairAnalysisHistos::UserHistogramReservedWords(const char* histClass, const TObject *hist)
{
  //
  // Creation of histogram for all pair or track types
  //
  TString title(hist->GetTitle());
  // Same Event Like Sign
  TIter nextClass(&fHistoList);
  THashList *l=0;
  while ( (l=static_cast<THashList*>(nextClass())) ){
    TString name(l->GetName());
    if (name.Contains(histClass)){
      TObject *h=hist->Clone();
      // Tobject has no function SetDirectory, didn't we need this???
      //      h->SetDirectory(0);
      ((TH1*)h)->SetTitle(Form("%s %s",title.Data(),l->GetName()));

      UserHistogram(l->GetName(),h);
    }
  }
  delete hist;

  return;
}

//_____________________________________________________________________________
void PairAnalysisHistos::DumpToFile(const char* file)
{
  //
  // Dump the histogram list to a newly created root file
  //
  TFile f(file,"recreate");
  fHistoList.Write(fHistoList.GetName(),TObject::kSingleKey);
  f.Close();
}

//_____________________________________________________________________________
TObject* PairAnalysisHistos::GetHist(const char* histClass, const char* name) const
{
  //
  // return object 'name' in 'histClass'
  //
  THashList *classTable=(THashList*)fHistoList.FindObject(histClass);
  if (!classTable) return 0x0;
  return classTable->FindObject(name);
}

//_____________________________________________________________________________
TH1* PairAnalysisHistos::GetHistogram(const char* histClass, const char* name) const
{
  //
  // return histogram 'name' in 'histClass'
  //
  return ((TH1*) GetHist(histClass, name));
}

//_____________________________________________________________________________
TObject* PairAnalysisHistos::GetHist(const char* cutClass, const char* histClass, const char* name) const
{
  //
  // return object from list of list of histograms
  // this function is thought for retrieving histograms if a list of PairAnalysisHistos is set
  //

  if (!fList) return 0x0;
  THashList *h=dynamic_cast<THashList*>(fList->FindObject(cutClass));
  if (!h)return 0x0;
  THashList *classTable=dynamic_cast<THashList*>(h->FindObject(histClass));
  if (!classTable) return 0x0;
  return classTable->FindObject(name);
}

//_____________________________________________________________________________
TH1* PairAnalysisHistos::GetHistogram(const char* cutClass, const char* histClass, const char* name) const
{
  //
  // return histogram from list of list of histograms
  // this function is thought for retrieving histograms if a list of PairAnalysisHistos is set
  //
  return ((TH1*) GetHist(cutClass, histClass, name));
}

//_____________________________________________________________________________
void PairAnalysisHistos::Draw(const Option_t* option)
{
  //
  // Draw histograms
  //

  TString drawStr(option);
  TObjArray *arr=drawStr.Tokenize(";");
  arr->SetOwner();
  TIter nextOpt(arr);

  TString drawClasses;
  TObjString *ostr=0x0;

  TString currentOpt;
  TString testOpt;
  while ( (ostr=(TObjString*)nextOpt()) ){
    currentOpt=ostr->GetString();
    currentOpt.Remove(TString::kBoth,'\t');
    currentOpt.Remove(TString::kBoth,' ');

    testOpt="classes=";
    if ( currentOpt.Contains(testOpt.Data()) ){
      drawClasses=currentOpt(testOpt.Length(),currentOpt.Length());
    }
  }

  delete arr;
  drawStr.ToLower();
  //optionsfList
  //   Bool_t same=drawOpt.Contains("same"); //FIXME not yet implemented

  TCanvas *c=0x0;
  if (gVirtualPS) {
    if (!gPad){
      Error("Draw","When writing to a file you have to create a canvas before opening the file!!!");
      return;
    }
    c=gPad->GetCanvas();
    c->cd();
    //     c=new TCanvas;
  }

  TIter nextClass(&fHistoList);
  THashList *classTable=0;
  //   Bool_t first=kTRUE;
  while ( (classTable=(THashList*)nextClass()) ){
    //test classes option
    if (!drawClasses.IsNull() && !drawClasses.Contains(classTable->GetName())) continue;
    //optimised division
    Int_t nPads = classTable->GetEntries();
    Int_t nCols = (Int_t)TMath::Ceil( TMath::Sqrt(nPads) );
    Int_t nRows = (Int_t)TMath::Ceil( (Double_t)nPads/(Double_t)nCols );

    //create canvas
    if (!gVirtualPS){
      TString canvasName;
      canvasName.Form("c%s_%s",GetName(),classTable->GetName());
      c=(TCanvas*)gROOT->FindObject(canvasName.Data());
      if (!c) c=new TCanvas(canvasName.Data(),Form("%s: %s",GetName(),classTable->GetName()));
      c->Clear();
    } else {
      //       if (first){
      //         first=kFALSE;
      //         if (nPads>1) gVirtualPS->NewPage();
      //       } else {
      if (nPads>1) c->Clear();
      //       }
    }
    if (nCols>1||nRows>1) c->Divide(nCols,nRows);

    //loop over histograms and draw them
    TIter nextHist(classTable);
    Int_t iPad=0;
    TH1 *h=0;
    while ( (h=(TH1*)nextHist()) ){
      TString drawOpt;
      if ( (h->InheritsFrom(TH2::Class())) ) drawOpt="colz";
      if (nCols>1||nRows>1) c->cd(++iPad);
      if ( TMath::Abs(h->GetXaxis()->GetBinWidth(1)-h->GetXaxis()->GetBinWidth(2))>1e-10 ) gPad->SetLogx();
      if ( TMath::Abs(h->GetYaxis()->GetBinWidth(1)-h->GetYaxis()->GetBinWidth(2))>1e-10 ) gPad->SetLogy();
      if ( TMath::Abs(h->GetZaxis()->GetBinWidth(1)-h->GetZaxis()->GetBinWidth(2))>1e-10 ) gPad->SetLogz();
      TString histOpt=h->GetOption();
      histOpt.ToLower();
      if (histOpt.Contains("logx")) gPad->SetLogx();
      if (histOpt.Contains("logy")) gPad->SetLogy();
      if (histOpt.Contains("logz")) gPad->SetLogz();
      histOpt.ReplaceAll("logx","");
      histOpt.ReplaceAll("logy","");
      histOpt.ReplaceAll("logz","");
      h->Draw(drawOpt.Data());
    }
    if (gVirtualPS) {
      c->Update();
    }

  }
  //   if (gVirtualPS) delete c;
}

//_____________________________________________________________________________
void PairAnalysisHistos::Print(const Option_t* option) const
{
  //
  // Print classes and histograms
  //
  TString optString(option);

  if (optString.IsNull()) PrintStructure();



}

//_____________________________________________________________________________
void PairAnalysisHistos::PrintStructure() const
{
  //
  // Print classes and histograms in the class to stdout
  //
  if (!fList){
    TIter nextClass(&fHistoList);
    THashList *classTable=0;
    while ( (classTable=(THashList*)nextClass()) ){
      TIter nextHist(classTable);
      TObject *o=0;
      Printf("+ %s\n",classTable->GetName());
      while ( (o=nextHist()) )
	Printf("| ->%s\n",o->GetName());
    }
  } else {
    TIter nextCutClass(fList);
    THashList *cutClass=0x0;
    while ( (cutClass=(THashList*)nextCutClass()) ) {
      TString cla=cutClass->GetName();
      if(cla.Contains("QAcuts")) continue;
      Printf("+ %s\n",cutClass->GetName());
      TIter nextClass(cutClass);
      THashList *classTable=0;
      while ( (classTable=(THashList*)nextClass()) ){
	TIter nextHist(classTable);
	TObject *o=0;
	Printf("|  + %s\n",classTable->GetName());
	while ( (o=nextHist()) )
	  Printf("|  | ->%s\n",o->GetName());
      }

    }
  }
}

//_____________________________________________________________________________
void PairAnalysisHistos::SetHistogramList(THashList &list, Bool_t setOwner/*=kTRUE*/)
{
  //
  // set histogram classes and histograms to this instance. It will take onwnership!
  //
  ResetHistogramList();
  TString name(GetName());
  //  if (name == "PairAnalysisHistos") 
  SetName(list.GetName());
  TIter next(&list);
  TObject *o;
  while ( (o=next()) ){
    fHistoList.Add(o);
  }
  if (setOwner){
    list.SetOwner(kFALSE);
    fHistoList.SetOwner(kTRUE);
  } else {
    fHistoList.SetOwner(kFALSE);
  }
}

//_____________________________________________________________________________
Bool_t PairAnalysisHistos::SetCutClass(const char* cutClass)
{
  //
  // Assign histogram list according to cutClass
  //

  if (!fList) return kFALSE;
  ResetHistogramList();
  THashList *h=dynamic_cast<THashList*>(fList->FindObject(cutClass));
  if (!h) {
    Warning("SetCutClass","cutClass '%s' not found", cutClass);
    return kFALSE;
  }
  SetHistogramList(*h,kFALSE);
  return kTRUE;
}
//_____________________________________________________________________________
Bool_t PairAnalysisHistos::IsHistogramOk(const char* histClass, const char* name)
{
  //
  // check whether the histogram class exists and the histogram itself does not exist yet
  //
  Bool_t isReserved=fReservedWords->Contains(histClass);
  if (!fHistoList.FindObject(histClass)&&!isReserved){
    Warning("IsHistogramOk","Cannot create histogram. Class '%s' not defined. Please create it using AddClass before.",histClass);
    return kFALSE;
  }
  if (GetHist(histClass,name)){
    Warning("IsHistogramOk","Cannot create histogram '%s' in class '%s': It already exists!",name,histClass);
    return kFALSE;
  }
  return kTRUE;
}

// //_____________________________________________________________________________
// TIterator* PairAnalysisHistos::MakeIterator(Bool_t dir) const
// {
//   //
//   //
//   //
//   return new TListIter(&fHistoList, dir);
// }

//_____________________________________________________________________________
void PairAnalysisHistos::ReadFromFile(const char* file, const char *task, const char *config)
{
  //
  // Read histos from file
  //
  TFile f(file);
  TIter nextKey(f.GetListOfKeys());
  TKey *key=0;
  while ( (key=(TKey*)nextKey()) ){
    TString name=key->GetName();
    // check for meta data
    if(name.Contains(Form("PairAnalysisMetaData_%s",task))) {
      fMetaData=new PairAnalysisMetaData();
      fMetaData->SetMetaData(*dynamic_cast<TList*>(f.Get(key->GetName())),kFALSE);
    }
    // check for histos
    if(!name.Contains(Form("PairAnalysisHistos_%s",task)))    continue;
    if(!strlen(task) && !name.Contains(task)) continue;
    TObject *o=f.Get(key->GetName());
    TList *list=dynamic_cast<TList*>(o);
    if (!list) continue;
    else fList=list;
    THashList *listCfg=dynamic_cast<THashList*>(list->FindObject(config));
    if (!listCfg) continue;
    SetHistogramList(*listCfg);
    break;
  }
  f.Close();
  // load style
  PairAnalysisStyler::LoadStyle();
}

//_____________________________________________________________________________
void PairAnalysisHistos::DrawTaskSame(TString histName, TString opt, TString histClassDenom, TString taskDenom)
{

  TString optString(opt);
  optString.ToLower();
  Bool_t optEff =optString.Contains("eff");
  //  fList->Print();
  //  fHistoList.Print();
  fHistoList.SetOwner(kFALSE);

  THashList *listDenom = dynamic_cast<THashList*>(fList->FindObject(taskDenom.Data()));
  if(listDenom) opt+="div";

  TIter nextCfg(fList);
  THashList *listCfg=0;
  while ( (listCfg=static_cast<THashList*>(nextCfg())) ){

    TString lname=listCfg->GetName();

    // exclude QAcuts
    if( lname.Contains("QAcuts_") ) continue;

    // skip same cfg if ratio
    if(!optEff && listDenom && lname.EqualTo(taskDenom)) continue;
    //if(listDenom && lname.EqualTo(taskDenom)) continue;

    Info("DrawClassSame"," Task name %s ",lname.Data());

    // update histogram list
    ResetHistogramList();
    TIter next(listCfg);
    Int_t idx=0;
    TObject *o;
    while ( (o=next()) ){
      fHistoList.AddAt(o,idx++);
    }

    // adapt name for legends
    SetName(listCfg->GetName());
    //fHistoList.Print();
    DrawSame(histName,(opt+"task").Data(),histClassDenom, listDenom);

  }

}

//_____________________________________________________________________________
TObjArray* PairAnalysisHistos::DrawSame(TString histName, const Option_t *opt, TString histClassDenom, THashList *listDenom)
{
  //
  // Draw all histograms with the same name into one canvas
  // if option contains 'leg' a legend will be created with the class name as caption
  // if option contains 'can' a new canvas is created
  // if option contains 'rebin' the objects are rebinned by 2
  // if option contains 'norm' the objects are normalized to 1
  // if option contains 'events' use meta data to normalize
  // if option contains 'logx,y,z' the axis are plotted in log
  // if option contains 'meta' the meta data are plotted
  // if option contains 'NoMc' mc signals are not plotted
  // if option contains 'NoMcTrue' mc truth signals are not plotted
  // if option contains 'OnlyMc' only mc signals are plotted
  // if option contains 'Eff' efficiencies are plotted
  // if option contains 'Ratio' the ratios of any histclass to 'histClassDenom' are plotted
  // if option contains 'meanX,Y' quote the mean in the legend
  // if option contains 'rmsX,Y' quote the rms in the legend
  // if option contains 'task' histograms of different tasks are compared (see DrawTaskSame)
  // if option contains 'div' histograms of different tasks are divided (see DrawTaskSame)
  // if option contains 'goff' graphics off
  // if option contains 'eps' save as eps file
  // if option contains 'png' save as png file

  TString optString(opt);
  optString.ToLower();
  printf("Plot hist: '%s' class-denom/sel: '%s' \t listDenom: '%s' \t options: '%s' \n",
       histName.Data(), histClassDenom.Data(), (listDenom?listDenom->GetName():""), optString.Data());
  Bool_t optGoff     =optString.Contains("goff");      optString.ReplaceAll("goff","");
  // Bool_t optEps      =optString.Contains("eps");       optString.ReplaceAll("eps","");
  // Bool_t optPng      =optString.Contains("png");       optString.ReplaceAll("png","");
  Bool_t optTask     =optString.Contains("task");      optString.ReplaceAll("task","");
  // options - calulation
  Bool_t optDiv      =optString.Contains("div");       optString.ReplaceAll("div","");
  Bool_t optEff      =optString.Contains("eff");       optString.ReplaceAll("eff","");
  Bool_t optRatio    =optString.Contains("ratio");     optString.ReplaceAll("ratio","");
  Bool_t optOneOver  =optString.Contains("oneover");   optString.ReplaceAll("oneover","");
  // options - selection
  Bool_t optNoMCtrue =optString.Contains("nomctrue");  optString.ReplaceAll("nomctrue","");
  Bool_t optNoMC     =optString.Contains("nomc");      optString.ReplaceAll("nomc","");
  Bool_t optOnlyMC   =optString.Contains("onlymc");    optString.ReplaceAll("onlymc","");
  Bool_t optSel      =optString.Contains("sel");       optString.ReplaceAll("sel","");
  Bool_t optExclSel  =histClassDenom.Contains("!");    histClassDenom.ReplaceAll("!","");
  // options - representation
  Bool_t optCan      =optString.Contains("can");       optString.ReplaceAll("can","");
  Bool_t optLeg      =optString.Contains("leg");       optString.ReplaceAll("leg","");
  Bool_t optMeta     =optString.Contains("meta");      optString.ReplaceAll("meta","");
  Bool_t optRbn      =optString.Contains("rebin");     optString.ReplaceAll("rebin","");
  Bool_t optSclMax   =optString.Contains("sclmax");    optString.ReplaceAll("sclmax","");
  Bool_t optNormY    =optString.Contains("normy");      optString.ReplaceAll("normy","");
  Bool_t optNorm     =optString.Contains("norm");      optString.ReplaceAll("norm","");
  Bool_t optEvt      =optString.Contains("events");    optString.ReplaceAll("events","");
  // options - information
  Bool_t optMeanX    =optString.Contains("meanx");     optString.ReplaceAll("meanx","");
  Bool_t optRmsX     =optString.Contains("rmsx");      optString.ReplaceAll("rmsx","");
  Bool_t optMeanY    =optString.Contains("meany");     optString.ReplaceAll("meany","");
  Bool_t optRmsY     =optString.Contains("rmsy");      optString.ReplaceAll("rmsy","");
  Bool_t optGeant    =optString.Contains("geant");     optString.ReplaceAll("geant","");

  // selction string
  TString select("");
  if(optSel) select=histClassDenom;

  // output array
  TObjArray *arr=0x0;
  if(optGoff) {
    Info("DrawSame","graphics option off, collect an array");
    //    arr=(TObjArray*)gROOT->FindObject(Form("%s",histName.Data()));
    arr=(TObjArray*)gROOT->FindObject(GetName());
    if(arr) { arr->ls(); arr->Clear(); }
    else      arr = new TObjArray();
    //    arr->SetName(Form("%s",histName.Data()));
    arr->SetName(GetName());
    arr->SetOwner(kFALSE);
  }

  // add canvas
  TCanvas *c=0;
  if (optCan){
    c=(TCanvas*)gROOT->FindObject(Form("c%s",histName.Data()));
    if (!c) c=new TCanvas(Form("c%s",histName.Data()),Form("All '%s' histograms",histName.Data()));
    // c->Clear();
    c->cd();
  }

  // count number of drawn objects in pad
  TObject *obj;
  Int_t nobj=0;
  TList *prim = gPad->GetListOfPrimitives();
  if(prim->GetSize()>1) prim->RemoveLast(); // remove redraw axis histogram
  for(Int_t io=0; io<prim->GetSize(); io++) {
    obj=prim->At(io);
    if(obj->InheritsFrom(TH1::Class()) && obj!=prim->At(io+1)) nobj++;
  }

  // add or get legend
  TLegend *leg=0;
  if ( (optLeg && optTask && !nobj) || (optLeg && !optTask) ) {
    leg=new TLegend(0. + gPad->GetLeftMargin()  + gStyle->GetTickLength("Y"),
		    0. + gPad->GetBottomMargin()+ gStyle->GetTickLength("X"),
		    1. - gPad->GetRightMargin() + gStyle->GetTickLength("Y"),
		    1. - gPad->GetTopMargin()   + gStyle->GetTickLength("X"),
		    GetName(),"nbNDC");
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

  // meta data
  Int_t events = 1;
  if(fMetaData && optEvt)  fMetaData->GetMeta("events",&events);

  Int_t i=(nobj ? 10 : 0); // TOD0: obsolete?
  if(optTask && nobj) i=nobj;
  TIter next(&fHistoList);
  THashList *classTable=0;
  TH1 *hFirst=0x0;
  while ( (classTable=(THashList*)next()) ){
    TString histClass=classTable->GetName();

    Int_t ndel = histClass.CountChar('_');
    Info("DrawSame","Check plot hist: '%s' class-denom: '%s' select: '%s' \t ndel: %d \t for class: '%s'",
	 histName.Data(), histClassDenom.Data(), select.Data(), ndel, histClass.Data() );

    // check MC options
    if( (optNoMC && ndel>0) ||
	(optEff && ndel<1)  ||
	(optNoMCtrue && histClass.Contains("_MCtruth")) ||
	(optOnlyMC && ndel<1)                      ) continue;

    // histclass selection
    if( optSel && (!histClass.Contains(select,TString::kIgnoreCase))^optExclSel ) continue;
    //    if( optSel && (!histClass.Contains(histClassDenom))^optExclSel ) continue;
    
    // find the histogram in the class table
    if ( TH1 *h=(TH1*)classTable->FindObject(histName.Data()) ){

      // check if efficiency caluclation is possible
      if(optEff && !histClass.Contains("_MCtruth")) histClassDenom = histClass + "_MCtruth";
      //      if(optEff && (!fHistoList.FindObject( histClassName.Data() ) || histClass.Contains("_MCtruth")) ) continue;
      //      if(optEff && (!fHistoList.FindObject( Form("%s_MCtruth",histClass.Data()) ) || histClass.Contains("_MCtruth")) ) continue;

      // check if ratio should be build
      if( (optEff || optRatio) && !optTask && (histClass.EqualTo(histClassDenom) || !fHistoList.FindObject(histClassDenom.Data())) ) continue;
      Info("DrawSame","histClass %s (denom %s) ",histClass.Data(),histClassDenom.Data());
      if (i==0) hFirst=h;

      // style
      h->UseCurrentStyle();
      h->SetTitle("");
      PairAnalysisStyler::Style(h,i);
      if(optString.Contains("scat")) h->SetMarkerStyle(kDot);
      if(optString.Contains("e"))    h->SetLineStyle(kSolid);
      if(optString.Contains("text")) { h->SetLineColor(1); h->SetMarkerColor(1); }

      // set geant process labels
      // if(!histName.CompareTo("GeantId")) PairAnalysisHelper::SetGEANTBinLabels(h);

      // normalisation
      h->Sumw2();
      if(optRbn)                    h->Rebin();
      if(optNormY && h->GetDimension()==2 && !(h->GetSumOfWeights()==0)) {
	TH2 *hsum = (TH2*) h->Clone("orig");
	hsum->Reset("CE");
	for(Int_t ix = 1; ix <= hsum->GetNbinsX(); ix++) {
	  Double_t ysum = h->Integral(ix,ix);
	  for(Int_t iy = 1; iy <= hsum->GetNbinsY(); iy++) {
	    hsum->SetBinContent(ix,iy,ysum);
	  }
	}
	h->Divide(hsum);
	delete hsum;
      }
      if(optNorm && !(h->GetSumOfWeights()==0)) h=h->DrawNormalized(i>0?(optString+"same").Data():optString.Data());
      if(optEvt)                    h->Scale(1./events);
      if(optSclMax)                 h->Scale(1./h->GetBinContent(h->GetMaximumBin()));

      // set title
      TString ytitle = h->GetYaxis()->GetTitle();
      TString ztitle = h->GetZaxis()->GetTitle();
      switch(h->GetDimension()) {
      case 1:
	if(optEvt)    h->SetYTitle( (ytitle+"/N_{evt}").Data() );
	if(optNorm)   h->SetYTitle( (ytitle.Append(" (normalized)")).Data() );
	if(optRatio)  h->SetYTitle( "ratio" );
	if(optDiv)    h->SetYTitle( "ratio" );
	if(optEff)    h->SetYTitle( "efficiency" );
	if(optSclMax) h->SetYTitle( (ytitle+"/N_{max}").Data() );
	break;
      case 2:
	if(optEvt)    h->SetZTitle( (ztitle+"/N_{evt}").Data() );
	//	if(optNormY)  h->SetYTitle( (ytitle.Append(" (normalized)")).Data() );
	if(optNorm)   h->SetZTitle( (ztitle.Prepend("normalized ")).Data() );
	if(optRatio)  h->SetZTitle( "ratio" );
	if(optDiv)    h->SetZTitle( "ratio" );
	if(optEff)    h->SetZTitle( "efficiency" );
	if(optSclMax) h->SetZTitle( (ztitle+"/N_{max}").Data() );
	break;
      }

      // ratio and drawing
      if( (optEff || optRatio) && !optNorm && !optEvt && !optTask)  {
	//	TString    clMC     = histClass+"_MCtruth";
	THashList *clDenom  = (THashList*)fHistoList.FindObject( histClassDenom.Data() );
	TH1 *hMC = (TH1*) h->Clone(); // needed to preserve the labeling of non-mc histogram
	TH1 *hdenom         = (TH1*) clDenom->FindObject( UserHistogram(histClassDenom.Data(),hMC).Data() );
	//	TH1 *hdenom         = (TH1*) clDenom->FindObject( UserHistogram(clMC.Data(),hMC).Data() );
	delete hMC; //delete the surplus object
	if(!hdenom) { Error("DrawSame","Denominator object not found"); continue; }
	// normalize and rebin only once
	hdenom->Sumw2();
	if(optRbn && (optEff || !(i%10)) )       hdenom->Rebin();
	if(optEvt && (optEff || !(i%10)) )       hdenom->Scale(1./events);
	//	Printf("h %p %f hdenom %p %f",h,h->GetEntries(),hdenom,hdenom->GetEntries());
	if(!hdenom || !h->Divide(hdenom))  { Warning("DrawSame(eff/ratio)","Division failed!!!!"); continue; }
      }
      else if( optTask && (optDiv || optEff || optRatio)) {
	// denominators
	TH1* hdenom=0x0;
	TH1* htden =0x0;
	if(optEff || optRatio) {
	  THashList *clDenom  = (THashList*)fHistoList.FindObject( histClassDenom.Data() );
	  TH1 *hMC = (TH1*) h->Clone(); // needed to preserve the labeling of non-mc histogram
	  TString histdenomMC = UserHistogram(histClassDenom.Data(),hMC);
	  //delete the surplus object
	  delete hMC;

	  if(clDenom) {
	    hdenom         = (TH1*) clDenom->FindObject( histdenomMC.Data() );
	  }

	  if(listDenom) {
	    THashList *clTaskDen = (THashList*)listDenom->FindObject( histClassDenom.Data() );
	    if(clTaskDen) {
	      //	  htden=(TH1*)clTaskDen->FindObject(hdenom->GetName());
	      htden=(TH1*)clTaskDen->FindObject( histdenomMC.Data() );
	      Info("DrawSame","calculate eff/ratio using task-denom: '%s' class-denom: '%s' hist-denom: '%s'",
		   listDenom->GetName(), histClassDenom.Data(), histdenomMC.Data());
	      // keep only one of them, otherwise you might divide the same objects twice
	      if(htden) hdenom=0x0;
	    }
	  }

	}


	// task ratio
	TH1 *htnom=0x0;
	if(optDiv && !optEff) {
	  THashList *clTaskNom = (THashList*)listDenom->FindObject( histClass.Data() );
	  if(!clTaskNom) continue;
	  htnom=(TH1*)clTaskNom->FindObject(histName.Data());
	  if(!htnom) continue;
	}

	if(hdenom) hdenom->Sumw2();
	if(htden)  htden->Sumw2();
	if(htnom)  htnom->Sumw2();

	// normalize and rebin only once
	if(optRbn && !i) {
	  // TODO: check for consistency if opttask, than htden is rebinned multiple times!
	  //	  Printf(" rebin spectra");
	  if(hdenom) hdenom->Rebin();
	  if(htden)  htden->Rebin();
	  if(htnom)  htnom->Rebin();
	}
	if(optEvt && !i ) {
	  if(hdenom) hdenom->Scale(1./events);
	  if(htden)  htden->Scale(1./events);
	  if(htnom)  htnom->Scale(1./events);
	}

	Info("DrawSame"," Entries: h %p %e,  hDenom %p %e,  htden %p %e ",
	     h,h->GetEntries(),hdenom,(hdenom?hdenom->GetEntries():0),
	     htden,(htden?htden->GetEntries():0));
	// Printf("h %p (bins%d) \t hdenom %p (bins%d) \t htdenom %p (bins%d) \t htnom %p (bins%d)",
	//        h,h->GetNbinsX(),hdenom,(hdenom?hdenom->GetNbinsX():0),
	//        htden,(htden?htden->GetNbinsX():0),htnom,(htnom?htnom->GetNbinsX():0));

	// standard ratio
	if(hdenom && !h->Divide(hdenom))        { Warning("DrawSame(eff/ratio)","h & denom division failed!!!!"); continue; }
	else if(htden  && htnom && !i && !htnom->Divide(htden)) { Warning("DrawSame(eff/ratio)","task-nom/task-denom division failed!!!!"); continue; }
	else if(optDiv && htnom &&!h->Divide(htnom)) { Warning("DrawSame(eff/ratio)","h & task-nom division failed!!!!"); continue; }
	else if(htden  && !h->Divide(htden))         { Warning("DrawSame(eff/ratio)","h & task-denom division failed!!!!"); continue; }
      }

      // flip content values
      if(optOneOver){
	TH1 *hOne = (TH1*) h->Clone("one");
	hOne->Reset("ICSE");
	for(Int_t ib=0;ib<(h->GetNbinsX()+2)*(h->GetNbinsY()+2)*(h->GetNbinsZ()+2);ib++)
	  hOne->SetBinContent(ib,1.);
	if(hOne->Divide(h)) h=hOne;
      }

      // geant labels
      if(optGeant) PairAnalysisHelper::SetGEANTBinLabels(h);

      // change name
      //      h->SetName(histClass.Data());
      h->SetTitle(histClass.Data());
      if(optGoff) arr->Add(h);

      // draw prepared histogram
      if(!optGoff) {
	optString.ReplaceAll(" ","");
	Info("DrawSame","draw object with draw options: '%s'",optString.Data());
	h->Draw(i>0?(optString+"same").Data():optString.Data());
      }



      // protection e.g. normalization not possible TProfile
      if(h && h->GetEntries()>0.) {

	TString ratioName=histClassDenom;
	TString divName=(listDenom?listDenom->GetName():"");
	if(optEff) divName+=(listDenom?"(MC)":"");
	// adapt legend name
	// remove reserved words
	TObjArray *reservedWords = fReservedWords->Tokenize(":;");
	for(Int_t ir=0; ir<reservedWords->GetEntriesFast(); ir++) {
	  histClass.ReplaceAll( ((TObjString*)reservedWords->At(ir))->GetString(), "");
	  ratioName.ReplaceAll( ((TObjString*)reservedWords->At(ir))->GetString(), "");
	  divName.ReplaceAll( ((TObjString*)reservedWords->At(ir))->GetString(), "");
	}
	// change default signal names to titles
	for(Int_t isig=0; isig<PairAnalysisSignalMC::kNSignals; isig++) {
	  histClass.ReplaceAll(PairAnalysisSignalMC::fgkSignals[isig][0],PairAnalysisSignalMC::fgkSignals[isig][1]);
	  ratioName.ReplaceAll(PairAnalysisSignalMC::fgkSignals[isig][0],PairAnalysisSignalMC::fgkSignals[isig][1]);
	  divName.ReplaceAll(PairAnalysisSignalMC::fgkSignals[isig][0],PairAnalysisSignalMC::fgkSignals[isig][1]);
	}
	// change MCtruth to MC
	for(Int_t isig=0; isig<PairAnalysisSignalMC::kNSignals; isig++) {
	  histClass.ReplaceAll("MCtruth","MC");
	  ratioName.ReplaceAll("MCtruth","MC");
	  divName.ReplaceAll("MCtruth","MC");
	}
	// remove pairing name if it is a MC
	for(Int_t iptype=0; iptype<PairAnalysis::kPairTypes; iptype++) {
	  if(ndel>0)                     histClass.ReplaceAll( PairAnalysis::PairClassName(iptype), "");
	  if(ratioName.CountChar('_')>0) ratioName.ReplaceAll( PairAnalysis::PairClassName(iptype), "");
	  if(divName.CountChar('_')>0)   divName.ReplaceAll( PairAnalysis::PairClassName(iptype), "");
	}
	// save Dalitz underscore
	histClass.ReplaceAll("_{Dalitz}","#{Dalitz}");
	ratioName.ReplaceAll("_{Dalitz}","#{Dalitz}");
	divName.ReplaceAll("_{Dalitz}","#{Dalitz}");
	// remove delimiters
	histClass.ReplaceAll("_"," ");
	ratioName.ReplaceAll("_"," ");
	divName.ReplaceAll("_"," ");
	histClass.ReplaceAll(".","");
	ratioName.ReplaceAll(".","");
	divName.ReplaceAll(".","");
	// get Dalitz back
	histClass.ReplaceAll("#{Dalitz}","_{Dalitz}");
	ratioName.ReplaceAll("#{Dalitz}","_{Dalitz}");
	divName.ReplaceAll("#{Dalitz}","_{Dalitz}");
	// remove trailing and leading spaces
	histClass.Remove(TString::kBoth,' ');
	ratioName.Remove(TString::kBoth,' ');
	divName.Remove(TString::kBoth,' ');

	//build final ratio name
	if(optRatio)  histClass+="/"+ratioName;

	// delete the surplus
	delete reservedWords;

	// modify legend option
	TString legOpt = optString+"L";
	legOpt.ReplaceAll("hist","");
	legOpt.ReplaceAll("scat","");
	//	legOpt.ReplaceAll("col","");
	legOpt.ReplaceAll("z","");
	legOpt.ReplaceAll("e","");
	if (optTask)                histClass.Prepend(Form("%s ",GetName()));
	if (optDiv && !optOneOver)  histClass.ReplaceAll(GetName(),Form("%s/%s",GetName(),divName.Data()));
	if (optDiv &&  optOneOver)  histClass.Prepend(Form("%s/",divName.Data()));
	//	else if(nobj)     histClass="";
	if(optMeanX) histClass+=Form(" #LTx#GT=%.1e",h->GetMean());
	if(optRmsX)  histClass+=Form(" RMS(x)=%.1e",h->GetRMS());
	if(optMeanY) histClass+=Form(" #LTy#GT=%.2e",h->GetMean(2));
	if(optRmsY)  histClass+=Form(" RMS(y)=%.2e",h->GetRMS(2));
	histClass.ReplaceAll("e+00","");
	// no entry for colored plots
	if (leg && !legOpt.Contains("col")) leg->AddEntry(h,histClass.Data(),legOpt.Data());

	//      if (leg) leg->AddEntry(h,classTable->GetName(),(optString+"L").Data());
	++i;

      }
      else if(nobj&&leg) leg->AddEntry(hFirst,"","");

      //++i;


    }
  }

  // legend
  if (leg) {
    PairAnalysisStyler::SetLegendAttributes(leg); // coordinates, margins, fillstyle, fontsize
    if(!nobj) leg->Draw(); // only draw the legend once
  }

  // axis maximum
  Double_t max=-1e10;
  Double_t min=+1e10;
  TListIter nextObj(gPad->GetListOfPrimitives(),kIterBackward);
  //  TObject *obj;
  while ((obj = nextObj())) {
    if(obj->InheritsFrom(TH1::Class())) {
      TH1* h1 = static_cast<TH1*>(obj);
      //      Printf("max%f \t min%f",h1->GetMaximum(),PairAnalysisHelper::GetContentMinimum(h1));
      max=TMath::Max(max,h1->GetMaximum());
      min=TMath::Min(min,PairAnalysisHelper::GetContentMinimum(h1));//hobj->GetBinContent(hobj->GetMinimumBin()));
      //Printf("max%f \t min%f",max,min);
      if(!optEff) h1->SetMaximum(max*(gPad->GetLogy()?5.:1.1));
      else        h1->SetMaximum(1.1);
      if(!optEff) h1->SetMinimum( min*(min<0.?1.1:0.9) ); //TODO: doesnt work, why?? Negative values?
      // automatically set log option
      if(gPad->GetLogy() && max/(min>0.?min:1.) > TMath::Power(10.,TGaxis::GetMaxDigits())) {
	h1->GetYaxis()->SetMoreLogLabels(kFALSE);
	h1->GetYaxis()->SetNoExponent(kFALSE);
      }
      if(gPad->GetLogx() && h1->GetXaxis()->GetXmax()/h1->GetXaxis()->GetXmin() > TMath::Power(10.,TGaxis::GetMaxDigits())) {
	h1->GetXaxis()->SetMoreLogLabels(kFALSE);
	h1->GetXaxis()->SetNoExponent(kFALSE);
      }
    }
  }

  // draw meta data
  if(!nobj && optMeta && fMetaData && !gPad->GetPrimitive("meta")) {
    fMetaData->DrawSame("");
  }

  // force legend to be drawn always on top, remove multiple versions of it
  // they show up when one uses the 'task' draw option
  if(!optTask) {
    if(leg) leg->DrawClone();
    nextObj.Reset();
    Int_t ileg = 0;
    while ((obj = nextObj())) {
      if(obj->InheritsFrom(TLegend::Class())) {
	if(ileg>0) delete obj;
	ileg++;
      }
    }
  }

  // styling
  // NOTE: this draws a copy of the first histogram
  gPad->RedrawAxis(); 

  // remove canvas
  if(optGoff) delete c;

  // // save as
  // if(optEps) c->SaveAs(Form("%s.eps",c->GetName()));
  // if(optPng) c->SaveAs(Form("%s.png",c->GetName()));

  return arr;
}

//_____________________________________________________________________________
void PairAnalysisHistos::SetReservedWords(const char* words)
{
  //
  // set reserved words
  //

  (*fReservedWords)=words;
}

//_____________________________________________________________________________
void PairAnalysisHistos::StoreVariables(TObject *obj, UInt_t valType[20])
{
  //
  //
  //
  if (!obj) return;
  if      (obj->InheritsFrom(TH1::Class()))         StoreVariables(static_cast<TH1*>(obj), valType);
  else if (obj->InheritsFrom(THnBase::Class()))     StoreVariables(static_cast<THnBase*>(obj), valType);

  return;

}


//_____________________________________________________________________________
void PairAnalysisHistos::StoreVariables(TH1 *obj, UInt_t valType[20])
{
  //
  // store variables in the axis (special for TProfile3D)
  //

  Int_t dim   = obj->GetDimension();

  // dimension correction for profiles
  if(obj->IsA() == TProfile::Class() || obj->IsA() == TProfile2D::Class() || obj->IsA() == TProfile3D::Class()) {
    dim++;
  }

  switch( dim ) {
  case 4:    obj->SetUniqueID(valType[3]); // Tprofile3D variable
  case 3:    obj->GetZaxis()->SetUniqueID(valType[2]);
  case 2:    obj->GetYaxis()->SetUniqueID(valType[1]);
  case 1:    obj->GetXaxis()->SetUniqueID(valType[0]);
  }

  return;
}

//_____________________________________________________________________________
void PairAnalysisHistos::StoreVariables(THnBase *obj, UInt_t valType[20])
{
  //
  // store variables in the axis
  //

  Int_t dim = obj->GetNdimensions();

  for(Int_t it=0; it<dim; it++) {
    obj->GetAxis(it)->SetUniqueID(valType[it]);
    obj->GetAxis(it)->SetName(Form("%s", PairAnalysisVarManager::GetValueName(valType[it])));
    obj->GetAxis(it)->SetTitle(Form("%s %s", PairAnalysisVarManager::GetValueLabel(valType[it]), PairAnalysisVarManager::GetValueUnit(valType[it])));
  }
  obj->Sumw2();
  return;
}

//_____________________________________________________________________________
void PairAnalysisHistos::FillValues(TObject *obj, const Double_t *values)
{
  //
  //
  //
  if (!obj) return;
  if      (obj->InheritsFrom(TH1::Class()))       FillValues(static_cast<TH1*>(obj), values);
  else if (obj->InheritsFrom(THnBase::Class()))   FillValues(static_cast<THnBase*>(obj), values);

  return;

}

//_____________________________________________________________________________
void PairAnalysisHistos::FillValues(TH1 *obj, const Double_t *values)
{
  //
  // fill values for TH1 inherted classes
  //

  Int_t dim   = obj->GetDimension();
  Bool_t bprf = kFALSE;
  //  UInt_t nValues = (UInt_t) PairAnalysisVarManager::kNMaxValues;

  UInt_t valueTypes=obj->GetUniqueID();
  if (valueTypes==(UInt_t)PairAnalysisHistos::kNoAutoFill) return;
  Bool_t weight = (valueTypes!=kNoWeights);

  // check if tprofile
  if(obj->IsA() == TProfile::Class()   ||
     obj->IsA() == TProfile2D::Class() ||
     obj->IsA() == TProfile3D::Class()  )    bprf=kTRUE;

  // TO BEAUTIFY: switch off manually weighting of profile3Ds
  if(obj->IsA() == TProfile3D::Class()) weight=kFALSE;

  // check for formulas
  TList *list = obj->GetListOfFunctions();
  TFormula *xform = dynamic_cast<TFormula*>(list->FindObject("xFormula"));
  TFormula *yform = dynamic_cast<TFormula*>(list->FindObject("yFormula"));
  TFormula *zform = dynamic_cast<TFormula*>(list->FindObject("zFormula"));
  TFormula *pform = dynamic_cast<TFormula*>(list->FindObject("pFormula"));
  TFormula *wform = dynamic_cast<TFormula*>(list->FindObject("wFormula"));
  //  Bool_t bform = (xform || yform || zform /*|| wform*/);

  // get variables from axis unique IDs
  UInt_t value1=obj->GetXaxis()->GetUniqueID();
  UInt_t value2=obj->GetYaxis()->GetUniqueID();
  UInt_t value3=obj->GetZaxis()->GetUniqueID();
  UInt_t value4=obj->GetUniqueID();            // get weighting/profile var stored in the unique ID

  Double_t fvals[4] = { values[value1], values[value2], values[value3], values[value4] };

  // use formulas to update fill values
  if(xform) fvals[0] = PairAnalysisHelper::EvalFormula(xform, values);
  if(yform) fvals[1] = PairAnalysisHelper::EvalFormula(yform, values);
  if(zform) fvals[2] = PairAnalysisHelper::EvalFormula(zform, values);
  if(wform) fvals[3] = PairAnalysisHelper::EvalFormula(wform, values);
  if(pform) fvals[3] = PairAnalysisHelper::EvalFormula(pform, values); // weighting overwriting for Profile3D

  // ask for inclusive trigger map variables
  if(value1!=PairAnalysisVarManager::kTriggerInclONL && value1!=PairAnalysisVarManager::kTriggerInclOFF &&
     value2!=PairAnalysisVarManager::kTriggerInclONL && value2!=PairAnalysisVarManager::kTriggerInclOFF &&
     value3!=PairAnalysisVarManager::kTriggerInclONL && value3!=PairAnalysisVarManager::kTriggerInclOFF &&
     value4!=PairAnalysisVarManager::kTriggerInclONL && value4!=PairAnalysisVarManager::kTriggerInclOFF ) {
    // no trigger map variable selected
    switch ( dim ) {
    case 1:
      if(!bprf && !weight)     obj->Fill(fvals[0]);                 // histograms
      else if(!bprf && weight) obj->Fill(fvals[0], fvals[3]); // weighted histograms
      else if(bprf && !weight) ((TProfile*)obj)->Fill(fvals[0],fvals[1]);   // profiles
      else                     ((TProfile*)obj)->Fill(fvals[0],fvals[1], fvals[3]); // weighted profiles
      break;
    case 2:
      if(!bprf && !weight)     obj->Fill(fvals[0], fvals[1]);                 // histograms
      else if(!bprf && weight) ((TH2*)obj)->Fill(fvals[0], fvals[1], fvals[3]); // weighted histograms
      else if(bprf && !weight) ((TProfile2D*)obj)->Fill(fvals[0], fvals[1], fvals[2]); // profiles
      else                     ((TProfile2D*)obj)->Fill(fvals[0], fvals[1], fvals[2], fvals[3]); // weighted profiles
      break;
    case 3:
      if(!bprf && !weight)     ((TH3*)obj)->Fill(fvals[0], fvals[1], fvals[2]);                 // histograms
      else if(!bprf && weight) ((TH3*)obj)->Fill(fvals[0], fvals[1], fvals[2], fvals[3]); // weighted histograms
      else if(bprf && !weight) ((TProfile3D*)obj)->Fill(fvals[0], fvals[1], fvals[2], fvals[3]); // profiles
      else                     Printf(" WARNING: weighting NOT yet possible for TProfile3Ds !");
      break;
    }
  }
  else {
    // fill inclusive trigger map variables
    if(weight) return;
    switch ( dim ) {
    case 1:
      for(Int_t i=0; i<30; i++) { if(TESTBIT((UInt_t)fvals[0],i)) obj->Fill(i); }
      break;
    case 2:
      if((value1==PairAnalysisVarManager::kTriggerInclOFF && value2==PairAnalysisVarManager::kTriggerInclONL) ||
	 (value1==PairAnalysisVarManager::kTriggerInclONL && value2==PairAnalysisVarManager::kTriggerInclOFF) ) {
	for(Int_t i=0; i<30; i++) {
	  if((UInt_t)fvals[0]==BIT(i)) {
	    for(Int_t i2=0; i2<30; i2++) {
	      if((UInt_t)fvals[1]==BIT(i2)) {
		obj->Fill(i, i2);
	      } // bit fired
	    } //loop 2
	  }//bit fired
	} // loop 1
      }
      else if(value1==PairAnalysisVarManager::kTriggerInclONL || value1==PairAnalysisVarManager::kTriggerInclOFF) {
	for(Int_t i=0; i<30; i++) { if(TESTBIT((UInt_t)fvals[0],i)) obj->Fill(i, fvals[1]); }
      }
      else if(value2==PairAnalysisVarManager::kTriggerInclONL || value2==PairAnalysisVarManager::kTriggerInclOFF) {
	for(Int_t i=0; i<30; i++) { if(TESTBIT((UInt_t)fvals[1],i)) obj->Fill(fvals[0], i); }
      }
      else //makes no sense
	return;
      break;
    default: return;
    }

  } //end: trigger filling


  return;
}

//_____________________________________________________________________________
void PairAnalysisHistos::FillValues(THnBase *obj, const Double_t *values)
{
  //
  // fill values for THn inherted classes
  //

  const Int_t dim   = obj->GetNdimensions();

  UInt_t valueTypes=obj->GetUniqueID();
  if (valueTypes==(UInt_t)PairAnalysisHistos::kNoAutoFill) return;
  Bool_t weight = (valueTypes!=kNoWeights);

  UInt_t value4=obj->GetUniqueID();            // weight variable

  Double_t fill[dim];
  for(Int_t it=0; it<dim; it++)   fill[it] = values[obj->GetAxis(it)->GetUniqueID()];
  if(!weight) obj->Fill(fill);
  else obj->Fill(fill, values[value4]);


  return;
}

//_____________________________________________________________________________
void PairAnalysisHistos::FillVarArray(TObject *obj, UInt_t *valType)
{
  //
  // extract variables stored in the axis (special for TProfile3D)
  //


  if (!obj) return;
  //  Printf(" fillvararray %s",obj->GetName());

  if (obj->InheritsFrom(TH1::Class())) {
    valType[0]=((TH1*)obj)->GetXaxis()->GetUniqueID();
    valType[1]=((TH1*)obj)->GetYaxis()->GetUniqueID();
    valType[2]=((TH1*)obj)->GetZaxis()->GetUniqueID();
    valType[3]=((TH1*)obj)->GetUniqueID();  // weighting(profile) var stored in unique ID
  }
  else if (obj->InheritsFrom(THnBase::Class())) {
    for(Int_t it=0; it<((THn*)obj)->GetNdimensions(); it++)
      valType[it]=((THn*)obj)->GetAxis(it)->GetUniqueID();
  }
  valType[19]=obj->GetUniqueID(); //weights
  return;
}

//_____________________________________________________________________________
void PairAnalysisHistos::AdaptNameTitle(TH1 *hist, const char* histClass) {

  //
  // adapt name and title of the histogram
  //

  Int_t dim            = hist->GetDimension();
  TString currentName  = "";//hist->GetName();
  TString currentTitle = "";//hist->GetTitle();
  TString hclass       = histClass;
  //get reserved class
  TObjArray *arr=hclass.Tokenize("_.");
  arr->SetOwner();
  hclass=((TObjString*)arr->At(0))->GetString();
  delete arr;

  Bool_t bname  = (currentName.IsNull());
  Bool_t btitle = (currentTitle.IsNull());
  Bool_t bprf   = kFALSE;
  if(hist->IsA() == TProfile::Class() || hist->IsA() == TProfile2D::Class() || hist->IsA() == TProfile3D::Class())
    bprf=kTRUE;

  // tprofile options
  Double_t pmin=0., pmax=0.;
  TString option = "", calcrange="";
  Bool_t bStdOpt=kTRUE;
  if(bprf) {
    switch( dim ) {
    case 3:
      option = ((TProfile3D*)hist)->GetErrorOption();
      pmin   = ((TProfile3D*)hist)->GetTmin();
      pmax   = ((TProfile3D*)hist)->GetTmax();
      break;
    case 2:
      option = ((TProfile2D*)hist)->GetErrorOption();
      pmin   = ((TProfile2D*)hist)->GetZmin();
      pmax   = ((TProfile2D*)hist)->GetZmax();
      break;
    case 1:
      option = ((TProfile*)hist)->GetErrorOption();
      pmin   = ((TProfile*)hist)->GetYmin();
      pmax   = ((TProfile*)hist)->GetYmax();
      break;
    }
    if(option.Contains("s",TString::kIgnoreCase)) bStdOpt=kFALSE;
    if(pmin!=pmax) calcrange=Form("#cbar_{%+.*f}^{%+.*f}",GetPrecision(pmin),pmin,GetPrecision(pmax),pmax);
  }

  UInt_t varx = hist->GetXaxis()->GetUniqueID();
  UInt_t vary = hist->GetYaxis()->GetUniqueID();
  UInt_t varz = hist->GetZaxis()->GetUniqueID();
  UInt_t varp = hist->GetUniqueID();
  Bool_t weight = (varp!=kNoWeights);
  if(bprf && dim==3) weight=kFALSE; // no weighting for profile3D

  // store titles in the axis
  if(btitle) {
    TString tit = "";
    /////// set NAMES
    hist->GetXaxis()->SetName(PairAnalysisVarManager::GetValueName(varx));
    hist->GetYaxis()->SetName(PairAnalysisVarManager::GetValueName(vary));
    hist->GetZaxis()->SetName(PairAnalysisVarManager::GetValueName(varz));
    // adapt according to formula
    TFormula *xform = dynamic_cast<TFormula*>(hist->GetListOfFunctions()->FindObject("xFormula"));
    TFormula *yform = dynamic_cast<TFormula*>(hist->GetListOfFunctions()->FindObject("yFormula"));
    TFormula *zform = dynamic_cast<TFormula*>(hist->GetListOfFunctions()->FindObject("zFormula"));
    TFormula *wform = dynamic_cast<TFormula*>(hist->GetListOfFunctions()->FindObject("wFormula"));
    if(xform) { hist->GetXaxis()->SetName(PairAnalysisHelper::GetFormulaName(xform).Data()); }
    if(yform) { hist->GetYaxis()->SetName(PairAnalysisHelper::GetFormulaName(yform).Data()); }
    if(zform) { hist->GetZaxis()->SetName(PairAnalysisHelper::GetFormulaName(zform).Data()); }
    /////// set TITLE
    hist->GetXaxis()->SetTitle(PairAnalysisVarManager::GetValueLabel(varx));
    hist->GetYaxis()->SetTitle(PairAnalysisVarManager::GetValueLabel(vary));
    hist->GetZaxis()->SetTitle(PairAnalysisVarManager::GetValueLabel(varz));
    // adapt according to formula
    if(xform){hist->GetXaxis()->SetTitle(PairAnalysisHelper::GetFormulaTitle(xform).Data()); }
    if(yform){hist->GetYaxis()->SetTitle(PairAnalysisHelper::GetFormulaTitle(yform).Data()); }
    if(zform){hist->GetZaxis()->SetTitle(PairAnalysisHelper::GetFormulaTitle(zform).Data()); }
    // profile axis
    if(bprf && dim<3) {
      TAxis *ax    = 0x0;
      switch( dim ) {
      case 2: ax = hist->GetZaxis(); break;
      case 1: ax = hist->GetYaxis(); break;
      }
      tit=ax->GetTitle();
      tit.Prepend( (bStdOpt ? "#LT" : "RMS(") );
      tit.Append ( (bStdOpt ? "#GT" : ")")    );
      tit.Append ( calcrange.Data()           );
      ax->SetTitle(tit.Data());
    }
    // append the units for all axes (except formula)
    tit=Form("%s %s",hist->GetXaxis()->GetTitle(), PairAnalysisVarManager::GetValueUnit(varx));
    if(!xform) hist->GetXaxis()->SetTitle(tit.Data());
    tit=Form("%s %s",hist->GetYaxis()->GetTitle(), PairAnalysisVarManager::GetValueUnit(vary));
    if(!yform) hist->GetYaxis()->SetTitle(tit.Data());
    tit=Form("%s %s",hist->GetZaxis()->GetTitle(), PairAnalysisVarManager::GetValueUnit(varz));
    if(!zform) hist->GetZaxis()->SetTitle(tit.Data());
    // overwrite titles with hist class if needed
    if(!bprf) {
      switch( dim ) {
      case 1: hist->GetYaxis()->SetTitle(Form("%ss",hclass.Data())); break;
      case 2: hist->GetZaxis()->SetTitle(Form("%ss",hclass.Data())); break;
      }
    }
    // weighted axis (maximal 2 dimensional)
    if(weight) {
      TAxis *ax    = hist->GetYaxis();
      if(dim==2)ax = hist->GetZaxis();
      tit=PairAnalysisVarManager::GetValueLabel(varp);
      if(wform) { tit=PairAnalysisHelper::GetFormulaTitle(wform); }
      ax->SetTitle(Form("%s weighted %s", tit.Data(), ax->GetTitle()));
    }

    // create an unique name
    TFormula *pform = dynamic_cast<TFormula*>(hist->GetListOfFunctions()->FindObject("pFormula"));
    if(bname)
      switch(dim) {
      case 3:
	currentName+=Form("%s_",hist->GetXaxis()->GetName());
	currentName+=Form("%s_",hist->GetYaxis()->GetName());
	currentName+=Form("%s", hist->GetZaxis()->GetName());
	if(bprf && !pform) currentName+=Form("-%s%s",PairAnalysisVarManager::GetValueName(varp),(bStdOpt ? "avg" : "rms"));
	else if (bprf)     currentName+=Form("-%s%s",PairAnalysisHelper::GetFormulaName(pform).Data(),(bStdOpt ? "avg" : "rms"));
	if(weight&&!bprf) currentName+=Form("-wght%s",PairAnalysisVarManager::GetValueName(varp));
	break;
      case 2:
	currentName+=Form("%s_",hist->GetXaxis()->GetName());
	currentName+=Form("%s", hist->GetYaxis()->GetName());
	if(bprf)   currentName+=Form("-%s%s",hist->GetZaxis()->GetName(),(bStdOpt ? "avg" : "rms"));
	if(weight && !wform)      currentName+=Form("-wght%s",PairAnalysisVarManager::GetValueName(varp));
	else if(weight &&  wform) currentName+=Form("-wght%s",PairAnalysisHelper::GetFormulaName(wform).Data());
	break;
      case 1:
	currentName+=Form("%s",hist->GetXaxis()->GetName());
	if(bprf)   currentName+=Form("-%s%s",hist->GetYaxis()->GetName(),(bStdOpt ? "avg" : "rms"));
	if(weight && !wform)      currentName+=Form("-wght%s",PairAnalysisVarManager::GetValueName(varp));
	else if(weight &&  wform) currentName+=Form("-wght%s",PairAnalysisHelper::GetFormulaName(wform).Data());
	break;
      }
    // to differentiate btw. leg and pair histos
    //    if(!strcmp(histClass,"Pair")) currentName.Prepend("p");
    if(hclass.Contains("Pair")) currentName.Prepend("p");
    hist->SetName(currentName.Data());
  }
}

//_____________________________________________________________________________
Int_t PairAnalysisHistos::GetPrecision(Double_t value)
{
  //
  // computes the precision of a double
  // usefull for axis ranges etc
  // TODO: move to PairAnalysisHelper

  Bool_t bfnd     = kFALSE;
  Int_t precision = 0;
  value*=1e6;
  while(!bfnd) {
    //    Printf(" value %f precision %d bfnd %d",TMath::Abs(value*TMath::Power(10,precision)), precision, bfnd);
    bfnd = ((TMath::Abs(value*TMath::Power(10,precision))/1e6  -  TMath::Floor(TMath::Abs(value*TMath::Power(10,precision))/1e6)) != 0.0
	    ? kFALSE
	    : kTRUE);
    if(!bfnd) precision++;
  }

  //  Printf("precision for %f found to be %d", value, precision);
  return precision;

}
