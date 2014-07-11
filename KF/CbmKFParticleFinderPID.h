//-----------------------------------------------------------
//-----------------------------------------------------------

#ifndef CbmKFParticleFinderPID_HH
#define CbmKFParticleFinderPID_HH

#include "FairTask.h"

#include "TString.h"

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
  
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void Finish();
  
  void SetPIDMode(int mode) { fPIDMode = mode; }
  const std::vector<int>& GetPID() const { return fPID; }
  
 private:
  
  void SetMCPID();
  void SetRecoPID();
  
  //names of input branches
  TString fStsTrackBranchName;      //! Name of the input TCA with reco tracks
  TString fGlobalTrackBranchName;      //! Name of the input TCA with gloabal tracks
  TString fTofBranchName;      //! Name of the input TCA with tof hits
  TString fMCTracksBranchName;      //! Name of the input TCA with MC tracks
  TString fTrackMatchBranchName;      //! Name of the input TCA with track match

  //input branches
  TClonesArray *fTrackArray; //input reco tracks
  TClonesArray *fGlobalTrackArray; //input reco tracks
  TClonesArray *fTofHitArray; //input reco tracks
  TClonesArray *fMCTrackArray; //mc tracks
  TClonesArray *fTrackMatchArray; //track match
  
  //PID variables
  Int_t fPIDMode;
  std::vector<int> fPID;
  
  ClassDef(CbmKFParticleFinderPID,1);
};

#endif
