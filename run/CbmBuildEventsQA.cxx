/** @file CbmStsBuildEventsQA.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 20.09.2016
 **/

#include "CbmBuildEventsQA.h"

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
CbmBuildEventsQA::CbmBuildEventsQA() :
	FairTask("BuildEventsQA"),
	fStsDigis(NULL),
	fEvents(NULL),
	fNofEntries(0)
{
}
// ===========================================================================



// =====   Destructor   ======================================================
CbmBuildEventsQA::~CbmBuildEventsQA() {
}
// ===========================================================================



// =====   Task execution   ==================================================
void CbmBuildEventsQA::Exec(Option_t* opt) {

	// --- Time and counters
	TStopwatch timer;
	timer.Start();
	Int_t nMCEvents     = 0;

	// --- Event loop
	Int_t nEvents = fEvents->GetEntriesFast();
	for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
		CbmEvent* event = (CbmEvent*) fEvents->At(iEvent);

		// --- Match event to MC
		MatchEvent(event);
		Int_t mcEventNr = event->GetMatch()->GetMatchedLink().GetEntry();
		LOG(INFO)  << GetName() << ": Event " << event->GetNumber()
				       << ", data objects : " << event->GetNofData()
				       << ", links: " << event->GetMatch()->GetNofLinks()
				       << ", matched MC event number " << mcEventNr
				       << FairLogger::endl;

		// --- Counters
		Int_t nDigis        = event->GetNofData(Cbm::kStsDigi);
		Int_t nDigiCorrect  = 0;
		Int_t nLinks        = 0;
		Int_t nLinksCorrect = 0;
		nMCEvents += event->GetMatch()->GetNofLinks();

		// --- Loop over STS digis
		for (Int_t iDigi = 0; iDigi < nDigis; iDigi++) {
			Int_t index = event->GetIndex(Cbm::kStsDigi, iDigi);
			CbmStsDigi* digi = (CbmStsDigi*) fStsDigis->At(index);
			assert(digi);

			// --- Check MC event of digi match
			if ( digi->GetMatch()->GetMatchedLink().GetEntry() == mcEventNr )
				nDigiCorrect++;

			for (Int_t iLink = 0; iLink < digi->GetMatch()->GetNofLinks(); iLink++) {
				Int_t entry = digi->GetMatch()->GetLink(iLink).GetEntry();
				nLinks++;
				if ( entry == mcEventNr ) nLinksCorrect++;
			} //# links in digi

		} //# digis


		// --- QA output
		LOG(INFO) << GetName()
				<< ": correct digis " << nDigiCorrect << " / " << nDigis
				<< " = " << 100.*Double_t(nDigiCorrect)/Double_t(nDigis)
				<< " %, correct digi links " << nLinksCorrect << " / "
				<< nLinks << " = " << 100.*Double_t(nLinksCorrect)/Double_t(nLinks)
				<< " % " << FairLogger::endl;

	} //# events


	// Timer and counters
	fNofEntries++;
	timer.Stop();

  // --- Execution log
  LOG(INFO) << "+ " << setw(20) << GetName() << ": Entry " << setw(6)
  		      << right << fNofEntries << ", real time " << fixed
  		      << setprecision(6) << timer.RealTime() << " s, events: "
  		      << fEvents->GetEntriesFast() << ", MC events: "
  		      << nMCEvents << FairLogger::endl;

}
// ===========================================================================



// =====   Task initialisation   =============================================
InitStatus CbmBuildEventsQA::Init() {

	// --- Get FairRootManager instance
  FairRootManager* ioman = FairRootManager::Instance();
  assert ( ioman );

  // --- Get input array (CbmEvent)
  fEvents = (TClonesArray*) ioman->GetObject("Event");
  assert ( fEvents );

  // --- Get input array (CbmStsDigi)
  fStsDigis = (TClonesArray*) ioman->GetObject("StsDigi");
  assert ( fStsDigis );

  return kSUCCESS;
}
// ===========================================================================


// =====   Match event   =====================================================
void CbmBuildEventsQA::MatchEvent(CbmEvent* event) {

	// TODO: This functionality should later be moved to the class
	// CbmMatchRecoToMC

	// --- Get event match object. If present, will be cleared first. If not,
	// --- it will be created.
	CbmMatch* match = event->GetMatch();
	if ( ! match ) {
		match = new CbmMatch();
		event->SetMatch(match);
	} //? event has no match

	// --- Loop over digis
	for (Int_t iDigi = 0; iDigi < event->GetNofData(Cbm::kStsDigi); iDigi++) {
		Int_t index = event->GetIndex(Cbm::kStsDigi, iDigi);
		CbmDigi* digi = (CbmStsDigi*) fStsDigis->At(index);
		assert(digi);

		// --- Skip if digi does not carry a match object
		if ( ! digi->GetMatch() ) continue;

		// --- Update event match with digi links
		// --- N.b.: The member "index" of CbmLink has here no meaning, since
		// --- there is only one MC event per tree entry.
		for (Int_t iLink = 0; iLink < digi->GetMatch()->GetNofLinks(); iLink++) {
			Int_t file = digi->GetMatch()->GetLink(iLink).GetFile();
			Int_t entry = digi->GetMatch()->GetLink(iLink).GetEntry();
			Double_t weight = digi->GetMatch()->GetLink(iLink).GetWeight();
			match->AddLink(weight, 0, entry, file);
		} //# links in digi

	} //#digis

}
// ===========================================================================


ClassImp(CbmBuildEventsQA)

