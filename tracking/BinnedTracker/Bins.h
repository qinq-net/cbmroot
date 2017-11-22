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

class CbmXBin;

class CbmTBin
{
public:
    struct HitHolder
    {
        const CbmPixelHit* hit;
        Int_t index;
        bool use;
        CbmTBin& bin;
        //std::list<HitHolder*> children;
        bool used;
        
        void SetUse(bool v)
        {
            use = v;
            bin.SetUseRc(v);
        }
    };
    
public:
    explicit CbmTBin(CbmXBin* owner) : fOwner(owner), fUse(false), fHits() {}
    CbmTBin(const CbmTBin&) = delete;
    CbmTBin& operator=(const CbmTBin&) = delete;
    bool Use() const { return fUse; }
    void SetUse(bool v) { fUse = v; }
    void SetUseRc(bool v);
    std::list<HitHolder>::iterator HitsBegin() { return fHits.begin(); }
    std::list<HitHolder>::iterator HitsEnd() { return fHits.end(); }
    void Clear() { fHits.clear(); }
    void AddHit(const CbmPixelHit* hit, Int_t index, bool use) { fHits.push_back({ hit, index, use, *this, /*{},*/ false }); }
    
private:
    CbmXBin* fOwner;
    bool fUse;
    std::list<HitHolder> fHits;
};

class CbmYBin;

class CbmXBin
{
public:
    CbmXBin(CbmYBin* owner, int nofTBins) : fOwner(owner), fUse(false), fTBins(reinterpret_cast<CbmTBin*> (new unsigned char[nofTBins * sizeof(CbmTBin)])), fNofTBins(nofTBins)
    {
        for (int i = 0; i < nofTBins; ++i)
            new(&fTBins[i]) CbmTBin(this);
    }
    
    CbmXBin(const CbmXBin&) = delete;
    CbmXBin& operator=(const CbmXBin&) = delete;
    bool Use() const { return fUse; }
    void SetUse(bool v) { fUse = v; }
    void SetUseRc(bool v);
    
    CbmTBin& operator[](int i) { return fTBins[i]; }
    
private:
    CbmYBin* fOwner;
    bool fUse;
    CbmTBin* fTBins;
    int fNofTBins;
};

inline void CbmTBin::SetUseRc(bool v)
{
    fUse = v;
    fOwner->SetUseRc(v);
}

class CbmZBin;

class CbmYBin
{
public:
    CbmYBin(CbmZBin* owner, int nofXBins, int nofTBins) : fOwner(owner), fUse(false),
            fXBins(reinterpret_cast<CbmXBin*> (new unsigned char[nofXBins * sizeof(CbmXBin)])), fNofXBins(nofXBins)
    {
        for (int i = 0; i < nofXBins; ++i)
            new(&fXBins[i]) CbmXBin(this, nofTBins);
    }
    
    CbmYBin(const CbmYBin&) = delete;
    CbmYBin& operator=(const CbmYBin&) = delete;
    bool Use() const { return fUse; }
    void SetUse(bool v) { fUse = v; }
    void SetUseRc(bool v);
    
    CbmXBin& operator[](int i) { return fXBins[i]; }
    
private:
    CbmZBin* fOwner;
    bool fUse;
    CbmXBin* fXBins;
    int fNofXBins;
};

inline void CbmXBin::SetUseRc(bool v)
{
    fUse = v;
    fOwner->SetUseRc(v);
}

class CbmZBin
{
public:
    explicit CbmZBin(int nofYBins, int nofXBins, int nofTBins) : fUse(false), fYBins(reinterpret_cast<CbmYBin*> (new unsigned char[nofYBins * sizeof(CbmYBin)])), fNofYBins(nofYBins)
    {
        for (int i = 0; i < nofYBins; ++i)
            new(&fYBins[i]) CbmYBin(this, nofXBins, nofTBins);
    }
    
    CbmZBin(const CbmZBin&) = delete;
    CbmZBin& operator=(const CbmZBin&) = delete;
    bool Use() const { return fUse; }
    void SetUse(bool v) { fUse = v; }
    
    CbmYBin& operator[](int i) { return fYBins[i]; }
    
private:
    bool fUse;
    CbmYBin* fYBins;
    int fNofYBins;
};

inline void CbmYBin::SetUseRc(bool v)
{
    fUse = v;
    
    if (fOwner)
        fOwner->SetUse(v);
}

#endif /* BINS_H */
