///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
// Authors:
//   * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
//   Julian Book   <Julian.Book@cern.ch>
/*

  Event mixing handler, that creates event pools of size fDepth. Events
  are ring-buffered and mixing is started for events that are similar according
  to the binnnings and variables specific via:

    AddVariable(PairAnalysisVarManager::ValueTypes type, TVectorD* const bins)

  You can specify with type of event mixing should be done (+-/-+,++,--) using:

    SetMixType(EMixType type)

*/
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include <TVectorD.h>
#include <TH1.h>
#include <TAxis.h>

#include "PairAnalysisTrack.h"

#include "PairAnalysis.h"
#include "PairAnalysisHelper.h"
//#include "PairAnalysisHistos.h"
#include "PairAnalysisMixedEvent.h"

#include "PairAnalysisMixingHandler.h"

ClassImp(PairAnalysisMixingHandler)

PairAnalysisMixingHandler::PairAnalysisMixingHandler() :
  TNamed(),
  fDepth(10),
  fArrPools("TClonesArray"),
  fAxes(kMaxCuts),
  fMixType(kOSonly),
  fMoveToSameVertex(kFALSE),
  fPID(0x0)
{
  //
  // Default Constructor
  //
  for (Int_t i=0; i<kMaxCuts; ++i){
    fEventCuts[i]=0;
  }
  fAxes.SetOwner(kTRUE);
}

//______________________________________________
PairAnalysisMixingHandler::PairAnalysisMixingHandler(const char* name, const char* title) :
  TNamed(name, title),
  fDepth(10),
  fArrPools("TClonesArray"),
  fAxes(kMaxCuts),
  fMixType(kOSonly),
  fMoveToSameVertex(kFALSE),
  fPID(0x0)
{
  //
  // Named Constructor
  //
  for (Int_t i=0; i<kMaxCuts; ++i){
    fEventCuts[i]=0;
  }
  fAxes.SetOwner(kTRUE);
}

//______________________________________________
PairAnalysisMixingHandler::~PairAnalysisMixingHandler()
{
  //
  // Default Destructor
  //
  fAxes.Delete();
  if(fPID) delete fPID;
}

//________________________________________________________________
void PairAnalysisMixingHandler::AddVariable(PairAnalysisVarManager::ValueTypes type,
                                             TVectorD * const bins)
{
  //
  // Add a variable to the mixing handler with arbitrary binning 'bins'
  //

  // limit number of variables to kMaxCuts
  if (fAxes.GetEntriesFast()>=kMaxCuts) return;

  Int_t size=fAxes.GetEntriesFast();
  fEventCuts[size]=(UShort_t)type;
  fAxes.Add(bins);
}

//______________________________________________
void PairAnalysisMixingHandler::Fill(const PairAnalysisEvent *ev, PairAnalysis *papa)
{
  //
  // fill event buffers and perform mixing if the pool depth is reached
  //

  //check if there are tracks available
  if (papa->GetTrackArray(0)->GetEntriesFast()==0 && papa->GetTrackArray(1)->GetEntriesFast()==0) return;

  TString dim;
  Int_t bin=FindBin(PairAnalysisVarManager::GetData(),&dim);

  //add mixing bin to event data
  PairAnalysisVarManager::SetValue(PairAnalysisVarManager::kMixingBin,bin);

  if (bin<0){
    Error("Fill","Bin outside range: %s",dim.Data());
    return;
  }

  // get mixing pool, create it if it does not yet exist.
  TClonesArray *poolp=static_cast<TClonesArray*>(fArrPools.At(bin));
  // do mixing
  if (poolp) {
    TClonesArray &pool=*poolp;
    DoMixing(pool,papa);
  }

  Int_t index1=0;

  if (!poolp){
    Info("Fill","New pool at %d (%s)",bin,dim.Data());
    poolp=new(fArrPools[bin]) TClonesArray("PairAnalysisMixedEvent",fDepth);
    poolp->SetUniqueID(0);
  } else {
    // one count further in the ring buffer
    index1=(poolp->GetUniqueID()+1)%fDepth;
  }

  TClonesArray &pool=*poolp;

  PairAnalysisMixedEvent *event=static_cast<PairAnalysisMixedEvent*>(pool.At(index1));
  if (!event){
    //    Info("Fill",Form("new event at %d: %d",bin,index1));
    event = new(pool[index1]) PairAnalysisMixedEvent();
    //    Int_t size = TMath::Max(papa->GetTrackArray(0)->GetEntriesFast(),papa->GetTrackArray(1)->GetEntriesFast()));
    event->Set();
    event->SetProcessID(fPID);
  } else {
    //    Info("Fill",Form("use event at %d: %d",bin,index1));
  }

  // event->SetProcessID(fPID);
  event->SetTracks(*papa->GetTrackArray(0), *papa->GetTrackArray(1));
  event->SetEventData(PairAnalysisVarManager::GetData());

  //set current event position in ring buffer
  pool.SetUniqueID(index1);
}

//______________________________________________
void PairAnalysisMixingHandler::DoMixing(TClonesArray &pool, PairAnalysis *papa)
{
  //
  // perform the mixing
  //

  // we need at least one event for mixing
  if (pool.GetEntriesFast()<1) return;
  //   printf("entries: %d\n",pool.GetEntriesFast());   //buffer track arrays and copy them back afterwards

  TObjArray arrTrDummy[4];
  for (Int_t i=0; i<4; ++i) arrTrDummy[i]=papa->fTracks[i];

  //buffer also global event data
  Double_t values[PairAnalysisVarManager::kNMaxValuesMC]={0};
  for (Int_t i=PairAnalysisVarManager::kPairMax; i<PairAnalysisVarManager::kNMaxValuesMC; ++i)
    values[i]=PairAnalysisVarManager::GetValue((PairAnalysisVarManager::ValueTypes)i);

  // The event data should alread be filled, since
  // all events are in the same mixing bin anyhow...
  TObject *o=0x0;
  TIter ev1P(&arrTrDummy[0]);
  TIter ev1N(&arrTrDummy[1]);

  //  for (Int_t i2=i1+1; i2<pool.GetEntriesFast(); ++i2){
  for (Int_t i1=0; i1<pool.GetEntriesFast(); ++i1){
    const PairAnalysisMixedEvent *ev2=static_cast<PairAnalysisMixedEvent*>(pool.At(i1));
    // don't mix with itself
    if (!ev2) continue;

    //clear arryas
    papa->fTracks[0].Clear();
    papa->fTracks[1].Clear();
    papa->fTracks[2].Clear();
    papa->fTracks[3].Clear();

    //setup track arrays
    ev1P.Reset();
    ev1N.Reset();
    TIter ev2P(ev2->GetTrackArrayP());
    TIter ev2N(ev2->GetTrackArrayN());
    //
    //move tracks to the same vertex (vertex of the first event), if requested
    //
    /*
      if (fMoveToSameVertex){
      const Double_t *varsFirst=values;
      const Double_t *varsMix=ev2->GetEventData();

      const Double_t vFirst[3]={varsFirst[AliDielectronVarManager::kXvPrim],
				varsFirst[AliDielectronVarManager::kYvPrim],
				varsFirst[AliDielectronVarManager::kZvPrim]};

      const Double_t vMix[3]  ={varsMix[AliDielectronVarManager::kXvPrim],
				varsMix[AliDielectronVarManager::kYvPrim],
				varsMix[AliDielectronVarManager::kZvPrim]};

      //loop over all tracks from the second event and move them to the vertex of the first
      AliVTrack *vtrack=0x0;
      while ( ( vtrack=(AliVTrack*)ev2P() ) ){
	MoveToSameVertex(vtrack, vFirst, vMix);
      }
      while ( ( vtrack=(AliVTrack*)ev2N() ) ){
	MoveToSameVertex(vtrack, vFirst, vMix);
      }

      ev2P.Reset();
      ev2N.Reset();
    }
    */

    //mixing of ev1- ev2 (pair type4). This is common for all mixing types
    while ( (o=ev1N()) ) papa->fTracks[1].Add(o);
    while ( (o=ev2P()) ) papa->fTracks[2].Add(o);
    papa->FillPairArrays(1,2);

    if (fMixType==kAll || fMixType==kOSandLS){
      // all 4 pair arrays will be filled
      while ( (o=ev1P()) ) papa->fTracks[0].Add(o);
      while ( (o=ev2N()) ) papa->fTracks[3].Add(o);
      papa->FillPairArrays(0,2);
      papa->FillPairArrays(1,3);
      if (fMixType==kAll) papa->FillPairArrays(0,3);
    }

    if (fMixType==kOSonly || fMixType==kOSandLS){
      //use the pair type of ev1- ev1 also for ev1 ev1-
      papa->fTracks[1].Clear();
      papa->fTracks[2].Clear();
      while ( (o=ev1P()) ) papa->fTracks[1].Add(o);
      while ( (o=ev2N()) ) papa->fTracks[2].Add(o);
      papa->FillPairArrays(1,2);
    }
  }

  //copy back the tracks
  for (Int_t i=0; i<4; ++i) {
    papa->fTracks[i].Clear();
    papa->fTracks[i]=arrTrDummy[i];
  }

  //set back global event values
  PairAnalysisVarManager::SetEventData(values);
}

//______________________________________________
void PairAnalysisMixingHandler::Init(const PairAnalysis *papa)
{
  //
  // initialise event buffers
  //
  Int_t size=GetNumberOfBins();

  Info("Init","Creating a pool array with size %d",size);
  if(papa && papa->DoEventProcess()) fArrPools.Expand(size);

  TString values;
  for (Int_t i=0; i<fAxes.GetEntriesFast(); ++i){
    TVectorD *bins=static_cast<TVectorD*>(fAxes.At(i));
    Int_t nRows=bins->GetNrows();
    values+=Form("%s: ",PairAnalysisVarManager::GetValueName(fEventCuts[i]));
    for (Int_t irow=0; irow<nRows; ++irow){
      values+=Form("%.2f, ",(*bins)[irow]);
    }
  }

  if (!fPID){
    fPID=TProcessID::AddProcessID();
  }

  Info("Init","%s",values.Data());
}

//______________________________________________
Int_t PairAnalysisMixingHandler::GetNumberOfBins() const
{
  //
  // return the number of bins this mixing handler has
  //
  Int_t size=1;
  for (Int_t i=0; i<fAxes.GetEntriesFast(); ++i)
    size*=((static_cast<TVectorD*>(fAxes.At(i)))->GetNrows()-1);
  return size;
}

//______________________________________________
Int_t PairAnalysisMixingHandler::FindBin(const Double_t values[], TString *dim)
{
  //
  // bin bin in mixing stack described by 'values'
  // if the values are outside the binning range -1 is returned
  // if dim is non NULL debug info will be stored in the variable
  //

  if (fAxes.GetEntriesFast()==0) {
    if (dim) (*dim)="single bin";
    return 0;
  }
  if (dim) (*dim)="";
  Int_t sizeAdd=1;
  Int_t bin=0;
  for (Int_t i=0; i<fAxes.GetEntriesFast(); ++i){
    Double_t val=values[fEventCuts[i]];
    TVectorD *bins=static_cast<TVectorD*>(fAxes.At(i));
    Int_t nRows=bins->GetNrows();
    if ( (val<(*bins)[0]) || (val>(*bins)[nRows-1]) ) {
      return -1;
    }

    Int_t pos=TMath::BinarySearch(nRows,bins->GetMatrixArray(),val);
    bin+=sizeAdd*pos;
    if (dim) (*dim)+=Form("%s: %f (%d); ",PairAnalysisVarManager::GetValueName(fEventCuts[i]),val,pos);
    sizeAdd*=(nRows-1);
  }

  return bin;
}

//______________________________________________
void PairAnalysisMixingHandler::MoveToSameVertex(PairAnalysisTrack * const vtrack, const Double_t *vFirst, const Double_t* vMix)
{
  //
  // move 'track' which belongs to the vertex information of vMix to the vertex of vFirst
  //
  return;
  /*
  static Bool_t printed=kFALSE;
  
  if (vtrack->IsA()==AliESDtrack::Class()){
    AliESDtrack *track=(AliESDtrack*)vtrack;

    //get track information
    Double_t x        = track->GetX();
    Double_t alpha    = track->GetAlpha();
    Double_t param[5] = {0};
    Double_t cov[15]  = {0};

    for (Int_t i=0; i<5;  ++i) param[i]=track->GetParameter()[i];
    for (Int_t i=0; i<15; ++i) cov[i]  =track->GetCovariance()[i];

    //translation
    Double_t vt[3] = {vMix[0]-vFirst[0],vMix[1]-vFirst[1],vMix[2]-vFirst[2]};
    //rotate to the track frame
//     track->Global2LocalPosition(vt,track->GetAlpha());

    //add to track position
//     x        = x       -vt[0];
//     param[0] = param[0]-vt[1];
//     param[1] = param[1]-vt[2];
    param[1] = param[1]-vt[2];
    
    //set updated track information
    track->Set(x, alpha, param, cov);
  } else {
    //             AliAODTrack *track=(AliAODTrack*)vtrack;
    //             Double_t pos[3]={0};
    //             track->GetPosition(pos);
    //             if (pos[0]>-999.){
      //               pos[0]=pos[0]-vMix[PairAnalysisVarManager::kXvPrim]+vFirst[PairAnalysisVarManager::kXvPrim];
      //               pos[1]=pos[1]-vMix[PairAnalysisVarManager::kYvPrim]+vFirst[PairAnalysisVarManager::kYvPrim];
      //               pos[2]=pos[2]-vMix[PairAnalysisVarManager::kZvPrim]+vFirst[PairAnalysisVarManager::kZvPrim];
      //               track->SetPosition(pos);
//       AliError("Move To same vertex not yet implemented for AOD!");
    if (!printed) {
//       Error("PairAnalysisMixingHandler::MoveToSameVertex","Move To same vertex not yet implemented for AOD!");
      printed=kTRUE;
    }
      //
      // Not that clear how to do it. In AOD track there is fPosition, fPositionAtDCA and "TRef fProdVertex"
      // where Xv(), Yv(), Zv() returns the position of fProdVertex
      //
    }
  */
}
