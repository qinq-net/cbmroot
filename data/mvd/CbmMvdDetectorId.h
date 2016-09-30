// -------------------------------------------------------------------------
// -----                   CbmMvdDetectorId header file                -----
// -----                 Created 22/04/09  by V. Friese                -----
// -------------------------------------------------------------------------


/** CbmMvdDetectorId.h
 ** Defines unique detector identifier for all MVD components. 
 ** Classes using this ID scheme should derive from this class.
 ** @author V.Friese <v.friese@gsi.de>
 **/
 
 
/** Current definition:
 ** System ID (kMVD=1) on bits 0-4
 ** Station number on bits 5-31
 **/
 

#ifndef CBMMVDDETECTORID_H
#define CBMMVDDETECTORID_H 1


#include "CbmDetectorList.h"

#include "FairLogger.h"

#include "TObject.h"

 
class CbmMvdDetectorId
{

  public:
  
  
  /** Constructor **/
  CbmMvdDetectorId();
  
  
  /** Destructor **/
  virtual ~CbmMvdDetectorId()  {}
  
  
  /** Create unique detector ID from station number **/
  Int_t DetectorId(Int_t iStation) const {
     return ( kMVD | (iStation << 5) );
  }
  
  
  /** Get System identifier from detector ID **/
  Int_t SystemId(Int_t detectorId) const {
    Int_t iSystem = detectorId & 31;
    if ( iSystem != kMVD ) {
    	LOG(ERROR) << "-E- CbmMvdDetectorId::GetSystemId : "
	     << "wrong system ID " << iSystem << FairLogger::endl;
	return -1;
    }
    return iSystem;
  }

  
  /** Get station number from detector ID **/
  Int_t StationNr(Int_t detectorId) const {
    return ( ( detectorId & ( ~31 ) ) >> 5 );
  }
  
  
  
  ClassDef(CbmMvdDetectorId,1);
  
};


#endif
