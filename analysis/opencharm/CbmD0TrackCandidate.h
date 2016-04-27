// -------------------------------------------------------------------------
// -----                     CbmD0TrackCandidate header file           -----
// -----                  Created 23/01/07  by C.Dritsa and M.Deveaux  -----
// -----                        updated 2016 by P.Sitzmann             -----
// -------------------------------------------------------------------------



#ifndef CBMD0TRACKCANDIDATE
#define CBMD0TRACKCANDIDATE 1

#include "TObject.h"

class CbmD0TrackCandidate : public TObject

{

public:

 /** Default constructor. Do not use it **/
CbmD0TrackCandidate();

/** Constructor with all variables **/

CbmD0TrackCandidate(Int_t globalTrackId , Int_t matchedTrackId, Double_t PVChi2, Double_t IP, Double_t Imx, Double_t Imy, Int_t nMvdHits = 0, Int_t nStsHits = 4, Int_t pidHypo = 211);

/** Destructor **/
virtual ~CbmD0TrackCandidate();

/** Getters **/

Double_t GetPVChi2()            { return fPVChi2;      };
Int_t    GetTrackIndex()         { return fTrackIndex;   };
Int_t    GetTrackIndexMC()       { return fTrackIndexMC; };
Int_t    GetNMvdHits()           { return fnMvdHits;     };
Int_t    GetNStsHits()           { return fnStsHits;     };
Int_t    GetPidHypo()		 { return fPidHypothesis;};
Double_t GetIP()                 { return fIP;};
Double_t GetImx()                { return fImx;};
Double_t GetImy()                { return fImy;};

void  SetPVChi2( Double_t S ) {  fPVChi2 = S ; };
void  SetTrackIndex( Int_t TrIndex ) { fTrackIndex = TrIndex ; };
void  SetTrackIndexMC( Int_t TrIndexMC ) { fTrackIndexMC = TrIndexMC; };



private:

Int_t    fTrackIndex;   // index of the corresponding CbmStsTrack
Int_t    fTrackIndexMC; // index of the corresponding MC-Track
Double_t fPVChi2;      // The PV-Chi2, which was generated from the fit of the track
Int_t    fnMvdHits;
Int_t    fnStsHits;
Int_t    fPidHypothesis;
Double_t fIP;
Double_t fImx;
Double_t fImy;


ClassDef(CbmD0TrackCandidate,1);
};


#endif
