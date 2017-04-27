//-----------------------------------------------------------
//-----------------------------------------------------------

// Cbm Headers ----------------------
#include "CbmKFParticleFinderQA.h"
#include "CbmTrack.h"
#include "CbmMCTrack.h"
#include "CbmTrackMatchNew.h"
#include "CbmMCDataManager.h"
#include "CbmMCDataArray.h"
#include "CbmMCEventList.h"

#include "FairRunAna.h"

//KF Particle headers
#include "KFParticleTopoReconstructor.h"
#include "KFTopoPerformance.h"
#include "KFMCTrack.h"
#include "KFParticleMatch.h"

//ROOT headers
#include "TClonesArray.h"
#include "TFile.h"
#include "TObject.h"
#include "TMath.h"
#include "TDatabasePDG.h"

//c++ and std headers
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
using std::vector;

CbmKFParticleFinderQA::CbmKFParticleFinderQA(const char* name, Int_t iVerbose, const KFParticleTopoReconstructor* tr, TString outFileName):
  FairTask(name, iVerbose), fMCTracksBranchName("MCTrack"), fTrackMatchBranchName("StsTrackMatch"), fMCTrackArray(0), fMCTrackArrayEvent(0), fTrackMatchArray(0), 
  fRecParticles(0), fMCParticles(0), fMatchParticles(0), fSaveParticles(0), fSaveMCParticles(0),
  fOutFileName(outFileName), fOutFile(0), fEfffileName("Efficiency.txt"), fTopoPerformance(0), fPrintFrequency(100), fNEvents(0), fSuperEventAnalysis(0)
{
  for(Int_t i=0; i<5; i++)
    fTime[i] = 0;
  
  fTopoPerformance = new KFTopoPerformance;
  fTopoPerformance->SetTopoReconstructor(tr);

  TFile* curFile = gFile;
  TDirectory* curDirectory = gDirectory;
  
  if(!(fOutFileName == ""))
    fOutFile = new TFile(fOutFileName.Data(),"RECREATE");
  else
    fOutFile = gFile;
  fTopoPerformance->CreateHistos("KFTopoReconstructor",fOutFile);
  
  gFile = curFile;
  gDirectory = curDirectory;
}

CbmKFParticleFinderQA::~CbmKFParticleFinderQA()
{
  if(fTopoPerformance) delete fTopoPerformance;  
  
  if(fSaveParticles)
    fRecParticles->Delete();
  if(fSaveMCParticles)
  {
    fMCParticles->Delete();
    fMatchParticles->Delete();
  }
}

InitStatus CbmKFParticleFinderQA::Init()
{
  //Get ROOT Manager
  FairRootManager* ioman= FairRootManager::Instance();
  
  if(ioman==0)
  {
    Error("CbmKFParticleFinderQA::Init","RootManager not instantiated!");
    return kERROR;
  }

    //check the mode
  fTimeSliceMode = 0;
  if( ioman->GetObject("TimeSlice.") )
    fTimeSliceMode = 1;
  
  //MC Tracks
  if(fTimeSliceMode)
  {
    FairRootManager *fManger = FairRootManager::Instance();
    CbmMCDataManager* mcManager = (CbmMCDataManager*) fManger->GetObject("MCDataManager");
    if(mcManager==0)
      Error("CbmKFParticleFinderQA::Init","MC Data Manager not found!");
    
    fMCTrackArray= mcManager->InitBranch("MCTrack");
  
    if(fMCTrackArray==0)
    {
      Error("CbmKFParticleFinderQA::Init","mc track array not found!");
      return kERROR;
    }
    
    fEventList =  (CbmMCEventList*) ioman->GetObject("MCEventList.");
    if(fEventList==0)
    {
      Error("CbmKFParticleFinderQA::Init","MC Event List not found!");
      return kERROR;
    }
  }
  else
  {
    fMCTrackArrayEvent = (TClonesArray*) ioman->GetObject("MCTrack");
  }
  
  //Track match
  fTrackMatchArray=(TClonesArray*) ioman->GetObject(fTrackMatchBranchName);
  if(fTrackMatchArray==0)
  {
    Error("CbmKFParticleFinderQA::Init","track match array not found!");
    return kERROR;
  }
  
  if(fSaveParticles)
  {
    // create and register TClonesArray with output reco particles
    fRecParticles = new TClonesArray("KFParticle",100);
    ioman->Register("RecoParticles", "KFParticle", fRecParticles, IsOutputBranchPersistent("RecoParticles"));
  }

  if(fSaveMCParticles)
  {
    // create and register TClonesArray with output MC particles
    fMCParticles = new TClonesArray("KFMCParticle",100);
    ioman->Register("KFMCParticles", "KFParticle", fMCParticles, IsOutputBranchPersistent("KFMCParticles"));

    // create and register TClonesArray with matching between reco and MC particles
    fMatchParticles = new TClonesArray("KFParticleMatch",100);
    ioman->Register("KFParticleMatch", "KFParticle", fMatchParticles, IsOutputBranchPersistent("KFParticleMatch"));
  }
  return kSUCCESS;
}

void CbmKFParticleFinderQA::Exec(Option_t* /*opt*/)
{ 
  if(!fSuperEventAnalysis)
  {
    if(fSaveParticles)
      fRecParticles->Delete();
    if(fSaveMCParticles)
    {
      fMCParticles->Delete();
      fMatchParticles->Delete();
    }
    
    int nMCEvents = 1;
    if(fTimeSliceMode)
      nMCEvents = fEventList->GetNofEvents();   
    
    std::cout<<nMCEvents<<" nMCEvents"<<endl;
    
    vector<KFMCTrack> mcTracks;
    vector< vector< vector<int> > > indexMap(1);
    indexMap[0].resize(nMCEvents);
    
    int mcIndexOffset = 0;
    
    for(int iMCEvent=0; iMCEvent<nMCEvents; iMCEvent++)
    {
      int nMCTracks = 0;
      if(fTimeSliceMode)
        nMCTracks = fMCTrackArray->Size(0, iMCEvent);
      else
        nMCTracks = fMCTrackArrayEvent->GetEntriesFast();
      
      if(fTimeSliceMode)
        indexMap[0][iMCEvent].resize(nMCTracks);
      
      for(Int_t iMC=0; iMC<nMCTracks; iMC++)
      {
        CbmMCTrack *cbmMCTrack;
        if(fTimeSliceMode)
          cbmMCTrack = (CbmMCTrack*)fMCTrackArray->Get(0, iMCEvent, iMC);
        else
          cbmMCTrack = (CbmMCTrack*)fMCTrackArrayEvent->At(iMC);
        
        KFMCTrack kfMCTrack;
        kfMCTrack.SetX ( cbmMCTrack->GetStartX() );
        kfMCTrack.SetY ( cbmMCTrack->GetStartY() );
        kfMCTrack.SetZ ( cbmMCTrack->GetStartZ() );
        kfMCTrack.SetPx( cbmMCTrack->GetPx() );
        kfMCTrack.SetPy( cbmMCTrack->GetPy() );
        kfMCTrack.SetPz( cbmMCTrack->GetPz() );
        
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
        
        if(cbmMCTrack->GetMotherId() >= 0)
          kfMCTrack.SetMotherId(cbmMCTrack->GetMotherId() + mcIndexOffset);
        else
          kfMCTrack.SetMotherId(-iMCEvent-1);
        kfMCTrack.SetQP(q/p);
        kfMCTrack.SetPDG(pdg);
        kfMCTrack.SetNMCPoints(0);
        
        if(fTimeSliceMode)
          indexMap[0][iMCEvent][iMC] = mcTracks.size();
        mcTracks.push_back(kfMCTrack);
      }
      
      mcIndexOffset += nMCTracks;
    }
    
    Int_t ntrackMatches=fTrackMatchArray->GetEntriesFast();
    
    std::cout<<ntrackMatches<<" ntrackMatches"<<std::endl;
    
    vector<int> trackMatch(ntrackMatches, -1);

    for(int iTr=0; iTr<ntrackMatches; iTr++)
    {
      CbmTrackMatchNew* stsTrackMatch = (CbmTrackMatchNew*) fTrackMatchArray->At(iTr);
      if(stsTrackMatch -> GetNofLinks() == 0) continue;
      Float_t bestWeight = 0.f;
      Float_t totalWeight = 0.f;
      Int_t mcTrackId = -1;
      CbmLink link;
      for(int iLink=0; iLink<stsTrackMatch -> GetNofLinks(); iLink++)
      {
        totalWeight += stsTrackMatch->GetLink(iLink).GetWeight();
        if( stsTrackMatch->GetLink(iLink).GetWeight() > bestWeight)
        {
          bestWeight = stsTrackMatch->GetLink(iLink).GetWeight();
          int iMCTrack = stsTrackMatch->GetLink(iLink).GetIndex();
          link = stsTrackMatch->GetLink(iLink);
          
          if(fTimeSliceMode)
            mcTrackId = indexMap[link.GetFile()][link.GetEntry()][iMCTrack];
          else
            mcTrackId = stsTrackMatch->GetLink(iLink).GetIndex();
          
         /// std::cout << "mcTrackId " << mcTrackId << std::endl;
        }
      }
      if(bestWeight/totalWeight < 0.7) continue;
//       if(mcTrackId >= nMCTracks || mcTrackId < 0)
//       {
//         std::cout << "Sts Matching is wrong!    StsTackId = " << mcTrackId << " N mc tracks = " << nMCTracks << std::endl;
//         continue;
//       }
      
      if(TMath::Abs(mcTracks[mcTrackId].PDG()) > 4000 && 
        !(TMath::Abs(mcTracks[mcTrackId].PDG()) == 1000010020 || TMath::Abs(mcTracks[mcTrackId].PDG()) == 1000010030 ||
          TMath::Abs(mcTracks[mcTrackId].PDG()) == 1000020030 || TMath::Abs(mcTracks[mcTrackId].PDG()) == 1000020040 ) )
        continue;
      mcTracks[mcTrackId].SetReconstructed();
      trackMatch[iTr] = mcTrackId;
    }

    fTopoPerformance->SetMCTracks(mcTracks);
    fTopoPerformance->SetTrackMatch(trackMatch);
    
    fTopoPerformance->CheckMCTracks();
    fTopoPerformance->MatchTracks();
    fTopoPerformance->FillHistos();
    
    fNEvents++;
    fTime[4] += fTopoPerformance->GetTopoReconstructor()->Time();
    for(int iT=0; iT<4; iT++)
      fTime[iT] += fTopoPerformance->GetTopoReconstructor()->StatTime(iT);
    if(fNEvents%fPrintFrequency == 0)
    {
      std::cout << "Topo reconstruction time"
        << " Real = " << std::setw( 10 ) << fTime[4]/fNEvents * 1.e3 << " ms" << std::endl;
      std::cout << "    Init                " << fTime[0]/fNEvents * 1.e3 << " ms" << std::endl;
      std::cout << "    PV Finder           " << fTime[1]/fNEvents * 1.e3 << " ms" << std::endl;
      std::cout << "    Sort Tracks         " << fTime[2]/fNEvents * 1.e3 << " ms" << std::endl;
      std::cout << "    KF Particle Finder  " << fTime[3]/fNEvents * 1.e3 << " ms" << std::endl;
    }
    
    // save particles to a ROOT file
    if(fSaveParticles)
    {
      for(unsigned int iP=0; iP < fTopoPerformance->GetTopoReconstructor()->GetParticles().size(); iP++)
      {
        new((*fRecParticles)[iP]) KFParticle(fTopoPerformance->GetTopoReconstructor()->GetParticles()[iP]);
      }
    }

    if(fSaveMCParticles)
    {
      for(unsigned int iP=0; iP < fTopoPerformance->GetTopoReconstructor()->GetParticles().size(); iP++)
      {
        new((*fMatchParticles)[iP]) KFParticleMatch();
        KFParticleMatch *p = (KFParticleMatch*)( fMatchParticles->At(iP) );

        Short_t matchType = 0;
        int iMCPart = -1;
        if(!(fTopoPerformance->ParticlesMatch()[iP].IsMatchedWithPdg())) //background
        {
          if(fTopoPerformance->ParticlesMatch()[iP].IsMatched())
          {
            iMCPart = fTopoPerformance->ParticlesMatch()[iP].GetBestMatchWithPdg();
            matchType = 1;
          }
        }
        else
        {
          iMCPart = fTopoPerformance->ParticlesMatch()[iP].GetBestMatchWithPdg();
          matchType = 2;
        }

        p->SetMatch(iMCPart);
        p->SetMatchType(matchType);
      }

      for(unsigned int iP=0; iP < fTopoPerformance->MCParticles().size(); iP++)
      {
        new((*fMCParticles)[iP]) KFMCParticle(fTopoPerformance->MCParticles()[iP]);
      }
    }
  }
}

void CbmKFParticleFinderQA::Finish()
{
  if(fSuperEventAnalysis)
  {
    fTopoPerformance->SetPrintEffFrequency(1);
    
    vector<KFMCTrack> mcTracks(0);
    Int_t ntrackMatches=fTopoPerformance->GetTopoReconstructor()->GetParticles().size();
    vector<int> trackMatch(ntrackMatches, -1);

    fTopoPerformance->SetMCTracks(mcTracks);
    fTopoPerformance->SetTrackMatch(trackMatch);
    fTopoPerformance->CheckMCTracks();
    fTopoPerformance->MatchTracks();
    fTopoPerformance->FillHistos();

    fTime[4] += fTopoPerformance->GetTopoReconstructor()->Time();
    for(int iT=0; iT<4; iT++)
      fTime[iT] += fTopoPerformance->GetTopoReconstructor()->StatTime(iT);

    std::cout << "Topo reconstruction time"
      << " Real = " << std::setw( 10 ) << fTime[4] * 1.e3 << " ms" << std::endl;
    std::cout << "    Init                " << fTime[0] * 1.e3 << " ms" << std::endl;
    std::cout << "    PV Finder           " << fTime[1] * 1.e3 << " ms" << std::endl;
    std::cout << "    Sort Tracks         " << fTime[2] * 1.e3 << " ms" << std::endl;
    std::cout << "    KF Particle Finder  " << fTime[3] * 1.e3 << " ms" << std::endl;
  }
  
  TDirectory *curr = gDirectory;
  TFile *currentFile = gFile;
  // Open output file and write histograms

  fOutFile->cd();
  WriteHistosCurFile(fTopoPerformance->GetHistosDirectory());
//         if (histodirmod!=NULL) WriteHistos(histodirmod);
        //WriteHistos(gDirectory);
  if(!(fOutFileName == ""))
  {   
    fOutFile->Close();
    fOutFile->Delete();
  }
  gFile = currentFile;
  gDirectory = curr;

  std::fstream eff(fEfffileName.Data(),std::fstream::out);
  eff << fTopoPerformance->fParteff;
  eff.close();
}

void CbmKFParticleFinderQA::WriteHistosCurFile( TObject *obj ){
  
  
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

void CbmKFParticleFinderQA::SetPrintEffFrequency(Int_t n)
{ 
  fTopoPerformance->SetPrintEffFrequency(n); 
  fPrintFrequency = n;
}

ClassImp(CbmKFParticleFinderQA);
