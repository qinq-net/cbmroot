/** @file CbmStsFindHitsSingleCluster.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 16.06.2014
 **/

#include "CbmStsFindHitsSingleCluster.h"

#include <iomanip>
#include <iostream>
#include "TClonesArray.h"
#include "FairEventHeader.h"
#include "FairRunAna.h"
#include "CbmStsSetup.h"

using namespace std;


// -----   Constructor   ---------------------------------------------------
CbmStsFindHitsSingleCluster::CbmStsFindHitsSingleCluster()
    : FairTask("StsFindHitsSingleCluster", 1)
    , fClusters(NULL)
    , fHits(NULL)
    , fSetup(NULL)
    , fTimer()
    , fNofTimeSlices(0.)
    , fNofClustersTot(0.)
    , fNofHitsTot(0.)
    , fTimeTot(0.)
    , fActiveModules()
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmStsFindHitsSingleCluster::~CbmStsFindHitsSingleCluster() {
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindHitsSingleCluster::Exec(Option_t*) {

	// --- Time-slice number
	Int_t iEvent = fNofTimeSlices;

	// Start timer and counter
	fTimer.Start();

    // --- Clear output arrays
	fHits->Delete();

	// --- Sort clusters into modules
	Int_t nClusters = SortClusters();

	// --- Find hits in modules
	Int_t nHits = 0;
	set<CbmStsModule*>::iterator it;
	//for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
	for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
		CbmStsModule* module = fSetup->GetModule(iModule);
		if ( module->GetNofClusters() == 0 ) continue;
		Int_t nModuleHits = module->MakeHitsFromClusters(fHits);
		LOG(DEBUG1) << GetName() << ": Module " << module->GetName()
    			      << ", clusters: " << module->GetNofClusters()
   		          << ", hits: " << nModuleHits << FairLogger::endl;
		nHits += nModuleHits;
	}

  // --- Counters
  fTimer.Stop();
  fNofTimeSlices++;
  fNofClustersTot += nClusters;
  fNofHitsTot     += nHits;
  fTimeTot        += fTimer.RealTime();

  LOG(INFO) << "+ " << setw(20) << GetName() << ": Time slice " << setw(6)
  		      << right << iEvent << ", real time " << fixed << setprecision(6)
  		      << fTimer.RealTime() << " s, clusters: " << nClusters
  		      << ", hits: " << nHits << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsFindHitsSingleCluster::Finish() {
	std::cout << std::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	LOG(INFO) << "Time slices processed  : " << fNofTimeSlices
	    << FairLogger::endl;
	LOG(INFO) << "Clusters / time slice  : "
			      << fNofClustersTot / Double_t(fNofTimeSlices)
			      << FairLogger::endl;
	LOG(INFO) << "Hits / time slice      : "
			      << fNofHitsTot / Double_t(fNofTimeSlices)
			      << FairLogger::endl;
	LOG(INFO) << "Hits per cluster  : " << fNofHitsTot / fNofClustersTot
			      << FairLogger::endl;
	LOG(INFO) << "Time per time slice    : "
	    << fTimeTot / Double_t(fNofTimeSlices)
			      << " " << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
}
// -------------------------------------------------------------------------


// -----   End-of-event action   -------------------------------------------
void CbmStsFindHitsSingleCluster::FinishEvent() {

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
InitStatus CbmStsFindHitsSingleCluster::Init()
{
    // --- Check IO-Manager
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {
    	LOG(ERROR) << GetName() << ": No FairRootManager!"
    			       << FairLogger::endl;
    	return kFATAL;
    }

    // --- Get input array (StsDigis)
    fClusters = (TClonesArray*)ioman->GetObject("StsCluster");
    if (NULL == fClusters) {
    	LOG(ERROR) << GetName() << ": No StsCluster array!" << FairLogger::endl;
    	return kERROR;
    }

    // --- Register output array
    fHits = new TClonesArray("CbmStsHit", 10000);
    ioman->Register("StsHit", "Hits in STS", fHits,IsOutputBranchPersistent("StsHit"));

    // --- Get STS setup
    fSetup = CbmStsSetup::Instance();

    LOG(INFO) << GetName() << ": Initialisation successful"
	<< FairLogger::endl;

    return kSUCCESS;
}
// -------------------------------------------------------------------------




// ----- Sort digis into module digi maps   --------------------------------
Int_t CbmStsFindHitsSingleCluster::SortClusters() {

	// --- Counters
	Int_t nClusters = 0;

	// --- Loop over clusters in input array
	for (Int_t iCluster = 0;
			 iCluster < fClusters->GetEntriesFast(); iCluster++) {
		CbmStsCluster* cluster = static_cast<CbmStsCluster*> (fClusters->At(iCluster));
		UInt_t address = cluster->GetAddress();
		cluster->SetIndex(iCluster);
		CbmStsModule* module =
				static_cast<CbmStsModule*>(fSetup->GetElement(address, kStsModule));

	  // --- Update set of active modules
		fActiveModules.insert(module);

		// --- Assign cluster to module
		module->AddCluster(cluster);
		nClusters++;

	}  // Loop over cluster array

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

ClassImp(CbmStsFindHitsSingleCluster)
