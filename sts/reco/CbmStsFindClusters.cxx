/** @file CbmStsFindClusters.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 16.06.2014
 **/

// --- Include class header
#include "reco/CbmStsFindClusters.h"

// --- Includes from C++
#include <iostream>

// --- Includes from ROOT
#include "TClonesArray.h"

// --- Includes from FAIRROOT
#include "FairEventHeader.h"
#include "FairRunAna.h"

// --- Includes from STS
#include "reco/CbmStsClusterFinderSimple.h"
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSetup.h"
#include "digitize/CbmStsPhysics.h"

using namespace std;


// -----   Constructor   ---------------------------------------------------
CbmStsFindClusters::CbmStsFindClusters(Int_t finderModel, Int_t algorithm)
    : FairTask("StsFindClusters", 1)
    , fDigis(NULL)
    , fClusters(NULL)
    , fSetup(NULL)
    , fFinder(NULL)      
    , fTimer()
    , fFinderModel(finderModel)
    , fAlgorithm(algorithm)
    , fELossModel(0)
    , fTimeSlice(NULL)
    , fDigiData()
    , fDaq(kFALSE)
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
CbmStsFindClusters::~CbmStsFindClusters() {
	if ( fFinder ) delete fFinder;
}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindClusters::Exec(Option_t* /*opt*/) {

	// --- Event number
	Int_t iEvent =
			FairRun::Instance()->GetEventHeader()->GetMCEntryNumber();

	// Start timer and counter
	fTimer.Start();
	Int_t nOfClusters = 0;

	// --- Clear output array
	fClusters->Delete();

	// --- Sort digis into modules
	Int_t nOfDigis = SortDigis();

	// --- Find clusters in modules
	/*
	set<CbmStsModule*>::iterator it;
	for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
		Int_t nClusters = 0;
		if(fUseFinderTb) {
			(*it)->SetDeadTime(fDeadTime);
			nClusters = fFinder->FindClustersTb(*it);
		}
		else
			nClusters = fFinder->FindClustersSimple(*it);
		LOG(DEBUG1) << GetName() << ": Module " << (*it)->GetName()
    			      << ", digis: " << (*it)->GetNofDigis()
   		          << ", clusters " << nClusters << FairLogger::endl;
		nOfClusters += nClusters;
	}
	*/
	for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
		CbmStsModule* module = fSetup->GetModule(iModule);
		if ( module->GetNofDigisTb() == 0 ) continue;
		Int_t nClusters = 0;
		if ( fUseFinderTb ) {
			module->SetDeadTime(fDeadTime);
			nClusters = fFinder->FindClustersTb(module);
		}
		else nClusters = fFinder->FindClustersSimple(module);
		LOG(DEBUG1) << GetName() << ": Module " << module->GetName()
    			      << ", digis: " << module->GetNofDigis()
   		          << ", clusters " << nClusters << FairLogger::endl;
		nOfClusters += nClusters;
	}

  // --- Counters
  fTimer.Stop();
  fNofEvents++;
  fNofDigisTot    += nOfDigis;
  fNofClustersTot += nOfClusters;
  fTimeTot        += fTimer.RealTime();

  LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
  		      << right << iEvent << ", real time " << fixed << setprecision(6)
  		      << fTimer.RealTime() << " s, digis: " << nOfDigis
  		      << ", clusters: " << nOfClusters << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsFindClusters::Finish() {
	std::cout << std::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	LOG(INFO) << "Events processed   : " << fNofEvents << FairLogger::endl;
	LOG(INFO) << "Digis / event      : " << fNofDigisTot / Double_t(fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "Clusters / event   : "
			      << fNofClustersTot / Double_t(fNofEvents)
			      << FairLogger::endl;
	if (fFinderModel == 2) LOG(INFO) << "ClustersWithGap / e: " 
			      << Double_t(fFinder -> GetNofClustersWithGap()) / Double_t (fNofEvents)
			      << FairLogger::endl;
	if (fFinderModel == 0) LOG(INFO) << "SplittedClusters/ e: " 
			      << Double_t(fFinder -> GetNofSplittedClusters()) / Double_t (fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "Digis per cluster  : " << fNofDigisTot / fNofClustersTot
			      << FairLogger::endl;
	LOG(INFO) << "Time per event     : " << fTimeTot / Double_t(fNofEvents)
			      << " s " << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   End-of-event action   -------------------------------------------
void CbmStsFindClusters::FinishEvent() {

	// --- Clear digi maps for all active modules
	Int_t nModules = 0;
	set<CbmStsModule*>::iterator it;
	for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
		if (fDaq)	(*it)->ClearDigisTb();
		else 		(*it)->ClearDigis();
		nModules++;
	}
	fActiveModules.clear();

	LOG(DEBUG) << GetName() << ": Cleared digis in " << nModules
			       << " modules. " << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsFindClusters::Init()
{
    // --- Check IO-Manager
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {
    	LOG(ERROR) << GetName() << ": No FairRootManager!"
    			       << FairLogger::endl;
    	return kFATAL;
    }

    // --- Get input array (StsDigis)
    fDigis = (TClonesArray*)ioman->GetObject("StsDigi");
    if (fDaq) {
		fTimeSlice = (CbmTimeSlice*) ioman->GetObject("TimeSlice.");
		if (NULL == fTimeSlice)
			LOG(FATAL) << GetName() << ": NoTimeSlice data!" << FairLogger::endl;
	}
	else {
		fDigis = (TClonesArray*) ioman->GetObject("StsDigi");
		if (NULL == fDigis) {
			LOG(ERROR) << GetName() << ": No StsDigi array!" << FairLogger::endl;
			return kERROR;
		}
	}

    // --- Register output array
    fClusters = new TClonesArray("CbmStsCluster", 10000);
    ioman->Register("StsCluster", "Cluster in STS", fClusters, IsOutputBranchPersistent("StsCluster"));

    // --- Get STS setup
    fSetup = CbmStsSetup::Instance();

    // --- Set module parameters if not done already
    // TODO: The module parameters are set by the STS digitiser. However,
    // this information is not persistent. If the reco run starts from the raw
    // data file, i.e. the digitiser is not part of the run, the module
    // parameters are not set. As a workaround, they are set here explicitly.
    // Obviously, this is not a good solution since consistency with the
    // parameters used by the digitiser is not assured. A proper treatment
    // of the STS parameters is needed (parameter file).
    CbmStsModule* module = fSetup->GetModule(0);
    if ( ! fSetup->GetModule(0)->IsSet() ) SetModuleParameters();

    if (fDaq) SetModuleParameters();

    // --- Instantiate StsPhysics
    CbmStsPhysics::Instance();

    // --- Create cluster finder
    fFinder = new CbmStsClusterFinderSimple(fFinderModel, fAlgorithm, fELossModel);
    fFinder->SetOutputArray(fClusters);

    LOG(INFO) << GetName() << ": Initialisation successful"
    		      << FairLogger::endl;
    return kSUCCESS;
}
// -------------------------------------------------------------------------




// ----- Sort digis into module digi maps   --------------------------------
Int_t CbmStsFindClusters::SortDigis() {

	// --- Counters
	Int_t nDigis   = 0;

	// --- Loop over digis in input array
	CbmStsDigi* digi = NULL;
	Int_t nofDigis = fDigis->GetEntriesFast();
	for (Int_t iDigi = 0; iDigi < nofDigis; iDigi++) {
		digi = static_cast<CbmStsDigi*> (fDigis->At(iDigi));
		if ( ! digi ) {
			LOG(FATAL) << GetName() << ": Invalid digi pointer!"
					       << FairLogger::endl;
			continue;
		}

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
		if ( fDaq || fUseFinderTb )
			module->AddDigiTb(digi, iDigi);
		else
			module->AddDigi(digi, iDigi);
		nDigis++;

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
void CbmStsFindClusters::SetModuleParameters()
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





// -----   MQ specific   ------------------------------------------------

// -----   MQ init   ------------------------------------------------
bool CbmStsFindClusters::InitMQ(const std::string& geo_file)
{
    // --- Register output array
    fClusters = new TClonesArray("CbmStsCluster", 10000);

    // --- Get STS setup
    fSetup = CbmStsSetup::Instance();
    fSetup->Init(geo_file.c_str());

    // --- Instantiate StsPhysics
    CbmStsPhysics::Instance();

    // --- Create cluster finder
    fFinder = new CbmStsClusterFinderSimple(fFinderModel, fAlgorithm, fELossModel);
    fFinder->SetOutputArray(fClusters);
    return true;
}

// -----   MQ set input   ------------------------------------------------
InitStatus CbmStsFindClusters::SetTimeSlice(CbmTimeSlice* ts)
{
    if(ts)
        fTimeSlice = ts;
    else
        return kERROR;

    return kSUCCESS;
}

// -----   MQ exec   ------------------------------------------------
void CbmStsFindClusters::ExecMQ() {

    // Start timer and counter
    fTimer.Start();
    Int_t nOfClusters = 0;

    // --- Clear output array
    fClusters->Delete();

    // --- Sort digis into modules
    Int_t nOfDigis = SortDigis();

    // --- Find clusters in modules
    set<CbmStsModule*>::iterator it;
    for (it = fActiveModules.begin(); it != fActiveModules.end(); it++) {
        Int_t nClusters = 0;
        if(fUseFinderTb)
            nClusters = fFinder->FindClustersTb(*it);
        else
            nClusters = fFinder->FindClustersSimple(*it);
        LOG(DEBUG1) << GetName() << ": Module " << (*it)->GetName()
                      << ", digis: " << (*it)->GetNofDigis()
                  << ", clusters " << nClusters << FairLogger::endl;
        nOfClusters += nClusters;
    }

  // --- Counters
  fTimer.Stop();
  fNofEvents++;
  fNofDigisTot    += nOfDigis;
  fNofClustersTot += nOfClusters;
  fTimeTot        += fTimer.RealTime();

  LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
              << right << ", time " << fixed << setprecision(6)
              << fTimer.RealTime() << " s, digis: " << nOfDigis
              << ", clusters: " << nOfClusters << FairLogger::endl;
}
ClassImp(CbmStsFindClusters)
