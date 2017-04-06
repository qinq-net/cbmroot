/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CbmGlobalTrackingQA.h
 * Author: tablyaz
 *
 * Created on April 5, 2017, 5:07 PM
 */

#ifndef CBMGLOBALTRACKINGQA_H
#define CBMGLOBALTRACKINGQA_H

#include "FairTask.h"
#include "TClonesArray.h"
#include "CbmMCDataArray.h"

class CbmGlobalTrackingQA : public FairTask
{
public:
    CbmGlobalTrackingQA();
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
private:
    TClonesArray* fTofHits;
    TClonesArray* fStsTracks;
    TClonesArray* fGlobalTracks;
    TClonesArray* fTofHitDigiMatches;
    TClonesArray* fTofDigis;
    TClonesArray* fStsHitDigiMatches;
    TClonesArray* fStsHits;
    TClonesArray* fStsClusters;
    TClonesArray* fStsDigis;
    CbmMCDataArray* fMCTracks;
    CbmMCDataArray* fStsMCPoints;
    CbmMCDataArray* fTrdMCPoints;
    CbmMCDataArray* fTofMCPoints;
ClassDef(CbmGlobalTrackingQA, 1)
};

#endif /* CBMGLOBALTRACKINGQA_H */

