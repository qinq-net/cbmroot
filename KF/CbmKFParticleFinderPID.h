//-----------------------------------------------------------
//-----------------------------------------------------------

#ifndef CbmKFParticleFinderPID_HH
#define CbmKFParticleFinderPID_HH

#include "FairTask.h"

#include "TString.h"


#include "CbmMCDataArray.h"
#include "CbmMCEventList.h"

#include <vector>

class TClonesArray;
class TFile;
class TObject;

class CbmKFParticleFinderPID : public FairTask {
 public:

  // Constructors/Destructors ---------
  CbmKFParticleFinderPID(const char* name = "CbmKFParticleFinderPID", Int_t iVerbose = 0);
  ~CbmKFParticleFinderPID();

  void SetStsTrackBranchName(const TString& name)   { fStsTrackBranchName = name;  }
  void SetGlobalTrackBranchName(const TString& name)   { fGlobalTrackBranchName = name;  }
  void SetTofBranchName(const TString& name)   { fTofBranchName = name;  }
  void SetMCTrackBranchName(const TString& name)   { fMCTracksBranchName = name;  }
  void SetTrackMatchBranchName(const TString& name)   { fTrackMatchBranchName = name;  }
  void SetTrdBranchName (const TString& name)      {   fTrdBranchName = name;  }
  void SetRichBranchName (const TString& name)      {   fRichBranchName = name;   }
  void SetMuchTrackBranchName (const TString& name) { fMuchTrackBranchName = name; }
  
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void Finish();
  
  void SetPIDMode(int mode) { fPIDMode = mode; }
  void SetSIS100() { fSisMode = 0; }
  void SetSIS300() { fSisMode = 1; }
  
  void DoNotUseTRD() { fTrdPIDMode = 0; }
  void UseTRDWknPID() { fTrdPIDMode = 1; }
  void UseTRDANNPID() { fTrdPIDMode = 2; }
  
  void DoNotUseRICH() { fRichPIDMode = 0; }
  void UseRICHRvspPID() { fRichPIDMode = 1; }
  void UseRICHANNPID() { fRichPIDMode = 2; }
  
  void DoNotUseMuch() { fMuchMode = 0; }
  void UseMuch() { fMuchMode = 1; }
 
  //setters for MuCh cuts
  void SetNMinStsHitsForMuon( int cut ) { fMuchCutsInt[0] = cut; }
  void SetNMinMuchHitsForLMVM( int cut ) { fMuchCutsInt[1] = cut; }
  void SetNMinMuchHitsForJPsi( int cut ) { fMuchCutsInt[2] = cut; }
  void SetMaxChi2ForStsMuonTrack( float cut ) { fMuchCutsFloat[0] = cut; }
  void SetMaxChi2ForMuchMuonTrack( float cut ) { fMuchCutsFloat[1] = cut; }
  
  const std::vector<int>& GetPID() const { return fPID; }
  
 private:
  
  const CbmKFParticleFinderPID& operator = (const CbmKFParticleFinderPID&);
  CbmKFParticleFinderPID(const CbmKFParticleFinderPID&);
  
  void SetMCPID();
  void SetRecoPID();
  
  //names of input branches
  TString fStsTrackBranchName;      //! Name of the input TCA with reco tracks
  TString fGlobalTrackBranchName;      //! Name of the input TCA with gloabal tracks
  TString fTofBranchName;      //! Name of the input TCA with tof hits
  TString fMCTracksBranchName;      //! Name of the input TCA with MC tracks
  TString fTrackMatchBranchName;      //! Name of the input TCA with track match
  TString fTrdBranchName;
  TString fRichBranchName;
  TString fMuchTrackBranchName;

  //input branches
  TClonesArray *fTrackArray; //input reco tracks
  TClonesArray *fGlobalTrackArray; //input reco tracks
  TClonesArray *fTofHitArray; //input reco tracks
  TClonesArray *fMCTrackArray; //mc tracks
  TClonesArray *fTrackMatchArray; //track match
  TClonesArray *fTrdTrackArray;
  TClonesArray *fRichRingArray;
  TClonesArray *fMuchTrackArray;//input much tracks
  CbmMCDataArray* fMCTracks;
  
  //PID variables
  Int_t fPIDMode;
  Int_t fSisMode;
  Int_t fTrdPIDMode;
  Int_t fRichPIDMode;
  Int_t fMuchMode;
  
  bool fTimeSliceMode;
  
  //MuCh cuts
  float fMuchCutsFloat[2];
  int   fMuchCutsInt[3];
  
  std::vector<int> fPID;
  
  ClassDef(CbmKFParticleFinderPID,1);
};

#endif
