/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Station4D.h
 * Author: tablyaz
 *
 * Created on August 7, 2017, 2:08 PM
 */

#ifndef STATION4D_H
#define STATION4D_H

#include "Station.h"

class CbmBinned4DStation : public CbmBinnedStation
{
public:
    CbmBinned4DStation(int nofZBins, int nofYBins, int nofXBins, int nofTBins) : fZBins(reinterpret_cast<CbmZBin*> (new unsigned char[nofZBins * sizeof(CbmZBin)])),
            fNofZBins(nofZBins), fNofYBins(nofYBins), fNofXBins(nofXBins), fNofTBins(nofTBins), fZBinSize(0), fYBinSize(0), fXBinSize(0), fTBinSize(0),
            fMinZ(0), fMaxZ(0), fMinY(0), fMaxY(0), fMinX(0), fMaxX(0), fMinT(0), fMaxT(0), fDefaultUse(false), fDtxSq(0), fDtySq(0)
    {
        for (int i = 0; i < nofZBins; ++i)
            new(&fZBins[i]) CbmZBin(nofYBins, nofXBins, nofTBins);
    }
    
    void AddHit(const CbmPixelHit* hit, Int_t index)
    {
        Double_t z = hit->GetZ();
        
        if (z < fMinZ || z >= fMaxZ)
            return;
        
        Double_t y = hit->GetY();
        
        if (y < fMinY || y >= fMaxY)
            return;
        
        Double_t x = hit->GetX();
        
        if (x < fMinX || x >= fMaxX)
            return;
        
        Double_t t = hit->GetTime();
        
        if (t < fMinT || t >= fMaxT)
            return;
        
        int zInd = (z - fMinZ) / fZBinSize;
        int yInd = (y - fMinY) / fYBinSize;
        int xInd = (x - fMinX) / fXBinSize;
        int tInd = (t - fMinT) / fTBinSize;
        
        CbmZBin& zBin = fZBins[zInd];
        CbmYBin& yBin = zBin[yInd];
        CbmXBin& xBin = yBin[xInd];
        CbmTBin& tBin = xBin[tInd];
        tBin.AddHit(hit, index, fDefaultUse);
        
        if (fDefaultUse)
        {
            zBin.SetUse();
            yBin.SetUse();
            xBin.SetUse();
            tBin.SetUse();
        }
    }
    
    void IterateHits(std::function<void(CbmTBin::HitHolder&)> handleHit)
    {
        for (int i = 0; i < fNofZBins; ++i)
        {
            CbmZBin& zBin = fZBins[i];
            
            if (!zBin.Use())
                continue;
            
            for (int j = 0; j < fNofYBins; ++j)
            {
                CbmYBin& yBin = zBin[j];
            
                if (!yBin.Use())
                    continue;
                
                for (int k = 0; k < fNofXBins; ++k)
                {
                    CbmXBin& xBin = yBin[k];
            
                    if (!xBin.Use())
                        continue;
                    
                    for (int l = 0; l < fNofTBins; ++l)
                    {
                        CbmTBin& tBin = xBin[l];
                        
                        if (!tBin.Use())
                            continue;
                        
                        for (std::list<CbmTBin::HitHolder>::iterator hi = tBin.HitsBegin(); hi != tBin.HitsEnd(); ++hi)
                        {
                            if (hi->use)
                                handleHit(*hi);
                        }
                    }
                }
            }
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
    
    int GetZInd(Double_t v) const
    {
        int ind = (v - fMinZ) / fZBinSize;
        
        if (ind < 0)
            ind = 0;
        else if (ind >= fNofZBins)
            ind = fNofZBins - 1;
        
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
    
    Double_t GetYEnterZ(int yInd, Double_t ty) const
    {
        if (ty > 0)
            return (fMinY + yInd * fYBinSize) / ty;
        else
            return (fMinY + (yInd + 1) * fYBinSize) / ty;
    }
    
    Double_t GetYExitZ(int yInd, Double_t ty) const
    {
        if (ty > 0)
            return (fMinY + (yInd + 1) * fYBinSize) / ty;
        else
            return (fMinY + yInd * fYBinSize) / ty;
    }
    
    Double_t GetXEnterZ(int xInd, Double_t tx) const
    {
        if (tx > 0)
            return (fMinX + xInd * fXBinSize) / tx;
        else
            return (fMinX + (xInd + 1) * fXBinSize) / tx;
    }
    
    Double_t GetXExitZ(int xInd, Double_t tx) const
    {
        if (tx > 0)
            return (fMinX + (xInd + 1) * fXBinSize) / tx;
        else
            return (fMinX + xInd * fXBinSize) / tx;
    }
    
    void SearchHits(const CbmPixelHit* searchHit, std::function<void(CbmTBin::HitHolder&)> handleHit)
    {
        Double_t searchX = searchHit->GetX();
        Double_t searchY = searchHit->GetY();
        Double_t searchZ = searchHit->GetZ();
        Double_t searchT = searchHit->GetTime();
        
        Double_t searchDx = searchHit->GetDx();
        Double_t searchDy = searchHit->GetDy();
        Double_t dt = searchHit->GetTimeError();
        Double_t dtSq = dt * dt;
        Double_t wT = cbmBinnedSigma * std::sqrt(dtSq + fDt * fDt);
        
        Double_t tx = searchX / searchZ;
        Double_t ty = searchY / searchZ;
        Double_t timeCoeff = std::sqrt(1 + tx * tx + ty * ty) / cbmBinnedSOL;
        Double_t dTx = searchDx / searchZ;
        Double_t dTxSq = dTx * dTx;
        Double_t wTx = cbmBinnedSigma * std::sqrt(dTxSq + fDtxSq);
        Double_t minTx = tx - wTx;
        Double_t maxTx = tx + wTx;
        Double_t dTy = searchDy / searchZ;
        Double_t dTySq = dTy * dTy;
        Double_t wTy = cbmBinnedSigma * std::sqrt(dTySq + fDtySq);
        Double_t minTy = ty - wTy;
        Double_t maxTy = ty + wTy;
        
        if (minTy > 0 && minTy * fMinZ >= fMaxY)
            return;
        else if (maxTy < 0 && maxTy * fMinZ < fMinY)
            return;
        
        if (minTx > 0 && minTx * fMinZ >= fMaxX)
            return;
        else if (maxTx < 0 && maxTx * fMinZ < fMinX)
            return;
        
        Double_t minZ = fMinZ;
        Double_t maxZ = fMaxZ;
        
        if (minTy > 0)
        {
            if (fMaxY / minTy < maxZ)
                maxZ = fMaxY / minTy;
        }
        else if (maxTy < 0)
        {
            if (fMinY / maxTy < maxZ)
                maxZ = fMinY / maxTy;
        }
        
        if (minTx > 0)
        {
            if (fMaxX / minTx < maxZ)
                maxZ = fMaxX / minTx;
        }
        else if (maxTx < 0)
        {
            if (fMinX / maxTx < maxZ)
                maxZ = fMinX / maxTx;
        }
        
        int maxZind = GetZInd(maxZ);
        
        for (int i = 0; i <= maxZind; ++i)
        {
            CbmZBin& zBin = fZBins[i];
            Double_t minZi = minZ + i * fZBinSize;
            Double_t maxZi = minZ + (i + 1) * fZBinSize;
            Double_t minY = minTy > 0 ? minTy * minZi : minTy * maxZi;
            int minYind = GetYInd(minY);
            Double_t maxY = maxTy > 0 ? maxTy * maxZi : maxTy * minZi;
            int maxYind = GetYInd(maxY);
            
            for (int j = minYind; j <= maxYind; ++j)
            {
                CbmYBin& yBin = zBin[j];
                Double_t minZj = maxZi;
                Double_t maxZj = minZi;
                Double_t tmp = GetYEnterZ(j, minTy);
                
                if (tmp < minZj && tmp > minZi)
                    minZj = tmp;
                
                tmp = GetYEnterZ(j, maxTy);
                
                if (tmp < minZj && tmp > minZi)
                    minZj = tmp;
                
                if (minZj == maxZi)
                    minZj = minZi;
                
                tmp = GetYExitZ(j, minTy);
                
                if (tmp > maxZj && tmp < maxZi)
                    maxZj = tmp;
                
                tmp = GetYExitZ(j, maxTy);
                
                if (tmp > maxZj && tmp < maxZi)
                    maxZj = tmp;
                
                if (maxZj == minZi)
                    maxZj = maxZi;
                
                Double_t minX = minTx > 0 ? minTx * minZj : minTx * maxZj;
                Double_t maxX = maxTx > 0 ? maxTx * maxZj : maxTx * minZj;
                int minXind = GetXInd(minX);
                int maxXind = GetXInd(maxX);
                
                for (int k = minXind; k <= maxXind; ++k)
                {
                    CbmXBin& xBin = yBin[k];
                    Double_t minZk = maxZj;
                    Double_t maxZk = minZj;
                    Double_t tmp2 = GetXEnterZ(k, minTx);

                    if (tmp2 < minZk && tmp2 > minZj)
                        minZk = tmp2;

                    tmp2 = GetXEnterZ(k, maxTx);

                    if (tmp2 < minZk && tmp2 > minZj)
                        minZk = tmp2;

                    if (minZk == maxZj)
                        minZk = minZj;

                    tmp2 = GetXExitZ(k, minTx);

                    if (tmp2 > maxZk && tmp2 < maxZj)
                        maxZk = tmp2;

                    tmp2 = GetXExitZ(k, maxTx);

                    if (tmp2 > maxZk && tmp2 < maxZj)
                        maxZk = tmp2;

                    if (maxZk == minZj)
                        maxZk = maxZj;

                    Double_t minT = searchT + (minZk - searchZ) * timeCoeff - wT;
                    Double_t maxT = searchT + (maxZk - searchZ) * timeCoeff + wT;
                    int minTind = GetTInd(minT);
                    int maxTind = GetTInd(maxT);
                    
                    for (int l = minTind; l <= maxTind; ++l)
                    {
                        CbmTBin& tBin = xBin[l];
                        std::list<CbmTBin::HitHolder>::iterator hitIter = tBin.HitsBegin();
                        std::list<CbmTBin::HitHolder>::iterator hitIterEnd = tBin.HitsEnd();

                        for (; hitIter != hitIterEnd; ++hitIter)
                        {
                            const CbmPixelHit* hit = hitIter->hit;
                            Double_t z = hit->GetZ();
                            Double_t zSq = z * z;
                            Double_t deltaY = hit->GetY() - ty * z;

                            if (deltaY * deltaY > cbmBinnedSigmaSq * (dTySq * zSq + hit->GetDy() * hit->GetDy()))
                                continue;

                            Double_t deltaX = hit->GetX() - tx * z;

                            if (deltaX * deltaX > cbmBinnedSigmaSq * (dTxSq * zSq + hit->GetDx() * hit->GetDx()))
                                continue;

                            Double_t deltaT = hit->GetTime() - searchT - (z - searchZ) * timeCoeff;

                            if (deltaT * deltaT > cbmBinnedSigmaSq * (dtSq + hit->GetTimeError() * hit->GetTimeError()))
                                continue;

                            handleHit(*hitIter);
                        }
                    }
                }
            }
        }
    }
    
private:
    CbmBinned4DStation(const CbmBinned4DStation&) = delete;
    CbmBinned4DStation& operator=(const CbmBinned4DStation&) = delete;
    
private:
    CbmZBin* fZBins;
    int fNofZBins;
    int fNofYBins;
    int fNofXBins;
    int fNofTBins;
    Double_t fZBinSize;
    Double_t fYBinSize;
    Double_t fXBinSize;
    Double_t fTBinSize;
    Double_t fMinZ;
    Double_t fMaxZ;
    Double_t fMinY;
    Double_t fMaxY;
    Double_t fMinX;
    Double_t fMaxX;
    Double_t fMinT;
    Double_t fMaxT;
    bool fDefaultUse;
    Double_t fDtxSq;
    Double_t fDtySq;
};

#endif /* STATION4D_H */
