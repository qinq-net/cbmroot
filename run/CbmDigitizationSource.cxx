/*
 * CbmDigitizationSource.cxx
 *
 *  Created on: 06.11.2018
 *      Author: vfriese
 */

#include "CbmDigitizationSource.h"

#include <cassert>
#include "TChain.h"
#include "TFolder.h"
#include "TROOT.h"
#include "FairRootManager.h"


// -----   Constructor   -----------------------------------------------------
CbmDigitizationSource::CbmDigitizationSource() :
        FairSource(),
        fInputs(),
        fNextEvent(),
        fMCEventHeader(),
        fListOfFolders(new TObjArray()),
        fCurrentTime(0.),
        fCurrentEntryId(0),
        fCurrentInputId(0),
        fCurrentRunId(0),
        fFirstCall(kTRUE) {
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmDigitizationSource::~CbmDigitizationSource() {
  for (auto const& mapEntry : fInputs)
    if (mapEntry.second) delete mapEntry.second;
}
// ---------------------------------------------------------------------------



// -----   Set the branch address of a branch   -----------------------------
Bool_t CbmDigitizationSource::ActivateObject(TObject** object,
                                             const char* branchName) {

  for (auto const& mapEntry : fInputs) {
    mapEntry.second->GetChain()->SetBranchStatus(branchName, 1);
    mapEntry.second->GetChain()->SetBranchAddress(branchName, object);
  }

  return kTRUE;
}
// ---------------------------------------------------------------------------



// -----   Add a transport input   -------------------------------------------
void CbmDigitizationSource::AddInput(UInt_t inputId, TChain* chain,
                                     Double_t rate) {
  if (fInputs.find(inputId) != fInputs.end()) {
    LOG(ERROR) << "DigitizationSource: input Id " << inputId
               << " is already defined!" << FairLogger::endl;
    return;
  }
  fInputs[inputId] = new CbmInputChain(chain, rate);
}
// TODO: Check whether branch / folder structure of the inputs are
// consistent.
// ---------------------------------------------------------------------------



// -----   Check the maximal entry the source can run to   -------------------
Int_t CbmDigitizationSource::CheckMaxEventNo(Int_t lastEntry) {
  if ( lastEntry != 0 ) return lastEntry;
  return 1e6;
}
// ---------------------------------------------------------------------------



// -----   Fill the event header   -------------------------------------------
void CbmDigitizationSource::FillEventHeader(FairEventHeader* event) {
  assert(event);
  event->SetEventTime(fCurrentTime);
  event->SetMCEntryNumber(fCurrentEntryId);
  event->SetInputFileId(fCurrentInputId);
  event->SetRunId(fCurrentRunId);
  LOG(DEBUG) << "DigitizationSource: Event with RunId " << fCurrentRunId
             << ", input " << fCurrentInputId << ", entry "
             << fCurrentEntryId << ", time " << fCurrentTime << " ns"
             << FairLogger::endl;
}
// ---------------------------------------------------------------------------



// -----   Initialisation   --------------------------------------------------
Bool_t CbmDigitizationSource::Init() {

  // No action if no inputs are defined
  if (fInputs.empty()) return kFALSE;

  // Register the input chains to FairRootManager
  for (auto const& mapEntry : fInputs) {
    CbmInputChain* input = mapEntry.second;
    FairRootManager::Instance()->SetInChain(input->GetChain(),
                                            mapEntry.first);
  }

  // Get folder from first input file and register it to FairRootManager
  TFile* file = fInputs[0]->GetChain()->GetFile();
  TFolder* folder = dynamic_cast<TFolder*>(file->Get("cbmroot"));
  assert(folder);
  gROOT->GetListOfBrowsables()->Add(folder);
  fListOfFolders->Add(folder);
  FairRootManager::Instance()->SetListOfFolders(fListOfFolders);

  // Activate the MC event header to all respective input branches.
  // This is necessary since it is used from this class.
  // The other branches will be activated on request by the method
  // ActivateObject called from FairRootManager
  fMCEventHeader = new FairMCEventHeader();
  TObject** object = reinterpret_cast<TObject**>(&fMCEventHeader);
  ActivateObject(object, "MCEventHeader.");

  // Set the time of the first event for each input
  for (auto const& mapEntry : fInputs) {
    CbmInputChain* input = mapEntry.second;
    Double_t time = input->GetDeltaT();
    LOG(INFO) << "First time for input " << mapEntry.first << " is " << time
              << FairLogger::endl;
    fNextEvent.insert(std::make_pair(time, mapEntry.first));
  }

  return kTRUE;
}
// ---------------------------------------------------------------------------



// -----   Define one input event   ------------------------------------------
Int_t CbmDigitizationSource::ReadEvent(UInt_t event) {

  // Before the actual run, ReadEvent is once called from FairRunAna::Init().
  // This is to get the run ID needed for setting the parameter containers.
  // Since the input entries are read sequentially, this would mean
  // always losing the first entry. We here protect against that by calling
  // the first entry of the first input directly, without incrementing its
  // current entry bookkeeper.
  if ( fFirstCall ) {
    LOG(INFO) << "First call to ReadEvent" << FairLogger::endl;
    CbmInputChain* input = fInputs.at(0);
    input->GetChain()->GetEntry(0);
    fCurrentRunId = fMCEventHeader->GetRunID();
    fCurrentInputId = 0;
    fCurrentEntryId = 0;
    fCurrentTime = 0.;
    fFirstCall = kFALSE;
    return 0;
  }

  Double_t time = fNextEvent.begin()->first;
  UInt_t inputId = fNextEvent.begin()->second;
  CbmInputChain* input = fInputs.at(inputId);
  assert(input);

  // Get the next entry from the respective input chain
  Int_t entryId = input->GetNextEntry();
  fCurrentTime = time;
  fCurrentEntryId = entryId;
  fCurrentInputId = inputId;

  LOG(INFO) << "DigitizationSource: Event " << event << " at t = " << time
            << " ns" << " from input " << inputId << " (entry " << entryId
            << ")" << FairLogger::endl;

  // Stop the run if the number of entries in this input is reached
  if (entryId < 0) {
    LOG(INFO) << "DigitizationSource: No more entries in input " << inputId
        << FairLogger::endl;
    return 1;
  }

  // Remove this input from the list of next events
  fNextEvent.erase(fNextEvent.begin());

  // Calculate next event time for this input
  time += input->GetDeltaT();
  fNextEvent.insert(std::make_pair(time, inputId));
  LOG(DEBUG) << "DigitizationSource: Next event time for input " << inputId
             << " is t = " << time << " ns" << FairLogger::endl;

  return 0;

}
// ---------------------------------------------------------------------------



ClassImp(CbmDigitizationSource)
