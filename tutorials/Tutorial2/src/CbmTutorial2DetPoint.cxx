#include "CbmTutorial2DetPoint.h"

#include <iostream>
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
CbmTutorial2DetPoint::CbmTutorial2DetPoint() : FairMCPoint() { }
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
CbmTutorial2DetPoint::CbmTutorial2DetPoint(Int_t trackID, Int_t detID, 
					 TVector3 pos, TVector3 mom, 
					 Double_t tof, Double_t length,
					 Double_t eLoss)
  : FairMCPoint(trackID, detID, pos, mom, tof, length, eLoss) { }
// -------------------------------------------------------------------------

// -----   Destructor   ----------------------------------------------------
CbmTutorial2DetPoint::~CbmTutorial2DetPoint() { }
// -------------------------------------------------------------------------

// -----   Public method Print   -------------------------------------------
void CbmTutorial2DetPoint::Print(const Option_t* opt) const {
  cout << "-I- CbmTutorial2DetPoint: Tutorial2Det point for track " << fTrackID
       << " in detector " << fDetectorID << endl;
  cout << "    Position (" << fX << ", " << fY << ", " << fZ
       << ") cm" << endl;
  cout << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
       << ") GeV" << endl;
  cout << "    Time " << fTime << " ns,  Length " << fLength
       << " cm,  Energy loss " << fELoss*1.0e06 << " keV" << endl;
}
// -------------------------------------------------------------------------

ClassImp(CbmTutorial2DetPoint)
  
