/** @file CbmStsSetup.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 27.05.2013
 **/


#ifndef CBMSTSSETUP_H
#define CBMSTSSETUP_H 1

#include <map>
#include "CbmStsElement.h"

class TGeoManager;
class CbmStsDigitize;
class CbmStsModule;
class CbmStsSensor;
class CbmStsSensorType;
class CbmStsStation;


/** @class CbmStsSetup
 ** @brief Class representing the top level of the STS setup
 ** @author V.Friese <v.friese@gsi.de>
 ** @version 1.0
 **
 ** The CbmStsSetup is a singleton class and represents the
 ** interface to the CBM geometry and its elements.
 **/
class CbmStsSetup : public CbmStsElement
{

  public:

    /** Destructor **/
    virtual ~CbmStsSetup() { };


    /** Define available sensor types
     ** Poor man's sensor database, with hardcoded values
     ** @value Number of available sensor types
     **/
    Int_t DefineSensorTypes();


    /** Get digitiser task **/
    CbmStsDigitize* GetDigitizer() const  { return fDigitizer; }


    /** Get an STS element by address
     ** @param address  Unique element address (see CbmStsAddress)
     ** @param level    Element level (EStsElementLevel)
     ** @return Pointer to STS element
     **/
    CbmStsElement* GetElement(UInt_t address, Int_t level);


    /** Get the name of an element level
     ** @param level    Element level (EStsElementLevel)
     ** @return  Name of element level (station, ladder, etc.)
     **/
    const char* GetLevelName(Int_t level);


    /** Get number of modules in setup **/
    Int_t GetNofModules() const { return fModules.size(); }


    /** Get number of sensors in setup **/
    Int_t GetNofSensors() const { return fSensors.size(); }


    /** Get number of available sensor types **/
    Int_t GetNofSensorTypes() const { return fSensorTypes.size(); }


    /** Get number of stations **/
    Int_t GetNofStations() const { return fStations.size(); }


    /** Get a module from the module array.
     ** For convenient loops over all modules.
     ** Note that the index of the module is meaningless.
     ** @param  index  Index of module in the array
     ** @return  Pointer to module
     **/
    CbmStsModule* GetModule(Int_t index) { return fModules[index]; }


    /** Get a sensor from the array.
     ** For convenient loops over all sensors.
     ** Note that the index of the sensor is meaningless.
     ** @param  index  Index of sensor in the array
     ** @return  Pointer to sensor
     **/
    CbmStsSensor* GetSensor(Int_t index) { return fSensors[index]; }


    /** Get a sensor type from the list of available ones
     ** @param index  Index of sensor type in the array.
     ** @value type   Pointer to sensor type. NULL if index is out of bounds.
     **/
    CbmStsSensorType* GetSensorType(Int_t index) {
    	if ( fSensorTypes.find(index) != fSensorTypes.end() )
    		return fSensorTypes[index];
    	return NULL;
    }


    /** Get a station
     ** @param stationId  Station number
     ** @value Pointer to station object. NULL is not present.
     **/
    CbmStsStation* GetStation(Int_t stationId) {
      if ( fStations.find(stationId) == fStations.end() ) return NULL;
      return fStations[stationId];
    }


    /** Get station number from address
     ** @param address  Unique detector address
     ** @value Station number
     **/
    Int_t GetStationNumber(UInt_t address);


    /** Initialise setup from geometry
     ** @param geo  Instance of TGeoManager
     ** @return kTRUE if successfully initialised; kFALSE else
     **/
    Bool_t Init(TGeoManager* geo);


    /** Initialise setup from a STS geometry file
     ** @param fileName  Name of geometry file
     ** @return kTRUE if successfully initialised; kFALSE else
     **/
    Bool_t Init(const char* fileName);


    /** Initialise daughters from geometry
     ** Overrides the mother class implementation in case of new geometries (with units) **/
    virtual void InitDaughters();


    /** Static instance of CbmStsSetup **/
    static CbmStsSetup* Instance();


    /** Set the digitiser task  **/
    void SetDigitizer(CbmStsDigitize* digitizer) {
    	fDigitizer = digitizer; }


    /** Set sensor parameters
     ** Set the sensor parameters that are not contained in the geometry,
     ** but required for digitisation and reconstruction, like strip pitch,
     ** stereo angle etc.
     ** @value Number of sensors the type of which was set.
     **/
    Int_t SetSensorTypes();



  private:

    static CbmStsSetup* fgInstance;    ///< Static instance of this class
    CbmStsDigitize* fDigitizer;   ///< Pointer to digitiser task

    Bool_t fIsInitialised;  ///< To protect against multiple initialisation.
    Bool_t fIsOld;          ///< Old setup with stations as top level

    /** These arrays allow convenient loops over all modules or sensors. **/
    std::vector<CbmStsModule*> fModules;   //! Array of modules
    std::vector<CbmStsSensor*> fSensors;   //! Array of sensors

    /** Stations (special case; are not elements in the setup) **/
    std::map<Int_t, CbmStsStation*> fStations;  //!

    /** Available sensor types **/
    std::map<Int_t, CbmStsSensorType*> fSensorTypes; //!

    /** Default constructor  **/
    CbmStsSetup();

    /** Create station objects **/
    Int_t CreateStations();

    /** Copy constructor (disabled) **/
    CbmStsSetup(const CbmStsSetup&) = delete;

    /** Assignment operator (disabled) **/
    CbmStsSetup operator=(const CbmStsSetup&) = delete;

    ClassDef(CbmStsSetup,1);

};

#endif /* CBMSTSSETUP_H */
