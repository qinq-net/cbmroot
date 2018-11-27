/** @file CbmMCInput.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 09.11.2018
 **/

#include "CbmMCInput.h"

#include <cassert>
#include "TFile.h"
#include "TList.h"
#include "TObjString.h"
#include "TRandom.h"
#include "FairMCEventHeader.h"
#include "FairLogger.h"


// -----   Default constructor   ---------------------------------------------
CbmMCInput::CbmMCInput() : CbmMCInput(nullptr, Cbm::kRegular) {
}
// ---------------------------------------------------------------------------



// -----   Constructor   -----------------------------------------------------
CbmMCInput::CbmMCInput(TChain* chain, Cbm::ETreeAccess mode) :
        TObject(),
        fChain(chain),
        fMode(mode),
        fBranches(),
        fLastUsedEntry(-1),
        fNofUsedEntries(0) {
}
// ---------------------------------------------------------------------------



// -----   Destructor   ------------------------------------------------------
CbmMCInput::~CbmMCInput() {
}
// ---------------------------------------------------------------------------



// -----   Get branch list   -------------------------------------------------
std::set<TString>& CbmMCInput::GetBranchList() {

  // At first call, read branch list from file
  if ( fBranches.empty() ) ReadBranches();

  return fBranches;
}
// ---------------------------------------------------------------------------



// -----   Get next entry from chain   ---------------------------------------
Int_t CbmMCInput::GetNextEntry() {

  assert(fChain);

  // Determine entry number to be retrieved
  Int_t entry = -1;
  if ( fMode == Cbm::kRandom ) {
    entry = gRandom->Integer(GetNofEntries());
  } //? Random entry number
  else {
    entry = fLastUsedEntry + 1;
    if ( entry >= GetNofEntries() ) entry = ( fMode == Cbm::kRepeat ? 0 : -1 );
  } //? Sequential entry number

  // Stop run when entryId is -1. This happens when mode is kRegular and
  // the end of the chain is reached.
  if ( entry < 0 ) return -1;

  // Get entry from chain
  assert(entry >= 0 && entry < GetNofEntries());  // Just to make sure...
  Int_t nBytes = fChain->GetEntry(entry);
  if (nBytes <= 0) LOG(WARNING) << "InputChain: " << nBytes
      << " Bytes read from tree!" << FairLogger::endl;
  fLastUsedEntry = entry;
  fNofUsedEntries++;

  return entry;

}
// ---------------------------------------------------------------------------



// -----   Get list of data branches from file   -----------------------------
UInt_t CbmMCInput::ReadBranches() {

  fBranches.clear();
  TList* listFile =
      dynamic_cast<TList*>(fChain->GetFile()->Get("BranchList"));
  assert(listFile);
  TObjString* branchName = nullptr;
  for (Int_t entry = 0; entry < listFile->GetEntries(); entry++) {
    branchName = dynamic_cast<TObjString*>(listFile->At(entry));
    assert(branchName);
    fBranches.insert(branchName->GetString());
  } //# Entries in branch list

  return fBranches.size();
}
// ---------------------------------------------------------------------------


ClassImp(CbmMCInput)

