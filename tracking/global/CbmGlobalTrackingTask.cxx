/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmGlobalTrackingTask.h"
#include "FairLogger.h"

#ifdef CBM_GLOBALTB_QA
#include "CbmMCDataManager.h"
#include "CbmMCTrack.h"
#include "CbmStsPoint.h"
#include "CbmTrdPoint.h"
#include "CbmTofPoint.h"
#include "TH1F.h"

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

#endif//CBM_GLOBALTB_QA

using std::vector;
using std::list;
using std::cout;
using std::endl;
using std::set;

CbmGlobalTrackingTask::CbmGlobalTrackingTask() : fTofGeometry(), fTofHits(0)
#ifdef CBM_GLOBALTB_QA
, fTofHitDigiMatches(0), fTofDigis(0), fMCTracks(0), fStsMCPoints(0), fTrdMCPoints(0), fTofMCPoints(0),
   fNofEvents(1000), fTimeSlice(0), fEventList(0)
#endif//CBM_GLOBALTB_QA
, fTracks(), fTofPoints()
{
}

static TH1F* hitVSTrackDeltaT = 0;

InitStatus CbmGlobalTrackingTask::Init()
{
   if (!fTofGeometry.Read())
      fLogger->Fatal(MESSAGE_ORIGIN, "Could not read the ToF geometry information");
   
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fTofHits = static_cast<TClonesArray*> (ioman->GetObject("TofHit"));
   
   if (0 == fTofHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No ToF hits");
   
   fTofGeometry.SetTofHits(fTofHits);
   
#ifdef CBM_GLOBALTB_QA
   hitVSTrackDeltaT = new TH1F("hitVSTrackDeltaT", "hitVSTrackDeltaT", 1000, -1000., 1000.);
   
   fTofHitDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("TofDigiMatch"));
   
   if (0 == fTofHitDigiMatches)
      fLogger->Fatal(MESSAGE_ORIGIN, "No ToF hit digi matches");
   
   fTofDigis = static_cast<TClonesArray*> (ioman->GetObject("TofDigi"));
   
   if (0 == fTofDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No ToF digis");
       
   fTimeSlice = static_cast<CbmTimeSlice*> (ioman->GetObject("TimeSlice."));
   
   if (0 == fTimeSlice)
      fLogger->Fatal(MESSAGE_ORIGIN, "No time slice");
   
   fEventList = static_cast<CbmMCEventList*> (ioman->GetObject("MCEventList."));
   
   if (0 == fEventList)
      fLogger->Fatal(MESSAGE_ORIGIN, "No event list");
   
   CbmMCDataManager* mcManager = static_cast<CbmMCDataManager*> (ioman->GetObject("MCDataManager"));
   
   if (0 == mcManager)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC data manager");
   
   fMCTracks = mcManager->InitBranch("MCTrack");
   
   if (0 == fMCTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC tracks");
   
   fTracks.resize(fNofEvents);
   
   for (int i = 0; i < fNofEvents; ++i)
   {
      int nofTracks = fMCTracks->Size(0, i);
      
      if (nofTracks > 0)
         fTracks[i].resize(nofTracks);
      
      vector<TrackData>& evTracks = fTracks[i];
      
      for (int j = 0; j < nofTracks; ++j)
      {
         TrackData& track = evTracks[j];
         track.hasSts = false;
         track.z = -1000;
         track.use = false;
         track.evN = i;
         track.ind = j;
         track.used = false;
      }
   }
   
   fStsMCPoints = mcManager->InitBranch("StsPoint");
   
   if (0 == fStsMCPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No STS MC points");
   
   for (int i = 0; i < fNofEvents; ++i)
   {
      int nofStsPoints = fStsMCPoints->Size(0, i);
      
      for (int j = 0; j < nofStsPoints; ++j)
      {
         const CbmStsPoint* point = static_cast<const CbmStsPoint*> (fStsMCPoints->Get(0, i, j));
         Int_t trackId = point->GetTrackID();
         fTracks[i][trackId].hasSts = true;
      }
   }
   
   fTrdMCPoints = mcManager->InitBranch("TrdPoint");
   
   if (0 == fTrdMCPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No TRD MC points");
   
   for (int i = 0; i < fNofEvents; ++i)
   {
      int nofTrdPoints = fTrdMCPoints->Size(0, i);
      
      for (int j = 0; j < nofTrdPoints; ++j)
      {
         const CbmTrdPoint* point = static_cast<const CbmTrdPoint*> (fTrdMCPoints->Get(0, i, j));
         Double_t z = (point->GetZIn() + point->GetZOut()) / 2;
         Int_t trackId = point->GetTrackID();
         TrackData& track = fTracks[i][trackId];
         
         if (track.hasSts && track.z < z)
         {
            track.x = (point->GetXIn() + point->GetXOut()) / 2;
            track.y = (point->GetYIn() + point->GetYOut()) / 2;
            track.z = z;
            track.t = point->GetTime();
         }
      }
   }
   
   fTofMCPoints = mcManager->InitBranch("TofPoint");
   
   if (0 == fTofMCPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No ToF MC points");
   
   fTofPoints.resize(fNofEvents);
   
   for (int i = 0; i < fNofEvents; ++i)
   {
      int nofTofPoints = fTofMCPoints->Size(0, i);
      
      if (nofTofPoints > 0)
         fTofPoints[i].resize(nofTofPoints);
      
      vector<PointData>& evPoints = fTofPoints[i];
      
      for (int j = 0; j < nofTofPoints; ++j)
      {
         PointData& point = evPoints[j];
         point.evN = i;
         point.ind = j;
         const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*> (fTofMCPoints->Get(0, i, j));
         Int_t trackId = tofPoint->GetTrackID();
         TrackData& track = fTracks[i][trackId];
         point.track = &track;
         point.x = tofPoint->GetX();
         point.y = tofPoint->GetY();
         point.z = tofPoint->GetZ();
         point.t = tofPoint->GetTime();
         track.tofPoints.push_back(&point);
         
         if (track.z > 0 && !track.use)
         {
            double deltaZ = point.z - track.z;
            track.tx = (point.x - track.x) / deltaZ;
            track.ty = (point.y - track.y) / deltaZ;
            track.use = true;
         }
      }
   }
#endif//CBM_GLOBALTB_QA
   
   return kSUCCESS;
}

//static int startEventNumber = 0;
//static int endEventNumber = 0;

static int nofReferenceTracks = 0;
static int nofMergedTracks = 0;
static int nofMergedTracks2 = 0;

#ifdef CBM_GLOBALTB_QA
static long fullDuration = 0;
#endif//CBM_GLOBALTB_QA

void CbmGlobalTrackingTask::Exec(Option_t* opt)
{
   // This is a temporary code to emulate reconstructed STS tracks with MC tracks

   //Double_t endTime = fTimeSlice->GetEndTime();
   //Double_t duration = fTimeSlice->GetDuration();
   
   set<TrackData*> mcTracks;
   Int_t nofHits = fTofHits->GetEntriesFast();
   
   for (int i = 0; i < nofHits; ++i)
   {
      const CbmTofHit* tofHit = static_cast<const CbmTofHit*> (fTofHits->At(i));
      const CbmMatch* hitMatch = static_cast<const CbmMatch*> (fTofHitDigiMatches->At(i));
      int nofDigis = hitMatch->GetNofLinks();
      
      for (int j = 0; j < nofDigis; ++j)
      {
         const CbmLink& lnk = hitMatch->GetLink(j);
         Int_t digiInd = lnk.GetIndex();
         CbmTofDigiExp* pDigi = static_cast<CbmTofDigiExp*> (fTofDigis->At(digiInd));
         const CbmMatch* pPointMatch = pDigi->GetMatch();
         Int_t nofPoints = pPointMatch->GetNofLinks();
         
         for (Int_t k = 0; k < nofPoints; ++k)
         {
            const CbmLink& pointLnk = pPointMatch->GetLink(k);
            Int_t evN = pointLnk.GetEntry() - 1;
            Int_t pointInd = pointLnk.GetIndex();
            const PointData& point = fTofPoints[evN][pointInd];
            mcTracks.insert(point.track);
            point.track->tofHits.insert(tofHit);
         }
      }
   }
   
   Double_t startTime = fTimeSlice->GetStartTime();
   fTofGeometry.Prepare(startTime);
   
   for (set<TrackData*>::iterator i = mcTracks.begin(); i != mcTracks.end(); ++i)
   {
      TrackData* track = *i;
      
      if (!track->use || track->used)
         continue;
      
      ++nofReferenceTracks;
      track->used = true;
      Double_t eventTime = fEventList->GetEventTime(track->evN + 1);         
      track->t += eventTime;
      
      for (set<const CbmTofHit*>::const_iterator j = track->tofHits.begin(); j != track->tofHits.end(); ++j)
      {
         const CbmTofHit* tofHit = *j;
         hitVSTrackDeltaT->Fill(tofHit->GetTime() - track->t/* - eventTime*/);
      }
      
#ifdef CBM_GLOBALTB_QA
      timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      long beginTime = ts.tv_sec * 1000000000 + ts.tv_nsec;
#endif//CBM_GLOBALTB_QA
      
      Int_t tofHitInd;
      fTofGeometry.Find(track->x, 2, track->y, 2, track->z, /*eventTime + */track->t, 4, track->tx, 0.01, track->ty, 0.01, tofHitInd);
      
#ifdef CBM_GLOBALTB_QA
      clock_gettime(CLOCK_REALTIME, &ts);
      long endTime = ts.tv_sec * 1000000000 + ts.tv_nsec;
      fullDuration += endTime - beginTime;
#endif//CBM_GLOBALTB_QA
         
      if (tofHitInd < 0)
         continue;
      
      ++nofMergedTracks2;
         
      const CbmMatch* hitMatch = static_cast<const CbmMatch*> (fTofHitDigiMatches->At(tofHitInd));
      int nofDigis = hitMatch->GetNofLinks();
      bool isMatch = false;

      for (int j = 0; j < nofDigis; ++j)
      {
         const CbmLink& lnk = hitMatch->GetLink(j);
         Int_t digiInd = lnk.GetIndex();
         CbmTofDigiExp* pDigi = static_cast<CbmTofDigiExp*> (fTofDigis->At(digiInd));
         
         const CbmMatch* pPointMatch = pDigi->GetMatch();
         Int_t nofPoints = pPointMatch->GetNofLinks();
         
         for (Int_t k = 0; k < nofPoints; ++k)
         {
            const CbmLink& pointLnk = pPointMatch->GetLink(k);
            Int_t evN = pointLnk.GetEntry() - 1;
            Int_t pointInd = pointLnk.GetIndex();

            for (list<PointData*>::const_iterator l = track->tofPoints.begin(); l != track->tofPoints.end(); ++l)
            {
               PointData* point = *l;
                  
               if (point->evN == evN && point->ind == pointInd)
               {
                  isMatch = true;
                  break;
               }
            }
               
            if (isMatch)
               break;
         }
            
         if (isMatch)
            break;
      }
         
      if (isMatch)
         ++nofMergedTracks;
   }
}

static void SaveHisto(TH1* histo)
{
   TFile* curFile = TFile::CurrentFile();
   TString histoName = histo->GetName();
   histoName += ".root";
   TFile fh(histoName.Data(), "RECREATE");
   histo->Write();
   fh.Close();
   delete histo;
   TFile::CurrentFile() = curFile;
}

extern int nofMRPCIntersections;
extern int nofToFIntersections;
extern int nofMRPCIntersectionsT;
extern int globalNofHits;
extern int globalNofHitsT;
extern int globalNofHitsM;

void CbmGlobalTrackingTask::Finish()
{
   cout << "Nof reference tracks: " << nofReferenceTracks << endl;
   cout << "Nof merged track: " << nofMergedTracks << endl;
   cout << "Nof merged track2: " << nofMergedTracks2 << endl;
   double eff = 100 * nofMergedTracks;
   eff /= nofReferenceTracks;
   cout << "The efficiency: " << eff << " %" << endl;
   eff = 100 * nofMergedTracks2;
   eff /= nofReferenceTracks;
   cout << "The efficiency2: " << eff << " %" << endl;
   cout << "nofMRPCIntersections: " << nofMRPCIntersections << endl;
   cout << "nofToFIntersections: " << nofToFIntersections << endl;
   cout << "nofMRPCIntersectionsT: " << nofMRPCIntersectionsT << endl;
   cout << "globalNofHits: " << globalNofHits << endl;
   cout << "globalNofHitsT: " << globalNofHitsT << endl;
   cout << "globalNofHitsM: " << globalNofHitsM << endl << endl << endl;
   cout << "Full merging duration was: " << fullDuration << endl << endl << endl;
   SaveHisto(hitVSTrackDeltaT);
}

ClassImp(CbmGlobalTrackingTask)