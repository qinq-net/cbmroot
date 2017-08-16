/** @file CbmStsSetup.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 27.05.2013
 **/

// Include class header
#include "CbmStsSetup.h"

// Includes from C++
#include <cassert>
#include <iostream>

// Includes from ROOT
#include "TFile.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TKey.h"

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
			     fIsOld(kFALSE), fModules(),
			     fSensors(), fStations() {
}
// -------------------------------------------------------------------------



// -----   Assign a sensor object to an address   --------------------------
CbmStsSensor* CbmStsSetup::AssignSensor(UInt_t address,
                                        TGeoPhysicalNode* node) {

  CbmStsSensor* sensor = nullptr;
  auto it = fSensors.find(address);
  if ( it != fSensors.end() ) {
    sensor = it->second; // Found in sensor list
    assert (sensor->GetAddress() == address);
    sensor->SetNode(node);
  }
  else sensor = DefaultSensor(address, node); // Not found; create and add.

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
CbmStsSensor* CbmStsSetup::DefaultSensor(UInt_t address,
                                         TGeoPhysicalNode* node) {

  // There should not already be a sensor object for this address
  assert ( fSensors.find(address) == fSensors.end() );

  // Sensor volume extension in x and y
  assert(node);
  TGeoBBox* shape = dynamic_cast<TGeoBBox*>(node->GetShape());
  assert(shape);
  Double_t volX = 2. * shape->GetDX();
  Double_t volY = 2. * shape->GetDY();

  // Default sensor type is DssdStereo
  CbmStsSensorDssdStereo* sensor = new CbmStsSensorDssdStereo(address, node);

  // Pitch and stereo angles
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

  // Set sensor parameters
  sensor->SetParameters(dy, nStrips, pitch, stereoF, stereoB);

  // Add sensor to sensor list
  fSensors[address] = sensor;

  return sensor;
}
// -------------------------------------------------------------------------



// -----   Define sensor types   -------------------------------------------
Int_t CbmStsSetup::DefineSensorTypes() {
  /*

	// Common parameters for all sensor types
    Double_t lz      = 0.03; // active thickness [cm]
	Double_t stereoF = 0.;   // stereo angle front side [degrees]
	Double_t stereoB = 7.5;  // stereo angle back side [degrees]

	// Parameters varying from type to type
	Double_t lx      = 0.;   // active size in x
	Double_t ly      = 0.;   // active size in y
	Int_t nStripsF   = 0;    // number of strips front side (58 mum)
	Int_t nStripsB   = 0;    // number of strips back side  (58 mum)

	// Sensor type CBM04H2: DSSD, 4 cm x 2.2 cm
	CbmStsSensorTypeDssd* sensor01 = new CbmStsSensorTypeDssd();
	lx       = 3.7584;    // active size in x
	ly       = 1.96;      // active size in y
	nStripsF = 648;       // number of strips front side (58 mum)
	nStripsB = 648;       // number of strips back side  (58 mum)
	sensor01->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor01->SetName("CBM04H2");
    sensor01->SetTitle("Dssd");
	fSensorTypes["CBM04H2"] = sensor01;


	// Sensor type CBM06H2: DSSD, 6.2 cm x 2.2 cm
	CbmStsSensorTypeDssd* sensor02 = new CbmStsSensorTypeDssd();
	lx       = 5.9392;    // active size in x
	ly       = 1.96;      // active size in y
	nStripsF = 1024;      // number of strips front side (58 mum)
	nStripsB = 1024;      // number of strips back side  (58 mum)
	sensor02->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor02->SetName("CBM06H2");
    sensor02->SetTitle("Dssd");
	fSensorTypes["CBM06H2"] = sensor02;

	// Sensor type CBM06H4: DSSD, 6.2 cm x 4.2 cm
	CbmStsSensorTypeDssd* sensor03 = new CbmStsSensorTypeDssd();
	lx       = 5.9392;    // active size in x
	ly       = 3.96;      // active size in y
	nStripsF = 1024;      // number of strips front side (58 mum)
	nStripsB = 1024;      // number of strips back side  (58 mum)
	sensor03->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor03->SetName("CBM06H4");
    sensor03->SetTitle("Dssd");
	fSensorTypes["CBM06H4"] = sensor03;

	// Sensor type CBM06H6: DSSD, 6.2 cm x 6.2 cm
	CbmStsSensorTypeDssd* sensor04 = new CbmStsSensorTypeDssd();
	lx       = 5.9392;    // active size in x
	ly       = 5.96;      // active size in y
	nStripsF = 1024;      // number of strips front side (58 mum)
	nStripsB = 1024;      // number of strips back side  (58 mum)
	sensor04->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor04->SetName("CBM06H6");
    sensor04->SetTitle("Dssd");
	fSensorTypes["CBM06H6"] = sensor04;

	// Sensor tyope CBM03H3: DSSD, 3.1 cm x 3.1 cm
	CbmStsSensorTypeDssd* sensor05 = new CbmStsSensorTypeDssd();
	lx       = 2.8594;    // active size in x
	ly       = 2.86;      // active size in y
	nStripsF = 493;       // number of strips front side (58 mum)
	nStripsB = 493;       // number of strips back side  (58 mum)
	sensor05->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor05->SetName("CBM03H3");
    sensor05->SetTitle("Dssd");
	fSensorTypes["CBM03H3"] = sensor05;

	// Sensor type CBM01H4: DSSD, 1.5 cm x 4.2 cm
	CbmStsSensorTypeDssd* sensor06 = new CbmStsSensorTypeDssd();
	lx       = 1.2586;    // active size in x
	ly       = 3.96;      // active size in y
	nStripsF = 217;       // number of strips front side (58 mum)
	nStripsB = 217;       // number of strips back side  (58 mum)
	sensor06->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor06->SetName("CBM01H4");
    sensor06->SetTitle("Dssd");
	fSensorTypes["CBM01H4"] = sensor06;

	// Sensor type CBM03H4: DSSD, 3.1 cm x 4.2 cm
	CbmStsSensorTypeDssd* sensor07 = new CbmStsSensorTypeDssd();
	lx       = 2.8594;    // active size in x
	ly       = 3.96;      // active size in y
	nStripsF = 493;       // number of strips front side (58 mum)
	nStripsB = 493;       // number of strips back side  (58 mum)
	sensor07->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor07->SetName("CBM03H4");
    sensor07->SetTitle("Dssd");
	fSensorTypes["CBM03H4"] = sensor07;

	// Sensor type Baby: DSSD Ortho, 1.62 cm x 1.62 cm
	CbmStsSensorTypeDssdOrtho* sensor08 = new CbmStsSensorTypeDssdOrtho();
	lx       = 1.28;      // active size in x
	ly       = 1.28;      // active size in y
	nStripsF = 256;       // number of strips front side (58 mum)
	nStripsB = 256;       // number of strips back side  (58 mum)
	sensor08->SetParameters(lx, ly, lz, nStripsF, nStripsB, 0., 0.);
	sensor08->SetName("Baby");
    sensor08->SetTitle("DssdOrtho");
	fSensorTypes["Baby"] = sensor08;

	// Sensor type CBM06H12: DSSD, 6.2 cm x 12.4 cm
	CbmStsSensorTypeDssd* sensor09 = new CbmStsSensorTypeDssd();
	lx       = 5.9392;    // active size in x
	ly       = 12.16;     // active size in y
	nStripsF = 1024;      // number of strips front side (58 mum)
	nStripsB = 1024;      // number of strips back side  (58 mum)
	sensor09->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor09->SetName("CBM06H12");
    sensor09->SetTitle("Dssd");
	fSensorTypes["CBM06H12"] = sensor09;

	return fSensorTypes.size();
	*/
  return 0;
}
// -------------------------------------------------------------------------



// -----   Get an element from the STS setup   -----------------------------
CbmStsElement* CbmStsSetup::GetElement(UInt_t address, Int_t level) {

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
	for (Int_t iLevel = 1; iLevel <= level; iLevel++)
		element =
				element->GetDaughter(CbmStsAddress::GetElementId(address, iLevel));

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
    case kStsChannel: return "channel"; break;
    default: return ""; break;
  }

}
// -------------------------------------------------------------------------



// -----   Get the station number from an address   ------------------------
Int_t CbmStsSetup::GetStationNumber(UInt_t address) {

  // In old, station-based geometries, the station equals the unit
  if ( fIsOld ) return CbmStsAddress::GetElementId(address, kStsUnit);

  // In new, unit-based geometries, the station is obtained from the ladder
  CbmStsElement* ladder = CbmStsSetup::GetElement(address, kStsLadder);
  assert(ladder);
  return ladder->GetPnode()->GetNode()->GetNumber() / 100 - 1;

}
// -------------------------------------------------------------------------



// -----   Initialisation from TGeoManager   -------------------------------
Bool_t CbmStsSetup::Init(TGeoManager* geo) {

	if ( fIsInitialised ) {
		LOG(FATAL) << GetName() << ": Setup is already initialised!"
				       << FairLogger::endl;
		return kFALSE;
	}

  cout << endl;
  LOG(INFO) << "=========================================================="
		        << FairLogger::endl;
  LOG(INFO) << "Initialising STS Setup from TGeoManager"
  		      << FairLogger::endl;

  // --- Catch non-existence of GeoManager
  assert (geo);

  // Get STS physics singleton instance
  CbmStsPhysics::Instance();

  // --- Get cave (top node)
  LOG(INFO) << fName << ": Reading geometry from TGeoManager "
            << geo->GetName() << FairLogger::endl;
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

  // --- Set system address
  fAddress = kSts;

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

  // --- Build arrays of modules
  for (Int_t iUnit = 0; iUnit < GetNofDaughters(); iUnit++) {
  	CbmStsElement* unit = GetDaughter(iUnit);
  	for (Int_t iLad = 0; iLad < unit->GetNofDaughters(); iLad++) {
  		CbmStsElement* ladd = unit->GetDaughter(iLad);
  		for (Int_t iHla = 0; iHla < ladd->GetNofDaughters(); iHla++) {
  			CbmStsElement* hlad = ladd->GetDaughter(iHla);
  			for (Int_t iMod = 0; iMod < hlad->GetNofDaughters(); iMod++) {
  				CbmStsElement* modu = hlad->GetDaughter(iMod);
  				fModules.push_back(dynamic_cast<CbmStsModule*>(modu));
  			}
  		}
  	}
  }

  // --- Statistics
  LOG(INFO) << fName << ": Elements in setup: " << FairLogger::endl;
  for (Int_t iLevel = 1; iLevel <= kStsSensor; iLevel++) {
	  TString name = GetLevelName(iLevel);
	  name += "s";
	  LOG(INFO) << "     " << setw(12) << name << setw(5) << right
	  		      << GetNofElements(iLevel) << FairLogger::endl;
  }

  // Set the sensor types
  Int_t nSensors = SetSensorTypes();
	LOG(INFO) << GetName() << ": Set types for " << nSensors
			      << " sensors" << FairLogger::endl;

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


  LOG(INFO) << "=========================================================="
		    << FairLogger::endl;
  cout << endl;

  fIsInitialised = kTRUE;
  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Initialisation from geometry file   -----------------------------
Bool_t CbmStsSetup::Init(const char* fileName) {

	if ( fIsInitialised ) {
		LOG(FATAL) << GetName() << ": Setup is already initialised!"
				       << FairLogger::endl;
		return kFALSE;
	}

  cout << endl;
  LOG(INFO) << "=========================================================="
		        << FairLogger::endl;
  LOG(INFO) << "Initialising STS Setup from file " << fileName
  		      << FairLogger::endl;

  // Check for existence of a TGeoManager. If present, exit function.
  if ( gGeoManager )
  	LOG(FATAL) << GetName() << ": found existing TGeoManager!"
  	<< FairLogger::endl;

	TGeoManager* newGeometry = new TGeoManager();

	// --- Open geometry file
	TFile geoFile(fileName);
	if ( ! geoFile.IsOpen() ) {
		LOG(ERROR) << GetName() << ": Could not open geometry file "
				       << fileName << ", setup is not initialised."
				       << FairLogger::endl;
		return kFALSE;
	}

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
		LOG(ERROR) << GetName() << ": No TOP volume in file!" << FairLogger::endl;
		return kFALSE;
	}
  newGeometry->SetTopVolume(topVolume);

  // --- Get cave (top node)
  newGeometry->CdTop();
  TGeoNode* cave = newGeometry->GetCurrentNode();

  // --- Get top STS node
  TGeoNode* sts = NULL;
  for (Int_t iNode = 0; iNode < cave->GetNdaughters(); iNode++) {
    TString name = cave->GetDaughter(iNode)->GetName();
     if ( name.Contains("STS", TString::kIgnoreCase) ) {
      sts = cave->GetDaughter(iNode);
      newGeometry->CdDown(iNode);
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

  // --- Set system address
  fAddress = kSts;

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

  // --- Build arrays of modules and sensors
  for (Int_t iUnit = 0; iUnit < GetNofDaughters(); iUnit++) {
  	CbmStsElement* unit = GetDaughter(iUnit);
  	for (Int_t iLad = 0; iLad < unit->GetNofDaughters(); iLad++) {
  		CbmStsElement* ladd = unit->GetDaughter(iLad);
  		for (Int_t iHla = 0; iHla < ladd->GetNofDaughters(); iHla++) {
  			CbmStsElement* hlad = ladd->GetDaughter(iHla);
  			for (Int_t iMod = 0; iMod < hlad->GetNofDaughters(); iMod++) {
  				CbmStsElement* modu = hlad->GetDaughter(iMod);
  				fModules.push_back(dynamic_cast<CbmStsModule*>(modu));
  			} //# modules
  		} //# halfladders
  	} //# ladders
  } //# units

  // --- Statistics
  LOG(INFO) << fName << ": Elements in setup: " << FairLogger::endl;
  for (Int_t iLevel = 1; iLevel <= kStsSensor; iLevel++) {
	  TString name = GetLevelName(iLevel);
	  name += "s";
	  LOG(INFO) << "     " << setw(12) << name << setw(5) << right
	  		      << GetNofElements(iLevel) << FairLogger::endl;
  }

  // Set the sensor types
  Int_t nSensors = SetSensorTypes();
	LOG(INFO) << GetName() << ": Set types for " << nSensors
			      << " sensors" << FairLogger::endl;

  // --- Initialise stations parameters
  for (Int_t iStat = 0; iStat < GetNofDaughters(); iStat++) {
  	CbmStsStation* station =
  			dynamic_cast<CbmStsStation*>(GetDaughter(iStat));
  	station->Init();
  }

  // --- Consistency check
  if ( GetNofSensors() != GetNofElements(kStsSensor) )
  	LOG(FATAL) << GetName() << ": inconsistent number of sensors! "
  			       << GetNofElements(kStsSensor) << " " << GetNofSensors()
  			       << FairLogger::endl;

  LOG(INFO) << "=========================================================="
		    << FairLogger::endl;
  cout << endl;

  fIsInitialised = kTRUE;
	return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Singleton instance   --------------------------------------------
CbmStsSetup* CbmStsSetup::Instance() {
  if ( ! fgInstance ) {
    fgInstance = new CbmStsSetup();
    if ( gGeoManager) fgInstance->Init(gGeoManager);
  }
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
  LOG(INFO) << GetName() << ": Set conditions for all sensors";
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



// -----   Set sensor types   ----------------------------------------------
// TODO: The current implementation is a workaround only. The sensor type
// is determined from the geometric size of the sensor. Nasty, isn't it?
Int_t CbmStsSetup::SetSensorTypes(TString fileName) {

    // Build sensor type database
	Int_t nTypes = DefineSensorTypes();
	LOG(INFO) << GetName() << ": " << nTypes << " sensor types in database"
			      << FairLogger::endl;


	// Read from file if specified
	if ( ! fileName.IsNull() ) {

	  std::fstream sensorFile(fileName.Data());
	  assert(sensorFile.is_open());
	  string input;
	  TString sName, sType;
	  vector<Double_t> params;

	  while ( kTRUE ) {  // read one line
	    if ( sensorFile.eof() ) break;
	    getline(sensorFile, input);
	    if ( input[0] == '#') continue;  //
	    std::stringstream line(input);
	    line >> sName >> sType;

	    // Look for sensor in setup


	    if ( sType.EqualTo("DSSD", TString::kIgnoreCase) ) {
	     Int_t nParams = 5;
	     Double_t par[10];
	     for (Int_t iParam = 0; iParam < nParams; iParam++) {
	       line >> par[iParam];
	     }
	     for (Int_t i=0; i<10; i++) std::cout << par[i] << " " << std::endl;
	   }

	    //}
	    LOG(INFO) << GetName() << " " << input << FairLogger::endl;
	    //std::cout << GetName() << " " << line << std::endl;
	    LOG(INFO) << GetName() << " Sensor name is  " << sName << FairLogger::endl;
	  }
	  sensorFile.close();


	}


	return 0;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSetup)

