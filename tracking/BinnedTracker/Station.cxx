/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "Tracker.h"

CbmBinnedStation::CbmBinnedStation(int nofYBins, int nofXBins, int nofTBins) : fMinZ(0), fMaxZ(0), fNofYBins(nofYBins), fNofXBins(nofXBins), fNofTBins(nofTBins),
   fYBinSize(0), fXBinSize(0), fTBinSize(500), fMinY(0), fMaxY(0), fMinX(0), fMaxX(0), fMinT(0), fMaxT(0), fDx(0), fDxSq(0), fDy(0), fDySq(0),
   fDt(0), fDtSq(0), fDefaultUse(false), fScatX(0), fScatXSq(0), fScatY(0), fScatYSq(0), fSegments(), fVertexBin(0),
   fVertexHolder({ &CbmBinnedTracker::Instance()->GetVertex(), -1, true, fVertexBin, {} }), fNofSigmasX(cbmBinnedSigma), fNofSigmasXSq(cbmBinnedSigmaSq),
   fNofSigmasY(cbmBinnedSigma), fNofSigmasYSq(cbmBinnedSigmaSq)
{
}
