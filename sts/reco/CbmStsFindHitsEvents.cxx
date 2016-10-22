/** @file CbmStsFindHitsEvents.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @sine 16.06.2014
 ** @date 21.10.2016
 **/

#include "reco/CbmStsFindHitsEvents.h"

#include <cassert>
#include <iostream>
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
    , fActiveModules()
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmStsFindHitsEvents::~CbmStsFindHitsEvents() {
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindHitsEvents::Exec(Option_t* opt) {

  // --- Clear output arrays
	fHits->Delete();

	// --- Event loop
	Int_t nEvents = fEvents->GetEntriesFast();
	LOG(DEBUG) << GetName() << ": reading time slice with " << nEvents
			      << " events " << FairLogger::endl;
	for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
		CbmEvent* event = static_cast<CbmEvent*> (fEvents->At(iEvent));
		ProcessEvent(event);
	}

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
	set<CbmStsModule*>::iterator it;
	for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
		(*it)->ClearClusters();
		nModules++;
	}
	fActiveModules.clear();

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
    fEvents = (TClonesArray*) ioman->GetObject("Event");
		if (NULL == fEvents) {
			LOG(FATAL) << GetName() << ": No event array!" << FairLogger::endl;
			return kERROR;
		}

    // --- Get input array (StsCluster)
    fClusters = (TClonesArray*)ioman->GetObject("StsCluster");
    if (NULL == fClusters) {
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
	Int_t nHits = 0;
	set<CbmStsModule*>::iterator it;
	for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
		CbmStsModule* module = (*it);
		if ( ! module ) {
			LOG(FATAL) << GetName() << ": Non-valid module pointer in vector!"
					       << FairLogger::endl;
			continue;
		}
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
  		      << right << event->GetNumber() << ", real time " << fixed
  		      << setprecision(6) << fTimer.RealTime() << " s, clusters: "
  		      << nClusters << ", hits: " << nHitsEvent << FairLogger::endl;

  // --- End-of-event action (clear cluster maps of modules)
  FinishEvent();

  return nHitsEvent;
}
// -------------------------------------------------------------------------


// ----- Sort digis into module digi maps   --------------------------------
Int_t CbmStsFindHitsEvents::SortClusters(CbmEvent* event) {

	// --- Counters
	Int_t nClusters = event->GetNofData(Cbm::kStsCluster);

	// --- Loop over clusters in event
	for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {
		UInt_t index = event->GetIndex(Cbm::kStsCluster, iCluster);
		CbmStsCluster* cluster =
				static_cast<CbmStsCluster*> (fClusters->At(index));
		assert(cluster);

		UInt_t address = cluster->GetAddress();
		cluster->SetIndex(index);
		CbmStsModule* module =
				static_cast<CbmStsModule*>(fSetup->GetElement(address, kStsModule));

	  // --- Update set of active modules
		fActiveModules.insert(module);

		// --- Assign cluster to module
		module->AddCluster(cluster);

	}  //# clusters in event

	// --- Debug output
	LOG(DEBUG) << GetName() << ": sorted " << nClusters << " clusters into "
			       << fActiveModules.size() << " module(s)." << FairLogger::endl;
	if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG3) ) {
		set <CbmStsModule*>::iterator it;
		for (it = fActiveModules.begin(); it != fActiveModules.end() ; it++) {
			CbmStsModule* module = (*it);
			LOG(DEBUG3) << GetName() << ": Module " << module->GetName()
						      << ", clusters " << module->GetNofClusters()
						      << FairLogger::endl;
		}  // active module loop
	}  //? DEBUG 3

	return nClusters;
}
// -------------------------------------------------------------------------

ClassImp(CbmStsFindHitsEvents)
