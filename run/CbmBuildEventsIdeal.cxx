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
	fEvents(NULL),
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

	// Clear output array
	fEvents->Delete();

	for (Int_t detector = kMvd; detector < kNofSystems; detector++) {

	  if ( fDigis[detector] == NULL ) continue;
	  UInt_t nDigis = fDigis[detector]->GetEntriesFast();
	  UInt_t nNoise = 0;

	  for (UInt_t iDigi= 0; iDigi < nDigis; iDigi++) {
	    CbmDigi* digi = dynamic_cast<CbmDigi*>(fDigis[detector]->At(iDigi));
	    assert(digi);

	    // This implementation uses only MC event number from
        // the matched link, i.e. that with the largest weight.
        // Can be refined later on.
	    Int_t eventNr = digi->GetMatch()->GetMatchedLink().GetEntry();

        // Ignore digis with missing event number (noise)
        if ( eventNr < 0 ) {
          nNoise++;
          continue;
        }

        // Get event pointer. If event is not yet present, create it.
        CbmEvent* event = NULL;
        if ( eventMap.find(eventNr) == eventMap.end() ) {
            event = new ( (*fEvents)[eventNr] ) CbmEvent(eventNr);
            eventMap[eventNr] = event;
        }
        else event = eventMap.at(eventNr);

        // Fill digi index into event
        switch (detector) {
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
	  LOG(DEBUG) << GetName() <<": ignored " << nNoise << " digis from "
	      << CbmModuleList::GetModuleNameCaps(detector)
	      << FairLogger::endl;

	} //# detector


	fNofEntries++;
	timer.Stop();

  // --- Execution log
  std::cout << std::endl;
  LOG(INFO) << "+ " << setw(20) << GetName() << ": Entry " << setw(6)
  		      << right << fNofEntries << ", real time " << fixed
  		      << setprecision(6) << timer.RealTime() << " s, events: "
  		      << fEvents->GetEntriesFast()
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
      case kTof:  branchName = "TofDigiExp";  break;
      case kPsd:  branchName = "PsdDigi";  break;
      default: break;
    } //? detector

    if ( ! branchName.IsNull() ) {
      fDigis[detector] =
          dynamic_cast<TClonesArray*>(ioman->GetObject(branchName.Data()));
      if ( fDigis[detector] != NULL ) LOG(INFO) << GetName()
          << ": found input branch " << branchName << FairLogger::endl;
    } //? branch name exists

  } //# systems

  // Register output array (CbmEvent)
  fEvents = new TClonesArray("CbmEvent",100);
  ioman->Register("Event", "CbmEvent", fEvents,
  			 	        IsOutputBranchPersistent("Event"));

  return kSUCCESS;
}
// ===========================================================================


ClassImp(CbmBuildEventsIdeal)

