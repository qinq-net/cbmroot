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

#include "CbmDetectorList.h"
#include "CbmMCTrack.h"

#include "PairAnalysisCutGroup.h"
#include "AnalysisCuts.h"

#include "PairAnalysisEvent.h"
#include "PairAnalysisTrack.h"
#include "PairAnalysisPairLV.h"

#include "PairAnalysisHelper.h"


ClassImp(PairAnalysisCutQA)


PairAnalysisCutQA::PairAnalysisCutQA() :
  TNamed(),
  fQAHistList()
{
  //
  // Default constructor
  //
  for(Int_t itype=0; itype<kNtypes; itype++) {
    fNCuts[itype]=1;
    for(Int_t i=0; i<20; i++) {
      fCutNames[i][itype]="";
    }
  }
  fTypeKeys[kTrack]  = "Track";
  fTypeKeys[kTrack2] = "Track2";
  fTypeKeys[kTrackMC]= "MCTrack";
  fTypeKeys[kPair]   = "Pair";
  fTypeKeys[kPrePair]= "Pair0";
  fTypeKeys[kEvent]  = "Event";
  fQAHistList.SetOwner(kFALSE);

}

//_____________________________________________________________________
PairAnalysisCutQA::PairAnalysisCutQA(const char* name, const char* title) :
  TNamed(name, title),
  fQAHistList()
{
  //
  // Named Constructor
  //
  for(Int_t itype=0; itype<kNtypes; itype++) {
    fNCuts[itype]=1;
    for(Int_t i=0; i<20; i++) {
      fCutNames[i][itype]="";
    }
  }
  fTypeKeys[kTrack]  = "Track";
  fTypeKeys[kTrack2] = "Track2";
  fTypeKeys[kTrackMC]= "MCTrack";
  fTypeKeys[kPair]   = "Pair";
  fTypeKeys[kPrePair]= "Pair0";
  fTypeKeys[kEvent]  = "Event";
  fQAHistList.SetOwner(kFALSE);
}

//_____________________________________________________________________
PairAnalysisCutQA::~PairAnalysisCutQA() 
{
  //
  //Default Destructor
  //
  fQAHistList.Clear();
}

//_____________________________________________________________________
void PairAnalysisCutQA::Init()
{

  fQAHistList.SetName(Form("%s",GetName()));

  THashList *table=new THashList;
  table->SetOwner(kTRUE);
  table->SetName("Event");
  fQAHistList.Add(table);

  table=new THashList;
  table->SetOwner(kTRUE);
  table->SetName("Track");
  fQAHistList.Add(table);

  table=new THashList;
  table->SetOwner(kTRUE);
  table->SetName("Pair");
  fQAHistList.Add(table);


  TH1I *fCutQA=0x0;     // qa histogram for counts
  TH2I *fPdgCutQA=0x0;  // qa histogram for PDG codes
  TProfile2D *fEffCutQA=0x0; // qa histogram for matching efficicy

  const TVectorD *binsPdg = PairAnalysisHelper::MakeLinBinning(5,0,5);
  const TVectorD *binsDet = PairAnalysisHelper::MakeLinBinning(6,0,6);
  // loop over all types
  for(Int_t itype=0; itype<kNtypes; itype++) {
    //    printf("\n type: %d\n",itype);
    fCutNames[0][itype]="no cuts";

    const TVectorD *binsX = PairAnalysisHelper::MakeLinBinning(fNCuts[itype],0,fNCuts[itype]);
    // create histogram based on added cuts
    fCutQA = new TH1I(fTypeKeys[itype],
			     Form("%sQA;cuts;# passed %ss",fTypeKeys[itype],fTypeKeys[itype]), 			     fNCuts[itype], binsX->GetMatrixArray());

    if(itype==kTrack || itype==kTrack2) {
      fPdgCutQA = new TH2I(Form("%sPDG",fTypeKeys[itype]),
				  Form("%sPDG;cuts;PDG code;# passed %ss",
				       fTypeKeys[itype],fTypeKeys[itype]),
				  fNCuts[itype],binsX->GetMatrixArray(),
				  binsPdg->GetNrows()-1,binsPdg->GetMatrixArray() );

      fEffCutQA = new TProfile2D(Form("%sMatchEff",fTypeKeys[itype]),
					Form("%sMatchEff;cuts;detector;<#epsilon_{match}^{MC}>",
					     fTypeKeys[itype]),
					fNCuts[itype],binsX->GetMatrixArray(),
					binsDet->GetNrows()-1,binsDet->GetMatrixArray() );
    }

    // delete surplus vector
    delete binsX;

    // Set labels to histograms
    // loop over all cuts
    for(Int_t i=0; i<fNCuts[itype]; i++) {
      fCutQA->GetXaxis()->SetBinLabel(i+1,fCutNames[i][itype]);
      if(fPdgCutQA) fPdgCutQA->GetXaxis()->SetBinLabel(i+1,fCutNames[i][itype]);
      if(fEffCutQA) fEffCutQA->GetXaxis()->SetBinLabel(i+1,fCutNames[i][itype]);
      //      printf(" %s \n",fCutNames[i][itype]);
    }

    // pdg label
    if(fPdgCutQA) {
      TString pdglbl="";
      for(Int_t i=0; i<binsPdg->GetNrows()-1; i++) {
	switch(i+1) {
	case 1:  pdglbl="electron"; break; // electron
	case 2:  pdglbl="muon";     break; // muon
	case 3:  pdglbl="pion";     break; // pion
	case 4:  pdglbl="kaon";     break; // kaon
	case 5:  pdglbl="proton";   break; // proton
	}
	fPdgCutQA->GetYaxis()->SetBinLabel(i+1,pdglbl.Data());
      }
    }

    // detector label
    if(fEffCutQA) {
      TString detlbl="";
      for(Int_t i=0; i<binsDet->GetNrows()-1; i++) {
	switch(i+1) {
	case 1:  detlbl=PairAnalysisHelper::GetDetName(kMVD);  break;
	case 2:  detlbl=PairAnalysisHelper::GetDetName(kSTS);  break;
	case 3:  detlbl=PairAnalysisHelper::GetDetName(kRICH); break;
	case 4:  detlbl=PairAnalysisHelper::GetDetName(kTRD);  break;
	case 5:  detlbl=PairAnalysisHelper::GetDetName(kTOF);  break;
	case 6:  detlbl=PairAnalysisHelper::GetDetName(kMUCH); break;
	}
	fEffCutQA->GetYaxis()->SetBinLabel(i+1,detlbl.Data());
      }
    }

    // add to output list
    switch(itype) {
      case kEvent: 
	static_cast<THashList*>(fQAHistList.FindObject("Event"))->AddLast(fCutQA);
	break;
      case kTrack: 
      case kTrack2: 
      case kTrackMC: 
	static_cast<THashList*>(fQAHistList.FindObject("Track"))->AddLast(fCutQA);
	if(fPdgCutQA) static_cast<THashList*>(fQAHistList.FindObject("Track"))->AddLast(fPdgCutQA);
	if(fEffCutQA) static_cast<THashList*>(fQAHistList.FindObject("Track"))->AddLast(fEffCutQA);
	break;
      case kPair: 
      case kPrePair: 
	static_cast<THashList*>(fQAHistList.FindObject("Pair"))->AddLast(fCutQA);
	break;
    }

  }

  // delete surplus
  delete binsPdg;

}

//_____________________________________________________________________
void PairAnalysisCutQA::AddTrackFilter(AnalysisFilter *filter)
{
  //
  // add track filter cuts to the qa histogram
  //


  TIter listIterator(filter->GetCuts());
  while (AnalysisCuts *thisCut = (AnalysisCuts*) listIterator()) {
    Bool_t addCut=kTRUE;

    // add new cut class to the list
    if(addCut) {
      fCutNames[fNCuts[kTrack]][kTrack]=thisCut->GetTitle();
      //      printf("add cut %s to %d \n",thisCut->GetTitle(),fNCuts[kTrack]);
      fNCuts[kTrack]++;
    }

  } // pair filter loop

}

//_____________________________________________________________________
void PairAnalysisCutQA::AddTrackFilterMC(AnalysisFilter *filter)
{
  //
  // add MC track filter cuts to the qa histogram
  //


  TIter listIterator(filter->GetCuts());
  while (AnalysisCuts *thisCut = (AnalysisCuts*) listIterator()) {
    Bool_t addCut=kTRUE;

    // add new cut class to the list
    if(addCut) {
      fCutNames[fNCuts[kTrackMC]][kTrackMC]=thisCut->GetTitle();
      //      printf("add cut %s to %d \n",thisCut->GetTitle(),fNCuts[kTrack]);
      fNCuts[kTrackMC]++;
    }

  } // pair filter loop

}

//_____________________________________________________________________
void PairAnalysisCutQA::AddTrackFilter2(AnalysisFilter *filter)
{
  //
  // add track filter cuts to the qa histogram
  //
  if(!filter) return;

  TIter listIterator(filter->GetCuts());
  while (AnalysisCuts *thisCut = (AnalysisCuts*) listIterator()) {
    Bool_t addCut=kTRUE;

    // add new cut class to the list
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

    // add new cut class to the list
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

    // add new cut class to the list
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

    // add new cut class to the list
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

  // find histolist
  THashList *histos=0x0;
  switch(idx) {
  case kEvent: 
    histos = static_cast<THashList*>(fQAHistList.FindObject("Event"));
    break;
  case kTrack: 
  case kTrack2: 
  case kTrackMC: 
    histos = static_cast<THashList*>(fQAHistList.FindObject("Track"));
    break;
  case kPair: 
  case kPrePair: 
    histos = static_cast<THashList*>(fQAHistList.FindObject("Pair"));
    break;
  }

  // loop over cutmask and check decision
  Int_t cutstep=1;
  for (Int_t iCut=0; iCut<fNCuts[idx]-1;++iCut) {
    //    UInt_t cutMask=1<<iCut;         // for each cut
    UInt_t cutMask=(1<<(iCut+1))-1; // increasing cut match

    // passed
    if ((mask&cutMask)==cutMask) {
      static_cast<TH1I*>(histos->FindObject(fTypeKeys[idx])) ->Fill(cutstep);
      if(pdg) static_cast<TH2I*>(histos->FindObject(Form("%sPDG",fTypeKeys[idx]))) ->Fill(cutstep, pdglbl.Data(),1.);

      // fill detector dependent
      if(idx==kTrack || idx==kTrack2) {
	TProfile2D *detQA = static_cast<TProfile2D*>(histos->FindObject(Form("%sMatchEff",fTypeKeys[idx])));
	PairAnalysisTrack *t = static_cast<PairAnalysisTrack*>(obj);
	detQA->Fill(cutstep, PairAnalysisHelper::GetDetName(kMVD), t->TestBit( BIT(14+kMVD) ) );
	detQA->Fill(cutstep, PairAnalysisHelper::GetDetName(kSTS), t->TestBit( BIT(14+kSTS) ) );
	detQA->Fill(cutstep, PairAnalysisHelper::GetDetName(kRICH),t->TestBit( BIT(14+kRICH)) );
	detQA->Fill(cutstep, PairAnalysisHelper::GetDetName(kTRD), t->TestBit( BIT(14+kTRD) ) );
	detQA->Fill(cutstep, PairAnalysisHelper::GetDetName(kTOF), t->TestBit( BIT(14+kTOF) ) );
	detQA->Fill(cutstep, PairAnalysisHelper::GetDetName(kMUCH),t->TestBit( BIT(14+kMUCH)) );
      }

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

  // find histolist
  THashList *histos=0x0;
  switch(idx) {
  case kEvent: 
    histos = static_cast<THashList*>(fQAHistList.FindObject("Event"));
    break;
  case kTrack: 
  case kTrack2: 
  case kTrackMC: 
    histos = static_cast<THashList*>(fQAHistList.FindObject("Track"));
    break;
  case kPair: 
  case kPrePair: 
    histos = static_cast<THashList*>(fQAHistList.FindObject("Pair"));
    break;
  }

  // fill
  static_cast<TH1I*>(histos->FindObject(fTypeKeys[idx])) ->Fill(0);
  if(pdg) static_cast<TH2I*>(histos->FindObject(Form("%sPDG",fTypeKeys[idx]))) ->Fill(0., pdglbl.Data(),1.);

  // fill detector dependent
  if(idx==kTrack || idx==kTrack2) {
    TProfile2D *detQA = static_cast<TProfile2D*>(histos->FindObject(Form("%sMatchEff",fTypeKeys[idx])));
    PairAnalysisTrack *t = static_cast<PairAnalysisTrack*>(obj);
    detQA->Fill(0., PairAnalysisHelper::GetDetName(kMVD), t->TestBit( BIT(14+kMVD) ) );
    detQA->Fill(0., PairAnalysisHelper::GetDetName(kSTS), t->TestBit( BIT(14+kSTS) ) );
    detQA->Fill(0., PairAnalysisHelper::GetDetName(kRICH),t->TestBit( BIT(14+kRICH)) );
    detQA->Fill(0., PairAnalysisHelper::GetDetName(kTRD), t->TestBit( BIT(14+kTRD) ) );
    detQA->Fill(0., PairAnalysisHelper::GetDetName(kTOF), t->TestBit( BIT(14+kTOF) ) );
    detQA->Fill(0., PairAnalysisHelper::GetDetName(kMUCH),t->TestBit( BIT(14+kMUCH)) );
  }

}

//______________________________________________________________________
UInt_t PairAnalysisCutQA::GetObjIndex(TObject *obj)
{
  //
  // return the corresponding idex
  //
  //  printf("INFO: object type is a %s \n", obj->IsA()->GetName());
  if(obj->IsA()==CbmMCTrack::Class()               )  return kTrackMC;
  else if(obj->IsA()==PairAnalysisTrack::Class()   )  return kTrack;
  else if(obj->IsA()==PairAnalysisPairLV::Class()  )  return kPair;
  else if(obj->IsA()==PairAnalysisEvent::Class()   )  return kEvent;
  else printf("ERROR: object type %s not yet supported, please let the author know\n", obj->IsA()->GetName());
  return -1;

}




