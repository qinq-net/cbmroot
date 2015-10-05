/*************************************************************************
* Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
**************************************************************************/

///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                        Basic Analysis Task                            //
//                                                                       //
///////////////////////////////////////////////////////////////////////////

#include "TSystem.h"
#include <TStopwatch.h>
#include <TChain.h>
#include <TH1D.h>

#include "FairRootManager.h"
#include "CbmRichElectronIdAnn.h"

#include "PairAnalysisMetaData.h"
#include "PairAnalysisEvent.h"
#include "PairAnalysis.h"
#include "PairAnalysisHistos.h"
//#include "PairAnalysisCF.h"
#include "PairAnalysisMC.h"
//#include "PairAnalysisMixingHandler.h"

#include "AnalysisTaskMultiPairAnalysis.h"

ClassImp(AnalysisTaskMultiPairAnalysis)

//_________________________________________________________________________________
AnalysisTaskMultiPairAnalysis::AnalysisTaskMultiPairAnalysis() :
  FairTask(),
  fMetaData(),
  fPairArray(0x0),
  fListPairAnalysis(),
  fListHistos(),
//  fListCF(),
  fgRichElIdAnn(0x0),
  fBeamEnergy(0.),
  fWeight(1.),
  fEventFilter(0x0),
  fEventStat(0x0),
  fInputEvent(0x0),
  fTimer(),
  fProcInfo()
{
  //
  // Constructor
  //
}

//_________________________________________________________________________________
AnalysisTaskMultiPairAnalysis::AnalysisTaskMultiPairAnalysis(const char *name) :
  FairTask(name),
  fMetaData(),
  fPairArray(0x0),
  fListPairAnalysis(),
  fListHistos(),
  //  fListCF(),
  fgRichElIdAnn(0x0),
  fBeamEnergy(0.),
  fWeight(1.),
  fEventFilter(0x0),
  fEventStat(0x0),
  fInputEvent(0x0),
  fTimer(),
  fProcInfo()
{
  //
  // Constructor
  //
  fMetaData.SetName(Form("PairAnalysisMetaData_%s",name));
  fListHistos.SetName(Form("PairAnalysisHistos_%s",name));
  //  fListCF.SetName(Form("PairAnalysisCF_%s",name));
  fListPairAnalysis.SetOwner();
  ((TList*)fMetaData.GetMetaData())->SetOwner();
  fListHistos.SetOwner();
  //  fListCF.SetOwner();
}

//_________________________________________________________________________________
AnalysisTaskMultiPairAnalysis::~AnalysisTaskMultiPairAnalysis()
{
  //
  // Destructor
  //

  //histograms and CF are owned by the papa framework.
  //however they are streamed to file, so in the first place the
  //lists need to be owner...
  fListPairAnalysis.SetOwner();
  ((TList*)fMetaData.GetMetaData())->SetOwner(kFALSE);
  fListHistos.SetOwner(kFALSE);
  //  fListCF.SetOwner(kFALSE);
  if(fgRichElIdAnn)    { delete fgRichElIdAnn;     fgRichElIdAnn=0; }
  // try to reduce memory issues
  if(fEventStat)       { delete fEventStat;       fEventStat=0; }
  if(fInputEvent)      { delete fInputEvent;      fInputEvent=0; }
}
//_________________________________________________________________________________
InitStatus AnalysisTaskMultiPairAnalysis::Init()
{
  //
  // Add all histogram manager histogram lists to the output TList
  //

  fTimer.Start();

  // fill metadata object
  fMetaData.Init();
  fMetaData.FillMeta("beamenergy",fBeamEnergy);

  if (!fListHistos.IsEmpty()/*||!fListCF.IsEmpty()*/) return kERROR; //already initialised

  TIter nextDie(&fListPairAnalysis);
  PairAnalysis *papa=0;
  while ( (papa=static_cast<PairAnalysis*>(nextDie())) ){
    papa->Init();
    if (papa->GetHistogramList())    fListHistos.Add(const_cast<THashList*>(papa->GetHistogramList()));
    if (papa->GetHistogramArray())   fListHistos.Add(const_cast<TObjArray*>(papa->GetHistogramArray()));
    if (papa->GetQAHistList())       fListHistos.Add(const_cast<THashList*>(papa->GetQAHistList()));
    //    if (papa->GetCFManagerPair())    fListCF.Add(const_cast<AliCFContainer*>(papa->GetCFManagerPair()->GetContainer()));
  }

  Int_t cuts=fListPairAnalysis.GetEntries();
  Int_t nbins=kNbinsEvent+2*cuts;
  if (!fEventStat){
    fEventStat=new TH1D("hEventStat","Event statistics",nbins,0,nbins);
    fEventStat->GetXaxis()->SetBinLabel(1,"Before Phys. Sel.");
    fEventStat->GetXaxis()->SetBinLabel(2,"After Phys. Sel.");

    //default names
    fEventStat->GetXaxis()->SetBinLabel(3,"Bin3 not used");
    fEventStat->GetXaxis()->SetBinLabel(4,"Bin4 not used");
    fEventStat->GetXaxis()->SetBinLabel(5,"Bin5 not used");

    //    if (fTriggerOnV0AND) fEventStat->GetXaxis()->SetBinLabel(3,"V0and triggers");
    if (fEventFilter)    fEventStat->GetXaxis()->SetBinLabel(4,"After Event Filter");
    //    if (fRejectPileup)   fEventStat->GetXaxis()->SetBinLabel(5,"After Pileup rejection");

    for (Int_t i=0; i<cuts; ++i){
      fEventStat->GetXaxis()->SetBinLabel((kNbinsEvent+1)+2*i,Form("#splitline{1 candidate}{%s}",fListPairAnalysis.At(i)->GetName()));
      fEventStat->GetXaxis()->SetBinLabel((kNbinsEvent+2)+2*i,Form("#splitline{With >1 candidate}{%s}",fListPairAnalysis.At(i)->GetName()));
    }
  }

  // Get Instance of FairRoot manager
  FairRootManager* man = FairRootManager::Instance();
  if (!man) { Fatal("AnalysisTaskMultiPairAnalysis::Init","No FairRootManager!"); }

  // Init the input event
  fInputEvent = new PairAnalysisEvent();
  fInputEvent->SetInput(man);

  // Connect the MC event
  PairAnalysisMC::Instance()->ConnectMCEvent();

  // init the RICH ANN-Pid repsonse
  fgRichElIdAnn = new CbmRichElectronIdAnn();
  fgRichElIdAnn->Init();
  PairAnalysisVarManager::SetRichPidResponse(fgRichElIdAnn);

  // initialization time and memory
  gSystem->GetProcInfo(&fProcInfo);
  printf("AnalysisTaskMultiPairAnalysis::Init:"" Real time %fs, CPU time %fs, Memory %i MB(res.) %i MB(virt.) \n",fTimer.RealTime(),fTimer.CpuTime(),fProcInfo.fMemResident/1024,fProcInfo.fMemVirtual/1024);
  fTimer.Reset();

  return kSUCCESS;
}

//_________________________________________________________________________________
void AnalysisTaskMultiPairAnalysis::Exec(Option_t *)
{
  //
  // Main loop. Called for every event
  //

  fTimer.Start(kFALSE);
  //  printf("\n\nAnalysisTaskMultiPairAnalysis::Exec: NEW event with %04d global tracks !!!!\r",
  //  fInputEvent->GetNumberOfTracks());
  //  printf("AnalysisTaskMultiPairAnalysis::Exec: global tracks %04d\n",fInputEvent->GetNumberOfTracks());
  //  printf("AnalysisTaskMultiPairAnalysis::Exec: mc tracks     %04d\n",fInputEvent->GetNumberOfMCTracks());

  if (fListHistos.IsEmpty()/*&&fListCF.IsEmpty()*/) return;
  Int_t bin = fEventStat->Fill(kAllEvents);

  Double_t evts = fEventStat->GetBinContent(bin);
  if(!(static_cast<Int_t>(evts)%10)) {
    gSystem->GetProcInfo(&fProcInfo);
    printf("AnalysisTaskMultiPairAnalysis::Exec: Process %.3e events, CPU time %.1fs, (%fs per event, eff %.3f), Memory %i MB(res.) %i MB(virt.) \n",
	   evts, fTimer.CpuTime(), fTimer.CpuTime()/evts, fTimer.CpuTime()/fTimer.RealTime(), fProcInfo.fMemResident/1024, fProcInfo.fMemVirtual/1024);
    fTimer.Continue();
  }
  //    Info("Exec", Form("Process %.3e events",fEventStat->GetBinContent(fEventStat->FindBin(kAllEvents))));

  // initialize track arrays and some track based variables
  fInputEvent->Init(); // NOTE: tracks are initialized with mass hypo PDG 11, and adapted later!
  PairAnalysisVarManager::SetEvent(fInputEvent);

  // set the beam energy to the varmanager
  PairAnalysisVarManager::SetValue(PairAnalysisVarManager::kEbeam, fBeamEnergy);
  PairAnalysisVarManager::SetValue(PairAnalysisVarManager::kWeight,fWeight);

  // Do trigger, event selection
  //  fInputEvent...

  //Fill Event histograms before the event filter for all instances
  TIter nextDie(&fListPairAnalysis);
  PairAnalysis *papa=0;
  Bool_t hasMC=kFALSE;//TODO:PairAnalysisMC::Instance()->HasMC();
  while ( (papa=static_cast<PairAnalysis*>(nextDie())) ){
    PairAnalysisHistos *h=papa->GetHistoManager();
    if (h) {
      PairAnalysisVarManager::SetFillMap(h->GetUsedVars());
      // fill MCtruth information
      //if (hasMC && PairAnalysisMC::Instance()->ConnectMCEvent() && h->GetHistogramList()->FindObject("MCEvent_noCuts")) {
      //	PairAnalysisVarManager::SetEvent(PairAnalysisMC::Instance()->GetMCEvent());
      //       h->FillClass("MCEvent_noCuts",PairAnalysisVarManager::GetData());
      //      }
      // fill reconstructed information
      if (h->GetHistogramList()->FindObject("Event_noCuts")) {
        h->FillClass("Event_noCuts",PairAnalysisVarManager::GetData());
      }
    }
  }
  nextDie.Reset();

  // common event filter for all instances
  if (fEventFilter) {
    if (!fEventFilter->IsSelected(fInputEvent)) return;
  }
  fEventStat->Fill(kFilteredEvents);

  // pileup
  fEventStat->Fill(kPileupEvents);

  //bz for AliKF
  //  Double_t bz = InputEvent()->GetMagneticField();
  //  AliKFParticle::SetField( bz );

  // energy for papa pair
  //  PairAnalysisPair::SetBeamEnergy(InputEvent(), fBeamEnergy);

  //Process event in all PairAnalysis instances
  Bool_t useInternal=kFALSE;
  Int_t ipapa=0;
  while ( (papa=static_cast<PairAnalysis*>(nextDie())) ){
    // event process
    if(papa->DoEventProcess()) {
      useInternal = papa->Process(fInputEvent);
      // input for internal train
      if(papa->DontClearArrays()) {
	fPairArray = (*(papa->GetPairArraysPointer()));
      }
    }
    else {
      // internal train
      if(useInternal) papa->Process(fPairArray);
    }

    // monitor pair candidates
    if (papa->HasCandidates()){
      Int_t ncandidates=papa->GetPairArray(1)->GetEntriesFast();
      if (ncandidates==1)     fEventStat->Fill((kNbinsEvent)  +2*ipapa);
      else if (ncandidates>1) fEventStat->Fill((kNbinsEvent+1)+2*ipapa);
    }

    ++ipapa;
  }

  fInputEvent->Clear();

}

//_________________________________________________________________________________
void AnalysisTaskMultiPairAnalysis::FinishTask()
{
  //
  // Write debug tree
  //

  // set meta data
  Double_t nevt = fEventStat->GetBinContent(fEventStat->FindBin(kFilteredEvents));
  fMetaData.FillMeta("events",static_cast<Int_t>(nevt));

  // write output to file
  Printf("AnalysisTaskMultiPairAnalysis::FinsihTask - write histo list to %s",
	 FairRootManager::Instance()->GetOutFile()->GetName());
  FairRootManager::Instance()->GetOutFile()->cd();

  fMetaData.GetMetaData()->Write(fMetaData.GetName(),TObject::kSingleKey);
  fListHistos.Write(fListHistos.GetName(),TObject::kSingleKey);

}

