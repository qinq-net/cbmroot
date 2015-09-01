//-----------------------------------------------------------
//-----------------------------------------------------------

#ifndef CbmKFTrackQA_HH
#define CbmKFTrackQA_HH

#include "FairTask.h"

#include "TString.h"

#include <vector>

class KFParticleTopoReconstructor;
class KFTopoPerformance;
class TClonesArray;
class TFile;
class TDirectory;
class TH1F;
class TObject;

class CbmKFTrackQA : public FairTask {
 public:

  // Constructors/Destructors ---------
  CbmKFTrackQA(const char* name = "CbmKFTrackQA", Int_t iVerbose = 0, TString outFileName="CbmKFTrackQA.root");
  ~CbmKFTrackQA();

  void SetStsTrackBranchName(const TString& name)   { fStsTrackBranchName = name;  }
  void SetGlobalTrackBranchName(const TString& name)   { fGlobalTrackBranchName = name;  }
  void SetTofBranchName(const TString& name)   { fTofBranchName = name;  }
  void SetMCTrackBranchName(const TString& name)   { fMCTracksBranchName = name;  }
  void SetTrackMatchBranchName(const TString& name)   { fTrackMatchBranchName = name;  }
  void SetMuchTrackMatchBranchName(const TString& name)   { fMuchTrackMatchBranchName = name;  }
  void SetTrdBranchName (const TString& name)      {   fTrdBranchName = name;  }
  void SetRichBranchName (const TString& name)      {   fRichBranchName = name;   }
  void SetMuchTrackBranchName (const TString& name) { fMuchTrackBranchName = name; }
  Int_t GetZtoNStation(Double_t getZ);

  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void Finish();
  
 private:
  
  const CbmKFTrackQA& operator = (const CbmKFTrackQA&);
  CbmKFTrackQA(const CbmKFTrackQA&);
  
  void WriteHistosCurFile( TObject *obj );
  
  //names of input branches
  TString fStsTrackBranchName;      //! Name of the input TCA with reco tracks
  TString fGlobalTrackBranchName;      //! Name of the input TCA with gloabal tracks
  TString fTofBranchName;      //! Name of the input TCA with tof hits
  TString fMCTracksBranchName;      //! Name of the input TCA with MC tracks
  TString fTrackMatchBranchName;      //! Name of the input TCA with track match
  TString fMuchTrackMatchBranchName;
  TString fTrdBranchName;
  TString fRichBranchName;
  TString fMuchTrackBranchName;

  //input branches
  TClonesArray *fTrackArray; //input reco tracks
  TClonesArray *fGlobalTrackArray; //input reco tracks
  TClonesArray *fTofHitArray; //input reco tracks
  TClonesArray *fMCTrackArray; //mc tracks
  TClonesArray *fTrackMatchArray; //track match
  TClonesArray *fMuchTrackMatchArray;//MuCh track match
  TClonesArray *fTrdTrackArray;
  TClonesArray *fRichRingArray;
  TClonesArray *fMuchTrackArray;//input much tracks
   
  //output file with histograms
  TString fOutFileName;
  TFile* fOutFile;
  TDirectory* fHistoDir;
  
  Int_t fNEvents;
  
  //histograms
    //STS
  static const int NStsHisto = 3;
  TH1F* hStsHisto[8][NStsHisto]; //All tracks, electrons, muons, pion, kaon, protons, fragments, ghost
    //Much
  static const int NMuchHisto = 5;
  TH1F* hMuchHisto[3][NMuchHisto]; //Muons, Background, Ghost
  
  ClassDef(CbmKFTrackQA,1);
};

#endif
