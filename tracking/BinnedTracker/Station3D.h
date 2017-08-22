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
            new(&fYBins[i]) CbmYBin(nofXBins, nofTBins);
    }
    
    void Clear()
    {
        for (int i = 0; i < fNofYBins; ++i)
        {
            CbmYBin& yBin = fYBins[i];
            
            for (int j = 0; j < fNofXBins; ++j)
            {
                CbmXBin& xBin = yBin[j];
                
                for (int k = 0; k < fNofTBins; ++k)
                {
                    CbmTBin& tBin = xBin[k];
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
            yBin.SetUse();
            xBin.SetUse();
            tBin.SetUse();
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
        Double_t dTy = searchDy / searchZ;
        
        Double_t deltaZ = fZ - searchZ;
        
        Double_t x = searchX + tx * deltaZ;
        Double_t y = searchY + ty * deltaZ;
        Double_t t = searchT + std::sqrt(1 + tx * tx + ty * ty) * deltaZ / cbmBinnedSOL;
        Double_t dx = dTx * fZ;
        Double_t dxSq = dx * dx;
        Double_t dy = dTy * fZ;
        Double_t dySq = dy * dy;
        
        Double_t wX = cbmBinnedSigma * std::sqrt(dx * dx + fDx * fDx);
        int lowerXind = GetXInd(x - wX);
        int upperXind = GetXInd(x + wX);
        Double_t wY = cbmBinnedSigma * std::sqrt(dy * dy + fDy * fDy);
        int lowerYind = GetYInd(y - wY);
        int upperYind = GetYInd(y + wY);
        Double_t wT = cbmBinnedSigma * std::sqrt(dt * dt + fDt * fDt);
        int lowerTind = GetTInd(t - wT);
        int upperTind = GetTInd(t + wT);
        
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
                        Double_t deltaY = hit->GetY() - y;
                        
                        if (deltaY * deltaY > cbmBinnedSigmaSq * (dySq + hit->GetDy() * hit->GetDy()))
                            continue;
                        
                        Double_t deltaX = hit->GetX() - x;
                        
                        if (deltaX * deltaX > cbmBinnedSigmaSq * (dxSq + hit->GetDx() * hit->GetDx()))
                            continue;
                        
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
