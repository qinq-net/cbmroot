/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmBinnedTrackerTask.h"
#include "GeoReader.h"
#include "FairLogger.h"
#include "global/CbmGlobalTrack.h"
#include "CbmStsTrack.h"
#include "CbmTrdTrack.h"
#include "CbmMuchTrack.h"
   
CbmBinnedTrackerTask::CbmBinnedTrackerTask() : fTracker(0), fGlobalTracks(0), fStsTracks(0)/*, fMuchTracks(0)*//*, fTrdTracks(0)*/
{
}

CbmBinnedTrackerTask::~CbmBinnedTrackerTask()
{
   delete fTracker;
}

InitStatus CbmBinnedTrackerTask::Init()
{
   CbmBinnedGeoReader* geoReader = CbmBinnedGeoReader::Instance();
   
   if (0 == geoReader)
      fLogger->Fatal(MESSAGE_ORIGIN, "Couldn't instantiate CbmBinnedGeoReader");
   
   geoReader->Read();
   fTracker = CbmBinnedTracker::Instance();
   fTracker->SetChiSqCut(320);
   
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fGlobalTracks = new TClonesArray("CbmGlobalTrack", 100);
   ioman->Register("GlobalTrack", "Global", fGlobalTracks, IsOutputBranchPersistent("GlobalTrack"));
   
   fStsTracks = new TClonesArray("CbmStsTrack", 100);
   ioman->Register("StsTrack", "STS", fStsTracks, IsOutputBranchPersistent("StsTrack"));
   
   //fMuchTracks = new TClonesArray("CbmMuchTrack", 100);
   //ioman->Register("MuchTrack", "Much", fMuchTracks, IsOutputBranchPersistent("MuchTrack"));
   
   //fTrdTracks = new TClonesArray("CbmTrdTrack", 100);
   //ioman->Register("TrdTrack", "Trd", fTrdTracks, IsOutputBranchPersistent("TrdTrack"));
   
   return kSUCCESS;
}

void CbmBinnedTrackerTask::Exec(Option_t* opt)
{
   fTracker->Reconstruct(-100);
   fStsTracks->Clear();
   //fTrdTracks->Clear();
   //fMuchTracks->Clear();
   fGlobalTracks->Clear();
   int trackNumber = 0;
   std::list<CbmBinnedTracker::Track*>::const_iterator tracksEnd = fTracker->GetTracksEnd();
   
   for (std::list<CbmBinnedTracker::Track*>::const_iterator trackIter = fTracker->GetTracksBegin(); trackIter != tracksEnd; ++trackIter)
   {
      CbmStsTrack* stsTrack = new ((*fStsTracks)[trackNumber]) CbmStsTrack();
      stsTrack->SetNDF(4);
      stsTrack->SetChiSq(1);
      stsTrack->SetPreviousTrackId(-1);
      /*CbmTrdTrack* trdTrack = new ((*fTrdTracks)[trackNumber]) CbmTrdTrack();
      trdTrack->SetNDF(8);
      trdTrack->SetChiSq(1);
      trdTrack->SetPreviousTrackId(trackNumber);*/
      /*CbmMuchTrack* muchTrack = new ((*fMuchTracks)[trackNumber]) CbmMuchTrack();
      muchTrack->SetNDF(6);
      muchTrack->SetChiSq(1);
      muchTrack->SetPreviousTrackId(trackNumber);*/
      CbmGlobalTrack* globalTrack = new ((*fGlobalTracks)[trackNumber]) CbmGlobalTrack();
      globalTrack->SetStsTrackIndex(trackNumber);
      //globalTrack->SetMuchTrackIndex(trackNumber);
      //globalTrack->SetTrdTrackIndex(trackNumber);
      globalTrack->SetNDF(20);
      globalTrack->SetChi2(1);
      ++trackNumber;
      
      const CbmBinnedTracker::Track* recoTrack = *trackIter;
      
      for (Int_t i = 0; i < 2; ++i)
      {
         CbmTBin::HitHolder* hh = recoTrack->fHits[i];
         stsTrack->AddStsHit(hh->index);
      }
      
      /*for (Int_t i = 2; i < 6; ++i)
      {
         CbmTBin::HitHolder* hh = recoTrack->fHits[i];
         trdTrack->AddHit(hh->index, kTRDHIT);
      }*/
      
      /*for (Int_t i = 6; i < 9; ++i)
      {
         CbmTBin::HitHolder* hh = recoTrack->fHits[i];
         muchTrack->AddHit(hh->index, kMUCHPIXELHIT);
      }*/
      
      //globalTrack->SetTofHitIndex(recoTrack->fHits[/*9*/6]->index);
   }
}

void CbmBinnedTrackerTask::Finish()
{
}

ClassImp(CbmBinnedTrackerTask)
