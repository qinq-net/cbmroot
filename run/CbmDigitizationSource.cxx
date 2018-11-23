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
        fBranches(),
        fCurrentTime(0.),
        fCurrentEntryId(0),
        fCurrentInputId(0),
        fCurrentRunId(0),
        fFirstCall(kTRUE),
        fEventMode(kFALSE) {
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmDigitizationSource::~CbmDigitizationSource() {
  for (auto const& mapEntry : fInputs)
    if (mapEntry.second) delete mapEntry.second;
  fBranches.clear();
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
                                     Double_t rate,
                                     Cbm::ETreeAccess mode) {

  // First input; defines branch list
  if ( fInputs.empty() ) {
    CbmMCInput* input = new CbmMCInput(chain, rate, mode);
    fBranches = input->GetBranchList();
    fInputs[inputId] = input;
  } //? First input

  // Not the first input; check consistency of branch lists
  else {
    if (fInputs.find(inputId) != fInputs.end()) {
    LOG(ERROR) << "DigitizationSource: input Id " << inputId
               << " is already defined!" << FairLogger::endl;
    return;
    }
    CbmMCInput* input = new CbmMCInput(chain, rate, mode);
    if ( ! CheckBranchList(input) ) {
      LOG(FATAL) << "DigitizationSource: Incompatible branch list!"
          << FairLogger::endl;
      delete input;
    } //? Branch list in new input is not compatible
    else fInputs[inputId] = input;
  } //? Not first input

}
// ---------------------------------------------------------------------------



// -----   Check the branch list of an input   -------------------------------
Bool_t CbmDigitizationSource::CheckBranchList(CbmMCInput* input) {

  assert(input);
  Bool_t success = kTRUE;
  for (auto const& entry : fBranches) {
    auto it = input->GetBranchList().find(entry);
    if ( it == input->GetBranchList().end() ) {
      LOG(DEBUG) << "DigitizationSource: required branch " << entry
          << " not present in input!" << FairLogger::endl;
      success = kFALSE;
      break;
    } //? Global branch not in input
  } //# Global branches

  if ( ! success ) {
    LOG(INFO) << "DigitizationSource: Global branch list is ";
    for (auto const& entry : fBranches ) LOG(INFO) << entry << " ";
    LOG(INFO) << FairLogger::endl;
    LOG(INFO) << "DigitizationSource: Input branch list is ";
    for (auto const& entry : input->GetBranchList() ) LOG(INFO) << entry << " ";
    LOG(INFO) << FairLogger::endl;
  } //? Branches not compatible

  return success;
}
// ---------------------------------------------------------------------------



// -----   Check the maximal entry the source can run to   -------------------
Int_t CbmDigitizationSource::CheckMaxEventNo(Int_t lastEntry) {

  // If lastEntry is specified, this is the value to be returned.
  // A bit strange syntax, but it's in FairRunAna.
  if ( lastEntry != 0 ) return lastEntry;

  // Catch the case when no input is connected
  if ( fInputs.empty() ) return 0;

  // If there is only one input or in the event-by-event mode,
  // return the number of entries in the first input.
  if ( fInputs.size() == 1 || fEventMode )
    return fInputs.begin()->second->GetNofEntries();

  // If there is more than one input, a maximum event number cannot be
  // defined. The run will proceed until one of the inputs is exhausted
  // or if terminated by CTRL+C.
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



// -----   Get an input   ----------------------------------------------------
CbmMCInput* CbmDigitizationSource::GetInput(UInt_t inputId) {
  if ( fInputs.find(inputId) == fInputs.end() ) return nullptr;
  else return fInputs[inputId];
}
// ---------------------------------------------------------------------------



// -----   Initialisation   --------------------------------------------------
Bool_t CbmDigitizationSource::Init() {

  // No action if no inputs are defined
  if (fInputs.empty()) return kFALSE;

  // Register the input chains to FairRootManager
  for (auto const& mapEntry : fInputs) {
    CbmMCInput* input = mapEntry.second;
    FairRootManager::Instance()->SetInChain(input->GetChain(),
                                            mapEntry.first);
  }

  // Get folder from first input file and register it to FairRootManager
  TFile* file = fInputs.begin()->second->GetChain()->GetFile();
  TFolder* folder = dynamic_cast<TFolder*>(file->Get("cbmroot"));
  assert(folder);
  gROOT->GetListOfBrowsables()->Add(folder);
  fListOfFolders->Add(folder);
  FairRootManager::Instance()->SetListOfFolders(fListOfFolders);

  // Activate the MC event header to all respective input branches.
  // This is necessary since it is used from this class.
  // The other branches will be activated if needed by a task
  // through the method ActivateObject called from FairRootManager
  fMCEventHeader = new FairMCEventHeader();
  TObject** object = reinterpret_cast<TObject**>(&fMCEventHeader);
  ActivateObject(object, "MCEventHeader.");

  // Set the time of the first event for each input
  if ( ! fEventMode ) {
    for (auto const& mapEntry : fInputs) {
      CbmMCInput* input = mapEntry.second;
      Double_t time = input->GetDeltaT();
      LOG(INFO) << "First time for input " << mapEntry.first << " is " << time
          << FairLogger::endl;
      fNextEvent.insert(std::make_pair(time, mapEntry.first));
    }
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
    CbmMCInput* input = fInputs.begin()->second;
    input->GetChain()->GetEntry(0);
    fCurrentRunId = fMCEventHeader->GetRunID();
    fCurrentInputId = fInputs.begin()->first;
    fCurrentEntryId = 0;
    fCurrentTime = 0.;
    fFirstCall = kFALSE;
    LOG(INFO) << "DigitizationSource: Run ID is " << fCurrentRunId
        << FairLogger::endl;
    return 0;
  }

  // In the event-by-event mode, get the respective event from the first
  // input; the event time is zero.
  if ( fEventMode ) {
    CbmMCInput* input = fInputs.begin()->second;
    // Stop run if out-of-range of input tree
    if ( event >= input->GetNofEntries() ) {
      LOG(INFO) << "DigitizationSource: Requested event " << event
          << " exceeds number of entries in input ( "
          << input->GetNofEntries() << " )" << FairLogger::endl;
      return 1;
    }
    input->GetChain()->GetEntry(event);
    fCurrentInputId = fInputs.begin()->first;
    fCurrentEntryId = event;
    fCurrentTime = 0.;
    LOG(INFO) << "DigitizationSource: Event " << event << " at t = "
              << fCurrentTime << " ns" << " from input " << fCurrentInputId
              << " (entry " << fCurrentEntryId << ")" << FairLogger::endl;
    return 0;
  } //? Event-by-event mode

  // Get the input with the next event time
  Double_t time = fNextEvent.begin()->first;
  UInt_t inputId = fNextEvent.begin()->second;
  CbmMCInput* input = fInputs.at(inputId);
  assert(input);

  // Get the next entry from the respective input chain
  fCurrentEntryId = input->GetNextEntry();
  fCurrentTime = time;
  fCurrentInputId = inputId;
  LOG(INFO) << "DigitizationSource: Event " << event << " at t = " << time
            << " ns" << " from input " << inputId << " (entry "
            << fCurrentEntryId << ")" << FairLogger::endl;

  // Stop the run if the number of entries in this input is reached
  if (fCurrentEntryId < 0) {
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
