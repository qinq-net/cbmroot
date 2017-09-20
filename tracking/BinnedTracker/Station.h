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
#include <set>
#include <iostream>

const Double_t cbmBinnedSigma = 4;
const Double_t cbmBinnedSigmaSq = cbmBinnedSigma * cbmBinnedSigma;
extern Double_t cbmBinnedSOL;
const Double_t cbmBinnedCrazyChiSq = 100000;

class CbmBinnedStation
{
public:
    struct Segment
    {
        CbmTBin::HitHolder* begin;
        CbmTBin::HitHolder* end;
        Double_t chiSq;
        std::list<Segment*> children;
        Segment* bestBranch;
        
        Segment(CbmTBin::HitHolder* beginHit, CbmTBin::HitHolder* endHit) : begin(beginHit), end(endHit), chiSq(cbmBinnedCrazyChiSq), children(), bestBranch(0) {}
        Segment(const Segment&) = default;
        Segment& operator=(const Segment&) = default;
    };
    
    struct SegmentComp
    {
        bool operator()(const Segment& s1, const Segment& s2)
        {
            if (&s1.begin < &s2.begin)
                return true;
            else if (&s1.end < &s2.end)
                return true;
            else
                return false;
        }
    };
    
public:
    CbmBinnedStation(Double_t minZ, Double_t maxZ, int nofYBins, int nofXBins, int nofTBins);
    CbmBinnedStation(const CbmBinnedStation&) = delete;
    CbmBinnedStation& operator=(const CbmBinnedStation&) = delete;
    
    virtual ~CbmBinnedStation() {}
    Double_t GetMinZ() const { return fMinZ; }
    Double_t GetMaxZ() const { return fMaxZ; }
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
    
    Double_t GetScatX() const { return fScatX; }
    Double_t GetScatXSq() const { return fScatXSq; }
    
    void SetScatX(Double_t v)
    {
        fScatX = v;
        fScatXSq = v * v;
    }
    
    Double_t GetScatY() const { return fScatY; }
    Double_t GetScatYSq() const { return fScatYSq; }
    
    void SetScatY(Double_t v)
    {
        fScatY = v;
        fScatYSq = v * v;
    }
    
    Double_t GetNofSigmaX() const { return fNofSigmasX; }
    Double_t GetNofSigmaXSq() const { return fNofSigmasXSq; }
    
    void SetNofSigmaX(Double_t v)
    {
        fNofSigmasX = v;
        fNofSigmasXSq = v * v;
    }
    
    Double_t GetNofSigmaY() const { return fNofSigmasY; }
    Double_t GetNofSigmaYSq() const { return fNofSigmasYSq; }
    
    void SetNofSigmaY(Double_t v)
    {
        fNofSigmasY = v;
        fNofSigmasYSq = v * v;
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
    
    virtual void Clear()
    {
        fSegments.clear();
    }
    
    virtual void AddHit(const CbmPixelHit* hit, Int_t index) = 0;
    virtual void IterateHits(std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    virtual void SearchHits(Segment& segment, std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    virtual void SearchHits(Double_t minZ, Double_t maxZ, Double_t minY, Double_t maxY, Double_t minX, Double_t maxX, Double_t minT, Double_t maxT,
        std::function<void(CbmTBin::HitHolder&)> handleHit) = 0;
    
    void IterateSegments(std::function<void(Segment&)> handleSegment)
    {
        for (std::set<Segment, SegmentComp>::iterator i = fSegments.begin(); i != fSegments.end(); ++i)
            handleSegment(const_cast<Segment&> (*i));
    }
    
    void CreateSegmentsFromHits()
    {
        IterateHits(
            [&](CbmTBin::HitHolder& hitHolder)->void
            {
                Segment segment(&fVertexHolder, &hitHolder);
                fSegments.insert(segment);
            }
        );
    }
    
    void NulifySegments()
    {
        for (std::set<Segment, SegmentComp>::iterator i = fSegments.begin(); i != fSegments.end(); ++i)
        {
            Segment& segment = const_cast<Segment&> (*i);
            segment.chiSq = 0;
        }
    }
    
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
    Double_t fScatX;
    Double_t fScatXSq;
    Double_t fScatY;
    Double_t fScatYSq;
    
public:
    std::set<Segment, SegmentComp> fSegments;
    CbmTBin fVertexBin;
    CbmTBin::HitHolder fVertexHolder;
    
protected:
    Double_t fNofSigmasX;
    Double_t fNofSigmasXSq;
    Double_t fNofSigmasY;
    Double_t fNofSigmasYSq;
};

#endif /* STATION_H */

