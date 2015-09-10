//-----------------------------------------------------------
//-----------------------------------------------------------

// Cbm Headers ----------------------
#include "CbmKFTrackQA.h"
#include "CbmTrack.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"

#include "FairRunAna.h"

//KF Particle headers
#include "KFParticleTopoReconstructor.h"
#include "KFTopoPerformance.h"
#include "KFMCTrack.h"
#include "KFParticleMatch.h"

//ROOT headers
#include "TClonesArray.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TH1F.h"
#include "TObject.h"
#include "TMath.h"
#include "TDatabasePDG.h"

//c++ and std headers
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

#include "CbmStsTrack.h"
#include "CbmGlobalTrack.h"
#include "CbmMuchTrack.h"
#include "CbmTrack.h"

using std::vector;

CbmKFTrackQA::CbmKFTrackQA(const char* name, Int_t iVerbose, TString outFileName):
  FairTask(name, iVerbose), fStsTrackBranchName("StsTrack"), fGlobalTrackBranchName("GlobalTrack"), 
  fTofBranchName("TofHit"), fMCTracksBranchName("MCTrack"), fTrackMatchBranchName("StsTrackMatch"),
  fMuchTrackMatchBranchName("MuchTrackMatch"),
  fTrdBranchName ("TrdTrack"), fRichBranchName ("RichRing"), fMuchTrackBranchName("MuchTrack"),
  fTrackArray(0), fGlobalTrackArray(0), fTofHitArray(0), fMCTrackArray(0), fTrackMatchArray(0), fTrdTrackArray(0), fRichRingArray(0), fMuchTrackArray(0), 
  fOutFileName(outFileName), fOutFile(0), fNEvents(0)
{ 
  TFile* curFile = gFile;
  TDirectory* curDirectory = gDirectory;
  
  if(!(fOutFileName == ""))
    fOutFile = new TFile(fOutFileName.Data(),"RECREATE");
  else
    fOutFile = gFile;
  
  fOutFile->cd();
  fHistoDir = fOutFile->mkdir( "KFTrackQA" );
  fHistoDir->cd();
  
  gDirectory->mkdir("STS");
  gDirectory->cd("STS");
  {
    TString histoName[NStsHisto] = {"NHits", "chi2/NDF", "prob"};
    TString axisName[NStsHisto] = {"N hits", "#chi^{2}/NDF", "prob"};
    int nBins[NStsHisto] = {16, 100, 100};
    float xMin[NStsHisto] = {-0.5, 0.f,  0.f};
    float xMax[NStsHisto] = {15.5, 20.f, 1.f};
    
    TString subdirs[8] = {"Tracks", "e", "mu", "pi", "K", "p", "fragments", "ghost"};

    for(int iDir=0; iDir<8; iDir++)
    {
      gDirectory->mkdir(subdirs[iDir].Data());
      gDirectory->cd(subdirs[iDir].Data());
      for(int iH=0; iH<NStsHisto; iH++)
      {
        hStsHisto[iDir][iH] = new TH1F(histoName[iH].Data(), histoName[iH].Data(), nBins[iH], xMin[iH], xMax[iH]);
        hStsHisto[iDir][iH] ->GetXaxis()->SetTitle(axisName[iH].Data());
      }
      gDirectory->cd(".."); //STS
    }
  }
  gDirectory->cd("..");

  gDirectory->mkdir("MuCh");
  gDirectory->cd("MuCh");
  {
    TString histoName[NMuchHisto] = {"NHits", "FirstStation", "LastStation", "chi2/NDF", "prob"};
    TString axisName[NMuchHisto] = {"N hits", "First Station", "Last Station", "#chi^{2}/NDF", "prob"};
    int nBins[NMuchHisto] = {16, 16, 16, 100, 100};
    float xMin[NMuchHisto] = {-0.5f, -0.5f, -0.5f, 0.f,  0.f};
    float xMax[NMuchHisto] = {15.5f, 15.5f, 15.5f, 20.f, 1.f};
    
    TString subdirs[3] = {"mu", "Background", "Ghost"};
    
    for(int iDir=0; iDir<3; iDir++)
    {
      gDirectory->mkdir(subdirs[iDir].Data());
      gDirectory->cd(subdirs[iDir].Data());
      for(int iH=0; iH<NMuchHisto; iH++)
      {
        hMuchHisto[iDir][iH] = new TH1F(histoName[iH].Data(), histoName[iH].Data(), nBins[iH], xMin[iH], xMax[iH]);
        hMuchHisto[iDir][iH] ->GetXaxis()->SetTitle(axisName[iH].Data());
      }
      gDirectory->cd(".."); //MuCh
    }
  }
  gDirectory->cd("..");
  
  gFile = curFile;
  gDirectory = curDirectory;
}

CbmKFTrackQA::~CbmKFTrackQA()
{
}

InitStatus CbmKFTrackQA::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
  {
    Error("CbmKFParticleFinderPID::Init","RootManager not instantiated!");
    return kERROR;
  }
  
  // Get sts tracks
  fTrackArray=(TClonesArray*) ioman->GetObject(fStsTrackBranchName);
  if(fTrackArray==0)
  {
    Error("CbmKFParticleFinderPID::Init","track-array not found!");
    return kERROR;
  }
  
  // Get global tracks
  fGlobalTrackArray=(TClonesArray*) ioman->GetObject(fGlobalTrackBranchName);
  if(fGlobalTrackArray==0)
    Error("CbmKFParticleFinderPID::Init","global track array not found!");
  
  // Get ToF hits
  fTofHitArray=(TClonesArray*) ioman->GetObject(fTofBranchName);
  if(fTofHitArray==0)
    Error("CbmKFParticleFinderPID::Init","TOF track-array not found!");
  
  // TRD
  fTrdTrackArray = (TClonesArray*) ioman->GetObject(fTrdBranchName);
  if(fTrdTrackArray==0)
    Error("CbmKFParticleFinderPID::Init","TRD track-array not found!");
    
  fRichRingArray = (TClonesArray*) ioman->GetObject(fRichBranchName);
  if(fRichRingArray == 0)
    Error("CbmKFParticleFinderPID::Init","Rich ring array not found!");
  
  fMCTrackArray=(TClonesArray*) ioman->GetObject(fMCTracksBranchName);
  if(fMCTrackArray==0)
  {
    Error("CbmKFParticleFinderPID::Init","mc track array not found!");
    return kERROR;
  }
  
  //Track match
  fTrackMatchArray=(TClonesArray*) ioman->GetObject(fTrackMatchBranchName);
  if(fTrackMatchArray==0)
  {
    Error("CbmKFParticleFinderPID::Init","track match array not found!");
    return kERROR;
  }
  
  //Much track match
  fMuchTrackMatchArray=(TClonesArray*) ioman->GetObject(fMuchTrackMatchBranchName);
  if(fMuchTrackMatchArray==0)
  {
    Error("CbmKFParticleFinderPID::Init","Much track match array not found!");
    return kERROR;
  }
  //Much
  fMuchTrackArray = (TClonesArray*) ioman->GetObject(fMuchTrackBranchName);
  if (fMuchTrackArray == 0)
  {
    Error("CbmKFParticleFinderPID::Init", "Much track-array not found!");
    return kERROR;
  }
    
  return kSUCCESS;
}

void CbmKFTrackQA::Exec(Option_t* opt)
{ 
  fNEvents++;

  Int_t nMCTracks = fMCTrackArray->GetEntriesFast();
  vector<KFMCTrack> mcTracks(nMCTracks);
  for(Int_t iMC=0; iMC<nMCTracks; iMC++)
  {
    CbmMCTrack *cbmMCTrack = (CbmMCTrack*)fMCTrackArray->At(iMC);

    
    mcTracks[iMC].SetX ( cbmMCTrack->GetStartX() );
    mcTracks[iMC].SetY ( cbmMCTrack->GetStartY() );
    mcTracks[iMC].SetZ ( cbmMCTrack->GetStartZ() );
    mcTracks[iMC].SetPx( cbmMCTrack->GetPx() );
    mcTracks[iMC].SetPy( cbmMCTrack->GetPy() );
    mcTracks[iMC].SetPz( cbmMCTrack->GetPz() );
    
    Int_t pdg = cbmMCTrack->GetPdgCode();
    Double_t q=1;
    if ( pdg < 9999999 && ( (TParticlePDG *)TDatabasePDG::Instance()->GetParticle(pdg) ))
      q = TDatabasePDG::Instance()->GetParticle(pdg)->Charge()/3.0;
    else if(pdg ==  1000010020) q =  1;
    else if(pdg == -1000010020) q = -1;
    else if(pdg ==  1000010030) q =  1;
    else if(pdg == -1000010030) q = -1;
    else if(pdg ==  1000020030) q =  2;
    else if(pdg == -1000020030) q = -2;
    else if(pdg ==  1000020040) q =  2;
    else if(pdg == -1000020040) q = -2;
    else q = 0;
    Double_t p = cbmMCTrack->GetP();
    
    mcTracks[iMC].SetMotherId(cbmMCTrack->GetMotherId());
    mcTracks[iMC].SetQP(q/p);
    mcTracks[iMC].SetPDG(pdg);
    mcTracks[iMC].SetNMCPoints(0);
  }
  
  Int_t ntrackMatches=fTrackMatchArray->GetEntriesFast();
  vector<int> trackMatch(ntrackMatches, -1);

  for(int iTr=0; iTr<ntrackMatches; iTr++)
  {
    CbmTrackMatchNew* stsTrackMatch = (CbmTrackMatchNew*) fTrackMatchArray->At(iTr);
    if(stsTrackMatch -> GetNofLinks() == 0) continue;
    Float_t bestWeight = 0.f;
    Float_t totalWeight = 0.f;
    Int_t mcTrackId = -1;
    for(int iLink=0; iLink<stsTrackMatch -> GetNofLinks(); iLink++)
    {
      totalWeight += stsTrackMatch->GetLink(iLink).GetWeight();
      if( stsTrackMatch->GetLink(iLink).GetWeight() > bestWeight)
      {
        bestWeight = stsTrackMatch->GetLink(iLink).GetWeight();
        mcTrackId = stsTrackMatch->GetLink(iLink).GetIndex();
      }
    }
    if(bestWeight/totalWeight < 0.7) continue;
    if(mcTrackId >= nMCTracks || mcTrackId < 0)
    {
      std::cout << "Sts Matching is wrong!    StsTackId = " << mcTrackId << " N mc tracks = " << nMCTracks << std::endl;
      continue;
    }
    
    mcTracks[mcTrackId].SetReconstructed();
    trackMatch[iTr] = mcTrackId;
  }
  
  ///Radevych
  Int_t nMuchTrackMatches=fMuchTrackMatchArray->GetEntriesFast();
  vector<int> trackMuchMatch(nMuchTrackMatches, -1);

  for(int iTr=0; iTr<nMuchTrackMatches; iTr++)
  {
    CbmTrackMatchNew* muchTrackMatch = (CbmTrackMatchNew*) fMuchTrackMatchArray->At(iTr);
//     std::cout << "muchTrackMatch -> GetNofLinks() == " << muchTrackMatch -> GetNofLinks() << std::endl;
//     std::cin.get();
    if(muchTrackMatch -> GetNofLinks() == 0) continue;
    Float_t bestWeight = 0.f;
    Float_t totalWeight = 0.f;
    Int_t mcTrackId = -1;
    for(int iLink=0; iLink<muchTrackMatch -> GetNofLinks(); iLink++)
    {
      totalWeight += muchTrackMatch->GetLink(iLink).GetWeight();
      if( muchTrackMatch->GetLink(iLink).GetWeight() > bestWeight)
      {
        bestWeight = muchTrackMatch->GetLink(iLink).GetWeight();
        mcTrackId = muchTrackMatch->GetLink(iLink).GetIndex();
      }
    }
    if(bestWeight/totalWeight < 0.7) continue;
    if(mcTrackId >= nMCTracks || mcTrackId < 0)
    {
      std::cout << "Much Matching is wrong!    MuchTackId = " << mcTrackId << " N mc tracks = " << nMCTracks << std::endl;
      continue;
    }
    
    //mcTracks[mcTrackId].SetReconstructed();//////?????????????????????????????????????????????????????????????????
    trackMuchMatch[iTr] = mcTrackId;
  }
  
  
  ///Radevych
  
  if (NULL == fGlobalTrackArray) { Fatal("KF Particle Finder", "No GlobalTrack array!"); }
  for (Int_t igt = 0; igt < fGlobalTrackArray->GetEntriesFast(); igt++)
  {
    const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*>(fGlobalTrackArray->At(igt));
    
    Int_t stsTrackIndex = globalTrack->GetStsTrackIndex();//for STS histos
    CbmStsTrack* cbmStsTrack = (CbmStsTrack*) fTrackArray->At(stsTrackIndex);
    int stsTrackMCIndex = trackMatch[stsTrackIndex];
    Double_t* stsHistoData = new Double_t[NStsHisto];
    stsHistoData[0] = cbmStsTrack->GetNofStsHits();//NHits
    stsHistoData[1] = cbmStsTrack->GetChiSq()/cbmStsTrack->GetNDF();//Chi2/NDF
    stsHistoData[2] = TMath::Prob(cbmStsTrack->GetChiSq(),cbmStsTrack->GetNDF());//prob
    if(stsTrackMCIndex>-1)
    {
      for(int iDir=0; iDir<7; iDir++)
      {
        for(int iH=0; iH<NStsHisto; iH++)
        {
          if(iDir == 0)//tracks
            hStsHisto[iDir][iH]->Fill(stsHistoData[iH]);
          if(TMath::Abs(mcTracks[stsTrackMCIndex].PDG()) == 11 && iDir==1)//e
            hStsHisto[iDir][iH]->Fill(stsHistoData[iH]);
          if(TMath::Abs(mcTracks[stsTrackMCIndex].PDG()) == 13 && iDir==2)//mu
            hStsHisto[iDir][iH]->Fill(stsHistoData[iH]);
          if(TMath::Abs(mcTracks[stsTrackMCIndex].PDG()) == 211 && iDir==3)//pi
            hStsHisto[iDir][iH]->Fill(stsHistoData[iH]);
          if(TMath::Abs(mcTracks[stsTrackMCIndex].PDG()) == 321 && iDir==4)//K
            hStsHisto[iDir][iH]->Fill(stsHistoData[iH]);
          if(TMath::Abs(mcTracks[stsTrackMCIndex].PDG()) == 2212 && iDir==5)//p
            hStsHisto[iDir][iH]->Fill(stsHistoData[iH]);
          if(TMath::Abs(mcTracks[stsTrackMCIndex].PDG()) > 1000000000 && iDir==6)//fragments
            hStsHisto[iDir][iH]->Fill(stsHistoData[iH]); 
        }
      }
      
    }
    else
      for(int iH=0; iH<NStsHisto; iH++)//ghost
        hStsHisto[7][iH]->Fill(stsHistoData[iH]);
    delete [] stsHistoData;
    
    
    Int_t muchIndex = globalTrack->GetMuchTrackIndex();//for MuCh histos
    if(muchIndex>-1)
    {
      CbmMuchTrack* muchTrack = (CbmMuchTrack*)fMuchTrackArray->At(muchIndex);
      
      int muchTrackMCIndex = trackMuchMatch[muchIndex];
      
      Double_t* muchHistoData = new Double_t[NMuchHisto];
      muchHistoData[0] = muchTrack->GetNofHits();//NHits
      muchHistoData[1] = GetZtoNStation(muchTrack->GetParamFirst()->GetZ());//FirstStation
      muchHistoData[2] = GetZtoNStation(muchTrack->GetParamLast()->GetZ());//LastStation
      muchHistoData[3] = muchTrack->GetChiSq()/muchTrack->GetNDF();//Chi2/NDF
      muchHistoData[4] = TMath::Prob(muchTrack->GetChiSq(),muchTrack->GetNDF());//prob
//       std::cout << "stsTrackMCIndex = " << stsTrackMCIndex << std::endl;
//       std::cout << "muchTrackMCIndex = " << muchTrackMCIndex << std::endl;
//       std::cin.get();
      if(stsTrackMCIndex<0 || stsTrackMCIndex!=muchTrackMCIndex) //ghost
        for(int iH=0; iH<NMuchHisto; iH++)
          hMuchHisto[2][iH]->Fill(muchHistoData[iH]);
      else
      {
        if(TMath::Abs(mcTracks[stsTrackMCIndex].PDG()) == 13) //muon 
          for(int iH=0; iH<NMuchHisto; iH++)
            hMuchHisto[0][iH]->Fill(muchHistoData[iH]);
        else //BG
          for(int iH=0; iH<NMuchHisto; iH++)
            hMuchHisto[1][iH]->Fill(muchHistoData[iH]);
      }
      delete [] muchHistoData;
    }
 
  }
  
}

void CbmKFTrackQA::Finish()
{
  TDirectory *curr = gDirectory;
  TFile *currentFile = gFile;
  // Open output file and write histograms

  fOutFile->cd();
  WriteHistosCurFile(fHistoDir);
  if(!(fOutFileName == ""))
  {   
    fOutFile->Close();
    fOutFile->Delete();
  }
  gFile = currentFile;
  gDirectory = curr;
}

void CbmKFTrackQA::WriteHistosCurFile( TObject *obj ){
  
  
  if( !obj->IsFolder() ) obj->Write();
  else{
    TDirectory *cur = gDirectory;
    TFile *currentFile = gFile;

    TDirectory *sub = cur->GetDirectory(obj->GetName());
    sub->cd();
    TList *listSub = (static_cast<TDirectory*>(obj))->GetList();
    TIter it(listSub);
    while( TObject *obj1=it() ) WriteHistosCurFile(obj1);
    cur->cd();
    gFile = currentFile;
    gDirectory = cur;
  }
}

Int_t CbmKFTrackQA::GetZtoNStation(Double_t getZ)
{
  if(TMath::Abs(getZ-145)<=2.0) return 1;
  if(TMath::Abs(getZ-155)<=2.0) return 2;
  if(TMath::Abs(getZ-165)<=2.0) return 3;
  if(TMath::Abs(getZ-195)<=2.0) return 4;
  if(TMath::Abs(getZ-205)<=2.0) return 5;
  if(TMath::Abs(getZ-215)<=2.0) return 6;
  if(TMath::Abs(getZ-245)<=2.0) return 7;
  if(TMath::Abs(getZ-255)<=2.0) return 8;
  if(TMath::Abs(getZ-265)<=2.0) return 9;
  if(TMath::Abs(getZ-305)<=2.0) return 10;
  if(TMath::Abs(getZ-315)<=2.0) return 11;
  if(TMath::Abs(getZ-325)<=2.0) return 12;
  if(TMath::Abs(getZ-370)<=2.0) return 13;
  if(TMath::Abs(getZ-380)<=2.0) return 14;
  if(TMath::Abs(getZ-390)<=2.0) return 15;
  if(TMath::Abs(getZ-500)<=2.0) return 16;
  if(TMath::Abs(getZ-510)<=2.0) return 17;
  if(TMath::Abs(getZ-520)<=2.0) return 18;
}

ClassImp(CbmKFTrackQA);
