/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CbmBinnedTrackerTask.h
 * Author: tablyaz
 *
 * Created on October 10, 2017, 5:07 PM
 */

#ifndef CBMBINNEDTRACKERCONFIGURATOR_H
#define CBMBINNEDTRACKERCONFIGURATOR_H

#include "FairTask.h"
#include "CbmPixelHit.h"
#include "Settings.h"
#include "CbmMCDataArray.h"
#include "TClonesArray.h"
#include "Tracker.h"

class CbmBinnedTrackerConfigurator : public FairTask
{    
public:
    CbmBinnedTrackerConfigurator();
    CbmBinnedTrackerConfigurator(const CbmBinnedTrackerConfigurator&) = delete;
    CbmBinnedTrackerConfigurator& operator=(const CbmBinnedTrackerConfigurator&) = delete;
    ~CbmBinnedTrackerConfigurator();

    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    void SetParContainers();
    void SetUse(ECbmModuleId m, bool v) { CbmBinnedSettings::Instance()->SetUse(m, v); }
    
private:
    CbmBinnedTracker* fTracker;
    CbmBinnedSettings* fSettings;
    TClonesArray* fStsHits;
    TClonesArray* fMuchHits;
    TClonesArray* fTrdHits;
    TClonesArray* fTofHits;
    TClonesArray* fStsClusters;
    TClonesArray* fMuchClusters;
    TClonesArray* fTrdClusters;
    
    TClonesArray* fTrdDigiMatches;
    TClonesArray* fTofHitDigiMatches;
    TClonesArray* fTofDigiPointMatches;
    
    TClonesArray* fStsDigis;
    TClonesArray* fMuchDigis;
    TClonesArray* fTrdDigis;
    TClonesArray* fTofDigis;
    
    CbmMCDataArray* fMCTracks;
    CbmMCDataArray* fStsPoints;
    CbmMCDataArray* fMuchPoints;
    CbmMCDataArray* fTrdPoints;
    CbmMCDataArray* fTofPoints;
    ClassDef(CbmBinnedTrackerConfigurator, 1)
};

#endif /* CBMBINNEDTRACKERCONFIGURATOR_H */
