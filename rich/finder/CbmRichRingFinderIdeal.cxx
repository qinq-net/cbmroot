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
#include "FairLogger.h"
#include "CbmMCEventList.h"
#include "CbmMCDataManager.h"
#include "CbmMCDataArray.h"

#include "TClonesArray.h"

#include <iostream>
#include <map>
#include <vector>

using namespace std;


CbmRichRingFinderIdeal::CbmRichRingFinderIdeal()
 : CbmRichRingFinder(),
   fRichPoints(NULL),
   fMcTracks(NULL),
   fRichDigis(NULL),
   fEventList(NULL)
{
    
}

CbmRichRingFinderIdeal::~CbmRichRingFinderIdeal()
{
    
}

void CbmRichRingFinderIdeal::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) {Fatal("CbmRichRingFinderIdeal::Init","RootManager is NULL!");}
    
    CbmMCDataManager* mcManager = (CbmMCDataManager*)ioman->GetObject("MCDataManager");
    if (mcManager == nullptr) LOG(FATAL) << "CbmRichRingFinderIdeal::Init() NULL MCDataManager." << FairLogger::endl;

    fMcTracks = mcManager->InitBranch("MCTrack");
    if (NULL == fMcTracks) {LOG(FATAL) << "CbmRichRingFinderIdeal::Init No MCTrack!" << FairLogger::endl;}

    fRichPoints = mcManager->InitBranch("RichPoint");
    if (NULL == fRichPoints) {LOG(FATAL) << "CbmRichRingFinderIdeal::Init No RichPoint!" << FairLogger::endl;}
    

    fEventList = (CbmMCEventList*) ioman->GetObject("MCEventList.");
    if (NULL == fEventList) {LOG(FATAL) << "CbmRichRingFinderIdeal::Init No MCEventList!" << FairLogger::endl;}

    fRichDigis  = (TClonesArray*) ioman->GetObject("RichDigi");
    if (NULL == fRichDigis) {LOG(FATAL) << "CbmRichRingFinderIdeal::Init No RichDigi!" << FairLogger::endl;}
}

Int_t CbmRichRingFinderIdeal::DoFind(
                                     TClonesArray* hitArray,
                                     TClonesArray* /*projArray*/,
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
    map<pair<Int_t, Int_t>, Int_t> hitMap;
    Int_t nofRichHits = hitArray->GetEntriesFast();
    for (Int_t iHit = 0; iHit < nofRichHits; iHit++) {
        const CbmRichHit* richHit = static_cast<CbmRichHit*>( hitArray->At(iHit) );
        if ( NULL == richHit ) continue;
        Int_t eventId = GetEventIdForRichHit(richHit);

        vector<pair<Int_t, Int_t>> motherIds = CbmMatchRecoToMC::GetMcTrackMotherIdsForRichHit(richHit, fRichDigis, fRichPoints, fMcTracks, eventId);
        for (UInt_t i = 0; i < motherIds.size(); i++) {
            hitMap[motherIds[i]]++;
        }
    }
    
    // Create STL map from MCTrack index to RichRing index
    map<pair<Int_t, Int_t>, Int_t> ringMap;
    Int_t nofRings  = 0;
    Int_t nofEvents = fEventList->GetNofEvents();
    for (Int_t iE = 0; iE < nofEvents; iE++) {
        Int_t fileId = fEventList->GetFileIdByIndex(iE);
        Int_t eventId = fEventList->GetEventIdByIndex(iE);

        // Create RichRings for McTracks
        Int_t nofMcTracks = fMcTracks->Size(fileId, eventId);
        for (Int_t iT = 0; iT < nofMcTracks; iT++) {
            const CbmMCTrack* mcTrack = static_cast<CbmMCTrack*>( fMcTracks->Get(fileId, eventId, iT) );
            if ( NULL == mcTrack ) continue;
            pair<Int_t, Int_t> val = std::make_pair(eventId, iT);
            if (hitMap[val] <= 0) continue;
            new((*ringArray)[nofRings]) CbmRichRing();
            ringMap[val] = nofRings++;
        }
    }

    // Loop over RichHits. Get corresponding MCPoint and MCTrack index
    for (Int_t iHit = 0; iHit < nofRichHits; iHit++) {
        const CbmRichHit* richHit = static_cast<CbmRichHit*>( hitArray->At(iHit) );
        if ( NULL == richHit ) continue;
        Int_t eventId = GetEventIdForRichHit(richHit);
        
        vector<pair<Int_t, Int_t> > motherIds = CbmMatchRecoToMC::GetMcTrackMotherIdsForRichHit(richHit, fRichDigis, fRichPoints, fMcTracks, eventId);
        
        for (UInt_t i = 0; i < motherIds.size(); i++) {
            if (ringMap.find(motherIds[i]) == ringMap.end()) continue;
            Int_t ringIndex = ringMap[motherIds[i]];
            CbmRichRing* ring = (CbmRichRing*) ringArray->At(ringIndex);
            if ( NULL == ring ) continue;
            
            ring->AddHit(iHit); // attach the hit to the ring
        }
    }
    
    LOG(INFO) << "-I- CbmRichRingFinderIdeal nofRings:" << nofRings << FairLogger::endl;
    
    return nofRings;
}


Int_t CbmRichRingFinderIdeal::GetEventIdForRichHit(const CbmRichHit* richHit)
{
    if (richHit == NULL) return -1;
    Int_t digiIndex = richHit->GetRefId();
    if (digiIndex < 0) return -1;
    const CbmRichDigi* digi = static_cast<const CbmRichDigi*>(fRichDigis->At(digiIndex));
    if (NULL == digi) return -1;
    CbmMatch* digiMatch = digi->GetMatch();
    if (NULL == digiMatch) return -1;
    return digiMatch->GetMatchedLink().GetEntry();
}

