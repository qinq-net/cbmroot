/**
 ** \file CbmStsTrack.h
 ** \author V.Friese <v.friese@gsi.de>
 ** \since 26.01.05
 ** \date 07.09.15
 **/

#include "CbmStsTrack.h"

#include <sstream>

using std::stringstream;


// -----   Constructor   ---------------------------------------------------
CbmStsTrack::CbmStsTrack()
  : CbmTrack(),
    fMvdHitIndex(),
    fB(0.)
{
}
// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmStsTrack::~CbmStsTrack() {
}
// -------------------------------------------------------------------------



// -----   Debug   ---------------------------------------------------------
string CbmStsTrack::ToString() const {
   stringstream ss;
   ss << "CbmStsTrack: nof STS hits=" << GetNofHits() << ", nof MVD hits="
         << GetNofMvdHits() << "chiSq=" << GetChiSq() << ", NDF="
         << GetNDF() << ", pidHypo=" << GetPidHypo() << ", previousTrackId="
         << GetPreviousTrackId() << ", flag=" << GetFlag() << ", B="
         << GetB() << "\n";
   return ss.str();
}
// -------------------------------------------------------------------------


ClassImp(CbmStsTrack)
