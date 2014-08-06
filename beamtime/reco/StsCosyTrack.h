#ifndef STSCOSYTRACK_H
#define STSCOSYTRACK_H 1


#include "FairTask.h"
#include <map>
#include "TH1F.h"
#include "TH2F.h"

using namespace std;

class TClonesArray;
class CbmStsCosyPoint;
class CbmStsHit;

class StsCosyTrack : public FairTask
{

 public:

  StsCosyTrack();
  ~StsCosyTrack();
  virtual InitStatus Init();
  virtual void Exec(Option_t* opt);
  Int_t GetEntries () { return fChain->GetEntries();}


 private:

  Int_t             fNDigis;   
  TClonesArray*     fPointArray;  //!
  TClonesArray*     fMCTrack;     //!
  TClonesArray*     fDigiArray;   
  TClonesArray*     fHits;   
  TChain *fChain;           
     

  StsCosyTrack(const StsCosyTrack&);
  StsCosyTrack operator=(const StsCosyTrack&);

  void Reset();
  void Finish();

  Bool_t FindTracks(); 
  Bool_t TestFit(); 

 // Map for layered hits
  map< Int_t , vector<CbmStsHit*> > fMapPts; //!

  TH1F* h_resX[5]; 
  TH1F* h_resY[5];
  TH1F* h_pullX[5]; 
  TH1F* h_pullY[5];
  TH1F *chi_change[3];
  
  TH1F* h_chi2; 
  TH1F* h_pchi2;
  TH1F* y_chi2; 
  TH1F* y_pchi2;
  
  TH2F * h_xx12;
  TH2F * h_xx02;
  TH2F * h_yy01;
  TH2F * h_xx01;
  TH2F * h_yy12;
  TH2F * h_yy02;
  
  ClassDef(StsCosyTrack,1);

};

#endif
