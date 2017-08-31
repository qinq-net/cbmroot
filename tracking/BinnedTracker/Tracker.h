/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Tracker.h
 * Author: tablyaz
 *
 * Created on August 4, 2017, 2:23 PM
 */

#ifndef TRACKER_H
#define TRACKER_H

#include "Station.h"

class CbmBinnedTracker
{
public:
    struct Track
    {
        Track(CbmTBin::HitHolder** hits, int length, Double_t chiSq) : fHits(new CbmTBin::HitHolder*[length]), fLength(length), fChiSq(chiSq)
        {
            for (int i = 0; i < fLength; ++i)
                fHits[i] = hits[i];
        }
        
        ~Track()
        {
            delete[] fHits;
        }
        
        Track(const Track&) = delete;
        Track& operator=(const Track&) = delete;        
        CbmTBin::HitHolder** fHits;
        int fLength;
        Double_t fChiSq;
    };
    
public:
    static CbmBinnedTracker* Instance()
    {
        static CbmBinnedTracker* theInstance = 0;
        
        if (0 == theInstance)
            theInstance = new CbmBinnedTracker;
        
        return theInstance;
    }
    
public:
    CbmBinnedTracker() : fStations(), fNofStations(0), fBeforeLastLevel(0), fChiSqCut(0), fTracks() {}
    CbmBinnedTracker(const CbmBinnedTracker&) = delete;
    CbmBinnedTracker& operator=(const CbmBinnedTracker&) = delete;
    
    void AddStation(CbmBinnedStation* station)
    {
        if (fStations.empty())
            station->SetDefaultUse();
        
        fStations.push_back(station);
        fNofStations = fStations.size();
        fBeforeLastLevel = fNofStations - 2;
    }
    
    void SetChiSqCut(Double_t v) { fChiSqCut = v; }
    
    void Reconstruct(Double_t startTime)
    {
        Clear();
        
        for (std::list<CbmBinnedStation*>::iterator i = fStations.begin(); i != fStations.end(); ++i)
        {
            CbmBinnedStation* aStation = *i;
            aStation->SetMinT(startTime);
        }
        
        CbmBinnedHitReader::Instance()->Read();        
        ReconstructLocal();
        ReconstructGlobal();
        std::cout << "Reconstructed " << fTracks.size() << " tracks" << std::endl;
    }
    
    std::list<Track*>::const_iterator GetTracksBegin() const { return fTracks.begin(); }
    std::list<Track*>::const_iterator GetTracksEnd() const { return fTracks.end(); }
    
private:
    void Clear()
    {
        for (std::list<CbmBinnedStation*>::iterator i = fStations.begin(); i != fStations.end(); ++i)
        {
            CbmBinnedStation* aStation = *i;
            aStation->Clear();
        }
        
        for (std::list<Track*>::iterator i = fTracks.begin(); i != fTracks.end(); ++i)
            delete *i;
        
        fTracks.clear();
    }
    
    void ReconstructLocal()
    {
        for (std::list<CbmBinnedStation*>::iterator i = fStations.begin(); true;)
        {
            CbmBinnedStation* curStation = *i;
            ++i;
            
            if (i == fStations.end())
                break;
            
            CbmBinnedStation* nextStation = *i;
            curStation->IterateHits(
                [&](CbmTBin::HitHolder& hitHolder)->void
                {
                    nextStation->SearchHits(hitHolder.hit,
                        [&](CbmTBin::HitHolder& hitHolder2)->void
                        {                            
                            hitHolder2.SetUse(true);
                            hitHolder.children.push_back(&hitHolder2);
                        }
                    );
                }
            );
        }
    }
    
    Double_t GetChiSq(const CbmPixelHit* hit1, const CbmPixelHit* hit) const
    {
        Double_t x1 = hit1->GetX();
        Double_t y1 = hit1->GetY();
        Double_t z1 = hit1->GetZ();
        Double_t t1 = hit1->GetTime();
        Double_t dx1 = hit1->GetDx();
        Double_t dy1 = hit1->GetDy();
        Double_t dt1 = hit1->GetTimeError();
        
        Double_t tx = x1 / z1;
        Double_t ty = y1 / z1;
        
        Double_t x = hit->GetX();
        Double_t y = hit->GetY();
        Double_t z = hit->GetZ();
        Double_t t = hit->GetTime();
        Double_t dx = hit->GetDx();
        Double_t dy = hit->GetDy();
        Double_t dt = hit->GetTimeError();
        
        Double_t deltaZ = z - z1;
        x1 += tx * deltaZ;
        y1 += ty * deltaZ;
        t1 += std::sqrt(1 + tx * tx + ty * ty) * deltaZ / cbmBinnedSOL;
        
        Double_t xTerm = (x - x1) * (x - x1) / (dx1 * dx1 * z * z / z1 / z1 + dx * dx);
        Double_t yTerm = (y - y1) * (y - y1) / (dy1 * dy1 * z * z / z1 / z1 + dy * dy);
        Double_t tTerm = (t - t1) * (t - t1) / (dt1 * dt1 + dt * dt);
        
        return xTerm + yTerm + tTerm;
    }
    
    Double_t GetChiSq(const CbmPixelHit* hit1, const CbmPixelHit* hit2, const CbmPixelHit* hit) const
    {
        Double_t x1 = hit1->GetX();
        Double_t y1 = hit1->GetY();
        Double_t z1 = hit1->GetZ();
        Double_t t1 = hit1->GetTime();
        Double_t dx1 = hit1->GetDx();
        Double_t dy1 = hit1->GetDy();
        Double_t dt1 = hit1->GetTimeError();
        
        Double_t x2 = hit2->GetX();
        Double_t y2 = hit2->GetY();
        Double_t z2 = hit2->GetZ();
        Double_t t2 = hit2->GetTime();
        Double_t dx2 = hit2->GetDx();
        Double_t dy2 = hit2->GetDy();
        Double_t dt2 = hit2->GetTimeError();
        
        Double_t deltaZ12 = z2 - z1;
        Double_t x12 = (x1 + x2) / 2;
        Double_t y12 = (y1 + y2) / 2;
        Double_t t12 = (t1 + t2) / 2;
        Double_t tx = (x2 - x1) / deltaZ12;
        Double_t ty = (y2 - y1) / deltaZ12;
        
        Double_t x = hit->GetX();
        Double_t y = hit->GetY();
        Double_t z = hit->GetZ();
        Double_t t = hit->GetTime();
        Double_t dx = hit->GetDx();
        Double_t dy = hit->GetDy();
        Double_t dt = hit->GetTimeError();
        
        Double_t deltaZ = z - (z1 + z2) / 2;
        x12 += tx * deltaZ;
        y12 += ty * deltaZ;
        t12 += std::sqrt(1 + tx * tx + ty * ty) * deltaZ / cbmBinnedSOL;
        
        return (x - x12) * (x - x12) / ((0.5 - deltaZ / deltaZ12) * (0.5 - deltaZ / deltaZ12) * dx1 * dx1 + (0.5 + deltaZ / deltaZ12) * (0.5 + deltaZ / deltaZ12) * dx2 * dx2 + dx * dx) +
               (y - y12) * (y - y12) / ((0.5 - deltaZ / deltaZ12) * (0.5 - deltaZ / deltaZ12) * dy1 * dy1 + (0.5 + deltaZ / deltaZ12) * (0.5 + deltaZ / deltaZ12) * dy2 * dy2 + dy * dy) +
               (t - t12) * (t - t12) / ((dt1 * dt1 + dt2 * dt2) / 2 + dt * dt);
    }
    
    void TraverseTrackCandidates(int level, CbmTBin::HitHolder** trackStart, Double_t chiSq, std::list<Track*>& candidates)
    {
        CbmTBin::HitHolder* hitHolder = trackStart[level];
        
        for (std::list<CbmTBin::HitHolder*>::iterator i = hitHolder->children.begin(); i != hitHolder->children.end(); ++i)
        {
            CbmTBin::HitHolder* childHolder = *i;
            const CbmPixelHit* childHit = childHolder->hit;
            Double_t deltaChiSq = 0 == level ? GetChiSq(hitHolder->hit, childHit) : GetChiSq(trackStart[level - 1]->hit, hitHolder->hit, childHit);
            Double_t chiSq2 = chiSq + deltaChiSq;            
            trackStart[level + 1] = childHolder;
            
            if (level == fBeforeLastLevel)
            {
                Track* aCandidate = new Track(trackStart, fNofStations, chiSq2);                
                candidates.push_back(aCandidate);
            }
            else
                TraverseTrackCandidates(level + 1, trackStart, chiSq2, candidates);
        }
    }
    
    void ReconstructGlobal()
    {
        CbmBinnedStation* startStation = fStations.front();
        startStation->IterateHits(
            [&](CbmTBin::HitHolder& hitHolder)->void
            {
                CbmTBin::HitHolder* trackHolders[fNofStations];
                trackHolders[0] = &hitHolder;
                std::list<Track*> candidates;
                TraverseTrackCandidates(0, trackHolders, 0, candidates);
                
                Track* bestCandidate = 0;
                
                for (std::list<Track*>::iterator i = candidates.begin(); i != candidates.end(); ++i)
                {
                    Track* aCandidate = *i;
                    
                    if (0 == bestCandidate || aCandidate->fChiSq < bestCandidate->fChiSq)
                    {
                        delete bestCandidate;
                        bestCandidate = aCandidate;
                    }
                    else
                        delete aCandidate;
                }
                
                if (0 != bestCandidate)
                    fTracks.push_back(bestCandidate);
            }
        );
    }
    
private:
    std::list<CbmBinnedStation*> fStations;
    int fNofStations;
    int fBeforeLastLevel;
    Double_t fChiSqCut;
    std::list<Track*> fTracks;
};

#endif /* TRACKER_H */
