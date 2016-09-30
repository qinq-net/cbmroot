// -------------------------------------------------------------------------
// -----                      CbmPsdPoint source file                 -----
// -----                   Created 28/07/04  by V. Friese              -----
// -------------------------------------------------------------------------


#include "CbmPsdPoint.h"

#include "FairLogger.h"


// -----   Default constructor   -------------------------------------------
CbmPsdPoint::CbmPsdPoint() 
  : FairMCPoint(), 
    fModuleID(0)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmPsdPoint::CbmPsdPoint(Int_t trackID, Int_t detID, TVector3 pos, 
			   TVector3 mom, Double_t tof, Double_t length, 
			   Double_t eLoss)
  : FairMCPoint(trackID, detID, pos, mom, tof, length, eLoss), 
    fModuleID(0)
{
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
CbmPsdPoint::~CbmPsdPoint() { }
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void CbmPsdPoint::Print(const Option_t* /*opt*/) const {
  LOG(INFO) << "-I- CbmPsdPoint: ECAL point for track " << fTrackID 
            << " in detector " << fDetectorID << FairLogger::endl;
  LOG(INFO) << "    Position (" << fX << ", " << fY << ", " << fZ
            << ") cm" << FairLogger::endl;
  LOG(INFO) << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
            << ") GeV" << FairLogger::endl;
  LOG(INFO) << "    Time " << fTime << " ns,  Length " << fLength 
            << " cm,  Energy loss " << fELoss*1.0e06 << " keV" << FairLogger::endl;
}
// -------------------------------------------------------------------------


ClassImp(CbmPsdPoint)
