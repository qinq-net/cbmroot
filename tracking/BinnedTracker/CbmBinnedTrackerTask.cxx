/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmBinnedTrackerTask.h"
#include "GeoReader.h"
#include "FairLogger.h"
#include "global/CbmGlobalTrack.h"
   
CbmBinnedTrackerTask::CbmBinnedTrackerTask() : fTracker(0), fGlobalTracks(0), fStsTracks(0), fMuchTracks(0), fTrdTracks(0)
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
   
   fMuchTracks = new TClonesArray("CbmMuchTrack", 100);
   ioman->Register("MuchTrack", "Much", fMuchTracks, IsOutputBranchPersistent("MuchTrack"));
   
   fTrdTracks = new TClonesArray("CbmTrdTrack", 100);
   ioman->Register("TrdTrack", "Trd", fTrdTracks, IsOutputBranchPersistent("TrdTrack"));
   
   return kSUCCESS;
}

void CbmBinnedTrackerTask::Exec(Option_t* opt)
{
   fTracker->Reconstruct(-100);
   fGlobalTracks->Clear();
   int trackNumber = 0;
   std::list<CbmBinnedTracker::Track*>::const_iterator tracksEnd = fTracker->GetTracksEnd();
   
   for (std::list<CbmBinnedTracker::Track*>::const_iterator trackIter = fTracker->GetTracksBegin(); trackIter != tracksEnd; ++trackIter)
   {
      CbmGlobalTrack* globalTrack = new ((*fGlobalTracks)[trackNumber]) CbmGlobalTrack();
      globalTrack->SetStsTrackIndex(trackNumber);
      globalTrack->SetMuchTrackIndex(trackNumber);
      globalTrack->SetTrdTrackIndex(trackNumber);
      ++trackNumber;
   }
}

void CbmBinnedTrackerTask::Finish()
{
}

ClassImp(CbmBinnedTrackerTask)
