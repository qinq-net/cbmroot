/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LxTBTrdTask.h
 * Author: tablyaz
 *
 * Created on October 4, 2016, 6:43 PM
 */

#ifndef LXTBTRDTASK_H
#define LXTBTRDTASK_H

#include "FairTask.h"
#include "LxTBBinned.h"
#include "CbmMCDataArray.h"

#define CUR_TIMEBIN_LENGTH 100
#define CUR_NOF_STATIONS 4
#define CUR_LAST_STATION CUR_NOF_STATIONS - 1

class LxTBTrdFinder : public FairTask
{
public:
#ifdef LXTB_QA
    struct TrackDataHolder
    {
        Int_t pointInds[CUR_NOF_STATIONS];
        bool isSignal;
        
        TrackDataHolder() : isSignal(false)
        {
            for (int i = 0; i < CUR_NOF_STATIONS; ++i)
                pointInds[i] = -1;
        }
    };
    
    struct PointDataHolder
    {
        Double_t x;
        Double_t y;
        Double_t z;
        Double_t t;
        Int_t eventId;
        Int_t trackId;
        Int_t pointId;
        Int_t layerNumber;
    };
#endif//LXTB_QA
    
public:
    LxTBTrdFinder();
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    
#ifdef LXTB_QA
    void SetNEvents(Int_t v) { fNEvents = v; }
#endif//LXTB_QA
    
private:
    void HandleGeometry();
    LxTbBinnedFinder* fFinder;
    std::list<LxTbBinnedFinder::Chain*> recoTracks;
    unsigned int nof_timebins;
    unsigned int last_timebin;
    TClonesArray* fTrdHits;
    TClonesArray* fTrdClusters;
    TClonesArray* fTrdDigiMatches;
#ifdef LXTB_QA
    CbmMCDataArray* fTrdMCPoints;
    std::vector<std::vector<TrackDataHolder> > fMCTracks;
    std::vector<std::vector<PointDataHolder> > fTrdPoints;    
    Int_t fNEvents;
#endif//LXTB_QA

ClassDef(LxTBTrdFinder, 1)
};

#endif /* LXTBTRDTASK_H */
