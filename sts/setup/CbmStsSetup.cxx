/** @file CbmStsSetup.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 27.05.2013
 **/

// Include class header
#include "CbmStsSetup.h"

// Includes from C++
#include <cassert>
#include <fstream>
#include <iostream>

// Includes from ROOT
#include "TFile.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TKey.h"
#include "TSystem.h"

// Includes form FairRoot
#include "FairField.h"

// Includes from CbmRoot
#include "CbmStsAddress.h"
#include "CbmStsDigitizeParameters.h"
#include "CbmStsMC.h"
#include "CbmStsModule.h"
#include "CbmStsPhysics.h"
#include "CbmStsSensor.h"
#include "CbmStsSensorDssd.h"
#include "CbmStsSensorDssdStereo.h"
#include "CbmStsStation.h"

using namespace std;


// -----   Initialisation of static singleton pointer   --------------------
CbmStsSetup* CbmStsSetup::fgInstance = NULL;
// -------------------------------------------------------------------------



// -----   Constructor   ---------------------------------------------------
CbmStsSetup::CbmStsSetup() : CbmStsElement(kSts, kStsSystem),
			     fDigitizer(NULL), fSettings(NULL),fIsInitialised(kFALSE),
			     fNofSensorsDefault(0),
			     fIsOld(kFALSE), fModules(),
			     fSensors(), fStations() {
}
// -------------------------------------------------------------------------



// -----   Assign a sensor object to an address   --------------------------
CbmStsSensor* CbmStsSetup::AssignSensor(Int_t address,
                                        TGeoPhysicalNode* node) {

  CbmStsSensor* sensor = nullptr;
  auto it = fSensors.find(address);
  if ( it != fSensors.end() ) {
    sensor = it->second; // Found in sensor list
    assert (sensor->GetAddress() == address);
    sensor->SetNode(node);
    sensor->Init();
    LOG(DEBUG1) << GetName() << ": Assigning " << sensor->ToString()
        << "\n\t\t to node " << node->GetName() << FairLogger::endl;
  }
  else {
    sensor = DefaultSensor(address, node); // Not found; create and add.
    LOG(DEBUG1) << GetName() << ": Assigning default " << sensor->ToString()
        << "\n\t\t to node " << node->GetName() << FairLogger::endl;
    fNofSensorsDefault++;
  }

  return sensor;
}
// -------------------------------------------------------------------------



// -----   Create station objects   ----------------------------------------
Int_t CbmStsSetup::CreateStations() {

  // For old geometries: the station corresponds to the unit
  if ( fIsOld ) {

    for (Int_t iUnit = 0; iUnit < GetNofDaughters(); iUnit++) {
      CbmStsElement* unit = GetDaughter(iUnit);

      // Create one station for each unit
      Int_t stationId = unit->GetIndex();
      TString name = Form("STS_S%02i", stationId+1);
      TString title = Form("STS Station %i", stationId+1);
      CbmStsStation* station = new CbmStsStation(name, title,
                                                 unit->GetPnode());
      // Add all ladders of the unit to the station
      for (Int_t iLadder = 0; iLadder < unit->GetNofDaughters(); iLadder++)
        station->AddLadder(unit->GetDaughter(iLadder));
      // Initialise station parameters
      station->Init();
      // Add station to station map
      assert ( fStations.find(stationId) == fStations.end() );
      fStations[stationId] = station;
    } //# units

    return fStations.size();
  } //? is old geometry?


  // Loop over all ladders. They are associated to a station.
  for (Int_t iUnit = 0; iUnit < GetNofDaughters(); iUnit++) {
    CbmStsElement* unit = GetDaughter(iUnit);
    for ( Int_t iLadder = 0; iLadder < unit->GetNofDaughters(); iLadder++) {
      CbmStsElement* ladder = unit->GetDaughter(iLadder);
      // This convention must be followed by the STS geometry
      Int_t nodeNumber = ladder->GetPnode()->GetNode()->GetNumber();
      Int_t stationId = nodeNumber / 100 - 1;
      // Get the station from the map. If not there, create it.
      CbmStsStation* station = NULL;
      if ( fStations.find(stationId) == fStations.end() ) {
        TString name = Form("STS_S%02i", stationId+1);
        TString title = Form("STS Station %i", stationId+1);
        station = new CbmStsStation(name, title);
        fStations[stationId] = station;
      } //? station not yet in map
      else station = fStations[stationId];

      // Add ladder to station
      station->AddLadder(ladder);

    } //# ladders
  } //# units

  // Initialise the stations
  auto it = fStations.begin();
  while ( it != fStations.end() ) {
    it->second->Init();
    it++;
  } //# stations

  // Check that the station number is set consecutively and that the
  // stations are ordered w.r.t. position along the beam
  Bool_t isOk = kTRUE;
  Double_t zPrevious = -999999;
  for (UInt_t iStation = 0; iStation < fStations.size(); iStation++) {
    if ( fStations.find(iStation) == fStations.end() ) {
      LOG(ERROR) << GetName() << ": Number of stations is "
          << fStations.size() << ", but station " << iStation
          << "is not present!" << FairLogger::endl;
      isOk = kFALSE;
    } //? station present?
    if ( fStations[iStation]->GetZ() <= zPrevious ) {
      LOG(ERROR) << GetName() << ": Disordered stations. Station "
          << iStation << " is at z = " << fStations[iStation]->GetZ()
          << "cm , previous is at z = " << zPrevious << " cm."
          << FairLogger::endl;
      isOk = kFALSE;
    } //? disordered in z
  } //# stations
  if ( ! isOk ) LOG(FATAL) << GetName() << ": Error in creation of stations."
      << FairLogger::endl;

  return fStations.size();
}
// -------------------------------------------------------------------------



// -----   Instantiate default sensor   ------------------------------------
CbmStsSensor* CbmStsSetup::DefaultSensor(Int_t address,
                                         TGeoPhysicalNode* node) {

  // There should not already be a sensor object for this address
  assert ( fSensors.find(address) == fSensors.end() );

  // Sensor volume extension in x and y
  assert(node);
  TGeoBBox* shape = dynamic_cast<TGeoBBox*>(node->GetShape());
  assert(shape);
  Double_t volX = 2. * shape->GetDX();
  Double_t volY = 2. * shape->GetDY();

  // Default pitch and stereo angles
  Double_t pitch   = 0.0058;
  Double_t stereoF = 0.;
  Double_t stereoB = 7.5;

  // Size of inactive area in both x and y (total left+right/top+bottom)
  Double_t dInact = 0.24;

  // Number of strips. Calculated from active size in x, except for 6.2 cm
  // sensors, where it is 1024.
  Int_t nStrips = 1024;
  if ( TMath::Abs(volX-6.2) > 0.01 )
    nStrips = Int_t( (volX - dInact) / pitch );

  // Active size in y
  Double_t dy = volY - dInact;

  // Create default sensor type DssdStereo
  CbmStsSensorDssdStereo* sensor
    = new CbmStsSensorDssdStereo(dy, nStrips, pitch, stereoF, stereoB);

  // Assign address and node and add sensor to sensor list
  sensor->SetAddress(address);
  sensor->SetNode(node);
  Bool_t status = sensor->Init();
  assert(status);
  fSensors[address] = sensor;

  return sensor;
}
// -------------------------------------------------------------------------



// -----   Get an element from the STS setup   -----------------------------
CbmStsElement* CbmStsSetup::GetElement(Int_t address, Int_t level) {

	// --- Check for initialisation
	if ( ! fAddress ) LOG(FATAL) << fName << ": not initialised!"
			                         << FairLogger::endl;

	// --- Catch non-STS addresses
	if ( CbmStsAddress::GetSystemId(address) != kSts ) {
		LOG(WARNING) << fName << ": No STS address " << address
				     << FairLogger::endl;
		return NULL;
	}

	// --- Catch illegal level numbers
	if ( level < 0 || level >= kStsNofLevels ) {
		LOG(WARNING) << fName << ": Illegal level " << level
				   << FairLogger::endl;
		return NULL;
	}

	CbmStsElement* element = this;
	for (Int_t iLevel = 1; iLevel <= level; iLevel++) {
		element =
				element->GetDaughter(CbmStsAddress::GetElementId(address, iLevel));
		assert(element);
	}

	return element;
}
// -------------------------------------------------------------------------



// -----   Get hierarchy level name   ---------------------------------------
const char* CbmStsSetup::GetLevelName(Int_t level) {

  // --- Catch legacy (setup with stations)
  if ( fIsOld && level == kStsUnit ) return "station";

  switch(level) {
    case kStsSystem: return "sts"; break;
    case kStsUnit: return "unit"; break;
    case kStsLadder: return "ladder"; break;
    case kStsHalfLadder: return "halfladder"; break;
    case kStsModule: return "module"; break;
    case kStsSensor: return "sensor"; break;
    case kStsSide: return "side"; break;
    default: return ""; break;
  }

}
// -------------------------------------------------------------------------



// -----   Get the station number from an address   ------------------------
Int_t CbmStsSetup::GetStationNumber(Int_t address) {

  // In old, station-based geometries, the station equals the unit
  if ( fIsOld ) return CbmStsAddress::GetElementId(address, kStsUnit);

  // In new, unit-based geometries, the station is obtained from the ladder
  CbmStsElement* ladder = CbmStsSetup::GetElement(address, kStsLadder);
  assert(ladder);
  return ladder->GetPnode()->GetNode()->GetNumber() / 100 - 1;

}
// -------------------------------------------------------------------------



// -----   Initialisation   ------------------------------------------------
Bool_t CbmStsSetup::Init(const char* geoFile, const char* parFile) {

  // Prevent duplicate initialisation
  assert( ! fIsInitialised );

  cout << endl;
  LOG(INFO) << "=========================================================="
                << FairLogger::endl;
  LOG(INFO) << "Initialising STS Setup "
            << FairLogger::endl;
  LOG(INFO) << FairLogger::endl;

  // Read sensor parameters from file, if specified
  if ( parFile ) ReadSensorParameters(parFile);

  // --- Set system address
  fAddress = CbmStsAddress::GetAddress();

  // --- If a geometry file was specified, read the geometry from file
  if ( parFile ) ReadGeometry(parFile);

  // --- Else, read the geometry from TGeoManager
  else {
    assert(gGeoManager);
    ReadGeometry(gGeoManager);
  }

  // --- Statistics
  LOG(INFO) << fName << ": Elements in setup: " << FairLogger::endl;
  for (Int_t iLevel = 1; iLevel <= kStsSensor; iLevel++) {
      TString name = GetLevelName(iLevel);
      name += "s";
      LOG(INFO) << "     " << setw(12) << name << setw(5) << right
                  << GetNofElements(iLevel) << FairLogger::endl;
  }
  LOG(INFO) << GetName() << ": " << fNofSensorsDefault
            << " sensors created from default." << FairLogger::endl;

  // --- Build the module array
  for (Int_t iUnit = 0; iUnit < GetNofDaughters(); iUnit++) {
    CbmStsElement* unit = GetDaughter(iUnit);
    for (Int_t iLad = 0; iLad < unit->GetNofDaughters(); iLad++) {
      CbmStsElement* ladd = unit->GetDaughter(iLad);
      for (Int_t iHla = 0; iHla < ladd->GetNofDaughters(); iHla++) {
        CbmStsElement* hlad = ladd->GetDaughter(iHla);
        for (Int_t iMod = 0; iMod < hlad->GetNofDaughters(); iMod++) {
          CbmStsElement* modu = hlad->GetDaughter(iMod);
          fModules.push_back(dynamic_cast<CbmStsModule*>(modu));
        } //# modules in half-ladder
      } //# half-ladders in ladder
    } //# ladders in unit
  } //# units in system

  // --- Create station objects
  Int_t nStations = CreateStations();
  LOG(INFO) << GetName() << ": Setup contains " << nStations
      << " stations objects." << FairLogger::endl;
  if ( FairLogger::GetLogger()->IsLogNeeded(DEBUG) ) {
    auto it = fStations.begin();
    while ( it != fStations.end() ) {
      LOG(DEBUG) << "  " << it->second->ToString() << FairLogger::endl;
      it++;
    } //# stations
  } //? Debug

  // --- Consistency check
  if ( GetNofSensors() != GetNofElements(kStsSensor) )
    LOG(FATAL) << GetName() << ": inconsistent number of sensors! "
                   << GetNofElements(kStsSensor) << " " << GetNofSensors()
                   << FairLogger::endl;
  if ( fModules.size() != GetNofElements(kStsModule) )
    LOG(FATAL) << GetName() << ": inconsistent number of modules! "
                   << GetNofElements(kStsModule) << " "
                   << fModules.size() << FairLogger::endl;

  LOG(INFO) << "=========================================================="
            << "\n" << FairLogger::endl;
  cout << endl;

  fIsInitialised = kTRUE;
  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Singleton instance   --------------------------------------------
CbmStsSetup* CbmStsSetup::Instance() {
  if ( ! fgInstance ) fgInstance = new CbmStsSetup();
  return fgInstance;
}
// -------------------------------------------------------------------------



// -----   Modify the strip pitch for all sensors   ------------------------
Int_t CbmStsSetup::ModifyStripPitch(Double_t pitch) {

  Int_t nModified = 0;
  for ( auto it = fSensors.begin(); it != fSensors.end(); it++ ) {

    CbmStsSensorDssd* sensor = dynamic_cast<CbmStsSensorDssd*>(it->second);
    if ( ! sensor ) continue;
    sensor->ModifyStripPitch(pitch);
    nModified++;
  }

  return nModified;
}
// -------------------------------------------------------------------------



// -----   Set conditions for all sensors   --------------------------------
Int_t CbmStsSetup::SetSensorConditions() {

  Int_t nSensors = 0;   // Sensor counter

  // Get conditions from parameter object
  assert(fSettings);
  Double_t vDep = fSettings->GetVdep();           // Full depletion voltage
  Double_t vBias = fSettings->GetVbias();         // Bias voltage
  Double_t temperature = fSettings->GetTemperature(); // Temperature
  Double_t cCoup = fSettings->GetCcoup();         // Coupling capacitance
  Double_t cIs = fSettings->GetCis();             // Inter-strip capacitance

  // --- Control output of parameters
  LOG(INFO) << GetName() << ": Set conditions for all sensors:"
            << FairLogger::endl;
  LOG(INFO) << "\t Full depletion voltage   " << vDep
                     << " V"<< FairLogger::endl;
  LOG(INFO) << "\t Bias voltage             " << vBias
                     << " V"<< FairLogger::endl;
  LOG(INFO) << "\t Temperature              " << temperature << " K"
                     << FairLogger::endl;
  LOG(INFO) << "\t Coupling capacitance      " << cCoup
                     << " pF" << FairLogger::endl;
  LOG(INFO) << "\t Inter-strip capacitance   " << cIs
                     << " pF" << FairLogger::endl;

  // --- Set conditions for all sensors
  for ( auto it = fSensors.begin(); it != fSensors.end(); it++ ) {

    // Get sensor centre coordinates in the global c.s.
    Double_t local[3] = { 0., 0., 0.}; // sensor centre in local C.S.
    Double_t global[3];                // sensor centre in global C.S.
    it->second->GetNode()->GetMatrix()->LocalToMaster(local, global);

    // Get the field in the sensor centre. Note that the values are in kG
    // and have to be converted to T
    Double_t field[3] = { 0., 0., 0.};
    if ( FairRun::Instance()->GetField() )
        FairRun::Instance()->GetField()->Field(global, field);
    it->second->SetConditions(vDep, vBias, temperature, cCoup,
                              cIs, field[0]/10., field[1]/10., field[2]/10.);

    nSensors++;
  } //# sensors

  return nSensors;
}
// -------------------------------------------------------------------------



// -----   Set conditions for all sensors   --------------------------------
Int_t CbmStsSetup::SetSensorConditions(Double_t vDep, Double_t vBias,
                                       Double_t temperature,
                                       Double_t cCoupling,
                                       Double_t cInterstrip) {

  Int_t nSensors = 0;   // Sensor counter

  // --- Set conditions for all sensors
  for ( auto it = fSensors.begin(); it != fSensors.end(); it++ ) {

    // Get sensor centre coordinates in the global c.s.
    Double_t local[3] = { 0., 0., 0.}; // sensor centre in local C.S.
    Double_t global[3];                // sensor centre in global C.S.
    it->second->GetNode()->GetMatrix()->LocalToMaster(local, global);

    // Get the field in the sensor centre
    Double_t field[3] = { 0., 0., 0.};
    if ( FairRun::Instance()->GetField() )
        FairRun::Instance()->GetField()->Field(global, field);
    it->second->SetConditions(vDep, vBias, temperature, cCoupling,
                              cInterstrip,
                              field[0]/10., field[1]/10., field[2]/10.);

    nSensors++;
  } //# sensors

  return nSensors;
}
// -------------------------------------------------------------------------



// -----   Read geometry from TGeoManager   --------------------------------
Bool_t CbmStsSetup::ReadGeometry(TGeoManager* geo) {

  // --- Catch non-existence of GeoManager
  assert (geo);
  LOG(INFO) << fName << ": Reading geometry from TGeoManager "
            << geo->GetName() << FairLogger::endl;

  // --- Get cave (top node)
  geo->CdTop();
  TGeoNode* cave = geo->GetCurrentNode();

  // --- Get top STS node
  TGeoNode* sts = NULL;
  for (Int_t iNode = 0; iNode < cave->GetNdaughters(); iNode++) {
    TString name = cave->GetDaughter(iNode)->GetName();
     if ( name.Contains("STS", TString::kIgnoreCase) ) {
      sts = cave->GetDaughter(iNode);
      LOG(INFO) << fName << ": STS top node is " << sts->GetName()
                << FairLogger::endl;
      break;
    }
  }
  if ( ! sts ) {
    LOG(ERROR) << fName << ": No top STS node found in geometry!"
               << FairLogger::endl;
    return kFALSE;
  }

  // --- Create physical node for sts
  TString path = cave->GetName();
  path = path + "/" + sts->GetName();
  fNode = new TGeoPhysicalNode(path);

  // --- Check for old geometry (with stations) or new geometry (with units)
  Bool_t hasStation = kFALSE;
  Bool_t hasUnit = kFALSE;
  for (Int_t iDaughter = 0; iDaughter < fNode->GetNode()->GetNdaughters();
       iDaughter++) {
    TString dName = fNode->GetNode()->GetDaughter(iDaughter)->GetName();
    if ( dName.Contains("station", TString::kIgnoreCase) ) hasStation = kTRUE;
    if ( dName.Contains("unit", TString::kIgnoreCase) ) hasUnit = kTRUE;
  }
  if ( hasUnit && (! hasStation) ) fIsOld = kFALSE;
  else if ( (! hasUnit) && hasStation) fIsOld = kTRUE;
  else if ( hasUnit && hasStation) LOG(FATAL) << GetName()
      << ": geometry contains both units and stations!" << FairLogger::endl;
  else LOG(FATAL) << GetName() << ": geometry contains neither units "
      << "nor stations!" << FairLogger::endl;
  if ( fIsOld ) LOG(WARNING) << GetName() << ": using old geometry (with stations)"
        << FairLogger::endl;

  // --- Recursively initialise daughter elements
  InitDaughters();

  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Read geometry from geometry file   ------------------------------
Bool_t CbmStsSetup::ReadGeometry(const char* fileName) {

  LOG(INFO) << fName << ": Reading geometry from file "
      << fileName << FairLogger::endl;

  // Exit if a TGeoManager is already present
  assert ( ! gGeoManager );

  // --- Open geometry file
  TFile geoFile(fileName);
  if ( ! geoFile.IsOpen() ) {
    LOG(FATAL) << GetName() << ": Could not open geometry file "
        << fileName << FairLogger::endl;
    return kFALSE;
  }

  // Create a new TGeoManager
  TGeoManager* stsGeometry = new TGeoManager("StsGeo",
                                             "STS stand-alone geometry");

  // --- Get top volume from file
  TGeoVolume* topVolume = NULL;
  TList* keyList = geoFile.GetListOfKeys();
  TKey* key = NULL;
  TIter keyIter(keyList);
  while ( (key = (TKey*) keyIter() ) ) {
    if ( strcmp( key->GetClassName(), "TGeoVolumeAssembly" ) == 0 ) {
      TGeoVolume* volume = (TGeoVolume*) key->ReadObj();
      if ( strcmp(volume->GetName(), "TOP") == 0 ) {
        topVolume = volume;
        break;
      }  //? volume name is "TOP"
    }    //? object class is TGeoVolumeAssembly
  }
  if ( ! topVolume) {
    LOG(FATAL) << GetName() << ": No TOP volume in file!" << FairLogger::endl;
    return kFALSE;
  }
  stsGeometry->SetTopVolume(topVolume);

  // --- Get cave (top node)
  stsGeometry->CdTop();
  TGeoNode* cave = stsGeometry->GetCurrentNode();

  // --- Get top STS node
  TGeoNode* sts = NULL;
  for (Int_t iNode = 0; iNode < cave->GetNdaughters(); iNode++) {
    TString name = cave->GetDaughter(iNode)->GetName();
    if ( name.Contains("STS", TString::kIgnoreCase) ) {
      sts = cave->GetDaughter(iNode);
      stsGeometry->CdDown(iNode);
      LOG(INFO) << fName << ": STS top node is " << sts->GetName()
                    << FairLogger::endl;
      break;
    }
  }
  if ( ! sts ) {
    LOG(ERROR) << fName << ": No top STS node found in geometry!"
        << FairLogger::endl;
    return kFALSE;
  }

  // --- Create physical node for sts
  TString path = cave->GetName();
  path = path + "/" + sts->GetName();
  fNode = new TGeoPhysicalNode(path);

  // --- Check for old geometry (with stations) or new geometry (with units)
  TString dName = fNode->GetNode()->GetDaughter(0)->GetName();
  LOG(DEBUG) << "First node is " << dName << FairLogger::endl;
  if ( dName.Contains("station", TString::kIgnoreCase) ) fIsOld = kTRUE;
  else if ( dName.Contains("unit", TString::kIgnoreCase) ) fIsOld = kFALSE;
  else LOG(FATAL) << GetName() << ": unknown geometry type; first level name is "
      << dName << FairLogger::endl;
  if ( fIsOld ) LOG(WARNING) << GetName() << ": using old geometry (with stations)"
      << FairLogger::endl;

  // --- Recursively initialise daughter elements
  InitDaughters();

  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Set sensor types   ----------------------------------------------
Int_t CbmStsSetup::ReadSensorParameters(const char* fileName) {

  // Input file
  std::fstream inFile;
  TString inputFile = fileName;

  // Try with argument as is (absolute path or current directory)
  inFile.open(inputFile.Data());

  // If not successful, look in the standard parameter directory
  if ( ! inFile.is_open() ) {
    inputFile = gSystem->Getenv("VMCWORKDIR");
    inputFile += "/parameters/sts/" + TString(fileName);
    inFile.open(inputFile.Data());
  }

  // If still not open, throw an error
  if ( ! inFile.is_open() ) {
    LOG(FATAL) << GetName() << ": Cannot read file " << fileName
        << " nor " << inputFile << FairLogger::endl;
    return -1;
  }

  string input;
  TString sName, sType;
  Int_t nSensors = 0;

  while ( kTRUE ) {  // read one line
    if ( inFile.eof() ) break;
    getline(inFile, input);
    if (input.empty() || input[0] == '#') continue;  // Comment line
    std::stringstream line(input);
    line >> sName >> sType;

    // Look for sensor in setup
    Int_t address = CbmStsSensor::GetAddressFromName(sName);

    // Bail out if sensor is already known
    if ( fSensors.find(address) != fSensors.end() ) {
      LOG(ERROR) << GetName() << ": sensor " << sName
          << " is already in the setup!" << FairLogger::endl;
      continue;
    }

    // Case sensor type DssdStereo
    if ( sType.EqualTo("DssdStereo", TString::kIgnoreCase) ) {
      Double_t dy      = 0.;  // active length in y
      Int_t    nStrips = 0;   // number of strips
      Double_t pitch   = 0.;  // strip pitch
      Double_t stereoF = 0.;  // stereo angle front side
      Double_t stereoB = 0.;  // stereo angle back side
      line >> dy >> nStrips >> pitch >> stereoF >> stereoB;
      CbmStsSensorDssdStereo* sensor
      = new CbmStsSensorDssdStereo(dy, nStrips, pitch, stereoF, stereoB);
      sensor->SetAddress(address);
      LOG(DEBUG) << "Created " << sensor->ToString() << FairLogger::endl;
      fSensors[address] = sensor;
      nSensors++;
    } //? sensor type DssdStereo

    else {
      LOG(FATAL) << GetName() << ": Unknown sensor type " << sType
          << " for sensor " << sName << FairLogger::endl;
    } //? Unknown sensor type

  } //# input lines

  inFile.close();
  LOG(INFO) << GetName() << ": Read " << nSensors
            << (nSensors == 1 ? " sensor" : " sensors") << " from "
            << inputFile << FairLogger::endl;
  assert( nSensors = fSensors.size() );

  return nSensors;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSetup)

