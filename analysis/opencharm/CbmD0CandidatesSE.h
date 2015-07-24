// -------------------------------------------------------------------------
// -----                CbmD0Candidates  header file                   -----
// -----               Created 09/02/07  by C. Dritsa                  -----
// -------------------------------------------------------------------------


/**  CbmD0Candidates.h
 *@authors C.Dritsa <c.dritsa@gsi.de>
 *acknowledgements to M.Deveaux
 *  Selection of candidate pairs
 *  for D0 reconstruction
 *
 **/




#ifndef CBMD0CANDIDATESSE_H
#define CBMD0CANDIDATESSE_H

#include "FairTask.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "CbmD0TrackCandidate.h"
#include "TNtuple.h"
#include "CbmMapsFileManager.h"
#include "TLorentzVector.h"
#include "FairField.h"

class TClonesArray;
class TObjArray;
class CbmStsKFSecondaryVertexFinder;
class CbmStsKFTrackFitter;
class CbmStsTrack;
class CbmVertex;
class FairTrackParam;



class CbmD0CandidatesSE : public FairTask
{
 public:
  CbmD0CandidatesSE();
  CbmD0CandidatesSE(char* name, Int_t iVerbose, Bool_t SuperEventMode, Double_t cutIPD0max, Double_t cutSVZmin, Double_t cutSVZmax);
  virtual ~CbmD0CandidatesSE();
  virtual InitStatus Init();
  virtual void Exec(Option_t* option);
  
  /*
  virtual void KminusReFit(CbmStsTrack* track);
  Double_t GetImpactParameterRadius(CbmTrackParam *etrack);
  Double_t GetImpactParameterX(CbmTrackParam *etrack);
  Double_t GetImpactParameterY(CbmTrackParam *etrack);
  Double_t GetTransverseMom(CbmTrackParam *etrack);
  Double_t GetMom(CbmTrackParam *etrack);
  //void SetParContainers() {; };
  */
  void SetHistoFileName( TString name ){ fHistoFileName = name;  };
  void SetNegativeFileName( TString filename ){ fNegativeFileName = filename;  };

private:
    Int_t fEventNumber;
    TClonesArray* fListRCD0;

    TClonesArray* fListMCTracks;
    TClonesArray* fPosMCTrackArray;
    TClonesArray* fNegMCTrackArray;

    TClonesArray* fListD0TrackCandidate;
    TClonesArray* fAllD0TrackArray;

    TClonesArray* fStsTrackArray;
    TClonesArray* fPosStsTrackArray;
    TClonesArray* fNegStsTrackArray;
    TClonesArray* fD0TrackArray;
    TClonesArray* fPosD0TrackArray;
    TClonesArray* fNegD0TrackArray;

    TClonesArray* fInfoArray;
    TClonesArray* fStsTrackMatches;
    TClonesArray* fPairs;

    TObjArray* fBufferStsTrackArraysN;
    TObjArray* fBufferMCTrackArraysN;
    TObjArray* fBufferD0TrackArraysN;

    CbmStsKFTrackFitter* fFit;
    CbmStsKFSecondaryVertexFinder* fSVF;
    CbmVertex* fPrimVtx;
    CbmVertex* fSecVtx;
    FairField* fMF;

    TString fHistoFileName;
    TString fNegativeFileName;

    Int_t  fFrameWorkEvent;
    Bool_t fSuperEventMode;

    Double_t GetPairTx( TVector3& mom1, TVector3& mom2 );
    Double_t GetPairTy( TVector3& mom1, TVector3& mom2 );

    Double_t GetPairPt( TVector3& mom1, TVector3& mom2 );
    Double_t GetPairPz( TVector3& mom1, TVector3& mom2 );

    Double_t GetMomentum( TVector3& mom1 );
    Double_t GetPairMomentum( TVector3& mom1, TVector3& mom2);

    Double_t GetPairImpactParameterR( Double_t Pair_Tx, Double_t Pair_Ty, Double_t SvX, Double_t SvY, Double_t SvZ, Double_t PvZ );
    Double_t GetEnergy1(CbmD0TrackCandidate* tr1);
    Double_t GetEnergy2(CbmD0TrackCandidate* tr1);
    Double_t GetPairEnergy( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    Double_t GetIM(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2);

    Double_t GetCos12( TVector3& mom1, TVector3& mom2 );
    Double_t GetIPAngle( TVector3& mom1, TVector3& mom2 );
    Double_t GetChi2Topo( TVector3& mom1, TVector3& mom2 );
    void     GetAP( TVector3 &mom1, TVector3 &mom2, Double_t Qp, Double_t &alpha, Double_t &ptt );
    void     FillBuffer( CbmMapsFileManager* BackgroundFile, TObjArray* BackgroundArray );
    Double_t BoostMomentum( TVector3& mom1, TVector3& mom2, CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    TNtuple* fTrackPairNtuple;

    TH1F* testN;
    TH1F* testP;

    Double_t fcutIPD0max,  fcutSVZmin,  fcutSVZmax;
    void SetCuts( Double_t ipD0max, Double_t SVZmin, Double_t SVZmax);

    /** Register the output arrays to the IOManager **/
    void Register();
    void Finish();

    ClassDef(CbmD0CandidatesSE,1);
};

#endif
