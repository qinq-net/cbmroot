#ifndef CBMANAMUONCANDIDATE_H
#define CBMANAMUONCANDIDATE_H

#include "CbmKFTrack.h"
#include "TLorentzVector.h"
#include "TH1D.h"
#define NPLANES 31

class CbmAnaMuonCandidate : public TObject{
  public:
    CbmAnaMuonCandidate();
    CbmAnaMuonCandidate(Int_t iTrack, TLorentzVector pMC);
    virtual ~CbmAnaMuonCandidate(){};
    Int_t* GetMuchPoints() { return muchPoints; };
    Int_t* GetMuchHits()   { return muchHits;   };
    void SetMuchPoint(Int_t planeId, Int_t index) { muchPoints[planeId]=index; }
    void SetMuchHit  (Int_t planeId, Int_t index) { muchHits[planeId]=index;   }

    void SetBeta  (Int_t index, Double_t beta) { fBeta[index]=beta;   }
    Double_t GetBeta  (Int_t index) {return fBeta[index];   }

    void SetMomentumMC(TLorentzVector mom) {fpMC = TLorentzVector(mom); }
    void SetMomentumRC(TLorentzVector mom) {fpRC = TLorentzVector(mom); }
    void SetMomentumRC(Double_t* T);

    void SetMCTrackId(Int_t trackId) {fMCTrackId    = trackId; }
    void SetMCSTSTrackId(Int_t trackIdSTS) {fMCTrackIdSTS    = trackIdSTS; }
    void SetMCMuchTrackId(Int_t trackIdMuCh) {fMCTrackIdMuCh    = trackIdMuCh; }
    void SetSTSTrackId(Int_t trackIdSTS) {fTrackIdSTS    = trackIdSTS; }
    void SetMuchTrackId(Int_t trackIdMuCh) {fTrackIdMuCh    = trackIdMuCh; }
    
    void SetMCMuchPdg(Int_t MuchPdg) {fMCMuchPdg    = MuchPdg; }
    void SetMCStsPdg(Int_t StsPdg) {fMCStsPdg    = StsPdg; }
    void SetMotherPdg(Int_t motherPdg) {fMotherPdg    = motherPdg; }

    void SetTrueMu(Int_t trueMu) {ftrueMu    = trueMu; }
    void SetNStsHits(Int_t nStsHits) {fNStsHits     = nStsHits; }
    void SetNMuchHits(Int_t nMuchHits) {fNMuchHits     = nMuchHits; }
    void SetChiToVertex(Double_t chi) {fChiToVertex = chi; }
    void SetChiMuch(Double_t chiM) {fChiMuch = chiM; }
    void SetChiSTS(Double_t chiSTS) {fChiSTS=chiSTS;}

    void SetTRDTrackId(Int_t trdtrackid) {TRDtrackid = trdtrackid; }
    void SetNTRDHits(Int_t ntrdhits) {TRDhits = ntrdhits; }
    void SetChiTRD(Double_t chi2trd) {TRDchi2=chi2trd;}
    
    void SetNTriggerHits(Int_t n) { fNTriggerHits = n; }
    void SetSign(Double_t sign) {fSign = sign; }

    TLorentzVector* GetMomentumMC() {return &fpMC; }
    TLorentzVector* GetMomentumRC() {return &fpRC; }

    Bool_t IsReconstructed(Int_t nMuchHitsCut=11, Int_t nStsHitsCut=7, Double_t chiToVertexCut=3. , Int_t nTRDHitsCut=2);

    void SetReconstructed(Bool_t isRec) {fIsReconstructed = isRec; }
    Double_t GetSign() {return fSign;}
    void SetNMuchPoints(Int_t nPoints) {fNMuchPoints = nPoints; }
    void SetNStsPoints(Int_t nPoints) {fNStsPoints = nPoints; }

    void SetTofM(Double_t m) {fTofM = m; }

    Int_t GetNMuchPoints() { return fNMuchPoints; }
    Int_t GetNStsPoints() { return fNStsPoints; }
    Int_t GetNMuchHits() { return fNMuchHits; }
    Int_t GetNStsHits() { return fNStsHits; }

    Double_t GetChiMuch() {return fChiMuch; }
    Double_t GetChiToVertex() {return fChiToVertex; }
    Double_t GetChiSTS() {return fChiSTS;}

    Int_t GetMCStsPdg() {return fMCStsPdg; }
    Int_t GetMCMuchPdg() {return fMCMuchPdg; }
    Int_t GetMotherPdg() {return fMotherPdg; }
    Int_t GetTrueMu() {return ftrueMu; }

    Int_t GetMCSTSTrackId() {return fMCTrackIdSTS; }
    Int_t GetMCMuChTrackId() {return fMCTrackIdMuCh; }
    Int_t GetSTSTrackId() {return fTrackIdSTS; }
    Int_t GetMuChTrackId() {return fTrackIdMuCh; }

    Int_t GetNTriggerHits() {return fNTriggerHits; }

    Int_t GetNTRDHits() { return TRDhits; }
    Double_t GetChiTRD() {return TRDchi2 ;}
    Int_t GetTrdTrackId() {return TRDtrackid; }
    
    
    Double_t GetTofM() {return fTofM; }

  private:
    CbmKFTrack track;
    Int_t fMCTrackId;
    Int_t fMCTrackIdSTS;
    Int_t fMCTrackIdMuCh;
    Int_t fTrackIdSTS;
    Int_t fTrackIdMuCh;
    
    Int_t fMCStsPdg;
    Int_t fMCMuchPdg;
    Int_t fMotherPdg;

    Int_t ftrueMu;
    TLorentzVector fpMC;
    TLorentzVector fpRC;
    Int_t muchPoints[NPLANES];
    Int_t muchHits[NPLANES];
    Int_t stsPoints[NPLANES];
    Int_t stsHits[NPLANES];
    Double_t fBeta[NPLANES];
    Bool_t fIsReconstructed;
    Int_t fNStsHits;
    Int_t fNMuchHits;
    

    Double_t fChiToVertex;
    Double_t fChiMuch;
    Double_t fChiSTS;

    Int_t TRDtrackid;
    Double_t TRDchi2;
    Int_t TRDhits;
    
    Int_t fNTriggerHits;
    Double_t fSign;
    Int_t fNMuchPoints;
    Int_t fNStsPoints;
    Double_t fTofM;
  ClassDef(CbmAnaMuonCandidate,2);
};

#endif
