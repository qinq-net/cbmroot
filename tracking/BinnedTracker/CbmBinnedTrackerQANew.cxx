/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <set>

#include "CbmBinnedTrackerQANew.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "setup/CbmStsSetup.h"
#include "CbmMCDataManager.h"
#include "global/CbmGlobalTrack.h"
#include "CbmMatch.h"
#include "CbmTrdCluster.h"
#include "CbmMCTrack.h"

using namespace std;

struct MCTrackDesc
{
    //set<Int_t> stsMCPoints;
    //set<Int_t> muchMCPoints;
    map<Int_t, pair<UInt_t, set<Int_t> > > trdMCPointMap = {};// Map an MC point index to TRD station number and a set of reconstructed global track indices.
    //set<Int_t> tofMCPoints;
    
    /*set<Int_t> stsHits;
    set<Int_t> muchHits;
    set<Int_t> trdHits;
    set<Int_t> tofHits;*/
    
    //map<Int_t, int> recoTracks;
    Int_t pdg = -1;
    Int_t parentInd = -1;
};

static vector<vector<MCTrackDesc> > gMCTracks;

CbmBinnedTrackerQANew::CbmBinnedTrackerQANew()
{
}

InitStatus CbmBinnedTrackerQANew::Init()
{
   CbmStsSetup* stsSetup = CbmStsSetup::Instance();
   
   if (!stsSetup->IsInit())
      stsSetup->Init();
   
   fSettings = CbmBinnedSettings::Instance();
   
   fMinTrackLength += fSettings->Use(kSts) ? fSettings->GetNofStsStations() : 0;
   fMinTrackLength += fSettings->Use(kMuch) ? fSettings->GetNofMuchStations() : 0;
   fMinTrackLength += fSettings->Use(kTrd) ? fSettings->GetNofTrdStations() : 0;
   fMinTrackLength += fSettings->Use(kTof) ? 1 : 0;
   int canSkipHits = 0.3 * fMinTrackLength;
   fMinTrackLength -= canSkipHits;
   
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   CbmMCDataManager* mcManager = static_cast<CbmMCDataManager*> (ioman->GetObject("MCDataManager"));
   
   if (0 == mcManager)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC data manager");
   
   fMCTracks = mcManager->InitBranch("MCTrack");
   
   if (0 == fMCTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC tracks in the input file");
   
   for (int i = 0; fMCTracks->Size(0, i) >= 0; ++i)
   {
      gMCTracks.push_back(vector<MCTrackDesc>());
      
      auto nofMcTracks = fMCTracks->Size(0, i);
      auto& eventTracks = gMCTracks.back();
      eventTracks.resize(nofMcTracks);
      
      for (Int_t j = 0; j < nofMcTracks; ++j)
      {
         MCTrackDesc& track = eventTracks[j];
         const CbmMCTrack* mcTrack = static_cast<const CbmMCTrack*> (fMCTracks->Get(0, i, j));
         track.pdg = mcTrack->GetPdgCode();
         track.parentInd = mcTrack->GetMotherId();
      }
   }// for (Int_t i = 0; fMCTracks->Size(0, i) >= 0; ++i)
   
   fGlobalTracks = static_cast<TClonesArray*> (ioman->GetObject("GlobalTrack"));
   
   if (0 == fGlobalTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No global tracks in the input file");
   
   if (fSettings->Use(kTrd))
   {
      fTrdTracks = static_cast<TClonesArray*> (ioman->GetObject("TrdTrack"));
   
      if (0 == fTrdTracks)
         fLogger->Fatal(MESSAGE_ORIGIN, "No trd tracks in the input file");
      
      fTrdHits = static_cast<TClonesArray*> (ioman->GetObject("TrdHit"));
   
      if (0 == fTrdHits)
         fLogger->Fatal(MESSAGE_ORIGIN, "No trd hits in the input file");

      fTrdClusters = static_cast<TClonesArray*> (ioman->GetObject("TrdCluster"));
   
      if (0 == fTrdClusters)
         fLogger->Fatal(MESSAGE_ORIGIN, "No global tracks in the input file");
   
      fTrdDigiMatches = static_cast<TClonesArray*> (ioman->GetObject("TrdDigiMatch"));
   
      if (0 == fTrdDigiMatches)
         fLogger->Fatal(MESSAGE_ORIGIN, "No trd hit to digi matches in the input file");
      
      fTrdPoints = mcManager->InitBranch("TrdPoint");
   
      if (0 == fTrdPoints)
         fLogger->Fatal(MESSAGE_ORIGIN, "No trd MC points in the input file");
      
      for (Int_t i = 0; fTrdPoints->Size(0, i) >= 0; ++i)
      {
         Int_t nofPoints = fTrdPoints->Size(0, i);
         vector<MCTrackDesc>& tracks = gMCTracks[i];
      
         for (Int_t j = 0; j < nofPoints; ++j)
         {
            const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, i, j));
            Int_t trackId = trdPoint->GetTrackID();
            int stationNumber = CbmTrdAddress::GetLayerId(trdPoint->GetModuleAddress());
            auto& trackDesk = tracks[trackId];
            trackDesk.trdMCPointMap[j] = { stationNumber, set<Int_t>() };
         }
      }
   }
   
   return kSUCCESS;
}

static Int_t gEventNumber = 0;
static int gNofRecoTracks = 0;
static int gNofMatchedRecoTracks = 0;
static int gNofClones = 0;

void CbmBinnedTrackerQANew::Exec(Option_t*)
{
    auto& eventMCTracks = gMCTracks[gEventNumber];
    Int_t nofGlobalTracks = fGlobalTracks->GetEntriesFast();
    
    for (Int_t i = 0; i < nofGlobalTracks; ++i)
    {
        const CbmGlobalTrack* globalTrack = static_cast<const CbmGlobalTrack*> (fGlobalTracks->At(i));
        //Int_t stsIndex = globalTrack->GetStsTrackIndex();
        //Int_t muchIndex = globalTrack->GetMuchTrackIndex();
        Int_t trdTrackIndex = globalTrack->GetTrdTrackIndex();
        //Int_t tofIndex = globalTrack->GetTofHitIndex();

        if (fSettings->Use(kTrd) && trdTrackIndex < 0)
            continue;
        
        ++gNofRecoTracks;
        
        map<void*, set<Int_t> > mcTrackPtrs;
        
        const CbmTrdTrack* trdTrack = static_cast<const CbmTrdTrack*> (fTrdTracks->At(trdTrackIndex));
        Int_t nofTrdHits = trdTrack->GetNofHits();

        for (Int_t j = 0; j < nofTrdHits; ++j)
        {
            Int_t trdHitInd = trdTrack->GetHitIndex(j);
            const CbmTrdHit* trdHit = static_cast<const CbmTrdHit*> (fTrdHits->At(trdHitInd));
            Int_t clusterId = trdHit->GetRefId();
            const CbmTrdCluster* cluster = static_cast<const CbmTrdCluster*> (fTrdClusters->At(clusterId));
            Int_t nofDigis = cluster->GetNofDigis();

            for (Int_t k = 0; k < nofDigis; ++k)
            {
                Int_t digiId = cluster->GetDigi(k);
                const CbmMatch* match = static_cast<const CbmMatch*> (fTrdDigiMatches->At(digiId));
                Int_t nofLinks = match->GetNofLinks();

                for (Int_t m = 0; m < nofLinks; ++m)
                {
                    const CbmLink& link = match->GetLink(m);
                    Int_t eventId = link.GetEntry();
                    Int_t mcPointId = link.GetIndex();
                    const CbmTrdPoint* trdPoint = static_cast<const CbmTrdPoint*> (fTrdPoints->Get(0, eventId, mcPointId));
                    Int_t trackId = trdPoint->GetTrackID();
                    auto& mcTrack = eventMCTracks[trackId];
                    auto recoIter = mcTrack.trdMCPointMap.find(mcPointId);
                    
                    if (recoIter != mcTrack.trdMCPointMap.end())
                        recoIter->second.second.insert(i);
                    
                    mcTrackPtrs[&mcTrack].insert(j);
                }
            }
        }// for (Int_t j = 0; j < nofTrdHits; ++j)
        
        for (const auto& j : mcTrackPtrs)
        {            
            if (j.second.size() >= int(0.7 * nofTrdHits))
            {
                ++gNofMatchedRecoTracks;
                break;
            }
        }
    }// for (Int_t i = 0; i < nofGlobalTracks; ++i)
    
    ++gEventNumber;
}//Exec()

void CbmBinnedTrackerQANew::Finish()
{
    int nofRefMCTRacks = 0;
    int nofMatchedRefMCTracks = 0;
    int maxTrackLength = 0;
    
    for (const auto& i : gMCTracks)
    {
        for (const auto& j : i)
        {            
            set<UInt_t> stationNofs;
            
            for (const auto& k : j.trdMCPointMap)
                stationNofs.insert(k.second.first);
            
            if (stationNofs.size() > maxTrackLength)
                maxTrackLength = stationNofs.size();
            
            if (stationNofs.size() < fMinTrackLength)
                continue;
            
            map<Int_t, set<UInt_t> > recoToStationMatches;
            
            for (const auto& k : j.trdMCPointMap)
            {   
                for (const auto& m : k.second.second)
                    recoToStationMatches[m].insert(k.second.first);
            }
            
            int nofMatchedReco = 0;
            
            for (const auto& k : recoToStationMatches)
            {
                if (k.second.size() >= int(stationNofs.size() * 0.7))
                    ++nofMatchedReco;
            }
            
            if (nofMatchedReco > 1)
                gNofClones += nofMatchedReco - 1;
            
            //if (abs(j.pdg) != 11)
                //continue;
            
            if (fPrimaryParticlePdg >= 0 && (j.parentInd < 0 || i[j.parentInd].pdg != fPrimaryParticlePdg))
                continue;
            
            ++nofRefMCTRacks;
            
            if (nofMatchedReco > 0)
                ++nofMatchedRefMCTracks;
        }
    }
    
    cout << "Maximum MC track length: " << maxTrackLength << endl;
    
    double res = 100 * nofMatchedRefMCTracks;
    res /= nofRefMCTRacks;
    cout << "The reconstruction efficiency: " << res << " % = 100 * " << nofMatchedRefMCTracks << " / " << nofRefMCTRacks << endl;
    
    res = 100 * (gNofRecoTracks - gNofMatchedRecoTracks);
    res /= gNofRecoTracks;
    cout << "The share of ghosts: " << res << " % = 100 * (" << gNofRecoTracks << " - " << gNofMatchedRecoTracks << ") / " << gNofRecoTracks << endl;
    
    res = 100 * gNofClones;
    res /= gNofRecoTracks;
    cout << "The share of clones: " << res << " % = 100 * " << gNofClones << " / " << gNofRecoTracks << endl;
}

void CbmBinnedTrackerQANew::SetParContainers()
{
   fSettings = static_cast<CbmBinnedSettings*> (FairRun::Instance()->GetRuntimeDb()->getContainer("CbmBinnedSettings"));
}

ClassImp(CbmBinnedTrackerQANew)
