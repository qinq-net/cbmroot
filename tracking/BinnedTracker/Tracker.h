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

//#define CBM_BINNED_DEBUG

#ifdef CBM_BINNED_DEBUG
#include "Debug.h"
#endif//CBM_BINNED_DEBUG

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
    CbmBinnedTracker(Double_t beamDx, Double_t beamDy) : fNofTrueSegments(0), fNofWrongSegments(0), fStations(), fStationArray(),
            fNofStations(0), fBeforeLastLevel(0), fChiSqCut(0), fTracks(), fBeamDx(beamDx), fBeamDxSq(beamDx * beamDx), fBeamDy(beamDy), fBeamDySq(beamDy * beamDy),
            fVertex(), fVertexPseudoStation(0)
#ifdef CBM_BINNED_DEBUG
    , fDebug()
#endif//CBM_BINNED_DEBUG
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
    
    ~CbmBinnedTracker()
    {
        delete fVertexPseudoStation;
    }
    
    CbmBinnedTracker(const CbmBinnedTracker&) = delete;
    CbmBinnedTracker& operator=(const CbmBinnedTracker&) = delete;
    
    void Init()
    {
        fVertexPseudoStation = new CbmBinned3DStation(-0.1, 0.1, 1, 1, 1);
        fVertexPseudoStation->SetMinY(-0.1);
        fVertexPseudoStation->SetMaxY(0.1);
        fVertexPseudoStation->SetMinX(-0.1);
        fVertexPseudoStation->SetMaxX(0.1);
        fVertexPseudoStation->Init();
        fVertexPseudoStation->AddHit(&fVertex, -1);
        
        for (std::map<Double_t, CbmBinnedStation*>::const_iterator i = fStations.begin(); i != fStations.end(); ++i)
            fStationArray.push_back(i->second);
    }
    
    Double_t GetBeamDxSq() const { return fBeamDxSq; }
    Double_t GetBeamDySq() const { return fBeamDySq; }
    CbmPixelHit& GetVertex() { return fVertex; }
    
    void AddStation(CbmBinnedStation* station)
    {
        if (fStations.empty())
            station->SetDefaultUse();
        
        fStations[station->GetMinZ()] = station;
        fNofStations = fStations.size();
        fBeforeLastLevel = fNofStations - 2;
    }
    
    void SetChiSqCut(Double_t v) { fChiSqCut = v; }
    
    void SetCheckUsed(bool v)
    {        
        for (std::vector<CbmBinnedStation*>::iterator i = fStationArray.begin(); i != fStationArray.end(); ++i)
        {
            CbmBinnedStation* aStation = *i;
            aStation->SetCheckUsed(v);
        }
    }
    
    void Reconstruct(Double_t startTime)
    {
        Clear();
        
        for (std::map<Double_t, CbmBinnedStation*>::iterator i = fStations.begin(); i != fStations.end(); ++i)
        {
            CbmBinnedStation* aStation = i->second;
            aStation->SetMinT(startTime);
        }
        
        CbmBinnedHitReader::Instance()->Read();        
        //ReconstructLocal();
        //ReconstructGlobal();
        FollowTracks(-1);
        SetCheckUsed(true);
        FollowTracks(0);
        
        std::cout << "Segments on stations: ";
        
        for (std::map<Double_t, CbmBinnedStation*>::const_iterator i = fStations.begin(); i != fStations.end(); ++i)
        {
            CbmBinnedStation* station = i->second;
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
        for (std::map<Double_t, CbmBinnedStation*>::iterator i = fStations.begin(); i != fStations.end(); ++i)
        {
            CbmBinnedStation* aStation = i->second;
            aStation->Clear();
        }
        
        for (std::list<Track*>::iterator i = fTracks.begin(); i != fTracks.end(); ++i)
            delete *i;
        
        fTracks.clear();
    }
    
    void ReconstructLocal()
    {
        std::map<Double_t, CbmBinnedStation*>::iterator i = fStations.begin();
        i->second->CreateSegmentsFromHits();
        
        for (; true;)
        {
            CbmBinnedStation* curStation = i->second;
            ++i;
            
            if (i == fStations.end())
                break;
            
            CbmBinnedStation* nextStation = i->second;
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
        
#if 0//def CBM_BINNED_DEBUG
        static int nofRefTracks = 0;
        static int nofRecoRefHits[] = { 0, 0, 0, 0, 0, 0 };
        fDebug.Exec();
        int stNo = 0;
        CbmBinnedStation* firstStation = fStations.begin()->second;
        
        for (int i = 0; i < fDebug.fMCTracks.size(); ++i)
        {
            const CbmBinnedDebug::MCTrack& mcTrack = fDebug.fMCTracks[i];
            
            if (!mcTrack.isRef)
                continue;
            
            ++nofRefTracks;
            bool found[] = { false, false, false, false, false, false };
            firstStation->IterateSegments(
                [&](CbmBinnedStation::Segment& segment)->void
                {
                    CbmTBin::HitHolder* hitHolder = segment.end;
                    
                    if (fDebug.TrackHasHit(stNo, i, hitHolder->index))
                        found[stNo] = true;
                    
                    std::list<CbmBinnedStation::Segment*> tmp(segment.children);
                    
                    while(!tmp.empty())
                    {
                        ++stNo;
                        std::list<CbmBinnedStation::Segment*> tmp2;
                        
                        for (std::list<CbmBinnedStation::Segment*>::const_iterator j = tmp.begin(); j != tmp.end(); ++j)
                        {
                            const CbmBinnedStation::Segment* s = *j;
                            tmp2.insert(tmp2.end(), s->children.begin(), s->children.end());
                            CbmTBin::HitHolder* hh = s->end;
                            
                            if (fDebug.TrackHasHit(stNo, i, hh->index))
                                found[stNo] = true;
                        }
                        
                        tmp.clear();
                        tmp.splice(tmp.end(), tmp2);
                    }
                    
                    stNo = 0;
                }
            );
            
            std::cout << "Reco hits: ";
            
            for (int j = 0; j < sizeof(nofRecoRefHits) / sizeof(int); ++j)
            {
                if (found[j])
                    ++nofRecoRefHits[j];
                
                double proc = 100 * nofRecoRefHits[j];
                proc /= nofRefTracks;
                std::cout << "[" << proc << "%=" << nofRecoRefHits[j] << "/" << nofRefTracks << "]";
            }
            
            std::cout << std::endl << std::endl << std::endl;
        }
#endif//CBM_BINNED_DEBUG
    }
    
    Double_t GetChiSq(Double_t scatXSq, Double_t scatYSq, CbmBinnedStation::Segment& segment1, CbmBinnedStation::Segment& segment2) const
    {
        const CbmPixelHit* hit1 = segment1.begin->hit;
        const CbmPixelHit* hit2 = segment1.end->hit;
        
        if (hit2 != segment2.begin->hit)
            return cbmBinnedCrazyChiSq;
        
        /*Double_t chiSqCoeffX1 = stationNumber < 1 ? 1 : fStations[stationNumber - 1]->GetNofSigmaXSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffY1 = stationNumber < 1 ? 1 : fStations[stationNumber - 1]->GetNofSigmaYSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffX2 = fStations[stationNumber]->GetNofSigmaXSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffY2 = fStations[stationNumber]->GetNofSigmaYSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffX = fStations[stationNumber + 1]->GetNofSigmaXSq() / cbmBinnedSigmaSq;
        Double_t chiSqCoeffY = fStations[stationNumber + 1]->GetNofSigmaYSq() / cbmBinnedSigmaSq;*/
        
        const CbmPixelHit* hit = segment2.end->hit;
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
        const CbmPixelHit* hit1 = segment1.begin->hit;
        const CbmPixelHit* hit2 = segment1.end->hit;
        
        if (hit2 != segment2.begin->hit)
            return cbmBinnedCrazyChiSq;
        
        const CbmPixelHit* hit = segment2.end->hit;
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
    
    /*void ReconstructGlobal()
    {
        std::map<Double_t, CbmBinnedStation*>::reverse_iterator stationIter = fStations.rbegin();
        CbmBinnedStation* lastStation = (stationIter++)->second;
        lastStation->NulifySegments();
        
        int stN = 4;
        CbmBinnedStation* nextStation = lastStation;
        
        for (; stationIter != fStations.rend(); ++stationIter)
        {
            CbmBinnedStation* station = stationIter->second;
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
                    
                    Double_t chiSq = GetChiSq(nextStation->GetScatXSq(), nextStation->GetScatYSq(), segment, *child) + child->chiSq;
                    
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
        
        fStations.begin()->second->IterateSegments(
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
    }*/
    /*void TraverseTrackCandidates(int level, CbmBinnedStation::Segment** trackStart, CbmTBin::HitHolder** hhs, Double_t chiSq, std::list<Track*>& candidates, Double_t scatXSqs[6], Double_t scatYSqs[6])
    {              
        CbmBinnedStation::Segment* segment = trackStart[level];

        for (std::list<CbmBinnedStation::Segment*>::iterator i = segment->children.begin(); i != segment->children.end(); ++i)
        {
            CbmBinnedStation::Segment* childSegment = *i;
            Double_t deltaChiSq = GetChiSq(scatXSqs[level + 1], scatYSqs[level + 1], *segment, *childSegment);
            Double_t chiSq2 = chiSq + deltaChiSq;            
            trackStart[level + 1] = childSegment;
            hhs[level + 1] = childSegment->end;

            if (level == 4)
            {
                Track* aCandidate = new Track(hhs, 6, chiSq2);                
                candidates.push_back(aCandidate);
            }
            else
                TraverseTrackCandidates(level + 1, trackStart, hhs, chiSq2, candidates, scatXSqs, scatYSqs);
        }
    }*/
    
    //KFStation fKFStations[6];

    void KFAddPointCoord(int stationNumber, CbmBinnedStation::KFParamsCoord& param, const CbmBinnedStation::KFParamsCoord& prevParam,
        Double_t m, Double_t V, Double_t& chi2, Double_t z, Double_t prevZ, int coordNumber)
    {
        //const KFStation& station = fKFStations[stationNumber];
        const CbmBinnedStation* station = fStationArray[stationNumber];
        CbmBinnedStation::KFStation::Q Q = { 0 == coordNumber ? station->GetScatXSq() : station->GetScatYSq(), 0, 0, 0 };
        Double_t deltaZ = z - prevZ;
        Double_t deltaZSq = deltaZ * deltaZ;

        // Extrapolate.
        param.coord += prevParam.tg * deltaZ; // params[k].tg is unchanged.

        // Filter.
        param.C11 += prevParam.C12 * deltaZ + prevParam.C21 * deltaZ + prevParam.C22 * deltaZSq + Q.Q11;
        param.C12 += prevParam.C22 * deltaZ + Q.Q12;
        param.C21 += prevParam.C22 * deltaZ + Q.Q21;
        param.C22 += Q.Q22;

        Double_t S = 1.0 / (V + param.C11);
        Double_t Kcoord = param.C11 * S;
        Double_t Ktg = param.C21 * S;
        Double_t dzeta = m - param.coord;
        param.coord += Kcoord * dzeta;
        param.tg += Ktg * dzeta;
        param.C21 -= param.C11 * Ktg;
        param.C22 -= param.C12 * Ktg;
        param.C11 *= 1.0 - Kcoord;
        param.C12 *= 1.0 - Kcoord;
        chi2 += dzeta * S * dzeta;
    }

    void KFAddPoint(int stationNumber, CbmBinnedStation::KFParams& param, const CbmBinnedStation::KFParams& prevParam, Double_t m[2], Double_t V[2], Double_t z, Double_t prevZ)
    {
        KFAddPointCoord(stationNumber, param.xParams, prevParam.xParams, m[0], V[0], param.chi2, z, prevZ, 0);
        KFAddPointCoord(stationNumber, param.yParams, prevParam.yParams, m[1], V[1], param.chi2, z, prevZ, 1);
    }
    
    void TraverseTrackCandidates(int level, CbmBinnedStation::Segment** trackStart, CbmTBin::HitHolder** hhs, std::list<Track*>& candidates, CbmBinnedStation::KFParams kfParamsPrev)
    {              
        CbmBinnedStation::Segment* segment = trackStart[level];
        const CbmPixelHit* hit = segment->end->hit;
        CbmBinnedStation::KFParams kfParams = kfParamsPrev;
        Double_t m[2] = { hit->GetX(), hit->GetY() };
        Double_t V[2] = { hit->GetDx() * hit->GetDx(), hit->GetDy() * hit->GetDy() };
        KFAddPoint(level, kfParams, kfParamsPrev, m, V, hit->GetZ(), 0 == level ? 0 : hhs[level - 1]->hit->GetZ());
        
        CbmBinnedSettings* settings = CbmBinnedSettings::Instance();
        
        if (level == fNofStations - 1)
        {
            Track* aCandidate = new Track(hhs, fNofStations, kfParams.chi2);
            candidates.push_back(aCandidate);
            return;
        }

        for (std::list<CbmBinnedStation::Segment*>::iterator i = segment->children.begin(); i != segment->children.end(); ++i)
        {
            CbmBinnedStation::Segment* childSegment = *i;         
            trackStart[level + 1] = childSegment;
            hhs[level + 1] = childSegment->end;
            TraverseTrackCandidates(level + 1, trackStart, hhs, candidates, kfParams);
        }
    }

    void ReconstructGlobal()
    {
        std::map<Double_t, CbmBinnedStation*>::iterator startStationIter = fStations.begin();        
        CbmBinnedStation* startStation = startStationIter->second;
        startStation->IterateSegments(
            [&](CbmBinnedStation::Segment& segment)->void
            {
                CbmBinnedStation::Segment* segments[fNofStations];
                segments[0] = &segment;
                CbmTBin::HitHolder* trackHolders[fNofStations];
                trackHolders[0] = segment.end;                
                std::list<Track*> candidates;
                
                //TraverseTrackCandidates(0, segments, trackHolders, 0, candidates, scatXSqs, scatYSqs);
                const CbmPixelHit* p1 = segment.begin->hit;
                const CbmPixelHit* p2 = segment.end->hit;
                CbmBinnedStation::KFParams kfParams =
                {
                    { p1->GetX(), 0, p1->GetDx() * p1->GetDx(), 0, 0, 1.0 },
                    { p1->GetY(), 0, p1->GetDy() * p1->GetDy(), 0, 0, 1.0 },
                    0
                };
                TraverseTrackCandidates(0, segments, trackHolders, candidates, kfParams);
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
        
#ifdef CBM_BINNED_DEBUG
        static int nofRefTracks = 0;
        static int nofRecoRefHits[] = { 0, 0, 0, 0, 0, 0 };
        fDebug.Exec();
        CbmBinnedStation* firstStation = fStations.begin()->second;
        
        for (int i = 0; i < fDebug.fMCTracks.size(); ++i)
        {
            const CbmBinnedDebug::MCTrack& mcTrack = fDebug.fMCTracks[i];
            
            if (!mcTrack.isRef)
                continue;
            
            for (std::list<Track*>::const_iterator j = fTracks.begin(); j != fTracks.end(); ++j)
            {
                const Track* recoTrack = *j;
                const CbmTBin::HitHolder* firstHit = recoTrack->fHits[0];
                
                if (!fDebug.TrackHasHit(0, i, firstHit->index))
                    continue;
                
                ++nofRecoRefHits[0];
                ++nofRefTracks;
                
                for (int k = 1; k < 6; ++k)
                {
                    if (fDebug.TrackHasHit(k, i, recoTrack->fHits[k]->index))
                        ++nofRecoRefHits[k];
                }
            }
        }
        
        std::cout << "Reco hits: ";

        for (int j = 0; j < sizeof (nofRecoRefHits) / sizeof (int); ++j)
        {
            double proc = 100 * nofRecoRefHits[j];
            proc /= nofRefTracks;
            std::cout << "[" << proc << "%=" << nofRecoRefHits[j] << "/" << nofRefTracks << "]";
        }

        std::cout << std::endl << std::endl << std::endl;
#endif//CBM_BINNED_DEBUG
    }
    
    bool FindBestPath(int stationNo, CbmBinnedStation::KFParams kfParams, CbmTBin::HitHolder** trackHolders, CbmTBin::HitHolder** bestTrackHolders, Double_t& bestChiSq)
    {
        bool pathResult = false;
        Double_t previousZ = trackHolders[stationNo - 1]->hit->GetZ();
        CbmBinnedStation* aStation = fStationArray[stationNo];
        aStation->SearchHits(kfParams, previousZ,
            [this, &stationNo, &kfParams, &trackHolders, &bestTrackHolders, &previousZ, &bestChiSq, &pathResult](CbmTBin::HitHolder& hitHolder)->void
            {
                trackHolders[stationNo] = &hitHolder;
                const CbmPixelHit* hit = hitHolder.hit;
                CbmBinnedStation::KFParams updKFParams = kfParams;
                Double_t m[2] = { hit->GetX(), hit->GetY() };
                Double_t V[2] = { hit->GetDx() * hit->GetDx(), hit->GetDy() * hit->GetDy() };
                KFAddPoint(stationNo, updKFParams, kfParams, m, V, hit->GetZ(), previousZ);
                
                if (updKFParams.chi2 > fChiSqCut || updKFParams.chi2 > bestChiSq)
                    return;
                
                bool result = false;
                
                if (stationNo < fNofStations - 1)
                    result = FindBestPath(stationNo + 1, updKFParams, trackHolders, bestTrackHolders, bestChiSq);
                else
                {
                    bestChiSq = updKFParams.chi2;
                    result = true;
                }
                
                if (result)
                {
                    bestTrackHolders[stationNo] = &hitHolder;
                    pathResult = true;
                }
            }
        );
        return pathResult;
    }

    void FollowTracks(int startStationNo)
    {
        CbmBinnedStation* leftStation = 0 > startStationNo ? fVertexPseudoStation : fStationArray[startStationNo];
        CbmBinnedStation* rightStation = fStationArray[startStationNo + 1];
        leftStation->IterateHits(
            [this, &startStationNo, &rightStation](CbmTBin::HitHolder& leftHitHolder)->void
            {
                CbmTBin::HitHolder* trackHolders[fNofStations];
                CbmTBin::HitHolder* tmpTrackHolders[fNofStations];
                const CbmPixelHit* leftHit = leftHitHolder.hit;
                CbmBinnedStation::KFParams kfParams =
                {
                    { leftHit->GetX(), 0, leftHit->GetDx() * leftHit->GetDx(), 0, 0, 1.0 },
                    { leftHit->GetY(), 0, leftHit->GetDy() * leftHit->GetDy(), 0, 0, 1.0 },
                    0
                };
                
                CbmTBin::HitHolder* startHitHolder = 0;
                Double_t bestChiSq = cbmBinnedCrazyChiSq;
                bool hasFoundPathLeft = false;
                
                if (startStationNo >= 0)
                {
                    startHitHolder = &leftHitHolder;
                    tmpTrackHolders[0] = &leftHitHolder;
                }
                
                rightStation->IterateHits(
                    [this, &startStationNo, &leftHitHolder, &trackHolders, &tmpTrackHolders, &leftHit, &kfParams, &startHitHolder,
                        &bestChiSq, &hasFoundPathLeft](CbmTBin::HitHolder& rightHitHolder)->void
                    {                        
                        if (startStationNo < 0)
                        {
                            startHitHolder = &rightHitHolder;
                            tmpTrackHolders[0] = &rightHitHolder;
                            bestChiSq = cbmBinnedCrazyChiSq;
                        }
                        else
                            tmpTrackHolders[1] = &rightHitHolder;
                        
                        const CbmPixelHit* rightHit = rightHitHolder.hit;
                        CbmBinnedStation::KFParams updKFParams = kfParams;
                        Double_t m[2] = {rightHit->GetX(), rightHit->GetY()};
                        Double_t V[2] = {rightHit->GetDx() * rightHit->GetDx(), rightHit->GetDy() * rightHit->GetDy()};
                        KFAddPoint(startStationNo + 1, updKFParams, kfParams, m, V, rightHit->GetZ(), leftHit->GetZ());

                        if (updKFParams.chi2 > fChiSqCut)
                            return;
                        
                        bool hasFoundPathRight = FindBestPath(startStationNo + 2, updKFParams, tmpTrackHolders, trackHolders, bestChiSq);
                        
                        if (hasFoundPathRight)
                        {
                            trackHolders[0] = tmpTrackHolders[0];
                            
                            if (startStationNo >=0 )
                            {
                                trackHolders[1] = tmpTrackHolders[1];
                                hasFoundPathLeft = true;
                            }
                            
                            if (startStationNo < 0)
                            {
                                Track* aTrack = new Track(trackHolders, fNofStations, bestChiSq);
                                fTracks.push_back(aTrack);
                                
                                for (int i = 0; i < fNofStations; ++i)
                                    trackHolders[i]->used = true;
                            }
                        }
                    }
                );
                
                if (startStationNo >= 0 && hasFoundPathLeft)
                {
                    Track* aTrack = new Track(trackHolders, fNofStations, bestChiSq);
                    fTracks.push_back(aTrack);
                }
            }
        );
    }
    
private:
    //std::list<CbmBinnedStation*> fStations;
    //std::vector<CbmBinnedStation*> fStations;
    std::map<Double_t, CbmBinnedStation*> fStations;
    std::vector<CbmBinnedStation*> fStationArray;
    int fNofStations;
    int fBeforeLastLevel;
    Double_t fChiSqCut;
    std::list<Track*> fTracks;
    Double_t fBeamDx;
    Double_t fBeamDxSq;
    Double_t fBeamDy;
    Double_t fBeamDySq;
    CbmPixelHit fVertex;
    CbmBinnedStation* fVertexPseudoStation;
#ifdef CBM_BINNED_DEBUG
    CbmBinnedDebug fDebug;
#endif
    
    friend class CbmBinnedTrackerQA;
};

#endif /* TRACKER_H */
