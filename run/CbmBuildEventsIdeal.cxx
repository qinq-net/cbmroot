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
#include "CbmDetectorList.h"
#include "CbmEvent.h"
#include "CbmLink.h"
#include "CbmMatch.h"
#include "CbmStsDigi.h"

using namespace std;


// =====   Constructor   =====================================================
CbmBuildEventsIdeal::CbmBuildEventsIdeal() :
	FairTask("BuildEventsIdeal"),
	fStsDigis(NULL),
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

	UInt_t nDigis = fStsDigis->GetEntriesFast();
	LOG(DEBUG) << GetName() << ": found " << nDigis << " digis "
			       << FairLogger::endl;
	for (UInt_t iDigi = 0; iDigi < nDigis; iDigi++) {
		CbmStsDigi* digi = (CbmStsDigi*) fStsDigis->At(iDigi);

		// This implementation uses only MC event number from
		// the matched link, i.e. that with the largest weight.
		// Can be refined later on.
		Int_t eventNr = digi->GetMatch()->GetMatchedLink().GetEntry();

		// Get event pointer. If event is not yet present, create it.
		CbmEvent* event = NULL;
		if ( eventMap.find(eventNr) == eventMap.end() ) {
			Int_t nEvents = fEvents->GetEntriesFast();
			event = new ( (*fEvents)[nEvents] ) CbmEvent(eventNr);
			eventMap[eventNr] = event;
		}
		else event = eventMap.at(eventNr);

		// Fill digi index into event
		event->AddData(Cbm::kStsDigi, iDigi);

	} //# digis


	fNofEntries++;
	timer.Stop();

  // --- Execution log
  std::cout << std::endl;
  LOG(INFO) << "+ " << setw(20) << GetName() << ": Entry " << setw(6)
  		      << right << fNofEntries << ", real time " << fixed
  		      << setprecision(6) << timer.RealTime() << " s, digis: "
  		      << nDigis << ", events: " << fEvents->GetEntriesFast()
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

  // --- Get input array (CbmStsDigi)
  fStsDigis = (TClonesArray*) ioman->GetObject("StsDigi");
  assert ( fStsDigis );

  // Register output array (CbmStsDigi)
  fEvents = new TClonesArray("CbmEvent",100);
  ioman->Register("Event", "CbmEvent", fEvents,
  			 	        IsOutputBranchPersistent("Event"));

  return kSUCCESS;
}
// ===========================================================================


ClassImp(CbmBuildEventsIdeal)

