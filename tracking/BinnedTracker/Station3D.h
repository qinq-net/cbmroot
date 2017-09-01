/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Station3D.h
 * Author: tablyaz
 *
 * Created on August 7, 2017, 2:08 PM
 */

#ifndef STATION3D_H
#define STATION3D_H

#include "Station.h"
#include <list>
#include <utility>

class CbmBinned3DStation : public CbmBinnedStation
{
public:
    CbmBinned3DStation(int nofYBins, int nofXBins, int nofTBins) : CbmBinnedStation(nofYBins, nofXBins, nofTBins),
            fYBins(reinterpret_cast<CbmYBin*> (new unsigned char[nofYBins * sizeof(CbmYBin)]))
    {
        for (int i = 0; i < nofYBins; ++i)
            new(&fYBins[i]) CbmYBin(0, nofXBins, nofTBins);
    }
    
    void Clear()
    {
        for (int i = 0; i < fNofYBins; ++i)
        {
            CbmYBin& yBin = fYBins[i];
            yBin.SetUse(false);
            
            for (int j = 0; j < fNofXBins; ++j)
            {
                CbmXBin& xBin = yBin[j];
                xBin.SetUse(false);
                
                for (int k = 0; k < fNofTBins; ++k)
                {
                    CbmTBin& tBin = xBin[k];
                    tBin.SetUse(false);
                    tBin.Clear();
                }
            }
        }
    }
    
    void AddHit(const CbmPixelHit* hit, Int_t index)
    {
        Double_t y = hit->GetY();
        
        if (y < fMinY || y >= fMaxY)
            return;
        
        Double_t x = hit->GetX();
        
        if (x < fMinX || x >= fMaxX)
            return;
        
        Double_t t = hit->GetTime();
        
        if (t < fMinT || t >= fMaxT)
            return;
        
        int yInd = (y - fMinY) / fYBinSize;
        int xInd = (x - fMinX) / fXBinSize;
        int tInd = (t - fMinT) / fTBinSize;
        
        CbmYBin& yBin = fYBins[yInd];
        CbmXBin& xBin = yBin[xInd];
        CbmTBin& tBin = xBin[tInd];
        tBin.AddHit(hit, index, fDefaultUse);
        
        if (fDefaultUse)
        {
            yBin.SetUse(true);
            xBin.SetUse(true);
            tBin.SetUse(true);
        }
    }
    
    void IterateHits(std::function<void(CbmTBin::HitHolder&)> handleHit)
    {
        for (int i = 0; i < fNofYBins; ++i)
        {
            CbmYBin& yBin = fYBins[i];
            
            if (!yBin.Use())
                continue;
            
            for (int j = 0; j < fNofXBins; ++j)
            {
                CbmXBin& xBin = yBin[j];
            
                if (!xBin.Use())
                    continue;
                
                for (int k = 0; k < fNofTBins; ++k)
                {
                    CbmTBin& tBin = xBin[k];
            
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
        
        Double_t tx = searchX / searchZ;
        Double_t ty = searchY / searchZ;
        Double_t dTx = searchDx / searchZ;
        Double_t wTx = cbmBinnedSigma * dTx;
        Double_t txMin = tx - wTx;
        Double_t txMax = tx + wTx;
        Double_t dTy = searchDy / searchZ;
        Double_t wTy = cbmBinnedSigma * dTy;
        Double_t tyMin = ty - wTy;
        Double_t tyMax = ty + wTy;
        
        Double_t deltaZMin = fMinZ - searchZ;
        Double_t deltaZMax = fMaxZ - searchZ;
        
        Double_t dxMin = dTx * fMinZ;
        Double_t dxMinSq = dxMin * dxMin;
        Double_t wxMin = cbmBinnedSigma * std::sqrt(dxMinSq + fDxSq);
        Double_t dxMax = dTx * fMaxZ;
        Double_t dxMaxSq = dxMax * dxMax;
        Double_t wxMax = cbmBinnedSigma * std::sqrt(dxMaxSq + fDxSq);
        Double_t dyMin = dTy * fMinZ;
        Double_t dyMinSq = dyMin * dyMin;
        Double_t wyMin = cbmBinnedSigma * std::sqrt(dyMinSq + fDySq);
        Double_t dyMax = dTy * fMaxZ;
        Double_t dyMaxSq = dyMax * dyMax;
        Double_t wyMax = cbmBinnedSigma * std::sqrt(dyMaxSq + fDySq);
        
        Double_t xMin = searchX + (txMin > 0 ? tx * deltaZMin - wxMin : tx * deltaZMax - wxMax) - 0.2;
        Double_t xMax = searchX + (txMax > 0 ? tx * deltaZMax + wxMax : tx * deltaZMin + wxMin) + 0.2;
            
        Double_t yMin = searchY + (tyMin > 0 ? ty * deltaZMin - wyMin : ty * deltaZMax - wyMax) - 0.2;
        Double_t yMax = searchY + (tyMax > 0 ? ty * deltaZMax + wyMax : ty * deltaZMin + wyMin) + 0.2;
        
        Double_t tCoeff = std::sqrt(1 + tx * tx + ty * ty) / cbmBinnedSOL;
        Double_t wT = cbmBinnedSigma * std::sqrt(dtSq + fDtSq);
        Double_t tMin = searchT + tCoeff * deltaZMin - wT;
        Double_t tMax = searchT + tCoeff * deltaZMax + wT;
        
        int lowerXind = GetXInd(xMin);
        int upperXind = GetXInd(xMax);
        int lowerYind = GetYInd(yMin);
        int upperYind = GetYInd(yMax);
        int lowerTind = GetTInd(tMin);
        int upperTind = GetTInd(tMax);
        
        for (int i = lowerYind; i <= upperYind; ++i)
        {
            CbmYBin& yBin = fYBins[i];
            
            for (int j = lowerXind; j <= upperXind; ++j)
            {
                CbmXBin& xBin = yBin[j];
                
                for (int k = lowerTind; k <= upperTind; ++k)
                {
                    CbmTBin& tBin = xBin[k];
                    std::list<CbmTBin::HitHolder>::iterator hitIter = tBin.HitsBegin();
                    std::list<CbmTBin::HitHolder>::iterator hitIterEnd = tBin.HitsEnd();
                    
                    for (; hitIter != hitIterEnd; ++hitIter)
                    {
                        const CbmPixelHit* hit = hitIter->hit;
                        Double_t hitZ = hit->GetZ();
                        Double_t dy = dTy * hitZ;
                        Double_t deltaZ = hitZ - searchZ;
                        Double_t y = searchY + ty * deltaZ;
                        Double_t deltaY = hit->GetY() - y;
                        
                        if (deltaY * deltaY > cbmBinnedSigmaSq * (dy * dy + hit->GetDy() * hit->GetDy() + 0.0025))
                            continue;
                        
                        Double_t dx = dTx * hitZ;
                        Double_t x = searchX + tx * deltaZ;
                        Double_t deltaX = hit->GetX() - x;
                        
                        if (deltaX * deltaX > cbmBinnedSigmaSq * (dx * dx + hit->GetDx() * hit->GetDx() + 0.0025))
                            continue;
                        
                        Double_t t = searchT + tCoeff * deltaZ;
                        Double_t deltaT = hit->GetTime() - t;
                        
                        if (deltaT * deltaT > cbmBinnedSigmaSq * (dtSq + hit->GetTimeError() * hit->GetTimeError()))
                            continue;
                        
                        handleHit(*hitIter);
                    }
                }
            }
        }
    }
    
private:
    CbmBinned3DStation(const CbmBinned3DStation&) = delete;
    CbmBinned3DStation& operator=(const CbmBinned3DStation&) = delete;
    
private:
    CbmYBin* fYBins;
};

#endif /* STATION3D_H */
