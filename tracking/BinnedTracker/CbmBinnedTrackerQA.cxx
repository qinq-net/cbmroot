/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmBinnedTrackerQA.h"
#include "global/CbmGlobalTrack.h"
#include "CbmMCTrack.h"
#include "CbmStsPoint.h"
#include "setup/CbmStsSetup.h"
#include "CbmMuchPoint.h"
#include "geo/CbmMuchGeoScheme.h"
#include "CbmTrdPoint.h"
#include "trd/CbmTrdAddress.h"
#include "CbmTofPoint.h"
#include "CbmStsHit.h"
#include "CbmStsDigi.h"
#include "CbmMatch.h"

using namespace std;

#define NOF_STS_STATIONS 2
#define NOF_MUCH_STATIONS 1
#define NOF_MUCH_LAYERS 3
#define NOF_TRD_LAYERS 4
#define NOF_TOF 1

struct TrackDesc
{
   pair<set<Int_t>, set<pair<Int_t, Int_t> > > sts[NOF_STS_STATIONS];
   pair<set<Int_t>, set<pair<Int_t, Int_t> > > much[NOF_MUCH_STATIONS * NOF_MUCH_LAYERS];
   pair<set<Int_t>, set<pair<Int_t, Int_t> > > trd[NOF_TRD_LAYERS];
   pair<set<Int_t>, set<pair<Int_t, Int_t> > > tof;
   
   TrackDesc() : tof() {}
};

static vector<vector<TrackDesc> > gTracks;

InitStatus CbmBinnedTrackerQA::Init()
{
   for (Int_t i = 0; fMCTracks->Size(0, i) >= 0; ++i)
   {
      gTracks.push_back(vector<TrackDesc>());
      Int_t nofMcTracks = fMCTracks->Size(0, i);
      gTracks.back().resize(nofMcTracks);
   }
   
   for (Int_t i = 0; fStsPoints->Size(0, i) >= 0; ++i)
   {
      Int_t nofPoints = fStsPoints->Size(0, i);
      vector<TrackDesc>& tracks = gTracks[i];
      
      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmStsPoint* stsPoint = static_cast<const CbmStsPoint*> (fStsPoints->Get(0, i, j));
         Int_t trackId = stsPoint->GetTrackID();
         Int_t stationNumber = CbmStsSetup::Instance()->GetStationNumber(stsPoint->GetDetectorID());
         tracks[trackId].sts[stationNumber].first.insert(j);
      }
   }
   
   for (Int_t i = 0; fMuchPoints->Size(0, i) >= 0; ++i)
   {
      Int_t nofPoints = fMuchPoints->Size(0, i);
      vector<TrackDesc>& tracks = gTracks[i];
      
      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmMuchPoint* muchPoint = static_cast<const CbmMuchPoint*> (fMuchPoints->Get(0, i, j));
         Int_t trackId = muchPoint->GetTrackID();
         int muchStationNumber = CbmMuchGeoScheme::GetStationIndex(muchPoint->GetDetectorID());
         int layerNumber = CbmMuchGeoScheme::GetLayerIndex(muchPoint->GetDetectorID());
         int stationNumber = muchStationNumber * NOF_MUCH_LAYERS + layerNumber;
         tracks[trackId].much[stationNumber].first.insert(j);
      }
   }
   
   for (Int_t i = 0; fTrdPoints->Size(0, i) >= 0; ++i)
   {
      Int_t nofPoints = fTrdPoints->Size(0, i);
      vector<TrackDesc>& tracks = gTracks[i];
      
      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, i, j));
         Int_t trackId = trdPoint->GetTrackID();
         int stationNumber = CbmTrdAddress::GetLayerId(trdPoint->GetModuleAddress());
         tracks[trackId].trd[stationNumber].first.insert(j);
      }
   }
   
   for (Int_t i = 0; fTofPoints->Size(0, i) >= 0; ++i)
   {
      Int_t nofPoints = fTofPoints->Size(0, i);
      vector<TrackDesc>& tracks = gTracks[i];
      
      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*> (fTofPoints->Get(0, i, j));
         Int_t trackId = tofPoint->GetTrackID();
         tracks[trackId].tof.first.insert(j);
      }
   }
   
   return kSUCCESS;
}

static Int_t gExecNumber = 0;

void CbmBinnedTrackerQA::Exec(Option_t* opt)
{
   Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
   
   for (Int_t i = 0; i < nofGlobalTracks; ++i)
   {
      const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*> (fGlobalTracks->At(i));
      Int_t stsIndex = globalTrack->GetStsTrackIndex();
      Int_t muchIndex = globalTrack->GetMuchTrackIndex();
      Int_t trdIndex = globalTrack->GetTrdTrackIndex();
      Int_t tofIndex = globalTrack->GetTofHitIndex();
      
      if (stsIndex < 0 || muchIndex < 0 || trdIndex < 0 || tofIndex < 0)
         continue;
      
      HandleSts(stsIndex);
      HandleMuch(static_cast<const CbmMuchTrack*> (fMuchTracks->At(muchIndex)));
      HandleTrd(static_cast<const CbmTrdTrack*> (fTrdTracks->At(trdIndex)));
      HandleTof(static_cast<const CbmTofHit*> (fTofHits->At(tofIndex)));
   }
   
   ++gExecNumber;
}

void CbmBinnedTrackerQA::HandleSts(Int_t stsTrackIndex)
{
   const CbmStsTrack* stsTrack = static_cast<const CbmStsTrack*> (fStsTracks->At(stsTrackIndex));
   Int_t nofStsHits = stsTrack->GetNofHits();
   
   for (Int_t i = 0; i < nofStsHits; ++i)
   {
      Int_t stsHitInd = stsTrack->GetStsHitIndex(i);
      const CbmStsHit* stsHit = static_cast<const CbmStsHit*> (fStsHits->At(stsHitInd));
      Int_t stationNumber = CbmStsSetup::Instance()->GetStationNumber(stsHit->GetAddress());
      Int_t frontClusterInd = stsHit->GetFrontClusterId();
      Int_t backClusterInd = stsHit->GetBackClusterId();
      const CbmStsCluster* frontCluster = static_cast<const CbmStsCluster*> (fStsClusters->At(frontClusterInd));
      Int_t nofFrontDigis = frontCluster->GetNofDigis();

      for (Int_t j = 0; j < nofFrontDigis; ++j)
      {
         Int_t stsDigiInd = frontCluster->GetDigi(j);
         const CbmStsDigi* stsDigi = static_cast<const CbmStsDigi*> (fStsDigis->At(stsDigiInd));
         const CbmMatch* match = stsDigi->GetMatch();
         Int_t nofLinks = match->GetNofLinks();

         for (Int_t k = 0; k < nofLinks; ++k)
         {
            const CbmLink& link = match->GetLink(k);
            Int_t eventId = link.GetEntry();
            Int_t mcPointId = link.GetIndex();
            const CbmStsPoint* stsPoint = static_cast<const CbmStsPoint*> (fStsPoints->Get(0, eventId, mcPointId));
            Int_t trackId = stsPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            
            if (trackDesk.sts[stationNumber].first.find(mcPointId) != trackDesk.sts[stationNumber].first.end())
               trackDesk.sts[stationNumber].second.insert(make_pair(gExecNumber, stsTrackIndex));
         }
      }
      
      const CbmStsCluster* backCluster = static_cast<const CbmStsCluster*> (fStsClusters->At(backClusterInd));
      Int_t nofBackDigis = backCluster->GetNofDigis();

      for (Int_t j = 0; j < nofBackDigis; ++j)
      {
         Int_t stsDigiInd = backCluster->GetDigi(j);
         const CbmStsDigi* stsDigi = static_cast<const CbmStsDigi*> (fStsDigis->At(stsDigiInd));
         const CbmMatch* match = stsDigi->GetMatch();
         Int_t nofLinks = match->GetNofLinks();

         for (Int_t k = 0; k < nofLinks; ++k)
         {
            const CbmLink& link = match->GetLink(k);
            Int_t eventId = link.GetEntry();
            Int_t mcPointId = link.GetIndex();
            const CbmStsPoint* stsPoint = static_cast<const CbmStsPoint*> (fStsPoints->Get(0, eventId, mcPointId));
            Int_t trackId = stsPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            
            if (trackDesk.sts[stationNumber].first.find(mcPointId) != trackDesk.sts[stationNumber].first.end())
               trackDesk.sts[stationNumber].second.insert(make_pair(gExecNumber, stsTrackIndex));
         }
      }
   }
}

void CbmBinnedTrackerQA::HandleMuch(const CbmMuchTrack* muchTrack)
{
   
}

void CbmBinnedTrackerQA::HandleTrd(const CbmTrdTrack* trdTrack)
{
   
}

void CbmBinnedTrackerQA::HandleTof(const CbmTofHit* tofHit)
{
   
}

void CbmBinnedTrackerQA::Finish()
{
   
}

ClassImp(CbmBinnedTrackerQA)
