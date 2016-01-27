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
#include "TNtuple.h"
#include "CbmMCTrack.h"

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
  CbmD0TrackSelection(char* name, Int_t iVerbose, Double_t cutP, Double_t cutPt, Double_t cutChi2, Double_t cutIP);
  virtual ~CbmD0TrackSelection();
  virtual InitStatus Init();
  virtual void Exec(Option_t* option);

  void SetNHitsOfLongTracks(Int_t N) {fNHitsOfLongTracks=N;};
  void SetPIDMode(TString pidMode);

  void SetUseMcInfo(Bool_t useMC) { bUseMCInfo = useMC;};
  void SetCuts(Double_t p, Double_t pt, Double_t PVsigma, Double_t IP);
  /** Virtual method Finish **/
  virtual void Finish();

private:


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

  float fvtx[3];

  CbmKFParticleInterface* kfpInterface;

  TString fPidMode;

  CbmL1PFFitter* fFit;
  CbmVertex* fPrimVtx;   
  L1FieldRegion* fField;

  Bool_t bUseMCInfo;
  Int_t fPVCutPassed;
  Int_t fPVCutNotPassed;
  Int_t fNoHPassed;
  Int_t fNoHNotPassed;
  
  Double_t fCutPt, fCutP, fCutChi2, fCutIP;


    void ClearArrays();
    Int_t GetPid(CbmGlobalTrack* globalTrack);
    void KMinusRefit(CbmStsTrack* stsTrack);
    CbmMCTrack*  GetMCTrackFromTrackID(Int_t ID);
      Double_t GetImpactParameterRadius( KFParticle *particle);
  Double_t GetImpactParameterX( KFParticle *particle);
  Double_t GetImpactParameterY( KFParticle *particle);
   void CheckMvdMatch(CbmStsTrack* stsTrack,Int_t mcTrackIndex,Int_t&  goodMatch, Int_t&  badMatch);

   CbmD0TrackSelection(const CbmD0TrackSelection&);
   CbmD0TrackSelection& operator=(const CbmD0TrackSelection&);


  ClassDef(CbmD0TrackSelection,1);
};

#endif
