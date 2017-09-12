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
    int fNofTrueSegments;
    int fNofWrongSegments;
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
    CbmBinnedTracker(Double_t beamDx, Double_t beamDy) : fNofTrueSegments(0), fNofWrongSegments(0), fStations(), fNofStations(0), fBeforeLastLevel(0), fChiSqCut(0), fTracks(),
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
        
        for (std::vector<CbmBinnedStation*>::iterator i = fStations.begin(); i != fStations.end(); ++i)
        {
            CbmBinnedStation* aStation = *i;
            aStation->SetMinT(startTime);
        }
        
        CbmBinnedHitReader::Instance()->Read();        
        ReconstructLocal();
        ReconstructGlobal();
        
        std::cout << "Segments on stations: ";
        
        for (std::vector<CbmBinnedStation*>::const_iterator i = fStations.begin(); i != fStations.end(); ++i)
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
        for (std::vector<CbmBinnedStation*>::iterator i = fStations.begin(); i != fStations.end(); ++i)
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
        fStations.front()->CreateSegmentsFromHits();
        
        for (std::vector<CbmBinnedStation*>::iterator i = fStations.begin(); true;)
        {
            CbmBinnedStation* curStation = *i;
            ++i;
            
            if (i == fStations.end())
                break;
            
            CbmBinnedStation* nextStation = *i;
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
        }
    }
    
    Double_t GetChiSq(int stationNumber, Double_t scatXSq, Double_t scatYSq, CbmBinnedStation::Segment& segment1, CbmBinnedStation::Segment& segment2) const
    {
        const CbmPixelHit* hit1 = segment1.begin.hit;
        const CbmPixelHit* hit2 = segment1.end.hit;
        
        if (hit2 != segment2.begin.hit)
            return cbmBinnedCrazyChiSq;
        
        /*Double_t chiSqCoeffX1 = stationNumber < 1 ? 1 : fStations[stationNumber - 1]->GetNofSigmaXSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffY1 = stationNumber < 1 ? 1 : fStations[stationNumber - 1]->GetNofSigmaYSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffX2 = fStations[stationNumber]->GetNofSigmaXSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffY2 = fStations[stationNumber]->GetNofSigmaYSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffX = fStations[stationNumber + 1]->GetNofSigmaXSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffY = fStations[stationNumber + 1]->GetNofSigmaYSq() / cbmBinnedSigmaSq;*/
        
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
        
        
        /*dx1Sq *= chiSqCoeffX1;
        dy1Sq *= chiSqCoeffY1;
        dx2Sq *= chiSqCoeffX2;
        dy2Sq *= chiSqCoeffY2;
        dxSq *= chiSqCoeffX;
        dySq *= chiSqCoeffY;*/
        
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
        
        return (x - x12) * (x - x12) / (dx12Sq + dxSq + scatXSq)
                + (y - y12) * (y - y12) / (dy12Sq + dySq + scatYSq)
                //+ (t - t12) * (t - t12) / (dt12Sq + dtSq)
                ;
    }
    
    Double_t GetChiSqWithoutTime(CbmBinnedStation::Segment& segment1, CbmBinnedStation::Segment& segment2) const
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
        
        Double_t x2 = hit2->GetX();
        Double_t y2 = hit2->GetY();
        Double_t z2 = hit2->GetZ();
        Double_t t2 = hit2->GetTime();
        Double_t dx2 = hit2->GetDx();
        Double_t dx2Sq = dx2 * dx2;
        Double_t dy2 = hit2->GetDy();
        Double_t dy2Sq = dy2 * dy2;
        
        Double_t x = hit->GetX();
        Double_t y = hit->GetY();
        Double_t z = hit->GetZ();
        Double_t t = hit->GetTime();
        Double_t dx = hit->GetDx();
        Double_t dxSq = dx * dx;
        Double_t dy = hit->GetDy();
        Double_t dySq = dy * dy;
        
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
        
        return (x - x12) * (x - x12) / (dx12Sq + dxSq) + (y - y12) * (y - y12) / (dy12Sq + dySq);
    }
    
    void ReconstructGlobal()
    {
        std::vector<CbmBinnedStation*>::reverse_iterator stationIter = fStations.rbegin();
        CbmBinnedStation* lastStation = *stationIter++;
        lastStation->NulifySegments();
        
        int stN = 4;
        CbmBinnedStation* nextStation = lastStation;
        
        for (; stationIter != fStations.rend(); ++stationIter)
        {
            CbmBinnedStation* station = *stationIter;
            station->IterateSegments(
            [&](CbmBinnedStation::Segment& segment)->void
            {
                CbmBinnedStation::Segment* closest = 0;
                Double_t clDist = 0;
                CbmBinnedStation::Segment* segChiSq = 0;
                Double_t minSegChiSq = 0;
                
                for (std::list<CbmBinnedStation::Segment*>::iterator i = segment.children.begin(); i != segment.children.end(); ++i)
                {
                    CbmBinnedStation::Segment* child = *i;
                    
                    if (child->chiSq > fChiSqCut)
                        continue;
                    
                    Double_t chiSq = GetChiSq(stN, nextStation->GetScatXSq(), nextStation->GetScatYSq(), segment, *child) + child->chiSq;
                    
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
            --stN;
            nextStation = station;
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
    //std::list<CbmBinnedStation*> fStations;
    std::vector<CbmBinnedStation*> fStations;
    int fNofStations;
    int fBeforeLastLevel;
    Double_t fChiSqCut;
    std::list<Track*> fTracks;
    Double_t fBeamDx;
    Double_t fBeamDxSq;
    Double_t fBeamDy;
    Double_t fBeamDySq;
    CbmPixelHit fVertex;
    
    friend class CbmBinnedTrackerQA;
};

#endif /* TRACKER_H */
