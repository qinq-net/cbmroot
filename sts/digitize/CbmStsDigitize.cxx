/** @file CbmStsDigitize.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 23.05.2014
 **/

// Include class header
#include "CbmStsDigitize.h"

// Includes from C++
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
#include "FairMCPoint.h"
#include "FairRunAna.h"

// Includes from CbmRoot
#include "CbmDaqBuffer.h"
#include "CbmStsDigi.h"
#include "CbmMCBuffer.h"
#include "CbmStsPoint.h"

// Includes from STS
#include "setup/CbmStsModule.h"
#include "setup/CbmStsSensor.h"
#include "setup/CbmStsSensorConditions.h"
#include "setup/CbmStsSetup.h"
#include "digitize/CbmStsPhysics.h"
#include "digitize/CbmStsSensorTypeDssd.h"
#include "digitize/CbmStsSensorTypeDssdIdeal.h"
#include "digitize/CbmStsSensorTypeDssdReal.h"



// -----   Standard constructor   ------------------------------------------
CbmStsDigitize::CbmStsDigitize(Int_t digiModel)
  : FairTask("StsDigitize"),
    fMode(0),
    fDigiModel(digiModel),
    fDynRange(0.),
    fThreshold(0.),
    fNofAdcChannels(0),
    fTimeResolution(0.),
    fDeadTime(0.),
    fNoise(0.),
    fSetup(NULL),
    fPoints(NULL),
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
    fNofSteps(0),
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



// -----   Create a digi object   ------------------------------------------
void CbmStsDigitize::CreateDigi(UInt_t address,
		              							ULong64_t time,
		              							UShort_t adc,
		              							const CbmMatch& match) {

	// Copy match object. Will be deleted in the digi destructor.
	CbmMatch* digiMatch = new CbmMatch();
	digiMatch->AddLink(match);

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
		CbmMatch* digiMatch2 = new ( (*fMatches)[nDigis] ) CbmMatch();
		digiMatch2->AddLink(match);
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
void CbmStsDigitize::Exec(Option_t* opt) {

	// --- Start timer and reset counters
	fTimer.Start();
	Reset();

	// --- Process StsPoints according to run mode
	switch (fMode) {
		case 0:  ProcessMCBuffer(); break;
		case 1:  ProcessMCEvent();  break;
		default: LOG(FATAL) << GetName() << ": unknown run mode " << fMode
				                << FairLogger::endl; break;
	}

	// --- Readout time: in stream mode the time of the last processed StsPoint.
	// --- Analog buffers will be digitised for signals at times smaller than
	// --- that time minus a safety margin depending on the module properties
	// --- (dead time and time resolution). In event mode, the readout time
	// --- is set to -1., meaning to digitise everything in the readout buffers.
	Double_t readoutTime = fMode == 0 ? fTimePointLast : -1.;

	// --- After the last event, process the remaining analog buffer
	if ( strncmp(opt, "finish", 6 ) == 0 ) readoutTime = -1.;

  // --- Process buffers of all modules
  for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++)
  	fSetup->GetModule(iModule)->ProcessAnalogBuffer(readoutTime);
  LOG(DEBUG) << GetName() << ": " << fNofDigis
  		       << ( fNofDigis == 1 ? " digi " :  " digis " ) << "sent to DAQ ";
  if ( fNofDigis )
  	LOG(DEBUG) << "( from "
 	             << setprecision(3) << fTimeDigiFirst << " ns to "
  		         << fTimeDigiLast << " ns )";
  LOG(DEBUG) << FairLogger::endl;

  // --- Check status of analog module buffers
  if ( gLogger->IsLogNeeded(DEBUG)) {
  	Int_t nSignalsTot  = 0;
  	Double_t timeFirst = -1;
  	Double_t timeLast  = -1.;
  	Int_t nSignals;
  	Double_t tFirst;
  	Double_t tLast;
    for (Int_t iModule = 0; iModule < fSetup->GetNofModules(); iModule++) {
    	fSetup->GetModule(iModule)->BufferStatus(nSignals, tFirst, tLast);
    	if ( nSignals ) {
    		nSignalsTot += nSignals;
    		timeFirst = timeFirst < 0. ? tFirst : TMath::Min(timeFirst, tFirst);
    		timeLast  = TMath::Max(timeLast, tLast);
    	} //? signals in module buffer?
    } // modules in setup
    LOG(DEBUG) << GetName() << ": " << nSignalsTot
    		       << ( nSignalsTot == 1 ? " signal " : " signals " )
    		       << "in analog buffers";
    if ( nSignalsTot )
    	LOG(DEBUG) << " ( from " << setprecision(3) << timeFirst << " ns to "
    	           << timeLast << " ns.";
    LOG(DEBUG) << FairLogger::endl;
  } //? Log needed?


  // --- Event log
  LOG(INFO) << "+ " << setw(20) << GetName() << ": step  " << setw(6)
  		      << right << fNofSteps << ", time " << fixed << setprecision(6)
  		      << fTimer.RealTime() << " s, points: " << fNofPoints
  		      << ", signals: " << fNofSignalsF << " / " << fNofSignalsB
  		      << ", digis: " << fNofDigis << FairLogger::endl;

  // --- Counters
  fTimer.Stop();
  fNofSteps++;
  fNofPointsTot   += fNofPoints;
  fNofSignalsFTot += fNofSignalsF;
  fNofSignalsBTot += fNofSignalsB;
  fNofDigisTot    += fNofDigis;
  fTimeTot        += fTimer.RealTime();

}
// -------------------------------------------------------------------------



// -----   Finish run    ---------------------------------------------------
void CbmStsDigitize::Finish() {

	// --- In stream mode: process the remaining points in the MCBuffer
	if ( fMode == 0 ) Exec("finish");

	std::cout << std::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
	LOG(INFO) << GetName() << ": Run summary" << FairLogger::endl;
	LOG(INFO) << "Steps processed  : " << fNofSteps << FairLogger::endl;
	LOG(INFO) << "StsPoint / step  : " << setprecision(1)
			      << fNofPointsTot / Double_t(fNofSteps)
			      << FairLogger::endl;
	LOG(INFO) << "Signals / step   : "
			      << fNofSignalsFTot / Double_t(fNofSteps)
			      << " / " << fNofSignalsBTot / Double_t(fNofSteps)
			      << FairLogger::endl;
	LOG(INFO) << "StsDigi / step   : " << fNofDigisTot  / Double_t(fNofSteps)
			      << FairLogger::endl;
	LOG(INFO) << "Digis per point  : " << setprecision(6)
			      << fNofDigisTot / fNofPointsTot << FairLogger::endl;
	LOG(INFO) << "Digis per signal : "
			      << fNofDigisTot / ( fNofSignalsFTot + fNofSignalsBTot )
			      << FairLogger::endl;
	LOG(INFO) << "Time per step    : " << fTimeTot / Double_t(fNofSteps)
			      << " s" << FairLogger::endl;
	LOG(INFO) << "=====================================" << FairLogger::endl;
}
// -------------------------------------------------------------------------



// -----   Initialisation    -----------------------------------------------
InitStatus CbmStsDigitize::Init() {

	std::cout << std::endl;
  LOG(INFO) << "=========================================================="
		        << FairLogger::endl;
 	LOG(INFO) << GetName() << ": Initialisation" << FairLogger::endl;
	LOG(INFO) << FairLogger::endl;

	// If the tasks CbmMCTimeSim and CbmDaq are found, run in stream mode.
	FairTask* timeSim = FairRunAna::Instance()->GetTask("MCTimeSim");
	FairTask* daq     = FairRunAna::Instance()->GetTask("Daq");
	if ( timeSim && daq ) {
  	LOG(INFO) << GetName() << ": Using stream mode."
  			      << FairLogger::endl;
  	fMode = 0;
	}  //? stream mode


  // --- Else: run in event-based mode.
  else {
  	LOG(INFO) << GetName() << ": Using event mode."
  			      << FairLogger::endl;
  	fMode = 1;

  	// Get input array (CbmStsPoint)
    FairRootManager* ioman = FairRootManager::Instance();
    if ( ! ioman ) Fatal("Init", "No FairRootManager");
    fPoints = (TClonesArray*) ioman->GetObject("StsPoint");
    if ( ! fPoints )
    	LOG(FATAL) << GetName() << ": No StsPoint array in event-based mode!"
      << FairLogger::endl;

    // Register output array (CbmStsDigi)
    fDigis = new TClonesArray("CbmStsDigi",1000);
    ioman->Register("StsDigi", "Digital response in STS", fDigis, kTRUE);

    // Register output array (CbmStsDigiMatch)
    // For backward compatibility only; the match object is already member
    // of CbmStsDigi.
    fMatches = new TClonesArray("CbmMatch", 1000);
    ioman->Register("StsDigiMatch", "MC link to StsDigi", fMatches, kTRUE);

  } //? event mode

  // Get STS setup interface
  fSetup = CbmStsSetup::Instance();

  // Instantiate StsPhysics
  CbmStsPhysics::Instance();

  // Assign types to the sensors in the setup
  SetSensorTypes();

  // Set sensor conditions
  SetSensorConditions();

  // Set the digitisation parameters of the modules
  SetModuleParameters();

  // Register this task to the setup
  fSetup->SetDigitizer(this);

  LOG(INFO) << GetName() << ": Initialisation successful"
		    << FairLogger::endl;
  LOG(INFO) << "=========================================================="
		        << FairLogger::endl;
	std::cout << std::endl;

	return kSUCCESS;

}
// -------------------------------------------------------------------------



// -----   Process points from MCBuffer   ----------------------------------
void CbmStsDigitize::ProcessMCBuffer() {

	// Time of first and last point
	Double_t timeFirst = -1.;
	Double_t timeLast  = -1.;

  // Loop over StsPoints from MCBuffer
  const CbmStsPoint* point = dynamic_cast<const CbmStsPoint*>
                             (CbmMCBuffer::Instance()->GetNextPoint(kSTS));
  while ( point ) {

  	// --- Set time of first point
  	if ( fNofPoints == 0 ) timeFirst = point->GetTime();

  	// --- Update time of last point
  	timeLast = TMath::Max(timeLast, point->GetTime());

  	// --- Entry (event) number
  	Int_t entry = point->GetEventID();

  	// --- Point index in original array
  	// TODO: This uses the FairLink set in CbmMCPointBuffer::Fill.
  	// To be redone with redesign of MCPoint classes.
    Int_t index = (point->GetLink(0)).GetIndex();

    CbmLink* link = new CbmLink(1., index, entry);
    LOG(DEBUG2) << GetName() << ": Processing point at " << index
    		     	  << ", event " << entry << ", time " << point->GetTime()
    		        << " ns" << FairLogger::endl;
    ProcessPoint(point, link);
    fNofPoints++;
    delete link;

    // Next StsPoint
    point = dynamic_cast<const CbmStsPoint*>
                        (CbmMCBuffer::Instance()->GetNextPoint(kSTS));

  } // Point loop

  // --- Control output
  LOG(DEBUG) << GetName() << ": read " << fNofPoints
  		       << ( fNofPoints == 1 ? "point" : " points" );
  if ( fNofPoints) LOG(DEBUG) << " from " << fixed << setprecision(3)
  		                        << timeFirst << " ns to " << timeLast
  		                        << " ns.";
  LOG(DEBUG) << FairLogger::endl;

  // --- Store time of last StsPoint
  fTimePointLast = timeLast;

}
// -------------------------------------------------------------------------



// -----   Process points from MC event    ---------------------------------
void CbmStsDigitize::ProcessMCEvent() {

	// --- MC Event number
	Int_t iEvent =
			FairRunAna::Instance()->GetEventHeader()->GetMCEntryNumber();

	// --- Check for input array. Should be caught in Init method.
	if ( ! fPoints ) {
		LOG(FATAL) << GetName() << ": No input array of CbmStsPoint!"
				       << FairLogger::endl;
		return;
	}
	if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG) ) std::cout << std::endl;
	LOG(DEBUG) << GetName() << ": processing event " << iEvent << " with "
			       << fPoints->GetEntriesFast() << " StsPoints "
			       << FairLogger::endl;


  // --- Loop over all StsPoints and execute the ProcessPoint method
  for (Int_t iPoint=0; iPoint<fPoints->GetEntriesFast(); iPoint++) {
   const CbmStsPoint* point = (const CbmStsPoint*) fPoints->At(iPoint);
   CbmLink* link = new CbmLink(1., iPoint);
   ProcessPoint(point, link);
   fNofPoints++;
   delete link;
  }  // StsPoint loop

}
// -------------------------------------------------------------------------



// -----  Process a StsPoint   ---------------------------------------------
void CbmStsDigitize::ProcessPoint(const CbmStsPoint* point, CbmLink* link) {

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
	if ( ! sensor ) LOG(ERROR) << GetName() << ": Sensor of StsPoint not found!"
			                       << FairLogger::endl;
	LOG(DEBUG2) << GetName() << ": Sending point to sensor "
			        << sensor->GetName() << " ( " << sensor->GetAddress()
			        << " ) " << FairLogger::endl;

	// --- Process the point on the sensor
	Int_t status = sensor->ProcessPoint(point, link);

	// --- Statistics
	Int_t nSignalsF = status / 1000;
	Int_t nSignalsB = status - 1000 * nSignalsF;
	LOG(DEBUG2) << GetName() << ": Produced signals: " << nSignalsF << " / "
			        << nSignalsB << FairLogger::endl;
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

 // --- Set parameters for all modules
	Int_t nModules = fSetup->GetNofModules();
	for (Int_t iModule = 0; iModule < nModules; iModule++) {
		fSetup->GetModule(iModule)->SetParameters(2048, fDynRange, fThreshold,
				                                      fNofAdcChannels,
				                                      fTimeResolution,
				                                      fDeadTime,
				                                      fNoise);
	}
	LOG(INFO) << GetName() << ": Set parameters for " << nModules
			      << " modules " << FairLogger::endl;
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
	Double_t cCoupling   = 100.;    //coupling capacitance, pF
	Double_t cInterstrip =   3.5;   //inter-strip capacitance, pF

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
		Double_t field[3];
		Double_t local[3] = { 0., 0., 0.}; // sensor centre in local C.S.
		Double_t global[3];               // sensor centre in global C.S.
		sensor->GetNode()->GetMatrix()->LocalToMaster(local, global);
		FairRunAna::Instance()->GetField()->Field(global, field);
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



// --- Set the types for the sensors in the setup --------------------------
// TODO: Currently hard-coded to be SensorTypeDssd. Should be made more
// flexible in the future.
void CbmStsDigitize::SetSensorTypes() {

	TString model;
	switch (fDigiModel) {
		case 0:  model = "ideal";   break;
		case 1:  model = "simple"; break;
		case 2:  model = "real"; break;
		default: model = "unknown"; break;
	}

	LOG(INFO) << GetName() << ": Detector response model: " << model
			      << FairLogger::endl;
	Int_t nSensors = fSetup->GetNofSensors();
	Int_t nTypes[6] = {0, 0, 0, 0, 0, 0};

	for (Int_t iSensor = 0; iSensor < fSetup->GetNofSensors(); iSensor++) {
		CbmStsSensor* sensor = fSetup->GetSensor(iSensor);
		TGeoBBox* shape = dynamic_cast<TGeoBBox*>
											(sensor->GetPnode()->GetVolume()->GetShape());
		Double_t volX = 2. * shape->GetDX();
		Double_t volY = 2. * shape->GetDY();

		// --- Instantiate sensor type object
		CbmStsSensorTypeDssd* type = NULL;
		switch (fDigiModel) {
			case 0:  type = new CbmStsSensorTypeDssdIdeal();
			         type->SetTitle("DssdIdeal");
			         break;
			case 1:  type = new CbmStsSensorTypeDssd();
			         type->SetTitle("Dssd");
			         break;
			case 2:	 type = new CbmStsSensorTypeDssdReal();
					     type->SetTitle("DssdReal");
					     break;
			default: type = NULL;
							 break;
		}
		if ( ! type ) {
			LOG(FATAL) << GetName() << ": Unknown sensor response model "
					       << fDigiModel << FairLogger::endl;
			continue;
		}
		sensor->SetType(type);

		// Sensor01: 4 x 2.2
		if ( volX > 3.99 && volX < 4.01 ) {
			Double_t lx = 3.7584;   // active size in x
			Double_t ly = 1.96;     // active size in y
			Double_t lz = 0.03;     // active thickness
			Int_t nStripsF = 648;   // number of strips front side (58 mum)
			Int_t nStripsB = 648;   // number of strips back side  (58 mum)
			Double_t stereoF = 0.;  // stereo angle front side
			Double_t stereoB = 7.5; // stereo angle back side
			type->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
			type->SetName("Sensor01");
			nTypes[0]++;
		} //? Sensor01

		// Sensor02: 6.2 x 2.2
		else if ( volX > 6.19 && volX < 6.21 && volY > 2.19 && volY < 2.21) {
			Double_t lx = 5.9392;   // active size in x
			Double_t ly = 1.96;     // active size in y
			Double_t lz = 0.03;     // active thickness
			Int_t nStripsF = 1024;  // number of strips front side (58 mum)
			Int_t nStripsB = 1024;  // number of strips back side  (58 mum)
			Double_t stereoF = 0.;  // stereo angle front side
			Double_t stereoB = 7.5; // stereo angle back side
			type->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
			type->SetName("Sensor02");
			nTypes[1]++;
		} //? Sensor02

		// Sensor03: 6.2 x 4.2
		else if ( volX > 6.19 && volX < 6.21 && volY > 4.19 && volY < 4.21) {
			Double_t lx = 5.9392;   // active size in x
			Double_t ly = 3.96;     // active size in y
			Double_t lz = 0.03;     // active thickness
			Int_t nStripsF = 1024;  // number of strips front side (58 mum)
			Int_t nStripsB = 1024;  // number of strips back side  (58 mum)
			Double_t stereoF = 0.;  // stereo angle front side
			Double_t stereoB = 7.5; // stereo angle back side
			type->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
			type->SetName("Sensor03");
			nTypes[2]++;
		} //? Sensor03

		// Sensor04: 6.2 x 6.2
		else if ( volX > 6.19 && volX < 6.21 && volY > 6.19 && volY < 6.21) {
			Double_t lx = 5.9392;   // active size in x
			Double_t ly = 5.96;     // active size in y
			Double_t lz = 0.03;     // active thickness
			Int_t nStripsF = 1024;  // number of strips front side (58 mum)
			Int_t nStripsB = 1024;  // number of strips back side  (58 mum)
			Double_t stereoF = 0.;  // stereo angle front side
			Double_t stereoB = 7.5; // stereo angle back side
			type->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
			type->SetName("Sensor04");
			nTypes[3]++;
		} //? Sensor04

		// Sensor05: 3.1 x 3.1
		else if ( volX > 3.09 && volX < 3.11 && volY > 3.09 && volY < 3.11) {
			Double_t lx = 2.8594;   // active size in x
			Double_t ly = 2.86;     // active size in y
			Double_t lz = 0.03;     // active thickness
			Int_t nStripsF = 493;   // number of strips front side (58 mum)
			Int_t nStripsB = 493;   // number of strips back side  (58 mum)
			Double_t stereoF = 0.;  // stereo angle front side
			Double_t stereoB = 7.5; // stereo angle back side
			type->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
			type->SetName("Sensor05");
			nTypes[4]++;
		} //? Sensor05

		// Sensor06: 1.5 x 4.2
		else if ( volX > 1.49 && volX < 1.51 && volY > 4.19 && volY < 4.21) {
			Double_t lx = 1.2586;   // active size in x
			Double_t ly = 3.96;     // active size in y
			Double_t lz = 0.03;     // active thickness
			Int_t nStripsF = 217;   // number of strips front side (58 mum)
			Int_t nStripsB = 217;   // number of strips back side  (58 mum)
			Double_t stereoF = 0.;  // stereo angle front side
			Double_t stereoB = 7.5; // stereo angle back side
			type->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
			type->SetName("Sensor06");
			nTypes[5]++;
		} //? Sensor06

		// Unknown sensor type
		else {
			LOG(FATAL) << GetName() << ": Unknown sensor type with x = " << volX
					       << ", y = " << volY << FairLogger::endl;
			continue;
		}  //? unknown type

		sensor->SetType(type);
	} //? Loop over sensors

	LOG(INFO) << GetName() << ": Set types for " << nSensors
			      << " sensors ( " << nTypes[0];
	for (Int_t iType = 0; iType < 6; iType++)
		LOG(INFO) << " / " << nTypes[iType];

	LOG(INFO) << " ) " << FairLogger::endl;

}
// -------------------------------------------------------------------------


ClassImp(CbmStsDigitize)

