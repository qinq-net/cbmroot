//-----------------------------------------------------------
//-----------------------------------------------------------

#ifndef CbmKFSigmaReconstructor_HH
#define CbmKFSigmaReconstructor_HH

#include "FairTask.h"
#include "TString.h"

#include <vector>

class TClonesArray;
class CbmVertex;
class CbmKFParticleFinderPID;
class TFile;
class TDirectory;
class TH1F;
class TObject;

class CbmKFSigmaReconstructor : public FairTask {
 public:

  // Constructors/Destructors ---------
  CbmKFSigmaReconstructor(const char* name = "CbmKFSigmaReconstructor", Int_t iVerbose = 0);
  ~CbmKFSigmaReconstructor();

  void SetPVToZero() { fPVFindMode = 0; }
  void ReconstructSinglePV() { fPVFindMode = 1; }
  void RconstructMultiplePV() { fPVFindMode = 2; }
  
  void SetStsTrackBranchName(const TString& name)   { fStsTrackBranchName = name;  }

  void SetPIDInformation(CbmKFParticleFinderPID* pid) { fPID = pid; }

  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void Finish();
  
  void SetMCPID();

 private:
     
  void WriteHistosCurFile( TObject *obj );
  
  const CbmKFSigmaReconstructor& operator = (const CbmKFSigmaReconstructor&);
  CbmKFSigmaReconstructor(const CbmKFSigmaReconstructor&);
   
  //names of input branches
  TString fStsTrackBranchName;      //! Name of the input TCA with reco tracks
  TString fMCTracksBranchName;      //! Name of the input TCA with MC tracks
  TString fTrackMatchBranchName;      //! Name of the input TCA with track match
  
  //input branches
  TClonesArray *fTrackArray;
  TClonesArray *fMCTrackArray; //mc tracks
  TClonesArray *fTrackMatchArray; //track match
  CbmVertex* fPrimVtx;

  int fPVFindMode;
  
  //PID information
  CbmKFParticleFinderPID* fPID;
  
  std::vector<int> fMCIndex;

  //file with histos
  TString fOutFileName;
  TFile* fOutFile;
  TDirectory* fDirectory;
  TH1F* fHistos[30];

  ClassDef(CbmKFSigmaReconstructor,1);
};

#endif
