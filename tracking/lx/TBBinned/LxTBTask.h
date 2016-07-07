/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LxTBTask.h
 * Author: Timur Ablyazimov
 *
 * Created on 14 Mar 2016
 */

#ifndef LXTBTASK_H
#define LXTBTASK_H

#include "FairTask.h"
#include "CbmMuchPoint.h"
#include "CbmTrdPoint.h"
#include <vector>

#define CUR_NOF_TRD_LAYERS 4
#define CUR_LAST_TRD_LAYER CUR_NOF_TRD_LAYERS - 1

#include "TGeoManager.h"
#include "CbmPixelHit.h"
#include "CbmMCDataArray.h"

#define CUR_NOF_STATIONS 4
#define CUR_LAST_STATION CUR_NOF_STATIONS - 1
#define CUR_NOF_TIMEBINS 5
#define CUR_LAST_TIMEBIN CUR_NOF_TIMEBINS - 1
#define CUR_TIMEBIN_LENGTH 100

#include "LxTBBinned.h"

class LxTBFinder : public FairTask
{
public:
    LxTBFinder();
    InitStatus Init();// Overridden from FairTask
    void Exec(Option_t* opt);// Overridden from FairTask
    void Finish();// Overridden from FairTask
    void SetEvByEv(bool v = true) { isEvByEv = v; }
    void SetUseTrd(bool v = true) { useTrd = v; }
    void SetUseIdeal(bool v = true) { useIdeal = v; }
    void SetUseAsciiSig(bool v = true) { useAsciiSig = v; }
    void SetSignalParticle(const char* name) { fSignalParticle = name; }
    
private:
    void HandleGeometry();
#ifdef LXTB_QA
    void AddHit(const CbmPixelHit* hit, Int_t stationNumber, bool isTrd);
#else
    void AddHit(const CbmPixelHit* hit, Int_t stationNumber);
#endif//LXTB_QA
    
    struct PointDataHolder
    {
        Double_t x;
        Double_t y;
        Double_t z;
        Double_t t;
        Int_t eventId;
        Int_t trackId;
        Int_t pointId;
        Int_t stationNumber;
        Int_t layerNumber;
    };
    
    std::vector<vector<PointDataHolder> > fMuchPoints;
    CbmMCDataArray* fMuchMCPoints;
    TClonesArray* fMuchPixelHits;
    TClonesArray* fMuchClusters;
    TClonesArray* fMuchPixelDigiMatches;
    
    std::vector<vector<PointDataHolder> > fTrdPoints;
    CbmMCDataArray* fTrdMCPoints;
    TClonesArray* fTrdHits;
    TClonesArray* fTrdClusters;
    TClonesArray* fTrdDigiMatches;
    
//#ifdef LXTB_QA
    struct TrackDataHolder
    {
        Int_t pointInds[CUR_NOF_STATIONS];
        Int_t trdPointInds[CUR_NOF_TRD_LAYERS];
        bool isSignalShort;
        bool isSignalLong;
        bool isPos;
        
        TrackDataHolder() : isSignalShort(false), isSignalLong(false), isPos(false)
        {
            for (int i = 0; i < CUR_NOF_STATIONS; ++i)
                pointInds[i] = -1;
            
            for (int i = 0; i < CUR_NOF_TRD_LAYERS; ++i)
                trdPointInds[i] = -1;
        }
    };
    
    std::vector<vector<TrackDataHolder> > fMCTracks;
//#endif//LXTB_QA
    bool isEvByEv;
    LxTbBinnedFinder* fFinder;
    list<LxTbBinnedFinder::Chain*> recoTracks;
    bool hasTrd;
    bool useTrd;
    bool useIdeal;
    bool useAsciiSig;
    const char* fSignalParticle;

ClassDef(LxTBFinder, 1)
};

#endif /* LXTBTASK_H */
