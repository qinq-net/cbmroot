/** @file CbmFindTracksEvent.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 ** @date 23.10.2016
 **/


// Includes from STS
#include "CbmStsFindTracksEvents.h"

#include <cassert>
#include "CbmEvent.h"

#include "CbmStsHit.h"
#include "CbmStsTrack.h"
#include "CbmStsTrackFinderIdeal.h"

// Includes from base
#include "FairField.h"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// Includes from ROOT
#include "TClonesArray.h"

// Includes from C++
#include <iostream>
#include <iomanip>

using std::cout;
using std::endl;
using std::right;
using std::left;
using std::fixed;
using std::setw;
using std::setprecision;

// -----   Standard constructor   ------------------------------------------
CbmStsFindTracksEvents::CbmStsFindTracksEvents(CbmStsTrackFinder* finder,
				                                       Bool_t useMvd)
  : FairTask("StsFindTracks"),
    fUseMvd(useMvd),
    fFinder(finder),
    fEvents(NULL),
    fMvdHits(NULL),
    fStsHits(NULL),
    fTracks(NULL),
    fTimer(),
    fNofEvents(0),
    fNofHits(0.),
    fNofTracks(0.),
    fTime(0.)
{
  if ( ! finder )  fFinder = new CbmStsTrackFinderIdeal();
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsFindTracksEvents::~CbmStsFindTracksEvents() {
  fTracks->Delete();
  if ( fFinder) delete fFinder;
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindTracksEvents::Exec(Option_t* opt) {

	// --- Clear output array
	fTracks->Delete();

	// --- Event loop
	Int_t nEvents = fEvents->GetEntriesFast();
	LOG(DEBUG) << GetName() << ": reading time slice with " << nEvents
			      << " events " << FairLogger::endl;
	for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
		CbmEvent* event = static_cast<CbmEvent*> (fEvents->At(iEvent));

		// --- Call track finder
		fTimer.Start();
		Int_t nTracks = fFinder->FindTracks(event);
		fTimer.Stop();

		// --- Event log
	  LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
	  		      << right << event->GetNumber()
	  		      << ", real time " << fixed << setprecision(6)
	  		      << fTimer.RealTime() << " s, hits: "
	  		      << event->GetNofData(Cbm::kStsHit) << ", tracks: "
	  		      << nTracks << FairLogger::endl;
	  LOG(DEBUG) << event->ToString() << FairLogger::endl;

	  // --- Counters
	  fNofEvents++;
	  fNofHits   += Double_t(event->GetNofData(Cbm::kStsHit));
	  fNofTracks += Double_t(nTracks);
	  fTime      += fTimer.RealTime();

	} //# event loop

}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsFindTracksEvents::Init() {

	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": initialising" << FairLogger::endl;

  // I/O manager
  FairRootManager* ioman = FairRootManager::Instance();
  assert(ioman);

  // Event array
  fEvents = (TClonesArray*) ioman->GetObject("Event");
  assert(fEvents);

  // Array of StsHits
  fStsHits = (TClonesArray*) ioman->GetObject("StsHit");
  assert(fStsHits);

  // Array of MvdHits
  if ( fUseMvd ) {
  	LOG(INFO) << GetName() << ": including MVD hits in tracking"
              << FairLogger::endl;
    fMvdHits = (TClonesArray*) ioman->GetObject("MvdHit");
    assert(fMvdHits);
  }

  // Create and register output array for StsTracks
  fTracks = new TClonesArray("CbmStsTrack",100);
  ioman->Register("StsTrack", "STS", fTracks,
  		            IsOutputBranchPersistent("StsTrack"));

  // Check for Track finder
  if (! fFinder) {
  	LOG(FATAL) << GetName() << ": no track finding engine selected!"
  			       << FairLogger::endl;
    return kERROR;
  }
  LOG(INFO) << GetName() << ": Use track finder " << fFinder->GetName()
  		      << FairLogger::endl;

  // Set members of track finder and initialise it
  fFinder->SetMvdHitArray(fMvdHits);
  fFinder->SetStsHitArray(fStsHits);
  fFinder->SetTrackArray(fTracks);
  fFinder->Init();

  // Screen output
  LOG(INFO) << GetName() << ": successfully initialised."
  		      << FairLogger::endl;
	LOG(INFO) << "====================================="
			      << FairLogger::endl << FairLogger::endl;

	return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsFindTracksEvents::Finish() {
	std::cout << std::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	LOG(INFO) << "Events processed   : " << fNofEvents << FairLogger::endl;
	LOG(INFO) << "Hits / event       : " << fNofHits / Double_t(fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "Tracks / event     : "
			      << fNofTracks / Double_t(fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "Hits per track     : " << fNofHits / fNofTracks
			      << FairLogger::endl;
	LOG(INFO) << "Time per event     : " << fTime / Double_t(fNofEvents)
			      << " s " << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
}
// -------------------------------------------------------------------------



ClassImp(CbmStsFindTracksEvents)
