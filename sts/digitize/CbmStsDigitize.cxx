/** @file CbmStsDigitize.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.05.2014
 **/

// Include class header
#include "CbmStsDigitize.h"

// Includes from C++
#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sstream>
#include <iomanip>
#include <iostream>

// Includes from ROOT
#include "TClonesArray.h"
#include "TGeoBBox.h"
#include "TGeoMatrix.h"
#include "TGeoPhysicalNode.h"
#include "TGeoVolume.h"

// Includes from FairRoot
#include "FairEventHeader.h"
#include "FairField.h"
#include "FairLink.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "FairMCPoint.h"
#include "FairRunAna.h"
#include "FairRunSim.h"
#include "FairRuntimeDb.h"

// Includes from CbmRoot
#include "CbmDaqBuffer.h"
#include "CbmMCTrack.h"
#include "CbmStsDigi.h"
#include "CbmStsPoint.h"

// Includes from STS
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensor.h"
#include "setup/CbmStsSensorConditions.h"
#include "setup/CbmStsSetup.h"
#include "digitize/CbmStsPhysics.h"
#include "digitize/CbmStsDigitizeParameters.h"

using std::fixed;
using std::right;
using std::setprecision;
using std::setw;
using std::string;

Bool_t fIsInitialised;   ///< kTRUE if Init() was called

Int_t  fEnergyLossModel;  ///< Energy loss model
Bool_t fUseLorentzShift;
Bool_t fUseDiffusion;
Bool_t fUseCrossTalk;
Bool_t fGenerateNoise;

CbmStsDigitizeParameters* fDigiPar; ///< Digitisation parameters

// -----   Standard constructor   ------------------------------------------
CbmStsDigitize::CbmStsDigitize()
  : FairTask("StsDigitize"),
    fMode(0),
    fIsInitialised(kFALSE),
    fEnergyLossModel(2),
    fUseLorentzShift(kTRUE),
    fUseDiffusion(kTRUE),
    fUseCrossTalk(kTRUE),
    fGenerateNoise(kFALSE),
    fDigiPar(NULL),
    fSetup(NULL),
    fPoints(NULL),
    fTracks(NULL),
    fDigis(NULL),
    fMatches(NULL),
    fTimer(),
    fSensorTypeFile(),
    fTimePointLast(-1.),
    fEventTimeCurrent(0.),
    fTimeDigiFirst(-1.),
    fTimeDigiLast(-1.),
    fNofPoints(0),
    fNofSignalsF(0),
    fNofSignalsB(0),
    fNofDigis(0),
    fNofEvents(0),
    fNofPointsTot(0.),
    fNofSignalsFTot(0.),
    fNofSignalsBTot(0.),
    fNofDigisTot(0.),
    fNofNoiseTot(0.),
    fTimeTot()
{ 
  Reset();
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsDigitize::~CbmStsDigitize() {
 if ( fDigis ) {
    fDigis->Delete();
    delete fDigis;
  }
 if ( fMatches ) {
	fMatches->Delete();
	delete fMatches;
 }

 Reset();
}
// -------------------------------------------------------------------------



// -----   Check the status of the analog buffers   ------------------------
string CbmStsDigitize::BufferStatus() const {

	Int_t    nSignals =  0;
	Double_t t1       = -1;
	Double_t t2       = -1.;

	Int_t    nSigModule;
	Double_t t1Module;
	Double_t t2Module;

  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
  	fSetup->GetModule(iModule)->BufferStatus(nSigModule, t1Module, t2Module);
  	if ( nSigModule ) {
  		nSignals += nSigModule;
  		t1 = t1 < 0. ? t1Module : TMath::Min(t1, t1Module);
  		t2  = TMath::Max(t2, t2Module);
  	} //? signals in module buffer?
  } //# modules in setup

	std::stringstream ss;
	ss << nSignals << ( nSignals == 1 ? " signal " : " signals " )
		 << "in analog buffers";
	if ( nSignals ) ss << " ( from " << fixed << setprecision(3)
			               << t1 << " ns to " << t2 << " ns )";
	return ss.str();
}
// -------------------------------------------------------------------------


// -----   Check the status of the analog buffers   ------------------------
string CbmStsDigitize::BufferStatus2() const {

	Int_t    nSignals =  0;
	Double_t t1       = -1;
	Double_t t2       = -1.;

	Int_t    nSigModule;
	Double_t t1Module;
	Double_t t2Module;

  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
  	fSetup->GetModule(iModule)->BufferStatus(nSigModule, t1Module, t2Module);
  	if ( nSigModule ) {
  		nSignals += nSigModule;
  		t1 = t1 < 0. ? t1Module : TMath::Min(t1, t1Module);
  		t2  = TMath::Max(t2, t2Module);
  		LOG(DEBUG) << iModule << " " << fSetup->GetModule(iModule)->GetName()
  				<< " " << nSigModule << " " << t1 << " " << t2 << FairLogger::endl;
  	} //? signals in module buffer?
  } //# modules in setup

	std::stringstream ss;
	ss << nSignals << ( nSignals == 1 ? " signal " : " signals " )
		 << "in analog buffers";
	if ( nSignals ) ss << " ( from " << fixed << setprecision(3)
			               << t1 << " ns to " << t2 << " ns )";
	return ss.str();
}
// -------------------------------------------------------------------------



// -----   Create a digi object   ------------------------------------------
void CbmStsDigitize::CreateDigi(Int_t address, UShort_t channel,
		              							Long64_t time,
		              							UShort_t adc,
		              							const CbmMatch& match) {

	// Copy match object. Will be deleted in the digi destructor.
	CbmMatch* digiMatch = new CbmMatch(match);

	// Update times of first and last digi
	fTimeDigiFirst = fNofDigis ?
			             TMath::Min(fTimeDigiFirst, Double_t(time)) : time;
	fTimeDigiLast  = TMath::Max(fTimeDigiLast, Double_t(time));

	// In stream mode: create digi and send it to Daq
	if ( fMode == 0 ) {
		CbmStsDigi* digi = new CbmStsDigi(address, channel, time, adc);
		digi->SetMatch(digiMatch);
    CbmDaqBuffer::Instance()->InsertData(digi);
	} //? Stream mode

	// In event mode: create digi and match in TClonesArrays
	else {
		if ( ! fDigis ) {
			LOG(FATAL) << GetName() << ": No output TClonesArray for StsDigis!"
				       	 << FairLogger::endl;
			return;
		}
		Int_t nDigis = fDigis->GetEntriesFast();
		CbmStsDigi* digi =
				new ( (*fDigis)[nDigis] ) CbmStsDigi(address, channel, time, adc);
		digi->SetMatch(digiMatch);

		// --- For backward compatibility:
		// --- create a second match in a separate branch
		new ( (*fMatches)[nDigis] ) CbmMatch(match);
	} //? event mode

	fNofDigis++;
	LOG(DEBUG3) << GetName() << ": created digi at " << time
			    << " ns with ADC " << adc << " at address " << address
			    << " (module "
			    << fSetup->GetElement(address, kStsModule)->GetName()
			    << ", channel " << channel
	            << ")" << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsDigitize::Exec(Option_t* /*opt*/) {


	// --- Start timer and reset counters
	fTimer.Start();
	Reset();

	// --- For debug: status of analogue buffers
    if ( gLogger->IsLogNeeded(DEBUG)) {
     	std::cout << std::endl;
     	LOG(DEBUG) << GetName() << ": " << BufferStatus() << FairLogger::endl;
    }

    // --- Store previous event time.  Get current event time.
    Int_t eventNumber = FairRootManager::Instance()->GetEntryNr();
    Double_t eventTimePrevious = fEventTimeCurrent;
    fEventTimeCurrent = FairRun::Instance()->GetEventHeader()->GetEventTime();

    // --- Generate noise from previous to current event time. Only in
    // --- streaming mode.
    if ( fMode == 0 && fDigiPar->GetGenerateNoise() ) {
      Int_t nNoise = 0;
      for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++)
        nNoise += fSetup->GetModule(iModule)->GenerateNoise(eventTimePrevious,
                                                            fEventTimeCurrent);
      fNofNoiseTot += Double_t(nNoise);
      LOG(INFO) << "+ " << setw(20) << GetName() << ": Generated  " << nNoise
          << " noise signals from t = " << eventTimePrevious << " ns to "
          << fEventTimeCurrent << " ns" << FairLogger::endl;
    }

	// --- Analogue response: Process the input array of StsPoints
	ProcessMCEvent();
	LOG(DEBUG) << GetName() << ": " << fNofSignalsF + fNofSignalsB
			       << " signals generated ( "
			       << fNofSignalsF << " / " << fNofSignalsB << " )"
			       << FairLogger::endl;
	// --- For debug: status of analogue buffers
	if ( gLogger->IsLogNeeded(DEBUG)) {
	  LOG(DEBUG) << GetName() << ": " << BufferStatus() << FairLogger::endl;
	}

	// --- Readout time: in stream mode the time of the current event.
	// --- Analogue buffers will be digitised for signals at times smaller than
	// --- that time minus a safety margin depending on the module properties
	// --- (dead time and time resolution). In event mode, the readout time
	// --- is set to -1., meaning to digitise everything in the readout buffers.
	Double_t readoutTime = fMode == 0 ? fEventTimeCurrent : -1.;
	LOG(DEBUG) << GetName() << ": Readout time is " << readoutTime << " ns"
			       << FairLogger::endl;

	// --- Digital response: Process buffers of all modules
	ProcessAnalogBuffers(readoutTime);


    // --- Check status of analogue module buffers
    if ( gLogger->IsLogNeeded(DEBUG)) {
      LOG(DEBUG) << GetName() << ": " << BufferStatus() << FairLogger::endl;
    }

  // --- Event log
  LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
  		      << right << eventNumber << ", real time " << fixed
  		      << setprecision(6) << fTimer.RealTime() << " s, event time "
  		      << fEventTimeCurrent << " ns, points: "
  		      << fNofPoints << ", signals: " << fNofSignalsF << " / "
  		      << fNofSignalsB << ", digis: " << fNofDigis << FairLogger::endl;

  // --- Counters
  fTimer.Stop();
  fNofEvents++;
  fNofPointsTot   += fNofPoints;
  fNofSignalsFTot += fNofSignalsF;
  fNofSignalsBTot += fNofSignalsB;
  fNofDigisTot    += fNofDigis;
  fTimeTot        += fTimer.RealTime();

}
// -------------------------------------------------------------------------



// -----   Finish run    ---------------------------------------------------
void CbmStsDigitize::Finish() {

	// --- Start timer and reset counters
	fTimer.Start();
	Reset();

	// --- Process the remaining points in the MCBuffer.
	// --- Relevant only in streaming mode
	// --- Reset digi counters
	std::cout << std::endl;
  LOG(DEBUG) << GetName() << ": Finish run" << FairLogger::endl;
  LOG(DEBUG) << GetName() << ": " << BufferStatus() << FairLogger::endl;
  ProcessAnalogBuffers(-1.); // -1 means process all data
  LOG(DEBUG) << GetName() << ": " << BufferStatus() << FairLogger::endl;

  fTimer.Stop();
  fNofPointsTot   += fNofPoints;
  fNofSignalsFTot += fNofSignalsF;
  fNofSignalsBTot += fNofSignalsB;
  fNofDigisTot    += fNofDigis;
  fTimeTot        += fTimer.RealTime();

	std::cout << std::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	LOG(INFO) << "Events processed    : " << fNofEvents << FairLogger::endl;
	LOG(INFO) << "StsPoint / event    : " << setprecision(1)
			      << fNofPointsTot / Double_t(fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "Signals / event     : "
			      << fNofSignalsFTot / Double_t(fNofEvents)
			      << " / " << fNofSignalsBTot / Double_t(fNofEvents)
			      << FairLogger::endl;
	LOG(INFO) << "StsDigi / event     : "
			      << fNofDigisTot  / Double_t(fNofEvents) << FairLogger::endl;
	LOG(INFO) << "Digis per point     : " << setprecision(6)
			      << fNofDigisTot / fNofPointsTot << FairLogger::endl;
	LOG(INFO) << "Digis per signal    : "
			      << fNofDigisTot / ( fNofSignalsFTot + fNofSignalsBTot )
			      << FairLogger::endl;
	LOG(INFO) << "Noise digis / event : " << fNofNoiseTot / Double_t(fNofEvents)
	          << FairLogger::endl;
	LOG(INFO) << "Noise fraction      : " << fNofNoiseTot / fNofDigisTot
	          << FairLogger::endl;
	LOG(INFO) << "Real time per event : " << fTimeTot / Double_t(fNofEvents)
			      << " s" << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Get event information   -----------------------------------------
void CbmStsDigitize::GetEventInfo(Int_t& inputNr, Int_t& eventNr,
		                              Double_t& eventTime) {

  // --- The event number is taken from the FairRootManager
  eventNr = FairRootManager::Instance()->GetEntryNr();

  // --- In a FairRunAna, take input number and time from FairEventHeader
  if ( FairRunAna::Instance() ) {
    FairEventHeader* event = FairRunAna::Instance()->GetEventHeader();
	assert ( event );
	 inputNr   = event->GetInputFileId();
	 eventTime = event->GetEventTime();
  } //? FairRunAna

  // --- In a FairRunSim, the input number and event time are always zero.
  else {
	if ( ! FairRunSim::Instance() )
	  LOG(FATAL) << GetName() << ": neither SIM nor ANA run."
			     << FairLogger::endl;
		inputNr   = 0;
		eventTime = 0.;
  } //? Not FairRunAna

}
// -------------------------------------------------------------------------

void CbmStsDigitize::SetParContainers()
{
   fDigiPar = static_cast<CbmStsDigitizeParameters*>(FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmStsDigitizeParameters"));
}


// -----   Initialisation    -----------------------------------------------
InitStatus CbmStsDigitize::Init() {

  // Get STS setup interface
  fSetup = CbmStsSetup::Instance();

  // Instantiate StsPhysics
  CbmStsPhysics::Instance();

  // Screen output
  std::cout << std::endl;
  LOG(INFO) << "=========================================================="
      << FairLogger::endl;
  LOG(INFO) << GetName() << ": Initialisation" << FairLogger::endl;
  LOG(INFO) << FairLogger::endl;

  // If the task CbmDaq is found, run in stream mode; else in event mode.
  FairTask* daq = FairRun::Instance()->GetTask("Daq");
  if ( daq ) {
    LOG(INFO) << GetName() << ": Using stream mode."
        << FairLogger::endl;
    fMode = 0;
  }  //? stream mode
  else {
    LOG(INFO) << GetName() << ": Using event mode."
        << FairLogger::endl;
    fMode = 1;
    SetGenerateNoise(kFALSE);  // Noise can be generated only in stream mode
  }

  // --- Write physics settings to the parameter container. The default values
  // --- are defined in the constructor; they may have been changed from the
  // --- macro level by SetProcesses().
  fDigiPar->SetProcesses(fEnergyLossModel, fUseLorentzShift, fUseDiffusion,
                         fUseCrossTalk, fGenerateNoise);

  // --- Set default parameters for the modules
  // --- The zero noise rate corresponds to a rise time of 80 ns (1/(pi*tau))
  Double_t dynRange = 75000.;          // dynamic range in e
  Double_t threshold = 3000.;          // threshold in e
  Int_t    nAdc = 32;                  // Number of ADC channels
  Double_t tResol = 5.;                // Time resolution in ns
  Double_t deadTime = 800.;            // Channel dead time in ns
  Double_t noise = 1000.;              // Noise RMS in e
  Double_t zeroNoiseRate = 3.9789e-3;  // Zero noise rate [1/ns]
  Double_t deadChannels = 0.;          // Fraction of dead channels in %
  fDigiPar->SetModuleParameters(dynRange, threshold, nAdc, tResol,
                                deadTime, noise, zeroNoiseRate,
                                deadChannels);

  // Initialise STS setup, sensor conditions and module parameters
  InitSetup();

  // --- Screen output of settings
  LOG(INFO) << GetName() << ": " << fDigiPar->ToString() << FairLogger::endl;

  // --- Get FairRootManager instance
  FairRootManager* ioman = FairRootManager::Instance();
  assert ( ioman );

  // --- Get input array (CbmStsPoint)
  fPoints = (TClonesArray*) ioman->GetObject("StsPoint");
  assert ( fPoints );

  // --- Get input array (CbmMCTrack)
  fTracks = (TClonesArray*) ioman->GetObject("MCTrack");
  assert ( fTracks );

  // --- In event mode: register output arrays
  if ( fMode == 1 ) {

    // Register output array (CbmStsDigi)
    fDigis = new TClonesArray("CbmStsDigi",1000);
    ioman->Register("StsDigi", "Digital response in STS", fDigis, IsOutputBranchPersistent("StsDigi"));

    // Register output array (CbmStsDigiMatch)
    // For backward compatibility only; the match object is already member
    // of CbmStsDigi.
    fMatches = new TClonesArray("CbmMatch", 1000);
    ioman->Register("StsDigiMatch", "MC link to StsDigi", fMatches, IsOutputBranchPersistent("StsDigiMatch"));

  } //? event mode

  // --- Screen output
  LOG(INFO) << GetName() << ": Initialisation successful"
      << FairLogger::endl;
  LOG(INFO) << "=========================================================="
      << FairLogger::endl;
  std::cout << std::endl;

  // Set static initialisation flag
  fIsInitialised = kTRUE;

  return kSUCCESS;
}
// -------------------------------------------------------------------------



// -----   Initialisation of setup    --------------------------------------
void CbmStsDigitize::InitSetup() {

    // Get STS setup interface
	fSetup = CbmStsSetup::Instance();

	// Register this task and the parameter container to the setup
	fSetup->SetDigitizer(this);
	fSetup->SetDigiParameters(fDigiPar);

    // Set sensor conditions
	SetSensorConditions();

    // Set the digitisation parameters of the modules
	SetModuleParameters();

}
// -------------------------------------------------------------------------



// -----   Process the analog buffers of all modules   ---------------------
void CbmStsDigitize::ProcessAnalogBuffers(Double_t readoutTime) {

	// --- Debug
	LOG(DEBUG) << GetName() << ": Processing analog buffers with readout "
			       << "time " << readoutTime << " ns" << FairLogger::endl;

	// --- Loop over all modules in the setup and process their buffers
  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++)
  	fSetup->GetModule(iModule)->ProcessAnalogBuffer(readoutTime);

  // --- Debug output
  LOG(DEBUG) << GetName() << ": " << fNofDigis
  		       << ( fNofDigis == 1 ? " digi " :  " digis " )
  		       << "created and sent to DAQ ";
  if ( fNofDigis ) LOG(DEBUG) << "( from " << fixed
 	             << setprecision(3) << fTimeDigiFirst << " ns to "
  		         << fTimeDigiLast << " ns )";
  LOG(DEBUG) << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Process points from MC event    ---------------------------------
void CbmStsDigitize::ProcessMCEvent() {

	// --- MC Event info (input file, entry number, start time)
	Int_t    inputNr   = 0;
	Int_t    eventNr   = 0;
	Double_t eventTime = 0.;
	GetEventInfo(inputNr, eventNr, eventTime);
	LOG(DEBUG) << GetName() << ": Processing event " << eventNr
			       << " from input " << inputNr << " at t = " << eventTime
			       << " ns with " << fPoints->GetEntriesFast() << " StsPoints "
			       << FairLogger::endl;


  // --- Loop over all StsPoints and execute the ProcessPoint method
  assert ( fPoints );
  for (Int_t iPoint=0; iPoint<fPoints->GetEntriesFast(); iPoint++) {
  	const CbmStsPoint* point = (const CbmStsPoint*) fPoints->At(iPoint);
  	CbmLink* link = new CbmLink(1., iPoint, eventNr, inputNr);

  	// --- Discard secondaries if the respective flag is set
  	if ( fDigiPar->GetDiscardSecondaries() ) {
  		Int_t iTrack = point->GetTrackID();
  		if ( iTrack >= 0 ) {  // MC track is present
  			CbmMCTrack* track = (CbmMCTrack*) fTracks->At(iTrack);
  			assert ( track );
  			if ( track->GetMotherId() >= 0 ) continue;
  		} //? MC track present
  	} //? discard secondaries

  	ProcessPoint(point, eventTime, link);
  	fNofPoints++;
  	delete link;
  }  //# StsPoints

}
// -------------------------------------------------------------------------



// -----  Process a StsPoint   ---------------------------------------------
void CbmStsDigitize::ProcessPoint(const CbmStsPoint* point,
		                              Double_t eventTime, CbmLink* link) {

	// Debug
	if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG2) ) point->Print();
	LOG(DEBUG2) << GetName() << ": Point coordinates: in (" << point->GetXIn()
			        << ", " << point->GetYIn() << ", " << point->GetZIn() << ")"
			        << ", out (" << point->GetXOut() << ", " << point->GetYOut()
			        << ", " << point->GetZOut() << ")" << FairLogger::endl;


	// --- Get the sensor the point is in
	Int_t address = point->GetDetectorID();
	CbmStsSensor* sensor = dynamic_cast<CbmStsSensor*>
					(fSetup->GetElement(address, kStsSensor));
	if ( ! sensor ) {
	  LOG(INFO) << GetName() << ": No sensor for address " << address
	      << FairLogger::endl;
	  LOG(INFO) << "Unit " << CbmStsAddress::GetElementId(address, kStsUnit);
      LOG(INFO) << " Ladder " << CbmStsAddress::GetElementId(address, kStsLadder);
      LOG(INFO) << " Half-ladder " << CbmStsAddress::GetElementId(address, kStsHalfLadder);
      LOG(INFO) << " Module " << CbmStsAddress::GetElementId(address, kStsModule);
      LOG(INFO) << " Sensor " << CbmStsAddress::GetElementId(address, kStsSensor);
      LOG(INFO) << FairLogger::endl;
	}
	if ( ! sensor ) LOG(ERROR) << GetName() << ": Sensor of StsPoint not found!"
			                       << FairLogger::endl;
	assert(sensor);
	LOG(DEBUG2) << GetName() << ": Sending point to sensor "
			        << sensor->GetName() << " ( " << sensor->GetAddress()
			        << " ) " << FairLogger::endl;

	// --- Process the point on the sensor
	Int_t status = sensor->ProcessPoint(point, eventTime, link);

	// --- Statistics
	Int_t nSignalsF = status / 1000;
	Int_t nSignalsB = status - 1000 * nSignalsF;
	LOG(DEBUG2) << GetName() << ": Produced signals: "
			        << nSignalsF + nSignalsB << " ( " << nSignalsF << " / "
			        << nSignalsB << " )" << FairLogger::endl;
	fNofSignalsF += nSignalsF;
	fNofSignalsB += nSignalsB;

}
// -------------------------------------------------------------------------



// -----   Private method ReInit   -----------------------------------------
InitStatus CbmStsDigitize::ReInit() {

  fSetup = CbmStsSetup::Instance();

  return kERROR;

}
// -------------------------------------------------------------------------



// -----   Private method Reset   ------------------------------------------
void CbmStsDigitize::Reset() {
	fTimeDigiFirst = fTimeDigiLast = -1.;
  fNofPoints = fNofSignalsF = fNofSignalsB = fNofDigis = 0;
  if ( fDigis ) fDigis->Delete();
  if ( fMatches ) fMatches->Delete();
}
// -------------------------------------------------------------------------



// -----   Activate noise generation   -------------------------------------
void CbmStsDigitize::SetGenerateNoise(Bool_t choice) {

  if ( fIsInitialised ) {
    LOG(ERROR) << GetName() << ": physics processes must be set before "
               << "initialisation! Statement will have no effect."
               << FairLogger::endl;
    return;
  }

  fGenerateNoise = choice;
}
// -------------------------------------------------------------------------



// -----   Set the digitisation parameters for the modules   ---------------
// TODO: Currently, all modules have the same parameters. In future,
// more flexible schemes must be used, in particular for the thresholds.
void CbmStsDigitize::SetModuleParameters() {

    // --- Set parameters for all modules
	Int_t nModules = fSetup->GetNofModules();
	for (Int_t iModule = 0; iModule < nModules; iModule++) {
		fSetup->GetModule(iModule)->SetParameters(2048,
		                                          fDigiPar->GetDynRange(),
		                                          fDigiPar->GetThreshold(),
				                                  fDigiPar->GetNofAdc(),
				                                  fDigiPar->GetTimeResolution(),
				                                  fDigiPar->GetDeadTime(),
				                                  fDigiPar->GetNoise(),
				                                  fDigiPar->GetZeroNoiseRate());
		fSetup->GetModule(iModule)->SetDeadChannels(fDigiPar->GetDeadChannelFrac());
	}
	LOG(INFO) << GetName() << ": Set parameters for " << nModules
			      << " modules " << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Set the digitisation parameters   -------------------------------
void CbmStsDigitize::SetParameters(Double_t dynRange, Double_t threshold,
                                   Int_t nAdc, Double_t timeResolution,
                                   Double_t deadTime, Double_t noise,
                                   Double_t zeroNoiseRate,
                                   Double_t deadChannelFrac) {

  if ( fIsInitialised ) {
    LOG(ERROR) << GetName() << ": physics processes must be set before "
               << "initialisation! Statement will have no effect."
               << FairLogger::endl;
    return;
  }
  assert(fDigiPar);
  fDigiPar->SetModuleParameters(dynRange, threshold, nAdc, timeResolution,
                                 deadTime, noise, zeroNoiseRate,
                                 deadChannelFrac);
}
// -------------------------------------------------------------------------



// -----   Set the switches for physical processes for the analogue response
void CbmStsDigitize::SetProcesses(Int_t eLossModel,
		                          Bool_t useLorentzShift,
		                          Bool_t useDiffusion,
		                          Bool_t useCrossTalk,
		                          Bool_t generateNoise) {
	  if ( fIsInitialised ) {
	  	LOG(ERROR) << GetName() << ": physics processes must be set before "
	  			   << "initialisation! Statement will have no effect."
	  	           << FairLogger::endl;
	  	return;
	  }
	  fEnergyLossModel = eLossModel;
	  fUseLorentzShift = useLorentzShift;
	  fUseDiffusion    = useDiffusion;
	  fUseCrossTalk    = useCrossTalk;
	  fGenerateNoise   = generateNoise;
}
// -------------------------------------------------------------------------



// -----   Set sensor types from file   ------------------------------------
void CbmStsDigitize::SetSensorTypes(const char* fileName) {

  Bool_t fileFound = kFALSE;
  std::fstream testFile;

  if ( fileName[0] == '/' ) {
    testFile.open(fileName);
    if ( testFile.is_open() )  {
      fileFound = kTRUE;
      testFile.close();
      fSensorTypeFile = fileName;
    }
  }  //? Absolute path?

  else {

    // --- Check whether local file exists
    fSensorTypeFile = gSystem->Getenv("PWD");
    fSensorTypeFile += "/";
    fSensorTypeFile += fileName;
    testFile.open(fSensorTypeFile.Data());
    if ( testFile.is_open() ) {
      fileFound = kTRUE;
      testFile.close();
    }  //? Found in local directory

    else {  // --- Otherwise, try in the parameter directory
      fSensorTypeFile = gSystem->Getenv("VMCWRKDIR");
      fSensorTypeFile += "/parameters/sts/";
      fSensorTypeFile += fileName;
      testFile.open(fSensorTypeFile.Data());
      if ( testFile.is_open() ) {
        fileFound = kTRUE;
        testFile.close();
      }  //? Found in source directory
    }  //? Not found in local directory

  }  //? Relative path

  if ( fileFound ) LOG(INFO) << GetName() << ":Using " << fSensorTypeFile
        << FairLogger::endl;
  else LOG(FATAL) << GetName() << ": Could not find file " << fileName
      << FairLogger::endl;

  testFile.open(fSensorTypeFile);
  string readstr;
  TString sName, sType;
  while ( kTRUE ) {
    if ( testFile.eof() ) break;
    getline(testFile, readstr);
    if ( readstr[0] == '#') continue;
    std::stringstream line(readstr);
    line >> sName >> sType;
   if ( sType.EqualTo("DSSD", TString::kIgnoreCase) ) {
     Int_t nParams = 5;
     Double_t par[10];
     for (Int_t iParam = 0; iParam < nParams; iParam++) {
       line >> par[iParam];
     }
     for (Int_t i=0; i<10; i++) std::cout << par[i] << " " << std::endl;
   }

    //}
    LOG(INFO) << GetName() << " " << readstr << FairLogger::endl;
    //std::cout << GetName() << " " << line << std::endl;
    LOG(INFO) << GetName() << " Sensor name is  " << sName << FairLogger::endl;
  }
  testFile.close();

}
// -------------------------------------------------------------------------



// -----   Set the operating parameters for the sensors   ------------------
// TODO: Currently, all sensors have the same parameters. In future,
// more flexible schemes must be used (initialisation from a database).
void CbmStsDigitize::SetSensorConditions() {

	// --- Current parameters are hard-coded
	Double_t vDep        =  70.;    //depletion voltage, V
	Double_t vBias       = 140.;    //bias voltage, V
	Double_t temperature = 268.;    //temperature of sensor, K
	Double_t cCoupling   =  17.5;   //coupling capacitance, pF
	Double_t cInterstrip =   1.;    //inter-strip capacitance, pF
	fDigiPar->SetSensorConditions(vDep, vBias, temperature,
	                               cCoupling, cInterstrip);
	Int_t nSensors = fSetup->SetSensorConditions();

	LOG(INFO) << GetName() << ": Set conditions for "
			      << nSensors << " sensors " << FairLogger::endl;
}
// -------------------------------------------------------------------------



ClassImp(CbmStsDigitize)

