/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "CbmBinnedTrackerTask.h"
   
CbmBinnedTrackerTask::CbmBinnedTrackerTask()
{
   
}

InitStatus CbmBinnedTrackerTask::Init()
{
   return kSUCCESS;
}

void CbmBinnedTrackerTask::Exec(Option_t* opt)
{   
   CbmBinnedTracker tracker;
   int qq = 0;
}

void CbmBinnedTrackerTask::Finish()
{
   
}

ClassImp(CbmBinnedTrackerTask)
