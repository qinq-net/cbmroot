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
#include "CbmTofHit.h"
#include "CbmMatch.h"
#include "CbmTofDigiExp.h"
#include <list>
#include <set>
#include <vector>
#include "CbmGlobalTrackingDefs.h"

class CbmGlobalTrackingQA : public FairTask
{
public:
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
        TrackData* parent;
        std::list<TrackData*> offsprings;
 
        TrackData() : hasSts(false), x(0), y(0), z(0), t(0), tx(0), ty(0), 
                      tofPoints(), use(false), evN(0), ind(0), tofHits(),
                      used(false), parent(nullptr), offsprings()
                      {}
//        TrackData(const TrackData&) = delete;
        TrackData& operator=(const TrackData&) = delete;
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
    
public:
    CbmGlobalTrackingQA();

    CbmGlobalTrackingQA(const CbmGlobalTrackingQA&) = delete;
    CbmGlobalTrackingQA& operator=(const CbmGlobalTrackingQA&) = delete;

    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
    void SetNofEvents(Int_t v)
    {
        fNofEvents = v;
    }
    
private:
    bool CheckMatch(const TrackData* stsMCTrack, Int_t tofHitInd, bool deepSearch = false) const;
    bool SemiTofTrack(const TrackData* mcTrack) const;
    
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
    std::vector<std::vector<TrackData> > fTracks;
    std::vector<std::vector<PointData> > fTofPoints;
    std::vector<std::vector<PointData> > fStsPoints;
    Int_t fNofEvents;
ClassDef(CbmGlobalTrackingQA, 1)
};

#endif /* CBMGLOBALTRACKINGQA_H */
