/** CbmMuch.h
 *
 * @author  M.Ryzhinskiy <m.ryzhinskiy@gsi.de>
 * @version 1.0
 * @since   21.03.07
 *
 *  Class for MUon CHambers detector
 *Modified 18/10/2017 by Omveer Singh
 */

#ifndef CBMMUCH_H
#define CBMMUCH_H


#include "TLorentzVector.h"
#include "FairDetector.h"
#include "TVector3.h"
#include <string>

class TClonesArray;
class CbmMuchPoint;
class FairVolume;
class CbmGeoMuchPar;
class TGeoMedium;
class TGeoCombiTrans;
//class CbmMuchGeoHandler;
//class CbmMuchGeoScheme;

class CbmMuch : public FairDetector
{

 public:

  /** Default constructor **/
  CbmMuch();


  /** Standard constructor.
   *@param name    detector name
   *@param active  sensitivity flag
   **/
  CbmMuch(const char* name, Bool_t active);
   void ExpandMuchNodes(TGeoNode* fN);
//void   UseGlobalPhysicsProcesses(Bool_t use) { fUseGlobalPhysicsProcesses=use; }


// Bool_t fIsSimulation;

  /** Destructor **/
  virtual ~CbmMuch();


  /** Virtual method ProcessHits
   **
   ** Defines the action to be taken when a step is inside the
   ** active volume. Creates a CbmMuchPoint and adds it to the
   ** collection.
   *@param vol  Pointer to the active volume
   **/
  virtual Bool_t ProcessHits(FairVolume* vol = 0);


  /** Virtual method BeginEvent
   **
   ** If verbosity level is set, print hit collection at the
   ** end of the event and resets it afterwards.
   **/

  virtual void BeginEvent();

  /** Virtual method EndOfEvent
   **
   ** If verbosity level is set, print hit collection at the
   ** end of the event and resets it afterwards.
   **/

  virtual void EndOfEvent();



  /** Virtual method Register
   **
   ** Registers the hit collection in the ROOT manager.
   **/
  virtual void Register();


  /** Accessor to the hit collection **/
  virtual TClonesArray* GetCollection(Int_t iColl) const;


  /** Virtual method Print
   **
   ** Screen output of hit collection.
   **/
  virtual void Print() const;


  /** Virtual method Reset
   **
   ** Clears the hit collection
   **/
  virtual void Reset();


  /** Virtual method CopyClones
   **
   ** Copies the hit collection with a given track index offset
   *@param cl1     Origin
   *@param cl2     Target
   *@param offset  Index offset
   **/
  virtual void CopyClones(TClonesArray* cl1, TClonesArray* cl2,
			  Int_t offset);


  /** Virtaul method Construct geometry
   **
   ** Constructs the Much geometry
   **/
 // virtual void ConstructNewGeometry();

// Construct AsciiGeometry
virtual void ConstructGeometry();
virtual void ConstructRootGeometry();
//virtual void ConstructAsciiGeometry();
Bool_t CheckIfSensitive(std::string name);

  //  void SaveGeoParams();

 private:
//virtual void Initialize();
 //CbmMuchGeoScheme* fGeoScheme;
//CbmMuchGeoHandler* fGeoHandler;
//virtual void SetSpecialPhysicsCuts();
//Bool_t         fUseGlobalPhysicsProcesses;
  /** Track information to be stored until the track leaves the
      active volume. **/
  Int_t          fTrackID;           //!  track index
  Int_t          fVolumeID;          //!  volume id
  TLorentzVector fPosIn, fPosOut;    //!  position
  TLorentzVector fMomIn, fMomOut;    //!  momentum
  Double32_t     fTime;              //!  time
  Double32_t     fLength;            //!  length
  Double32_t     fELoss;             //!  energy loss

  Int_t          fPosIndex;          //!
  TClonesArray*  fMuchCollection;    //!  The hit collection
  Bool_t         kGeoSaved;          //!
  TList *flGeoPar; //!
  CbmGeoMuchPar* fPar;               //!  parameter container
 std::string fVolumeName;
//TClonesArray* fMuchPoints;

  //TGeoCombiTrans*   fCombiTrans;
  /** Private method AddHit
   **
   ** Adds a MuchPoint to the HitCollection
   **/
Bool_t IsNewGeometryFile(TString filename);

  CbmMuchPoint* AddHit(Int_t trackID, Int_t detID,
		       TVector3 posIn, TVector3 posOut,
		       TVector3 momIn, TVector3 momOut,
		       Double_t time, Double_t length, Double_t eLoss);


  /** Private method ResetParameters
   **
   ** Resets the private members for the track parameters
   **/
  void ResetParameters();
// virtual void        ConstructRootGeometry();
  Int_t GetDetId(FairVolume* vol);
private:
  Int_t Intersect(Float_t x, Float_t y, Float_t lx, Float_t ly, Float_t r);
  TGeoMedium* CreateMedium(const char* matName);

  CbmMuch(const CbmMuch&);
  CbmMuch& operator=(const CbmMuch&);
TGeoCombiTrans*   fCombiTrans;
  ClassDef(CbmMuch,1);

};


inline void CbmMuch::ResetParameters() {
  fTrackID = fVolumeID = 0;
  fPosIn .SetXYZM(0.0, 0.0, 0.0, 0.0);
  fPosOut.SetXYZM(0.0, 0.0, 0.0, 0.0);
  fMomIn .SetXYZM(0.0, 0.0, 0.0, 0.0);
  fMomOut.SetXYZM(0.0, 0.0, 0.0, 0.0);
  fTime = fLength = fELoss = 0;
  fPosIndex = 0;
};


#endif
