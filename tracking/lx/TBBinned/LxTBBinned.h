/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LxTBBinned.h
 * Author: Timur Ablyazimov
 *
 * Created on 31 Mar 2016
 */

#ifndef LXTBBINNED_H
#define LXTBBINNED_H

#include "LxTBMatEffs.h"
#include <time.h>
#include <list>
#include <memory>
#include "CbmPixelHit.h"

#define NOF_SIGMAS 4
#define NOF_SIGMAS_SQ NOF_SIGMAS * NOF_SIGMAS
#define TIME_ERROR 4

using namespace std;

static Double_t speedOfLight = 0;

struct LxTbBinnedRay;

struct LxTbBinnedPoint
{
    scaltype x;
    scaltype dx;
    scaltype y;
    scaltype dy;
    scaltype t;
    scaltype dt;
    bool use;
    list<LxTbBinnedRay> neighbours;
    
#ifdef LXTB_QA
    Int_t eventId;
    Int_t trackId;
    const CbmPixelHit* pHit;
    bool isTrd;
    
    struct PointDesc
    {
        Int_t eventId;
        Int_t pointId;
        Int_t trackId;
    };
    
    list<PointDesc> mcRefs;
#endif//LXTB_QA
    
    LxTbBinnedPoint(scaltype X, scaltype Dx, scaltype Y, scaltype Dy, scaltype T, scaltype Dt, bool Use) : x(X), dx(Dx), y(Y), dy(Dy),
        t(T), dt(Dt), use(Use)
#ifdef LXTB_QA
    , pHit(0), isTrd(false)
#endif//LXTB_QA
    {}
};

struct LxTbBinnedRay
{
    const LxTbBinnedPoint* lPoint;
    scaltype tx;
    scaltype dtxSq;
    scaltype ty;
    scaltype dtySq;
    scaltype chi2;
    
    LxTbBinnedRay(scaltype deltaZ, const LxTbBinnedPoint& rP, const LxTbBinnedPoint& lP, scaltype Chi2) : lPoint(&lP), tx((lP.x - rP.x) / deltaZ),
        dtxSq((lP.dx - rP.dx) * (lP.dx - rP.dx) / (deltaZ * deltaZ)), ty((lP.y - rP.y) / deltaZ),
        dtySq((lP.dy - rP.dy) * (lP.dy - rP.dy) / (deltaZ * deltaZ)), chi2(Chi2)
    {
    }
};

struct LxTbXBin
{
    list<LxTbBinnedPoint> points;
    bool use;
    
    LxTbXBin() : use(false) {}
    
    void Clear()
    {
        points.clear();
    }
};

struct LxTbYXBin
{
    LxTbXBin* xBins;
    int nofXBins;
    bool use;
    
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
};

struct LxTbTYXBin
{
    LxTbYXBin* yxBins;
    int nofYXBins;
    bool use;
    
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
};

struct LxTbBinnedStation
{
    int stationNumber;
    scaltype z;
    int nofXBins;
    int nofYBins;
    int nofTYXBins;
    int lastXBin;
    int lastYBin;
    scaltype minX;
    scaltype maxX;
    scaltype binSizeX;
    scaltype minY;
    scaltype maxY;
    scaltype binSizeY;
    LxTbAbsorber absorber;
    scaltype deltaThetaX;
    scaltype deltaThetaY;
    scaltype dispX;
    scaltype dispY;
    LxTbTYXBin* tyxBins;
    
    LxTbBinnedStation(int nofxb, int nofyb, int noftb) : nofXBins(nofxb), nofYBins(nofyb), nofTYXBins(noftb), lastXBin(nofxb - 1), lastYBin(nofyb - 1),
        tyxBins(reinterpret_cast<LxTbTYXBin*> (new unsigned char[noftb * sizeof(LxTbTYXBin)]))
    {
        for (int i = 0; i < noftb; ++i)
            new (&tyxBins[i]) LxTbTYXBin(nofXBins, nofYBins);
    }
    
    ~LxTbBinnedStation()
    {
        delete[] reinterpret_cast<unsigned char*> (tyxBins);
    }
    
    void Clear()
    {
        for (int i = 0; i < nofTYXBins; ++i)
            tyxBins[i].Clear();
    }
            
};

struct LxTbBinnedTrdStation
{
    scaltype Zs[4];
    int nofXBins;
    int nofYBins;
    int nofTYXBins;
    int lastXBin;
    int lastYBin;
    scaltype minX;
    scaltype maxX;
    scaltype binSizeX;
    scaltype minY;
    scaltype maxY;
    scaltype binSizeY;
    LxTbAbsorber absorber;
    scaltype deltaThetaX;
    scaltype deltaThetaY;
    scaltype dispXs[4];
    scaltype dispYs[4];
    LxTbTYXBin* tyxBinsArr[4];
    
    LxTbBinnedTrdStation(int nofxb, int nofyb, int noftb) : nofXBins(nofxb), nofYBins(nofyb), nofTYXBins(noftb), lastXBin(nofxb - 1), lastYBin(nofyb - 1)
    {
        for (int i = 0; i < 4; ++i)
        {
            tyxBinsArr[i] = reinterpret_cast<LxTbTYXBin*> (new unsigned char[noftb * sizeof(LxTbTYXBin)]);
            
            for (int j = 0; j < noftb; ++j)
            {
                new (&tyxBinsArr[i][j]) LxTbTYXBin(nofXBins, nofYBins);
            }
        }
    }
    
    ~LxTbBinnedTrdStation()
    {
        for (int i = 0; i < 4; ++i)
            delete[] reinterpret_cast<unsigned char*> (tyxBinsArr[i]);
    }
    
    void Clear()
    {
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < nofTYXBins; ++j)
                tyxBinsArr[i][j].Clear();
        }
    }
};

static long fullDuration = 0;

struct LxTbBinnedFinder
{
    struct Chain
    {
        LxTbBinnedPoint** points;
        int nofPoints;
        scaltype chi2;
        bool highMom;
        
        Chain(const LxTbBinnedPoint* const* pts, int nofPts, scaltype Chi2) : points(new LxTbBinnedPoint*[nofPts]),
            nofPoints(nofPts), chi2(Chi2), highMom(false)
        {
            for (int i = 0; i < nofPoints; ++i)
                points[i] = new LxTbBinnedPoint(*pts[i]);
        }
        
        ~Chain()
        {
            for (int i = 0; i < nofPoints; ++i)
                delete points[i];
            
            delete[] points;
        }
    };
    
    struct ChainImpl
    {
        //const LxTbBinnedPoint* points[];// Change to dynamical allocation
        const LxTbBinnedPoint** points;
        scaltype chi2;
        
        ChainImpl(const LxTbBinnedPoint** pts, int nofPts, scaltype Chi2) : points(new const LxTbBinnedPoint*[nofPts]), chi2(Chi2)
        {
            memcpy(points, pts, nofPts * sizeof(const LxTbBinnedPoint*));
        }
        
        void Destroy()
        {
            delete[] points;
        }
    };
    
    struct TriggerTimeArray
    {
        TriggerTimeArray(int noftb, int tbl, scaltype& mt) : nofTimebins(noftb), tbLength(tbl), minT(mt), triggerTimeBins(new list<pair<scaltype, scaltype> > [noftb])
        {
        }
        
        ~TriggerTimeArray()
        {
            delete[] triggerTimeBins;
        }
        
        void Clear()
        {
            for (int i = 0; i < nofTimebins; ++i)
                triggerTimeBins[i].clear();
        }
        
        void Insert(const pair<scaltype, scaltype>& v)
        {
            scaltype wT = NOF_SIGMAS * sqrt(2.0) * v.second;
            int minInd = (v.first - wT - minT) / tbLength;
            int maxInd = (v.first + wT - minT) / tbLength;
            
            if (minInd < 0)
                minInd = 0;
            else if (minInd >= nofTimebins)
                minInd = nofTimebins - 1;
            
            if (maxInd < 0)
                maxInd = 0;
            else if (maxInd >= nofTimebins)
                maxInd = nofTimebins - 1;
            
            bool busy = false;
            
            for (int i = minInd; i <= maxInd; ++i)
            {
                for (list<pair<scaltype, scaltype> >::const_iterator j = triggerTimeBins[i].begin(); j != triggerTimeBins[i].end(); ++j)
                {
                    const pair<scaltype, scaltype>& p = *j;
                    
                    if (fabs(v.first - p.first) <= NOF_SIGMAS * sqrt(v.second * v.second + p.second * p.second))
                    {
                        busy = true;
                        break;
                    }
                }
                
                if (busy)
                    break;
            }
            
            if (!busy)
            {
                int ind = (v.first - minT) / tbLength;
                
                if (ind < 0)
                    ind = 0;
                else if (ind >= nofTimebins)
                    ind = nofTimebins - 1;
                
                triggerTimeBins[ind].push_back(v);
            }
        }
        
        int nofTimebins;
        int tbLength;
        scaltype& minT;
        list<pair<scaltype, scaltype> >* triggerTimeBins;
    };
    
    struct SignalParticle
    {
        const char* fName;
        int fPdgCode;
        scaltype fMinEnergy;
        bool fHasTrd;
    };
    
    static SignalParticle particleDescs[];
    SignalParticle* fSignalParticle;
    
    LxTbBinnedStation* stations;
    //int nofStations;
    LxTbBinnedTrdStation trdStation;
    scaltype minT;
    scaltype maxT;
    list<Chain*>* recoTracks;
    int nofTrackBins;
    bool fHasTrd;
    int fNofTrdLayers;
    
    TriggerTimeArray triggerTimes_trd0_sign0_dist0;
    TriggerTimeArray triggerTimes_trd0_sign0_dist1;
    TriggerTimeArray triggerTimes_trd0_sign1_dist0;
    TriggerTimeArray triggerTimes_trd0_sign1_dist1;
    TriggerTimeArray triggerTimes_trd1_sign0_dist0;
    TriggerTimeArray triggerTimes_trd1_sign0_dist1;
    TriggerTimeArray triggerTimes_trd1_sign1_dist0;
    TriggerTimeArray triggerTimes_trd1_sign1_dist1;

    int fNofStations;
    int fLastStationNumber;
    int fNofTimeBins;
    int fLastTimeBinNumber;
    int fTimeBinLength;
    int fTimeSliceLength;
    
    LxTbBinnedFinder(int nofTrdLayers, int nofStations, int nofTimeBins, pair<int, int>* nofSpatBins, int nofTrdXBins, int nofTrdYBins, int timeBinLength) :
        fSignalParticle(&particleDescs[0]),
        stations(reinterpret_cast<LxTbBinnedStation*> (new unsigned char[nofStations * sizeof(LxTbBinnedStation)])),
        trdStation(nofTrdXBins, nofTrdYBins, nofTimeBins), fNofStations(nofStations), fLastStationNumber(nofStations - 1), minT(0), maxT(0),
        recoTracks(new list<Chain*>[nofTimeBins]), nofTrackBins(nofTimeBins),
        fHasTrd(nofTrdLayers > 0), fNofTrdLayers(nofTrdLayers), fTimeBinLength(timeBinLength), fNofTimeBins(nofTimeBins), fLastTimeBinNumber(nofTimeBins - 1), fTimeSliceLength(nofTimeBins * timeBinLength),
        triggerTimes_trd0_sign0_dist0(nofTimeBins, timeBinLength, minT),
        triggerTimes_trd0_sign0_dist1(nofTimeBins, timeBinLength, minT),
        triggerTimes_trd0_sign1_dist0(nofTimeBins, timeBinLength, minT),
        triggerTimes_trd0_sign1_dist1(nofTimeBins, timeBinLength, minT),
        triggerTimes_trd1_sign0_dist0(nofTimeBins, timeBinLength, minT),
        triggerTimes_trd1_sign0_dist1(nofTimeBins, timeBinLength, minT),
        triggerTimes_trd1_sign1_dist0(nofTimeBins, timeBinLength, minT),
        triggerTimes_trd1_sign1_dist1(nofTimeBins, timeBinLength, minT)
    {
        for (int i = 0; i < fNofStations; ++i)
            new (&stations[i]) LxTbBinnedStation(nofSpatBins[i].first, nofSpatBins[i].second, nofTimeBins);
    }
    
    virtual ~LxTbBinnedFinder()
    {
        for (int i = 0; i < fNofStations; ++i)
            delete[] reinterpret_cast<unsigned char*> (&stations[i]);
            
        delete[] recoTracks;
    }
    
    void SetSignalParticle(const char* name)
    {
        for (int i = 0; true; ++i)
        {
            if (0 == strlen(particleDescs[i].fName))
                break;
            
            if (0 == strcmp(particleDescs[i].fName, name))
            {
                fSignalParticle = &particleDescs[i];
                break;
            }
        }
    }
    
    void Clear()
    {        
        for (int i = 0; i < fNofStations; ++i)
            stations[i].Clear();
        
        trdStation.Clear();
        
        for (int i = 0; i < nofTrackBins; ++i)
            recoTracks[i].clear();
        
        triggerTimes_trd0_sign0_dist0.Clear();
        triggerTimes_trd0_sign0_dist1.Clear();
        triggerTimes_trd0_sign1_dist0.Clear();
        triggerTimes_trd0_sign1_dist1.Clear();
        triggerTimes_trd1_sign0_dist0.Clear();
        triggerTimes_trd1_sign0_dist1.Clear();
        triggerTimes_trd1_sign1_dist0.Clear();
        triggerTimes_trd1_sign1_dist1.Clear();
    }
    
    void SetTSBegin(unsigned long long tsLowBound)
    {
        minT = tsLowBound;
    }
    
    void Init()
    {
        fHasTrd = fHasTrd && fSignalParticle->fHasTrd;
        scaltype E = fSignalParticle->fMinEnergy;// GeV
        scaltype E0 = E;
        
        for (int i = 0; i < fNofStations; ++i)
        {
            stations[i].binSizeX = (stations[i].maxX - stations[i].minX) / stations[i].nofXBins;
            stations[i].binSizeY = (stations[i].maxY - stations[i].minY) / stations[i].nofYBins;
            scaltype L = stations[i].absorber.width;// / std::cos(3.14159265 * 15 / 180);
            E -= EnergyLoss(E, L, &stations[i].absorber);
            
            if (i > 0)
            {
                scaltype Escat = (E0 + E) / 2;
                scaltype deltaTheta = CalcThetaPrj(Escat, L, &stations[i].absorber);
                stations[i].deltaThetaX = deltaTheta;
                stations[i].deltaThetaY = deltaTheta;
                scaltype deltaZ = stations[i].z - stations[i - 1].z;
                stations[i].dispX = deltaTheta * deltaZ;
                stations[i].dispY = deltaTheta * deltaZ;
            }
            
            E0 = E;
        }
        
        if (fHasTrd)
        {
            trdStation.binSizeX = (trdStation.maxX - trdStation.minX) / trdStation.nofXBins;
            trdStation.binSizeY = (trdStation.maxY - trdStation.minY) / trdStation.nofYBins;
            scaltype L = trdStation.absorber.width;// / std::cos(3.14159265 * 15 / 180);
            E -= EnergyLoss(E, L, &trdStation.absorber);
            scaltype Escat = (E0 + E) / 2;
            scaltype deltaTheta = CalcThetaPrj(Escat, L, &trdStation.absorber);
            trdStation.deltaThetaX = deltaTheta;
            trdStation.deltaThetaY = deltaTheta;
            
            for (int i = 0; i < fNofTrdLayers; ++i)
            {
                scaltype deltaZ = trdStation.Zs[i] - stations[fLastStationNumber].z;
                trdStation.dispXs[i] = deltaTheta * deltaZ;
                trdStation.dispYs[i] = deltaTheta * deltaZ;
            }
            
            E0 = E;
        }
    }
    
    void FindNeighbours(scaltype x, scaltype wX, scaltype y, scaltype wY, scaltype t, scaltype wT, int layerIndex, list<LxTbBinnedPoint*>& results)
    {        
        if (x + wX < trdStation.minX || x - wX > trdStation.maxX || y + wY < trdStation.minY || y - wY > trdStation.maxY || t + wT < minT || t - wT > maxT)
            return;

        int lTindMin = (t - wT - minT) / fTimeBinLength;

        if (lTindMin < 0)
            lTindMin = 0;
        else if (lTindMin > fLastTimeBinNumber)
            lTindMin = fLastTimeBinNumber;

        int lTindMax = (t + wT - minT) / fTimeBinLength;

        if (lTindMax < 0)
            lTindMax = 0;
        else if (lTindMax > fLastTimeBinNumber)
            lTindMax = fLastTimeBinNumber;

        int lYindMin = (y - wY - trdStation.minY) / trdStation.binSizeY;

        if (lYindMin < 0)
            lYindMin = 0;
        else if (lYindMin > trdStation.lastYBin)
            lYindMin = trdStation.lastYBin;

        int lYindMax = (y + wY - trdStation.minY) / trdStation.binSizeY;

        if (lYindMax < 0)
            lYindMax = 0;
        else if (lYindMax > trdStation.lastYBin)
            lYindMax = trdStation.lastYBin;

        int lXindMin = (x - wX - trdStation.minX) / trdStation.binSizeX;

        if (lXindMin < 0)
            lXindMin = 0;
        else if (lXindMin > trdStation.lastXBin)
            lXindMin = trdStation.lastXBin;

        int lXindMax = (x + wX - trdStation.minX) / trdStation.binSizeX;

        if (lXindMax < 0)
            lXindMax = 0;
        else if (lXindMax > trdStation.lastXBin)
            lXindMax = trdStation.lastXBin;

        LxTbTYXBin* tyxBins = trdStation.tyxBinsArr[layerIndex];

        for (int lTind = lTindMin; lTind <= lTindMax; ++lTind)
        {
            LxTbTYXBin& lTyxBin = tyxBins[lTind];

            for (int lYind = lYindMin; lYind <= lYindMax; ++lYind)
            {
                LxTbYXBin& lYxBin = lTyxBin.yxBins[lYind];

                for (int lXind = lXindMin; lXind <= lXindMax; ++lXind)
                {
                    LxTbXBin& lXBin = lYxBin.xBins[lXind];

                    for (list<LxTbBinnedPoint>::iterator i = lXBin.points.begin(); i != lXBin.points.end(); ++i)
                    {
                        LxTbBinnedPoint& point = *i;

                        if (point.x > x - wX && point.x < x + wX && point.y > y - wY && point.y < y + wY && point.t > t - wT && point.t < t + wT)
                            results.push_back(&point);
                    }
                }
            }
        }
    }
    
    void Reconstruct()
    {
        //cout << "LxTbBinnedFinder::Reconstruct() started" << endl;
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        long beginTime = ts.tv_sec * 1000000000 + ts.tv_nsec;
        scaltype c = speedOfLight;
        maxT = minT + fTimeSliceLength;
        LxTbBinnedStation& lastStation = stations[fLastStationNumber];
        int nofXBins = lastStation.nofXBins;
        int nofYBins = lastStation.nofYBins;
        
        /*if (hasTrd)
        {
            scaltype lastZ = lastStation.z;
            scaltype deltaZsTrd[4] = {trdStation.Zs[0] - lastZ, trdStation.Zs[2] - lastZ, trdStation.Zs[2] - lastZ, trdStation.Zs[3] - lastZ};

            for (int i = 0; i < NOF_TIMEBINS; ++i)
            {

                LxTbTYXBin& lTyxBin = lastStation.tyxBins[i];

                for (int j = 0; j < nofYBins; ++j)
                {
                    LxTbYXBin& lYxBin = lTyxBin.yxBins[j];

                    for (int k = 0; k < nofXBins; ++k)
                    {
                        LxTbXBin& lXBin = lYxBin.xBins[k];

                        for (list<LxTbBinnedPoint>::iterator l = lXBin.points.begin(); l != lXBin.points.end(); ++l)
                        {
                            LxTbBinnedPoint& lPoint = *l;
                            scaltype tx = lPoint.x / lastZ;
                            scaltype ty = lPoint.y / lastZ;
                            scaltype trdPx0 = lPoint.x + tx * deltaZsTrd[0];
                            scaltype trdPy0 = lPoint.y + ty * deltaZsTrd[0];
                            scaltype trajLen0 = std::sqrt(1 + tx * tx + ty * ty) * deltaZsTrd[0];
                            scaltype trdPt0 = lPoint.t + 1.e9 * trajLen0 / c; // 1.e9 to convert to ns.
                            scaltype wX0 = NOF_SIGMAS * trdStation.dispXs[0];
                            scaltype wY0 = NOF_SIGMAS * trdStation.dispYs[0];
                            scaltype wT0 = NOF_SIGMAS * std::sqrt(2.0) * lPoint.dt;
                            list<LxTbBinnedPoint*> results0;
                            FindNeighbours(trdPx0, wX0, trdPy0, wY0, trdPt0, wT0, 0, results0);

                            scaltype trdPx1 = lPoint.x + tx * deltaZsTrd[1];
                            scaltype trdPy1 = lPoint.y + ty * deltaZsTrd[1];
                            scaltype trajLen1 = std::sqrt(1 + tx * tx + ty * ty) * deltaZsTrd[1];
                            scaltype trdPt1 = lPoint.t + 1.e9 * trajLen1 / c; // 1.e9 to convert to ns.
                            scaltype wX1 = NOF_SIGMAS * trdStation.dispXs[1];
                            scaltype wY1 = NOF_SIGMAS * trdStation.dispYs[1];
                            scaltype wT1 = NOF_SIGMAS * std::sqrt(2.0) * lPoint.dt;
                            list<LxTbBinnedPoint*> results1;
                            FindNeighbours(trdPx1, wX1, trdPy1, wY1, trdPt1, wT1, 1, results1);

                            for (list<LxTbBinnedPoint*>::const_iterator m = results0.begin(); m != results0.end(); ++m)
                            {
                                LxTbBinnedPoint& trdPoint0 = *(*m);

                                for (list<LxTbBinnedPoint*>::const_iterator n = results1.begin(); n != results1.end(); ++n)
                                {
                                    LxTbBinnedPoint& trdPoint1 = *(*n);
                                    scaltype trdTx = (trdPoint1.x - trdPoint0.x) / (trdStation.Zs[1] - trdStation.Zs[0]);
                                    scaltype trdTy = (trdPoint1.y - trdPoint0.y) / (trdStation.Zs[1] - trdStation.Zs[0]);

                                    scaltype trdPx2 = trdPoint1.x + trdTx * (trdStation.Zs[2] - trdStation.Zs[1]);
                                    scaltype trdPy2 = trdPoint1.y + trdTy * (trdStation.Zs[2] - trdStation.Zs[1]);
                                    scaltype trajLen2 = std::sqrt(1 + trdTx * trdTx + trdTx * trdTx) * (trdStation.Zs[2] - trdStation.Zs[1]);
                                    scaltype trdPt2 = trdPoint1.t + 1.e9 * trajLen2 / c; // 1.e9 to convert to ns.
                                    scaltype wX2 = 0.35135;
                                    scaltype wY2 = 0.33515;
                                    scaltype wT2 = NOF_SIGMAS * std::sqrt(2.0) * trdPoint1.dt;
                                    list<LxTbBinnedPoint*> results2;
                                    FindNeighbours(trdPx2, wX2, trdPy2, wY2, trdPt2, wT2, 2, results2);

                                    if (results2.empty())
                                        continue;

                                    scaltype trdPx3 = trdPoint1.x + trdTx * (trdStation.Zs[3] - trdStation.Zs[1]);
                                    scaltype trdPy3 = trdPoint1.y + trdTy * (trdStation.Zs[3] - trdStation.Zs[1]);
                                    scaltype trajLen3 = std::sqrt(1 + trdTx * trdTx + trdTx * trdTx) * (trdStation.Zs[3] - trdStation.Zs[1]);
                                    scaltype trdPt3 = trdPoint1.t + 1.e9 * trajLen3 / c; // 1.e9 to convert to ns.
                                    scaltype wX3 = 0.909;
                                    scaltype wY3 = 0.8455;
                                    scaltype wT3 = NOF_SIGMAS * std::sqrt(2.0) * trdPoint1.dt;
                                    list<LxTbBinnedPoint*> results3;
                                    FindNeighbours(trdPx3, wX3, trdPy3, wY3, trdPt3, wT3, 3, results3);

                                    if (results3.empty())
                                        continue;

                                    lPoint.use = true;
                                    lXBin.use = true;
                                    lYxBin.use = true;
                                    lTyxBin.use = true;
                                }
                            }
                        }
                    }
                }
            }
        }*/
        
        for (int i = fLastStationNumber; i >=1; --i)
        {
            LxTbBinnedStation& rStation = stations[i];
            LxTbBinnedStation& lStation = stations[i - 1];
            scaltype binSizeX = lStation.binSizeX;
            scaltype binSizeY = lStation.binSizeY;
            scaltype rZ = rStation.z;
            scaltype lZ = lStation.z;
            scaltype deltaZ = lZ - rZ;
            scaltype dispXSq = rStation.dispX * rStation.dispX;
            scaltype dispYSq = rStation.dispY * rStation.dispY;
            scaltype minX = lStation.minX;
            scaltype maxX = lStation.maxX;
            scaltype minY = lStation.minY;
            scaltype maxY = lStation.maxY;
            int rNofXBins = rStation.nofXBins;
            int rNofYBins = rStation.nofYBins;
            int lastXBin = lStation.nofXBins - 1;
            int lastYBin = lStation.nofYBins - 1;
            
            for (int j = 0; j < fNofTimeBins; ++j)
            {
                LxTbTYXBin& rTyxBin = rStation.tyxBins[j];
                
                if (!rTyxBin.use)
                    continue;
                
                for (int k = 0; k < rNofYBins; ++k)
                {
                    LxTbYXBin& rYxBin = rTyxBin.yxBins[k];
                    
                    if (!rYxBin.use)
                        continue;
                    
                    for (int l = 0; l < rNofXBins; ++l)
                    {
                        LxTbXBin& rXBin = rYxBin.xBins[l];
                        
                        if (!rXBin.use)
                            continue;
                        
                        for (list<LxTbBinnedPoint>::iterator m = rXBin.points.begin(); m != rXBin.points.end(); ++m)
                        {
                            LxTbBinnedPoint& rPoint = *m;
                            
                            if (!rPoint.use)
                                continue;
                            
                            scaltype tx = rPoint.x / rZ;
                            scaltype ty = rPoint.y / rZ;
                            scaltype pX = rPoint.x + tx * deltaZ;
                            scaltype pY = rPoint.y + ty * deltaZ;
                            scaltype trajLen = std::sqrt(1 + tx * tx + ty * ty) * deltaZ;
                            scaltype pT = rPoint.t + 1.e9 * trajLen / c;// 1.e9 to convert to ns and trajLen is negative.
                            scaltype rDxSq = rPoint.dx * rPoint.dx;
                            scaltype xDiv0 = dispXSq + rDxSq;
                            scaltype wX = NOF_SIGMAS * std::sqrt(xDiv0 + rDxSq);
                            scaltype rDySq = rPoint.dy * rPoint.dy;
                            scaltype yDiv0 = dispYSq + rDySq;
                            scaltype wY = NOF_SIGMAS * std::sqrt(yDiv0 + rDySq);
                            scaltype wT = NOF_SIGMAS * std::sqrt(2.0) * rPoint.dt;
                            
                            if (pX + wX < minX || pX - wX > maxX || pY + wY < minY || pY - wY > maxY || pT + wT < minT || pT - wT > maxT)
                                continue;
                            
                            int lTindMin = (pT - wT - minT) / fTimeBinLength;
                            
                            if (lTindMin < 0)
                                lTindMin = 0;
                            else if (lTindMin > fLastTimeBinNumber)
                                lTindMin = fLastTimeBinNumber;
                            
                            int lTindMax = (pT + wT - minT) / fTimeBinLength;
                            
                            if (lTindMax < 0)
                                lTindMax = 0;
                            else if (lTindMax > fLastTimeBinNumber)
                                lTindMax = fLastTimeBinNumber;
                            
                            int lYindMin = (pY - wY - minY) / binSizeY;
                            
                            if (lYindMin < 0)
                                lYindMin = 0;
                            else if (lYindMin > lastYBin)
                                lYindMin = lastYBin;
                            
                            int lYindMax = (pY + wY - minY) / binSizeY;
                            
                            if (lYindMax < 0)
                                lYindMax = 0;
                            else if (lYindMax > lastYBin)
                                lYindMax = lastYBin;
                            
                            int lXindMin = (pX - wX - minX) / binSizeX;
                            
                            if (lXindMin < 0)
                                lXindMin = 0;
                            else if (lXindMin > lastXBin)
                                lXindMin = lastXBin;
                            
                            int lXindMax = (pX + wX - minX) / binSizeX;
                            
                            if (lXindMax < 0)
                                lXindMax = 0;
                            else if (lXindMax > lastXBin)
                                lXindMax = lastXBin;
                            
                            for (int lTind = lTindMin; lTind <= lTindMax; ++lTind)
                            {
                                LxTbTYXBin& lTyxBin = lStation.tyxBins[lTind];
                                
                                for (int lYind = lYindMin; lYind <= lYindMax; ++lYind)
                                {
                                    LxTbYXBin& lYxBin = lTyxBin.yxBins[lYind];
                                    
                                    for (int lXind = lXindMin; lXind <= lXindMax; ++lXind)
                                    {
                                        LxTbXBin& lXBin = lYxBin.xBins[lXind];
                                        
                                        for (list<LxTbBinnedPoint>::iterator n = lXBin.points.begin(); n != lXBin.points.end(); ++n)
                                        {
                                            LxTbBinnedPoint& lPoint = *n;
                                            scaltype xDiv = xDiv0 + lPoint.dx * lPoint.dx;
                                            scaltype wX_prec_sq = NOF_SIGMAS_SQ * xDiv;
                                            scaltype yDiv = yDiv0 + lPoint.dy * lPoint.dy;
                                            scaltype wY_prec_sq = NOF_SIGMAS_SQ * yDiv;
                                            scaltype deltaX = lPoint.x - pX;
                                            scaltype deltaXSq = deltaX * deltaX;
                                            scaltype deltaY = lPoint.y - pY;
                                            scaltype deltaYSq = deltaY * deltaY;
                                            scaltype deltaT = lPoint.t - pT;
                                            scaltype deltaTSq = deltaT * deltaT;
                                            
                                            if (deltaXSq < wX_prec_sq && deltaYSq < wY_prec_sq &&
                                                    deltaTSq < wT * wT)
                                            {
                                                lPoint.use = true;
                                                lXBin.use = true;
                                                rPoint.neighbours.push_back(LxTbBinnedRay(deltaZ, rPoint, lPoint, deltaXSq / xDiv +
                                                    deltaYSq / yDiv + deltaTSq / (rPoint.dt * rPoint.dt + lPoint.dt * lPoint.dt)));
                                            }
                                        }
                                        
                                        if (lXBin.use)
                                            lYxBin.use = true;
                                    }
                                    
                                    if (lYxBin.use)
                                        lTyxBin.use = true;
                                }
                            }
                        }// for (list<LxTbBinnedPoint>::iterator m = rStation
                    }// for (int l = 0; l < NOF_XBINS; ++l)
                }// for (int k = 0; k < NOF_YBINS; ++k)
            }// for (int j = 0; j < NOF_TIMEBINS; ++j)
        }// for (int i = fLastStationNumber; i >=1; --i)
        
        const LxTbBinnedPoint* points[fNofStations];
        
        for (int i = 0; i < fNofTimeBins; ++i)
        {
            LxTbTYXBin& rTyxBin = lastStation.tyxBins[i];

            if (!rTyxBin.use)
                continue;

            for (int j = 0; j < nofYBins; ++j)
            {
                LxTbYXBin& rYxBin = rTyxBin.yxBins[j];

                if (!rYxBin.use)
                    continue;

                for (int k = 0; k < nofXBins; ++k)
                {
                    LxTbXBin& rXBin = rYxBin.xBins[k];

                    if (!rXBin.use)
                        continue;

                    for (list<LxTbBinnedPoint>::iterator l = rXBin.points.begin(); l != rXBin.points.end(); ++l)
                    {
                        LxTbBinnedPoint& rPoint = *l;                        
                        scaltype chi2 = 0;
                        list<ChainImpl> chains;
                        FindChains(fLastStationNumber, &rPoint, 0, points, chi2, chains);
                        const ChainImpl* bestChain = 0;
                        
                        for (list<ChainImpl>::const_iterator m = chains.begin(); m != chains.end(); ++m)
                        {
                            const ChainImpl& chain = *m;
                            
                            if (0 == bestChain || chain.chi2 < chi2)
                            {
                                bestChain = &chain;
                                chi2 = chain.chi2;
                            }
                        }
                        
                        if (0 != bestChain)
                        {
                            int trackBinInd = (bestChain->points[fLastStationNumber]->t - minT) / fTimeBinLength;
                            
                            if (trackBinInd < 0)
                                trackBinInd = 0;
                            else if (trackBinInd > nofTrackBins)
                                trackBinInd = nofTrackBins;
                            
                            recoTracks[trackBinInd].push_back(new Chain(bestChain->points, fNofStations, chi2));
                        }
                        
                        for (list<ChainImpl>::iterator m = chains.begin(); m != chains.end(); ++m)
                        {
                            ChainImpl& chain = *m;
                            delete[] chain.points;
                        }
                    }
                }
            }
        }
        
        if (fHasTrd)
        {
            scaltype lastZ = lastStation.z;
            scaltype deltaZsTrd[4] = {trdStation.Zs[0] - lastZ, trdStation.Zs[2] - lastZ, trdStation.Zs[2] - lastZ, trdStation.Zs[3] - lastZ};
            
            for (int i = 0; i < nofTrackBins; ++i)
            {
                list<Chain*>& recoTracksBin = recoTracks[i];
                
                for (list<Chain*>::iterator j = recoTracksBin.begin(); j != recoTracksBin.end(); ++j)
                {
                    Chain* track = *j;
                    LxTbBinnedPoint& lPoint = *track->points[track->nofPoints - 1];
                    scaltype tx = lPoint.x / lastZ;
                    scaltype ty = lPoint.y / lastZ;
                    scaltype trdPx0 = lPoint.x + tx * deltaZsTrd[0];
                    scaltype trdPy0 = lPoint.y + ty * deltaZsTrd[0];
                    scaltype trajLen0 = std::sqrt(1 + tx * tx + ty * ty) * deltaZsTrd[0];
                    scaltype trdPt0 = lPoint.t + 1.e9 * trajLen0 / c; // 1.e9 to convert to ns.
                    scaltype wX0 = NOF_SIGMAS * trdStation.dispXs[0];
                    scaltype wY0 = NOF_SIGMAS * trdStation.dispYs[0];
                    scaltype wT0 = NOF_SIGMAS * std::sqrt(2.0) * lPoint.dt;
                    list<LxTbBinnedPoint*> results0;
                    FindNeighbours(trdPx0, wX0, trdPy0, wY0, trdPt0, wT0, 0, results0);

                    scaltype trdPx1 = lPoint.x + tx * deltaZsTrd[1];
                    scaltype trdPy1 = lPoint.y + ty * deltaZsTrd[1];
                    scaltype trajLen1 = std::sqrt(1 + tx * tx + ty * ty) * deltaZsTrd[1];
                    scaltype trdPt1 = lPoint.t + 1.e9 * trajLen1 / c; // 1.e9 to convert to ns.
                    scaltype wX1 = NOF_SIGMAS * trdStation.dispXs[1];
                    scaltype wY1 = NOF_SIGMAS * trdStation.dispYs[1];
                    scaltype wT1 = NOF_SIGMAS * std::sqrt(2.0) * lPoint.dt;
                    list<LxTbBinnedPoint*> results1;
                    FindNeighbours(trdPx1, wX1, trdPy1, wY1, trdPt1, wT1, 1, results1);

                    for (list<LxTbBinnedPoint*>::const_iterator k = results0.begin(); k != results0.end(); ++k)
                    {
                        LxTbBinnedPoint& trdPoint0 = *(*k);

                        for (list<LxTbBinnedPoint*>::const_iterator l = results1.begin(); l != results1.end(); ++l)
                        {
                            LxTbBinnedPoint& trdPoint1 = *(*l);
                            scaltype trdTx = (trdPoint1.x - trdPoint0.x) / (trdStation.Zs[1] - trdStation.Zs[0]);
                            scaltype trdTy = (trdPoint1.y - trdPoint0.y) / (trdStation.Zs[1] - trdStation.Zs[0]);

                            scaltype trdPx2 = trdPoint1.x + trdTx * (trdStation.Zs[2] - trdStation.Zs[1]);
                            scaltype trdPy2 = trdPoint1.y + trdTy * (trdStation.Zs[2] - trdStation.Zs[1]);
                            scaltype trajLen2 = std::sqrt(1 + trdTx * trdTx + trdTx * trdTx) * (trdStation.Zs[2] - trdStation.Zs[1]);
                            scaltype trdPt2 = trdPoint1.t + 1.e9 * trajLen2 / c; // 1.e9 to convert to ns.
                            scaltype wX2 = 0.35135;
                            scaltype wY2 = 0.33515;
                            scaltype wT2 = NOF_SIGMAS * std::sqrt(2.0) * trdPoint1.dt;
                            list<LxTbBinnedPoint*> results2;
                            FindNeighbours(trdPx2, wX2, trdPy2, wY2, trdPt2, wT2, 2, results2);

                            if (results2.empty())
                                continue;

                            scaltype trdPx3 = trdPoint1.x + trdTx * (trdStation.Zs[3] - trdStation.Zs[1]);
                            scaltype trdPy3 = trdPoint1.y + trdTy * (trdStation.Zs[3] - trdStation.Zs[1]);
                            scaltype trajLen3 = std::sqrt(1 + trdTx * trdTx + trdTx * trdTx) * (trdStation.Zs[3] - trdStation.Zs[1]);
                            scaltype trdPt3 = trdPoint1.t + 1.e9 * trajLen3 / c; // 1.e9 to convert to ns.
                            scaltype wX3 = 0.909;
                            scaltype wY3 = 0.8455;
                            scaltype wT3 = NOF_SIGMAS * std::sqrt(2.0) * trdPoint1.dt;
                            list<LxTbBinnedPoint*> results3;
                            FindNeighbours(trdPx3, wX3, trdPy3, wY3, trdPt3, wT3, 3, results3);

                            if (!results3.empty())
                            {
                                track->highMom = true;
                                break;
                            }
                        }

                        if (track->highMom)
                            break;
                    }
                }
            }
        }
        
        Trigger();
        
        clock_gettime(CLOCK_REALTIME, &ts);
        long endTime = ts.tv_sec * 1000000000 + ts.tv_nsec;
        fullDuration += endTime - beginTime;
        //cout << "LxTbNaiveFinder::Reconstruct() full duration was: " << fullDuration << endl;
        //cout << "LxTbNaiveFinder::Reconstruct() the number of found tracks: " << recoTracks.size() << endl;
    }
    
    void FindChains(int stationIndex, const LxTbBinnedPoint* rPoint, const LxTbBinnedRay* rRay,
        const LxTbBinnedPoint** points, scaltype chi2, list<ChainImpl>& chains)
    {
        points[stationIndex] = rPoint;
        
        if (0 == stationIndex)
        {
            ChainImpl chain(points, fNofStations, chi2);
            chains.push_back(chain);
            return;
        }
        
        for (list<LxTbBinnedRay>::const_iterator i = rPoint->neighbours.begin(); i != rPoint->neighbours.end(); ++i)
        {
            const LxTbBinnedRay& lRay = *i;
            const LxTbBinnedPoint* lPoint = lRay.lPoint;
            scaltype chi2_2 = chi2 + lRay.chi2;
            
            if (0 != rRay)
                //chi2_2 += (lRay.tx - rRay->tx) * (lRay.tx - rRay->tx) / (rRay->dtxSq + lRay.dtxSq) +
                            //(lRay.ty - rRay->ty) * (lRay.ty - rRay->ty) / (rRay->dtySq + lRay.dtySq);
                chi2_2 += (lRay.tx - rRay->tx) * (lRay.tx - rRay->tx) / (stations[stationIndex].deltaThetaX * stations[stationIndex].deltaThetaX) +
                        (lRay.ty - rRay->ty) * (lRay.ty - rRay->ty) / (stations[stationIndex].deltaThetaY * stations[stationIndex].deltaThetaY);
            
            FindChains(stationIndex - 1, lPoint, &lRay, points, chi2_2, chains);
        }
    }
    
    void TriggerBin(list<Chain*>& recoTracksBin, list<Chain*>& borderTracks, int i, bool handleBorder)
    {
        list<Chain*>::const_iterator it = handleBorder ? borderTracks.begin() : recoTracksBin.begin();
        list<Chain*>::const_iterator endIt = handleBorder ? borderTracks.end() : recoTracksBin.end();
        
        for (; it != endIt; ++it)
        {
            Chain* track = *it;
            LxTbBinnedPoint* point = track->points[fLastStationNumber];
            scaltype dt = point->dt;
            scaltype dtSq = dt * dt;
            scaltype wt = NOF_SIGMAS * sqrt(2.0) * dt;

            if (!handleBorder && point->t + wt > minT + (i + 1) * fTimeBinLength)
                borderTracks.push_back(track);

            scaltype trackSign = (track->points[1]->x - track->points[0]->x) / (stations[1].z - stations[0].z) - track->points[0]->x / stations[0].z;
            list<Chain*>::const_iterator it2 = handleBorder ? recoTracksBin.begin() : it;
            list<Chain*>::const_iterator endIt2 = recoTracksBin.end();
            
            if (!handleBorder)
                ++it2;

            for (; it2 != endIt2; ++it2)
            {
                Chain* track2 = *it2;
                LxTbBinnedPoint* point2 = track2->points[fLastStationNumber];
                scaltype dt2 = point2->dt;
                scaltype dt2Sq = dt2 * dt2;

                if (fabs(point2->t - point->t) > NOF_SIGMAS * sqrt(dtSq + dt2Sq))
                    continue;

                scaltype track2Sign = (track2->points[1]->x - track2->points[0]->x) / (stations[1].z - stations[0].z) - track2->points[0]->x / stations[0].z;
                scaltype dist = sqrt((track2->points[0]->x - track->points[0]->x) * (track2->points[0]->x - track->points[0]->x) +
                        (track2->points[0]->y - track->points[0]->y) * (track2->points[0]->y - track->points[0]->y));
                pair<scaltype, scaltype> pairTime((point->t + point2->t) / 2,
                    sqrt(dtSq + dt2Sq) / 2);

                if (track->highMom && track2->highMom)
                {
                    if (trackSign * track2Sign < 0)
                    {
                        if (dist >= 50.0)
                            triggerTimes_trd1_sign1_dist1.Insert(pairTime);
                        
                        triggerTimes_trd1_sign1_dist0.Insert(pairTime);
                    }

                    if (dist >= 50.0)
                        triggerTimes_trd1_sign0_dist1.Insert(pairTime);
                        
                    triggerTimes_trd1_sign0_dist0.Insert(pairTime);
                }


                if (trackSign * track2Sign < 0)
                {
                    if (dist >= 50.0)
                        triggerTimes_trd0_sign1_dist1.Insert(pairTime);

                    triggerTimes_trd0_sign1_dist0.Insert(pairTime);
                }
                
                if (dist >= 50.0)
                    triggerTimes_trd0_sign0_dist1.Insert(pairTime);
                
                triggerTimes_trd0_sign0_dist0.Insert(pairTime);
            }
        }
    }
    
    void Trigger()
    {
        list<Chain*> borderTracks;
        
        for (int i = 0; i < nofTrackBins; ++i)
        {            
            TriggerBin(recoTracks[i], borderTracks, 0, true);   
            borderTracks.clear();            
            TriggerBin(recoTracks[i], borderTracks, i, false);
        }
    }
};

#endif /* LXTBBINNED_H */
