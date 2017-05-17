/** @file CbmStsSetup.cxx
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 27.05.2013
 **/

// Include class header
#include "setup/CbmStsSetup.h"

// Includes from C++
#include <cassert>
#include <iostream>

// Includes from ROOT
#include "TFile.h"
#include "TGeoBBox.h"
#include "TGeoManager.h"
#include "TKey.h"

// Includes from CbmRoot
#include "CbmDetectorList.h"

// Includes from STS
#include "CbmStsAddress.h"
#include "CbmStsMC.h"
#include "CbmStsModule.h"
#include "CbmStsSensor.h"
#include "CbmStsSensorTypeDssd.h"
#include "CbmStsSensorTypeDssdOrtho.h"
#include "CbmStsStation.h"

using std::setw;
using std::right;
using std::cout;
using std::endl;
using std::map;

// -----   Initialisation of static singleton pointer   --------------------
CbmStsSetup* CbmStsSetup::fgInstance = NULL;
// -------------------------------------------------------------------------



// -----   Constructor   ---------------------------------------------------
CbmStsSetup::CbmStsSetup() : CbmStsElement("STS", "system", kStsSystem),
			     fDigitizer(NULL), fIsInitialised(kFALSE),
			     fIsOld(kFALSE), fModules(),
			     fSensors(), fSensorTypes(), fStations() {
}
// -------------------------------------------------------------------------



// -----   Create station objects   ----------------------------------------
Int_t CbmStsSetup::CreateStations() {

  // For old geometries: the station equals the unit
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
        station->AddDaughter(unit->GetDaughter(iLadder));
      // Initialise station paraneters
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
      Int_t stationId = ladder->GetIndex() / 100;
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
      station->AddDaughter(ladder);

    } //# ladders
  } //# units

  // Initialise the stations
  auto it = fStations.begin();
  while ( it != fStations.end() ) {
    it->second->Init();
    it++;
  } //# stations

  return fStations.size();
}
// -------------------------------------------------------------------------



// -----   Define sensor types   -------------------------------------------
Int_t CbmStsSetup::DefineSensorTypes() {

	// Common parameters for all sensor types
    Double_t lz      = 0.03; // active thickness [cm]
	Double_t stereoF = 0.;   // stereo angle front side [degrees]
	Double_t stereoB = 7.5;  // stereo angle back side [degrees]

	// Parameters varying from type to type
	Double_t lx      = 0.;   // active size in x
	Double_t ly      = 0.;   // active size in y
	Int_t nStripsF   = 0;    // number of strips front side (58 mum)
	Int_t nStripsB   = 0;    // number of strips back side  (58 mum)

	// Sensor01: DSSD, 4 cm x 2.2 cm
	CbmStsSensorTypeDssd* sensor01 = new CbmStsSensorTypeDssd();
	lx       = 3.7584;    // active size in x
	ly       = 1.96;      // active size in y
	nStripsF = 648;       // number of strips front side (58 mum)
	nStripsB = 648;       // number of strips back side  (58 mum)
	sensor01->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor01->SetName("Sensor01");
	fSensorTypes[0] = sensor01;

	// Sensor02: DSSD, 6.2 cm x 2.2 cm
	CbmStsSensorTypeDssd* sensor02 = new CbmStsSensorTypeDssd();
	lx       = 5.9392;    // active size in x
	ly       = 1.96;      // active size in y
	nStripsF = 1024;      // number of strips front side (58 mum)
	nStripsB = 1024;      // number of strips back side  (58 mum)
	sensor02->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor02->SetName("Sensor02");
	fSensorTypes[1] = sensor02;

	// Sensor03: DSSD, 6.2 cm x 4.2 cm
	CbmStsSensorTypeDssd* sensor03 = new CbmStsSensorTypeDssd();
	lx       = 5.9392;    // active size in x
	ly       = 3.96;      // active size in y
	nStripsF = 1024;      // number of strips front side (58 mum)
	nStripsB = 1024;      // number of strips back side  (58 mum)
	sensor03->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor03->SetName("Sensor03");
	fSensorTypes[2] = sensor03;

	// Sensor04: DSSD, 6.2 cm x 6.2 cm
	CbmStsSensorTypeDssd* sensor04 = new CbmStsSensorTypeDssd();
	lx       = 5.9392;    // active size in x
	ly       = 5.96;      // active size in y
	nStripsF = 1024;      // number of strips front side (58 mum)
	nStripsB = 1024;      // number of strips back side  (58 mum)
	sensor04->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor04->SetName("Sensor04");
	fSensorTypes[3] = sensor04;

	// Sensor05: DSSD, 3.1 cm x 3.1 cm
	CbmStsSensorTypeDssd* sensor05 = new CbmStsSensorTypeDssd();
	lx       = 2.8594;    // active size in x
	ly       = 2.86;      // active size in y
	nStripsF = 493;       // number of strips front side (58 mum)
	nStripsB = 493;       // number of strips back side  (58 mum)
	sensor05->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor05->SetName("Sensor05");
	fSensorTypes[4] = sensor05;

	// Sensor06: DSSD, 1.5 cm x 4.2 cm
	CbmStsSensorTypeDssd* sensor06 = new CbmStsSensorTypeDssd();
	lx       = 1.2586;    // active size in x
	ly       = 3.96;      // active size in y
	nStripsF = 217;       // number of strips front side (58 mum)
	nStripsB = 217;       // number of strips back side  (58 mum)
	sensor06->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor06->SetName("Sensor06");
	fSensorTypes[5] = sensor06;

	// Sensor07: DSSD, 3.1 cm x 4.2 cm
	CbmStsSensorTypeDssd* sensor07 = new CbmStsSensorTypeDssd();
	lx       = 2.8594;    // active size in x
	ly       = 3.96;      // active size in y
	nStripsF = 493;       // number of strips front side (58 mum)
	nStripsB = 493;       // number of strips back side  (58 mum)
	sensor07->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor07->SetName("Sensor07");
	fSensorTypes[6] = sensor07;

	// Sensor08: DSSD Ortho (Baby), 1.62 cm x 1.62 cm
	CbmStsSensorTypeDssdOrtho* sensor08 = new CbmStsSensorTypeDssdOrtho();
	lx       = 1.28;      // active size in x
	ly       = 1.28;      // active size in y
	nStripsF = 256;       // number of strips front side (58 mum)
	nStripsB = 256;       // number of strips back side  (58 mum)
	sensor08->SetParameters(lx, ly, lz, nStripsF, nStripsB, 0., 0.);
	sensor08->SetName("Baby");
	fSensorTypes[7] = sensor08;

	// Sensor09: DSSD, 6.2 cm x 12.4 cm
	CbmStsSensorTypeDssd* sensor09 = new CbmStsSensorTypeDssd();
	lx       = 5.9392;    // active size in x
	ly       = 12.16;     // active size in y
	nStripsF = 1024;      // number of strips front side (58 mum)
	nStripsB = 1024;      // number of strips back side  (58 mum)
	sensor09->SetParameters(lx, ly, lz, nStripsF, nStripsB, stereoF, stereoB);
	sensor09->SetName("Sensor09");
	fSensorTypes[8] = sensor09;

	return fSensorTypes.size();
}
// -------------------------------------------------------------------------



// -----   Get an element from the STS setup   -----------------------------
CbmStsElement* CbmStsSetup::GetElement(UInt_t address, Int_t level) {

	// --- Check for initialisation
	if ( ! fAddress ) LOG(FATAL) << fName << ": not initialised!"
			                         << FairLogger::endl;

	// --- Catch non-STS addresses
	if ( CbmStsAddress::GetSystemId(address) != kSTS ) {
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
  else LOG(FATAL) << GetName() << ": implementation for ststaion number "
      << " in unit-based geometry is still missing!" << FairLogger::endl;
  return 0;
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
  if ( ! geo ) {
    LOG(FATAL) << fName << ": no TGeoManager!" << FairLogger::endl;
    return kFALSE;
  }

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
      geo->CdDown(iNode);
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
  fAddress = kSTS;

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
  if ( fIsOld ) CbmStsElement::InitDaughters(); // use method from CbmStsElement
  else InitDaughters();

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
  				for (Int_t iSen = 0; iSen < modu->GetNofDaughters(); iSen++) {
  					CbmStsSensor* sensor =
  							dynamic_cast<CbmStsSensor*>(modu->GetDaughter(iSen));
  					fSensors.push_back(sensor);
  				}
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

  // --- Temporary protection against new geometries
  if ( ! fIsOld ) LOG(FATAL) << GetName() << ": You are using a new "
      << "STS geometry with units as top level. The software to support "
      << "this is not yet ready. Please use an older geometry meanwhile."
      << FairLogger::endl;


  // Set the sensor types
  Int_t nSensors = SetSensorTypes();
	LOG(INFO) << GetName() << ": Set types for " << nSensors
			      << " sensors" << FairLogger::endl;

  // --- Create station objects
  Int_t nStations = CreateStations();
  LOG(INFO) << GetName() << ": Setup contains " << nStations
      << " stations." << FairLogger::endl;
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
  fAddress = kSTS;

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
  if ( fIsOld ) CbmStsElement::InitDaughters(); // use method from CbmStsElement
  else InitDaughters();

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
  				for (Int_t iSen = 0; iSen < modu->GetNofDaughters(); iSen++) {
  					CbmStsSensor* sensor =
  							dynamic_cast<CbmStsSensor*>(modu->GetDaughter(iSen));
  					fSensors.push_back(sensor);
  				} //# sensors
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



// -----   InitDaughters   -------------------------------------------------
void CbmStsSetup::InitDaughters() {

	// --- Catch absence of TGeoManager
	if ( ! gGeoManager ) {
		LOG(ERROR) << fName << ": cannot initialise without TGeoManager!"
				<< FairLogger::endl;
		return;
	}

	// --- Catch physical node not being set
	if ( ! fNode ) {
		LOG(ERROR) << fName << ": physical node is not set!"
				<< FairLogger::endl;
		return;
	}

	TGeoNode* mNode = fNode->GetNode();   // This node
	TString   mPath = fNode->GetName();   // Full path to this node
	//Int_t nDaughters = 0;
	for (Int_t iNode = 0; iNode < mNode->GetNdaughters(); iNode++) {
		TGeoNode* unitNode = mNode->GetDaughter(iNode);
		assert(unitNode);

		// Check name of daughter node (should be a "unit") for level name
		TString uName = unitNode->GetName();
		if ( ! uName.Contains("unit", TString::kIgnoreCase ) ) continue;
		TString uPath = mPath + "/" + uName;

		// Loop over unit daughters (should be ladders)
		for (Int_t iLadder = 0; iLadder < unitNode->GetNdaughters(); iLadder++) {
			TGeoNode* ladderNode = unitNode->GetDaughter(iLadder);
			assert(ladderNode);

			// Check name of ladder node
			TString lName = ladderNode->GetName();
			if ( ! lName.Contains("ladder", TString::kIgnoreCase ) ) continue;

			// Get station number
			Int_t iStation = ladderNode->GetNumber() / 100;
			Int_t ladderNumber = ladderNode->GetNumber() - 100 * iStation;

			// Get station, if it already exists. Otherwise, create new one
			CbmStsStation* station = dynamic_cast<CbmStsStation*>(GetDaughter(iStation-1));
			if ( ! station ) {
				TString name = Form("Station%02i", iStation);
				station = new CbmStsStation(name, "", NULL);
				AddDaughter(station);
			}

			// Create a physical node for the ladder
			TString lPath = uPath + "/" + lName;
			TGeoPhysicalNode* pNode = new TGeoPhysicalNode(lPath.Data());

			// Create the ladder element
			TString name = Form("Ladder%02i", ladderNumber);
			const char* title = ladderNode->GetVolume()->GetName();
			CbmStsElement* ladder = new CbmStsElement(name, title, kStsLadder, pNode);

			// Add the ladder to its station
			station->AddDaughter(ladder);

			// Initialise the ladder daughters recursively
			ladder->InitDaughters();

		} // #ladder nodes
	} // #unit nodes

}
// -------------------------------------------------------------------------



// -----   Instance   ------------------------------------------------------
CbmStsSetup* CbmStsSetup::Instance() {
  if ( ! fgInstance ) {
    fgInstance = new CbmStsSetup();
    if ( gGeoManager) fgInstance->Init(gGeoManager);
  }
  return fgInstance;
}
// -------------------------------------------------------------------------



// -----   Set sensor types   ----------------------------------------------
// TODO: The current implementation is a workaround only. The sensor type
// is determined from the geometric size of the sensor. Nasty, isn't it?
Int_t CbmStsSetup::SetSensorTypes() {

	Int_t nTypes = DefineSensorTypes();
	LOG(INFO) << GetName() << ": " << nTypes << " sensor types in database"
			      << FairLogger::endl;

	Int_t nSensors = 0; // counter

	for (Int_t iSensor = 0; iSensor < GetNofSensors(); iSensor++) {
		CbmStsSensor* sensor = GetSensor(iSensor);

		// Sensor extension in x and y
		TGeoBBox* shape =
				dynamic_cast<TGeoBBox*>(sensor->GetPnode()->GetShape());
		Double_t volX = 2. * shape->GetDX();
		Double_t volY = 2. * shape->GetDY();

		// Assign type
		Int_t iType = -1;
		if      ( volX > 3.99 && volX < 4.01 )                                 iType = 0;
		else if ( volX > 6.19 && volX < 6.21 && volY >  2.19 && volY <  2.21 ) iType = 1;
		else if ( volX > 6.19 && volX < 6.21 && volY >  4.19 && volY <  4.21 ) iType = 2;
		else if ( volX > 6.19 && volX < 6.21 && volY >  6.19 && volY <  6.21 ) iType = 3;
		else if ( volX > 3.09 && volX < 3.11 && volY >  3.09 && volY <  3.11 ) iType = 4;
		else if ( volX > 1.49 && volX < 1.51 && volY >  4.19 && volY <  4.21 ) iType = 5;
		else if ( volX > 3.09 && volX < 3.11 && volY >  4.19 && volY <  4.21 ) iType = 6;
		else if ( volX > 1.61 && volX < 1.63 && volY >  1.61 && volY <  1.63 ) iType = 7;
		else if ( volX > 6.19 && volX < 6.21 && volY > 12.39 && volY < 12.41 ) iType = 8;

		// Look for type in DB
		map<Int_t, CbmStsSensorType*>::iterator it = fSensorTypes.find(iType);
		if ( it == fSensorTypes.end() )
			LOG(FATAL) << GetName() << ": sensor type for sensor with x = " << volX
					       << " cm and y = " << volY << " cm is not defined!"
					       << FairLogger::endl;

		// Set sensor type to sensor
		sensor->SetType((*it).second);
		nSensors++;

	} //# sensors

	return nSensors;
}
// -------------------------------------------------------------------------


ClassImp(CbmStsSetup)

