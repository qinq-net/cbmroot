/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <complex>

#include "CbmBinnedTrackerQA.h"
#include "global/CbmGlobalTrack.h"
#include "CbmMCTrack.h"
#include "CbmStsPoint.h"
#include "setup/CbmStsSetup.h"
#include "CbmMuchPoint.h"
#include "geo/CbmMuchGeoScheme.h"
#include "trd/CbmTrdAddress.h"
#include "CbmTofPoint.h"
#include "CbmStsHit.h"
#include "CbmStsDigi.h"
#include "CbmMatch.h"
#include "CbmMuchPixelHit.h"
#include "CbmMuchCluster.h"
#include "CbmMuchDigi.h"
#include "CbmTrdCluster.h"
#include "CbmTrdDigi.h"
#include "CbmTofDigiExp.h"
#include "CbmMCDataManager.h"
#include "TH1.h"
#include "GeoReader.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

using namespace std;

//#define TRD_IDEAL

#define NOF_STS_STATIONS 2
#define NOF_MUCH_STATIONS 1
#define NOF_MUCH_LAYERS 3
#define NOF_TRD_LAYERS 4
#define NOF_TOF 1

struct TrackDesc
{
   // In the pairs below 
   pair<set<Int_t>, set<Int_t> > sts[NOF_STS_STATIONS];
   //pair<set<Int_t>, set<Int_t> > much[NOF_MUCH_STATIONS * NOF_MUCH_LAYERS];
   pair<set<Int_t>, set<Int_t> > trd[NOF_TRD_LAYERS];
   pair<set<Int_t>, set<Int_t> > tof;
   set<const CbmStsPoint*> stsPoints[NOF_STS_STATIONS];
   set<const CbmTrdPoint*> trdPoints[NOF_TRD_LAYERS];
   bool isPrimary;
   const CbmMCTrack* ptr;
   Double_t nearestHitDist[NOF_TRD_LAYERS];
   Double_t nearestHitDistSts[NOF_STS_STATIONS];
   const CbmTrdPoint* nearestPoints[NOF_TRD_LAYERS];
   const CbmStsPoint* nearestPointsSts[NOF_STS_STATIONS];
   Double_t pullX[NOF_TRD_LAYERS];
   Double_t pullY[NOF_TRD_LAYERS];
   Double_t pullXsts[NOF_STS_STATIONS];
   Double_t pullYsts[NOF_STS_STATIONS];
   
   TrackDesc() : tof(), isPrimary(false), ptr(0), nearestHitDist{ -1, -1, -1, -1 }, nearestHitDistSts{ -1, -1 }, nearestPoints{ 0, 0, 0, 0 }, nearestPointsSts{ 0, 0 },
   pullX{ 0, 0, 0, 0 }, pullY{ 0, 0, 0, 0 }, pullXsts{ 0, 0 }, pullYsts{ 0, 0 }
   {
   }
   
   TrackDesc(const TrackDesc&) = default;
   TrackDesc& operator=(const TrackDesc&) = default;
};

static vector<vector<TrackDesc> > gTracks;

//CbmBinnedTracker* gTracker = 0;
//CbmBinnedStation* gTrdStation[NOF_TRD_LAYERS];

static TH1F* stsXResHisto = 0;
static TH1F* stsYResHisto = 0;
static TH1F* stsTResHisto = 0;
static TH1F* trdXResHisto = 0;
static TH1F* trdYResHisto = 0;
static TH1F* trdTResHisto = 0;
static TH1F* muchXResHisto = 0;
static TH1F* muchYResHisto = 0;
static TH1F* muchTResHisto = 0;
static TH1F* tofXResHisto = 0;
static TH1F* tofYResHisto = 0;
static TH1F* tofTResHisto = 0;

static TH1F* stsXPullHistos[] = { 0, 0 };
static TH1F* stsYPullHistos[] = { 0, 0 };

static TH1F* trdXPullHistos[] = { 0, 0, 0, 0 };
static TH1F* trdYPullHistos[] = { 0, 0, 0, 0 };

static TH1F* extrStsXHisto = 0;
static TH1F* extrStsYHisto = 0;

static TH1F* vtxXHisto = 0;
static TH1F* vtxYHisto = 0;
static TH1F* vtxZHisto = 0;

static TH1F* extrTrdXHistos[] = { 0, 0, 0, 0 };
static TH1F* extrTrdYHistos[] = { 0, 0, 0, 0 };

static TH1F* trdNearestHitDistHistos[] = { 0, 0, 0, 0 };
static int trdNofStrangerHits[] = { 0, 0, 0, 0 };

CbmBinnedTrackerQA::CbmBinnedTrackerQA() : fSettings(0), fGlobalTracks(0), fStsTracks(0)/*, fMuchTracks(0)*/, fTrdTracks(0), fStsHits(0), fMuchHits(0), fTrdHits(0), fTofHits(0),
   fStsClusters(0), fMuchClusters(0), fTrdClusters(0), fTrdDigiMatches(0), fTofHitDigiMatches(0), fTofDigiPointMatches(0),
   fStsDigis(0), fMuchDigis(0), fTrdDigis(0), fTofDigis(0), fMCTracks(0), fStsPoints(0), fMuchPoints(0), fTrdPoints(0), fTofPoints(0)
{
}

InitStatus CbmBinnedTrackerQA::Init()
{
   /*CbmBinnedGeoReader* geoReader = CbmBinnedGeoReader::Instance();
   
   if (0 == geoReader)
      fLogger->Fatal(MESSAGE_ORIGIN, "Couldn't instantiate CbmBinnedGeoReader");
   
   geoReader->Read();
   gTracker = CbmBinnedTracker::Instance();
   
   int nofStations = gTracker->fStations.size();
   map<Double_t, CbmBinnedStation*>::iterator stIter = gTracker->fStations.begin();
   ++stIter;
   ++stIter;
   
   for (int i = 0; i < NOF_TRD_LAYERS; ++i)
   {
      gTrdStation[i] = (stIter++)->second;
      gTrdStation[i]->SetMinT(-100);
   }*/
   
   stsXResHisto = new TH1F("stsXResHisto", "stsXResHisto", 200, -0.1, 0.1);
   stsYResHisto = new TH1F("stsYResHisto", "stsYResHisto", 200, -0.1, 0.1);
   stsTResHisto = new TH1F("stsTResHisto", "stsTResHisto", 200, -10.0, 10.0);
   trdXResHisto = new TH1F("trdXResHisto", "trdXResHisto", 600, -60.0, 60.0);
   trdYResHisto = new TH1F("trdYResHisto", "trdYResHisto", 600, -60.0, 60.0);
   trdTResHisto = new TH1F("trdTResHisto", "trdTResHisto", 200, -10.0, 10.0);
   muchXResHisto = new TH1F("muchXResHisto", "muchXResHisto", 200, -3.0, 3.0);
   muchYResHisto = new TH1F("muchYResHisto", "muchYResHisto", 200, -3.0, 3.0);
   muchTResHisto = new TH1F("muchTResHisto", "muchTResHisto", 200, -10.0, 100.0);
   tofXResHisto = new TH1F("tofXResHisto", "tofXResHisto", 200, -3.0, 3.0);
   tofYResHisto = new TH1F("tofYResHisto", "tofYResHisto", 200, -3.0, 3.0);
   tofTResHisto = new TH1F("tofTResHisto", "tofTResHisto", 200, -10.0, 10.0);
   
   extrStsXHisto = new TH1F("extrStsXHisto", "extrStsXHisto", 200, -0.5, 0.5);
   extrStsYHisto = new TH1F("extrStsYHisto", "extrStsYHisto", 200, -0.5, 0.5);
   
   vtxXHisto = new TH1F("vtxXHisto", "vtxXHisto", 100, -0.5, 0.5);
   vtxYHisto = new TH1F("vtxYHisto", "vtxYHisto", 100, -0.5, 0.5);
   vtxZHisto = new TH1F("vtxZHisto", "vtxZHisto", 100, -0.5, 0.5);
   
   for (int i = 0; i < 2; ++i)
   {
      char name[256];
      sprintf(name, "stsXPullHistos_%d", i);
      Double_t range = 5;
      stsXPullHistos[i] = new TH1F(name, name, 200, -range, range);
      sprintf(name, "stsYPullHistos_%d", i);
      stsYPullHistos[i] = new TH1F(name, name, 200, -range, range);
   }
   
   for (int i = 0; i < 4; ++i)
   {
      char name[256];
      sprintf(name, "extrTrdXHisto_%d", i);
      extrTrdXHistos[i] = new TH1F(name, name, 200, 0.5, 0.5);
      sprintf(name, "extrTrdYHisto_%d", i);
      extrTrdYHistos[i] = new TH1F(name, name, 200, 0.5, 0.5);
      
      sprintf(name, "trdNearestHitDistHistos_%d", i);
      trdNearestHitDistHistos[i] = new TH1F(name, name, 200, -100, 100);
      
      sprintf(name, "trdXPullHistos_%d", i);
      Double_t range = i % 2 ? 5 : 25;
      trdXPullHistos[i] = new TH1F(name, name, 200, -range, range);
      sprintf(name, "trdYPullHistos_%d", i);
      range = i % 2 ? 25 : 5;
      trdYPullHistos[i] = new TH1F(name, name, 200, -range, range);
   }
   
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fGlobalTracks = static_cast<TClonesArray*> (ioman->GetObject("GlobalTrack"));
   
   if (0 == fGlobalTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No global tracks in the input file");
   
   fStsTracks = static_cast<TClonesArray*> (ioman->GetObject("StsTrack"));
   
   if (0 == fStsTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts tracks in the input file");
   
   //fMuchTracks = static_cast<TClonesArray*> (ioman->GetObject("MuchTrack"));
   
   //if (0 == fMuchTracks)
      //fLogger->Fatal(MESSAGE_ORIGIN, "No much tracks in the input file");
   
   fTrdTracks = static_cast<TClonesArray*> (ioman->GetObject("TrdTrack"));
   
   if (0 == fTrdTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No trd tracks in the input file");
   
   fStsHits = static_cast<TClonesArray*> (ioman->GetObject("StsHit"));
   
   if (0 == fStsHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts hits in the input file");
   
   /*fMuchHits = static_cast<TClonesArray*> (ioman->GetObject("MuchPixelHit"));
   
   if (0 == fMuchHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much hits in the input file");*/
   
   fTrdHits = static_cast<TClonesArray*> (ioman->GetObject("TrdHit"));
   
   if (0 == fTrdHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No trd hits in the input file");
   
   fTofHits = static_cast<TClonesArray*> (ioman->GetObject("TofHit"));
   
   if (0 == fTofHits)
      fLogger->Fatal(MESSAGE_ORIGIN, "No tof hits in the input file");
   
   fStsClusters = static_cast<TClonesArray*> (ioman->GetObject("StsCluster"));
   
   if (0 == fStsClusters)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts clusters in the input file");
   
   /*fMuchClusters = static_cast<TClonesArray*> (ioman->GetObject("MuchCluster"));
   
   if (0 == fMuchClusters)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much clusters in the input file");*/
   
#ifndef TRD_IDEAL   
   fTrdClusters = static_cast<TClonesArray*> (ioman->GetObject("TrdCluster"));
   
   if (0 == fTrdClusters)
      fLogger->Fatal(MESSAGE_ORIGIN, "No global tracks in the input file");
   
   fTrdDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("TrdDigiMatch"));
   
   if (0 == fTrdDigiMatches)
      fLogger->Fatal(MESSAGE_ORIGIN, "No trd hit to digi matches in the input file");
#endif//TRD_IDEAL
   
   fTofHitDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("TofDigiMatch"));
   
   if (0 == fTofHitDigiMatches)
      fLogger->Fatal(MESSAGE_ORIGIN, "No tof hit to digi matches in the input file");
   
   fTofDigiPointMatches = static_cast<TClonesArray*> (ioman->GetObject("TofDigiMatchPoints"));
   
   if (0 == fTofDigiPointMatches)
      fLogger->Fatal(MESSAGE_ORIGIN, "No tof digi to point matches in the input file");
   
   fStsDigis = static_cast<TClonesArray*> (ioman->GetObject("StsDigi"));
   
   if (0 == fStsDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No sts digis in the input file");
   
   /*fMuchDigis = static_cast<TClonesArray*> (ioman->GetObject("MuchDigi"));
   
   if (0 == fMuchDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much digis in the input file");*/
   
#ifndef TRD_IDEAL
   fTrdDigis = static_cast<TClonesArray*> (ioman->GetObject("TrdDigi"));
   
   if (0 == fTrdDigis)
      fLogger->Fatal(MESSAGE_ORIGIN, "No trd digis in the input file");
#endif//TRD_IDEAL
   
   fTofDigis = static_cast<TClonesArray*> (ioman->GetObject("TofDigi"));
   
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
   
   /*fMuchPoints = mcManager->InitBranch("MuchPoint");
   
   if (0 == fMuchPoints)
      fLogger->Fatal(MESSAGE_ORIGIN, "No much MC points in the input file");*/
   
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
      vector<TrackDesc>& eventTracks = gTracks.back();
      eventTracks.resize(nofMcTracks);
      
      for (Int_t j = 0; j < nofMcTracks; ++j)
      {
         TrackDesc& track = eventTracks[j];
         const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*> (fMCTracks->Get(0, i, j));
         track.ptr = mcTrack;
         
         if (mcTrack->GetMotherId() < 0)
         {
            track.isPrimary = true;
            vtxXHisto->Fill(mcTrack->GetStartX());
            vtxYHisto->Fill(mcTrack->GetStartY());
            vtxZHisto->Fill(mcTrack->GetStartZ());
         }
      }
   }
   
   for (Int_t i = 0; fStsPoints->Size(0, i) >= 0; ++i)
   {
      Int_t nofPoints = fStsPoints->Size(0, i);
      vector<TrackDesc>& tracks = gTracks[i];
      
      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmStsPoint* stsPoint = static_cast<const CbmStsPoint*> (fStsPoints->Get(0, i, j));
         Int_t trackId = stsPoint->GetTrackID();
         //Int_t stationNumber = CbmStsSetup::Instance()->GetStationNumber(stsPoint->GetDetectorID());
         Int_t stationNumber = CbmStsAddress::GetElementId(stsPoint->GetDetectorID(), kSts);
         //tracks[trackId].sts[stationNumber].first.insert(j);
         TrackDesc& trackDesk = tracks[trackId];
         trackDesk.stsPoints[stationNumber].insert(stsPoint);
      }
   }
   
   /*for (Int_t i = 0; fMuchPoints->Size(0, i) >= 0; ++i)
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
         //tracks[trackId].much[stationNumber].first.insert(j);
      }
   }*/
   
   for (Int_t i = 0; fTrdPoints->Size(0, i) >= 0; ++i)
   {
      Int_t nofPoints = fTrdPoints->Size(0, i);
      vector<TrackDesc>& tracks = gTracks[i];
      
      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, i, j));
         Int_t trackId = trdPoint->GetTrackID();
         int stationNumber = CbmTrdAddress::GetLayerId(trdPoint->GetModuleAddress());
         //tracks[trackId].trd[stationNumber].first.insert(j);
         TrackDesc& trackDesk = tracks[trackId];
         trackDesk.trdPoints[stationNumber].insert(trdPoint);
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
      }
   }
   
   return kSUCCESS;
}

void CbmBinnedTrackerQA::IterateTrdHits(std::function<void(const CbmTrdHit*, const CbmTrdPoint*)> handleData)
{
   Int_t nofTrdHits = fTrdHits->GetEntriesFast();
   
   for (Int_t i = 0; i < nofTrdHits; ++i)
   {
      const CbmTrdHit* trdHit = static_cast<const CbmTrdHit*> (fTrdHits->At(i));
      Int_t stationNumber = trdHit->GetPlaneId();      
      Int_t clusterId = trdHit->GetRefId();
      const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*> (fTrdClusters->At(clusterId));
      Int_t nofDigis = cluster->GetNofDigis();
      
      for (Int_t j = 0; j < nofDigis; ++j)
      {
         Int_t digiId = cluster->GetDigi(j);
         const CbmMatch* match = static_cast<const CbmMatch*> (fTrdDigiMatches->At(digiId));
         Int_t nofLinks = match->GetNofLinks();

         for (Int_t k = 0; k < nofLinks; ++k)
         {
            const CbmLink& link = match->GetLink(k);
            Int_t eventId = link.GetEntry();
            Int_t mcPointId = link.GetIndex();
            const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, eventId, mcPointId));
            handleData(trdHit, trdPoint);
         }
      }
   }// TRD hits
}

static Int_t gEventNumber = 0;

void CbmBinnedTrackerQA::Exec(Option_t* opt)
{
   //gTracker->Clear();        
   //CbmBinnedHitReader::Instance()->Read();
        
   Int_t nofStsHits = fStsHits->GetEntriesFast();
   
   for (Int_t i = 0; i < nofStsHits; ++i)
   {
      const CbmStsHit* stsHit = static_cast<const CbmStsHit*> (fStsHits->At(i));
      //Int_t stationNumber = CbmStsSetup::Instance()->GetStationNumber(stsHit->GetAddress());
      Int_t stationNumber = CbmStsAddress::GetElementId(stsHit->GetAddress(), kSts);
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
            stsXResHisto->Fill(stsHit->GetX() - (stsPoint->GetXIn() + stsPoint->GetXOut()) / 2);
            stsYResHisto->Fill(stsHit->GetY() - (stsPoint->GetYIn() + stsPoint->GetYOut()) / 2);
            stsTResHisto->Fill(stsHit->GetTime() - stsPoint->GetTime());
            Int_t trackId = stsPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            trackDesk.sts[stationNumber].first.insert(i);
            Double_t mcX = (stsPoint->GetXIn() + stsPoint->GetXOut()) / 2;
            Double_t mcY = (stsPoint->GetYIn() + stsPoint->GetYOut()) / 2;
            
            if (trackDesk.isPrimary && !trackDesk.stsPoints[0].empty() && !trackDesk.stsPoints[1].empty())
            {
               Double_t dist = std::sqrt((stsHit->GetX() - mcX) * (stsHit->GetX() - mcX) + (stsHit->GetY() - mcY) * (stsHit->GetY() - mcY));
               
               if (trackDesk.nearestHitDistSts[stationNumber] < 0 || dist < trackDesk.nearestHitDistSts[stationNumber])
               {
                  trackDesk.nearestHitDistSts[stationNumber] = dist;
                  trackDesk.nearestPointsSts[stationNumber] = stsPoint;
                  trackDesk.pullXsts[stationNumber] = (stsHit->GetX() - mcX) / stsHit->GetDx();
                  trackDesk.pullYsts[stationNumber] = (stsHit->GetY() - mcY) / stsHit->GetDy();
               }
            }
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
            stsXResHisto->Fill(stsHit->GetX() - (stsPoint->GetXIn() + stsPoint->GetXOut()) / 2);
            stsYResHisto->Fill(stsHit->GetY() - (stsPoint->GetYIn() + stsPoint->GetYOut()) / 2);
            stsTResHisto->Fill(stsHit->GetTime() - stsPoint->GetTime());
            Int_t trackId = stsPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            trackDesk.sts[stationNumber].first.insert(i);
            Double_t mcX = (stsPoint->GetXIn() + stsPoint->GetXOut()) / 2;
            Double_t mcY = (stsPoint->GetYIn() + stsPoint->GetYOut()) / 2;
            
            if (trackDesk.isPrimary && !trackDesk.stsPoints[0].empty() && !trackDesk.stsPoints[1].empty())
            {
               Double_t dist = std::sqrt((stsHit->GetX() - mcX) * (stsHit->GetX() - mcX) + (stsHit->GetY() - mcY) * (stsHit->GetY() - mcY));
               
               if (trackDesk.nearestHitDistSts[stationNumber] < 0 || dist < trackDesk.nearestHitDistSts[stationNumber])
               {
                  trackDesk.nearestHitDistSts[stationNumber] = dist;
                  trackDesk.nearestPointsSts[stationNumber] = stsPoint;
                  trackDesk.pullXsts[stationNumber] = (stsHit->GetX() - mcX) / stsHit->GetDx();
                  trackDesk.pullYsts[stationNumber] = (stsHit->GetY() - mcY) / stsHit->GetDy();
               }
            }
         }
      }
   }// STS hits
   
   /*Int_t nofMuchHits = fMuchHits->GetEntriesFast();
   
   for (Int_t i = 0; i < nofMuchHits; ++i)
   {
      const CbmMuchPixelHit* muchHit = static_cast<const CbmMuchPixelHit*> (fMuchHits->At(i));
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
            muchXResHisto->Fill(muchHit->GetX() - (muchPoint->GetXIn() + muchPoint->GetXOut()) / 2);
            muchYResHisto->Fill(muchHit->GetY() - (muchPoint->GetYIn() + muchPoint->GetYOut()) / 2);
            muchTResHisto->Fill(muchHit->GetTime() - muchPoint->GetTime());
            Int_t trackId = muchPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            trackDesk.much[stationNumber].first.insert(i);
         }
      }
   }// MUCH hits*/
   
   Int_t nofTrdHits = fTrdHits->GetEntriesFast();
   
   for (Int_t i = 0; i < nofTrdHits; ++i)
   {
      const CbmTrdHit* trdHit = static_cast<const CbmTrdHit*> (fTrdHits->At(i));
      Int_t stationNumber = trdHit->GetPlaneId();      
      Int_t clusterId = trdHit->GetRefId();
#ifndef TRD_IDEAL
      const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*> (fTrdClusters->At(clusterId));
      Int_t nofDigis = cluster->GetNofDigis();
      
      for (Int_t j = 0; j < nofDigis; ++j)
      {
         Int_t digiId = cluster->GetDigi(j);
         const CbmMatch* match = static_cast<const CbmMatch*> (fTrdDigiMatches->At(digiId));
         Int_t nofLinks = match->GetNofLinks();

         for (Int_t k = 0; k < nofLinks; ++k)
         {
            const CbmLink& link = match->GetLink(k);
            Int_t eventId = link.GetEntry();
            Int_t mcPointId = link.GetIndex();
            const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, eventId, mcPointId));
            Double_t mcX = (trdPoint->GetXIn() + trdPoint->GetXOut()) / 2;
            Double_t mcY = (trdPoint->GetYIn() + trdPoint->GetYOut()) / 2;
            trdXResHisto->Fill(trdHit->GetX() - mcX);
            trdYResHisto->Fill(trdHit->GetY() - mcY);
            trdTResHisto->Fill(trdHit->GetTime() - trdPoint->GetTime());
            Int_t trackId = trdPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            trackDesk.trd[stationNumber].first.insert(i);
            
            if (trackDesk.isPrimary && !trackDesk.trdPoints[0].empty() && !trackDesk.trdPoints[1].empty() && !trackDesk.trdPoints[2].empty() && !trackDesk.trdPoints[3].empty())
            {
               Double_t dist = std::sqrt((trdHit->GetX() - mcX) * (trdHit->GetX() - mcX) + (trdHit->GetY() - mcY) * (trdHit->GetY() - mcY));
               
               if (trackDesk.nearestHitDist[stationNumber] < 0 || dist < trackDesk.nearestHitDist[stationNumber])
               {
                  trackDesk.nearestHitDist[stationNumber] = dist;
                  trackDesk.nearestPoints[stationNumber] = trdPoint;
                  trackDesk.pullX[stationNumber] = (trdHit->GetX() - mcX) / trdHit->GetDx();
                  trackDesk.pullY[stationNumber] = (trdHit->GetY() - mcY) / trdHit->GetDy();
               }
            }
         }
      }
#else//TRD_IDEAL
      const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, gEventNumber, clusterId));
      Double_t mcX = (trdPoint->GetXIn() + trdPoint->GetXOut()) / 2;
      Double_t mcY = (trdPoint->GetYIn() + trdPoint->GetYOut()) / 2;
      trdXResHisto->Fill(trdHit->GetX() - mcX);
      trdYResHisto->Fill(trdHit->GetY() - mcY);
      trdTResHisto->Fill(trdHit->GetTime() - trdPoint->GetTime());
      Int_t trackId = trdPoint->GetTrackID();
      TrackDesc& trackDesk = gTracks[gEventNumber][trackId];
      trackDesk.trd[stationNumber].first.insert(i);

      if (trackDesk.isPrimary && !trackDesk.trdPoints[0].empty() && !trackDesk.trdPoints[1].empty() && !trackDesk.trdPoints[2].empty() && !trackDesk.trdPoints[3].empty())
      {
         Double_t dist = std::sqrt((trdHit->GetX() - mcX) * (trdHit->GetX() - mcX) + (trdHit->GetY() - mcY) * (trdHit->GetY() - mcY));

         if (trackDesk.nearestHitDist[stationNumber] < 0 || dist < trackDesk.nearestHitDist[stationNumber])
         {
            trackDesk.nearestHitDist[stationNumber] = dist;
            trackDesk.nearestPoints[stationNumber] = trdPoint;
            trackDesk.pullX[stationNumber] = (trdHit->GetX() - mcX) / trdHit->GetDx();
            trackDesk.pullY[stationNumber] = (trdHit->GetY() - mcY) / trdHit->GetDy();
         }
      }
#endif//TRD_IDEAL
   }// TRD hits
   
   /*IterateTrdHits(
      [](const CbmTrdHit* trdHit, const CbmTrdPoint* trdPoint)->void
      {
         Double_t mcX = (trdPoint->GetXIn() + trdPoint->GetXOut()) / 2;
         Double_t mcY = (trdPoint->GetYIn() + trdPoint->GetYOut()) / 2;
      }
   );*/
   /*vector<TrackDesc>& evMCTracks = gTracks[gEventNumber];
   int trackIndex = 0;
   
   for (vector<TrackDesc>::const_iterator i = evMCTracks.begin(); i != evMCTracks.end(); ++i)
   {
      const TrackDesc& trackDesc = *i;
      int nofSt = -1;
      
      for (int j = 0; j < NOF_TRD_LAYERS; ++j)
      {         
         if (trackDesc.nearestHitDist[j] >= 0)
         {
            Double_t dist = trackDesc.nearestHitDist[j];
            const CbmTrdPoint* trdPoint = trackDesc.nearestPoints[j];
            Double_t mcX = (trdPoint->GetXIn() + trdPoint->GetXOut()) / 2;
            Double_t mcY = (trdPoint->GetYIn() + trdPoint->GetYOut()) / 2;
            bool sameTrack = false;
            int nofHits = 0;
            gTrdStation[j]->SearchHits(0, 0, mcY - dist, mcY + dist, mcX - dist, mcX + dist, -90, 290,
               [&](CbmTBin::HitHolder& hitHolder)->void
               {
                  const CbmPixelHit* hit = hitHolder.hit;
                  Double_t x = hit->GetX();
                  Double_t y = hit->GetY();
                  Double_t dist2 = std::sqrt((x - mcX) * (x - mcX) + (y - mcY) * (y - mcY));
                  
                  if (dist2 < dist)
                  {
                     ++nofHits;
                     Int_t clusterId = hit->GetRefId();
#ifndef TRD_IDEAL
                     const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*> (fTrdClusters->At(clusterId));
                     Int_t nofDigis = cluster->GetNofDigis();

                     for (Int_t k = 0; k < nofDigis; ++k)
                     {
                        Int_t digiId = cluster->GetDigi(k);
                        const CbmMatch* match = static_cast<const CbmMatch*> (fTrdDigiMatches->At(digiId));
                        Int_t nofLinks = match->GetNofLinks();

                        for (Int_t l = 0; l < nofLinks; ++l)
                        {
                           const CbmLink& link = match->GetLink(l);
                           Int_t eventId = link.GetEntry();
                           Int_t mcPointId = link.GetIndex();
                           const CbmTrdPoint* trdPoint2 = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, eventId, mcPointId));
                           Int_t trackId = trdPoint2->GetTrackID();
                           
                           if (trackId == trackIndex)
                           {
                              sameTrack = true;
                              break;
                           }
                        }
                        
                        if (sameTrack)
                           break;
                     }
#else//TRD_IDEAL
                     const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, gEventNumber, clusterId));
                     Int_t trackId = trdPoint->GetTrackID();

                     if (trackId == trackIndex)
                        sameTrack = true;
#endif//TRD_IDEAL
                  }
               }
            );
            
            if (!sameTrack && nofHits > 0)
               ++nofSt;
         }
      }
      
      if (nofSt >= 0)
         ++trdNofStrangerHits[nofSt];
      
      ++trackIndex;
   }*/
   
   Int_t nofTofHits = fTofHitDigiMatches->GetEntriesFast();
   
   for (Int_t i = 0; i < nofTofHits; ++i)
   {
      const CbmTofHit* tofHit = static_cast<const CbmTofHit*> (fTofHits->At(i));
      const CbmMatch* tofHitMatch = static_cast<const CbmMatch*> (fTofHitDigiMatches->At(i));
      Int_t nofTofDigis = tofHitMatch->GetNofLinks();
   
      for (Int_t j = 0; j < nofTofDigis; ++j)
      {
         const CbmLink& digiLink = tofHitMatch->GetLink(j);
         Int_t digiInd = digiLink.GetIndex();
         const CbmMatch* pointMatch = static_cast<const CbmMatch*> (fTofDigiPointMatches->At(digiInd));
         Int_t nofPoints = pointMatch->GetNofLinks();

         for (Int_t k = 0; k < nofPoints; ++k)
         {
            const CbmLink& pointLink = pointMatch->GetLink(k);
            Int_t eventId = pointLink.GetEntry();
            Int_t pointId = pointLink.GetIndex();
            const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*> (fTofPoints->Get(0, eventId, pointId));
            tofXResHisto->Fill(tofHit->GetX() - tofPoint->GetX());
            tofYResHisto->Fill(tofHit->GetY() - tofPoint->GetY());
            tofTResHisto->Fill(tofHit->GetTime() - tofPoint->GetTime());
            Int_t trackId = tofPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            trackDesk.tof.first.insert(i);
         }
      }
   }// TOF hits
   
   Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
   
   for (Int_t i = 0; i < nofGlobalTracks; ++i)
   {
      const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*> (fGlobalTracks->At(i));
      Int_t stsIndex = globalTrack->GetStsTrackIndex();
      Int_t muchIndex = globalTrack->GetMuchTrackIndex();
      Int_t trdIndex = globalTrack->GetTrdTrackIndex();
      Int_t tofIndex = globalTrack->GetTofHitIndex();
      
      if (stsIndex < 0)
         continue;
      
      //if (muchIndex < 0)
         //continue;
      
      if (trdIndex < 0)
         continue;
      
      if (tofIndex < 0)
         continue;
      
      HandleSts(stsIndex);
      //HandleMuch(muchIndex);
      HandleTrd(trdIndex);
      HandleTof(tofIndex);
   }
   
   ++gEventNumber;
}

void CbmBinnedTrackerQA::HandleSts(Int_t stsTrackIndex)
{
   const CbmStsTrack* stsTrack = static_cast<const CbmStsTrack*> (fStsTracks->At(stsTrackIndex));
   Int_t nofStsHits = stsTrack->GetNofHits();
   
   for (Int_t i = 0; i < nofStsHits; ++i)
   {
      Int_t stsHitInd = stsTrack->GetStsHitIndex(i);
      const CbmStsHit* stsHit = static_cast<const CbmStsHit*> (fStsHits->At(stsHitInd));
      //Int_t stationNumber = CbmStsSetup::Instance()->GetStationNumber(stsHit->GetAddress());
      Int_t stationNumber = CbmStsAddress::GetElementId(stsHit->GetAddress(), kSts);
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
            
            if (trackDesk.sts[stationNumber].first.find(stsHitInd) != trackDesk.sts[stationNumber].first.end())
               trackDesk.sts[stationNumber].second.insert(stsTrackIndex);
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
            
            if (trackDesk.sts[stationNumber].first.find(stsHitInd) != trackDesk.sts[stationNumber].first.end())
               trackDesk.sts[stationNumber].second.insert(stsTrackIndex);
         }
      }
   }
}

/*void CbmBinnedTrackerQA::HandleMuch(Int_t muchTrackIndex)
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
            
            //if (trackDesk.much[stationNumber].first.find(muchHitInd) != trackDesk.much[stationNumber].first.end())
               //trackDesk.much[stationNumber].second.insert(muchTrackIndex);
         }
      }
   }
}*/

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
#ifndef TRD_IDEAL
      const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*> (fTrdClusters->At(clusterId));
      Int_t nofDigis = cluster->GetNofDigis();
      
      for (Int_t j = 0; j < nofDigis; ++j)
      {
         Int_t digiId = cluster->GetDigi(j);
         const CbmMatch* match = static_cast<const CbmMatch*> (fTrdDigiMatches->At(digiId));
         Int_t nofLinks = match->GetNofLinks();

         for (Int_t k = 0; k < nofLinks; ++k)
         {
            const CbmLink& link = match->GetLink(k);
            Int_t eventId = link.GetEntry();
            Int_t mcPointId = link.GetIndex();
            const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, eventId, mcPointId));
            Int_t trackId = trdPoint->GetTrackID();
            TrackDesc& trackDesk = gTracks[eventId][trackId];
            
            if (trackDesk.trd[stationNumber].first.find(trdHitInd) != trackDesk.trd[stationNumber].first.end())
               trackDesk.trd[stationNumber].second.insert(trdTrackIndex);
         }
      }
#else//TRD_IDEAL
      const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, gEventNumber, clusterId));
      Int_t trackId = trdPoint->GetTrackID();
      TrackDesc& trackDesk = gTracks[gEventNumber][trackId];

      if (trackDesk.trd[stationNumber].first.find(trdHitInd) != trackDesk.trd[stationNumber].first.end())
         trackDesk.trd[stationNumber].second.insert(trdTrackIndex);
#endif//TRD_IDEAL
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
      const CbmMatch* pointMatch = static_cast<const CbmMatch*> (fTofDigiPointMatches->At(digiInd));
      Int_t nofPoints = pointMatch->GetNofLinks();

      for (Int_t j = 0; j < nofPoints; ++j)
      {
         const CbmLink& pointLink = pointMatch->GetLink(j);
         Int_t eventId = pointLink.GetEntry();
         Int_t pointId = pointLink.GetIndex();
         const CbmTofPoint* tofPoint = static_cast<const CbmTofPoint*> (fTofPoints->Get(0, eventId, pointId));
         Int_t trackId = tofPoint->GetTrackID();
         TrackDesc& trackDesk = gTracks[eventId][trackId];
            
         if (trackDesk.tof.first.find(tofHitIndex) != trackDesk.tof.first.end())
            trackDesk.tof.second.insert(tofHitIndex);
      }
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

void CbmBinnedTrackerQA::Finish()
{
   int nofAllTracks = 0;
   int nofRefTracks = 0;
   int nofMatchedRefTracks = 0;
   int nofSts[2] = { 0, 0 };
   int nofTrd[4] = { 0, 0, 0, 0 };
   //int nofMuch[3] = { 0, 0, 0 };
   //int nofTof = 0;
   
   for (vector<vector<TrackDesc> >::const_iterator i = gTracks.begin(); i != gTracks.end(); ++i)
   {
      const vector<TrackDesc>& evTracks = *i;
      
      for (vector<TrackDesc>::const_iterator j = evTracks.begin(); j != evTracks.end(); ++j)
      {
         ++nofAllTracks;
         const TrackDesc& trackDesc = *j;
         
         if (!trackDesc.isPrimary)
            continue;
         
         bool isRef = true;

         for (int k = 0; k < NOF_STS_STATIONS; ++k)
         {
            if (trackDesc.sts[k].first.empty())
            {
               isRef = false;
               break;
            }
         }
         
         if (!isRef)
            continue;
         
         for(set<const CbmStsPoint*>::const_iterator k = trackDesc.stsPoints[0].begin(); k != trackDesc.stsPoints[0].end(); ++k)
         {
            const CbmStsPoint* p0 = *k;
            Double_t x0 = (p0->GetXIn() + p0->GetXOut()) / 2;
            Double_t y0 = (p0->GetYIn() + p0->GetYOut()) / 2;
            Double_t z0 = (p0->GetZIn() + p0->GetZOut()) / 2;
            Double_t tx =  (x0 - trackDesc.ptr->GetStartX()) / (z0 - trackDesc.ptr->GetStartZ());
            Double_t ty =  (y0 - trackDesc.ptr->GetStartY()) / (z0 - trackDesc.ptr->GetStartZ());
            
            for(set<const CbmStsPoint*>::const_iterator l = trackDesc.stsPoints[1].begin(); l != trackDesc.stsPoints[1].end(); ++l)
            {
               const CbmStsPoint* p1 = *l;
               Double_t x1 = (p1->GetXIn() + p1->GetXOut()) / 2;
               Double_t y1 = (p1->GetYIn() + p1->GetYOut()) / 2;
               Double_t z1 = (p1->GetZIn() + p1->GetZOut()) / 2;
               extrStsXHisto->Fill(x1 - tx * z1);
               extrStsYHisto->Fill(y1 - ty * z1);
            }
         }
         
         for (int k = 0; k < NOF_TRD_LAYERS; ++k)
         {
            if (trackDesc.trd[k].first.empty())
            {
               isRef = false;
               break;
            }
         }
         
         if (!isRef)
            continue;
         
         for (set<const CbmStsPoint*>::const_iterator k = trackDesc.stsPoints[0].begin(); k != trackDesc.stsPoints[0].end(); ++k)
         {
            const CbmStsPoint* p1 = *k;
            Double_t x1 = (p1->GetXIn() + p1->GetXOut()) / 2;
            Double_t y1 = (p1->GetYIn() + p1->GetYOut()) / 2;
            Double_t z1 = (p1->GetZIn() + p1->GetZOut()) / 2;
            
            for (set<const CbmStsPoint*>::const_iterator l = trackDesc.stsPoints[1].begin(); l != trackDesc.stsPoints[1].end(); ++l)
            {
               const CbmStsPoint* p2 = *l;
               Double_t x2 = (p2->GetXIn() + p2->GetXOut()) / 2;
               Double_t y2 = (p2->GetYIn() + p2->GetYOut()) / 2;
               Double_t z2 = (p2->GetZIn() + p2->GetZOut()) / 2;
               Double_t tx = (x2 - x1) / (z2 - z1);
               Double_t ty = (y2 - y1) / (z2 - z1);
               
               for (set<const CbmTrdPoint*>::const_iterator m = trackDesc.trdPoints[0].begin(); m != trackDesc.trdPoints[0].end(); ++m)
               {
                  const CbmTrdPoint* p = *m;
                  Double_t x = (p->GetXIn() + p->GetXOut()) / 2;
                  Double_t y = (p->GetYIn() + p->GetYOut()) / 2;
                  Double_t z = (p->GetZIn() + p->GetZOut()) / 2;
                  Double_t deltaZ = z - z2;
                  extrTrdXHistos[0]->Fill(x - x2 - tx * deltaZ);
                  extrTrdYHistos[0]->Fill(y - y2 - ty * deltaZ);
               }
            }
         }
         
         for (set<const CbmStsPoint*>::const_iterator k = trackDesc.stsPoints[1].begin(); k != trackDesc.stsPoints[1].end(); ++k)
         {
            const CbmStsPoint* p1 = *k;
            Double_t x1 = (p1->GetXIn() + p1->GetXOut()) / 2;
            Double_t y1 = (p1->GetYIn() + p1->GetYOut()) / 2;
            Double_t z1 = (p1->GetZIn() + p1->GetZOut()) / 2;
            
            for (set<const CbmTrdPoint*>::const_iterator l = trackDesc.trdPoints[0].begin(); l != trackDesc.trdPoints[0].end(); ++l)
            {
               const CbmTrdPoint* p2 = *l;
               Double_t x2 = (p2->GetXIn() + p2->GetXOut()) / 2;
               Double_t y2 = (p2->GetYIn() + p2->GetYOut()) / 2;
               Double_t z2 = (p2->GetZIn() + p2->GetZOut()) / 2;
               Double_t tx = (x2 - x1) / (z2 - z1);
               Double_t ty = (y2 - y1) / (z2 - z1);
               
               for (set<const CbmTrdPoint*>::const_iterator m = trackDesc.trdPoints[1].begin(); m != trackDesc.trdPoints[1].end(); ++m)
               {
                  const CbmTrdPoint* p = *m;
                  Double_t x = (p->GetXIn() + p->GetXOut()) / 2;
                  Double_t y = (p->GetYIn() + p->GetYOut()) / 2;
                  Double_t z = (p->GetZIn() + p->GetZOut()) / 2;
                  Double_t deltaZ = z - z2;
                  extrTrdXHistos[1]->Fill(x - x2 - tx * deltaZ);
                  extrTrdYHistos[1]->Fill(y - y2 - ty * deltaZ);
               }
            }
         }
         
         for (set<const CbmTrdPoint*>::const_iterator k = trackDesc.trdPoints[0].begin(); k != trackDesc.trdPoints[0].end(); ++k)
         {
            const CbmTrdPoint* p1 = *k;
            Double_t x1 = (p1->GetXIn() + p1->GetXOut()) / 2;
            Double_t y1 = (p1->GetYIn() + p1->GetYOut()) / 2;
            Double_t z1 = (p1->GetZIn() + p1->GetZOut()) / 2;
            
            for (set<const CbmTrdPoint*>::const_iterator l = trackDesc.trdPoints[1].begin(); l != trackDesc.trdPoints[1].end(); ++l)
            {
               const CbmTrdPoint* p2 = *l;
               Double_t x2 = (p2->GetXIn() + p2->GetXOut()) / 2;
               Double_t y2 = (p2->GetYIn() + p2->GetYOut()) / 2;
               Double_t z2 = (p2->GetZIn() + p2->GetZOut()) / 2;
               Double_t tx = (x2 - x1) / (z2 - z1);
               Double_t ty = (y2 - y1) / (z2 - z1);
               
               for (set<const CbmTrdPoint*>::const_iterator m = trackDesc.trdPoints[2].begin(); m != trackDesc.trdPoints[2].end(); ++m)
               {
                  const CbmTrdPoint* p = *m;
                  Double_t x = (p->GetXIn() + p->GetXOut()) / 2;
                  Double_t y = (p->GetYIn() + p->GetYOut()) / 2;
                  Double_t z = (p->GetZIn() + p->GetZOut()) / 2;
                  Double_t deltaZ = z - z2;
                  extrTrdXHistos[2]->Fill(x - x2 - tx * deltaZ);
                  extrTrdYHistos[2]->Fill(y - y2 - ty * deltaZ);
               }
            }
         }
         
         for (set<const CbmTrdPoint*>::const_iterator k = trackDesc.trdPoints[1].begin(); k != trackDesc.trdPoints[1].end(); ++k)
         {
            const CbmTrdPoint* p1 = *k;
            Double_t x1 = (p1->GetXIn() + p1->GetXOut()) / 2;
            Double_t y1 = (p1->GetYIn() + p1->GetYOut()) / 2;
            Double_t z1 = (p1->GetZIn() + p1->GetZOut()) / 2;
            
            for (set<const CbmTrdPoint*>::const_iterator l = trackDesc.trdPoints[2].begin(); l != trackDesc.trdPoints[2].end(); ++l)
            {
               const CbmTrdPoint* p2 = *l;
               Double_t x2 = (p2->GetXIn() + p2->GetXOut()) / 2;
               Double_t y2 = (p2->GetYIn() + p2->GetYOut()) / 2;
               Double_t z2 = (p2->GetZIn() + p2->GetZOut()) / 2;
               Double_t tx = (x2 - x1) / (z2 - z1);
               Double_t ty = (y2 - y1) / (z2 - z1);
               
               for (set<const CbmTrdPoint*>::const_iterator m = trackDesc.trdPoints[3].begin(); m != trackDesc.trdPoints[3].end(); ++m)
               {
                  const CbmTrdPoint* p = *m;
                  Double_t x = (p->GetXIn() + p->GetXOut()) / 2;
                  Double_t y = (p->GetYIn() + p->GetYOut()) / 2;
                  Double_t z = (p->GetZIn() + p->GetZOut()) / 2;
                  Double_t deltaZ = z - z2;
                  extrTrdXHistos[3]->Fill(x - x2 - tx * deltaZ);
                  extrTrdYHistos[3]->Fill(y - y2 - ty * deltaZ);
               }
            }
         }
         
         /*for (int k = 0; k < NOF_MUCH_STATIONS * NOF_MUCH_LAYERS; ++k)
         {
            if (trackDesc.much[k].first.empty())
            {
               isRef = false;
               break;
            }
         }
         
         if (!isRef)
            continue;*/
         
         if (trackDesc.tof.first.empty())
            continue;
         
         ++nofRefTracks;
         
         map<Int_t, Int_t> matchedReco;
         
         for (int k = 0; k < NOF_STS_STATIONS; ++k)
         {
            for (set<Int_t>::const_iterator l = trackDesc.sts[k].second.begin(); l != trackDesc.sts[k].second.end(); ++l)
            {
               map<Int_t, Int_t>::iterator mrIter = matchedReco.find(*l);
               
               if (mrIter != matchedReco.end())
                  ++mrIter->second;
               else
                  matchedReco[*l] = 1;
            }
            
            if (!trackDesc.sts[k].second.empty())
               ++nofSts[k];
         }
         
         for (int k = 0; k < NOF_TRD_LAYERS; ++k)
         {
            for (set<Int_t>::const_iterator l = trackDesc.trd[k].second.begin(); l != trackDesc.trd[k].second.end(); ++l)
            {
               map<Int_t, Int_t>::iterator mrIter = matchedReco.find(*l);
               
               if (mrIter != matchedReco.end())
                  ++mrIter->second;
               else
                  matchedReco[*l] = 1;
            }
            
            if (!trackDesc.trd[k].second.empty())
               ++nofTrd[k];
         }
         
         for (int k = 0; k < NOF_STS_STATIONS; ++k)
         {
            if (trackDesc.nearestHitDistSts[k] >= 0)
            {
               stsXPullHistos[k]->Fill(trackDesc.pullXsts[k]);
               stsYPullHistos[k]->Fill(trackDesc.pullYsts[k]);
            }
         }
         
         for (int k = 0; k < NOF_TRD_LAYERS; ++k)
         {
            if (trackDesc.nearestHitDist[k] >= 0)
            {
               trdNearestHitDistHistos[k]->Fill(trackDesc.nearestHitDist[k]);
               trdXPullHistos[k]->Fill(trackDesc.pullX[k]);
               trdYPullHistos[k]->Fill(trackDesc.pullY[k]);
            }
         }
         
         /*for (int k = 0; k < NOF_MUCH_STATIONS * NOF_MUCH_LAYERS; ++k)
         {
            for (set<Int_t>::const_iterator l = trackDesc.much[k].second.begin(); l != trackDesc.much[k].second.end(); ++l)
            {
               map<Int_t, Int_t>::iterator mrIter = matchedReco.find(*l);
               
               if (mrIter != matchedReco.end())
                  ++mrIter->second;
               else
                  matchedReco[*l] = 1;
            }
            
            if (!trackDesc.much[k].second.empty())
               ++nofMuch[k];
         }*/
         
         /*for (set<Int_t>::const_iterator k = trackDesc.tof.second.begin(); k != trackDesc.tof.second.end(); ++k)
         {
            map<Int_t, Int_t>::iterator mrIter = matchedReco.find(*k);
               
            if (mrIter != matchedReco.end())
               ++mrIter->second;
            else
               matchedReco[*k] = 1;
         }
         
         if (!trackDesc.tof.second.empty())
            ++nofTof;*/
         
         if (matchedReco.empty())
            continue;
         
         /*map<Int_t, Int_t>::const_iterator maxIter = max_element(matchedReco.begin(), matchedReco.end(),
            [](const pair<Int_t, Int_t>& p1, const pair<Int_t, Int_t>& p2)
            {
               return p1.second < p2.second;
            });
            
         if (maxIter->second < 7)
            continue;*/
         
         int maxMatch = 0;
         
         for (map<Int_t, Int_t>::const_iterator k = matchedReco.begin(); k != matchedReco.end(); ++k)
         {
            if (maxMatch < k->second)
               maxMatch = k->second;
         }
         
         if (maxMatch < 5)
            continue;
            
         ++nofMatchedRefTracks;
      }
   }
   
   double eff = 100 * nofMatchedRefTracks;
   eff /= nofRefTracks;
   cout << "The track reconstruction efficiency: " << eff << "%: " << nofMatchedRefTracks << "/" << nofRefTracks << endl;
   cout << "Total tracks: " << nofAllTracks << endl;
   cout << "Nof STS[0]: " << nofSts[0] << endl;
   cout << "Nof STS[1]: " << nofSts[1] << endl;
   cout << "Nof TRD[0]: " << nofTrd[0] << endl;
   cout << "Nof TRD[1]: " << nofTrd[1] << endl;
   cout << "Nof TRD[2]: " << nofTrd[2] << endl;
   cout << "Nof TRD[3]: " << nofTrd[3] << endl;
   //cout << "Nof MUCH[0]: " << nofMuch[0] << endl;
   //cout << "Nof MUCH[1]: " << nofMuch[1] << endl;
   //cout << "Nof MUCH[2]: " << nofMuch[2] << endl;
   //cout << "Nof TOF: " << nofTof << endl;
   
   cout << "Nof stranger hits on TRD stations: ";
   
   for (int i = 0; i < 4; ++i)
      cout << "[" << trdNofStrangerHits[i] << "]";
   
   cout << endl;
   
   SaveHisto(stsXResHisto);
   SaveHisto(stsYResHisto);
   SaveHisto(stsTResHisto);
   SaveHisto(trdXResHisto);
   SaveHisto(trdYResHisto);
   SaveHisto(trdTResHisto);
   SaveHisto(muchXResHisto);
   SaveHisto(muchYResHisto);
   SaveHisto(muchTResHisto);
   SaveHisto(tofXResHisto);
   SaveHisto(tofYResHisto);
   SaveHisto(tofTResHisto);
   
   SaveHisto(extrStsXHisto);
   SaveHisto(extrStsYHisto);
   
   SaveHisto(vtxXHisto);
   SaveHisto(vtxYHisto);
   SaveHisto(vtxZHisto);
   
   for (int i = 0; i < 2; ++i)
   {      
      SaveHisto(stsXPullHistos[i]);
      SaveHisto(stsYPullHistos[i]);
   }
   
   for (int i = 0; i < 4; ++i)
   {
      SaveHisto(extrTrdXHistos[i]);
      SaveHisto(extrTrdYHistos[i]);
      
      SaveHisto(trdNearestHitDistHistos[i]);
      
      SaveHisto(trdXPullHistos[i]);
      SaveHisto(trdYPullHistos[i]);
   }
}

void CbmBinnedTrackerQA::SetParContainers()
{
   fSettings = static_cast<CbmBinnedSettings*> (FairRun::Instance()->GetRuntimeDb()->getContainer("CbmBinnedSettings"));
}

ClassImp(CbmBinnedTrackerQA)
