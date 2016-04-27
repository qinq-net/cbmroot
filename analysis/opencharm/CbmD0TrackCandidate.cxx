// -------------------------------------------------------------------------
// -----                     CbmD0TrackCandidate header file           -----
// -----                  Created 23/01/07  by C.Dritsa and M.Deveaux  -----
// -----                        updated 2016 by P.Sitzmann             -----
// -------------------------------------------------------------------------

#include "CbmD0TrackCandidate.h"


// -----   Default constructor   -------------------------------------------
CbmD0TrackCandidate::CbmD0TrackCandidate()
    : TObject(),
      fTrackIndex(),
      fTrackIndexMC(),
      fPVChi2(),
      fnMvdHits(),
      fnStsHits(),
      fPidHypothesis(),
      fIP(),
      fImx(),
      fImy()

{
    fPVChi2 = 0;
    fTrackIndex = fTrackIndexMC = -1;

}
// -------------------------------------------------------------------------



// -----   Constructor with parameters   -----------------------------------
CbmD0TrackCandidate::CbmD0TrackCandidate(Int_t globalTrackId , Int_t matchedTrackId, Double_t PVChi2, Double_t IP, Double_t Imx, Double_t Imy, Int_t nMvdHits, Int_t nStsHits, Int_t pidHypo)
    : TObject(),
      fTrackIndex(),
      fTrackIndexMC(),
      fPVChi2(),
      fnMvdHits(),
      fnStsHits(),
      fPidHypothesis(),
      fIP(),
      fImx(),
      fImy()
{

    fTrackIndex   = globalTrackId;
    fTrackIndexMC = matchedTrackId;
    fPVChi2      = PVChi2;
    fnMvdHits     = nMvdHits;
    fnStsHits     = nStsHits;
    fPidHypothesis= pidHypo;
    fIP = IP;
    fImx = Imx;
    fImy = Imy;
}

// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmD0TrackCandidate::~CbmD0TrackCandidate(){}
// -------------------------------------------------------------------------

ClassImp(CbmD0TrackCandidate)
