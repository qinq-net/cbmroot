/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LxTBBinned2.h
 * Author: tablyaz
 *
 * Created on October 14, 2016, 11:30 AM
 */

#ifndef LXTBBINNED2_H
#define LXTBBINNED2_H

#include "CbmPixelHit.h"
#include "LxTBDefinitions.h"
#include <list>

struct LxTbBinnedPoint
{
    scaltype x;
    scaltype dx;
    scaltype y;
    scaltype dy;
    timetype t;
    timetype dt;
    bool use;
    Int_t refId;
    
#ifdef LXTB_QA
    const CbmPixelHit* pHit;
    bool isTrd;
    Int_t stationNumber;
    
    struct PointDesc
    {
        Int_t eventId;
        Int_t pointId;
        Int_t trackId;
    };
    
    std::list<PointDesc> mcRefs;
#endif//LXTB_QA
    
    LxTbBinnedPoint(scaltype X, scaltype Dx, scaltype Y, scaltype Dy, timetype T, timetype Dt, Int_t ri, bool Use) : x(X), dx(Dx), y(Y), dy(Dy),
        t(T), dt(Dt), refId(ri), use(Use)
#ifdef LXTB_QA
    , pHit(0), isTrd(false)
#endif//LXTB_QA
    {}
};

class LxTbXBin
{
public:
    LxTbXBin() : use(false), maxDx(0), maxDy(0), maxDt(0) {}
    
    void Clear()
    {
        points.clear();
        maxDx = 0;
        maxDy = 0;
        maxDt = 0;
    }
    
    void AddPoint(const LxTbBinnedPoint& point)
    {
        points.push_back(point);
        
        if (point.dx > maxDx)
            maxDx = point.dx;
        
        if (point.dy > maxDy)
            maxDy = point.dy;
        
        if (point.dt > maxDt)
            maxDt = point.dt;
    }
    
private:
    std::list<LxTbBinnedPoint> points;
    bool use;
    scaltype maxDx;
    scaltype maxDy;
    timetype maxDt;
};

class LxTbYXBin
{
public:
    LxTbYXBin(int nxbs) : xBins(new LxTbXBin[nxbs]), nofXBins(nxbs), use(false) {}
    
    ~LxTbYXBin()
    {
        delete[] xBins;
    }
    
    void Clear()
    {
        for (int i = 0; i < nofXBins; ++i)
            xBins[i].Clear();
    }
    
private:
    LxTbXBin* xBins;
    int nofXBins;
    bool use;
};

class LxTbTYXBin
{
public:
    LxTbTYXBin(int nxbs, int nybs) : yxBins(reinterpret_cast<LxTbYXBin*> (new unsigned char[nybs * sizeof(LxTbYXBin)])), nofYXBins(nybs), use(false)
    {
        for (int i = 0; i < nybs; ++i)
            new (&yxBins[i]) LxTbYXBin(nxbs);
    }
    
    ~LxTbTYXBin()
    {
        delete[] reinterpret_cast<unsigned char*> (yxBins);
    }
    
    void Clear()
    {
        for (int i = 0; i < nofYXBins; ++i)
            yxBins[i].Clear();
    }
    
private:
    LxTbYXBin* yxBins;
    int nofYXBins;
    bool use;
};

class LxTbLayer
{
public:
    LxTbLayer(int nofxb, int nofyb, int noftb) : tyxBins(reinterpret_cast<LxTbTYXBin*> (new unsigned char[noftb * sizeof(LxTbTYXBin)])), nofTYXBins(noftb) {}
    
    ~LxTbLayer()
    {
        delete[] reinterpret_cast<unsigned char*> (tyxBins);
    }
    
    void Clear()
    {
        for (int i = 0; i < nofTYXBins; ++i)
            tyxBins[i].Clear();
    }
    
    
    
private:
    LxTbTYXBin* tyxBins;
    int nofTYXBins;
};

#endif /* LXTBBINNED2_H */

