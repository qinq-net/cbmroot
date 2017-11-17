/** @file CbmStsFindHitsEvents.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @sine 16.06.2014
 ** @date 21.10.2016
 **/

#include "reco/CbmStsFindHitsEvents.h"

#include <cassert>
#include <iostream>
#include <iomanip>

#include "TClonesArray.h"
#include "FairEventHeader.h"
#include "FairRunAna.h"
#include "CbmEvent.h"
#include "setup/CbmStsSetup.h"

using namespace std;


// -----   Constructor   ---------------------------------------------------
CbmStsFindHitsEvents::CbmStsFindHitsEvents()
    : FairTask("StsFindHits", 1)
    , fEvents(NULL)
    , fClusters(NULL)
    , fHits(NULL)
    , fSetup(NULL)
    , fTimer()
    , fDTime(0.)
    , fNofEvents(0.)
    , fNofClustersTot(0.)
    , fNofHitsTot(0.)
    , fTimeTot(0.)
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmStsFindHitsEvents::~CbmStsFindHitsEvents() {
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindHitsEvents::Exec(Option_t* /*opt*/) {

  // --- Clear output arrays
	fHits->Delete();

    // --- Event loop (from event objects)
    if ( fEvents ) {
      Int_t nEvents = fEvents->GetEntriesFast();
      LOG(DEBUG) << GetName() << ": reading time slice with " << nEvents
          << " events " << FairLogger::endl;
      for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
        CbmEvent* event = static_cast<CbmEvent*> (fEvents->At(iEvent));
        ProcessEvent(event);
      } //# events
    } //? event branch present

    else // Old event-by-event simulation without event branch
      ProcessEvent(NULL);

}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsFindHitsEvents::Finish() {
	std::cout << std::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	LOG(INFO) << "Events processed  : " << fNofEvents << FairLogger::endl;
	LOG(INFO) << "Clusters / event  : "
			      << fNofClustersTot / Double_t(fNofEvents) << FairLogger::endl;
	LOG(INFO) << "Hits / event      : "
			      << fNofHitsTot / Double_t(fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "Hits per cluster  : " << fNofHitsTot / fNofClustersTot
			      << FairLogger::endl;
	LOG(INFO) << "Time per event    : " << fTimeTot / Double_t(fNofEvents)
			      << " " << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;

}
// -------------------------------------------------------------------------


// -----   End-of-event action   -------------------------------------------
void CbmStsFindHitsEvents::FinishEvent() {

	// --- Clear cluster sets for all active modules
	Int_t nModules = 0;
	for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
		CbmStsModule* module = fSetup->GetModule(iModule);
		if ( module->GetNofClusters() == 0 ) continue;
		module->ClearClusters();
		nModules++;
	}

	LOG(DEBUG) << GetName() << ": Cleared clusters in " << nModules
			       << " modules. " << FairLogger::endl;
}
// -------------------------------------------------------------------------


// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsFindHitsEvents::Init()
{
    // --- Check IO-Manager
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {
    	LOG(ERROR) << GetName() << ": No FairRootManager!"
    			       << FairLogger::endl;
    	return kFATAL;
    }

    // --- Get input array (Events)
    fEvents = dynamic_cast<TClonesArray*>(ioman->GetObject("Event"));
    if ( ! fEvents ) {
      LOG(WARNING) << GetName()
          << ": No event array! Will process entire tree."
          << FairLogger::endl;
    }

    // --- Get input array (StsCluster)
    fClusters = dynamic_cast<TClonesArray*>(ioman->GetObject("StsCluster"));
    if ( ! fClusters) {
    	LOG(ERROR) << GetName() << ": No StsCluster array!" << FairLogger::endl;
    	return kERROR;
    }

    // --- Register output array
    fHits = new TClonesArray("CbmStsHit", 10000);
    ioman->Register("StsHit", "Hits in STS", fHits,
    		            IsOutputBranchPersistent("StsHit"));

    // --- Get STS setup
    fSetup = CbmStsSetup::Instance();

    LOG(INFO) << GetName() << ": Initialisation successful"
	            << FairLogger::endl;

    return kSUCCESS;
}
// -------------------------------------------------------------------------


// -----   Event processing   ----------------------------------------------
Int_t CbmStsFindHitsEvents::ProcessEvent(CbmEvent* event) {

	// Start timer and counter
	fTimer.Start();
	Int_t nHitsEvent = 0;

	// --- Sort clusters into modules
	Int_t nClusters = SortClusters(event);

	// --- Find hits in modules
	for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
		CbmStsModule* module = fSetup->GetModule(iModule);
		if ( module->GetNofClusters() == 0 ) continue;
		if ( fDTime ) module->SetDeadTime(fDTime);
		Int_t nHitsModule = module->FindHits(fHits, event);
		LOG(DEBUG1) << GetName() << ": Module " << module->GetName()
    			      << ", clusters: " << module->GetNofClusters()
   		          << ", hits: " << nHitsModule << FairLogger::endl;
		nHitsEvent += nHitsModule;
	}

  // --- Counters
  fTimer.Stop();
  fNofEvents++;
  fNofClustersTot += nClusters;
  fNofHitsTot     += nHitsEvent;
  fTimeTot        += fTimer.RealTime();

  LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
  		      << right << (event ? event->GetNumber() : 0) << ", real time " << fixed
  		      << setprecision(6) << fTimer.RealTime() << " s, clusters: "
  		      << nClusters << ", hits: " << nHitsEvent << FairLogger::endl;

  // --- End-of-event action (clear cluster maps of modules)
  FinishEvent();

  return nHitsEvent;
}
// -------------------------------------------------------------------------


// ----- Sort clusters into module cluster maps   --------------------------------
Int_t CbmStsFindHitsEvents::SortClusters(CbmEvent* event) {

	// --- Counters
    Int_t nClusters = 0;
    if ( event ) nClusters = event->GetNofData(kStsCluster);
    else         nClusters = fClusters->GetEntriesFast();
    LOG(INFO) << GetName() << ": event " << (event ? event->GetNumber() : 0)
      << ", clusters " << nClusters << FairLogger::endl;

	// --- Loop over clusters in event
	for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {
      UInt_t index = (event ? event->GetIndex(kStsCluster, iCluster) : iCluster);
		CbmStsCluster* cluster =
				static_cast<CbmStsCluster*> (fClusters->At(index));
		assert(cluster);

		UInt_t address = cluster->GetAddress();
		cluster->SetIndex(index);
		CbmStsModule* module =
				static_cast<CbmStsModule*>(fSetup->GetElement(address, kStsModule));

		// --- Assign cluster to module
		module->AddCluster(cluster);

	}  //# clusters in event

	// --- Debug output
	if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG) ) {
		Int_t nActiveModules = 0;
		for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
			CbmStsModule* module = fSetup->GetModule(iModule);
			if ( module->GetNofClusters() == 0 ) continue;
			nActiveModules++;
			LOG(DEBUG3) << GetName() << ": Module " << module->GetName()
						<< ", clusters " << module->GetNofClusters()
						<< FairLogger::endl;
		} //# modules in setup
		LOG(DEBUG) << GetName() << ": sorted " << nClusters << " clusters into "
				   << nActiveModules << " module(s)." << FairLogger::endl;
	} //? DEBUG

	return nClusters;
}
// -------------------------------------------------------------------------

ClassImp(CbmStsFindHitsEvents)
