#ifndef DataTreeCbmInterface_H
#define DataTreeCbmInterface_H 1

#include "CbmMCEventData.h"
#include "CbmPsdEventData.h"
#include "CbmStsEventData.h"

#include "CbmKFPartEfficiencies.h"
#include "CbmKFParticleFinder.h"

#include "FairTask.h"
#include "CbmVertex.h"
#include <vector>
#include "TLorentzVector.h"
#include <map>
#include <cstring>

#include <TGeoManager.h>
#include "TGraphErrors.h"
#include <iostream>
#include "TClonesArray.h"
#include "TProfile.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TFile.h"
#include "FairMCEventHeader.h"

#include "CbmKFVertex.h"
#include "CbmTrackMatch.h"

#include "DataTreeEvent.h"
#include "DataTreeTrack.h"
#include "DataTreeConstants.h"



class TClonesArray;
class CbmVertex;
class TDirectory;
class TH1F;
class TProfile;
class TH2F;


class DataTreeCbmInterface : public FairTask
{

public:

  DataTreeCbmInterface();
  ~DataTreeCbmInterface();

  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  virtual void Finish();

  void SetOutputFile(const TString filename) { fOutputFileName = filename; }

  void LoadGeo(const TString &geoFile);
  //void SetPsdXpos(const Float_t pos) {  fPsdXpos = pos;  }
  //void SetPsdZpos(const Float_t pos) {  fPsdZpos = pos;  }
  //void SetNPsdModules(const Int_t n)  { fPsdModules = n; }

  void SetKFParticleFinderTOF(const CbmKFParticleFinder* finder) { fFinderTOF = finder; }
  void SetKFParticleFinderMC(const CbmKFParticleFinder* finder) { fFinderMC = finder; }

private:

  void InitInput();
  void InitOutput();
  void InitOutputTree();
  void InitDataTreeEvent();
  void ClearEvent();
  void ReadEvent();
  void ReadPSD();
  void ReadTracks();
  void LinkSTS();
  void ReadTOF();
  void ReadMC();
  int GetMCTrackMatch(const int idx);
  void ReadV0(const int UseMCpid = 0);

  TString fOutputFileName {""};
  TFile* fTreeFile {nullptr};
  TTree* fDataTree {nullptr};

  CbmVertex* fPrimVtx {nullptr};
  FairMCEventHeader* fHeader {nullptr};
  TClonesArray* flistPSDhit {nullptr};
  TClonesArray* flistPSDdigit {nullptr};
  TClonesArray* flistMCtrack {nullptr};
  TClonesArray* flistSTSRECOtrack {nullptr};
  TClonesArray* flistSTStrackMATCH {nullptr};
  TClonesArray *fGlobalTrackArray {nullptr};
  TClonesArray *fTofHitArray {nullptr};
  TClonesArray *fTofHitMatchArray {nullptr};

  DataTreeEvent* fDTEvent {nullptr};

  int fPsdModules{0};
  TVector3 fPsdPosition;
  std::map <int, TVector3 > fPsdModulePositions;


  std::vector<int> fMCTrackIDs;
  std::vector<int> fTrackIDs;

  const CbmKFParticleFinder* fFinderTOF;
  const CbmKFParticleFinder* fFinderMC;

  ClassDef(DataTreeCbmInterface, 1)
};

#endif
