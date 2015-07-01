// -----------------------------------------------------------------------
// -----               CbmTrackCandidate source file                 -----
// -----         Created 23/01/07  by C. Dritsa and M. Deveaux       -----
// -----------------------------------------------------------------------



#include "CbmD0TrackCandidate.h"


// -----   Default constructor   -------------------------------------------
CbmD0TrackCandidate::CbmD0TrackCandidate() : FairTrackParam() {
    SetX(0); SetY(0); SetZ(0); SetTx(0); SetTy(0); SetQp(0);
    for (int i=0; i<4; i++){
	for (int j=0; j<4; j++){
	    SetCovariance(i,j,0);
	}
    }

    fPVSigma = 0;
    fTrackIndex = fTrackIndexMC = -1;

}
// -------------------------------------------------------------------------



// -----   Constructor with parameters   -----------------------------------
CbmD0TrackCandidate::CbmD0TrackCandidate( FairTrackParam* e_track, Double_t PVSigma,
					 Int_t trackIndex ,Int_t mcTrackIndex, Int_t nMvdHits, Int_t nStsHits, Int_t pidHypo )
 :FairTrackParam(*e_track)
{

    fTrackIndex   = trackIndex;
    fTrackIndexMC = mcTrackIndex;
    fPVSigma      = PVSigma;
    fnMvdHits     = nMvdHits;
    fnStsHits     = nStsHits;
    fPidHypothesis= pidHypo;
}

// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmD0TrackCandidate::~CbmD0TrackCandidate(){}
// -------------------------------------------------------------------------

ClassImp(CbmD0TrackCandidate)
