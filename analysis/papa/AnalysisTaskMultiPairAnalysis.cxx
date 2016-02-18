///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                        Basic Analysis Task                            //
//
// Authors:
//   * Copyright(c) 1998-2009, ALICE Experiment at CERN, All rights reserved. *
//   Julian Book   <Julian.Book@cern.ch>
/*

  This task can hold multiple instances of PairAnalysis and a common set 
  of meta data PairAnalysisMetaData.

  A.t.m. you have to provide the beam energy via SetBeamEnergy(Double_t beamEbyHand)
  and can optionally add common event cuts for all PairAnalysis instances via
  SetEventFilter(AnalysisCuts *const filter)


*/
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
#include "PairAnalysisMC.h"

#include "AnalysisTaskMultiPairAnalysis.h"

ClassImp(AnalysisTaskMultiPairAnalysis)

//_________________________________________________________________________________
AnalysisTaskMultiPairAnalysis::AnalysisTaskMultiPairAnalysis() :
  AnalysisTaskMultiPairAnalysis("name")
{
  //
  // Constructor
  //
}

//_________________________________________________________________________________
AnalysisTaskMultiPairAnalysis::AnalysisTaskMultiPairAnalysis(const char *name) :
  FairTask(name),
  fMetaData(),
  fListPairAnalysis(),
  fListHistos(),
  fTimer(),
  fProcInfo()
{
  //
  // Named Constructor
  //
  fMetaData.SetName(Form("PairAnalysisMetaData_%s",name));
  fListHistos.SetName(Form("PairAnalysisHistos_%s",name));
  fListPairAnalysis.SetOwner();
  ((TList*)fMetaData.GetMetaData())->SetOwner();
  fListHistos.SetOwner();
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
  if(fgRichElIdAnn)    { delete fgRichElIdAnn;    fgRichElIdAnn=0; }
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
  fMetaData.FillMeta("beamenergy",fBeamEnergy); // TODO: internal access to FairBaseParSet::GetBeamMom()

  if (!fListHistos.IsEmpty()) return kERROR; //already initialised

  // register output for each analysis instance
  TIter nextDie(&fListPairAnalysis);
  PairAnalysis *papa=0;
  while ( (papa=static_cast<PairAnalysis*>(nextDie())) ){
    papa->Init();
    if (papa->GetHistogramList())       fListHistos.Add(const_cast<THashList*>(papa->GetHistogramList()));
    if (papa->GetHistogramArray())      fListHistos.Add(const_cast<TObjArray*>(papa->GetHistogramArray()));
    if (papa->GetQAHistList())          fListHistos.Add(const_cast<THashList*>(papa->GetQAHistList()));
    if (papa->GetCutStepHistogramList())fListHistos.Add(static_cast<THashList*>(papa->GetCutStepHistogramList()));
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
  printf("AnalysisTaskMultiPairAnalysis::Init:"" Real time %fs, CPU time %fs, Memory %li MB(res.) %li MB(virt.) \n",fTimer.RealTime(),fTimer.CpuTime(),fProcInfo.fMemResident/1024,fProcInfo.fMemVirtual/1024);
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

  if (fListHistos.IsEmpty()) return;
  fEventsTotal++;
  if( !(fEventsTotal%10) ) {
    gSystem->GetProcInfo(&fProcInfo);
    printf("AnalysisTaskMultiPairAnalysis::Exec: Process %.3e events, CPU time %.1fs, (%fs per event, eff %.3f), Memory %li MB(res.) %li MB(virt.) \n",
	   (Double_t)fEventsTotal, fTimer.CpuTime(), fTimer.CpuTime()/fEventsTotal, fTimer.CpuTime()/fTimer.RealTime(), fProcInfo.fMemResident/1024, fProcInfo.fMemVirtual/1024);
    fTimer.Continue();
  }

  // initialize track arrays and some track based variables
  fInputEvent->Init(); // NOTE: tracks are initialized with mass hypo PDG 11, and adapted later!
  PairAnalysisVarManager::SetEvent(fInputEvent);

  // set the beam energy to the varmanager
  PairAnalysisVarManager::SetValue(PairAnalysisVarManager::kEbeam, fBeamEnergy);

  // magnetic field

  //Fill Event histograms before the event filter for all instances
  TIter nextDie(&fListPairAnalysis);
  PairAnalysis *papa=0;
  Bool_t hasMC=kFALSE;//TODO:PairAnalysisMC::Instance()->HasMC();
  while ( (papa=static_cast<PairAnalysis*>(nextDie())) ){
    PairAnalysisHistos *h=papa->GetHistoManager();
    if (h) {
      PairAnalysisVarManager::SetFillMap(h->GetUsedVars());
      // fill MCtruth information
      //if (hasMC && PairAnalysisMC::Instance()->ConnectMCEvent() && h->GetHistogramList()->FindObject("MCEvent.noCuts")) {
      //	PairAnalysisVarManager::SetEvent(PairAnalysisMC::Instance()->GetMCEvent());
      //       h->FillClass("MCEvent.noCuts",PairAnalysisVarManager::GetData());
      //      }
      // fill reconstructed information
      if (h->GetHistogramList()->FindObject("Event.noCuts")) {
        h->FillClass("Event.noCuts",PairAnalysisVarManager::GetData());
      }
    }
  }
  nextDie.Reset();

  // common event filter for all instances
  if (fEventFilter) {
    if (!fEventFilter->IsSelected(fInputEvent)) return;
  }
  fEventsSelected++;

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
    // if (papa->HasCandidates()){
    //   Int_t ncandidates=papa->GetPairArray(1)->GetEntriesFast();
    // }

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
  fMetaData.FillMeta("events",fEventsSelected);

  // write output to file
  Printf("AnalysisTaskMultiPairAnalysis::FinsihTask - write histo list to %s",
	 FairRootManager::Instance()->GetOutFile()->GetName());
  FairRootManager::Instance()->GetOutFile()->cd();

  fMetaData.GetMetaData()->Write(fMetaData.GetName(),TObject::kSingleKey);
  fListHistos.Write(fListHistos.GetName(),TObject::kSingleKey);

}

