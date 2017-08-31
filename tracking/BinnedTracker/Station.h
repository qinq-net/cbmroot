/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Station.h
 * Author: tablyaz
 *
 * Created on August 7, 2017, 2:05 PM
 */

#ifndef STATION_H
#define STATION_H

#include "Bins.h"
#include <functional>
#include "HitReader.h"

const Double_t cbmBinnedSigma = 4;
const Double_t cbmBinnedSigmaSq = cbmBinnedSigma * cbmBinnedSigma;
extern Double_t cbmBinnedSOL;

class CbmBinnedStation
{
public:
    CbmBinnedStation(int nofYBins, int nofXBins, int nofTBins) : fMinZ(0), fMaxZ(0), fNofYBins(nofYBins), fNofXBins(nofXBins), fNofTBins(nofTBins),
            fYBinSize(0), fXBinSize(0), fTBinSize(100), fMinY(0), fMaxY(0), fMinX(0), fMaxX(0), fMinT(0), fMaxT(0), fDx(0), fDxSq(0), fDy(0), fDySq(0),
            fDt(0), fDtSq(0), fDefaultUse(false) {}    
    CbmBinnedStation(const CbmBinnedStation&) = delete;
    CbmBinnedStation& operator=(const CbmBinnedStation&) = delete;
    
    virtual ~CbmBinnedStation() {}
    void SetMinZ(Double_t v) { fMinZ = v; }
    void SetMaxZ(Double_t v) { fMaxZ = v; }
    void SetMinY(Double_t v) { fMinY = v; }
    void SetMaxY(Double_t v) { fMaxY = v; }
    void SetMinX(Double_t v) { fMinX = v; }
    void SetMaxX(Double_t v) { fMaxX = v; }
    
    void SetTBinSize(Double_t v) { fTBinSize = v; }
    
    void SetMinT(Double_t v)
    {
        fMinT = v;
        fMaxT = fMinT + fNofTBins * fTBinSize;
    }
    
    void SetDefaultUse() { fDefaultUse = true; }
    
    Double_t GetDx() const { return fDx; }
    
    void SetDx(Double_t v)
    {
        if (v > fDx)
        {
            fDx = v;
            fDxSq = v * v;
        }
    }
    
    Double_t GetDy() const { return fDy; }
    
    void SetDy(Double_t v)
    {
        if (v > fDy)
        {
            fDy = v;
            fDySq = v * v;
        }
    }
    
    Double_t GetDt() const { return fDt; }
    
    void SetDt(Double_t v)
    {
        if (v > fDt)
        {
            fDt = v;
            fDtSq = v * v;
        }
    }
    
    int GetXInd(Double_t v) const
    {
        int ind = (v - fMinX) / fXBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofXBins)
            ind = fNofXBins - 1;
        
        return ind;
    }
    
    int GetYInd(Double_t v) const
    {
        int ind = (v - fMinY) / fYBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofYBins)
            ind = fNofYBins - 1;
        
        return ind;
    }
    
    int GetTInd(Double_t v) const
    {
        int ind = (v - fMinT) / fTBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofTBins)
            ind = fNofTBins - 1;
        
        return ind;
    }
    
    virtual void Init()
    {
        fYBinSize = (fMaxY - fMinY) / fNofYBins;
        fXBinSize = (fMaxX - fMinX) / fNofXBins;
    }
    
    virtual void Clear() = 0;
    
    virtual void AddHit(const CbmPixelHit* hit, Int_t index) = 0;
    virtual void IterateHits(std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    virtual void SearchHits(const CbmPixelHit* searchHit, std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    
protected:
    Double_t fMinZ;
    Double_t fMaxZ;
    int fNofYBins;
    int fNofXBins;
    int fNofTBins;
    Double_t fYBinSize;
    Double_t fXBinSize;
    Double_t fTBinSize;
    Double_t fMinY;
    Double_t fMaxY;
    Double_t fMinX;
    Double_t fMaxX;
    Double_t fMinT;
    Double_t fMaxT;
    Double_t fDx;
    Double_t fDxSq;
    Double_t fDy;
    Double_t fDySq;
    Double_t fDt;
    Double_t fDtSq;
    bool fDefaultUse;
};

#endif /* STATION_H */

