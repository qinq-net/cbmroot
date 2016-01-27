// -------------------------------------------------------------------------
// -----                CbmD0CandidatesSE  header file                 -----
// -----               Created 09/02/07  by C. Dritsa                  -----
// -------------------------------------------------------------------------


/**  CbmD0CandidatesSE.h
 *  @authors C.Dritsa <c.dritsa@gsi.de>
 *  update 2015/16 P.Sitzmann <p.sitzmann@gsi.de>
 *  acknowledgements to M.Deveaux
 *  Selection of candidate pairs in SuperEvent Mode
 *  for D0 reconstruction
 *
 **/

#ifndef CBMD0CANDIDATESSE_H
#define CBMD0CANDIDATESSE_H

#include "FairTask.h"
#include "TString.h"
#include "CbmMapsFileManager.h"
#include "TLorentzVector.h"
#include "FairField.h"

class TClonesArray;
class TObjArray;
class KFParticle;
class CbmStsTrack;
class CbmVertex;

class CbmD0CandidatesSE : public FairTask
{
 public:
  CbmD0CandidatesSE();
  CbmD0CandidatesSE(char* name, Int_t iVerbose, Double_t cutIPD0, Double_t cutSVZ);
  virtual ~CbmD0CandidatesSE();
  virtual InitStatus Init();
  virtual void Exec(Option_t* option);

  void SetNegativeFileName( TString filename ){ fNegativeFileName = filename;  };
  void SetCuts( Double_t cutIPD0, Double_t cutSVZ){fcutIPD0 = cutIPD0; fcutSVZ = cutSVZ;};

private:

    Int_t fEventNumber;
    Double_t fcutIPD0, fcutSVZ;

    TClonesArray* fStsTrackMatches;
    TClonesArray* fListMCTracks;
    TClonesArray* fKaonParticleArray;
    TClonesArray* fPionParticleArray;
    TClonesArray* fListMCTracksPos;
    TClonesArray* fListMCTracksNeg;

    TClonesArray* fD0Candidates;
    TClonesArray* fKaonParticles;

    TObjArray* fKaonBuffer;
    CbmVertex* fPrimVtx;

    float fvtx[3];

    Int_t  fFrameWorkEvent;

    TString fNegativeFileName;
    Bool_t bTestMode;

    /** Register the output arrays to the IOManager **/
    void Register();
    void Finish();

    Double_t GetPairTx( TVector3& mom1, TVector3& mom2 );
    Double_t GetPairTy( TVector3& mom1, TVector3& mom2 );

    Double_t GetPairPt( TVector3& mom1, TVector3& mom2 );
    Double_t GetPairPz( TVector3& mom1, TVector3& mom2 );

    Double_t GetMomentum( TVector3& mom1 );
    Double_t GetPairMomentum( TVector3& mom1, TVector3& mom2);

    Double_t GetPairImpactParameterR(KFParticle* particle );

    Double_t GetCos12( TVector3& mom1, TVector3& mom2 );
    Double_t GetIPAngle( TVector3& mom1, TVector3& mom2 );
    Double_t GetChi2Topo( TVector3& mom1, TVector3& mom2 );
    void     GetAP( TVector3 &mom1, TVector3 &mom2, Double_t Qp, Double_t &alpha, Double_t &ptt );
    void     FillBuffer( CbmMapsFileManager* BackgroundFile, TObjArray* BackgroundArray );


    //Double_t BoostMomentum( TVector3& mom1, TVector3& mom2, CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    //Double_t GetEnergy1(CbmD0TrackCandidate* tr1);
    //Double_t GetEnergy2(CbmD0TrackCandidate* tr1);
    //Double_t GetPairEnergy( CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2 );
    //Double_t GetIM(CbmD0TrackCandidate* tr1, CbmD0TrackCandidate* tr2);


    CbmD0CandidatesSE(const CbmD0CandidatesSE&);
    CbmD0CandidatesSE& operator=(const CbmD0CandidatesSE&);

    ClassDef(CbmD0CandidatesSE,1);
};

#endif
