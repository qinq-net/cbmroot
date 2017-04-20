/** @file CbmStsFindClustersStream.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 16.06.2014
 **/

// --- Include class header
#include "reco/CbmStsFindClustersStream.h"

// --- Includes from C++
#include <cassert>

// --- Includes from ROOT and FairRoot
#include "TClonesArray.h"
#include "FairEventHeader.h"
#include "FairRun.h"

// --- Include from CBMROOT
#include "CbmEvent.h"
#include "CbmStsAddress.h"
#include "CbmStsDigi.h"

// --- Includes from STS
#include "reco/CbmStsClusterAnalysis.h"
#include "reco/CbmStsClusterFinderModule.h"
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSetup.h"

using std::fixed;
using std::right;
using std::setprecision;
using std::setw;

// -----   Constructor   ---------------------------------------------------
CbmStsFindClustersStream::CbmStsFindClustersStream()
    : FairTask("StsFindClustersStream", 1)
    , fEvents(NULL)
    , fDigis(NULL)
    , fClusters(NULL)
    , fSetup(NULL)
    , fAna(NULL)
    , fTimer()
    , fEventMode(kFALSE)
    , fLegacy(kFALSE)
    , fNofEntries(0)
    , fNofUnits(0)
    , fNofDigis(0.)
    , fNofClusters(0.)
    , fTimeTot(0.)
    , fDynRange(40960.)
    , fThreshold(4000.)
    , fNofAdcChannels(4096)
    , fTimeResolution(5.)
    , fDeadTime(9999999.)
    , fNoise(0.)
    , fModules()
    , fDigiMap()
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsFindClustersStream::~CbmStsFindClustersStream() {

  // Delete cluster analysis
  if ( fAna ) delete fAna;

  // Delete cluster finder modules
  auto it = fModules.begin();
  while ( it != fModules.end() ) delete it->second;

}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsFindClustersStream::Exec(Option_t* opt) {

  if ( fEventMode && ! fLegacy) LOG(INFO) << GetName() << ": Processing time slice "
      << fNofEntries << FairLogger::endl;

  // --- Reset output array
  fClusters->Delete();

  // --- In legacy mode: process legacy event
  if ( fLegacy ) {
    ProcessLegacyEvent();
    return;
  }

  // --- Number of time slices or events
  Int_t nUnits = ( (fEventMode && !fLegacy) ? fEvents->GetEntriesFast() : 1);

  // --- Loop over input units
  for (Int_t iUnit = 0; iUnit < nUnits; iUnit++) {
    CbmEvent* event = ( (fEventMode && !fLegacy) ?
        dynamic_cast<CbmEvent*>(fEvents->At(iUnit)) : NULL);
    ProcessData(event);
  }

  if ( fEventMode ) LOG(INFO) << GetName() << ": " << nUnits
      << (nUnits == 1 ? " event" : " events") << " processed in time slice "
      << fNofEntries << FairLogger::endl;

  fNofEntries++;
}
// -------------------------------------------------------------------------



// -----   End-of-run action   ---------------------------------------------
void CbmStsFindClustersStream::Finish() {
  std::cout << std::endl;
  LOG(INFO) << "=====================================" << FairLogger::endl;
  LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;

  if ( ! fLegacy ) LOG(INFO) << "Time slices           : "
        << fNofEntries << FairLogger::endl;

  // --- Time slice mode
  if ( ! fEventMode) {
    LOG(INFO) << "Digis / time slice    : "
        << fNofDigis / Double_t(fNofEntries) << FairLogger::endl;
    LOG(INFO) << "Clusters / time slice : "
        << fNofClusters / Double_t(fNofEntries) << FairLogger::endl;
    LOG(INFO) << "Digis per cluster     : "
        << fNofDigis / fNofClusters << FairLogger::endl;
    LOG(INFO) << "Time per time slice   : "
        << fTimeTot / Double_t(fNofEntries) << " s " << FairLogger::endl;
  } //? time-based mode

  // --- Event-by-event mode
  else {
    LOG(INFO) << "Events                : "
        << fNofUnits << FairLogger::endl;
    LOG(INFO) << "Digis / event         : "
        << fNofDigis / Double_t(fNofUnits) << FairLogger::endl;
    LOG(INFO) << "Clusters / event      : "
        << fNofClusters / Double_t(fNofUnits) << FairLogger::endl;
    LOG(INFO) << "Digis per cluster     : "
        << fNofDigis / fNofClusters << FairLogger::endl;
    LOG(INFO) << "Time per event        : "
        << fTimeTot / Double_t(fNofUnits) << " s " << FairLogger::endl;
  } //? event-by-event mode

  LOG(INFO) << "=====================================" << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
InitStatus CbmStsFindClustersStream::Init()
{

  // --- Get STS setup
  fSetup = CbmStsSetup::Instance();

  // --- Instantiate cluster analysis
  fAna = new CbmStsClusterAnalysis();

  // --- Something for the screen
  LOG(INFO) << "\n=========================================================="
                << FairLogger::endl;
  LOG(INFO) << GetName() << ": Initialising " << FairLogger::endl;

  // --- Check IO-Manager
  FairRootManager* ioman = FairRootManager::Instance();
  assert(ioman);

  // --- In event mode: get input array (CbmEvent)
  if ( fEventMode ) {
    LOG(INFO) << GetName() << ": Using event-by-event mode"
          << FairLogger::endl;
      fEvents = dynamic_cast<TClonesArray*>(ioman->GetObject("Event"));
      if ( ! fEvents ) {
        LOG(WARNING) << GetName()
            << ": Event mode selected but no event array found! \n"
            << "           The task will run over the entire input array "
            << "(legacy mode)." << FairLogger::endl;
        fLegacy = kTRUE;
      }
    }
    else LOG(INFO) << GetName() << ": Using time-based mode"
        << FairLogger::endl;

    // --- Get input array (StsDigis)
    fDigis = (TClonesArray*)ioman->GetObject("StsDigi");
    assert(fDigis);

    // --- Register output array
    fClusters = new TClonesArray("CbmStsCluster", 1e6);
    ioman->Register("StsCluster", "Cluster in STS", fClusters, IsOutputBranchPersistent("StsCluster"));


    // --- Set module parameters if not done already
    // TODO: The module parameters are set by the STS digitiser. However,
    // this information is not persistent. If the reco run starts from the raw
    // data file, i.e. the digitiser is not part of the run, the module
    // parameters are not set. As a workaround, they are set here explicitly.
    // Obviously, this is not a good solution since consistency with the
    // parameters used by the digitiser is not assured. A proper treatment
    // of the STS parameters is needed (parameter file).
    if ( ! fSetup->GetModule(0)->IsSet() ) SetModuleParameters();

    // --- Instantiate the cluster finding modules
    Int_t nModules = fSetup->GetNofModules();
    for (Int_t iModule = 0; iModule < nModules; iModule++) {
      CbmStsModule* module = fSetup->GetModule(iModule);
      UInt_t address = module->GetAddress();
      const char* name = module->GetName();
      Double_t deltaT = 3.* TMath::Sqrt(2.) * module->GetTimeResolution();
      Int_t nChannels = module->GetNofChannels();
      CbmStsClusterFinderModule* finderModule =
          new CbmStsClusterFinderModule(nChannels, deltaT, name, module, fClusters);
      fModules[address] = finderModule;
    }
    LOG(INFO) << GetName() << ": " << fModules.size()
      << " modules created." << FairLogger::endl;


    LOG(INFO) << GetName() << ": Initialisation successful."
    		      << FairLogger::endl;
    LOG(INFO) << "==========================================================\n"
                  << FairLogger::endl;
    return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Process one time slice or event   -------------------------------
void CbmStsFindClustersStream::ProcessData(CbmEvent* event) {

  // --- Event or time slice number
  Int_t unitId = ( event ? event->GetNumber() : fNofUnits);

  // --- Reset all cluster finder modules
  fTimer.Start();
  for (auto it = fModules.begin(); it != fModules.end(); it++)
      it->second->Reset();
  fTimer.Stop();
  Double_t time1 = fTimer.RealTime();

  // --- Start index of newly created clusters
  Int_t indexFirst = fClusters->GetEntriesFast();

  // --- Number of input digis
  fTimer.Start();
  Int_t nDigis = (event ? event->GetNofData(Cbm::kStsDigi)
      : fDigis->GetEntriesFast() );

  // --- Loop over input digis
  Int_t digiIndex = -1;
  for (Int_t iDigi = 0; iDigi < nDigis; iDigi++) {
    digiIndex = (event ? event->GetIndex(Cbm::kStsDigi, iDigi) : iDigi);
    CbmStsDigi* digi = dynamic_cast<CbmStsDigi*>(fDigis->At(digiIndex));
    ProcessDigi(digiIndex);
  }  //# digis in time slice or event
  fTimer.Stop();
  Double_t time2 = fTimer.RealTime();

  // --- Process remaining clusters in the buffers
  fTimer.Start();
  for (auto it = fModules.begin(); it != fModules.end(); it++)
       it->second->ProcessBuffer();
  fTimer.Stop();
  Double_t time3 = fTimer.RealTime();

  // --- Stop index of newly created clusters
  Int_t indexLast = fClusters->GetEntriesFast();

  // --- Determine cluster parameters
  fTimer.Start();
  for (Int_t index = indexFirst; index < indexLast; index++) {
    CbmStsCluster* cluster = dynamic_cast<CbmStsCluster*>(fClusters->At(index));
    CbmStsModule* module = dynamic_cast<CbmStsModule*>
      (fSetup->GetElement(cluster->GetAddress(), kStsModule));
    fAna->Analyze(cluster, module, fDigis);
  }
  fTimer.Stop();
  Double_t time4 = fTimer.RealTime();

  // --- In event-by-event mode: register clusters to event
  fTimer.Start();
  if ( event ) for (Int_t index = indexFirst; index < indexLast; index++)
    event->AddData(Cbm::kStsCluster, index);
  fTimer.Stop();
  Double_t time5 = fTimer.RealTime();

  // --- Counters
  Int_t nClusters = indexLast - indexFirst;
  Double_t realTime = time1 + time2 + time3 + time4 + time5;
  fNofUnits++;
  fNofDigis    += nDigis;
  fNofClusters += nClusters;
  fTimeTot     += realTime;

  // --- Screen output
  TString unit = (fEventMode ? " Event " : " Time slice ");
  LOG(DEBUG) << GetName() << ": created " << nClusters << " from index "
      << indexFirst << " to " << indexLast << FairLogger::endl;
  LOG(DEBUG) << GetName() << ": reset " << time1 << ", process digis " << time2
      << ", process buffers " << time3 << ", analyse " << time4 << ", register "
      << time5 << FairLogger::endl;
  LOG(INFO) << "+ " << setw(20) << GetName() << ": " << unit << setw(6)
              << right << unitId << ", real time " << fixed << setprecision(6)
              << realTime << " s, digis: " << nDigis
              << ", clusters: " << nClusters << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Process one digi object   ---------------------------------------
void CbmStsFindClustersStream::ProcessDigi(Int_t index) {

  // --- Get the digi object
  CbmStsDigi* digi = dynamic_cast<CbmStsDigi*>(fDigis->At(index));
  assert(digi);

  // --- Get the cluster finder module
  UInt_t address =
      CbmStsAddress::GetMotherAddress(digi->GetAddress(), kStsModule);
  CbmStsClusterFinderModule* module = fModules.at(address);
  assert(module);

  // --- Digi channel and time
  Int_t digiChannel = CbmStsAddress::GetElementId(digi->GetAddress(),
                                                  kStsChannel);
  assert ( digiChannel >= 0 || digiChannel < module->GetSize() );
  UShort_t channel = UShort_t(digiChannel);

  // --- Process digi in module
  module->ProcessDigi(channel, digi->GetTime(), index);

}
// -------------------------------------------------------------------------



// -----   Process a legacy event   ----------------------------------------
void CbmStsFindClustersStream::ProcessLegacyEvent() {

  // --- Event number. Note that the FairRun counting start with 1.
  Int_t eventNumber =
      FairRun::Instance()->GetEventHeader()->GetMCEntryNumber() - 1;

  // --- Reset output array
  fClusters->Delete();

  // --- Reset all cluster finder modules
  fTimer.Start();
  for (auto it = fModules.begin(); it != fModules.end(); it++)
      it->second->Reset();
  fTimer.Stop();
  Double_t time1 = fTimer.RealTime();  // time for module reset

  // --- Read legacy event
  fTimer.Start();
  Int_t nDigis = ReadLegacyEvent();
  assert ( fDigiMap.size() == nDigis );
  fTimer.Stop();
  Double_t time2 = fTimer.RealTime();  // time for reading event

  // --- Start index of newly created clusters
  Int_t indexFirst = fClusters->GetEntriesFast();

  // --- Loop over input digis
  fTimer.Start();
  Int_t digiIndex = -1;
  for ( auto it = fDigiMap.begin(); it != fDigiMap.end(); it++) {
    digiIndex = it->second;
    CbmStsDigi* digi = dynamic_cast<CbmStsDigi*>(fDigis->At(digiIndex));
    ProcessDigi(digiIndex);
  }  //# digis in time slice or event
  fTimer.Stop();
  Double_t time3 = fTimer.RealTime();  // time for processing digis

  // --- Process remaining clusters in the buffers
  fTimer.Start();
  for (auto it = fModules.begin(); it != fModules.end(); it++)
       it->second->ProcessBuffer();
  fTimer.Stop();
  Double_t time4 = fTimer.RealTime(); // time for processing buffers

  // --- Stop index of newly created clusters
  Int_t indexLast = fClusters->GetEntriesFast();

  // --- Determine cluster parameters
  fTimer.Start();
  for (Int_t index = indexFirst; index < indexLast; index++) {
    CbmStsCluster* cluster = dynamic_cast<CbmStsCluster*>(fClusters->At(index));
    CbmStsModule* module = dynamic_cast<CbmStsModule*>
      (fSetup->GetElement(cluster->GetAddress(), kStsModule));
    fAna->Analyze(cluster, module, fDigis);
  }
  fTimer.Stop();
  Double_t time5 = fTimer.RealTime();  // time for cluster analysis

  // --- Counters
  Int_t nClusters = indexLast - indexFirst;
  Double_t realTime = time1 + time2 + time3 + time4 + time5;
  fNofUnits++;
  fNofDigis    += nDigis;
  fNofClusters += nClusters;
  fTimeTot     += realTime;

  // --- Screen output
  TString unit = (fEventMode ? " Event " : " Time slice ");
  LOG(DEBUG) << GetName() << ": created " << nClusters << " from index "
      << indexFirst << " to " << indexLast << FairLogger::endl;
  LOG(DEBUG) << GetName() << ": reset " << time1 << ",read event " << time2
      << ", process digis " << time3 << ", process buffers " << time4
      << ", analyse " << time5  << FairLogger::endl;
  LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
              << right << eventNumber << ", real time " << fixed << setprecision(6)
              << realTime << " s, digis: " << nDigis
              << ", clusters: " << nClusters << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Read legacy event   ---------------------------------------------
Int_t CbmStsFindClustersStream::ReadLegacyEvent() {

  fDigiMap.clear();
  Int_t nDigis = fDigis->GetEntriesFast();
  for (Int_t index = 0; index < nDigis; index++) {
    CbmStsDigi* digi = dynamic_cast<CbmStsDigi*>(fDigis->At(index));
    Double_t time = digi->GetTime();
    fDigiMap.insert(std::pair<Double_t, Int_t>(time, index));
  } //# digis in input array

  return nDigis;
}
// -------------------------------------------------------------------------



// -----   Set parameters of the modules    --------------------------------
void CbmStsFindClustersStream::SetModuleParameters()
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


ClassImp(CbmStsFindClustersStream)