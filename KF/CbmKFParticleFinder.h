//-----------------------------------------------------------
//-----------------------------------------------------------

#ifndef CbmKFParticleFinder_HH
#define CbmKFParticleFinder_HH

#include "FairTask.h"
#include "CbmStsTrack.h"

#include "TString.h"
#include <vector>

class CbmKFParticleFinderPID;
class KFParticleTopoReconstructor;
class TClonesArray;
class CbmVertex;
class KFPTrackVector;

struct KFFieldVector
{
  float fField[10];
};

class CbmKFParticleFinder : public FairTask {
 public:

  // Constructors/Destructors ---------
  CbmKFParticleFinder(const char* name = "CbmKFParticleFinder", Int_t iVerbose = 0);
  ~CbmKFParticleFinder();

  void SetPVToZero() { fPVFindMode = 0; }
  void ReconstructSinglePV() { fPVFindMode = 1; }
  void RconstructMultiplePV() { fPVFindMode = 2; }
  
  void SetStsTrackBranchName(const TString& name)   { fStsTrackBranchName = name;  }

  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void Finish();
  
  const KFParticleTopoReconstructor * GetTopoReconstructor() const { return fTopoReconstructor; }
  
  void SetPIDInformation(CbmKFParticleFinderPID* pid) { fPID = pid; }
  
  // set cuts
  void SetPrimaryProbCut(float prob);
    
  // Set SE analysis
  void SetSuperEventAnalysis();
  
 private:
  
  double InversedChi2Prob(double p, int ndf) const;
  void FillKFPTrackVector(KFPTrackVector* tracks, const std::vector<CbmStsTrack>& vRTracks,
                          const std::vector<KFFieldVector>& vField, 
                          const std::vector<int>& pdg, const std::vector<int>& trackId,
                          const std::vector<float>& vChiToPrimVtx, bool atFirstPoint = 1) const;

  const CbmKFParticleFinder& operator = (const CbmKFParticleFinder&);
  CbmKFParticleFinder(const CbmKFParticleFinder&);
   
  //names of input branches
  TString fStsTrackBranchName;      //! Name of the input TCA with reco tracks

  //input branches
  TClonesArray* fTrackArray;
  TClonesArray* fEvents;
  CbmVertex* fPrimVtx;

  //topology reconstructor
  KFParticleTopoReconstructor *fTopoReconstructor;
//   KFParticleTopoReconstructor* eventTopoReconstructor;
  int fPVFindMode;
  
  //PID information
  CbmKFParticleFinderPID* fPID;
    
  //for super event analysis
  bool fSuperEventAnalysis;
  std::vector<CbmStsTrack> fSETracks;
  std::vector<KFFieldVector> fSEField;
  std::vector<int> fSEpdg;
  std::vector<int> fSETrackId;
  std::vector<float> fSEChiPrim;
  
  bool fTimeSliceMode;
  
  ClassDef(CbmKFParticleFinder,1);
};

#endif
