/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   CbmGlobalTrackingTof.h
 * Author: tablyaz
 *
 * Created on January 31, 2017, 5:51 PM
 */

#ifndef CBMGLOBALTRACKINGTOF_H
#define CBMGLOBALTRACKINGTOF_H

#include <list>
#include "CbmGlobalTrackingDefs.h"
#include "FairTrackParam.h"
#include "TClonesArray.h"
#include <set>
#include <map>

#define CBM_GLOBALTB_TOF_3D

#ifdef CBM_GLOBALTB_TOF_3D
//#define CBM_GLOBALTB_TOF_3D_CUBOIDS
#endif//CBM_GLOBALTB_TOF_3D

#ifdef CBM_GLOBALTB_TOF_3D
#ifdef CBM_GLOBALTB_TOF_3D_CUBOIDS
struct Cuboid;
#endif//CBM_GLOBALTB_TOF_3D_CUBOIDS
struct ZBin;
#endif//CBM_GLOBALTB_TOF_3D

class CbmGlobalTrackingTofGeometry
{
public:
    struct Rect
    {
        scaltype minX;
        scaltype maxX;
        scaltype minY;
        scaltype maxY;
    };
    
    CbmGlobalTrackingTofGeometry();
    ~CbmGlobalTrackingTofGeometry();
    bool Read();
    
    int GetXInd(scaltype x) const
    {
        int ind = (x - fMinX) / fXBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofXBins)
            ind = fNofXBins - 1;
        
        return ind;
    }
    
    int GetYInd(scaltype y) const
    {
        int ind = (y - fMinY) / fYBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofYBins)
            ind = fNofYBins - 1;
        
        return ind;
    }
    
    int GetZInd(scaltype z) const
    {
        int ind = (z - fMinZ) / fZBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofZBins)
            ind = fNofZBins - 1;
        
        return ind;
    }
    
    int GetTInd(timetype t) const
    {
        int ind = (t - fStartTime) / fTBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofTBins)
            ind = fNofTBins - 1;
        
        return ind;
    }
    
    void SetNofTBins(int v)
    {
        fNofTBins = v;
    }
    
    void SetTBinSize(scaltype v)
    {
        fStartTime = v;
    }
    
    void SetStartTime(timetype v)
    {
        fTBinSize = v;
    }
    
    void SetTofHits(TClonesArray* v)
    {
        fTofHits = v;
    }
    
    void Clear();
    void Prepare(timetype startTime);
    void Find(const FairTrackParam& trackParams, timetype trackTime, timetype errT, Int_t& tofHitInd, Double_t& length);
    void Find(scaltype x1, scaltype y1, scaltype z1, scaltype tx, scaltype ty, std::map<int, std::map<int, std::map<int, scaltype> > >& inds);
    //void Find(scaltype x0, scaltype errX, scaltype y0, scaltype errY, scaltype z0, scaltype t0, scaltype errT,
                //scaltype tx, scaltype errTx, scaltype ty, scaltype errTy, Int_t& tofHitInd);
    
private:
    scaltype fC;
#ifdef CBM_GLOBALTB_TOF_3D_CUBOIDS
    std::list<Cuboid*> fCuboids;
#endif//CBM_GLOBALTB_TOF_3D_CUBOIDS
    int fNofTBins;
    int fNofXBins;
    int fNofYBins;
    int fNofZBins;
    ZBin* fZBins;
    scaltype fTBinSize;
    scaltype fMinX;
    scaltype fMaxX;
    scaltype fXBinSize;
    scaltype fMinY;
    scaltype fMaxY;
    scaltype fYBinSize;
    scaltype fMinZ;
    scaltype fMaxZ;
    scaltype fZBinSize;
    timetype fStartTime;
    timetype fEndTime;
    TClonesArray* fTofHits;
};

#endif /* CBMGLOBALTRACKINGTOF_H */
