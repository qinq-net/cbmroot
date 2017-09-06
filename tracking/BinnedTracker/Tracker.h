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
#include "Station3D.h"
#include <iostream>
using namespace std;

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
    static CbmBinnedTracker* Instance();
    
public:
    CbmBinnedTracker(Double_t beamDx, Double_t beamDy) : fStations(), fNofStations(0), fBeforeLastLevel(0), fChiSqCut(0), fTracks(),
            fBeamDx(beamDx), fBeamDxSq(beamDx * beamDx), fBeamDy(beamDy), fBeamDySq(beamDy * beamDy), fVertex()
    {
        fVertex.SetX(0);
        fVertex.SetY(0);
        fVertex.SetZ(0);
        fVertex.SetDx(beamDx);
        fVertex.SetDy(beamDy);
        fVertex.SetDxy(0);
        fVertex.SetZ(0);
        fVertex.SetTime(0);
        fVertex.SetTimeError(0);
    }
    
    CbmBinnedTracker(const CbmBinnedTracker&) = delete;
    CbmBinnedTracker& operator=(const CbmBinnedTracker&) = delete;
    
    Double_t GetBeamDxSq() const { return fBeamDxSq; }
    Double_t GetBeamDySq() const { return fBeamDySq; }
    CbmPixelHit& GetVertex() { return fVertex; }
    
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
        
        std::cout << "Segments on stations: ";
        
        for (std::list<CbmBinnedStation*>::const_iterator i = fStations.begin(); i != fStations.end(); ++i)
        {
            CbmBinnedStation* station = *i;
            std::cout << "[" << station->fSegments.size() << "]";
        }
        
        std::cout << std::endl;
        
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
        static int nofUsedHits[] = { 0, 0, 0, 0, 0, 0 };
        int stNo = 0;
        
        fStations.front()->CreateSegmentsFromHits();
        
        for (std::list<CbmBinnedStation*>::iterator i = fStations.begin(); true;)
        {
            CbmBinnedStation* curStation = *i;
            ++i;
            
            if (i == fStations.end())
                break;
            
            CbmBinnedStation* nextStation = *i;
            /*curStation->IterateHits(
                [&](CbmTBin::HitHolder& hitHolder)->void
                {
                   ++nofUsedHits[stNo];
                    nextStation->SearchHits(hitHolder.hit,
                        [&](CbmTBin::HitHolder& hitHolder2)->void
                        {                            
                            if (nextStation == fStations.back() && !hitHolder2.use)
                                ++nofUsedHits[stNo + 1];
                            
                            hitHolder2.SetUse(true);
                            hitHolder.children.push_back(&hitHolder2);
                        }
                    );*/
            curStation->IterateSegments(
                [&](CbmBinnedStation::Segment& segment)->void
                {
                    nextStation->SearchHits(segment,
                        [&](CbmTBin::HitHolder& hitHolder)->void
                        {                                                        
                        }
                    );
                }
            );
            
            ++stNo;
        }
        
        cout << "NOF used hits on:" << endl;
        
        for (int i = 0; i < sizeof(nofUsedHits) / sizeof(int); ++i)
            cout << "On station " << i << ": " << nofUsedHits[i] << endl;
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
        
        x1 = tx * z;
        y1 = ty * z;
        t1 += std::sqrt(1 + tx * tx + ty * ty) * (z - z1) / cbmBinnedSOL;
        
        Double_t xTerm = (x - x1) * (x - x1) / ((1.5 - 2 * z / z1) * (1.5 - 2 * z / z1) * fBeamDxSq +
                  (2 * z / z1 - 0.5) * (2 * z / z1 - 0.5) * dx1 * dx1 + dx * dx);
        Double_t yTerm = (y - y1) * (y - y1) / ((1.5 - 2 * z / z1) * (1.5 - 2 * z / z1) * fBeamDySq +
                  (2 * z / z1 - 0.5) * (2 * z / z1 - 0.5) * dy1 * dy1 + dy * dy);
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
    
    Double_t GetChiSq(CbmBinnedStation::Segment& segment1, CbmBinnedStation::Segment& segment2) const
    {
        const CbmPixelHit* hit1 = segment1.begin.hit;
        const CbmPixelHit* hit2 = segment1.end.hit;
        
        if (hit2 != segment2.begin.hit)
            return cbmBinnedCrazyChiSq;
        
        const CbmPixelHit* hit = segment2.end.hit;
        Double_t x1 = hit1->GetX();
        Double_t y1 = hit1->GetY();
        Double_t z1 = hit1->GetZ();
        Double_t t1 = hit1->GetTime();
        Double_t dx1 = hit1->GetDx();
        Double_t dx1Sq = dx1 * dx1;
        Double_t dy1 = hit1->GetDy();
        Double_t dy1Sq = dy1 * dy1;
        Double_t dt1 = hit1->GetTimeError();
        Double_t dt1Sq = dt1 * dt1;
        
        Double_t x2 = hit2->GetX();
        Double_t y2 = hit2->GetY();
        Double_t z2 = hit2->GetZ();
        Double_t t2 = hit2->GetTime();
        Double_t dx2 = hit2->GetDx();
        Double_t dx2Sq = dx2 * dx2;
        Double_t dy2 = hit2->GetDy();
        Double_t dy2Sq = dy2 * dy2;
        Double_t dt2 = hit2->GetTimeError();
        Double_t dt2Sq = dt2 * dt2;
        
        Double_t x = hit->GetX();
        Double_t y = hit->GetY();
        Double_t z = hit->GetZ();
        Double_t t = hit->GetTime();
        Double_t dx = hit->GetDx();
        Double_t dxSq = dx * dx;
        Double_t dy = hit->GetDy();
        Double_t dySq = dy * dy;
        Double_t dt = hit->GetTimeError();
        Double_t dtSq = dt * dt;
        
        Double_t deltaZ12 = z2 - z1;
        Double_t deltaZ = z - (z1 + z2) / 2;
        Double_t coeff1 = (0.5 - deltaZ / deltaZ12);
        Double_t coeff1Sq = coeff1 * coeff1;
        Double_t coeff2 = (0.5 + deltaZ / deltaZ12);
        Double_t coeff2Sq = coeff2 * coeff2;
        
        Double_t x12 = coeff1 * x1 + coeff2 * x2;
        Double_t dx12Sq = coeff1Sq * dx1Sq + coeff2Sq * dx2Sq;
        Double_t y12 = coeff1 * y1 + coeff2 * y2;
        Double_t dy12Sq = coeff1Sq * dy1Sq + coeff2Sq * dy2Sq;
        
        Double_t tx = (x2 - x1) / deltaZ12;
        Double_t ty = (y2 - y1) / deltaZ12;
        Double_t t12;
        Double_t dt12Sq;
        
        if (0 == dt1)
        {
            t12 = t2 + std::sqrt(1 + tx * tx + ty * ty) * (z - z2) / cbmBinnedSOL;
            dt12Sq = dt2Sq;
        }
        else
        {
            t12 = (t1 + t2) /2 + std::sqrt(1 + tx * tx + ty * ty) * deltaZ12 / cbmBinnedSOL;
            dt12Sq = (dt1Sq + dt2Sq) / 2;
        }
        
        return (x - x12) * (x - x12) / (dx12Sq + dxSq) + (y - y12) * (y - y12) / (dy12Sq + dySq) + (t - t12) * (t - t12) / (dt12Sq + dtSq);
    }
    
    void TraverseTrackCandidates(int level, CbmTBin::HitHolder** trackStart, Double_t chiSq, std::list<Track*>& candidates)
    {
        static int nofCalls = 0;
        static int nofTracks = 0;
        static int maxNofCalls = 0;
        static int totalNofCalls = 0;
        static int maxBranches[] = { 0, 0, 0, 0, 0 };
        static Double_t maxHitErrorX[] = { 0, 0, 0, 0, 0 };
        static Double_t maxHitErrorY[] = { 0, 0, 0, 0, 0 };
        
        if (0 == level)
        {
            if (maxNofCalls < nofCalls)
                maxNofCalls = nofCalls;
            
            totalNofCalls += nofCalls;
            
            if (nofTracks > 0)
            {
                std::cout << std::endl << std::endl << "The number of recursive calls per track: " << nofCalls << std::endl;
                std::cout << "The average number of recursive calls per track: " << totalNofCalls / nofTracks << std::endl;
                std::cout << "The maximum number of recursive calls per track: " << maxNofCalls << std::endl;
                std::cout << "The maximum number of branches: ";
                
                for (int i = 0; i < sizeof(maxBranches) / sizeof(int); ++i)
                    std::cout << "[" << maxBranches[i] << "]";
                
                std::cout << std::endl;
                std::cout << "The maximum hit errors X: ";
                
                for (int i = 0; i < sizeof(maxHitErrorX) / sizeof(Double_t); ++i)
                    std::cout << "[" << maxHitErrorX[i] << "]";
                
                std::cout << std::endl;
                std::cout << "The maximum hit errors Y: ";
                
                for (int i = 0; i < sizeof(maxHitErrorY) / sizeof(Double_t); ++i)
                    std::cout << "[" << maxHitErrorY[i] << "]";
                
                std::cout << std::endl;
                std::cout << "Nof tracks: " << nofTracks << std::endl << std::endl;
            }
            
            nofCalls = 1;
            ++nofTracks;
        }
        else
            ++nofCalls;
        
        CbmTBin::HitHolder* hitHolder = trackStart[level];
        
        int nofBranches = hitHolder->children.size();
        
        if (maxBranches[level] < nofBranches)
            maxBranches[level] = nofBranches;
        
        if (maxHitErrorX[level] < hitHolder->hit->GetDx())
            maxHitErrorX[level] = hitHolder->hit->GetDx();
        
        if (maxHitErrorY[level] < hitHolder->hit->GetDy())
            maxHitErrorY[level] = hitHolder->hit->GetDy();
        
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
        /*CbmBinnedStation* startStation = fStations.front();
        startStation->IterateHits(
            [&](CbmTBin::HitHolder& hitHolder)->void
            {
                CbmTBin::HitHolder* trackHolders[fNofStations];
                trackHolders[0] = &hitHolder;
                std::list<Track*> candidates;
                TraverseTrackCandidates(0, trackHolders, 0, candidates);
                
                int nofCandidates = candidates.size();
                
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
        );*/
        std::list<CbmBinnedStation*>::reverse_iterator stationIter = fStations.rbegin();
        CbmBinnedStation* lastStation = *stationIter++;
        lastStation->NulifySegments();
        
        for (; stationIter != fStations.rend(); ++stationIter)
        {
            CbmBinnedStation* station = *stationIter;
            station->IterateSegments(
            [&](CbmBinnedStation::Segment& segment)->void
            {   
                for (std::list<CbmBinnedStation::Segment*>::iterator i = segment.children.begin(); i != segment.children.end(); ++i)
                {
                    CbmBinnedStation::Segment* child = *i;
                    
                    if (child->chiSq > fChiSqCut)
                        continue;
                    
                    Double_t chiSq = GetChiSq(segment, *child) + child->chiSq;
                    
                    if (chiSq > fChiSqCut)
                        continue;
                    
                    if (segment.chiSq > chiSq)
                    {
                        segment.bestBranch = child;
                        segment.chiSq = chiSq;
                    }
                }
            }
            );
        }
        
        fStations.front()->IterateSegments(
            [&](CbmBinnedStation::Segment& segment)->void
            {
                if (segment.chiSq > fChiSqCut)
                    return;
                
                CbmTBin::HitHolder* trackHolders[fNofStations];
                CbmBinnedStation::Segment* segChain = &segment;
                int i = 0;
                
                while (0 != segChain)
                {
                    trackHolders[i++] = &segChain->end;
                    segChain = segChain->bestBranch;
                }
                
                Track* aCandidate = new Track(trackHolders, fNofStations, segment.chiSq);                
                fTracks.push_back(aCandidate);
            }
        );
    }
    
private:
    std::list<CbmBinnedStation*> fStations;
    int fNofStations;
    int fBeforeLastLevel;
    Double_t fChiSqCut;
    std::list<Track*> fTracks;
    Double_t fBeamDx;
    Double_t fBeamDxSq;
    Double_t fBeamDy;
    Double_t fBeamDySq;
    CbmPixelHit fVertex;
};

#endif /* TRACKER_H */
