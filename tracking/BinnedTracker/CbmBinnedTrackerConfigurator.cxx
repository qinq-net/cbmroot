/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmBinnedTrackerConfigurator.h"
#include "FairLogger.h"
#include "CbmMCDataManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "GeoReader.h"

CbmBinnedTrackerConfigurator::CbmBinnedTrackerConfigurator() : fTracker(0), fSettings(0), fStsHits(0), fMuchHits(0), fTrdHits(0), fTofHits(0),
   fStsClusters(0), fMuchClusters(0), fTrdClusters(0), fTrdDigiMatches(0), fTofHitDigiMatches(0), fTofDigiPointMatches(0),
   fStsDigis(0), fMuchDigis(0), fTrdDigis(0), fTofDigis(0), fMCTracks(0), fStsPoints(0), fMuchPoints(0), fTrdPoints(0), fTofPoints(0)
{
   fSettings = CbmBinnedSettings::Instance();
   fSettings->SetUse(false);
   fSettings->SetConfiguring();
}

CbmBinnedTrackerConfigurator::~CbmBinnedTrackerConfigurator()
{
   delete fTracker;
}

InitStatus CbmBinnedTrackerConfigurator::Init()
{
   CbmBinnedGeoReader* geoReader = CbmBinnedGeoReader::Instance();
   
   if (0 == geoReader)
      fLogger->Fatal(MESSAGE_ORIGIN, "Couldn't instantiate CbmBinnedGeoReader");
   
   geoReader->Read();
   
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   CbmMCDataManager* mcManager = static_cast<CbmMCDataManager*> (ioman->GetObject("MCDataManager"));
   
   if (0 == mcManager)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC data manager");
   
   fMCTracks = mcManager->InitBranch("MCTrack");
   
   if (0 == fMCTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC tracks in the input file");
   
   if (fSettings->Use(kSts))
   {
      fStsHits = static_cast<TClonesArray*> (ioman->GetObject("StsHit"));
   
      if (0 == fStsHits)
         fLogger->Fatal(MESSAGE_ORIGIN, "No sts hits in the input file");
   }
   
   return kSUCCESS;
}

void CbmBinnedTrackerConfigurator::Exec(Option_t* opt)
{
   
}

void CbmBinnedTrackerConfigurator::Finish()
{
   
}

void CbmBinnedTrackerConfigurator::SetParContainers()
{
   fSettings = static_cast<CbmBinnedSettings*> (FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmBinnedSettings"));
}

ClassImp(CbmBinnedTrackerConfigurator)
