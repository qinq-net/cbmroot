/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmBinnedTrackerTask.h"
#include "GeoReader.h"
#include "FairLogger.h"
   
CbmBinnedTrackerTask::CbmBinnedTrackerTask() : fTracker(0)
{
}

CbmBinnedTrackerTask::~CbmBinnedTrackerTask()
{
   delete fTracker;
}

InitStatus CbmBinnedTrackerTask::Init()
{
   CbmBinnedGeoReader* geoReader = CbmBinnedGeoReader::Instance();
   
   if (0 == geoReader)
      fLogger->Fatal(MESSAGE_ORIGIN, "Couldn't instantiate CbmBinnedGeoReader");
   
   geoReader->Read();
   
   return kSUCCESS;
}

void CbmBinnedTrackerTask::Exec(Option_t* opt)
{   
}

void CbmBinnedTrackerTask::Finish()
{
}

ClassImp(CbmBinnedTrackerTask)
