// -------------------------------------------------------------------------
// -----              CbmMvdQa  header file                            -----
// -----              Created 12/01/15  by P. Sitzmann                 -----
// -------------------------------------------------------------------------


/**  CbmMvdQa.h
 *@author P.Sitzmann <p.sitzmann@gsi.de>
 *
 *  Selection of Qa functions for the mvd
 *
 **/


#ifndef CBMMVDQA_H
#define CBMMVDQA_H 1

#include "FairTask.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH3F.h"
#include "TString.h"
#include "TNtuple.h"
#include <iostream>
#include "TVector3.h"
#include "TClonesArray.h"
#include "CbmTrackMatchNew.h"


class CbmStsTrack;


class TClonesArray;
class CbmStsTrack;
class CbmVertex;
class FairTrackParam;
class CbmMvdDetector;

class CbmLitTrackParam;

class CbmStsTrack;

class CbmMvdQa : public FairTask
{
public:
  CbmMvdQa();
  
  CbmMvdQa(const char* name, Int_t iMode = 0, Int_t iVerbose = 0);
 
  ~CbmMvdQa();

  void     SetMinHitReq(Int_t nrOfHits){fminHitReq = nrOfHits;};
  void     SetMatches(Int_t trackID, CbmStsTrack* stsTrack);
  void     Exec(Option_t* opt); 

  InitStatus Init();

  void Finish();
   
  void SetUseMcQa(){useMcQa = kTRUE;};
  void SetUseDigiQa(){useDigiQa = kTRUE;};
  void SetUseHitQa(){useHitQa = kTRUE;};
  void SetUseTrackQa(){useTrackQa = kTRUE;};

private:

  Int_t fNHitsOfLongTracks;
  Int_t fEventNumber;
  Int_t fminHitReq;
  Float_t fMvdRecoRatio;
  Int_t fBadTrack;
  Int_t fUsedTracks;
  Int_t fnrOfMergedHits;

  TClonesArray* fStsTrackArray;
  TClonesArray* fStsTrackArrayP;
  TClonesArray* fStsTrackArrayN;
  TClonesArray* fStsTrackMatches;
  TClonesArray* fGlobalTrackArray;
 

  TClonesArray* fListMCTracks;
  TClonesArray* fMCTrackArrayP;
  TClonesArray* fMCTrackArrayN;

  TClonesArray* fMcPoints;
  TClonesArray* fMvdDigis;
  TClonesArray* fMvdCluster;
  TClonesArray* fMvdHits;
  TClonesArray* fMvdHitMatchArray;
  TClonesArray* fBadTracks;

  TClonesArray* fInfoArray;
  TH2F* fMvdResHistoX;
  TH2F* fMvdResHistoR;
  TH2F* fMvdResHistoY;
  TH2F* fMvdResHistoXY;
  TH1F* fMatchingEffiHisto;

  TH2F* fMvdDigiDist1;
  TH2F* fMvdDigiDist2;
  TH2F* fMvdDigiDist3;
  TH2F* fMvdDigiDist4;
  TH2F* fMvdDigiWorst;
  TH2F* fMvdHitWorst;
  TH2F* fMvdMCWorst;
  TH2F* fMvdMCWorstDelta;
  TH2F* fMvdMCBank[63];
  TH2F* fMvdMCHitsStations[4];
  TH1F* fWordsPerSuperRegion;
  TH1F* fWorstSuperPerEvent;

  TH2I* fMvdBankDist;


  TH1F* fMvdTrackQa1F[20];
  TH2F* fMvdTrackQa2F[20];

  CbmVertex* fPrimVtx;
  CbmVertex* fSecVtx;

  CbmMvdDetector* fDetector;

  Bool_t useMcQa;
  Bool_t useDigiQa;
  Bool_t useHitQa;
  Bool_t useTrackQa;

 // Double_t GetImpactParameterRadius(CbmLitTrackParam etrack);
 // Double_t GetImpactParameterX(CbmLitTrackParam etrack);
 // Double_t GetImpactParameterY(CbmLitTrackParam etrack);

 // Double_t GetTransverseMomentum(CbmLitTrackParam etrack);
 // Double_t GetMomentum(CbmLitTrackParam etrack);
 // Double_t GetMomentumZ(CbmLitTrackParam etrack);
 // Double_t GetAngle(CbmLitTrackParam etrack);

  void SetupHistograms();

  void ExecDigiQa();
  void ExecHitQa();
  void ExecMCQa();
  void ExecTrackQa();

  void FinishMCQa();
  void FinishDigiQa();
  void FinishHitQa();
  void FinishTrackQa();

  CbmMvdQa(const CbmMvdQa&);
  CbmMvdQa& operator=(const CbmMvdQa&);

 ClassDef(CbmMvdQa,1);
};


#endif
