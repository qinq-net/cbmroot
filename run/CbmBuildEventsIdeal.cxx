/** @file CbmStsBuildEventsIdeal.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 17.09.2016
 **/

#include "CbmBuildEventsIdeal.h"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <map>
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "FairLogger.h"
#include "FairRootManager.h"
#include "CbmEvent.h"
#include "CbmLink.h"
#include "CbmMatch.h"
#include "CbmStsDigi.h"
#include "CbmTofDigiExp.h"

using namespace std;


// =====   Constructor   =====================================================
CbmBuildEventsIdeal::CbmBuildEventsIdeal() :
	FairTask("BuildEventsIdeal"),
	fDigis(),
	fMatches(),
	fEvents(nullptr),
	fNofEntries(0)
{
}
// ===========================================================================



// =====   Destructor   ======================================================
CbmBuildEventsIdeal::~CbmBuildEventsIdeal() {
}
// ===========================================================================



// =====   Task execution   ==================================================
void CbmBuildEventsIdeal::Exec(Option_t*) {

  TStopwatch timer;
  timer.Start();
  std::map<Int_t, CbmEvent*> eventMap;
  Int_t nEvents = 0;
  UInt_t nDigisTot = 0;
  UInt_t nDigisNoMatch = 0;
  UInt_t nDigisNoise = 0;

  // Clear output array
  fEvents->Delete();

  for ( auto it1 = fDigis.begin(); it1 != fDigis.end(); it1++ ) {

    Int_t system = it1->first;
    TClonesArray* digis = it1->second;
    assert(digis);
    TClonesArray* matches = nullptr;
    auto it2 = fMatches.find(system);
    if ( it2 != fMatches.end() ) {
      assert(it2->first == system);
      matches = it2->second;
    }

    UInt_t nDigis = digis->GetEntriesFast();
    if ( matches ) assert( matches->GetEntriesFast() == nDigis );
    UInt_t nNoise = 0;
    UInt_t nNoMatch = 0;

    for (UInt_t iDigi= 0; iDigi < nDigis; iDigi++) {
      CbmDigi* digi = dynamic_cast<CbmDigi*>(digis->At(iDigi));
      assert(digi);

      CbmMatch* match = nullptr;
      if ( matches ) match = dynamic_cast<CbmMatch*>(matches->At(iDigi));
      else match = digi->GetMatch();
      if ( ! match ) {
        LOG(DEBUG) << fName << ": no match for digi object from system "
            << system << "; ignoring digi" << FairLogger::endl;
        nNoMatch++;
        continue;
      }

      // This implementation uses only MC event number from
      // the matched link, i.e. that with the largest weight.
      // Can be refined later on.
      Int_t mcEventNr = digi->GetMatch()->GetMatchedLink().GetEntry();

      // Ignore digis with missing event number (noise)
      if ( mcEventNr < 0 ) {
        nNoise++;
        continue;
      }

      // Get event pointer. If event is not yet present, create it.
      CbmEvent* event = NULL;
      if ( eventMap.find(mcEventNr) == eventMap.end() ) {
        event = new ( (*fEvents)[nEvents] ) CbmEvent(nEvents);
        eventMap[mcEventNr] = event;
        nEvents++;
      }
      else event = eventMap.at(mcEventNr);

      // Fill digi index into event
      switch (system) {
        case kMvd:  event->AddData(kMvdDigi,  iDigi); break;
        case kSts:  event->AddData(kStsDigi,  iDigi); break;
        case kRich: event->AddData(kRichDigi, iDigi); break;
        case kMuch: event->AddData(kMuchDigi, iDigi); break;
        case kTrd:  event->AddData(kTrdDigi,  iDigi); break;
        case kTof:  event->AddData(kTofDigi,  iDigi); break;
        case kPsd:  event->AddData(kPsdDigi,  iDigi); break;
        break;
      } //? detector

    } //# digis
    LOG(DEBUG) << GetName() <<": Detector "
        << CbmModuleList::GetModuleNameCaps(system) << ", digis " << nDigis
        << ", no match " << nNoMatch << ", noise " << nNoise
        << FairLogger::endl;
    nDigisTot += nDigis;
    nDigisNoMatch += nNoMatch;
    nDigisNoise += nNoise;

  } //# detectors


  fNofEntries++;
  timer.Stop();
  assert( nEvents == fEvents->GetEntriesFast() );

  // --- Execution log
  std::cout << std::endl;
  LOG(INFO) << "+ " << setw(15) << GetName() << ": Time-slice " << setw(3)
             << right << fNofEntries << ", digis: " << nDigisTot << ", no match: "
             << nDigisNoMatch << ", noise: " << nDigisNoise << ". Exec time "
             << fixed << setprecision(6) << timer.RealTime() << " s."
             << FairLogger::endl;

  // --- For debug: event info
  if (gLogger->IsLogNeeded(DEBUG)) {
    for (Int_t iEvent = 0; iEvent < fEvents->GetEntriesFast(); iEvent++ ) {
      CbmEvent* event = (CbmEvent*) fEvents->At(iEvent);
      LOG(INFO) << event->ToString() << FairLogger::endl;
    }
  }

}
// ===========================================================================



// =====   Task initialisation   =============================================
InitStatus CbmBuildEventsIdeal::Init() {

	// --- Get FairRootManager instance
  FairRootManager* ioman = FairRootManager::Instance();
  assert ( ioman );

  // --- Get input arrays (digi)
  for (Int_t detector = kMvd; detector <= kNofSystems; detector++) {

    TString branchName = "";
    switch ( detector ) {
      case kMvd:  branchName = "MvdDigi";  break;
      case kSts:  branchName = "StsDigi";  break;
      case kRich: branchName = "RichDigi"; break;
      case kMuch: branchName = "MuchDigi"; break;
      case kTrd:  branchName = "TrdDigi";  break;
      case kTof:  branchName = "TofDigi";  break;
      case kPsd:  branchName = "PsdDigi";  break;
      default: break;
    } //? detector

    if ( ! branchName.IsNull() ) {
      TClonesArray* testDigi =
          dynamic_cast<TClonesArray*>(ioman->GetObject(branchName.Data()));
      if ( testDigi ) {
        fDigis[detector] = testDigi;
        LOG(INFO) << GetName()
          << ": found input branch " << branchName << FairLogger::endl;
      } //? digi branch is present

      TString matchBranch = branchName + "Match";
      TClonesArray* testMatch =
          dynamic_cast<TClonesArray*>(ioman->GetObject(matchBranch.Data()));
      if ( testMatch ) {
        fMatches[detector] = testMatch;
        LOG(INFO) << GetName()
          << ": found input branch " << branchName << FairLogger::endl;
      } //? digi branch is present

    } //? branch name is known

  } //# systems


  // Register output array (CbmEvent)
  fEvents = new TClonesArray("CbmEvent",100);
  ioman->Register("Event", "CbmEvent", fEvents,
  			 	        IsOutputBranchPersistent("Event"));
  if ( ! fEvents ) LOG(FATAL) << "Output branch was not created" << FairLogger::endl;

  return kSUCCESS;
}
// ===========================================================================


ClassImp(CbmBuildEventsIdeal)

