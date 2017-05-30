/** @file CbmStsDigitize.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.05.2014
 **/

// Include class header
#include "CbmStsDigitize.h"

// Includes from C++
#include <cassert>
#include <cstring>
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
#include "digitize/CbmStsSensorTypeDssd.h"

using std::fixed;
using std::right;
using std::setprecision;
using std::setw;
using std::string;

// -----   Static member variables   ---------------------------------------
// The default setting is the fully realistic response.
Int_t  CbmStsDigitize::fElossModel      = 2;  //  energy loss fluctuations
Bool_t CbmStsDigitize::fUseLorentzShift = kFALSE;  // Lorentz shift off
Bool_t CbmStsDigitize::fUseDiffusion    = kTRUE;  // Diffusion on
Bool_t CbmStsDigitize::fUseCrossTalk    = kTRUE;  // Cross talk on
Bool_t CbmStsDigitize::fIsInitialised   = kFALSE;
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmStsDigitize::CbmStsDigitize()
  : FairTask("StsDigitize"),
    fMode(0),
    fProcessSecondaries(kTRUE),
    fDynRange(40960.),
    fThreshold(4000.),
    fNofAdcChannels(4096),
    fTimeResolution(5.),
    fDeadTime(9999999.),
    fNoise(0.),
    fDeadChannelFraction(0.),
    fStripPitch(-1.),
    fSetup(NULL),
    fPoints(NULL),
    fTracks(NULL),
    fDigis(NULL),
    fMatches(NULL),
    fTimer(),
    fTimePointLast(-1.),
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
void CbmStsDigitize::CreateDigi(UInt_t address,
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
		CbmStsDigi* digi = new CbmStsDigi(address, time, adc);
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
				new ( (*fDigis)[nDigis] ) CbmStsDigi(address, time, adc);
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
			        << ", channel "
			        << CbmStsAddress::GetElementId(address, kStsChannel)
	            << ")" << FairLogger::endl;

}
// -------------------------------------------------------------------------



// -----   Task execution   ------------------------------------------------
void CbmStsDigitize::Exec(Option_t* /*opt*/) {

	// --- Start timer and reset counters
	fTimer.Start();
	Reset();

	// --- For debug: status of analog buffers
  if ( gLogger->IsLogNeeded(DEBUG)) {
   	std::cout << std::endl;
    LOG(DEBUG) << GetName() << ": " << BufferStatus() << FairLogger::endl;
  }

	// --- Analog reponse: Process the input array of StsPoints
	ProcessMCEvent();
	LOG(DEBUG) << GetName() << ": " << fNofSignalsF + fNofSignalsB
			       << " signals generated ( "
			       << fNofSignalsF << " / " << fNofSignalsB << " )"
			       << FairLogger::endl;
	// --- For debug: status of analog buffers
  if ( gLogger->IsLogNeeded(DEBUG)) {
    LOG(DEBUG) << GetName() << ": " << BufferStatus() << FairLogger::endl;
  }

	// --- Readout time: in stream mode the time of the last processed StsPoint.
	// --- Analog buffers will be digitised for signals at times smaller than
	// --- that time minus a safety margin depending on the module properties
	// --- (dead time and time resolution). In event mode, the readout time
	// --- is set to -1., meaning to digitise everything in the readout buffers.
	Double_t eventTime
		= FairRun::Instance()->GetEventHeader()->GetEventTime();
	Double_t readoutTime = fMode == 0 ? eventTime : -1.;
	LOG(DEBUG) << GetName() << ": Readout time is " << readoutTime << " ns"
			       << FairLogger::endl;

  // --- Digital response: Process buffers of all modules
	ProcessAnalogBuffers(readoutTime);


  // --- Check status of analog module buffers
  if ( gLogger->IsLogNeeded(DEBUG)) {
    LOG(DEBUG) << GetName() << ": " << BufferStatus() << FairLogger::endl;
  }

  // --- Event log
  LOG(INFO) << "+ " << setw(20) << GetName() << ": Event " << setw(6)
  		      << right << fNofEvents << ", real time " << fixed
  		      << setprecision(6) << fTimer.RealTime() << " s, points: "
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
	LOG(INFO) << "Real time per event : " << fTimeTot / Double_t(fNofEvents)
			      << " s" << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Get event information   -----------------------------------------
void CbmStsDigitize::GetEventInfo(Int_t& inputNr, Int_t& eventNr,
		                              Double_t& eventTime) {


	// --- In a FairRunAna, take the information from FairEventHeader
	if ( FairRunAna::Instance() ) {
		FairEventHeader* event = FairRunAna::Instance()->GetEventHeader();
		assert ( event );
	  inputNr   = event->GetInputFileId();
	  eventNr = FairRootManager::Instance()->GetEntryNr();
	  eventTime = event->GetEventTime();
	}

	// --- In a FairRunSim, the input number and event time are always zero;
	// --- only the event number is retrieved.
	else {
		if ( ! FairRunSim::Instance() )
			LOG(FATAL) << GetName() << ": neither SIM nor ANA run."
					       << FairLogger::endl;
		inputNr   = 0;
	    eventNr = FairRootManager::Instance()->GetEntryNr();
		eventTime = 0.;
	}

}
// -------------------------------------------------------------------------



// -----   Initialisation    -----------------------------------------------
InitStatus CbmStsDigitize::Init() {

  // Get STS setup interface
  fSetup = CbmStsSetup::Instance();

	std::cout << std::endl;
  LOG(INFO) << "=========================================================="
		        << FairLogger::endl;
 	LOG(INFO) << GetName() << ": Initialisation" << FairLogger::endl;
	LOG(INFO) << FairLogger::endl;

	// If the task CbmDaq is found, run in stream mode.
	FairTask* daq     = FairRun::Instance()->GetTask("Daq");
	if ( daq ) {
  	LOG(INFO) << GetName() << ": Using stream mode."
  			      << FairLogger::endl;
  	fMode = 0;
	}  //? stream mode

  // --- Else: run in event-based mode.
  else {
  	LOG(INFO) << GetName() << ": Using event mode."
  			      << FairLogger::endl;
  	fMode = 1;
  }

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

  // Processes
  TString model;
  switch ( fElossModel) {
    case 0: model = "IDEAL"; break;
    case 1: model = "UNIFORM"; break;
    case 2: model = "NON-UNIFORM"; break;
    default: LOG(FATAL) << GetName() << ": Unknown energy loss model "
        << fElossModel << FairLogger::endl;
  } //? fElossModel
  LOG(INFO) << GetName() << ": Energy loss model " << model
      << FairLogger::endl;
  LOG(INFO) << GetName() << ": Lorentz-Shift     "
      << (fUseLorentzShift ? "ON" : "OFF") << FairLogger::endl;
  LOG(INFO) << GetName() << ": Diffusion         "
      << (fUseDiffusion ? "ON" : "OFF") << FairLogger::endl;
  LOG(INFO) << GetName() << ": Cross-Talk        "
      << (fUseCrossTalk ? "ON" : "OFF") << FairLogger::endl;

	// Initialise STS setup
	InitSetup();

  // Instantiate StsPhysics
  CbmStsPhysics::Instance();

  // Register this task to the setup
  fSetup->SetDigitizer(this);

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


	// Modify the strip pitch for DSSD sensor type, if explicitly set by user
	Int_t nModified = 0;
	if ( fStripPitch > 0. ) {
		Int_t nTypes = fSetup->GetNofSensorTypes();
		for (Int_t iType = 0; iType < nTypes; iType++) {
			CbmStsSensorType* type = fSetup->GetSensorType(iType);
			if ( ! type ) continue;
			// Skip types other than DSSD
			if ( type->InheritsFrom("CbmStsSensorTypeDssd") ) {
				CbmStsSensorTypeDssd* dssdType =
						dynamic_cast<CbmStsSensorTypeDssd*>(type);
				dssdType->SetStripPitch(fStripPitch);
				nModified++;
			} //? DSSD type
		} //# sensor types
		LOG(INFO) << GetName() << ": modified strip pitch to " << fStripPitch
				      << " cm for "<< nModified << " sensor types."
				      << FairLogger::endl;
	} //? strip pitch set by user

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
  	if ( ! fProcessSecondaries ) {
  		Int_t iTrack = point->GetTrackID();
  		if ( iTrack >= 0 ) {  // MC track is present
  			CbmMCTrack* track = (CbmMCTrack*) fTracks->At(iTrack);
  			assert ( track );
  			if ( track->GetMotherId() >= 0 ) continue;
  		} //? MC track present
  	}

  	ProcessPoint(point, eventTime, link);
  	fNofPoints++;
  	delete link;
  }  // StsPoint loop

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
	UInt_t address = point->GetDetectorID();
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



// -----   Set the digitisation parameters for the modules   ---------------
// TODO: Currently, all modules have the same parameters. In future,
// more flexible schemes must be used, in particular for the thresholds.
void CbmStsDigitize::SetModuleParameters() {

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
	LOG(INFO) << "\t Dead channel fraction " << setw(10) << right
			      << fDeadChannelFraction << " %" << FairLogger::endl;

 // --- Set parameters for all modules
	Int_t nModules = fSetup->GetNofModules();
	for (Int_t iModule = 0; iModule < nModules; iModule++) {
		fSetup->GetModule(iModule)->SetParameters(2048, fDynRange, fThreshold,
				                                      fNofAdcChannels,
				                                      fTimeResolution,
				                                      fDeadTime,
				                                      fNoise);
		fSetup->GetModule(iModule)->SetDeadChannels(fDeadChannelFraction);
	}
	LOG(INFO) << GetName() << ": Set parameters for " << nModules
			      << " modules " << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Set the percentage of dead channels   ---------------------------
void CbmStsDigitize::SetDeadChannelFraction(Double_t fraction) {
	if ( fraction < 0. ) {
		LOG(WARNING) << GetName()
				         << ": illegal dead channel fraction " << fraction
				         << "% , is set to 0 %" << FairLogger::endl;
		fDeadChannelFraction = 0.;
		return;
	}
	if ( fraction > 100. ) {
		LOG(WARNING) << GetName()
				         << ": illegal dead channel fraction " << fraction
				         << "% , is set to 100 %" << FairLogger::endl;
		fDeadChannelFraction = 100.;
		return;
	}
	fDeadChannelFraction = fraction;
}
// -------------------------------------------------------------------------



// -----   Set the switches for physical processes for real digitiser model
void CbmStsDigitize::SetProcesses(Int_t eLossModel,
		                              Bool_t useLorentzShift,
		                              Bool_t useDiffusion,
		                              Bool_t useCrossTalk) {
	  if ( fIsInitialised ) {
	  	LOG(ERROR) << "STS digitize: physics processes must be set before "
	  			       << "initialisation! Statement will have no effect."
	  			       << FairLogger::endl;
	  	return;
	  }
	  fElossModel      = eLossModel;
	  fUseLorentzShift = useLorentzShift;
	  fUseDiffusion    = useDiffusion;
	  fUseCrossTalk    = useCrossTalk;
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

	// --- Control output of parameters
	LOG(INFO) << GetName() << ": Sensor operation conditions :"
			      << FairLogger::endl;
	LOG(INFO) << "\t Depletion voltage       " << setw(10) << right
				    << vDep << " V"<< FairLogger::endl;
	LOG(INFO) << "\t Bias voltage            " << setw(10) << right
			      << vBias << " V"<< FairLogger::endl;
	LOG(INFO) << "\t Temperature             " << setw(10) << right
			      << temperature << FairLogger::endl;
	LOG(INFO) << "\t Coupling capacitance    " << setw(10) << right
			      << cCoupling << " pF" << FairLogger::endl;
	LOG(INFO) << "\t Inter-strip capacitance " << setw(10) << right
			      << cInterstrip << " pF" << FairLogger::endl;

	CbmStsSensorConditions cond(vDep, vBias, temperature, cCoupling,
			                        cInterstrip);

	// --- Set conditions for all sensors
	for (Int_t iSensor = 0; iSensor < fSetup->GetNofSensors(); iSensor++) {
		CbmStsSensor* sensor = fSetup->GetSensor(iSensor);
		// -- Get field in sensor centre
		Double_t field[3] = { 0., 0., 0.};
		Double_t local[3] = { 0., 0., 0.}; // sensor centre in local C.S.
		Double_t global[3];               // sensor centre in global C.S.
		sensor->GetNode()->GetMatrix()->LocalToMaster(local, global);
		if ( FairRun::Instance()->GetField() )
		    FairRun::Instance()->GetField()->Field(global, field);
		cond.SetField(field[0]/10., field[1]/10., field[2]/10.); // kG->T !
    // --- Set the condition container
		sensor->SetConditions(cond);
		LOG(DEBUG1) << sensor->GetName() << ": conditions "
				      << sensor->GetConditions().ToString() << FairLogger::endl;
	} // sensor loop

	LOG(INFO) << GetName() << ": Set conditions for "
			      << fSetup->GetNofSensors() << " sensors " << FairLogger::endl;
}
// -------------------------------------------------------------------------



ClassImp(CbmStsDigitize)

