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


    /** Get the digitiser task **/
    CbmStsDigitize* GetDigitizer() const { return fDigitizer; }


    /** Get digitisation parameters **/
    CbmStsDigitizeParameters* GetDigiParameters() const {
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


    /** Get number of stations **/
    Int_t GetNofStations() const { return fStations.size(); }


    /** Get a module from the module array.
     ** For convenient loops over all modules.
     ** Note that the index of the module is meaningless.
     ** @param  index  Index of module in the array
     ** @return  Pointer to module
     **/
    CbmStsModule* GetModule(Int_t index) { return fModules[index]; }


    /** Get a station
     ** @param stationId  Station number
     ** @value Pointer to station object. NULL if not present.
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


    /** @brief Initialise the setup
     ** @param sensorParameterFile  Name of file with sensor parameters
     ** @param geometryFile         Name of file with STS geometry
     ** @return  kTRUE if successfully initialised
     **
     ** The setup will be initialised from the STS geometry, either
     ** taken from the TGeoManager or, if specified, read from a
     ** geometry file.
     ** In addition to the geometry, internal parameters of the sensors
     ** have to be specified. This can be done through the parameter file,
     ** which should contain for each sensor to be defined a line with
     ** sensor_name sensor_type parameters
     ** The number of parameters depends on the sensor type.
     ** If no parameter file is specified, the default sensor type will be
     ** used (see method DefautSensor()).
     **/
    Bool_t Init(const char* geometryFile = nullptr,
                const char* sensorParameterFile = nullptr);


    /** @brief Initialisation status
     ** @return kTRUE if setup is initialised
     **/
    Bool_t IsInit() const { return fIsInitialised; }


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



  private:

    static CbmStsSetup* fgInstance;    ///< Static instance of this class
    CbmStsDigitize* fDigitizer;        ///< Pointer to digitiser task
    CbmStsDigitizeParameters* fSettings;     ///< Pointer to digitiser settings
    TString fSensorTypeFile;  ///< File name associating sensor types to the sensors

    Bool_t fIsInitialised;  ///< To protect against multiple initialisation.
    Bool_t fIsOld;          ///< Old setup with stations as top level

    Int_t fNofSensorsDefault;  ///< Number of sensors created by default

    /** These arrays allow convenient loops over all modules or sensors. **/
    std::vector<CbmStsModule*> fModules;   //! Array of modules
    std::map<Int_t, CbmStsSensor*> fSensors;   //! Map of sensors. Key is address.

    /** Stations (special case; are not elements in the setup) **/
    std::map<Int_t, CbmStsStation*> fStations;  //!

    /** Default constructor  **/
    CbmStsSetup();


    /** Create station objects **/
    Int_t CreateStations();


    /** @brief Read the geometry from TGeoManager
     ** @param geoManager  Instance of TGeoManager
     ** @return kTRUE if successfully read; kFALSE else
     **
     ** The ROOT geometry is browsed for elements of the setup,
     ** which are then instantiated and connected to the respective
     ** physical node.
     **/
    Bool_t ReadGeometry(TGeoManager* geoManager);


    /** @brief Read the geometry from a ROOT geometry file
     ** @param fileName  Name of geometry file
     ** @return kTRUE if successfully read; kFALSE else
     **
     ** The ROOT geometry is browsed for elements of the setup,
     ** which are then instantiated and connected to the respective
     ** physical node.
     **/
    Bool_t ReadGeometry(const char* fileName);


    /** @brief Read sensor parameters from file
     ** @param fileName  Parameter file name
     ** @value Number of sensors the type of which was set.
     **
     ** Read the sensor parameters that are not contained in the geometry,
     ** but required for digitisation and reconstruction, like strip pitch,
     ** stereo angle etc. Sensors not contained in the file will be
     ** instantiated with default parameters.
     **/
    Int_t ReadSensorParameters(const char* fileName);


    /** Copy constructor (disabled) **/
    CbmStsSetup(const CbmStsSetup&) = delete;


    /** Assignment operator (disabled) **/
    CbmStsSetup operator=(const CbmStsSetup&) = delete;

    ClassDef(CbmStsSetup, 2);

};

#endif /* CBMSTSSETUP_H */
