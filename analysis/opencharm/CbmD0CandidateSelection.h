// -------------------------------------------------------------------------
// -----                CbmD0CandidateSelection  header file           -----
// -----                  Created 09/02/07  by C. Dritsa               -----
// -----                    Update 06/15  by P.Sitzmann                -----
// -------------------------------------------------------------------------


/**  CbmD0CandidateSelection.h
 *@author C.Dritsa <c.dritsa@gsi.de>
 * update 2015 P.Sitzmann <p.sitzmann@gsi.de>
 *
 *  Selection of candidate pairs
 *  for D0 reconstruction
 *
 **/




#ifndef CBMD0CANDIDATESELECTION_H
#define CBMD0CANDIDATESELECTION_H

#include "FairTask.h"
#include "CbmMCTrack.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "CbmD0TrackCandidate.h"
#include "TNtuple.h"
#include "FairField.h"


class TClonesArray;
class CbmKFSecondaryVertexFinder;
class CbmStsKFTrackFitter;
class CbmStsTrack;
class CbmVertex;
class FairTrackParam;
class CbmD0Tools;
class CbmKFParticleInterface;
class KFParticle;
class KFPVertex;


class CbmD0CandidateSelection : public FairTask
{
 public:
  CbmD0CandidateSelection();
  CbmD0CandidateSelection(char* name, Int_t iVerbose, Double_t cutIPD, Double_t cutSVZ);
  ~CbmD0CandidateSelection();
  InitStatus Init();
  void Exec(Option_t* option);
  
  void SetHistoFileName( TString name ){ fHistoFileName = name;  };
  Double_t SetKFParticle(Bool_t useKFparticle) { fUseKF = useKFparticle;};
  void SetTestMode ( Bool_t testMode ) { bTestMode = testMode; };


private:
    Int_t fEventNumber;
    TClonesArray* fKaonParticleArray;
    TClonesArray* fPionParticleArray;
    TClonesArray* fD0Candidates;
    TClonesArray* fListMCTracksPos;
    TClonesArray* fListMCTracksNeg;
    TClonesArray* fListMCTracks;
    TClonesArray* fListD0TrackCandidate;
    TClonesArray* fD0TrackArray;
    TClonesArray* fInfoArray;
    TClonesArray* fStsTrackMatches;
    TClonesArray* fPosStsTracks;
    TClonesArray* fNegStsTracks;
    CbmVertex* fPrimVtx;
    float fvtx[3];
    CbmD0Tools* tools;
    CbmKFParticleInterface* kfpInterface;
    FairField* fMF;
    Bool_t bTestMode;
    Bool_t f_particleIsMCD0;

    TString fHistoFileName;

    void CheckIfParticleIsD0(Int_t iNeg, Int_t iPos);
    Double_t GetPairImpactParameterR(KFParticle* particle );
    Double_t GetPairPt(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2);
    Double_t GetPairPz(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2);
    Double_t GetPairTx(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2);
    Double_t GetPairTy(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2);
    Double_t GetMomentum(CbmD0TrackCandidate* tr1);
    Double_t GetPairMomentum(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2);
    Double_t GetEnergy(CbmD0TrackCandidate* tr1);
    Double_t GetPairEnergy( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    Double_t GetIM(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2);
    Double_t GetCos12(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    Double_t GetIPAngle(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    Double_t GetChi2Topo(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    Double_t GetAPalpha(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    Double_t GetAPptt(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    TNtuple* fTrackPairNtuple;

    Double_t fcutIPD0, fcutSVZ;
    Bool_t fUseKF;

    void SetCuts(Double_t ipD0, Double_t SVZ);

    /** Register the output arrays to the IOManager **/
    void Register();
    void Finish();

  ClassDef(CbmD0CandidateSelection,1);
};

#endif
