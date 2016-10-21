/** @file CbmStsFindClustersEvents.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 16.06.2014
 ** @date 19.10.2016
 **/

// --- Include class header
#include "reco/CbmStsFindClustersEvents.h"

// --- Includes from C++
#include <iostream>

// --- Includes from ROOT
#include "TClonesArray.h"

// --- Includes from FAIRROOT
//#include "FairEventHeader.h"
//#include "FairRunAna.h"

// --- Includes from STS
#include "CbmEvent.h"
#include "reco/CbmStsClusterAnalysis.h"
#include "reco/CbmStsClusterFinder.h"
#include "reco/CbmStsClusterFinderGap.h"
#include "reco/CbmStsClusterFinderIdeal.h"
#include "reco/CbmStsClusterFinderReal.h"
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSetup.h"
#include "digitize/CbmStsPhysics.h"

using namespace std;

// TODO: General cleanup; flexibility to choose cluster finder
// (factory scheme)


// -----   Constructor   ---------------------------------------------------
CbmStsFindClustersEvents::CbmStsFindClustersEvents(Int_t finderModel)
    : FairTask("StsFindClusters", 1)
    , fEvents(NULL)
    , fDigis(NULL)
    , fClusters(NULL)
    , fSetup(NULL)
    , fFinder(NULL)
    , fAna (NULL)
    , fTimer()
    , fFinderModel(finderModel)
    , fELossModel(0)
    , fUseFinderTb(kFALSE)
    , fDeadTime(9999999.)
    , fNofEvents(0.)
    , fNofDigisTot(0.)
    , fNofClustersTot(0.)
    , fTimeTot(0.)
    , fDynRange(40960.)
    , fThreshold(4000.)
    , fNofAdcChannels(4096)
    , fTimeResolution(5.)
    , fNoise(0.)
    , fActiveModules()
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmStsFindClustersEvents::~CbmStsFindClustersEvents() {
	if ( fFinder ) delete fFinder;
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindClustersEvents::Exec(Option_t* opt) {

	// --- Clear output array
	fClusters->Delete();

	// --- Event loop
	Int_t nEvents = fEvents->GetEntriesFast();
	LOG(INFO) << GetName() << ": reading time slice with " << nEvents
			      << " events " << FairLogger::endl;
	for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
		CbmEvent* event = static_cast<CbmEvent*> (fEvents->At(iEvent));
		ProcessEvent(event);
	}

}
// -------------------------------------------------------------------------


// -----  Process one event   ----------------------------------------------
void CbmStsFindClustersEvents::ProcessEvent(CbmEvent* event) {

	// Start timer and counter
	fTimer.Start();
	Int_t nClustersEvent = 0;

	// --- Sort digis into modules
	Int_t nDigis = SortDigis(event);

	// --- Find clusters in modules
	set<CbmStsModule*>::iterator it;
	for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
		Int_t nClusters = 0;
	  nClusters = fFinder->FindClusters(*it, event);
		LOG(DEBUG1) << GetName() << ": Module " << (*it)->GetName()
    			      << ", digis: " << (*it)->GetNofDigis()
   		          << ", clusters " << nClusters << FairLogger::endl;
		nClustersEvent += nClusters;
	}

	// --- Analyse the clusters in the event
	Int_t nClusters = event->GetNofData(Cbm::kStsCluster);
	for (Int_t iCluster = 0; iCluster < nClusters; iCluster++) {
		Int_t index = event->GetIndex(Cbm::kStsCluster, iCluster);
		CbmStsCluster* cluster = (CbmStsCluster*) fClusters->At(index);
		CbmStsModule* module =
				(CbmStsModule*) fSetup->GetElement(cluster->GetAddress(), kStsModule);
		fAna->Analyze(cluster, module, fDigis);
	}

  // --- Counters
  fTimer.Stop();
  fNofEvents++;
  fNofDigisTot    += nDigis;
  fNofClustersTot += nClustersEvent;
  fTimeTot        += fTimer.RealTime();

  LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
  		      << right << event->GetNumber()
  		      << ", real time " << fixed << setprecision(6)
  		      << fTimer.RealTime() << " s, digis: " << nDigis
  		      << ", clusters: " << nClustersEvent << FairLogger::endl;

  // --- End-of-event action (clear digi maps of modules)
  FinishEvent();

}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsFindClustersEvents::Finish() {
	std::cout << std::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	LOG(INFO) << "Events processed   : " << fNofEvents << FairLogger::endl;
	LOG(INFO) << "Digis / event      : " << fNofDigisTot / Double_t(fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "Clusters / event   : "
			      << fNofClustersTot / Double_t(fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "Digis per cluster  : " << fNofDigisTot / fNofClustersTot
			      << FairLogger::endl;
	LOG(INFO) << "Time per event     : " << fTimeTot / Double_t(fNofEvents)
			      << " s " << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   End-of-event action   -------------------------------------------
void CbmStsFindClustersEvents::FinishEvent() {

	// --- Clear digi maps for all active modules
	Int_t nModules = 0;
	set<CbmStsModule*>::iterator it;
	for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
		(*it)->ClearDigis();
		nModules++;
	}
	fActiveModules.clear();

	LOG(DEBUG) << GetName() << ": Cleared digis in " << nModules
			       << " modules. " << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsFindClustersEvents::Init()
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

    // --- Get input array (StsDigis)
    fDigis = (TClonesArray*) ioman->GetObject("StsDigi");
		if (NULL == fDigis) {
			LOG(ERROR) << GetName() << ": No StsDigi array!" << FairLogger::endl;
			return kERROR;
		}

    // --- Register output array
    fClusters = new TClonesArray("CbmStsCluster", 10000);
    ioman->Register("StsCluster", "Cluster in STS", fClusters,
    		            IsOutputBranchPersistent("StsCluster"));

    // --- Get STS setup
    fSetup = CbmStsSetup::Instance();

    // --- Instantiate StsPhysics
    CbmStsPhysics::Instance();

    // --- Create cluster finder and analysis
    switch (fFinderModel) {
    	case 0: fFinder = new CbmStsClusterFinderIdeal(fClusters); break;
    	case 1: fFinder = new CbmStsClusterFinderReal(fClusters); break;
    	case 2: fFinder = new CbmStsClusterFinderGap(fClusters); break;
    	default: LOG(FATAL) << GetName() << ": Unknown cluster finder model"
    										  << fFinderModel << FairLogger::endl; break;
    }
    LOG(INFO) << GetName() << ": Use cluster finder "
              << fFinder->GetTitle() << FairLogger::endl;
    fAna    = new CbmStsClusterAnalysis();

    LOG(INFO) << GetName() << ": Initialisation successful"
    		      << FairLogger::endl;
    return kSUCCESS;
}
// -------------------------------------------------------------------------




// ----- Sort digis into module digi maps   --------------------------------
Int_t CbmStsFindClustersEvents::SortDigis(CbmEvent* event) {

	// --- Counters
	Int_t nDigis   = event->GetNofData(Cbm::kStsDigi);

	// --- Loop over digis in event
	for (Int_t iDigi = 0; iDigi < nDigis; iDigi++) {
		UInt_t index = event->GetIndex(Cbm::kStsDigi, iDigi);
		CbmStsDigi* digi = (CbmStsDigi*) fDigis->At(index);
		assert(digi);

		// --- Get the module
		UInt_t address = digi->GetAddress();
		CbmStsModule* module =
				static_cast<CbmStsModule*>(fSetup->GetElement(address, kStsModule));
		if ( ! module ) {
			LOG(FATAL) << GetName() << ": Module " << address
					       << " not present in STS setup!" << FairLogger::endl;
			continue;
		}

		// --- Add module to list of active modules, if not yet present.
		fActiveModules.insert(module);

		// --- Add the digi to the module
	  module->AddDigi(digi, iDigi);

	}  // Loop over digi array

	// --- Debug output
	LOG(DEBUG) << GetName() << ": sorted " << nDigis << " digis into "
			       << fActiveModules.size() << " module(s)." << FairLogger::endl;
	if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG3) ) {
		set<CbmStsModule*>::iterator it;
		for (it = fActiveModules.begin(); it != fActiveModules.end() ; it++) {
				LOG(DEBUG3) << GetName() << ": Module " << (*it)->GetName()
						        << ", digis " << (*it)->GetNofDigis()
						        << FairLogger::endl;
		}
	}

	return nDigis;
}
// -------------------------------------------------------------------------



// -----   Set parameters of the modules    --------------------------------
void CbmStsFindClustersEvents::SetModuleParameters()
{
	// --- Control output of parameters
	LOG(INFO) << GetName() << ": Digitisation parameters :"
			      << FairLogger::endl;
	LOG(INFO) << "\t Dynamic range   " << setw(10) << right
				    << fDynRange << " e"<< FairLogger::endl;
	LOG(INFO) << "\t Threshold       " << setw(10) << right
			      << fThreshold << " e"<< FairLogger::endl;
	LOG(INFO) << "\t ADC channels    " << setw(10) << right
			      << fNofAdcChannels << FairLogger::endl;
	LOG(INFO) << "\t Time resolution " << setw(10) << right
			      << fTimeResolution << " ns" << FairLogger::endl;
	LOG(INFO) << "\t Dead time       " << setw(10) << right
			      << fDeadTime << " ns" << FairLogger::endl;
	LOG(INFO) << "\t ENC             " << setw(10) << right
			      << fNoise << " e" << FairLogger::endl;

	// --- Set parameters for all modules
	Int_t nModules = fSetup->GetNofModules();
	for (Int_t iModule = 0; iModule < nModules; iModule++) {
		fSetup->GetModule(iModule)->SetParameters(2048, fDynRange, fThreshold,
												  fNofAdcChannels, fTimeResolution,
												  fDeadTime, fNoise);
	}
	LOG(INFO) << GetName() << ": Set parameters for " << nModules
			      << " modules " << FairLogger::endl;
}
// -------------------------------------------------------------------------







ClassImp(CbmStsFindClustersEvents)
