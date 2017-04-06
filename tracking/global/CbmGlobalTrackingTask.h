/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CbmGlobalTrackingTask.h
 * Author: tablyaz
 *
 * Created on January 31, 2017, 3:33 PM
 */

#ifndef CBMGLOBALTRACKINGTASK_H
#define CBMGLOBALTRACKINGTASK_H

#include "CbmGlobalTrackingTof.h"
#include "FairTask.h"
#include "TClonesArray.h"
#include "CbmTimeSlice.h"
#include "CbmTofHit.h"

class CbmGlobalTrackingTask : public FairTask
{
public:
    CbmGlobalTrackingTask();
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
    void SetNofTBins(int v)
    {
        fTofGeometry.SetNofTBins(v);
    }
    
    void SetTBinSize(scaltype v)
    {
        fTofGeometry.SetTBinSize(v);
    }
    
private:
    CbmGlobalTrackingTofGeometry fTofGeometry;
    CbmTimeSlice* fTimeSlice;
    TClonesArray* fTofHits;
    TClonesArray* fStsTracks;
    TClonesArray* fGlobalTracks;
    
ClassDef(CbmGlobalTrackingTask, 1)        
};

#endif /* CBMGLOBALTRACKINGTASK_H */

