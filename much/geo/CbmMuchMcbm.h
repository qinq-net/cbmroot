/** CbmMuchMcbm.h
 *
 * @author  M.Ryzhinskiy <m.ryzhinskiy@gsi.de>
 * @version 1.0
 * @since   21.03.07
 *
 *  Class for MUon CHambers detector
 *
 */

#ifndef CBMMUCHMCBM_H
#define CBMMUCHMCBM_H


#include "TLorentzVector.h"
#include "FairDetector.h"


class TClonesArray;
class CbmMuchPoint;
class FairVolume;
class CbmGeoMuchPar;
class TGeoMedium;
class TGeoCombiTrans;

class CbmMuchMcbm : public FairDetector
{

 public:

  /** Default constructor **/
  CbmMuchMcbm();


  /** Standard constructor.
   *@param name    detector name
   *@param active  sensitivity flag
   **/
  CbmMuchMcbm(const char* name, Bool_t active);


  /** Destructor **/
  virtual ~CbmMuchMcbm();


  /** @brief Check whether a volume is sensitive.
   **
   ** @param(name)  Volume name
   ** @value        kTRUE if volume is sensitive, else kFALSE
   **
   ** The decision is based on the volume name (has to contain "Sensor").
   ** Virtual from FairModule.
   **/
  virtual Bool_t CheckIfSensitive(std::string name) {
    return ( TString(name).Contains("ctive") ? kTRUE : kFALSE );
  }

  
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
  virtual void ConstructGeometry();

  virtual void        ConstructRootGeometry();
  virtual void        ConstructAsciiGeometry();
  void                ExpandMuchNodes(TGeoNode* fN);

  //  void SaveGeoParams();

 private:


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
  TGeoCombiTrans* fCombiTrans;       //! Transformation matrix for geometry positioning         

  /** Private method AddHit
   **
   ** Adds a MuchPoint to the HitCollection
   **/
  CbmMuchPoint* AddHit(Int_t trackID, Int_t detID,
		       TVector3 posIn, TVector3 posOut,
		       TVector3 momIn, TVector3 momOut,
		       Double_t time, Double_t length, Double_t eLoss);

  /** @brief Check how the TGeoVolume in file was produced
   *  Check how the TGeoVolume in the geometry file was produced.
   *  The new way is to export the volume with the Export function
   *  of TGeoVolume together with a TGeoMatrix.
   *  To identify a file of new type check for TGeoVolume and a TGeoMatrix
   *  derived class in the file.
   */
  Bool_t IsNewGeometryFile(TString filename);

  /** Private method ResetParameters
   **
   ** Resets the private members for the track parameters
   **/
  void ResetParameters();
  Int_t GetDetId(FairVolume* vol);

 private:

  Int_t Intersect(Float_t x, Float_t y, Float_t lx, Float_t ly, Float_t r);
  TGeoMedium* CreateMedium(const char* matName);

  CbmMuchMcbm(const CbmMuchMcbm&);
  CbmMuchMcbm& operator=(const CbmMuchMcbm&);

  ClassDef(CbmMuchMcbm,1);

};


inline void CbmMuchMcbm::ResetParameters() {
  fTrackID = fVolumeID = 0;
  fPosIn .SetXYZM(0.0, 0.0, 0.0, 0.0);
  fPosOut.SetXYZM(0.0, 0.0, 0.0, 0.0);
  fMomIn .SetXYZM(0.0, 0.0, 0.0, 0.0);
  fMomOut.SetXYZM(0.0, 0.0, 0.0, 0.0);
  fTime = fLength = fELoss = 0;
  fPosIndex = 0;
};

#endif /* CBMMUCHMCBM_H */

