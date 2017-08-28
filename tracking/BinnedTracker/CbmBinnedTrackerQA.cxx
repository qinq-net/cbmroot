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
#include "CbmMuchPixelHit.h"
#include "CbmMuchCluster.h"
#include "CbmMuchDigi.h"
#include "CbmTrdHit.h"
#include "CbmTrdCluster.h"
#include "CbmTrdDigi.h"
#include "CbmTofDigiExp.h"
#include "CbmMCDataManager.h"

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

CbmBinnedTrackerQA::CbmBinnedTrackerQA() : fGlobalTracks(0), fStsTracks(0), fMuchTracks(0), fTrdTracks(0), fStsHits(0), fMuchHits(0), fTrdHits(0), fTofHits(0),
   fStsClusters(0), fMuchClusters(0), fTrdClusters(0), fTofHitDigiMatches(0), fStsDigis(0), fMuchDigis(0), fTrdDigis(0), fTofDigis(0),
   fMCTracks(0), fStsPoints(0), fMuchPoints(0), fTrdPoints(0), fTofPoints(0)
{
}

InitStatus CbmBinnedTrackerQA::Init()
{
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fGlobalTracks = static_cast<TClonesArray*> (ioman->GetObject("GlobalTrack"));
   
   if (0 == fGlobalTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No global tracks in the input file");
   
   fStsTracks = static_cast<TClonesArray*> (ioman->GetObject("StsTrack"));
   
   if (0 == fStsTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts tracks in the input file");
   
   fMuchTracks = static_cast<TClonesArray*> (ioman->GetObject("MuchTrack"));
   
   if (0 == fMuchTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much tracks in the input file");
   
   fTrdTracks = static_cast<TClonesArray*> (ioman->GetObject("TrdTrack"));
   
   if (0 == fTrdTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No trd tracks in the input file");
   
   fStsHits = static_cast<TClonesArray*> (ioman->GetObject("StsHit"));
   
   if (0 == fStsHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts hits in the input file");
   
   fMuchHits = static_cast<TClonesArray*> (ioman->GetObject("MuchHit"));
   
   if (0 == fMuchHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much hits in the input file");
   
   fTrdHits = static_cast<TClonesArray*> (ioman->GetObject("TrdHit"));
   
   if (0 == fTrdHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No trd hits in the input file");
   
   fTofHits = static_cast<TClonesArray*> (ioman->GetObject("TofHit"));
   
   if (0 == fTofHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No tof hits in the input file");
   
   fStsClusters = static_cast<TClonesArray*> (ioman->GetObject("StsCluster"));
   
   if (0 == fStsClusters)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts clusters in the input file");
   
   fMuchClusters = static_cast<TClonesArray*> (ioman->GetObject("MuchCluster"));
   
   if (0 == fMuchClusters)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much clusters in the input file");
   
   fTrdClusters = static_cast<TClonesArray*> (ioman->GetObject("TrdCluster"));
   
   if (0 == fTrdClusters)
      fLogger->Fatal(MESSAGE_ORIGIN, "No global tracks in the input file");
   
   fTofHitDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("TofDigiMatch"));
   
   if (0 == fTofHitDigiMatches)
      fLogger->Fatal(MESSAGE_ORIGIN, "No tof hit to digi matches in the input file");
   
   fStsDigis = static_cast<TClonesArray*> (ioman->GetObject("StsDigi"));
   
   if (0 == fStsDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts digis in the input file");
   
   fMuchDigis = static_cast<TClonesArray*> (ioman->GetObject("MuchDigi"));
   
   if (0 == fMuchDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much digis in the input file");
   
   fTrdDigis = static_cast<TClonesArray*> (ioman->GetObject("TrdDigi"));
   
   if (0 == fTrdDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No trd digis in the input file");
   
   fTofDigis = static_cast<TClonesArray*> (ioman->GetObject("TofDigiExp"));
   
   if (0 == fTofDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No tof digis in the input file");
   
   CbmMCDataManager* mcManager = static_cast<CbmMCDataManager*> (ioman->GetObject("MCDataManager"));
   
   if (0 == mcManager)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC data manager");
   
   fMCTracks = mcManager->InitBranch("MCTrack");
   
   if (0 == fMCTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC tracks in the input file");
   
   fStsPoints = mcManager->InitBranch("StsPoint");
   
   if (0 == fStsPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts MC points in the input file");
   
   fMuchPoints = mcManager->InitBranch("MuchPoint");
   
   if (0 == fMuchPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much MC points in the input file");
   
   fTrdPoints = mcManager->InitBranch("TrdPoint");
   
   if (0 == fTrdPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No trd MC points in the input file");
   
   fTofPoints = mcManager->InitBranch("TofPoint");
   
   if (0 == fTofPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No tof MC points in the input file");
    
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
      HandleMuch(muchIndex);
      HandleTrd(trdIndex);
      HandleTof(tofIndex);
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

void CbmBinnedTrackerQA::HandleMuch(Int_t muchTrackIndex)
{
   const CbmMuchTrack* muchTrack = static_cast<const CbmMuchTrack*> (fMuchTracks->At(muchTrackIndex));
   Int_t nofMuchHits = muchTrack->GetNofHits();
   
   for (Int_t i = 0; i < nofMuchHits; ++i)
   {
      Int_t muchHitInd = muchTrack->GetHitIndex(i);
      const CbmMuchPixelHit* muchHit = static_cast<const CbmMuchPixelHit*> (fMuchHits->At(muchHitInd));
      Int_t muchStationNumber = CbmMuchGeoScheme::GetStationIndex(muchHit->GetAddress());
      Int_t layerNumber = CbmMuchGeoScheme::GetLayerIndex(muchHit->GetAddress());
      Int_t stationNumber = muchStationNumber * NOF_MUCH_LAYERS + layerNumber;
      Int_t clusterId = muchHit->GetRefId();
      const CbmMuchCluster* cluster = static_cast<const CbmMuchCluster*> (fMuchClusters->At(clusterId));
      Int_t nofDigis = cluster->GetNofDigis();
      
      for (Int_t j = 0; j < nofDigis; ++j)
      {
         Int_t digiId = cluster->GetDigi(j);
         const CbmMuchDigi* digi = static_cast<const CbmMuchDigi*> (fMuchDigis->At(digiId));
         const CbmMatch* match = digi->GetMatch();
         Int_t nofLinks = match->GetNofLinks();

         for (Int_t k = 0; k < nofLinks; ++k)
         {
            const CbmLink& link = match->GetLink(k);
            Int_t eventId = link.GetEntry();
            Int_t mcPointId = link.GetIndex();
            const CbmMuchPoint* muchPoint = static_cast<const CbmMuchPoint*> (fMuchPoints->Get(0, eventId, mcPointId));
            Int_t trackId = muchPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            
            if (trackDesk.much[stationNumber].first.find(mcPointId) != trackDesk.much[stationNumber].first.end())
               trackDesk.much[stationNumber].second.insert(make_pair(gExecNumber, muchTrackIndex));
         }
      }
   }
}

void CbmBinnedTrackerQA::HandleTrd(Int_t trdTrackIndex)
{
   const CbmTrdTrack* trdTrack = static_cast<const CbmTrdTrack*> (fTrdTracks->At(trdTrackIndex));
   Int_t nofTrdHits = trdTrack->GetNofHits();
   
   for (Int_t i = 0; i < nofTrdHits; ++i)
   {
      Int_t trdHitInd = trdTrack->GetHitIndex(i);
      const CbmTrdHit* trdHit = static_cast<const CbmTrdHit*> (fTrdHits->At(trdHitInd));
      Int_t stationNumber = trdHit->GetPlaneId();
      Int_t clusterId = trdHit->GetRefId();
      const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*> (fTrdClusters->At(clusterId));
      Int_t nofDigis = cluster->GetNofDigis();
      
      for (Int_t j = 0; j < nofDigis; ++j)
      {
         Int_t digiId = cluster->GetDigi(j);
         const CbmTrdDigi* digi = static_cast<const CbmTrdDigi*> (fTrdDigis->At(digiId));
         const CbmMatch* match = digi->GetMatch();
         Int_t nofLinks = match->GetNofLinks();

         for (Int_t k = 0; k < nofLinks; ++k)
         {
            const CbmLink& link = match->GetLink(k);
            Int_t eventId = link.GetEntry();
            Int_t mcPointId = link.GetIndex();
            const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, eventId, mcPointId));
            Int_t trackId = trdPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            
            if (trackDesk.trd[stationNumber].first.find(mcPointId) != trackDesk.trd[stationNumber].first.end())
               trackDesk.trd[stationNumber].second.insert(make_pair(gExecNumber, trdTrackIndex));
         }
      }
   }
}

void CbmBinnedTrackerQA::HandleTof(Int_t tofHitIndex)
{
   const CbmMatch* tofHitMatch = static_cast<const CbmMatch*> (fTofHitDigiMatches->At(tofHitIndex));
   Int_t nofTofDigis = tofHitMatch->GetNofLinks();
   
   for (Int_t i = 0; i < nofTofDigis; ++i)
   {
      const CbmLink& digiLink = tofHitMatch->GetLink(i);
      Int_t digiInd = digiLink.GetIndex();
      const CbmTofDigiExp* digi = static_cast<const CbmTofDigiExp*> (fTofDigis->At(digiInd));
      const CbmMatch* pointMatch = digi->GetMatch();
      Int_t nofPoints = pointMatch->GetNofLinks();

      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmLink& pointLink = pointMatch->GetLink(j);
         Int_t eventId = pointLink.GetEntry();
         Int_t pointId = pointLink.GetIndex();
         const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*> (fTofPoints->Get(0, eventId, pointId));
         Int_t trackId = tofPoint->GetTrackID();
         TrackDesc& trackDesk = gTracks[eventId][trackId];
            
         if (trackDesk.tof.first.find(pointId) != trackDesk.tof.first.end())
            trackDesk.tof.second.insert(make_pair(gExecNumber, tofHitIndex));
      }
   }
}

void CbmBinnedTrackerQA::Finish()
{
   
}

ClassImp(CbmBinnedTrackerQA)
