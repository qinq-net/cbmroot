// -------------------------------------------------------------------------
// -----              CbmD0TrackSelection  header file                 -----
// -----               Created 09/02/07  by C. Dritsa                  -----
// -------------------------------------------------------------------------


/**  CbmD0Candidates.h
 *@author C.Dritsa <c.dritsa@gsi.de>
 *
 *  Selection of candidate tracks
 *  for D0 reconstruction
 *
 **/


#ifndef CBMD0TRACKSELECTION_H
#define CBMD0TRACKSELECTION_H

#include "FairTask.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "CbmD0TrackCandidate.h"
#include "TNtuple.h"
#include "CbmMCTrack.h"
#include <iostream>

class TClonesArray;
class CbmL1PFFitter;
class CbmStsTrack;
class CbmGlobalTrack;
class CbmVertex;
class FairTrackParam;
class CbmKFParticleInterface;
class L1FieldRegion;
class KFPVertex;
class KFParticle;
class CbmMCTrack;

class CbmD0TrackSelection : public FairTask
{
 public:
  CbmD0TrackSelection();
  CbmD0TrackSelection(char* name, Int_t iVerbose, Double_t cutP, Double_t cutPt, Double_t cutPV, Double_t cutIP);
  virtual ~CbmD0TrackSelection();
  virtual InitStatus Init();
  virtual void Exec(Option_t* option);
  void CheckMvdMatch(CbmStsTrack* stsTrack,Int_t mcTrackIndex,Int_t&  goodMatch, Int_t&  badMatch);
  //Int_t CheckMvdMatch(CbmStsTrack* stsTrack,Int_t mcTrackIndex);

  Double_t GetImpactParameterRadius( KFParticle *particle);
  Double_t GetImpactParameterX( KFParticle *particle);
  Double_t GetImpactParameterY( KFParticle *particle);

 // Double_t GetTransverseMom(FairTrackParam *etrack);
 // Double_t GetMom(FairTrackParam *etrack);

  void SetHistoFileName( TString name ){ fHistoFileName = name;  };
  void SetNHitsOfLongTracks(Int_t N) {fNHitsOfLongTracks=N;};
  void ShowADI(void) {fadi = kTRUE;};
  void SetPIDMode(TString pidMode);
  void SaveCutEff(void);
  void SetCutEffLogfile(TString filename)                {logfile = filename;};
 
  void SetUseMcInfo(Bool_t useMC) { bUseMCInfo = useMC;};
  void SetCuts(Double_t p, Double_t pt, Double_t PVsigma, Double_t IP);
  /** Virtual method Finish **/
  virtual void Finish();

private:

    void ClearArrays();
    Int_t GetPid(CbmGlobalTrack* globalTrack);
    void KMinusRefit(CbmStsTrack* stsTrack);
    CbmMCTrack*  GetMCTrackFromTrackID(Int_t ID);

  Int_t fNHitsOfLongTracks;
  Int_t fEventNumber;
  TClonesArray* fMcPoints;
  TClonesArray* fStsTrackArray;
  TClonesArray* fGlobalTracks;
  TClonesArray* fTrdTracks;
  TClonesArray* fTofHits;
  TClonesArray* fRichRings;
  TClonesArray* fStsTrackArrayP;
  TClonesArray* fStsTrackArrayN;
  TClonesArray* fMCTrackArrayP;
  TClonesArray* fMCTrackArrayN;
  TClonesArray* fInfoArray;
  TClonesArray* fStsTrackMatches;
  TClonesArray* fKaonParticleArray;
  TClonesArray* fPionParticleArray;
  TClonesArray* fListMCTracks;
  TClonesArray* fMvdHitMatchArray;
  TClonesArray* fKFParticleArray;

  CbmKFParticleInterface* kfpInterface;

  TString fPidMode;

  CbmL1PFFitter* fFit;
  CbmVertex* fPrimVtx;
  CbmVertex* fSecVtx;

  KFPVertex* fPrimVtxKF;

  L1FieldRegion* fField;

  TString fHistoFileName;

  Bool_t bUseMCInfo;
  Bool_t fadi;
  TString logfile;
	


  Int_t fPVCutPassed;
  Int_t fPVCutNotPassed;
  Int_t fNoHPassed;
  Int_t fNoHNotPassed;
  
  Double_t fCutPt, fCutP, fCutPV, fCutIP;

  ClassDef(CbmD0TrackSelection,1);
};

#endif
