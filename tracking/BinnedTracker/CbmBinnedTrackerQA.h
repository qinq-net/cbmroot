/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CbmBinnedTrackerQA.h
 * Author: tablyaz
 *
 * Created on August 24, 2017, 6:21 PM
 */

#ifndef CBMBINNEDTRACKERQA_H
#define CBMBINNEDTRACKERQA_H

#include "FairTask.h"
#include "TClonesArray.h"
#include "CbmMCDataArray.h"
#include "CbmStsTrack.h"
#include "CbmMuchTrack.h"
#include "CbmTrdTrack.h"
#include "CbmTofHit.h"

class CbmBinnedTrackerQA : public FairTask
{
public:
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
private:
    void HandleSts(Int_t stsTrackIndex);
    void HandleMuch(const CbmMuchTrack* muchTrack);
    void HandleTrd(const CbmTrdTrack* trdTrack);
    void HandleTof(const CbmTofHit* tofHit);
    
private:
    TClonesArray* fGlobalTracks;
    TClonesArray* fStsTracks;
    TClonesArray* fMuchTracks;
    TClonesArray* fTrdTracks;
    TClonesArray* fTofTracks;
    TClonesArray* fStsHits;
    TClonesArray* fMuchHits;
    TClonesArray* fTrdHits;
    TClonesArray* fTofHits;
    TClonesArray* fStsClusters;
    TClonesArray* fMuchClusters;
    TClonesArray* fTrdClusters;
    
    TClonesArray* fStsDigis;
    
    CbmMCDataArray* fMCTracks;
    CbmMCDataArray* fStsPoints;
    CbmMCDataArray* fMuchPoints;
    CbmMCDataArray* fTrdPoints;
    CbmMCDataArray* fTofPoints;
    ClassDef(CbmBinnedTrackerQA, 1)
};

#endif /* CBMBINNEDTRACKERQA_H */

