/** @file CbmMCInputSet.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.11.2018
 **/

#include "CbmMCInputSet.h"

#include <cassert>
#include "FairLogger.h"


// -----   Default constructor   ---------------------------------------------
CbmMCInputSet::CbmMCInputSet() : CbmMCInputSet(-1.) {
}
// ---------------------------------------------------------------------------



// -----   Constructor   -----------------------------------------------------
CbmMCInputSet::CbmMCInputSet(Double_t rate) :
            TObject(),
            fRate(rate),
            fInputs(),
            fInputHandle(),
            fBranches(),
            fDeltaDist(nullptr) {

  if (rate > 0.) {
    Double_t mean = 1.e9 / rate;       // mean time between events
    fDeltaDist = new TF1("DeltaDist", "exp(-x/[0])/[0]", 0., 10. * mean);
    fDeltaDist->SetParameter(0, mean);
  }
  fInputHandle = fInputs.begin();
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmMCInputSet::~CbmMCInputSet() {
  if (fDeltaDist) delete fDeltaDist;
  for (auto const& entry : fInputs) if ( entry.second ) delete entry.second;
}
// ---------------------------------------------------------------------------



// -----   Add an input to the set   -----------------------------------------
void CbmMCInputSet::AddInput(UInt_t inputId, CbmMCInput* input) {

  // Catch invalid input pointer.
  if ( ! input ) {
    LOG(FATAL) << "MCInputSet: invalid input for input ID " << inputId
        << "!" << FairLogger::endl;
    return;
  } //? No valid input pointer

  // The first input defines the reference branch list.
  if ( fInputs.empty() ) {
    fBranches = input->GetBranchList();
    fInputs[inputId] = input;
    return;
  } //? First input

  // Catch input ID already being used.
  if ( fInputs.find(inputId) != fInputs.end() ) {
    LOG(FATAL) << "MCInputSet: input ID " << inputId
        << " is already defined!" << FairLogger::endl;
    return;
  } //? Input ID already used

  // Check compatibility of the input branch list with the reference list.
  if ( ! CheckBranchList(input) ) {
    LOG(FATAL) << "MCInputSet: Incompatible branch list!"
        << FairLogger::endl;
    return;
  } //? Branch list not compatible

  fInputs[inputId] = input;
  fInputHandle = fInputs.begin();
}
// ---------------------------------------------------------------------------



// -----   Check the branch list of an input   -------------------------------
Bool_t CbmMCInputSet::CheckBranchList(CbmMCInput* input) {

  assert(input);
  Bool_t success = kTRUE;
  for (auto const& entry : fBranches) {
    auto it = input->GetBranchList().find(entry);
    if ( it == input->GetBranchList().end() ) {
      LOG(DEBUG) << "MCInputSet: Required branch " << entry
          << " not present in input!" << FairLogger::endl;
      success = kFALSE;
      break;
    } //? Global branch not in input
  } //# Global branches

  if ( ! success ) {
    LOG(INFO) << "MCInputSet: Reference branch list is ";
    for (auto const& entry : fBranches ) LOG(INFO) << entry << " ";
    LOG(INFO) << FairLogger::endl;
    LOG(INFO) << "MCInputSet: Input branch list is ";
    for (auto const& entry : input->GetBranchList() ) LOG(INFO) << entry << " ";
    LOG(INFO) << FairLogger::endl;
  } //? Branches not compatible

  return success;
}
// ---------------------------------------------------------------------------



// -----   Time difference to next event   -----------------------------------
Double_t CbmMCInputSet::GetDeltaT() {
  if ( ! fDeltaDist ) return 0.;
  return fDeltaDist->GetRandom();
}
// ---------------------------------------------------------------------------



// -----   Get next entry from chain   ---------------------------------------
std::tuple<Bool_t, UInt_t, Int_t> CbmMCInputSet::GetNextEntry() {

  // Flag for having reached the last input
  Bool_t allInputsUsed = kFALSE;

  // The input handle points to the input to be used
  Int_t entry = fInputHandle->second->GetNextEntry();
  Int_t inputId = fInputHandle->first;

  // Increment input handle. If end of set reached, signal that and
  // reset the handle to the begin.
  fInputHandle++;
  if ( fInputHandle == fInputs.end() ) {
    allInputsUsed = kTRUE;
    fInputHandle = fInputs.begin();
  }

  return std::make_tuple(allInputsUsed, inputId, entry);

}
// ---------------------------------------------------------------------------



ClassImp(CbmMCInputSet)

