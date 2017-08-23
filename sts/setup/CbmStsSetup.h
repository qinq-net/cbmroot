/** @file CbmStsSetup.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 27.05.2013
 **/


#ifndef CBMSTSSETUP_H
#define CBMSTSSETUP_H 1

#include <map>
#include "CbmStsElement.h"
#include "CbmStsSensor.h"

class TGeoManager;
class CbmStsDigitize;
class CbmStsDigitizeParameters;
class CbmStsModule;
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


    /** Assign a sensor to an address
     ** @param  address  Unique element address
     ** @param  node     Physical node of sensor
     ** @return  Pointer to sensor object
     **
     ** The sensor object is taken from the sensor list and gets the
     ** geometric node assigned. If it is not present in the list,
     ** the default sensor is instantiated (see DefaultSensor()).
     **/
    CbmStsSensor* AssignSensor(Int_t address,
                               TGeoPhysicalNode* node = nullptr);


    /** @brief Instantiate the default sensor class for a given node
     ** @param address  Unique element address
     ** @param node Sensor node in the ROOT geometry
     ** @value Pointer to CbmStsSensor object
     **
     ** Instantiates a default sensor object and adds it to the sensor list.
     ** The default sensor class and parameters are hard-coded here.
     ** The default is CbmStsSensorDssdStereo, with 58 mum pitch and
     ** stereo angles 0 degrees for front side and 7.5 degrees for back side.
     ** The active size in y is 0.24 cm smaller than the geometric
     ** extension. The number of strips is adjusted to the geometric extension
     ** in x minus 0.24 cm inactive area.
     **/
    CbmStsSensor* DefaultSensor(Int_t address, TGeoPhysicalNode* node);


    /** Define available sensor types
     ** Poor man's sensor database, with hardcoded values
     ** @value Number of available sensor types
     **/
    Int_t DefineSensorTypes();


    /** Get the digitiser task **/
    CbmStsDigitize* GetDigitizer() const { return fDigitizer; }


    /** Get digitize parameters **/
    CbmStsDigitizeParameters* GetDigiParameters() const  {
      return fSettings;
    }

    /** Get an STS element by address
     ** @param address  Unique element address (see CbmStsAddress)
     ** @param level    Element level (EStsElementLevel)
     ** @return Pointer to STS element
     **/
    CbmStsElement* GetElement(Int_t address, Int_t level);


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
    //Int_t GetNofSensorTypes() const { return fSensorTypes.size(); }


    /** Get number of stations **/
    Int_t GetNofStations() const { return fStations.size(); }


    /** Get a module from the module array.
     ** For convenient loops over all modules.
     ** Note that the index of the module is meaningless.
     ** @param  index  Index of module in the array
     ** @return  Pointer to module
     **/
    CbmStsModule* GetModule(Int_t index) { return fModules[index]; }


   /** Get a sensor type from the list of available ones
     ** @param index  Index of sensor type in the array.
     ** @value type   Pointer to sensor type. NULL if index is out of bounds.
    CbmStsSensorType* GetSensorType(Int_t index) {
    	if ( fSensorTypes.find(index) != fSensorTypes.end() )
    		return fSensorTypes[index];
    	return NULL;
    }
    **/


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
    Int_t GetStationNumber(Int_t address);


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


    /** Static instance of CbmStsSetup **/
    static CbmStsSetup* Instance();


    /** Print list of sensors with parameters **/
    void ListSensors() const {
      for (auto it = fSensors.begin(); it != fSensors.end(); it++)
     LOG(INFO) << it->second->ToString() << FairLogger::endl;
    }


    /** @brief Modify the strip pitch for all sensors
     ** @param pitch  New strip pitch [cm]
     ** @value Number of modified sensors
     **
     ** This method will have affect only for sensor of type DSSD.
     ** The strip pitch of all sensors is modified to the specified
     ** value. The number of strips in the sensor is re-calculated.
     **/
    Int_t ModifyStripPitch(Double_t pitch);


    /** @brief Set the digitiser task
     ** @param digitizer  Pointer to STS digitiser task
     **
     ** In the digitisation run, the digitiser task is available from
     ** the setup instance. In other runs, this will be NULL.
     **/
    void SetDigitizer(CbmStsDigitize* digitizer) {
      fDigitizer = digitizer;
    }


    /** @brief Set the digitiser parameters 
     ** @param settings  Pointer to digitiser parameters
     **
     ** The parameters are registered by the digitiser task during digitisation.
     ** They have to be set by a reconstruction task during reconstruction.
     **/
    void SetDigiParameters(CbmStsDigitizeParameters* settings) {
      fSettings = settings;
    }


    /** @brief Set conditions for all sensors (same values for all)
     ** @param vDep         Full depletion voltage [V]
     ** @param vBias        Bias voltage [V]
     ** @param temperature  Temperature [K]
     ** @param cCoupling    Coupling capacitance [pF]
     ** @param cInterstrip  Inter-strip capacitance [pF]
     ** @value Number if sensors the conditions were set to.
     **
     ** The values are taken from the member fSettings, which has to be
     ** initialised before.
     **/
    Int_t SetSensorConditions();


    /** @brief Set conditions for all sensors (same values for all)
     ** @param vDep         Full depletion voltage [V]
     ** @param vBias        Bias voltage [V]
     ** @param temperature  Temperature [K]
     ** @param cCoupling    Coupling capacitance [pF]
     ** @param cInterstrip  Inter-strip capacitance [pF]
     ** @value Number if sensors the conditions were set to.
     **
     ** In addition to the specified values, the magnetic field in the
     ** sensor centre is evaluated and stored.
     **/
    Int_t SetSensorConditions(Double_t vDep, Double_t vBias,
                              Double_t temperature, Double_t cCoupling,
                              Double_t cInterstrip);


    /** Set sensor parameters
     ** Set the sensor parameters that are not contained in the geometry,
     ** but required for digitisation and reconstruction, like strip pitch,
     ** stereo angle etc.
     ** @value Number of sensors the type of which was set.
     **/
    Int_t SetSensorTypes(TString fileName = "");




  private:

    static CbmStsSetup* fgInstance;    ///< Static instance of this class
    CbmStsDigitize* fDigitizer;        ///< Pointer to digitiser task
    CbmStsDigitizeParameters* fSettings;     ///< Pointer to digitiser settings
    TString fSensorTypeFile;  ///< File name associating sensor types to the sensors

    Bool_t fIsInitialised;  ///< To protect against multiple initialisation.
    Bool_t fIsOld;          ///< Old setup with stations as top level

    /** These arrays allow convenient loops over all modules or sensors. **/
    std::vector<CbmStsModule*> fModules;   //! Array of modules
    std::map<UInt_t, CbmStsSensor*> fSensors;   //! Map of sensors. Key is address.

    /** Stations (special case; are not elements in the setup) **/
    std::map<Int_t, CbmStsStation*> fStations;  //!

    /** Sensor type "database". Key is the type name. **/
    //std::map<TString, CbmStsSensorType*> fSensorTypes; //!

    /** Default constructor  **/
    CbmStsSetup();

    /** Create station objects **/
    Int_t CreateStations();

    /** Copy constructor (disabled) **/
    CbmStsSetup(const CbmStsSetup&) = delete;

    /** Assignment operator (disabled) **/
    CbmStsSetup operator=(const CbmStsSetup&) = delete;

    ClassDef(CbmStsSetup, 2);

};

#endif /* CBMSTSSETUP_H */
