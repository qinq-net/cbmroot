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
#include "CbmMCEventList.h"
#include "CbmTofHit.h"

#ifdef CBM_GLOBALTB_QA
#include "CbmMCDataArray.h"
#endif//CBM_GLOBALTB_QA

class CbmGlobalTrackingTask : public FairTask
{
public:
#ifdef CBM_GLOBALTB_QA
    struct PointData;
    
    struct TrackData
    {
        bool hasSts;
        scaltype x;
        scaltype y;
        scaltype z;
        scaltype t;
        scaltype tx;
        scaltype ty;
        std::list<PointData*> tofPoints;
        bool use;
        int evN;
        int ind;
        std::set<const CbmTofHit*> tofHits;
        bool used;
    };
    
    struct PointData
    {
        scaltype x;
        scaltype y;
        scaltype z;
        scaltype t;
        TrackData* track;
        int evN;
        int ind;
    };
#endif//CBM_GLOBALTB_QA
    
    CbmGlobalTrackingTask();
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
#ifdef CBM_GLOBALTB_QA
    void SetNofEvents(Int_t v) { fNofEvents = v; }
#endif//CBM_GLOBALTB_QA
    
private:
    CbmGlobalTrackingTofGeometry fTofGeometry;
    TClonesArray* fTofHits;
    TClonesArray* fStsTracks;
    
#ifdef CBM_GLOBALTB_QA
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
    CbmTimeSlice* fTimeSlice;
    CbmMCEventList* fEventList;
    std::vector<std::vector<TrackData> > fTracks;
    std::vector<std::vector<PointData> > fTofPoints;
    std::vector<std::vector<PointData> > fStsPoints;
    Int_t fNofEvents;
#endif//CBM_GLOBALTB_QA
    
ClassDef(CbmGlobalTrackingTask, 1)        
};

#endif /* CBMGLOBALTRACKINGTASK_H */

