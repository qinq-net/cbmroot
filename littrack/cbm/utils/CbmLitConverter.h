#ifndef CBMLITCONVERTER_H_
#define CBMLITCONVERTER_H_

#include "base/CbmLitTypes.h"
#include "base/CbmLitEnums.h"

#include "data/CbmLitHit.h"
#include "data/CbmLitPixelHit.h"
#include "data/CbmLitStripHit.h"
#include "data/CbmLitTrack.h"
#include "data/CbmLitTrackParam.h"
#include "data/CbmLitFitNode.h"
#include "data/CbmLitTofTrack.h"

#include "CbmLitConverterFairTrackParam.h"

#include "CbmTrack.h"
#include "CbmTofTrack.h"
#include "CbmHit.h"
#include "CbmPixelHit.h"
#include "CbmStripHit.h"
#include "FairTrackParam.h"
#include "CbmStsTrack.h"
#include "CbmTrdTrack.h"
#include "CbmMuchStrawHit.h"
#include "CbmMuchGeoScheme.h"
#include "CbmMvdHit.h"
#include "CbmMuchTrack.h"

#include "CbmGlobalTrack.h"
#include "CbmTrdAddress.h"

#include "TClonesArray.h"

#include <iostream>
#include <cmath>
#include <set>
#include <cassert>


class CbmLitConverter
{
public:
    
    static void CbmPixelHitToCbmLitPixelHit(
                                            const CbmPixelHit* hit,
                                            Int_t index,
                                            CbmLitPixelHit* litHit)
    {
        assert(hit->GetType() == kTRDHIT || hit->GetType() == kMUCHPIXELHIT || hit->GetType() == kTOFHIT);
        
        litHit->SetX(hit->GetX());
        litHit->SetY(hit->GetY());
        litHit->SetZ(hit->GetZ());
        litHit->SetDx(hit->GetDx());
        litHit->SetDy(hit->GetDy());
        litHit->SetDz(hit->GetDz());
        litHit->SetDxy(hit->GetDxy());
        litHit->SetRefId(index);
        
        if (hit->GetType() == kTRDHIT) {
            litHit->SetDetectorId(kLITTRD, hit->GetPlaneId());
        } else if (hit->GetType() == kMUCHPIXELHIT) {
            litHit->SetDetectorId(kLITMUCH, (hit->GetPlaneId() - 1) / 2);
        } else if (hit->GetType() == kTOFHIT) {
            litHit->SetDetectorId(kLITTOF, 0);
        }
    }
    
    static void CbmStripHitToCbmLitStripHit(
                                            const CbmStripHit* hit,
                                            Int_t index,
                                            CbmLitStripHit* litHit)
    {
        assert(hit->GetType() == kMUCHSTRAWHIT);
        
        litHit->SetU(hit->GetU());
        litHit->SetDu(hit->GetDu());
        litHit->SetZ(hit->GetZ());
        litHit->SetDz(hit->GetDz());
        litHit->SetPhi(hit->GetPhi());
        litHit->SetCosPhi(std::cos(litHit->GetPhi()));
        litHit->SetSinPhi(std::sin(litHit->GetPhi()));
        litHit->SetRefId(index);
        
        if (hit->GetType() == kMUCHSTRAWHIT) {
            litHit->SetSegment((static_cast<const CbmMuchStrawHit*>(hit))->GetSegment());
            Int_t address = hit->GetAddress();
            litHit->SetDetectorId(kLITMUCH, (hit->GetPlaneId() - 1) / 2);
        }
    }
    
    /*
     static void CbmHitToCbmLitPixelHit(
     const CbmHit* hit,
     Int_t index,
     CbmLitPixelHit* litHit,
     LitSystemId sysId)
     {
     assert(sysId == kLITMVD || sysId == kLITSTS);
     
     litHit->SetX(hit->GetX());
     litHit->SetY(hit->GetY());
     litHit->SetZ(hit->GetZ());
     litHit->SetDx(hit->GetDx());
     litHit->SetDy(hit->GetDy());
     litHit->SetDz(hit->GetDz());
     litHit->SetDxy(hit->GetCovXY());
     litHit->SetRefId(index);
     
     litHit->SetDetectorId(sysId, hit->GetStationNr() - 1);
     }
     */
    
    static void CbmMvdHitToCbmLitPixelHit(
                                          const CbmMvdHit* hit,
                                          Int_t index,
                                          CbmLitPixelHit* litHit)
    {
        litHit->SetX(hit->GetX());
        litHit->SetY(hit->GetY());
        litHit->SetZ(hit->GetZ());
        litHit->SetDx(hit->GetDx());
        litHit->SetDy(hit->GetDy());
        litHit->SetDz(hit->GetDz());
        litHit->SetDxy(0.);
        litHit->SetRefId(index);
        
        litHit->SetDetectorId(kLITMVD, hit->GetStationNr());
    }
    
    static void CbmStsTrackToCbmLitTrack(
                                         const CbmStsTrack* stsTrack,
                                         CbmLitTrack* litTrack)
    {
        // TODO note that hits are not copied now
        
        litTrack->SetQuality(kLITGOOD);
        litTrack->SetChi2(stsTrack->GetChiSq());
        litTrack->SetNDF(stsTrack->GetNDF());
        litTrack->SetPreviousTrackId(-1);
        CbmLitTrackParam paramFirst, paramLast;
        //TODO remove this const typecasting
        CbmLitConverterFairTrackParam::FairTrackParamToCbmLitTrackParam((const_cast<CbmStsTrack*> (stsTrack))->GetParamFirst(), &paramFirst);
        CbmLitConverterFairTrackParam::FairTrackParamToCbmLitTrackParam((const_cast<CbmStsTrack*> (stsTrack))->GetParamLast(), &paramLast);
        litTrack->SetParamFirst(&paramFirst);
        litTrack->SetParamLast(&paramLast);
    }
    
    static void CbmTrackToCbmLitTrack(
                                      const CbmTrack* track,
                                      const HitPtrVector& lhits,
                                      CbmLitTrack* ltrack)
    {
        for (Int_t iHit = 0; iHit < track->GetNofHits(); iHit++) {
            // Now we convert only pixel hits
            if (track->GetHitType(iHit) != kPIXELHIT && track->GetHitType(iHit) != kTRDHIT && track->GetHitType(iHit) != kMUCHPIXELHIT) continue;
            Int_t hitId = track->GetHitIndex(iHit);
            ltrack->AddHit(lhits[hitId]);
        }
        
        ltrack->SetQuality(kLITGOOD);
        ltrack->SetChi2(track->GetChiSq());
        ltrack->SetNDF(track->GetNDF());
        ltrack->SetPreviousTrackId(track->GetPreviousTrackId());
        ltrack->SetLastStationId(track->GetFlag());
        ltrack->SetPDG(track->GetPidHypo());
        CbmLitTrackParam paramFirst, paramLast;
        CbmLitConverterFairTrackParam::FairTrackParamToCbmLitTrackParam(track->GetParamFirst(), &paramFirst);
        CbmLitConverterFairTrackParam::FairTrackParamToCbmLitTrackParam(track->GetParamLast(), &paramLast);
        ltrack->SetParamFirst(&paramFirst);
        ltrack->SetParamLast(&paramLast);
    }
    
    static void CbmTrackArrayToCbmLitTrackArray(
                                                const TClonesArray* tracks,
                                                const HitPtrVector& lhits,
                                                TrackPtrVector& ltracks)
    {
        Int_t nofTracks = tracks->GetEntriesFast();
        for (Int_t iTrack = 0; iTrack < nofTracks; iTrack++) {
            const CbmTrack* track = static_cast<const CbmTrack*>(tracks->At(iTrack));
            CbmLitTrack* ltrack = new CbmLitTrack();
            CbmTrackToCbmLitTrack(track, lhits, ltrack);
            ltracks.push_back(ltrack);
        }
    }
    
    static void CbmLitTrackToCbmTrack(
                                      const CbmLitTrack* litTrack,
                                      CbmTrack* track,
                                      LitSystemId systemId)
    {
        Double_t chiSq = 0.;
        Int_t ndf = 0;
        Int_t firstHit = -1;
        Int_t lastHit = -1;
        for (Int_t iHit = 0; iHit < litTrack->GetNofHits(); iHit++) {
            const CbmLitHit* hit = litTrack->GetHit(iHit);
            LitHitType type = hit->GetType();
            LitSystemId det = hit->GetSystem();
            if (det != systemId) continue;
            if (firstHit < 0) firstHit = iHit;
            lastHit = iHit;
            if (det == kLITMUCH && type == kLITPIXELHIT) {
                ndf += 2;
                track->AddHit(hit->GetRefId(), kMUCHPIXELHIT);
            } else if (det == kLITMUCH && type == kLITSTRIPHIT) {
                ndf++;
                track->AddHit(hit->GetRefId(), kMUCHSTRAWHIT);
            } else if (det == kLITTRD) {
                ndf += 2;
                track->AddHit(hit->GetRefId(), kTRDHIT);
            }
            chiSq += litTrack->GetFitNode(iHit)->GetChiSqFiltered();
        }
        ndf -= 5;
        if (ndf <= 0) ndf = 1;
        
        track->SetChiSq(chiSq);
        track->SetNDF(ndf);
        track->SetPreviousTrackId(litTrack->GetPreviousTrackId());
        track->SetFlag(litTrack->GetQuality());
        FairTrackParam parLast, parFirst;
        CbmLitConverterFairTrackParam::CbmLitTrackParamToFairTrackParam(litTrack->GetFitNode(lastHit)->GetUpdatedParam(), &parLast);
        CbmLitConverterFairTrackParam::CbmLitTrackParamToFairTrackParam(litTrack->GetFitNode(firstHit)->GetUpdatedParam(), &parFirst);
        track->SetParamLast(&parLast);
        track->SetParamFirst(&parFirst);
    }
    
    static void LitTrackVectorToGlobalTrackArray(
                                                 const TrackPtrVector& litTracks,
                                                 const TofTrackPtrVector& litTofTracks,
                                                 TClonesArray* globalTracks,
                                                 TClonesArray* stsTracks,
                                                 TClonesArray* trdTracks,
                                                 TClonesArray* muchTracks,
                                                 TClonesArray* tofTracks)
    {
        // Loop over STS tracks and create GlobalTrack for each StsTrack
        Int_t globalTrackNo = globalTracks->GetEntriesFast();
        Int_t nofStsTracks = stsTracks->GetEntriesFast();
        for (Int_t iTrack = 0; iTrack < nofStsTracks; iTrack++) {
            CbmGlobalTrack* globalTrack = new ((*globalTracks)[globalTrackNo++]) CbmGlobalTrack();
            globalTrack->SetStsTrackIndex(iTrack);
        }
        
        Int_t trdTrackNo = (trdTracks != NULL) ? trdTracks->GetEntriesFast() : 0;
        Int_t muchTrackNo = (muchTracks != NULL) ? muchTracks->GetEntriesFast() : 0;
        Int_t tofTrackNo = (tofTracks != NULL) ? tofTracks->GetEntriesFast() : 0;
        
        for (Int_t iTrack = 0; iTrack < litTracks.size(); iTrack++) {
            const CbmLitTrack* litTrack = litTracks[iTrack];
            
            if (litTrack->GetQuality() == kLITBAD) { continue; }
            if (litTrack->GetNofHits() < 1) { continue; }
            if (litTrack->GetNofHits() != litTrack->GetFitNodes().size()) {
                LOG(ERROR) << "CbmLitConverter::LitTrackVectorToGlobalTrackArray: unequal number of hits and fit nodes" << std::endl
                << litTrack->ToString();
                continue;
            }
            
            CbmGlobalTrack* globalTrack = static_cast<CbmGlobalTrack*>(globalTracks->At(litTrack->GetPreviousTrackId()));
            
            // Set last parameter of the CbmGlobal track to be last parameter of CbmLitTrack
            FairTrackParam parLast;
            CbmLitConverterFairTrackParam::CbmLitTrackParamToFairTrackParam(litTrack->GetParamLast(), &parLast);
            globalTrack->SetParamLast(&parLast);
            
            Bool_t isCreateMuchTrack = false, isCreateTrdTrack = false;
            for (Int_t iHit = 0; iHit < litTrack->GetNofHits(); iHit++) {
                const CbmLitHit* thisHit = litTrack->GetHit(iHit);
                LitSystemId thisDetId = thisHit->GetSystem();
                if (thisDetId == kLITMUCH && muchTracks != NULL) { isCreateMuchTrack = true; }
                if (thisDetId == kLITTRD && trdTracks != NULL) { isCreateTrdTrack = true; }
            }
            if (isCreateTrdTrack) {
                CbmTrdTrack* track = new ((*trdTracks)[trdTrackNo++]) CbmTrdTrack();
                CbmLitTrackToCbmTrack(litTrack, track, kLITTRD);
                globalTrack->SetTrdTrackIndex(trdTrackNo - 1);
            }
            if (isCreateMuchTrack) {
                CbmMuchTrack* track = new ((*muchTracks)[muchTrackNo++]) CbmMuchTrack();
                CbmLitTrackToCbmTrack(litTrack, track, kLITMUCH);
                globalTrack->SetMuchTrackIndex(muchTrackNo - 1);
            }
        }
        
        for (Int_t iTrack = 0; iTrack < litTofTracks.size(); iTrack++) {
            const CbmLitTofTrack* litTofTrack = litTofTracks[iTrack];
            CbmTofTrack* track = new ((*tofTracks)[tofTrackNo++]) CbmTofTrack();
            Int_t globalTrackId = litTofTrack->GetTrack()->GetPreviousTrackId();
            Int_t tofHitId = litTofTrack->GetHit()->GetRefId();
            track->SetTofHitIndex(tofHitId);
            track->SetTrackIndex(globalTrackId);
            track->SetDistance(litTofTrack->GetDistance());
            FairTrackParam par;
            CbmLitConverterFairTrackParam::CbmLitTrackParamToFairTrackParam(litTofTrack->GetTrackParam(), &par);
            track->SetTrackParameter(&par);
            
            CbmGlobalTrack* globalTrack = static_cast<CbmGlobalTrack*>(globalTracks->At(globalTrackId));
            globalTrack->SetTofHitIndex(tofHitId);
        }
    }
    
    static void HitArrayToHitVector(
                                    const TClonesArray* hits,
                                    HitPtrVector& litHits)
    {
        Int_t nofHits = hits->GetEntriesFast();
        for(Int_t iHit = 0; iHit < nofHits; iHit++) {
            CbmHit* hit = (CbmHit*) hits->At(iHit);
            if(NULL == hit) { continue; }
            if (hit->GetType() == kMUCHSTRAWHIT) {
                CbmLitStripHit* litHit = new CbmLitStripHit();
                CbmStripHit* stripHit = static_cast<CbmStripHit*>(hit);
                CbmStripHitToCbmLitStripHit(stripHit, iHit, litHit);
                litHits.push_back(litHit);
            } else {
                CbmLitPixelHit* litHit = new CbmLitPixelHit();
                CbmPixelHit* pixelHit = static_cast<CbmPixelHit*>(hit);
                CbmPixelHitToCbmLitPixelHit(pixelHit, iHit, litHit);
                litHits.push_back(litHit);
            }
        }
    }
    
    static void MvdHitArrayToHitVector(
                                       const TClonesArray* hits,
                                       HitPtrVector& litHits)
    {
        Int_t nofHits = hits->GetEntriesFast();
        for(Int_t iHit = 0; iHit < nofHits; iHit++) {
            CbmMvdHit* hit = static_cast<CbmMvdHit*>(hits->At(iHit));
            if(NULL == hit) { continue; }
            CbmLitPixelHit* litHit = new CbmLitPixelHit();
            CbmMvdHitToCbmLitPixelHit(hit, iHit, litHit);
            litHits.push_back(litHit);
        }
    }
    
    static void StsTrackArrayToTrackVector(
                                           const TClonesArray* tracks,
                                           TrackPtrVector& litTracks)
    {
        Int_t nofTracks = tracks->GetEntriesFast();
        for(Int_t iTrack = 0; iTrack < nofTracks; iTrack++) {
            CbmStsTrack* track = (CbmStsTrack*) tracks->At(iTrack);
            if (track == NULL) { continue; }
            if (track->GetParamLast()->GetQp() == 0) { continue; }
            CbmLitTrack* litTrack = new CbmLitTrack;
            CbmStsTrackToCbmLitTrack(track, litTrack);
            litTrack->SetPreviousTrackId(iTrack);
            litTrack->SetRefId(iTrack);
            litTracks.push_back(litTrack);
        }
    }
};

#endif /*CBMLITCONVERTER_H_*/
