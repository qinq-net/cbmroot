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

const Double_t cbmBinnedSigma = 4;
const Double_t cbmBinnedSigmaSq = cbmBinnedSigma * cbmBinnedSigma;
const Double_t cbmBinnedSOL = 0;

class CbmBinnedStation
{
public:
    CbmBinnedStation() : fZ(0), fDx(0), fDy(0), fDt(0) {}
    virtual ~CbmBinnedStation() {}
    Double_t GetZ() const { return fZ; }
    void SetZ(Double_t v) { fZ = v; }
    Double_t GetDx() const { return fDx; }
    void SetDx(Double_t v) { if (v > fDx) fDx = v; }
    Double_t GetDy() const { return fDy; }
    void SetDy(Double_t v) { if (v > fDy) fDy = v; }
    Double_t GetDt() const { return fDt; }
    void SetDt(Double_t v) { if (v > fDt) fDt = v; }
    virtual void AddHit(const CbmPixelHit* hit, Int_t index) = 0;
    virtual void IterateHits(std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    virtual void SearchHits(const CbmPixelHit* searchHit, std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    
protected:
    Double_t fZ;
    Double_t fDx;
    Double_t fDy;
    Double_t fDt;
};

#endif /* STATION_H */

