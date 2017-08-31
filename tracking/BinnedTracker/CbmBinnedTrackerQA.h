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
    CbmBinnedTrackerQA();
    CbmBinnedTrackerQA(const CbmBinnedTrackerQA&) = delete;
    CbmBinnedTrackerQA& operator=(const CbmBinnedTrackerQA&) = delete;
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
private:
    void HandleSts(Int_t stsTrackIndex);
    //void HandleMuch(Int_t muchTrackIndex);
    void HandleTrd(Int_t trdTrackIndex);
    void HandleTof(Int_t tofHitIndex);
    
private:
    TClonesArray* fGlobalTracks;
    TClonesArray* fStsTracks;
    //TClonesArray* fMuchTracks;
    TClonesArray* fTrdTracks;
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
    ClassDef(CbmBinnedTrackerQA, 1)
};

#endif /* CBMBINNEDTRACKERQA_H */

