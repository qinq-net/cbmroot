/** @file CbmStsSetup.h
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 10.06.2013
 **/

#ifndef CBMSETUP_H
#define CBMSETUP_H 1


#include <map>
#include "TNamed.h"
#include "TString.h"
#include "TVector3.h"

class FairModule;
class FairRunSim;
class CbmFieldMap;

using namespace std;



class CbmSetup : public TNamed
{
  public:


    /** Destructor **/
    ~CbmSetup() { };


    /** Clear the setup
     **
     ** All settings are removed.
     **/
    virtual void Clear(Option_t* opt = "");


    /** Create the field map using the given settings **/
    CbmFieldMap* CreateFieldMap();


    /** Get a geometry tag
     ** @param[in]  moduleId  Module identifier (type ESystemId or EPassiveId)
     ** @param[out] tag       Geometry tag for this module
     ** @return kTRUE if module is present in setup; else kFALSE
     **/
    Bool_t GetGeoTag(Int_t moduleId, TString& tag);


    /** Get a geometry file name
     ** @param[in]  moduleId  Module identifier (type ESystemId or EPassiveId)
     ** @param[out] fileName  Geometry file name for this module
     ** @return kTRUE if module is present in setup; else kFALSE
     **/
    Bool_t GetGeoFileName(Int_t moduleId, TString& fileName);


    /** Get number of modules in the setup
     ** @value  Number of modules in setup
     **/
    Int_t GetNofModules() const { return fGeoTags.size(); }


    /** Get the PSD x position
     ** @param xPos  x position of PSD [cm]
     **/
    Double_t GetPsdPositionX() const { return fPsdPositionX; }


    /** Get the PSD z position
     ** @param zPos  z position of PSD [cm]
     **/
    Double_t GetPsdPositionZ() const { return fPsdPositionZ; }


    /** Initialisation
     ** @param run  Pointer to FairRunSim instance
     ** The availability of the input files is checked, and if given,
     ** modules and field are instantiated and registered to the run.
     ** If the argument is not specified or NULL, only a check of the
     ** availability of the input files is performed.
     **/
    void Init(FairRunSim* run = NULL);


    /** Get singleton instance of CbmSetup **/
    static CbmSetup* Instance();


    /** Get the activity flag of a detector
     ** @param moduleId  Module identifier (type ESystemId or EPassiveId)
     ** @return kTRUE if detector and active, else kFALSE
     **/
    Bool_t IsActive(Int_t moduleId);


    /** Check whether the setup is empty (contains no modules)
     ** @value kTRUE if the number of modules is null
     **/
    Bool_t IsEmpty() const { return (GetNofModules() == 0); }


    /** Output to screen  **/
    virtual void Print(Option_t* opt = "");


    /** Remove a module from the current setup
     ** @param moduleId  Module identifier (enum SystemId or kMagnet etc.)
     **/
    void RemoveModule(Int_t moduleId);


    /** Activate a module (detector)
     ** @param moduleId  Module identifier. SystemId for detectors, or kMagnet, kPipe, kTarget
     ** @param active    Activity tag for module (only in case of detectors)
     **
     ** Activate or deactivate a detector already present in the setup.
     ** The method will have no effect if called for a passive module (target,
     ** pipe, magnet). If a detector is flagged active,
     ** its ProcessHits method will be called during the transport simulation.
     **/
    void SetActive(Int_t moduleId, Bool_t active = kTRUE);


    /** Set the magnetic field map
     ** @param tag   Field map tag
     ** @param scale Field scaling factor
     **
     ** The magnetic field map is automatically selected according to the
     ** magnet geometry version. The user can, however, override this by
     ** choosing a different field map. In this case, consistency between
     ** field map and magnet geometry is within the responsibility of the
     ** user.
     **/
    void SetField(const char* tag, Double_t scale = 1., Double_t xPos = 0.,
    		          Double_t yPos = 0., Double_t zPos = 0.);


    /** Set the field scaling factor
     ** @param scale  Field scaling factor
     **
     ** The currently selected field map will be scaled by the specified
     ** factor.
     **/
    void SetFieldScale(Double_t scale) { fFieldScale = scale; }


    /** Add a module to the setup
     ** @param moduleId  Module identifier. SystemId for detectors, or kMagnet, kPipe, kTarget
     ** @param geoTag    Geometry version for module
     ** @param active    Activity tag for module (only in case of detectors)
     **
     ** The module / detector will be added to the setup. If a detector is flagged active,
     ** its ProcessHits method will be called during the transport simulation.
     **/
    void SetModule(Int_t moduleId, const char* geoTag, Bool_t active = kTRUE);


    /** Set the PSD
     ** @param geoFile  Setup file name for PSD
     ** @param zPos     z position of PSD [cm]
     ** @param xPos     x position of PSD [cm]
     ** @param active    Activity tag for module (only in case of detectors)
     **
     ** The construction of the PSD does not follow the same convention
     ** (with geometry tag) as that of the other modules. It thus necessitates
     ** special treatment.
     **/
    void SetPsd(TString geoFile, Double_t zPos, Double_t xPos,
    		        Bool_t active = kTRUE) {
    	fGeoTags[kPsd]      = "NDEF";
    	fGeoFileNames[kPsd] = geoFile;
    	fActive[kPsd]       = active;
    	fPsdPositionZ       = zPos;
    	fPsdPositionX       = xPos;
    }



  private:

    static CbmSetup* fgInstance;            ///< Pointer to static instance

    map<Int_t, TString> fGeoTags;           ///< Geometry version tags
    map<Int_t, TString> fGeoFileNames;      ///< Geometry file names
    map<Int_t, Bool_t>  fActive;            ///< Activity tags for the modules
    TString             fFieldTag;          ///< Version tag for magnetic field
    Double_t            fFieldScale;        ///< Field scaling factor
    TVector3            fFieldPosition;     ///< Position of field map origin
    Double_t            fPsdPositionX;      ///< x position of PSD [cm]
    Double_t            fPsdPositionZ;      ///< z position of PSD [cm]


    /** Default constructor **/
    CbmSetup()  :
    	TNamed("CBM Setup", ""),
    	fGeoTags(),
    	fGeoFileNames(),
    	fActive(),
    	fFieldTag(""),
    	fFieldScale(1.0),
    	fFieldPosition(0., 0., 0.),
    	fPsdPositionX(0.),
    	fPsdPositionZ(0.)
    { };


    /** Constructor with pre-defined setup
     ** @param  setup   Name of setup
     **/
    CbmSetup(const char* setup) :
    	TNamed("CBM Setup", ""),
    	fGeoTags(),
     	fGeoFileNames(),
      fActive(),
      fFieldTag(""),
      fFieldScale(1.0),
      fFieldPosition(0., 0., 0.),
      fPsdPositionX(0.),
      fPsdPositionZ(0.)
    {
      ReadSetup(setup);
    };


    /** Copy constructor and assignment operator (not implemented ) **/
    CbmSetup(const CbmSetup&);
    CbmSetup operator=(const CbmSetup&);


    /** Instantiate a module
     ** @param moduleId  Module identifier (ESystemId or EPassiveId)
     ** @param active    Activity flag
     **
     ** This factory method is not yet implemented since it would require
     ** too many directories to be added to the include path variable.
     **/
    FairModule* CreateModule(Int_t moduleId, Bool_t active);


    /** Get the geometry file name for a given module
     ** @param[in] moduleId  Module identifier (ESystemId or EPassiveId)
     ** @param[out] fileName  Geometry file name (full path)
     ** @value  kTRUE if geometry tag was set and file was found
     **
     ** If a geometry tag is defined for the specified module, the
     ** geometry file name is constructed according to the naming conventions.
     ** First, a ROOT file is looked for; if that is not found, an ASCII
     ** file is looked for. If no tag was specified or no file can be found,
     ** the method return kFALSE.
     **/
    Bool_t GetFile(Int_t moduleId, TString& fileName);


    /** Get the position of the magnet module
     ** @param[in] geoTag  Geometry tag for the magnet module
     ** @param[out] x,y,z  Magnet centre coordinates in the global c.s. [cm]
     ** @value  kTRUE if a valid magnet geometry files was found
     **
     ** The method looks in the magnet geometry file for the translation
     ** of the magnet node w.r.t. the global c.s. This can be used to
     ** automatically place the field map correctly.
     **/
    Bool_t GetMagnetPosition(const char* geoTag,
                             Double_t& x, Double_t& y, Double_t& z);


    /** Read a predefined setup from file
     ** @param setupName  Name tag of setup
     ** The setup file defines the geometry flags for all modules
     ** in the setup.
     ** The method is not in use for the time being. The setup is
     ** defined via ROOT macros.
     **/
    void ReadSetup(const char* setupName);


    ClassDef(CbmSetup, 2);

};

#endif /* CBMSETUP_H */
