/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Bins.h
 * Author: tablyaz
 *
 * Created on August 3, 2017, 4:01 PM
 */

#ifndef BINS_H
#define BINS_H

#include <list>
#include <cmath>
#include "CbmPixelHit.h"

class CbmTBin
{
public:
    struct HitHolder
    {
        const CbmPixelHit* hit;
        Int_t index;
        bool use;
        std::list<HitHolder*> children;
    };
    
public:
    CbmTBin() : fUse(false), fHits() {}
    bool Use() const { return fUse; }
    void SetUse() { fUse = true; }
    std::list<HitHolder>::iterator HitsBegin() { return fHits.begin(); }
    std::list<HitHolder>::iterator HitsEnd() { return fHits.end(); }
    void Clear() { fHits.clear(); }
    void AddHit(const CbmPixelHit* hit, Int_t index, bool use) { fHits.push_back({ hit, index, use, {} }); }
    
private:
    bool fUse;
    std::list<HitHolder> fHits;
};

class CbmXBin
{
public:
    explicit CbmXBin(int nofTBins) : fUse(false), fTBins(reinterpret_cast<CbmTBin*> (new unsigned char[nofTBins * sizeof(CbmTBin)])), fNofTBins(nofTBins)
    {
        for (int i = 0; i < nofTBins; ++i)
            new(&fTBins[i]) CbmTBin;
    }
    
    bool Use() const { return fUse; }
    void SetUse() { fUse = true; }
    
    CbmTBin& operator[](int i) { return fTBins[i]; }
    
private:
    bool fUse;
    CbmTBin* fTBins;
    int fNofTBins;
};

class CbmYBin
{
public:
    explicit CbmYBin(int nofXBins, int nofTBins) : fUse(false), fXBins(reinterpret_cast<CbmXBin*> (new unsigned char[nofXBins * sizeof(CbmXBin)])), fNofXBins(nofXBins)
    {
        for (int i = 0; i < nofXBins; ++i)
            new(&fXBins[i]) CbmXBin(nofTBins);
    }
    
    bool Use() const { return fUse; }
    void SetUse() { fUse = true; }
    
    CbmXBin& operator[](int i) { return fXBins[i]; }
    
private:
    bool fUse;
    CbmXBin* fXBins;
    int fNofXBins;
};

class CbmZBin
{
public:
    explicit CbmZBin(int nofYBins, int nofXBins, int nofTBins) : fUse(false), fYBins(reinterpret_cast<CbmYBin*> (new unsigned char[nofYBins * sizeof(CbmYBin)])), fNofYBins(nofYBins)
    {
        for (int i = 0; i < nofYBins; ++i)
            new(&fYBins[i]) CbmYBin(nofXBins, nofTBins);
    }
    
    bool Use() const { return fUse; }
    void SetUse() { fUse = true; }
    
    CbmYBin& operator[](int i) { return fYBins[i]; }
    
private:
    bool fUse;
    CbmYBin* fYBins;
    int fNofYBins;
};

#endif /* BINS_H */
