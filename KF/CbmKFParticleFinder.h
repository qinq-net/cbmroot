//-----------------------------------------------------------
//-----------------------------------------------------------

#ifndef CbmKFParticleFinder_HH
#define CbmKFParticleFinder_HH

#include "FairTask.h"

#include "TString.h"

#include <vector>

class CbmKFParticleFinderPID;
class KFParticleTopoReconstructor;
class TClonesArray;
class CbmVertex;

class CbmKFParticleFinder : public FairTask {
 public:

  // Constructors/Destructors ---------
  CbmKFParticleFinder(const char* name = "CbmKFParticleFinder", Int_t iVerbose = 0);
  ~CbmKFParticleFinder();

  void SetStsTrackBranchName(const TString& name)   { fStsTrackBranchName = name;  }

  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);

  const KFParticleTopoReconstructor * GetTopoReconstructor() const { return fTopoReconstructor; }
  
  void SetPIDInformation(CbmKFParticleFinderPID* pid) { fPID = pid; }
  
 private:
   
  //names of input branches
  TString fStsTrackBranchName;      //! Name of the input TCA with reco tracks

  //input branches
  TClonesArray *fTrackArray;
  CbmVertex* fPrimVtx;

  //topology reconstructor
  KFParticleTopoReconstructor *fTopoReconstructor;
  
  //PID information
  CbmKFParticleFinderPID* fPID;
  
  ClassDef(CbmKFParticleFinder,1);
};

#endif
