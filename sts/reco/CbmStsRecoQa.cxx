/*
 * CbmStsTestQa.cxx
 *
 *  Created on: 07.11.2016
 *      Author: vfriese
 */

#include "CbmStsRecoQa.h"

#include <cassert>
#include <fstream>
#include <iomanip>
#include "TClonesArray.h"
#include "TStopwatch.h"
#include "FairLogger.h"
#include "CbmEvent.h"
#include "CbmHistManager.h"
#include "CbmStsAddress.h"
#include "CbmStsCluster.h"
#include "CbmStsHit.h"
#include "CbmStsTrack.h"
#include "CbmVertex.h"

using namespace std;

CbmStsRecoQa::CbmStsRecoQa() :
	fEvents(NULL),
	fTracks(NULL),
	fNofTs(0),
	fNofEvents(0),
	fNofTracksTot(0.),
	fNofGoodTracks(0.),
	fNofHitsTot(0.),
	fPTot(0.),
	fTimeTot(0.)
{
	SetName("StsRecoQa");
}

CbmStsRecoQa::~CbmStsRecoQa() {
}



// -----   Execution   -----------------------------------------------------
void CbmStsRecoQa::Exec(Option_t* /*opt*/) {

	// If there is an event branch: do the event loop
	if ( fEvents ) {
		Int_t nEvents = fEvents->GetEntriesFast();
		LOG(DEBUG) << GetName() << ": found time slice with " << nEvents
				   << " events." << FairLogger::endl;

		for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
			CbmEvent* event = dynamic_cast<CbmEvent*>(fEvents->At(iEvent));
			assert(event);
			ProcessEvent(event);
			fNofEvents++;
		}
	}

	// If there is no event brnahc, process the entire tree
	else {
 	  ProcessEvent();
	  fNofEvents++;
	}

	fNofTs++;
}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsRecoQa::Finish() {
  std::cout << std::endl;
  LOG(INFO) << "=====================================" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
  LOG(INFO) << "Events processed    : " << fNofEvents << FairLogger::endl;
  LOG(INFO) << "Tracks / event      : " << fixed << setprecision(3)
                << fNofTracksTot / Double_t(fNofEvents) << FairLogger::endl;
  LOG(INFO) << "Good tracks / event : " << fixed << setprecision(3)
                << fNofGoodTracks / Double_t(fNofEvents) << FairLogger::endl;
  LOG(INFO) << "Av. hits / track    : " << fixed << setprecision(3)
                << fNofHitsTot / Double_t(fNofTracksTot)
                << FairLogger::endl;
  LOG(INFO) << "Average momentum    : " << fixed << setprecision(3)
            << fPTot / Double_t(fNofGoodTracks) << " GeV"
            << FairLogger::endl;
  LOG(INFO) << "Time per event      : " << fixed << setprecision(6)
            << fTimeTot / Double_t(fNofEvents) << " s"
            << " " << FairLogger::endl;
  LOG(INFO) << "=====================================" << FairLogger::endl;


}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsRecoQa::Init()
{
    // --- Check IO-Manager
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {
    	LOG(ERROR) << GetName() << ": No FairRootManager!"
    			       << FairLogger::endl;
    	return kFATAL;
    }

    // --- Get input array (events)
    fEvents = dynamic_cast<TClonesArray*>(ioman->GetObject("Event"));

    // --- Get input array (tracks)
	fTracks = dynamic_cast<TClonesArray*>(ioman->GetObject("StsTrack"));
	assert(fTracks);

	return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Process one event   ---------------------------------------------
void CbmStsRecoQa::ProcessEvent(CbmEvent* event) {

 	// Timer
	TStopwatch timer;
	timer.Start();
	Int_t eventNr = ( event ? event->GetNumber() : fNofEvents );
	Int_t nTracks = ( event ? event->GetNofData(kStsTrack)
	    : fTracks->GetEntriesFast() );
	LOG(DEBUG) << GetName() << ": event " << eventNr << ", STS tracks: "
	    << nTracks << FairLogger::endl;

	// Track loop
	for (Int_t iTrack = 0; iTrack < nTracks; iTrack++) {
	  Int_t index = ( event ? event->GetIndex(kStsTrack, iTrack) : iTrack );
		CbmStsTrack* track = dynamic_cast<CbmStsTrack*>(fTracks->At(index));
		assert(track);

		Int_t nHits = track->GetNofHits();
		Double_t qp = TMath::Abs(track->GetParamFirst()->GetQp());
		if ( qp > 0.01) {
		  fPTot += 1. / qp;
		  fNofGoodTracks++;
		}

//		Double_t p = 1. / TMath::Abs( track->GetParamFirst()->GetQp());

		fNofHitsTot += Double_t(nHits);
		fNofTracksTot++;
	}


	// Event log
	timer.Stop();
	LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
	  		      << right << eventNr
	  		      << ", real time " << fixed << setprecision(6)
	  		      << timer.RealTime() << " s, tracks: " << nTracks
				  << FairLogger::endl;
	fTimeTot += timer.RealTime();

}
// -------------------------------------------------------------------------

ClassImp(CbmStsRecoQa)

