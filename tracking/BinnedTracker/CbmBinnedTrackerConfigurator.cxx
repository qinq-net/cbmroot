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

CbmBinnedTrackerConfigurator::CbmBinnedTrackerConfigurator(bool useAllDetectors) : fUseAllDetectors(useAllDetectors), fTracker(0), fSettings(0), fMCTracks(0),
   fStsPoints(0), fMuchPoints(0), fTrdPoints(0), fTofPoints(0)
{
   fill_n(fUseModules, int(kLastModule), fUseAllDetectors);
   fUseModules[kMuch] = false;// Temporary hack
   fUseModules[kRich] = false;// Temporary hack
}

CbmBinnedTrackerConfigurator::~CbmBinnedTrackerConfigurator()
{
   delete fTracker;
}

InitStatus CbmBinnedTrackerConfigurator::Init()
{
   fSettings->SetConfiguring(true);
   fSettings->SetUse(fUseModules);
   fSettings->SetNofStsStations(0);
   fSettings->SetNofMuchStations(0);
   fSettings->SetNofTrdStations(0);
   CbmBinnedGeoReader* geoReader = CbmBinnedGeoReader::Instance();
   
   if (0 == geoReader)
      fLogger->Fatal(MESSAGE_ORIGIN, "Couldn't instantiate CbmBinnedGeoReader");
   
   geoReader->Read();
   
   FairRootManager* ioman = FairRootManager::Instance();
    
   if (0 == ioman)
      fLogger->Fatal(MESSAGE_ORIGIN, "No FairRootManager");
   
   fMCTracks = static_cast<TClonesArray*> (ioman->GetObject("MCTrack"));
   
   if (0 == fMCTracks)
      fLogger->Fatal(MESSAGE_ORIGIN, "No MC tracks in the input file");
   
   if (fSettings->Use(kSts))
   {      
      fStsPoints = static_cast<TClonesArray*> (ioman->GetObject("StsPoint"));
   
      if (0 == fStsPoints)
         fLogger->Fatal(MESSAGE_ORIGIN, "No sts MC points in the input file");
   }
   
   if (fSettings->Use(kMuch))
   {
      fMuchPoints = static_cast<TClonesArray*> (ioman->GetObject("MuchPoint"));
   
      if (0 == fMuchPoints)
         fLogger->Fatal(MESSAGE_ORIGIN, "No much MC points in the input file");
   }
   
   if (fSettings->Use(kTrd))
   {
      fTrdPoints = static_cast<TClonesArray*> (ioman->GetObject("TrdPoint"));
      
      if (0 == fTrdPoints)
         fLogger->Fatal(MESSAGE_ORIGIN, "No trd MC points in the input file");
   }
   
   if (fSettings->Use(kTof))
   {
      fTofPoints = static_cast<TClonesArray*> (ioman->GetObject("TofPoint"));
      
      if (0 == fTofPoints)
         fLogger->Fatal(MESSAGE_ORIGIN, "No tof MC points in the input file");
   }
   
   return kSUCCESS;
}

void CbmBinnedTrackerConfigurator::Exec(Option_t* opt)
{
   CbmBinnedHitReader::Instance()->Read();
   CbmBinnedHitReader::Instance()->Handle();
}

void CbmBinnedTrackerConfigurator::Finish()
{
   CbmBinnedHitReader::Instance()->Finish();
}

void CbmBinnedTrackerConfigurator::SetParContainers()
{
   fSettings = static_cast<CbmBinnedSettings*> (FairRunAna::Instance()->GetRuntimeDb()->getContainer("CbmBinnedSettings"));
}

ClassImp(CbmBinnedTrackerConfigurator)
