// -------------------------------------------------------------------------
// -----                      CbmMvdCluster header file            -----
// -----
// -------------------------------------------------------------------------


/** CbmMvdCluster.h



**/


#ifndef CBMMVDCLUSTER_H
#define CBMMVDCLUSTER_H 1


//#include "CbmHit.h"
#include "TVector3.h"
//#include "CbmMvdHit.h"
#include "TObject.h"

class CbmMvdCluster : public TObject
{

 public:    

  /** Default constructor **/
  CbmMvdCluster();


  /** Standard constructor 
  *@param statNr Station number
  *@param pos    Position coordinates [cm]
  *@param dpos   Errors in position coordinates [cm]
  *@param flag   Hit flag
  *@param chargeArray Array of charge of 49 pixels
  **/
  
  CbmMvdCluster(Int_t* digiList, Short_t digisInThisObject, Short_t totalDigisInCluster, Int_t neighbourDown);

  CbmMvdCluster(Int_t dummyInt1, TVector3 dummyVect1, TVector3 dummyVect2, Int_t dummyInt2, Short_t* dummyShort1, Float_t dummyFloat1, Float_t dummyFloat2){};
  //this constructor is only for littrack error, littrack has to implement new clustertyp
  
  
  /** Destructor **/
  virtual ~CbmMvdCluster();


  //these functions are only for littrack
    Int_t GetDominatorX(){;};
    Int_t GetDominatorY(){;};
    Int_t GetTrackID(){;};
    Int_t GetContributors(){;};
    
    void SetDebuggingInfo(Short_t* foo1, Float_t foo2[5], Float_t foo3[5]){;};
    void SetContributors(Short_t short1){;};
    void PrintCluster(){;};
    //
  
  /** Setters **/
  
  void SetNeighbourUp(Int_t index){fNeighbourUp=index;};
  void SetNeighbourDown(Int_t index){fNeighbourDown=index;};
  
  /** Accessors **/
  
  Int_t   GetNeighbourDown(){return fNeighbourDown;};
  Int_t   GetNeighbourUp(){return fNeighbourUp;};
  Short_t GetDigisInThisObject(){return fDigisInThisObject;};
  Short_t GetTotalDigisInCluster(){return fTotalDigisInCluster;};
  Short_t GetMaxDigisInThisObject(){return fMaxDigisInObject;};
  
  Int_t  GetDigiIndex(UInt_t index);
  Int_t* GetDigiList(){return fDigiArray;};
  
   
 protected:
     static const Short_t fMaxDigisInObject=8;
     Int_t fDigiArray[fMaxDigisInObject];
     Int_t fNeighbourDown;
     Int_t fNeighbourUp;
     Short_t fDigisInThisObject;
     Short_t fTotalDigisInCluster;
       
  ClassDef(CbmMvdCluster,1);

};


#endif
