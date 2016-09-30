// -------------------------------------------------------------------------
// -----                      CbmMvdPoint source file                  -----
// -----                  Created 06/11/06  by V. Friese               -----
// -------------------------------------------------------------------------
#include "CbmMvdPoint.h"

#include "FairLogger.h"

// -----   Default constructor   -------------------------------------------
CbmMvdPoint::CbmMvdPoint()
  : FairMCPoint(),
    CbmMvdDetectorId(),
    fX_out(0), 
    fY_out(0), 
    fZ_out(0), 
    fPx_out(0), 
    fPy_out(0), 
    fPz_out(0),
    fPdgCode(0),
    fPointId(-1),
    fFrame(0),
    fStartTime(0.)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmMvdPoint::CbmMvdPoint(Int_t trackID, Int_t pdgCode, Int_t stationNr, 
			 TVector3 posIn, TVector3 posOut, TVector3 momIn, 
			 TVector3 momOut, Double_t tof, Double_t length, 
			 Double_t eLoss, Int_t frame) 
  : FairMCPoint(trackID, stationNr, posIn, momIn, tof, length, eLoss),
    CbmMvdDetectorId(),
    fX_out(posOut.X()),
    fY_out(posOut.Y()),
    fZ_out(posOut.Z()),
    fPx_out(momOut.Px()),
    fPy_out(momOut.Py()),
    fPz_out(momOut.Pz()),
    fPdgCode(pdgCode),
    fPointId(-1),
    fFrame(frame),
    fStartTime(0)
{
FairRunSim* run = FairRunSim::Instance();
FairPrimaryGenerator* gen = run->GetPrimaryGenerator();
FairMCEventHeader* event = gen->GetEvent();

fStartTime = event->GetT();
fDetectorID = DetectorId(stationNr);
}

// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmMvdPoint::~CbmMvdPoint() { }
// -------------------------------------------------------------------------




// -----   Public method Print   -------------------------------------------
void CbmMvdPoint::Print(const Option_t* /*opt*/) const {
  LOG(INFO) << "MVD Point for track " << fTrackID 
            << " in station " << GetStationNr() << FairLogger::endl;
  LOG(INFO) << "    Position (" << fX << ", " << fY << ", " << fZ
            << ") cm" << FairLogger::endl;
  LOG(INFO) << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
            << ") GeV" << FairLogger::endl;
  LOG(INFO) << "    Time " << fTime << " ns,  Length " << fLength 
            << " cm,  Energy loss " << fELoss*1.0e06 << " keV" << FairLogger::endl;
}
// -------------------------------------------------------------------------

// -----   Public method GetAbsTime   --------------------------------------
Int_t CbmMvdPoint::GetAbsTime(){
  
  
  Int_t absTime = fTime + fStartTime;
  
  return absTime;
  
}
// -------------------------------------------------------------------------

ClassImp(CbmMvdPoint)
