/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmGlobalTrackingQA.h"
#include "FairLogger.h"
#include "CbmMCDataManager.h"
#include "CbmMCTrack.h"
#include "CbmStsPoint.h"
#include "CbmTofPoint.h"
#include "CbmStsTrack.h"
#include "CbmStsHit.h"
#include "CbmStsCluster.h"
#include "CbmGlobalTrack.h"
#include "CbmStsDigi.h"
#include <map>

using std::list;
using std::set;
using std::vector;
using std::map;
using std::cout;
using std::endl;

CbmGlobalTrackingQA::CbmGlobalTrackingQA() : fTofHits(0), fStsTracks(0), fGlobalTracks(0), fTofHitDigiMatches(0), fTofDigis(0),
   fStsHitDigiMatches(0), fStsHits(0), fStsClusters(0), fStsDigis(0), fMCTracks(0), fStsMCPoints(0), fTrdMCPoints(0), fTofMCPoints(0),
   fTracks(), fTofPoints(), fStsPoints(), fNofEvents(1000)
{
}

InitStatus CbmGlobalTrackingQA::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fTofHits = static_cast<TClonesArray*> (ioman->GetObject("TofHit"));
   
   if (0 == fTofHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No ToF hits");
   
   fStsTracks = static_cast<TClonesArray*> (ioman->GetObject("StsTrack"));
   
   if (0 == fStsTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No STS tracks");
   
   fGlobalTracks = static_cast<TClonesArray*> (ioman->GetObject("GlobalTrack"));
   
   if (0 == fGlobalTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No global tracks");
   
   fTofHitDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("TofDigiMatch"));
   
   if (0 == fTofHitDigiMatches)
      fLogger->Fatal(MESSAGE_ORIGIN, "No ToF hit digi matches");
   
   fTofDigis = static_cast<TClonesArray*> (ioman->GetObject("TofDigiExp"));
   
   if (0 == fTofDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No ToF digis");
   
   fStsHits = static_cast<TClonesArray*> (ioman->GetObject("StsHit"));
   
   if (0 == fStsHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No STS hits");
   
   fStsClusters = static_cast<TClonesArray*> (ioman->GetObject("StsCluster"));
   
   if (0 == fStsClusters)
      fLogger->Fatal(MESSAGE_ORIGIN, "No STS clusters");
   
   fStsDigis = static_cast<TClonesArray*> (ioman->GetObject("StsDigi"));
   
   if (0 == fStsDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No STS digis");
   
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
         const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*> (fMCTracks->Get(0, i, j));
         int motherId = mcTrack->GetMotherId();
         
         if (motherId < 0)
            track.parent = 0;
         else
         {
            track.parent = &evTracks[motherId];
            evTracks[motherId].offsprings.push_back(&track);
         }
      }
   }
   
   fStsMCPoints = mcManager->InitBranch("StsPoint");
   
   if (0 == fStsMCPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No STS MC points");
   
   fStsPoints.resize(fNofEvents);
   
   for (int i = 0; i < fNofEvents; ++i)
   {
      int nofStsPoints = fStsMCPoints->Size(0, i);
      
      if (nofStsPoints > 0)
         fStsPoints[i].resize(nofStsPoints);
      
      vector<PointData>& evPoints = fStsPoints[i];
      
      for (int j = 0; j < nofStsPoints; ++j)
      {
         PointData& point = evPoints[j];
         point.evN = i;
         point.ind = j;
         const CbmStsPoint* stsPoint = static_cast<const CbmStsPoint*> (fStsMCPoints->Get(0, i, j));
         Int_t trackId = stsPoint->GetTrackID();
         fTracks[i][trackId].hasSts = true;
         TrackData& track = fTracks[i][trackId];
         point.track = &track;
         point.x = (stsPoint->GetXIn() + stsPoint->GetXOut()) / 2;
         point.y = (stsPoint->GetYIn() + stsPoint->GetYOut()) / 2;
         point.z = (stsPoint->GetZIn() + stsPoint->GetZOut()) / 2;
         point.t = stsPoint->GetTime();
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
   
   return kSUCCESS;
}

bool CbmGlobalTrackingQA::CheckMatch(const TrackData* stsMCTrack, Int_t tofHitInd, bool deepSearch) const
{
   set<TrackData*> tofMCTracks;

   const CbmMatch* tofHitMatch = static_cast<const CbmMatch*> (fTofHitDigiMatches->At(tofHitInd));
   int nofTofDigis = tofHitMatch->GetNofLinks();

   for (int i = 0; i < nofTofDigis; ++i)
   {
      const CbmLink& lnk = tofHitMatch->GetLink(i);
      Int_t digiInd = lnk.GetIndex();
      CbmTofDigiExp* pDigi = static_cast<CbmTofDigiExp*> (fTofDigis->At(digiInd));

      const CbmMatch* pPointMatch = pDigi->GetMatch();
      Int_t nofPoints = pPointMatch->GetNofLinks();

      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmLink& pointLnk = pPointMatch->GetLink(j);
         Int_t evN = pointLnk.GetEntry() - 1;
         Int_t pointInd = pointLnk.GetIndex();
         tofMCTracks.insert(fTofPoints[evN][pointInd].track);
      }
   }

   bool isMatched = false;

   for (set<TrackData*>::const_iterator i = tofMCTracks.begin(); i != tofMCTracks.end(); ++i)
   {
      const TrackData* tofMCTrack = *i;

      if (deepSearch)
      {
         for (const TrackData* mcTrack = tofMCTrack; 0 != mcTrack; mcTrack = mcTrack->parent)
         {
            if (mcTrack == stsMCTrack)
               return true;
         }
      }
      else if (tofMCTrack == stsMCTrack)
         return true;
   }
   
   return false;
}

bool CbmGlobalTrackingQA::SemiTofTrack(const TrackData* mcTrack) const
{
   if (!mcTrack->tofPoints.empty())
      return true;
   
   for (list<TrackData*>::const_iterator i = mcTrack->offsprings.begin(); i != mcTrack->offsprings.end(); ++i)
   {
      if (SemiTofTrack(*i))
         return true;
   }
   
   return false;
}

int nofReferenceTofTracks = 0;
int nofCorMergedReferenceTofTracks = 0;
int nofWroMergedReferenceTofTracks = 0;
int nofNonMergedReferenceTofTracks = 0;

int nofSemiTofTracks = 0;
int nofCorMergedSemiTofTracks = 0;
int nofWroMergedSemiTofTracks = 0;
int nofNonMergedSemiTofTracks = 0;

int nofNonTofTracks = 0;
int nofMergedNonTofTracks = 0;
int nofNonMergedNonTofTracks = 0;

void CbmGlobalTrackingQA::Exec(Option_t* opt)
{
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
            point.track->tofHits.insert(tofHit);
         }
      }
   }//for (int i = 0; i < nofHits; ++i)
   
   int nofGlobalTracks = fGlobalTracks->GetEntriesFast();
   
   for (int i = 0; i < nofGlobalTracks; ++i)
   {
      const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*> (fGlobalTracks->At(i));
      int stsTrackInd = globalTrack->GetStsTrackIndex();
      
      if (stsTrackInd < 0)
         continue;
      
      const CbmStsTrack* stsTrack = static_cast<const CbmStsTrack*> (fStsTracks->At(stsTrackInd));
      map<TrackData*, int> stsMCTracks;
      int nofStsHits = stsTrack->GetNofHits();
      int nofStsMatches = 0;
      
      for (int j = 0; j < nofStsHits; ++j)
      {
         int stsHitInd = stsTrack->GetHitIndex(j);
         const CbmStsHit* stsHit = static_cast<const CbmStsHit*> (fStsHits->At(stsHitInd));
         int frontClusterInd = stsHit->GetFrontClusterId();
         int backClusterInd = stsHit->GetBackClusterId();
         const CbmStsCluster* frontCluster = static_cast<const CbmStsCluster*> (fStsClusters->At(frontClusterInd));
         const CbmStsCluster* backCluster = static_cast<const CbmStsCluster*> (fStsClusters->At(backClusterInd));
         int nofFrontDigis = frontCluster->GetNofDigis();
         
         for (int k = 0; k < nofFrontDigis; ++k)
         {
            int stsDigiInd = frontCluster->GetDigi(k);
            const CbmStsDigi* stsDigi = static_cast<const CbmStsDigi*> (fStsDigis->At(stsDigiInd));
            const CbmMatch* match = stsDigi->GetMatch();
            int nofLinks = match->GetNofLinks();
            
            for (int l = 0; l < nofLinks; ++l)
            {
               const CbmLink& link = match->GetLink(l);
               int eventId = link.GetEntry();
               int mcPointId = link.GetIndex();
               ++nofStsMatches;
               map<TrackData*, int>::iterator iter = stsMCTracks.find(fStsPoints[eventId][mcPointId].track);
               
               if (iter != stsMCTracks.end())
                  ++iter->second;
               else
                  stsMCTracks[fStsPoints[eventId][mcPointId].track] = 1;
            }
         }
         
         int nofBackDigis = backCluster->GetNofDigis();
         
         for (int k = 0; k < nofBackDigis; ++k)
         {
            int stsDigiInd = backCluster->GetDigi(k);
            const CbmStsDigi* stsDigi = static_cast<const CbmStsDigi*> (fStsDigis->At(stsDigiInd));
            const CbmMatch* stsMatch = stsDigi->GetMatch();
            int nofLinks = stsMatch->GetNofLinks();
            
            for (int l = 0; l < nofLinks; ++l)
            {
               const CbmLink& link = stsMatch->GetLink(l);
               int eventId = link.GetEntry();
               int mcPointId = link.GetIndex();
               ++nofStsMatches;
               map<TrackData*, int>::iterator iter = stsMCTracks.find(fStsPoints[eventId][mcPointId].track);
               
               if (iter != stsMCTracks.end())
                  ++iter->second;
               else
                  stsMCTracks[fStsPoints[eventId][mcPointId].track] = 1;
            }
         }
      }// for (int j = 0; j < nofStsHits; ++j)
      
      if (stsMCTracks.empty())
         continue;
      
      map<TrackData*, int>::iterator lastStsTrackIter = stsMCTracks.end();
      --lastStsTrackIter;
      
      if (lastStsTrackIter->second < 0.7 * nofStsMatches)
         continue;
      
      int tofHitInd = globalTrack->GetTofHitIndex();
      
      if (lastStsTrackIter->first->tofPoints.empty())
      {         
         if (SemiTofTrack(lastStsTrackIter->first))
         {
            ++nofSemiTofTracks;
            
            if (tofHitInd >= 0)
            {               
               if (CheckMatch(lastStsTrackIter->first, tofHitInd, true))
                  ++nofCorMergedSemiTofTracks;
               else
                  ++nofWroMergedSemiTofTracks;
            }
            else
               ++nofNonMergedSemiTofTracks;
         }
         else
         {
            ++nofNonTofTracks;
         
            if (tofHitInd >= 0)
               ++nofMergedNonTofTracks;
            else
               ++nofNonMergedNonTofTracks;
         }
      }
      
      if (lastStsTrackIter->first->tofPoints.empty() || lastStsTrackIter->first->tofHits.empty())
         continue;
      
      ++nofReferenceTofTracks;
      
      if (tofHitInd < 0)
      {
         ++nofNonMergedReferenceTofTracks;
         continue;
      }
      
      if (CheckMatch(lastStsTrackIter->first, tofHitInd))
         ++nofCorMergedReferenceTofTracks;
      else
         ++nofWroMergedReferenceTofTracks;
   }//for (int i = 0; i < nofGlobalTracks; ++i)
}

void CbmGlobalTrackingQA::Finish()
{
   double eff = 100 * nofCorMergedReferenceTofTracks;
   eff /= nofReferenceTofTracks;
   cout << "The correctly merged reference tracks: " << eff << " % [" << nofCorMergedReferenceTofTracks << " / " << nofReferenceTofTracks << " ]" << endl;
   eff = 100 * nofWroMergedReferenceTofTracks;
   eff /= nofReferenceTofTracks;
   cout << "The incorrectly merged reference tracks: " << eff << " % [" << nofWroMergedReferenceTofTracks << " / " << nofReferenceTofTracks << " ]" << endl;
   eff = 100 * nofNonMergedReferenceTofTracks;
   eff /= nofReferenceTofTracks;
   cout << "The non merged reference tracks: " << eff << " % [" << nofNonMergedReferenceTofTracks << " / " << nofReferenceTofTracks << " ]" << endl;
   
   eff = 100 * nofCorMergedSemiTofTracks;
   eff /= nofSemiTofTracks;
   cout << "The correctly merged semi ToF tracks: " << eff << " % [" << nofCorMergedSemiTofTracks << " / " << nofSemiTofTracks << " ]" << endl;
   eff = 100 * nofWroMergedSemiTofTracks;
   eff /= nofSemiTofTracks;
   cout << "The incorrectly merged semi ToF tracks: " << eff << " % [" << nofWroMergedSemiTofTracks << " / " << nofSemiTofTracks << " ]" << endl;
   eff = 100 * nofNonMergedReferenceTofTracks;
   eff /= nofSemiTofTracks;
   cout << "The non merged semi ToF tracks: " << eff << " % [" << nofNonMergedReferenceTofTracks << " / " << nofSemiTofTracks << " ]" << endl;
   
   eff = 100 * nofMergedNonTofTracks;
   eff /= nofNonTofTracks;
   cout << "The merged non ToF tracks: " << eff << " % [" << nofMergedNonTofTracks << " / " << nofNonTofTracks << " ]" << endl;
   eff = 100 * nofNonMergedNonTofTracks;
   eff /= nofNonTofTracks;
   cout << "The non merged non ToF tracks: " << eff << " % [" << nofNonMergedNonTofTracks << " / " << nofNonTofTracks << " ]" << endl;
}

ClassImp(CbmGlobalTrackingQA)
