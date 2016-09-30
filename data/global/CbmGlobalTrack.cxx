// -------------------------------------------------------------------------
// -----                    CbmGlobalTrack source file                 -----
// -----                  Created 01/12/05  by V. Friese               -----
// -----                  Modified 04/06/09  by A. Lebedev             -----
// -------------------------------------------------------------------------
#include "CbmGlobalTrack.h"

#include "FairLogger.h"

// -----   Default constructor   -------------------------------------------
CbmGlobalTrack::CbmGlobalTrack() 
  : TObject(),
    fStsTrack(-1),
    fTrdTrack(-1),
    fMuchTrack(-1),
    fRichRing(-1),
    fTofHit(-1),
    fParamFirst(),
    fParamLast(),
    fParamPrimaryVertex(),
    fPidHypo(0),
    fChi2(0.),
    fNDF(0),
    fFlag(0),
    fLength(0.)
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmGlobalTrack::~CbmGlobalTrack() { }
// -------------------------------------------------------------------------



// -----   Public method Print   -------------------------------------------
void CbmGlobalTrack::Print(Option_t*) const {
  LOG(INFO) << "StsTrack " << fStsTrack << ", TrdTrack " << fTrdTrack << ", MuchTrack " << fMuchTrack
            << ", RichRing " << fRichRing << ", TofHit " << fTofHit 
            << FairLogger::endl;
//  LOG(INFO) << "Parameters at first plane: " << FairLogger::endl;
//  fParamFirst.Print();
//  LOG(INFO) << "Parameters at last plane: " << FairLogger::endl;
//  fParamLast.Print();
  LOG(INFO) << "chi2 = " << fChi2 << ", NDF = " << fNDF
            << ", Quality flag " << fFlag << FairLogger::endl;
  LOG(INFO) << "length = " << fLength << FairLogger::endl;
}
// -------------------------------------------------------------------------


ClassImp(CbmGlobalTrack)
