// -------------------------------------------------------------------------
// -----                      CbmEcalHit source file                   -----
// -----               Created 11/03/05  by Yu.Kharlov                 -----
// -------------------------------------------------------------------------


#include "CbmEcalHit.h"

#include "FairLogger.h"

// -----   Default constructor   -------------------------------------------
CbmEcalHit::CbmEcalHit() 
  : TObject(), 
    fEnergy(0.), 
    fPSEnergy(0.), 
    fTime(0.), 
    fTrackId(0), 
    fDetectorId(0)
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmEcalHit::~CbmEcalHit() {}
// -------------------------------------------------------------------------


// -----   Public method Print   -------------------------------------------
void CbmEcalHit::Print(const Option_t* /*opt*/) const {
  LOG(INFO) << "ECAL hit: cellid=" << GetDetectorId() << ", Energy=" << fEnergy;
  LOG(INFO) << ", PS Energy=" << fPSEnergy;
  if (fTrackId>0) LOG(INFO) << ", TrackID= " << fTrackId;
  LOG(INFO) << FairLogger::endl;
//  LOG(INFO) << ", x=" << GetX() << ", y=" << GetY() << FairLogger::endl; 
}
// -------------------------------------------------------------------------

ClassImp(CbmEcalHit)
