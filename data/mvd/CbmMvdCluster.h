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
#include "CbmMvdDetectorId.h"
#include <map>



class CbmMvdCluster : public CbmCluster, public CbmMvdDetectorId
{
 public:    

  /** Default constructor **/
  CbmMvdCluster();

  CbmMvdCluster(const CbmMvdCluster&);

  CbmMvdCluster& operator=(const CbmMvdCluster&){return *this;};

  /** Destructor **/
  virtual ~CbmMvdCluster();
 
  /** Setters **/
  void SetPixelMap(std::map <std::pair<Int_t, Int_t>, Int_t > PixelMap); 
  void SetRefId(Int_t RefId){fRefId = RefId;}; //* stores the index to the global TClonesArray	

 /** Accessors **/
  Short_t GetTotalDigisInCluster(){return fPixelMap.size();};
  std::map <std::pair<Int_t, Int_t>, Int_t > GetPixelMap(){return fPixelMap;};
  Int_t   GetStationNr() {return (Int_t) GetAddress()/1000;};
  Int_t   GetRefId(){return fRefId;};
  Int_t   GetDetectorId() {return DetectorId(GetStationNr());};
  Int_t   GetSensorNr(){return GetAddress()- ( ( (Int_t) GetAddress()/1000) * 1000 ) ;};

  Float_t GetClusterCharge(){return fClusterCharge;};
   
 protected:
    std::map <std::pair<Int_t, Int_t>, Int_t > fPixelMap;
    Int_t fRefId;
    Float_t fClusterCharge;

  ClassDef(CbmMvdCluster,4);

};

#endif
