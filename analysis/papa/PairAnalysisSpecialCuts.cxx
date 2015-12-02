///////////////////////////////////////////////////////////////////////////
//         Class PairAnalysisSpecialCuts
//         Provide cuts for using objects
// Authors:                                                              //
//   Julian Book   <Julian.Book@cern.ch>                                 //
/*



*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////


#include <TAxis.h>
#include <TFormula.h>
#include <TGraph.h>
#include <THnBase.h>
#include <TSpline.h>

#include "PairAnalysisSpecialCuts.h"
#include "PairAnalysisMC.h"
#include "PairAnalysisHelper.h"

ClassImp(PairAnalysisSpecialCuts)


PairAnalysisSpecialCuts::PairAnalysisSpecialCuts() :
  AnalysisCuts(),
  fUsedVars(new TBits(PairAnalysisVarManager::kNMaxValuesMC)),
  fNActiveCuts(0),
  fActiveCutsMask(0),
  fSelectedCutsMask(0),
  fCutOnMCtruth(kFALSE),
  fCutType(kAll)
{
  //
  // Default costructor
  //
  for (Int_t i=0; i<PairAnalysisSpecialCuts::kNMaxCuts; ++i){
    fActiveCuts[i]=0;
    fCutExclude[i]=kFALSE;
    fCutMin[i]=0x0;
    fCutMax[i]=0x0;
    fVarFormula[i]=0x0;
  }
  PairAnalysisVarManager::InitFormulas();
}

//________________________________________________________________________
PairAnalysisSpecialCuts::PairAnalysisSpecialCuts(const char* name, const char* title) :
  AnalysisCuts(name,title),
  fUsedVars(new TBits(PairAnalysisVarManager::kNMaxValuesMC)),
  fNActiveCuts(0),
  fActiveCutsMask(0),
  fSelectedCutsMask(0),
  fCutOnMCtruth(kFALSE),
  fCutType(kAll)
{
  //
  // Named contructor
  //
  for (Int_t i=0; i<PairAnalysisSpecialCuts::kNMaxCuts; ++i){
    fActiveCuts[i]=0;
    fCutExclude[i]=kFALSE;
    fCutMin[i]=0x0;
    fCutMax[i]=0x0;
    fVarFormula[i]=0x0;
  }
  PairAnalysisVarManager::InitFormulas();
}

//________________________________________________________________________
PairAnalysisSpecialCuts::~PairAnalysisSpecialCuts()
{
  //
  // Destructor
  //
  if (fUsedVars) delete fUsedVars;
  for (Int_t i=0; i<PairAnalysisSpecialCuts::kNMaxCuts; ++i){
    fActiveCuts[i]=0;
    if(fCutMin[i]) delete fCutMin[i];
    if(fCutMax[i]) delete fCutMax[i];
    if(fVarFormula[i]) delete fVarFormula[i];
  }
}

//________________________________________________________________________
Bool_t PairAnalysisSpecialCuts::IsSelected(TObject* track)
{
  //
  // Make cut decision
  //

  //reset
  fSelectedCutsMask=0;
  SetSelected(kFALSE);

  if (!track) return kFALSE;

  //If MC cut, get MC truth
  if (fCutOnMCtruth){
    PairAnalysisTrack *part=static_cast<PairAnalysisTrack*>(track);
    track=PairAnalysisMC::Instance()->GetMCTrackFromMCEvent(part->GetLabel());
    if (!track) return kFALSE;
  }
  
  //Fill values
  Double_t values[PairAnalysisVarManager::kNMaxValuesMC];
  PairAnalysisVarManager::SetFillMap(fUsedVars);
  PairAnalysisVarManager::Fill(track,values);

  for (Int_t iCut=0; iCut<fNActiveCuts; ++iCut){
    Int_t cut=fActiveCuts[iCut];
    SETBIT(fSelectedCutsMask,iCut);

    // variable you want to cut on
    Double_t compValue = 0.;
    if(fVarFormula[iCut])   compValue = PairAnalysisHelper::EvalFormula(fVarFormula[iCut], values);
    else                    compValue = values[cut];

    // cut limits
    Double_t cutMin = -9.e30;
    Double_t cutMax = +9.e30;


    /// use a THnBase inherited cut object //
    if ( (fCutMin[iCut] && fCutMin[iCut]->IsA() == THnBase::Class()) ||
	 (fCutMax[iCut] && fCutMax[iCut]->IsA() == THnBase::Class())   ) {

	THnBase *hn = static_cast<THnBase*>(fCutMin[iCut]);
	if(fCutMin[iCut]) {
	  Double_t *vals = new Double_t[hn->GetNdimensions()];//={-1};
	  // get array of values for the corresponding dimensions using axis names
	  for(Int_t idim=0; idim<hn->GetNdimensions(); idim++) {
	    vals[idim] = values[PairAnalysisVarManager::GetValueType(hn->GetAxis(idim)->GetName())]; /// TODO: what about formulas in the axis
	  }
	  // find bin for values (w/o creating it in case it is not filled)
	  Long_t bin = hn->GetBin(vals,kFALSE);
	  if(bin>0)  cutMin = hn->GetBinContent(bin);
	  delete [] vals;
	}

	THnBase *hx = static_cast<THnBase*>(fCutMax[iCut]);
	if(fCutMax[iCut]) {
	  Double_t *vals = new Double_t[hx->GetNdimensions()];//={-1};
	  // get array of values for the corresponding dimensions using axis names
	  for(Int_t idim=0; idim<hx->GetNdimensions(); idim++) {
	    vals[idim] = values[PairAnalysisVarManager::GetValueType(hx->GetAxis(idim)->GetName())]; /// TODO: what about formulas in the axis
	  }
	  // find bin for values (w/o creating it in case it is not filled)
	  Long_t bin = hx->GetBin(vals,kFALSE);
	  if(bin>0)  cutMax = hx->GetBinContent(bin);
	  delete [] vals;
	}

      }
    else if ( (fCutMin[iCut] && fCutMin[iCut]->IsA() == TFormula::Class()) ||
	      (fCutMax[iCut] && fCutMax[iCut]->IsA() == TFormula::Class()) ) {
      /// use formulas for the cut //
      TFormula *formN = static_cast<TFormula*>(fCutMin[iCut]);
      if(formN)  cutMin   = PairAnalysisHelper::EvalFormula(formN,values);

      TFormula *formM = static_cast<TFormula*>(fCutMax[iCut]);
      if(formM)  cutMax   = PairAnalysisHelper::EvalFormula(formM,values);

    }
    else if ( (fCutMin[iCut] && fCutMin[iCut]->IsA() == TSpline3::Class()) ||
	      (fCutMax[iCut] && fCutMax[iCut]->IsA() == TSpline3::Class()) ) {
      /// use spline/graph for the cut //
      TSpline3 *splineN = static_cast<TSpline3*>(fCutMin[iCut]);
      if(splineN)  cutMin   = splineN->Eval(compValue);

      TSpline3 *splineM = static_cast<TSpline3*>(fCutMax[iCut]);
      if(splineM)  cutMax   = splineM->Eval(compValue);
    }


    // apply cut
    if ( ((compValue<cutMin) || (compValue>cutMax))^fCutExclude[iCut] ) CLRBIT(fSelectedCutsMask,iCut);

    // cut type and decision
    if ( fCutType==kAll && !TESTBIT(fSelectedCutsMask,iCut) ) return kFALSE; // option to (minor) speed improvement
  }

  Bool_t isSelected=(fSelectedCutsMask==fActiveCutsMask);
  if ( fCutType==kAny ) isSelected=(fSelectedCutsMask>0);
  SetSelected(isSelected);
  return isSelected;
}

//________________________________________________________________________
void PairAnalysisSpecialCuts::AddCut(PairAnalysisVarManager::ValueTypes type, const char *formulaMin, const char *formulaMax, Bool_t excludeRange)
{
  //
  // Set cut range and activate it
  //
  fCutMin[fNActiveCuts]=PairAnalysisHelper::GetFormula(formulaMin, formulaMin);
  fCutMax[fNActiveCuts]=PairAnalysisHelper::GetFormula(formulaMax, formulaMax);
  fCutExclude[fNActiveCuts]=excludeRange;
  SETBIT(fActiveCutsMask,fNActiveCuts);
  fActiveCuts[fNActiveCuts]=(UShort_t)type;
  fUsedVars->SetBitNumber(type,kTRUE);
  ++fNActiveCuts;
}

//________________________________________________________________________
void PairAnalysisSpecialCuts::AddCut(const char *formula, const char *formulaMin, const char *formulaMax, Bool_t excludeRange)
{
  //
  // Set cut range and activate it
  //
  fCutMin[fNActiveCuts]=PairAnalysisHelper::GetFormula(formulaMin, formulaMin);
  fCutMax[fNActiveCuts]=PairAnalysisHelper::GetFormula(formulaMax, formulaMax);
  fCutExclude[fNActiveCuts]=excludeRange;
  SETBIT(fActiveCutsMask,fNActiveCuts);
  fActiveCuts[fNActiveCuts]=(UShort_t)PairAnalysisVarManager::kNMaxValuesMC;
  fVarFormula[fNActiveCuts]=PairAnalysisHelper::GetFormula(formula, formula);
  ++fNActiveCuts;
}

//________________________________________________________________________
void PairAnalysisSpecialCuts::AddCut(PairAnalysisVarManager::ValueTypes type, TGraph *const graphMin, TGraph *const graphMax, Bool_t excludeRange)
{
  //
  // Set cut range and activate it
  //
  if(graphMin) {
    TSpline3 *splineMin = new TSpline3("splineMin",graphMin); //spline w/o begin and end point conditions
    fCutMin[fNActiveCuts]=splineMin;
  }
  if(graphMax) {
    TSpline3 *splineMax = new TSpline3("splineMax",graphMax); //spline w/o begin and end point conditions
    fCutMax[fNActiveCuts]=splineMax;
  }
  fCutExclude[fNActiveCuts]=excludeRange;
  SETBIT(fActiveCutsMask,fNActiveCuts);
  fActiveCuts[fNActiveCuts]=(UShort_t)type;
  fUsedVars->SetBitNumber(type,kTRUE);
  ++fNActiveCuts;
}

//________________________________________________________________________
void PairAnalysisSpecialCuts::AddCut(const char *formula, TGraph *const graphMin, TGraph *const graphMax, Bool_t excludeRange)
{
  //
  // Set cut range and activate it
  //
  if(graphMin) {
    TSpline3 *splineMin = new TSpline3("splineMin",graphMin); //spline w/o begin and end point conditions
    fCutMin[fNActiveCuts]=splineMin;
  }
  if(graphMax) {
    TSpline3 *splineMax = new TSpline3("splineMax",graphMax); //spline w/o begin and end point conditions
    fCutMax[fNActiveCuts]=splineMax;
  }
  fCutExclude[fNActiveCuts]=excludeRange;
  SETBIT(fActiveCutsMask,fNActiveCuts);
  fActiveCuts[fNActiveCuts]=(UShort_t)PairAnalysisVarManager::kNMaxValuesMC;
  fVarFormula[fNActiveCuts]=PairAnalysisHelper::GetFormula(formula, formula);
  ++fNActiveCuts;
}

//________________________________________________________________________
void PairAnalysisSpecialCuts::AddCut(PairAnalysisVarManager::ValueTypes type, THnBase *const histMin, THnBase *const histMax, Bool_t excludeRange)
{
  //
  // Set cut range and activate it
  //
  fCutMin[fNActiveCuts]=histMin;
  fCutMax[fNActiveCuts]=histMax;
  fCutExclude[fNActiveCuts]=excludeRange;
  SETBIT(fActiveCutsMask,fNActiveCuts);
  fActiveCuts[fNActiveCuts]=(UShort_t)type;
  fUsedVars->SetBitNumber(type,kTRUE);
  ++fNActiveCuts;
}

//________________________________________________________________________
void PairAnalysisSpecialCuts::AddCut(const char *formula, THnBase *const histMin, THnBase *const histMax, Bool_t excludeRange)
{
  //
  // Set cut range and activate it
  //
  fCutMin[fNActiveCuts]=histMin;
  fCutMax[fNActiveCuts]=histMax;
  fCutExclude[fNActiveCuts]=excludeRange;
  SETBIT(fActiveCutsMask,fNActiveCuts);
  fActiveCuts[fNActiveCuts]=(UShort_t)PairAnalysisVarManager::kNMaxValuesMC;
  fVarFormula[fNActiveCuts]=PairAnalysisHelper::GetFormula(formula, formula);
  ++fNActiveCuts;
}

//________________________________________________________________________
void PairAnalysisSpecialCuts::Print(const Option_t* /*option*/) const
{
  //
  // Print cuts and the range
  //
  printf("cut ranges for '%s'\n",GetTitle());
  if (fCutType==kAll){
    printf("All Cuts have to be fulfilled\n");
  } else {
    printf("Any Cut has to be fulfilled\n");
  }

  printf("special cut class - TODO: implement print functionality\n");

  /*
  for (Int_t iCut=0; iCut<fNActiveCuts; ++iCut){
    Int_t cut=(Int_t)fActiveCuts[iCut];
    Bool_t inverse=fCutExclude[iCut];
    Bool_t bitcut=fBitCut[iCut];
    Bool_t hncut=(fUpperCut[iCut]&&fUpperCut[iCut]->IsA()==THnBase::Class());
    Bool_t fcut=(fUpperCut[iCut]&&fUpperCut[iCut]->IsA()==TFormula::Class());

    if(hncut) {
      TString dep="";
      THnBase *hn = static_cast<THnBase*>(fUpperCut[iCut]);
      for(Int_t idim=0; idim<hn->GetNdimensions(); idim++)
	dep+=Form("%s%s",(idim?",":""),hn->GetAxis(idim)->GetName());

      if (!inverse){
	printf("Cut %02d: %f < %s < obj(%s)\n", iCut,
	       fCutMin[iCut], PairAnalysisVarManager::GetValueName((Int_t)cut), dep.Data());
      } else {
	printf("Cut %02d: !(%f < %s < obj(%s))\n", iCut,
	       fCutMin[iCut], PairAnalysisVarManager::GetValueName((Int_t)cut), dep.Data());
      }
    }
    else if(fcut) {
      // variable defined by a formula
      TFormula *form = static_cast<TFormula*>(fUpperCut[iCut]);
      TString tit(form->GetExpFormula());
      // replace parameter variables with names labels
      for(Int_t j=0;j<form->GetNpar();j++) tit.ReplaceAll(Form("[%d]",j),form->GetParName(j));
      if (!inverse) printf("Cut %02d: %f < %s < %f\n", iCut, fCutMin[iCut], tit.Data(), fCutMax[iCut]);
      else          printf("Cut %02d: !(%f < %s < %f)\n", iCut, fCutMin[iCut], tit.Data(), fCutMax[iCut]);
    }
    else
      printf("cut class not found\n");
  } //loop over cuts
  */
}
