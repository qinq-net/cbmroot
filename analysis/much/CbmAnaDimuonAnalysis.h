
/** CbmAnaDimuonAnalysis.h
 *@author E.Kryshen <e.kryshen@gsi.de>
 *@since 2010-01-02
 **
 **/

#ifndef CBMANADIMUONANALYSIS_H_
#define CBMANADIMUONANALYSIS_H_ 1

#include "FairTask.h"
#include "TString.h"
#include <map>

class CbmAnaDimuonAnalysis;
class CbmAnaMuonCandidate;
class TClonesArray;
class CbmTrackMatch;
class TH1D;
class TH2D;
class TH3D;
class TLorentzVector;
class CbmMuchTrack;
class CbmTrdTrack;
class CbmTofHit;
class CbmMuchGeoScheme;
class CbmStsKFTrackFitter;
class CbmVertex;
using std::map;

class CbmAnaDimuonAnalysis : public FairTask{
public:
  CbmAnaDimuonAnalysis(TString digiFileName, Int_t nSignalPairs);
  virtual ~CbmAnaDimuonAnalysis(){}
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void Finish();
  virtual void SetParContainers();

  // Set STS acceptance criteria as a number of STS points
  void SetStsPointsAccQuota(Int_t nPoints)  { fStsPointsAccQuota = nPoints; }
  void SetMuchPointsAccQuota(Int_t nPoints) { fMuchPointsAccQuota = nPoints; }
  void SetStsTrueHitQuota(Double_t quota)   { fStsTrueHitQuota = quota; }
  void SetMuchTrueHitQuota(Double_t quota)  { fMuchTrueHitQuota = quota; }
  void IsTriggerEnabled(Bool_t isTriggerEnabled) {fIsTriggerEnabled = isTriggerEnabled; }
  Int_t GetMCTrackId(Int_t iMuchTrack);

  void SetChi2MuChCut(Double_t cut)  { fChi2MuchCut = cut; }
  void SetChi2VertexCut(Double_t cut)  { fChi2VertexCut = cut; }
  void SetNofMuchCut(Int_t cut)  { fNofMuchCut = cut; }
  void SetNofStsCut(Int_t cut)  { fNofStsCut = cut; }
  //  void SetNofTRDCut(Int_t cut)  { fNofTRDCut = cut; }
  void SetTriggerStationNo(Int_t stnlast) {fTriggerStationIndex = stnlast;}
private:
  CbmAnaMuonCandidate* GetMu(Int_t trackId);
  void DigiToTrackMatch(const TClonesArray* digiMatches,Int_t digiIndex,std::map<Int_t, Int_t> &matchMap);

private:
  Int_t         fEvent;      //!  
  Int_t          fmuon ;              //!
  Int_t          fmuonprim ;              //!
  TClonesArray* fMCTracks;          //!
  TClonesArray* fStsTracks;         //!
  TClonesArray* fStsPoints;         //!
  TClonesArray* fMuchPoints;        //!
  TClonesArray* fMuchPixelHits;     //!
  TClonesArray* fMuchStrawHits;     //!
  TClonesArray* fMuchTracks;        //!
  TClonesArray* fTRDTracks;         //!
  TClonesArray* fTRDPoints;
  TClonesArray* fTRDTrackMatches;
  TClonesArray* fMuchTrackMatches;  //!
  TClonesArray* fStsTrackMatches;   //!
  TClonesArray* fMuCandidates;      //!
  TClonesArray* fDimuonCandidates;  //!
  TClonesArray* fGlobalTracks;      //!
  TClonesArray* fPixelDigiMatches;
  TClonesArray* fStrawDigiMatches;
  TClonesArray* fClusters;
  TClonesArray* fTofHit;
  TClonesArray* fTRDHit;
  CbmStsKFTrackFitter* fFitter;
  CbmVertex* fVertex;
  
  Int_t    fStsPointsAccQuota;
  Double_t fStsTrueHitQuota;
  Int_t    fMuchPointsAccQuota;
  Double_t fMuchTrueHitQuota;
  TString  fHistoFileName;
  TString  fDigiFileName;
  Int_t fTriggerStationIndex;
  Int_t fNLayers;   
  Int_t fNGhostSTS;   
  Int_t fNGhostMUCHreco;   


  Double_t fChi2MuchCut; 
  Double_t fChi2VertexCut; 
  Int_t fNofMuchCut; 
  Int_t fNofStsCut;
  //Int_t fNofTRDCut;

  TH1D* fHchi2STS;
  TH1D* fHchi2STSmu;
  TH2D* fP_chi2Vertex;
  TH1D* fChi2;
  TH1D* fChi2refit;
  
  map<Int_t,Int_t> mapRecSts;       //!
  map<Int_t,Int_t> mapRecMuch;      //!

  map<Int_t,Int_t> mapAllMuPlus;    //!
  map<Int_t,Int_t> mapAccMuPlus;    //!
  map<Int_t,Int_t> mapRecMuPlus;    //!
  map<Int_t,Int_t> mapAllMuMinus;   //!
  map<Int_t,Int_t> mapAccMuMinus;   //!
  map<Int_t,Int_t> mapRecMuMinus;   //!

  CbmMuchGeoScheme* fGeoScheme;     //!
  Int_t fSignalPairs;               //!
  Bool_t fIsTriggerEnabled; 
  
  ClassDef(CbmAnaDimuonAnalysis,2);
};

#endif
