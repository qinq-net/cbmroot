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
// class CbmStsKFSecondaryVertexFinder;
class CbmStsKFTrackFitter;
class CbmStsTrack;
class CbmVertex;
class FairTrackParam;

class CbmD0TrackSelection : public FairTask
{
 public:
  CbmD0TrackSelection();
  CbmD0TrackSelection(char* name, Int_t iVerbose, Double_t cutP, Double_t cutPt, Double_t cutPV, Double_t cutIP);
  virtual ~CbmD0TrackSelection();
  virtual InitStatus Init();
  virtual void Exec(Option_t* option);
  void KminusReFit(CbmStsTrack* track);
  void CheckMvdMatch(CbmStsTrack* stsTrack,Int_t mcTrackIndex,Int_t&  goodMatch, Int_t&  badMatch);
  //Int_t CheckMvdMatch(CbmStsTrack* stsTrack,Int_t mcTrackIndex);

  Double_t GetImpactParameterRadius(FairTrackParam *etrack);
  Double_t GetImpactParameterX(FairTrackParam *etrack);
  Double_t GetImpactParameterY(FairTrackParam *etrack);

  Double_t GetTransverseMom(FairTrackParam *etrack);
  Double_t GetMom(FairTrackParam *etrack);
  Bool_t   IsLong(CbmStsTrack* track);

  //void SetParContainers() {; };

  void ShowDebugHistos (){fShowDebugHistos= kTRUE;}
  void SetHistoFileName( TString name ){ fHistoFileName = name;  };
  void SetNHitsOfLongTracks(Int_t N) {fNHitsOfLongTracks=N;};
  void ShowADI(void) {fadi = kTRUE;};
  void SetPIDMode(TString pidMode);
  void SaveCutEff(void);
  void SetCutEffLogfile(TString filename)                {logfile = filename;};
  void FillDebugHistos(CbmStsTrack* stsTrack, CbmMCTrack* mcTrack, Int_t nrOfBadMatch, Int_t nrOfGoodMatch);
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
  TClonesArray* fTofTracks;
  TClonesArray* fRichRings;
  TClonesArray* fStsTrackArrayP;
  TClonesArray* fStsTrackArrayN;
  TClonesArray* fMCTrackArrayP;
  TClonesArray* fMCTrackArrayN;
  TClonesArray* fInfoArray;
  TClonesArray* fStsTrackMatches;
  TClonesArray* fListRCD0;
  TClonesArray* fListMCTracks;
  TClonesArray* fListD0TrackCandidate;
  TClonesArray* fListD0TrackCandidateP;
  TClonesArray* fListD0TrackCandidateN;
  TClonesArray* fMvdHitMatchArray;

  TString fPidMode;

  CbmStsKFTrackFitter* fFit;
  CbmVertex* fPrimVtx;
  CbmVertex* fSecVtx;
  TString fHistoFileName;

  Bool_t fShowDebugHistos;
  Bool_t bUseMCInfo;
  Bool_t fadi;
  TString logfile;
	
  TNtuple* fTrackAnalysisTuple;

  Int_t fPVCutPassed;
  Int_t fPVCutNotPassed;
  Int_t fNoHPassed;
  Int_t fNoHNotPassed;
  
  Double_t fCutPt, fCutP, fCutPV, fCutIP;

  TH1F* PVnTracks;
  TH1F* PVZ;

  TH1F*  fpZMcHisto;
  TH1F*  fpZRecoHisto;
  TH1F*  fpErrorHisto;
  TH1F*  fpZErrorHisto;
  TH1F*  fptErrorHisto;

  TH1F*  ptHisto;
  TH1F*  imrHisto;
  TH1F*  QpHisto;
  TH2F*  fMvdEfficiencyHisto;
  TH1F*  fPVSigmaHisto;
  TH1F*  fIPHisto;
  TH1F*  fIPPVHisto;
  TH1F*  fIPRes;

  TH1F*  fIPResMergedHits;
  TH1F*  fIPResMergedHitsX;
  TH1F*  fIPResMergedHitsY;
  TH1F*  fIPResCorrectHits;
  TH1F*  fIPResCorrectHitsX;
  TH1F*  fIPResCorrectHitsY;

  TH2F*  fpErrorHisto_vs_mom;
  TH1F*  fpErrorHistoPcut;
  TH1F*  fIPResMergedHitsMom;
  TH1F*  fIPResMergedHitsXMom;
  TH1F*  fIPResMergedHitsYMom;
  TH1F*  fIPResCorrectHitsMom;
  TH1F*  fIPResCorrectHitsXMom;
  TH1F*  fIPResCorrectHitsYMom;

  TH1F*  fIPResAllHitsXMom;
  TH1F*  fIPResAllHitsYMom;
  TH1F*  fIPResAllHitsX;
  TH1F*  fIPResAllHitsY;
                            
  TH2F*  fIPResMergedHitsVsMom;
  TH2F*  fIPResCorrectHitsVsMom;
                            
  TH1F* fpErrorHistoCorr;
  TH1F* fpErrorHistoCorrMom;
  TH1F* fpErrorHistoMerg;
  TH1F* fpErrorHistoMergMom;


  TH1F* fIPMergedHitsSig;
  TH1F* fIPMergedHitsSigX;
  TH1F* fIPMergedHitsSigY;

  TH1F* fIPCorrHitsSig;
  TH1F* fIPCorrHitsSigX;
  TH1F* fIPCorrHitsSigY;

  TH1F* fIPAllHitsSigX;
  TH1F* fIPAllHitsSigY;



TH1F* fIPResMissing1HitsX;
TH1F* fIPResMissing2HitsX;
TH1F* fIPResWrong1HitsX;
TH1F* fIPResWrong2HitsX;
TH1F* fIPResMissing1HitsY;
TH1F* fIPResMissing2HitsY;
TH1F* fIPResWrong1HitsY;
TH1F* fIPResWrong2HitsY;



                 
  TH1F* fPVMergedHits;
  TH1F* fPVCorrectHits;

                     
  TH1F*  fPVMergedHitsSig;
  TH2F*  fPVMergedHitsSigP;
  TH1F*  fPVMergedHitsBg;
  TH2F*  fPVMergedHitsBgP;
  TH1F*  fPVCorrectHitsSig;
  TH2F*  fPVCorrectHitsSigP;
  TH1F*  fPVCorrectHitsBg;
  TH2F*  fPVCorrectHitsBgP;
  TH1F*  fPVAllHitsBg;
  
  ClassDef(CbmD0TrackSelection,1);
};

#endif
