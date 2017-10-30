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
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#ifdef __MACH__
#include <mach/mach_time.h>
#include <sys/time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
inline int clock_gettime(int /*clk_id*/, struct timespec *t){
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    uint64_t time;
    time = mach_absolute_time();
    double nseconds = ((double)time * (double)timebase.numer)/((double)timebase.denom);
    double seconds = ((double)time * (double)timebase.numer)/((double)timebase.denom * 1e9);
    t->tv_sec = seconds;
    t->tv_nsec = nseconds;
    return 0;
}
#else
#include <time.h>
#endif

using std::fill_n;
using std::copy;

CbmBinnedTrackerTask* CbmBinnedTrackerTask::fInstance = 0;
   
CbmBinnedTrackerTask::CbmBinnedTrackerTask(bool useAllDetectors, Double_t beamWidthX, Double_t beamWidthY) : fUseAllDetectors(useAllDetectors), fIsOnlyPrimary(false),
   fSettings(0), fBeamDx(beamWidthX), fBeamDy(beamWidthY), fTracker(0), fGlobalTracks(0), fStsTracks(0), fMuchTracks(0), fTrdTracks(0)
{
   fInstance = this;
   fill_n(fUseModules, int(kLastModule), fUseAllDetectors);
   fUseModules[kMuch] = false;// Temporary hack
   fUseModules[kRich] = false;// Temporary hack
}

CbmBinnedTrackerTask::~CbmBinnedTrackerTask()
{
   delete fTracker;
}

InitStatus CbmBinnedTrackerTask::Init()
{
   fSettings = CbmBinnedSettings::Instance();
   fSettings->SetConfiguring(false);
   fSettings->SetOnlyPrimary(fIsOnlyPrimary);
   fSettings->SetUse(fUseModules);
   fSettings->SetNofStsStations(0);
   fSettings->SetNofMuchStations(0);
   fSettings->SetNofTrdStations(0);
   fSettings->SetNofStations(0);
   CbmBinnedGeoReader* geoReader = CbmBinnedGeoReader::Instance();
   
   if (0 == geoReader)
      fLogger->Fatal(MESSAGE_ORIGIN, "Couldn't instantiate CbmBinnedGeoReader");
   
   geoReader->Read();
   fTracker = CbmBinnedTracker::Instance();
   fTracker->SetChiSqCut(224);
   fTracker->Init();
   
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fGlobalTracks = new TClonesArray("CbmGlobalTrack", 100);
   ioman->Register("GlobalTrack", "Global", fGlobalTracks, IsOutputBranchPersistent("GlobalTrack"));
   
   if (fSettings->Use(kSts))
   {
      fStsTracks = new TClonesArray("CbmStsTrack", 100);
      ioman->Register("StsTrack", "STS", fStsTracks, IsOutputBranchPersistent("StsTrack"));
   }
   
   if (fSettings->Use(kMuch))
   {
      fMuchTracks = new TClonesArray("CbmMuchTrack", 100);
      ioman->Register("MuchTrack", "Much", fMuchTracks, IsOutputBranchPersistent("MuchTrack"));
   }
   
   if (fSettings->Use(kTrd))
   {
      fTrdTracks = new TClonesArray("CbmTrdTrack", 100);
      ioman->Register("TrdTrack", "Trd", fTrdTracks, IsOutputBranchPersistent("TrdTrack"));
   }
   
   fSettings->setChanged();
   fSettings->setInputVersion(-2,1);
   
   return kSUCCESS;
}

static long fullDuration = 0;

void CbmBinnedTrackerTask::Exec(Option_t* opt)
{
   timespec ts;
   clock_gettime(CLOCK_REALTIME, &ts);
   long beginTime = ts.tv_sec * 1000000000 + ts.tv_nsec;
   fTracker->Reconstruct(-100);
   clock_gettime(CLOCK_REALTIME, &ts);
   long endTime = ts.tv_sec * 1000000000 + ts.tv_nsec;
   fullDuration += endTime - beginTime;
   
   if (fSettings->Use(kSts))
      fStsTracks->Clear();
   
   if (fSettings->Use(kMuch))
      fMuchTracks->Clear();
   
   if (fSettings->Use(kTrd))
      fTrdTracks->Clear();
   
   fGlobalTracks->Clear();
   int trackNumber = 0;
   std::list<CbmBinnedTracker::Track*>::const_iterator tracksEnd = fTracker->GetTracksEnd();
   
   for (std::list<CbmBinnedTracker::Track*>::const_iterator trackIter = fTracker->GetTracksBegin(); trackIter != tracksEnd; ++trackIter)
   {
      const CbmBinnedTracker::Track* recoTrack = *trackIter;
      int previousTrackId = -1;
      int stationNumber = 0;
      int nofStations = 0;
      CbmGlobalTrack* globalTrack = new ((*fGlobalTracks)[trackNumber]) CbmGlobalTrack();
      
      if (fSettings->Use(kSts))
      {
         globalTrack->SetStsTrackIndex(trackNumber);
         CbmStsTrack* stsTrack = new ((*fStsTracks)[trackNumber]) CbmStsTrack();
         stsTrack->SetNDF(fSettings->GetNofStsStations() * 2);
         stsTrack->SetChiSq(1);
         stsTrack->SetPreviousTrackId(previousTrackId);
         previousTrackId = trackNumber;
         nofStations += fSettings->GetNofStsStations();
         
         for (; stationNumber < nofStations; ++stationNumber)
         {
            CbmTBin::HitHolder* hh = recoTrack->fHits[stationNumber];
            stsTrack->AddStsHit(hh->index);
         }
      }
      
      if (fSettings->Use(kMuch))
      {
         globalTrack->SetMuchTrackIndex(trackNumber);
         CbmMuchTrack* muchTrack = new ((*fMuchTracks)[trackNumber]) CbmMuchTrack();
         muchTrack->SetNDF(fSettings->GetNofMuchStations() * 2);
         muchTrack->SetChiSq(1);
         muchTrack->SetPreviousTrackId(previousTrackId);
         previousTrackId = trackNumber;
         nofStations += fSettings->GetNofMuchStations();
      
         for (; stationNumber < nofStations; ++stationNumber)
         {
            CbmTBin::HitHolder* hh = recoTrack->fHits[stationNumber];
            muchTrack->AddHit(hh->index, kMUCHPIXELHIT);
         }
      }
      
      if (fSettings->Use(kTrd))
      {
         globalTrack->SetTrdTrackIndex(trackNumber);
         CbmTrdTrack* trdTrack = new ((*fTrdTracks)[trackNumber]) CbmTrdTrack();
         trdTrack->SetNDF(fSettings->GetNofTrdStations() * 2);
         trdTrack->SetChiSq(1);
         trdTrack->SetPreviousTrackId(previousTrackId);
         previousTrackId = trackNumber;
         nofStations += fSettings->GetNofTrdStations();
      
         for (; stationNumber < nofStations; ++stationNumber)
         {
            CbmTBin::HitHolder* hh = recoTrack->fHits[stationNumber];
            trdTrack->AddHit(hh->index, kTRDHIT);
         }
      }
      
      if (fSettings->Use(kTof))
         globalTrack->SetTofHitIndex(recoTrack->fHits[nofStations++]->index);
      
      globalTrack->SetNDF(nofStations * 2);
      globalTrack->SetChi2(recoTrack->fChiSq);
      ++trackNumber;
   }
}

void CbmBinnedTrackerTask::Finish()
{
   double segTrue = 100 * fTracker->fNofTrueSegments;
   segTrue /= fTracker->fNofTrueSegments + fTracker->fNofWrongSegments;
   double segWrong = 100 * fTracker->fNofWrongSegments;
   segWrong /= fTracker->fNofTrueSegments + fTracker->fNofWrongSegments;
   cout << "True segments = " << segTrue << " [" << fTracker->fNofTrueSegments << "/" << fTracker->fNofTrueSegments + fTracker->fNofWrongSegments << "]" << endl;
   cout << "Wrong segments = " << segWrong << " [" << fTracker->fNofWrongSegments << "/" << fTracker->fNofTrueSegments + fTracker->fNofWrongSegments << "]" << endl;
   cout << "Full reconstruction duration: " << fullDuration << " nanoseconds" << endl;
}

void CbmBinnedTrackerTask::SetParContainers()
{
   fSettings = static_cast<CbmBinnedSettings*> (FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmBinnedSettings"));
}

ClassImp(CbmBinnedTrackerTask)
