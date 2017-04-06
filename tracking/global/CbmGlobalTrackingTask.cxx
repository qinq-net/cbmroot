/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmGlobalTrackingTask.h"
#include "FairLogger.h"
#include "TH1F.h"
#include "CbmStsTrack.h"
#include "CbmStsHit.h"
#include "CbmStsCluster.h"
#include "CbmGlobalTrack.h"

#define CBM_GLOBALTB_PRINT_PERF

#ifdef CBM_GLOBALTB_PRINT_PERF
#ifdef __MACH__
#include <mach/mach_time.h>
#include <sys/time.h>
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 0
inline int clock_gettime(int clk_id, struct timespec *t){
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
#endif//CBM_GLOBALTB_PRINT_PERF

using std::vector;
using std::list;
using std::cout;
using std::endl;
using std::set;
using std::map;

CbmGlobalTrackingTask::CbmGlobalTrackingTask() : fTofGeometry(), fTimeSlice(0), fTofHits(0), fStsTracks(0), fGlobalTracks(0)
{
}

InitStatus CbmGlobalTrackingTask::Init()
{
   if (!fTofGeometry.Read())
      fLogger->Fatal(MESSAGE_ORIGIN, "Could not read the ToF geometry information");
   
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fTimeSlice = static_cast<CbmTimeSlice*> (ioman->GetObject("TimeSlice."));

   if (0 == fTimeSlice)
      fLogger->Fatal(MESSAGE_ORIGIN, "No time slice");

   fTofHits = static_cast<TClonesArray*> (ioman->GetObject("TofHit"));
   
   if (0 == fTofHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No ToF hits");
   
   fTofGeometry.SetTofHits(fTofHits);
   
   fStsTracks = static_cast<TClonesArray*> (ioman->GetObject("StsTrack"));
   
   if (0 == fStsTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No STS tracks");
   
   fGlobalTracks = new TClonesArray("CbmGlobalTrack",100);
   ioman->Register("GlobalTrack", "Global", fGlobalTracks, IsOutputBranchPersistent("GlobalTrack"));
   
   return kSUCCESS;
}

#ifdef CBM_GLOBALTB_PRINT_PERF
static long fullDuration = 0;
#endif//CBM_GLOBALTB_PRINT_PERF

void CbmGlobalTrackingTask::Exec(Option_t* opt)
{
   fGlobalTracks->Clear();
   Double_t startTime = fTimeSlice->GetStartTime();
   fTofGeometry.Prepare(startTime);
   int nofStsTracks = fStsTracks->GetEntriesFast();
   
   for (int i = 0; i < nofStsTracks; ++i)
   {
      const CbmStsTrack* track = static_cast<const CbmStsTrack*> (fStsTracks->At(i));
      CbmGlobalTrack* globalTrack = new ((*fGlobalTracks)[i]) CbmGlobalTrack();
      globalTrack->SetStsTrackIndex(i);
      Int_t tofHitInd;
      const FairTrackParam* param = track->GetParamLast();
#ifdef CBM_GLOBALTB_PRINT_PERF
      timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      long beginTime = ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif//CBM_GLOBALTB_PRINT_PERF
      //fTofGeometry.Find(param->GetX(), param->GetCovariance(0, 0), param->GetY(), param->GetCovariance(1, 1), param->GetZ(),
         //track->GetTime(), track->GetTimeError(), param->GetTx(), TMath::Sqrt(param->GetCovariance(2, 2)), param->GetTy(), TMath::Sqrt(param->GetCovariance(3, 3)), tofHitInd);
      fTofGeometry.Find(*param, track->GetTime(), track->GetTimeError(), tofHitInd);
      globalTrack->SetTofHitIndex(tofHitInd);
#ifdef CBM_GLOBALTB_PRINT_PERF
      clock_gettime(CLOCK_REALTIME, &ts);
      long endTime = ts.tv_sec * 1000000000 + ts.tv_nsec;
      fullDuration += endTime - beginTime;
#endif//CBM_GLOBALTB_PRINT_PERF
   }//for (int i = 0; i < nofStsTracks; ++i)
}

extern int nofMRPCIntersections;
extern int nofToFIntersections;
extern int nofMRPCIntersectionsT;
extern int globalNofHits;
extern int globalNofHitsT;
extern int globalNofHitsM;

void CbmGlobalTrackingTask::Finish()
{
   cout << "nofMRPCIntersections: " << nofMRPCIntersections << endl;
   cout << "nofToFIntersections: " << nofToFIntersections << endl;
   cout << "nofMRPCIntersectionsT: " << nofMRPCIntersectionsT << endl;
   cout << "globalNofHits: " << globalNofHits << endl;
   cout << "globalNofHitsT: " << globalNofHitsT << endl;
   cout << "globalNofHitsM: " << globalNofHitsM << endl << endl << endl;
   cout << "Full merging duration was: " << fullDuration << endl << endl << endl;
}

ClassImp(CbmGlobalTrackingTask)
