// -------------------------------------------------------------------------
// -----                      CbmRichPoint source file                 -----
// -----               Created 28/04/04  by B. Polichtchouk            -----
// -------------------------------------------------------------------------

#include "CbmRichPoint.h"

#include "FairLogger.h"

// -----   Default constructor   -------------------------------------------
CbmRichPoint::CbmRichPoint() : FairMCPoint() {}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmRichPoint::CbmRichPoint(Int_t trackID, Int_t detID, TVector3 pos, 
			   TVector3 mom, Double_t tof, Double_t length, 
			   Double_t eLoss)
  : FairMCPoint(trackID, detID, pos, mom, tof, length, eLoss) { }
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmRichPoint::~CbmRichPoint() { }
// -------------------------------------------------------------------------


// -----   Public method Print   -------------------------------------------
void CbmRichPoint::Print(const Option_t* /*opt*/) const {
  LOG(INFO) << "-I- CbmRichPoint: RICH Point for track " << fTrackID 
            << " in detector " << fDetectorID << FairLogger::endl;
  LOG(INFO) << "    Position (" << fX << ", " << fY << ", " << fZ
            << ") cm" << FairLogger::endl;
  LOG(INFO) << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
            << ") GeV" << FairLogger::endl;
  LOG(INFO) << "    Time " << fTime << " ns,  Length " << fLength 
            << " cm,  Energy loss " << fELoss*1.0e06 << " keV" << FairLogger::endl;
}
// -------------------------------------------------------------------------



ClassImp(CbmRichPoint)
