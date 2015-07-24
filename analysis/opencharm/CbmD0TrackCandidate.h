// -------------------------------------------------------------------------
// -----                     CbmD0TrackCandidate header file           -----
// -----                  Created 23/01/07  by C.Dritsa and M.Deveaux  -----
// -------------------------------------------------------------------------



#ifndef CBMD0TRACKCANDIDATE
#define CBMD0TRACKCANDIDATE 1


#include "TMatrixFSym.h"
#include "TVector3.h"
#include "FairTrackParam.h"


class CbmD0TrackCandidate : public FairTrackParam

{

public:

 /** Default constructor. Do not use it **/
CbmD0TrackCandidate();

/** Constructor with all variables **/

CbmD0TrackCandidate(FairTrackParam* e_track, Double_t PVSigma, Int_t trackIndex, Int_t mcTrackIndex = -1, Int_t nMvdHits = 0, Int_t nStsHits = 4, Int_t pidHypo = 211);

/** Destructor **/
virtual ~CbmD0TrackCandidate();

/** Getters **/

Double_t GetPVSigma()            { return fPVSigma;      };
Int_t    GetTrackIndex()         { return fTrackIndex;   };
Int_t    GetTrackIndexMC()       { return fTrackIndexMC; };
Int_t    GetNMvdHits()           { return fnMvdHits;     };
Int_t    GetNStsHits()           { return fnStsHits;     };
Int_t    GetPidHypo()		 { return fPidHypothesis;};

void  SetPVSigma( Double_t S ) {  fPVSigma = S ; };
void  SetTrackIndex( Int_t TrIndex ) { fTrackIndex = TrIndex ; };
void  SetTrackIndexMC( Int_t TrIndexMC ) { fTrackIndexMC = TrIndexMC; };



private:

Int_t    fTrackIndex;   // index of the corresponding CbmStsTrack
Int_t    fTrackIndexMC; // index of the corresponding MC-Track
Double_t fPVSigma;      // The PV-Sigma, which was generated from the fit of the track
Int_t    fnMvdHits;
Int_t    fnStsHits;
Int_t    fPidHypothesis;


ClassDef(CbmD0TrackCandidate,1);
};


#endif
