/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmGlobalTrackingQA.h"
#include "CbmTofHit.h"
#include "FairLogger.h"
#include "CbmMCDataManager.h"
#include <list>
#include <set>

using std::list;
using std::set;

#define scaltype double

struct PointData;
    
    struct TrackData
    {
        bool hasSts;
        scaltype x;
        scaltype y;
        scaltype z;
        scaltype t;
        scaltype tx;
        scaltype ty;
        list<PointData*> tofPoints;
        bool use;
        int evN;
        int ind;
        set<const CbmTofHit*> tofHits;
        bool used;
        TrackData* parent;
        list<TrackData*> offsprings;
    };
    
    struct PointData
    {
        scaltype x;
        scaltype y;
        scaltype z;
        scaltype t;
        TrackData* track;
        int evN;
        int ind;
    };

CbmGlobalTrackingQA::CbmGlobalTrackingQA() : fTofHits(0), fStsTracks(0), fGlobalTracks(0), fTofHitDigiMatches(0), fTofDigis(0),
   fStsHitDigiMatches(0), fStsHits(0), fStsClusters(0), fStsDigis(0), fMCTracks(0), fStsMCPoints(0), fTrdMCPoints(0), fTofMCPoints(0)
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
   
   return kSUCCESS;
}

void CbmGlobalTrackingQA::Exec(Option_t* opt)
{
   
}

void CbmGlobalTrackingQA::Finish()
{       
}

ClassImp(CbmGlobalTrackingQA)
   