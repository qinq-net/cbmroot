/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CbmBinnedTrackerTask.h
 * Author: tablyaz
 *
 * Created on August 3, 2017, 4:33 PM
 */

#ifndef CBMBINNEDTRACKERTASK_H
#define CBMBINNEDTRACKERTASK_H

#include "FairTask.h"
#include "Tracker.h"
#include "CbmPixelHit.h"

class CbmBinnedTrackerTask : public FairTask
{
public:
    static CbmBinnedTrackerTask* Instance() { return fInstance; }
    
public:
    CbmBinnedTrackerTask(bool useAllDetectors, Double_t beamWidthX, Double_t beamWidthY);
    CbmBinnedTrackerTask(const CbmBinnedTrackerTask&) = delete;
    CbmBinnedTrackerTask& operator=(const CbmBinnedTrackerTask&) = delete;
    ~CbmBinnedTrackerTask();

    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    void SetParContainers();
    
    Double_t GetBeamDx() const { return fBeamDx; }
    Double_t GetBeamDy() const { return fBeamDy; }
    void SetUse(ECbmModuleId m, bool v) { CbmBinnedSettings::Instance()->SetUse(m, v); }
    
private:
    static CbmBinnedTrackerTask* fInstance;
    
private:
    bool fUseAllDetectors;
    CbmBinnedSettings* fSettings;
    Double_t fBeamDx;
    Double_t fBeamDy;
    CbmBinnedTracker* fTracker;
    TClonesArray* fGlobalTracks;
    TClonesArray* fStsTracks;
    TClonesArray* fMuchTracks;
    TClonesArray* fTrdTracks;
    ClassDef(CbmBinnedTrackerTask, 1)
};

#endif /* CBMBINNEDTRACKERTASK_H */

