/**
 *  $Id: CbmRichRingFinderIdeal.cxx,v 1.5 2006/02/01 13:16:58 hoehne Exp $
 *
 *  Class  : CbmRichRingFinderIdeal
 *  Description: This is the implementation of the Ideal RichRingFinder. This
 *               takes the Rich hits and associates them with the MC Track.
 *
 *  Author : S. Lebedev (2016) (Initial version Supriya Das (2005))
 *
 */
#include "CbmRichRingFinderIdeal.h"

#include "CbmRichHit.h"
#include "CbmRichRing.h"
#include "CbmRichPoint.h"
#include "CbmMCTrack.h"
#include "FairRootManager.h"
#include "CbmRichDigi.h"
#include "CbmMatch.h"
#include "CbmMatchRecoToMC.h"

#include "TClonesArray.h"

#include <iostream>
#include <map>
#include <vector>

using namespace std;


CbmRichRingFinderIdeal::CbmRichRingFinderIdeal():
fRichPoints(NULL),
fMcTracks(NULL)
{
    
}

CbmRichRingFinderIdeal::~CbmRichRingFinderIdeal()
{
    
}

void CbmRichRingFinderIdeal::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {Fatal("CbmRichRingFinderIdeal::Init","RootManager is NULL!");}
    
    fMcTracks  = (TClonesArray*) ioman->GetObject("MCTrack");
    if (NULL == fMcTracks) {Fatal("CbmRichRingFinderIdeal::Init","No MCTrack array!");}
    
    fRichPoints  = (TClonesArray*) ioman->GetObject("RichPoint");
    if (NULL == fRichPoints) {Fatal("CbmRichRingFinderIdeal::Init","No RichPoint array!");}
    
    fRichDigis  = (TClonesArray*) ioman->GetObject("RichDigi");
    if (NULL == fRichDigis) {Fatal("CbmRichRingFinderIdeal::Init","No RichDigi array!");}
}

Int_t CbmRichRingFinderIdeal::DoFind(
                                     TClonesArray* hitArray,
                                     TClonesArray* projArray,
                                     TClonesArray* ringArray)
{
    if ( NULL == hitArray) {
        cout << "-E- CbmRichRingFinderIdeal::DoFind, RichHit array missing!" << endl;
        return -1;
    }
    
    if ( NULL == ringArray ) {
        cout << "-E- CbmRichRingFinderIdeal::DoFind, Ring array missing!" << endl;
        return -1;
    }
    
    // Create STL map from MCtrack index to number of valid RichHits
    map<Int_t, Int_t> hitMap;
    Int_t nRichHits = hitArray->GetEntriesFast();
    for (Int_t iHit = 0; iHit < nRichHits; iHit++) {
        CbmRichHit* pRhit = (CbmRichHit*) hitArray->At(iHit);
        if ( NULL == pRhit ) continue;
        vector<Int_t> motherIds = CbmMatchRecoToMC::GetMcTrackMotherIdsForRichHit(pRhit, fRichDigis, fRichPoints, fMcTracks);
        for (Int_t i = 0; i < motherIds.size(); i++) {
            hitMap[motherIds[i]]++;
        }
        //cout << "motherId=" << motherId << " " << hitMap[motherId] << endl;
    }
    
    // Create STL map from MCTrack index to RichRing index
    map<Int_t, Int_t> ringMap;
    
    // Create RICHRings for MCTracks
    Int_t nRings  = 0; // number of RichRings
    Int_t nMCTracks = fMcTracks->GetEntriesFast();
    for (Int_t iMCTrack = 0; iMCTrack < nMCTracks; iMCTrack++) {
        CbmMCTrack* pMCtr = (CbmMCTrack*) fMcTracks->At(iMCTrack);
        if ( NULL == pMCtr ) continue;
        if (hitMap[iMCTrack] <= 0) continue;
        new((*ringArray)[nRings]) CbmRichRing();
        ringMap[iMCTrack] = nRings++;
        //cout << "nRings=" << nRings << endl;
    }
    
    // Loop over RichHits. Get corresponding MCPoint and MCTrack index
    for (Int_t iHit = 0; iHit < nRichHits; iHit++) {
        CbmRichHit* pRhit = (CbmRichHit*) hitArray->At(iHit);
        if ( NULL == pRhit ) continue;
        
        vector<Int_t> motherIds = CbmMatchRecoToMC::GetMcTrackMotherIdsForRichHit(pRhit, fRichDigis, fRichPoints, fMcTracks);
        
        for (Int_t i = 0; i < motherIds.size(); i++) {
            Int_t motherId = motherIds[i];
            if (motherId < 0 || motherId > nMCTracks) continue;
            
            if (ringMap.find(motherId) == ringMap.end()) continue;
            
            Int_t ringIndex = ringMap[motherId];
            //cout << ringIndex << endl;
            
            CbmRichRing* pRing = (CbmRichRing*) ringArray->At(ringIndex);
            if ( NULL == pRing ) continue;
            
            pRing->AddHit(iHit); // attach the hit to the ring
        }
    }
    
    cout << "-I- CbmRichRingFinderIdeal: all " << nMCTracks << ", rec. " << nRings << endl;
    
    return nRings;
}









