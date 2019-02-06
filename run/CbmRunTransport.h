/** @author Volker Friese <v.friese@gsi.de>
 ** @date 31.01.2019
 **/

#ifndef CBMRUNTRANSPORT_H
#define CBMRUNTRANSPORT_H 1


#include "TNamed.h"
#include "TString.h"

class FairGenerator;
class FairPrimaryGenerator;
class FairRunSim;
class CbmSetup;
class CbmTarget;


enum ECbmGenerator {
  kUnigen, kUrqmd, kPluto
};

enum ECbmEngine {
  kGEANT3, kGEANT4
};


class CbmRunTransport : public TNamed
{

  public:

    /** @brief Constructor **/
    CbmRunTransport();


    /** @brief Destructor  **/
    virtual ~CbmRunTransport();


    /** @brief Add an input by file name and generator type
     ** @param fileName   Name of input file
     ** @param generator  Type of generator input
     **/
    void AddInput(const char* fileName, ECbmGenerator generator = kUnigen);


    /** @brief Add an input by generator instance
     ** @param Pointer to generator instance
     **/
    void AddInput(FairGenerator* generator);


    /** @brief Trigger generation of a run info file
     ** @param choice  If kTRUE, run info file will be generated.
     **
     ** The run info file contains information on resource usage.
     **/
    void GenerateRunInfo(Bool_t choice = kTRUE) { fGenerateRunInfo = choice; }


    /** @brief Detector setup interface
     ** @value Pointer to CbmSetup class
     **/
    CbmSetup* GetSetup() const { return fSetup; }


    /** @brief Use a standard setup
     ** @param setupName  Name of standard setup
     **/
    void LoadSetup(const char* setupName);


    /** @brief Execute transport run
     ** @param nEvents  Number of events to process
     **/
    void Run(Int_t nEvents);


    /** @brief Set the beam angle (emittency at the beam position)
     ** @param x0  Beam mean angle in the x-z plane [rad]
     ** @param y0  Beam mean angle in the y-z plane [rad]
     ** @param sigmaX  Beam angle width in x-z [rad].
     ** @param sigmaY  Beam angle width in y-z [rad].
     **
     ** For each event, beam angles in x and y will be generated
     ** from Gaussian distributions with the specified parameters.
     ** The event will be rotated accordingly from the beam C.S.
     ** into the global C.S.
     **
     ** Without using this method, the default beam is always in z direction.
     **/
    void SetBeamAngle(Double_t x0, Double_t y0,
                      Double_t sigmaX, Double_t sigmaY);


    /** @brief Set the beam position
     ** @param x0  Beam centre position in x [cm]
     ** @param y0  Beam centre position in y [cm]
     ** @param sigmaX  Beam width in x [cm]. Default is 0.1.
     ** @param sigmaY  Beam width in y [cm]. Default is 0.1.
     **
     ** The beam parameters are used to generate the event vertex.
     ** A Gaussian beam profile in x and y is assumed.
     ** If sigmaX or sigmaY are null, the event vertex is
     ** always at (x0, y0).
     ** Smearing of the event vertex in the transverse plane can be
     ** deactivated by the method SetBeamSmearXY.
     ** Without using this method, the default beam is centred at the
     ** origin.
     **/
    void SetBeamPosition(Double_t x0, Double_t y0,
                         Double_t sigmaX = 0.1, Double_t sigmaY = 0.1);


    /** @brief Set transport engine
     ** @param engine  kGEANT3 or kGEANT4
     **
     ** By default, GEANt3 is used.
     **/
    void SetEngine(ECbmEngine engine) { fEngine = engine; }


    /** @brief Define geometry file name (output)
     ** @param name  Name for geometry file
     **
     ** If a file name is specified, a ROOT file containing the used
     ** geometry will be created (containing the TGeoManager).
     **/
    void SetGeoFileName(TString name);


    /** @brief Define parameter file name
     ** @param name  Name for parameter file
     **
     ** If the parameter file does not exist, it will be created.
     **/
    void SetParFileName(TString name);


    /** @brief Define the target
     ** @param medium  Name of target medium
     ** @param thickness  Thickness of target (in z) [cm]
     ** @param diameter   Target diameter [cm]
     **
     ** The target is a disk with the z axis as symmetry axis.
     ** By default, it is centred at the origin. Its position
     ** and rotation w.r.t. the y axis can be changed by the methods
     ** SetTargetPosition and SetTargetRotation.
     **/
    void SetTarget(const char* medium, Double_t thickness, Double_t diameter);


    /** @brief Define the target position in the global C.S.
     ** @param x  Target centre position in x [cm]
     ** @param y  Target centre position in y [cm]
     ** @param z  Target centre position in z [cm]
     **
     ** The method SetTarget has to be called before.
     ** By default, the target is positioned at the origin of the global C.S.
     **/
    void SetTargetPosition(Double_t x, Double_t y, Double_t z);


    /** @brief Define the target rotation around the y axis in the global C.S.
     ** @param angle  Rotation angle around y axis [degrees]
     **
     ** The method SetTarget has to be called before.
     ** By default, the target is perpendicular to the z axis.
     **/
    void SetTargetRotation(Double_t angle);


    /** @brief Define output file name
     ** @param name  Name for output file
     **/
    void SetOutFileName(TString name);


    /** @brief Enable smearing of event vertex in x and y.
     ** @param choice  If kTRUE(default), smearing is enabled.
     **
     ** If enabled, the event vertex will be sampled in x and y
     ** from a Gaussian distribution with the parameters specified
     ** by the method SetBeamPosition.
     **
     ** TODO: It is not guaranteed that the generated event vertex
     ** falls into the target volume. In order to ensure that,
     ** the method FairPrimaryGenerator::MakeVertex would have
     ** to be modified.
     */
    void SetVertexSmearXY(Bool_t choice = kTRUE);


    /** @brief Enable smearing of event vertex in z.
     ** @param choice  If kTRUE(default), smearing is enabled.
     **
     ** If enabled, the event vertex z coordinate will be sampled
     ** from a flat distribution inside the target.
     ** If no target is defined, the event z vertex will always be zero.
     */
    void SetVertexSmearZ(Bool_t choice = kTRUE) { fVertexSmearZ = choice; }



  private:

    CbmSetup* fSetup;
    CbmTarget* fTarget;  //!
    FairPrimaryGenerator* fEventGen;
    FairRunSim* fRun;
    TString fOutFileName;
    TString fParFileName;
    TString fGeoFileName;
    std::vector<FairGenerator*> fGenerators;
    Double_t fRealTimeInit;
    Double_t fRealTimeRun;
    Double_t fCpuTime;
    Bool_t fVertexSmearZ;
    ECbmEngine fEngine;
    Bool_t fGenerateRunInfo;




    /** @brief Create and register the setup modules **/
    void RegisterSetup();



    ClassDef(CbmRunTransport, 1);
};

#endif /* CBMRUNTRANSPORT_H */

