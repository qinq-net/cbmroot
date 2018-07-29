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
#include "TClonesArray.h"
#include "Tracker.h"

class CbmBinnedTrackerConfigurator : public FairTask
{    
public:
    explicit CbmBinnedTrackerConfigurator(bool useAllDetectors);
    CbmBinnedTrackerConfigurator(const CbmBinnedTrackerConfigurator&) = delete;
    CbmBinnedTrackerConfigurator& operator=(const CbmBinnedTrackerConfigurator&) = delete;
    ~CbmBinnedTrackerConfigurator();

    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    void SetParContainers();
    void SetUse(ECbmModuleId m, bool v) { fUseModules[m] = v; }
    
    void SetPrimaryParticle(EPrimaryParticleId v)
    {
        fPrimaryParticleIds.clear();
        fPrimaryParticleIds.push_back(v);
    }
    
    void AddPrimaryParticle(EPrimaryParticleId v)
    {
        fPrimaryParticleIds.push_back(v);
    }
    
private:
    bool fUseAllDetectors;
    bool fUseModules[kLastModule];
    CbmBinnedTracker* fTracker;
    CbmBinnedSettings* fSettings;
    std::list<EPrimaryParticleId> fPrimaryParticleIds;
    
    TClonesArray* fMCTracks;
    TClonesArray* fStsPoints;
    TClonesArray* fMuchPoints;
    TClonesArray* fTrdPoints;
    TClonesArray* fTofPoints;
    ClassDef(CbmBinnedTrackerConfigurator, 1)
};

#endif /* CBMBINNEDTRACKERCONFIGURATOR_H */
