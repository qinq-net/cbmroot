///////////////////////////////////////////////////////////////////////////
//                Dielectron Spectrum                                  //
//                                                                       //
//                                                                       //
/*

 */
//                                                                       //
///////////////////////////////////////////////////////////////////////////

//#include <TObject.h>
#include <TROOT.h>
#include <TTree.h>
#include <TEventList.h>

#include <TCanvas.h>
#include <TPad.h>

#include <TVectorT.h>
#include <TPaveText.h>
#include <TF1.h>
#include <TH1.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TGraphErrors.h>
#include <TDatabasePDG.h>
#include <TList.h>
#include <TFormula.h>

#include "PairAnalysisVarManager.h"
#include "PairAnalysisHistos.h"
#include "PairAnalysisHF.h"
#include "PairAnalysisStyler.h"


#include "PairAnalysisSignalExt.h"

#include "PairAnalysisSpectrum.h"

//______________________________________________


ClassImp(PairAnalysisSpectrum)

//______________________________________________
PairAnalysisSpectrum::PairAnalysisSpectrum() :
PairAnalysisSpectrum("spectrum","title")
{
  //
  // Named Constructor
  //
}

//______________________________________________
PairAnalysisSpectrum::PairAnalysisSpectrum(const char* name, const char* title) :
  PairAnalysisFunction(name,title),
  fRawInput(0),
  fMCInput(0),
  fCorrInput(0),
  fExtractor(0)
{
  //
  // Named Constructor
  //
  fTree = new TTree("PAPa","PAPa-Spectrum");
  fExt = new Extraction;
  for(Int_t i=0;i<100;i++) fInputKeys[i]="";

}

//______________________________________________
PairAnalysisSpectrum::~PairAnalysisSpectrum()
{
  //
  // Default Destructor
  //
  if(fResults)     delete fResults;
  if(fExtractions) delete fExtractions;
}

//______________________________________________
void PairAnalysisSpectrum::SetSpectrumCalc(const char *form, Double_t *params, Double_t *paramsE)
{
  //
  //
  //
  fCalculation = new TFormula("PAPa-Formula",form);
  fParams      = params;
  fParamsE     = paramsE;
}

//______________________________________________
void PairAnalysisSpectrum::AddInput( TObjArray *raw, TObjArray *mc, TString identifier)
{
  //
  // add input
  //
  fRawInput.Add(raw);
  if(mc) fMCInput.Add(mc);
  fInputKeys[fIdx]=identifier;
  fIdx++;
 }

//______________________________________________
void PairAnalysisSpectrum::Init()
{
  //
  // Initialize the tree
  //
  //  fTree ->Branch("hSignal","TH1",&fHistSignal,32000,0);
  fTree ->Branch("Extraction",&fExt);
}

//______________________________________________
void PairAnalysisSpectrum::Process()
{
  //
  //
  //
  Init();

  Int_t i=-1;

  TIter nextRaw(&fRawInput); // raw content
  TObject *obj=0x0;
  PairAnalysisHF         *hf  = 0x0;
  PairAnalysisHistos     *h   = 0x0;
  while( (obj = nextRaw()) )
    {
      i++;
      TObject::Info("Process","Check Extraction for %s",fInputKeys[i].Data());
      TObject::Info("Process","-----------------------------------------------------------------------");
      TObject::Info("Process","Input type: %s \n",obj->ClassName());

      if(fVarBinning) {
	TObject::Info("Process","Binning provided for %s from %.3f to %.3f",
		      fVar.Data(),fVarBinning->Min(),fVarBinning->Max());
	//	fVarBinning->Print();
      }

      TObjArray *histArr=dynamic_cast<TObjArray*>(obj);
      if(!(histArr) ) {
	if(!(h=dynamic_cast<PairAnalysisHistos*>(obj)) ) {
	  if(!(hf=dynamic_cast<PairAnalysisHF*>(obj)) ) {
	    TObject::Error("Process","No input format found");
	    continue;
	  }
	}
      }

      // get extractor
      PairAnalysisSignalExt *sig=dynamic_cast<PairAnalysisSignalExt*>(fExtractor.At(i));
      if(!sig) continue;

      // only integrated via histos
      if(!fVarBinning) {
	// get output
	if(!histArr && h) histArr = h->DrawSame("pM-wghtWeight","can nomc goff"); //NOTE w/o "can" it crashes
	//	if(!histArr && h) histArr = h->DrawSame("pM","can nomc goff"); //NOTE w/o "can" it crashes

	histArr->Print();

	// process
	sig->Process(histArr);
	if(gErrorIgnoreLevel<kWarning) sig->Print("");

	// validate signal extraction
	if(sig->GetSignal() < 0.) continue;

	// fill the tree
	fExt->setup= fInputKeys[i];
	fExt->setupId = i;
	fExt->var  = 1.; //integrated
	fExt->varE = 0.;
	fExt->s    = sig->GetSignal();
	fExt->sE   = sig->GetSignalError();
	fExt->b    = sig->GetBackground();
	fExt->bE   = sig->GetBackgroundError();
	fExt->sb   = sig->GetSB();
	fExt->sbE  = sig->GetSBError();
	fExt->sgn  = sig->GetSignificance();
	fExt->sgnE = sig->GetSignificanceError();
	fExt->HistSignal = NULL; //dynamic_cast<TH1F*>(sig->GetSignalHistogram());

	fTree->Fill();
      }
      else {

	// get 2D histogram
	if(h && !h->SetCutClass(fInputKeys[i].Data())) continue;

	if(!histArr && h) histArr = h->DrawSame(Form("pM_%s",fVar.Data()),"can nomc goff");

	TH2 *histPM = (TH2*) sig->FindObject(histArr, PairAnalysis::kSEPM);
	if(!histPM) return;

	TObjArray tmpArr;
	tmpArr.SetOwner(kFALSE);

	// loop over all bins
	for(Int_t bin=0;bin<fVarBinning->GetNrows()-1;bin++)
	  {
	    tmpArr.Clear();

	    // var bin limits
	    Double_t xLo = fVarBinning->GetMatrixArray()[bin];
	    Double_t xHi = fVarBinning->GetMatrixArray()[bin+1]-0.000001;
	    // axis limits
	    Int_t binLo = histPM->GetYaxis()->FindBin(xLo);
	    Int_t binHi = histPM->GetYaxis()->FindBin(xHi);
	    // found bin limits
	    Double_t fndLo = histPM->GetYaxis()->GetBinLowEdge(binLo);
	    Double_t fndHi = histPM->GetYaxis()->GetBinLowEdge(binHi+1);
	    //	    printf("binning requested, found of %s: %.3f-%.3f, %.3f-%.3f \n", fVar.Data(), xLo,xHi, fndLo,fndHi );

	    // fill array for signal extraction
	    for(Int_t ih=0; ih<histArr->GetEntriesFast(); ih++ ) {
	      TH2 *hist = (TH2*) histArr->UncheckedAt(ih);
	      tmpArr.Add( hist->ProjectionX(hist->GetTitle(), binLo,binHi, "e"));
	    }
	    //	    tmpArr.Print();

	    // process
	    sig->Process(&tmpArr);
	    if(gErrorIgnoreLevel<kWarning) sig->Print("");

	    // validate signal extraction
	    //	    if(sig->GetSignal() < 0.) continue;

	    // fill the tree
	    fExt->setup   = fInputKeys[i];
	    fExt->setupId = i;
	    fExt->var  = (fndHi-fndLo)/2+fndLo; // center of the found bin
	    fExt->varE = (fndHi-fndLo)/2;       // found bin width
	    fExt->s    = sig->GetSignal();
	    fExt->sE   = sig->GetSignalError();
	    fExt->b    = sig->GetBackground();
	    fExt->bE   = sig->GetBackgroundError();
	    fExt->sb   = sig->GetSB();
	    fExt->sbE  = sig->GetSBError();
	    fExt->sgn  = sig->GetSignificance();
	    fExt->sgnE = sig->GetSignificanceError();
	    fExt->HistSignal = NULL; //dynamic_cast<TH1F*>(sig->GetSignalHistogram());

	    fTree->Fill();

	  // set variable
	  // hf->AddCutVariable( (EValueTypes)
	  // 		     PairAnalysisVarManager::GetValueType(fVar.Data()),
	  // 		     fVarBinning->GetMatrixArray()[bin],
	  // 		     fVarBinning->GetMatrixArray()[bin+1]
	  // 		     );

	  // get histogram array
	  //...

	} //end binning

      } // end 2D

    }// end raw content

  //  fTree->Print();

}

void  PairAnalysisSpectrum::Draw(const char* varexp, const char* selection, Option_t* option)
{
  //
  // TTree draw alias
  //

  TString optString(option);
  optString.ToLower();
  printf("Plot spectrum: '%s' \t selection: '%s' \t options: '%s' \n", varexp, selection, optString.Data());
  Bool_t optLegFull  =optString.Contains("legf");      optString.ReplaceAll("legf","");
  Bool_t optLeg      =optString.Contains("leg");       optString.ReplaceAll("leg","");
  //  Bool_t optMeta     =optString.Contains("meta");      optString.ReplaceAll("meta","");
  Bool_t optNoMCtrue =optString.Contains("nomctrue");  optString.ReplaceAll("nomctrue","");
  Bool_t optMC       =optString.Contains("mc");        optString.ReplaceAll("mc","");

  // canvas key
  TString ckey(varexp);
  TObjArray *carr=ckey.Tokenize("+-*/:");
  carr->SetOwner();
  ckey = ((TObjString*)carr->At(0))->GetString();
  Int_t ndim = carr->GetEntriesFast();
  delete carr;

  // counter
  Long64_t n =1;

  // load style
  PairAnalysisStyler::LoadStyle();

  // canvas
  TCanvas *c=(TCanvas*)gROOT->FindObject(varexp);
  if(!c) c=new TCanvas(varexp,varexp);
  c->cd();

  // count number of drawn objects in pad
  TObject *obj;
  Int_t nobj=0;
  TList *prim = gPad->GetListOfPrimitives();
  //////    if(prim->GetSize()>1) prim->RemoveLast(); // remove redraw axis histogram
  for(Int_t io=0; io<prim->GetSize(); io++) {
    obj=prim->At(io);
    if(obj->InheritsFrom(TGraph::Class()) && obj!=prim->At(io+1)) nobj++;
  }

  TLegend *leg=0;
  if( (optLeg && !nobj) ) {
    //  if ( (optLeg && optTask && !nobj) || (optLeg && !optTask && !optDet) ) { 
    leg=new TLegend(0. + gPad->GetLeftMargin()  + gStyle->GetTickLength("Y"),
		    0. + gPad->GetBottomMargin()+ gStyle->GetTickLength("X"),
		    1. - gPad->GetRightMargin() + gStyle->GetTickLength("Y"),
		    1. - gPad->GetTopMargin()   + gStyle->GetTickLength("X"),
		    GetName(),"nbNDC");
  }
  else if(optLeg && nobj) {
    leg=(TLegend*)prim->FindObject("TPave");
  }

  Info("Draw","Basics: nobj: %d \t leg: %p",nobj,leg);

  // logaritmic style
  if(optString.Contains("logx")) gPad->SetLogx();
  if(optString.Contains("logy")) gPad->SetLogy();
  if(optString.Contains("logz")) gPad->SetLogz();
  optString.ReplaceAll("logx","");
  optString.ReplaceAll("logy","");
  optString.ReplaceAll("logz","");

  if(ndim<3) {    // build own histogram with labels

    //    printf("try to get errors for %d dimensional input \n",ndim);
    TString varkey(varexp);

    // get event list
    n = fTree->Draw(">>elist",selection);
    if(!n) return;
    TEventList *elist = (TEventList*)gDirectory->Get("elist");
    if(elist) {
      elist->SetReapplyCut(kTRUE); // important!
      //	elist->Print("all");
      fTree->SetEventList(elist);
    }

    TH1D *hist =0x0;
    // set up a proper histogram with labels
    if(varkey.Contains("setup")) {

      hist = new TH1D(); // activate buffer
      hist->SetNameTitle(varexp,selection);

      //read strings for all entries
      for (Long64_t i=0;i<n;i++) {
	fTree->GetEntry(i);
	hist->Fill((fExt->setup).Data(),1.);
	//	  printf(" read from tree: %s \n",(fExt->setup).Data());
      }

      hist->Draw("AXIS");
      hist->GetXaxis()->SetRange(1,n);
    }

    // get errors from tree
    if(ndim>1) varkey.ReplaceAll(ckey+":",ckey+":"+ckey+"E:");
    else       varkey.ReplaceAll(ckey,ckey+":"+ckey+"E");
    // execute tree draw command with graphics off to get errors
    //printf("execute collect/draw command for %s \n",varkey.Data());
    fTree->Draw(varkey, selection, "goff");

    // for one dimensional histograms
    Double_t *xval = new Double_t[fTree->GetSelectedRows()];
    for(Int_t ix=0; ix<fTree->GetSelectedRows(); ix++) xval[ix]=1.;

    // setup the final tgraph WITH error bars
    TGraphErrors *gr = 0x0;
    if(ndim>1) gr = new TGraphErrors(fTree->GetSelectedRows(),fTree->GetV3(),fTree->GetV1(),0,fTree->GetV2());
    else       gr = new TGraphErrors(fTree->GetSelectedRows(),fTree->GetV1(),xval,fTree->GetV2(),0);
    delete xval;

    PairAnalysisStyler::Style(gr,nobj);
    gr->SetName(Form("%s",selection));

    if(!gr) return;
    //      gr->Print();
    Info("Draw"," Draw object with options: '%s'",optString.Data());
    if(!PairAnalysisStyler::GetFirstHistogram())   gr->Draw((optString+"A").Data());
    else {
      gr->Draw((optString+"same").Data());

      // set axis maximum
      Double_t *valE = gr->GetEY();
      Double_t *val  = gr->GetY();
      Int_t npnts = gr->GetN();
      Int_t idx[1000];
      TMath::Sort(npnts,val,idx,kTRUE); // kFALSE=increasing numbers

      Double_t errmin = (TMath::IsNaN(valE[idx[npnts-1]]) ? 0. : valE[idx[npnts-1]] );
      Double_t min = (val[idx[npnts-1]]-errmin)*0.9;
      Double_t tmpmin = PairAnalysisStyler::GetFirstHistogram()->GetMinimum();
      PairAnalysisStyler::GetFirstHistogram()->SetMinimum( (tmpmin < min ? tmpmin : min) );
      Double_t errmax = (TMath::IsNaN(valE[idx[0]]) ? 0. : valE[idx[0]] );
      Double_t max = (val[idx[0]]+errmax)*1.1;
      Double_t tmpmax = PairAnalysisStyler::GetFirstHistogram()->GetMaximum();
      PairAnalysisStyler::GetFirstHistogram()->SetMaximum( (tmpmax > max ? tmpmax : max) );
      //	PairAnalysisStyler::GetFirstHistogram()->SetMaximum( val[idx[0]]*1.1 );
    }

    // legend
    TString legOpt = optString+"L";
    legOpt.ReplaceAll("hist","");
    legOpt.ReplaceAll("scat","");
    if(legOpt.Contains("col")) legOpt="";
    legOpt.ReplaceAll("z","");
    legOpt.ReplaceAll("e","");

    TString legkey = gr->GetName();
    if (leg) leg->AddEntry(gr,gr->GetName(),legOpt.Data());

  }
  else {
    // execute tree draw command
    fTree->Draw(varexp, selection, option);
  }


  // modify axis and titles
  //  printf("modify axis titles \n");
  TString var(varexp);
  TObjArray *arr=var.Tokenize(":");
  arr->SetOwner();
  TString xt="";
  TString yt="Entries";
  xt = ((TObjString*)arr->At(0))->GetString();
  if(xt.EqualTo("sb"))       xt=PairAnalysisSignalExt::GetValueName(3);
  else if(xt.EqualTo("s"))   xt=PairAnalysisSignalExt::GetValueName(0);
  else if(xt.EqualTo("b"))   xt=PairAnalysisSignalExt::GetValueName(1);
  else if(xt.EqualTo("sgn")) xt=PairAnalysisSignalExt::GetValueName(2);
  else if(xt.EqualTo("var")) {
    UInt_t varx = PairAnalysisVarManager::GetValueType(fVar.Data());
    xt=Form("%s %s",PairAnalysisVarManager::GetValueLabel(varx),PairAnalysisVarManager::GetValueUnit(varx));
  }

  if(arr->GetEntriesFast()<2)  {
    PairAnalysisStyler::GetFirstHistogram()->SetXTitle(xt.Data());
    PairAnalysisStyler::GetFirstHistogram()->SetYTitle(yt.Data());
  }
  else {
    PairAnalysisStyler::GetFirstHistogram()->SetYTitle(xt.Data());
    xt = ((TObjString*)arr->At(1))->GetString();
    if(xt.EqualTo("sb"))       xt=PairAnalysisSignalExt::GetValueName(3);
    else if(xt.EqualTo("s"))   xt=PairAnalysisSignalExt::GetValueName(0);
    else if(xt.EqualTo("b"))   xt=PairAnalysisSignalExt::GetValueName(1);
    else if(xt.EqualTo("sgn")) xt=PairAnalysisSignalExt::GetValueName(2);
    else if(xt.EqualTo("var")) {
      UInt_t varx = PairAnalysisVarManager::GetValueType(fVar.Data());
      xt=Form("%s %s",PairAnalysisVarManager::GetValueLabel(varx),PairAnalysisVarManager::GetValueUnit(varx));
    }
    PairAnalysisStyler::GetFirstHistogram()->SetXTitle(xt.Data());
  }

  // delete array
  delete arr;

  // set ndivisions
  if(fVarBinning)
    PairAnalysisStyler::GetFirstHistogram()->SetAxisRange(fVarBinning->Min(),fVarBinning->Max(),"X");
  //    PairAnalysisStyler::GetFirstHistogram()->GetXaxis()->SetNdivisions(, 0, 0, kFALSE);


  // legend
  if (leg) {
    PairAnalysisStyler::SetLegendAttributes(leg,optLegFull); // coordinates, margins, fillstyle, fontsize
    if(!nobj) leg->Draw(); // only draw the legend once
  }

  /// release eventlist
  fTree->SetEventList(0);


}


void PairAnalysisSpectrum::DrawSystRawYields()
{

  // load style
  PairAnalysisStyler::LoadStyle();

  // add canvas
  TCanvas *c=0;
  c=(TCanvas*)gROOT->FindObject(Form("cSignalExtraction"));
  if (!c) c=new TCanvas(Form("cSignalExtraction"),Form("SignalExtraction"));
  //    c->Clear();
  c->cd();

  Int_t nobj=0;
  TObject *obj;
  // count number of drawn objects in pad
  TList *prim = gPad->GetListOfPrimitives();
  for(Int_t io=0; io<prim->GetSize(); io++) {
    obj=prim->At(io);
    if(obj->InheritsFrom(TH1::Class()) && obj!=prim->At(io+1)) nobj++;
  }

  // add or get legend
  // TLegend *leg=0;
  // //  if ( (optLeg && optTask && !nobj) || (optLeg && !optTask) ) {
  //   leg=new TLegend(.75,.3,
  // 		    1.-gPad->GetTopMargin()-gStyle->GetTickLength("X"),
  // 		    1.-gPad->GetRightMargin()-gStyle->GetTickLength("Y"),
  // 		    fArrHists->GetName(),"nbNDC");
  //if(optTask) leg->SetHeader("");
  // }
  // else if(optLeg && nobj) {
  //   leg=(TLegend*)prim->FindObject("TPave");
  // }

  // logaritmic style
  // if(optString.Contains("logx")) gPad->SetLogx();
  // if(optString.Contains("logy")) gPad->SetLogy();
  // if(optString.Contains("logz")) gPad->SetLogz();
  // optString.ReplaceAll("logx","");
  // optString.ReplaceAll("logy","");
  // optString.ReplaceAll("logz","");

  //    Long64_t N = fTree->Draw("b:s:sE","","goff");


  // //    TGraphErrors *gr = new TGraphErrors(N,fTree->GetV1(),fTree->GetV2(),fTree->GetV3());
  // TGraphErrors *gr = new TGraphErrors(N,fTree->GetV1(),fTree->GetV2(),fTree->GetV3());
  // PairAnalysisStyler::Style(0);

  // gr->SetMarkerStyle(24);
  // gr->Print();
  // //    gr->Draw("alpe");
  // gr->Draw("lpe");

  // Long64_t N =     fTree->Draw("B:Division>>hDivision(3,0.,3.,200,0.,200.)","","goff");
  // TH2F *hDivision = (TH2F*)gDirectory->Get("hDivision");
  // hDivision->SetTitle("Average Cost per Division");
  // //    hDivision->LabelsOption(">"); //sort by decreasing bin contents

  // // hDivision->SetMaximum(13000);
  // // hDivision->SetMinimum(7000);
  // hDivision->SetStats(0);
  // hDivision->SetMarkerStyle(21);
  // hDivision->Draw();

  fTree->Draw("Extraction.b");
    


}

void PairAnalysisSpectrum::Write()
{
  //
  // write to output file
  //
  //  fTree->Print();
  TFile *fout = new TFile("test.root","RECREATE");
  fout->cd();
  fTree->Write();
  fout->Close();

}
