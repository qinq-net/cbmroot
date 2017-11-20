// -------------------------------------------------------------------------
// -----                      CbmMvdCluster header file            -----
// -----
// -------------------------------------------------------------------------


/** CbmMvdCluster.h



**/


#ifndef CBMMVDCLUSTER_H
#define CBMMVDCLUSTER_H 1


#include "TVector3.h"
#include "TObject.h"
#include "CbmCluster.h"
#include "FairLogger.h"
#include <map>

class CbmMvdCluster : public CbmCluster
{

 public:    

  /** Default constructor **/
  CbmMvdCluster();

  CbmMvdCluster(Int_t* digiList, Short_t digisInThisObject, Short_t totalDigisInCluster, Int_t neighbourDown); 


CbmMvdCluster(const CbmMvdCluster&);

CbmMvdCluster& operator=(const CbmMvdCluster&){return *this;};

  /** Destructor **/
  virtual ~CbmMvdCluster();
 
  /** Setters **/
  void SetNeighbourUp(Int_t index){fNeighbourUp=index;};
  void SetNeighbourDown(Int_t index){fNeighbourDown=index;};
  void SetPixelMap(std::map <std::pair<Int_t, Int_t>, Int_t > PixelMap); 
  void SetStationNr(Int_t stationNr){fStation = stationNr;};
  void SetRefId(Int_t RefId){fRefId = RefId;}; //* stores the index to the global TClonesArray	
  void SetDetectorId(Int_t detId)      { fDetectorId = detId;};
  void SetSensorNr(Int_t SensorNr){fSensorNr = SensorNr;};
 

 /** Accessors **/
  Int_t   GetNeighbourDown(){return fNeighbourDown;};
  Int_t   GetNeighbourUp(){return fNeighbourUp;};
  Short_t GetDigisInThisObject(){return fDigisInThisObject;};
  Short_t GetTotalDigisInCluster(){return fPixelMap.size();};
  Short_t GetMaxDigisInThisObject(){return fMaxDigisInObject;};
  std::map <std::pair<Int_t, Int_t>, Int_t > GetPixelMap(){return fPixelMap;};
  Int_t   GetStationNr() {return fStation;};
  Int_t*  GetDigiList(){return fDigiArray;};
  Int_t   GetRefId(){return fRefId;};
  Int_t   GetDetectorId() {return fDetectorId;};
  Int_t   GetSensorNr(){return fSensorNr;};

  Float_t GetClusterCharge(){return fClusterCharge;};
   
 protected:
     static const Short_t fMaxDigisInObject=8;
     Int_t fDigiArray[fMaxDigisInObject];
     std::map <std::pair<Int_t, Int_t>, Int_t > fPixelMap;
     Int_t fNeighbourDown;
     Int_t fNeighbourUp;
     Short_t fDigisInThisObject;
     Short_t fTotalDigisInCluster;
     Int_t fStation;
     Int_t fRefId;
     Int_t fDetectorId;
     Int_t fSensorNr;
     Float_t fClusterCharge;

  ClassDef(CbmMvdCluster,3);

};

#endif
