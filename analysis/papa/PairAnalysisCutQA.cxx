/*************************************************************************
 * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
 **************************************************************************/

//////////////////////////////////////////////////////////////////////////
//                           CutQA                                      //
//                                                                      //
/*
   Allow to monitor how many tracks,pair,events pass the selection criterion 
   in any of the cuts added to the corresponding filters. All you need to 
   add to your config is the following:

   papa->SetCutQA();


*/
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "PairAnalysisCutQA.h"

#include <TList.h>
#include <TCollection.h>
#include <TVectorD.h>

#include "PairAnalysisCutGroup.h"
#include "AnalysisCuts.h"

#include "PairAnalysisEvent.h"
//#include "AliVParticle.h"
#include "PairAnalysisTrack.h"
#include "PairAnalysisPair.h"

#include "PairAnalysisHelper.h"


ClassImp(PairAnalysisCutQA)


PairAnalysisCutQA::PairAnalysisCutQA() :
  TNamed(),
  fQAHistArray()
{
  //
  // Default constructor
  //
  for(Int_t itype=0; itype<kNtypes; itype++) {
    fCutQA[itype]=0x0;
    fPdgCutQA[itype]=0x0;
    fNCuts[itype]=1;
    for(Int_t i=0; i<20; i++) {
      fCutNames[i][itype]="";
    }
  }
  fTypeKeys[kTrack]  = "Track";
  fTypeKeys[kTrack2] = "Track2";
  fTypeKeys[kPair]   = "Pair";
  fTypeKeys[kEvent]  = "Event";
  fQAHistArray.SetOwner();
}

//_____________________________________________________________________
PairAnalysisCutQA::PairAnalysisCutQA(const char* name, const char* title) :
  TNamed(name, title),
  fQAHistArray()
{
  //
  // Named Constructor
  //
  for(Int_t itype=0; itype<kNtypes; itype++) {
    fCutQA[itype]=0x0;
    fPdgCutQA[itype]=0x0;
    fNCuts[itype]=1;
    for(Int_t i=0; i<20; i++) {
      fCutNames[i][itype]="";
    }
  }
  fTypeKeys[kTrack]  = "Track";
  fTypeKeys[kTrack2] = "Track2";
  fTypeKeys[kPair]   = "Pair";
  fTypeKeys[kEvent]  = "Event";
  fQAHistArray.SetOwner();
}

//_____________________________________________________________________
PairAnalysisCutQA::~PairAnalysisCutQA() 
{
  //
  //Default Destructor
  //
  fQAHistArray.Delete();
}

//_____________________________________________________________________
void PairAnalysisCutQA::Init()
{

  fQAHistArray.SetName(Form("%s",GetName()));

  // const TVectorD *binsPdg = PairAnalysisHelper::MakeArbitraryBinning("-2212,-321,-211,-13,-11,11,13,211,321,2212");
  const TVectorD *binsPdg = PairAnalysisHelper::MakeLinBinning(5,0,5);
  // loop over all types
  for(Int_t itype=0; itype<kNtypes; itype++) {
    //    printf("\n type: %d\n",itype);
    fCutNames[0][itype]="no cuts";

    const TVectorD *binsX = PairAnalysisHelper::MakeLinBinning(fNCuts[itype],0,fNCuts[itype]);
    // create histogram based on added cuts
    fCutQA[itype] = new TH1I(fTypeKeys[itype],
			     Form("%sQA;cuts;# passed %ss",fTypeKeys[itype],fTypeKeys[itype]), 			     fNCuts[itype], binsX->GetMatrixArray());

    if(itype==kTrack || itype==kTrack2) {
      fPdgCutQA[itype] = new TH2I(Form("%sPDG",fTypeKeys[itype]),
				  Form("%sQA;cuts;PDG code;# passed %ss",
				       fTypeKeys[itype],fTypeKeys[itype]),
				  fNCuts[itype],binsX->GetMatrixArray(),
				  binsPdg->GetNrows()-1,binsPdg->GetMatrixArray() );

    }

    // delete surplus
    delete binsX;

    // Set labels to histograms
    // loop over all cuts
    for(Int_t i=0; i<fNCuts[itype]; i++) {
      fCutQA[itype]->GetXaxis()->SetBinLabel(i+1,fCutNames[i][itype]);
      if(fPdgCutQA[itype]) fPdgCutQA[itype]->GetXaxis()->SetBinLabel(i+1,fCutNames[i][itype]);
      //      printf(" %s \n",fCutNames[i][itype]);
    }

    // pdg label
    if(fPdgCutQA[itype]) {
      TString pdglbl="";
      for(Int_t i=0; i<binsPdg->GetNrows()-1; i++) {
	switch(i+1) {
	case 1:  pdglbl="electron"; break; // electron
	case 2:  pdglbl="muon";     break; // muon
	case 3:  pdglbl="pion";     break; // pion
	case 4:  pdglbl="kaon";     break; // kaon
	case 5:  pdglbl="proton";   break; // proton
	}
	fPdgCutQA[itype]->GetYaxis()->SetBinLabel(i+1,pdglbl.Data());
      }
    }

    // add to output array
    fQAHistArray.AddLast(fCutQA[itype]);
    if(fPdgCutQA[itype]) fQAHistArray.AddLast(fPdgCutQA[itype]);
  }

  // delete surplus
  delete binsPdg;

}

//_____________________________________________________________________
void PairAnalysisCutQA::AddTrackFilter(AnalysisFilter *trackFilter)
{
  //
  // add track filter cuts to the qa histogram
  //


  TIter listIterator(trackFilter->GetCuts());
  while (AnalysisCuts *thisCut = (AnalysisCuts*) listIterator()) {
    Bool_t addCut=kTRUE;

    // add new cut class to the array
    if(addCut) {
      fCutNames[fNCuts[kTrack]][kTrack]=thisCut->GetTitle();
      //      printf("add cut %s to %d \n",thisCut->GetTitle(),fNCuts[kTrack]);
      fNCuts[kTrack]++;
    }

  } // pair filter loop

}

//_____________________________________________________________________
void PairAnalysisCutQA::AddTrackFilter2(AnalysisFilter *trackFilter)
{
  //
  // add track filter cuts to the qa histogram
  //
  if(!trackFilter) return;

  TIter listIterator(trackFilter->GetCuts());
  while (AnalysisCuts *thisCut = (AnalysisCuts*) listIterator()) {
    Bool_t addCut=kTRUE;

    // add new cut class to the array
    if(addCut) {
      fCutNames[fNCuts[kTrack2]][kTrack2]=thisCut->GetTitle();
      //      printf("add cut %s to %d \n",thisCut->GetTitle(),fNCuts[kTrack]);
      fNCuts[kTrack2]++;
    }

  } // pair filter loop

}


//_____________________________________________________________________
void PairAnalysisCutQA::AddPairFilter(AnalysisFilter *pairFilter)
{
  //
  // add track filter cuts to the qa histogram
  //

  TIter listIterator(pairFilter->GetCuts());
  while (AnalysisCuts *thisCut = (AnalysisCuts*) listIterator()) {
    Bool_t addCut=kTRUE;

    // add new cut class to the array
    if(addCut) {
      fCutNames[fNCuts[kPair]][kPair]=thisCut->GetTitle();
      //  printf("add cut %s to %d \n",thisCut->GetTitle(),fNCuts[kPair]);
      fNCuts[kPair]++;
    }

  } // trk filter loop

}

//_____________________________________________________________________
void PairAnalysisCutQA::AddPrePairFilter(AnalysisFilter *pairFilter)
{
  //
  // add track filter cuts to the qa histogram
  //
  if(!pairFilter) return;

  TIter listIterator(pairFilter->GetCuts());
  while (AnalysisCuts *thisCut = (AnalysisCuts*) listIterator()) {
    Bool_t addCut=kTRUE;

    // add new cut class to the array
    if(addCut) {
      fCutNames[fNCuts[kPrePair]][kPrePair]=thisCut->GetTitle();
      //  printf("add cut %s to %d \n",thisCut->GetTitle(),fNCuts[kPair]);
      fNCuts[kPrePair]++;
    }

  } // trk filter loop

}

//_____________________________________________________________________
void PairAnalysisCutQA::AddEventFilter(AnalysisFilter *eventFilter)
{
  //
  // add track filter cuts to the qa histogram
  //


  TIter listIterator(eventFilter->GetCuts());
  while (AnalysisCuts *thisCut = (AnalysisCuts*) listIterator()) {
    Bool_t addCut=kTRUE;

    // add new cut class to the array
    if(addCut) {
      fCutNames[fNCuts[kEvent]][kEvent]=thisCut->GetTitle();
      //      printf("add cut %s to %d \n",thisCut->GetTitle(),fNCuts[kEvent]);
      fNCuts[kEvent]++;
    }

  } // trk filter loop

}

//_____________________________________________________________________
void PairAnalysisCutQA::Fill(UInt_t mask, TObject *obj, UInt_t addIdx)
{
  //
  // fill the corresponding step in the qa histogram
  //

  UInt_t idx = GetObjIndex(obj)+addIdx;

  // pdg to pdg label
  Int_t pdg = 0;
  TString pdglbl="";
  if(idx==kTrack || idx==kTrack2) {
    pdg = (Int_t) (static_cast<PairAnalysisTrack*>(obj)->PdgCode());
    switch(TMath::Abs(pdg)) {
    case 11:  pdglbl="electron"; break; // electron
    case 13:  pdglbl="muon";     break; // muon
    case 211: pdglbl="pion";     break; // pion
    case 321: pdglbl="kaon";     break; // kaon
    case 2212:pdglbl="proton";   break; // proton
    }
  }

  // loop over cutmask and check decision
  Int_t cutstep=1;
  for (Int_t iCut=0; iCut<fNCuts[idx]-1;++iCut) {
    //    UInt_t cutMask=1<<iCut;         // for each cut
    UInt_t cutMask=(1<<(iCut+1))-1; // increasing cut match

    // passed
    if ((mask&cutMask)==cutMask) {
      fCutQA[idx]->Fill(cutstep);
      if(pdg) fPdgCutQA[idx]->Fill(cutstep, pdglbl.Data(),1.);
      ++cutstep;
    }

  }

}

//_____________________________________________________________________
void PairAnalysisCutQA::FillAll(TObject *obj, UInt_t addIdx)
{
  //
  // fill the corresponding step in the qa histogram
  //

  UInt_t idx = GetObjIndex(obj)+addIdx;

  // pdg to pdg label
  Int_t pdg = 0;
  TString pdglbl="";
  if(idx==kTrack || idx==kTrack2) {
    pdg = (Int_t) (static_cast<PairAnalysisTrack*>(obj)->PdgCode());
    switch(TMath::Abs(pdg)) {
    case 11:  pdglbl="electron"; break; // electron
    case 13:  pdglbl="muon";     break; // muon
    case 211: pdglbl="pion";     break; // pion
    case 321: pdglbl="kaon";     break; // kaon
    case 2212:pdglbl="proton";   break; // proton
    }
  }

  // fill
  fCutQA[idx]->Fill(0);
  if(pdg) fPdgCutQA[idx]->Fill(0., pdglbl.Data(),1);


}

//______________________________________________________________________
UInt_t PairAnalysisCutQA::GetObjIndex(TObject *obj)
{
  //
  // return the corresponding idex
  //
  //  printf("INFO: object type is a %s \n", obj->IsA()->GetName());
  if(obj->InheritsFrom(PairAnalysisPair::Class())    )  return kPair;
  if(obj->InheritsFrom(PairAnalysisTrack::Class())   )  return kTrack;
  if(obj->InheritsFrom(PairAnalysisEvent::Class())   )  return kEvent;
  //  printf("FATAL: object type %s not yet supported, please let the author know\n", obj->IsA()->GetName());
  return -1;

}




