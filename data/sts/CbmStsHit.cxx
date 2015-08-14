/**
 ** \file CbmStsHit.cxx
 ** \author Volker Friese <v.friese@gsi.de>
 ** \since 30.08.06
 **
 ** Updated 14/03/2014 by Andrey Lebedev <andrey.lebedev@gsi.de>.
 ** Updated 10/06/2015 by Volker Friese <v.friese@gsi.de>.
 **/

#include "CbmStsHit.h"


// -----   Default constructor
CbmStsHit::CbmStsHit()
    : CbmPixelHit()
    , fFrontClusterId(-1)
    , fBackClusterId(-1)
    , fTime(0.)
    , fTimeError(0.)
{
}



// -----   Constructor with parameters
CbmStsHit::CbmStsHit(Int_t address, const TVector3& pos, const TVector3& dpos,
		                 Double_t dxy, Int_t frontClusterId, Int_t backClusterId,
		                 Double_t time, Double_t timeError)
    : CbmPixelHit(address, pos, dpos, dxy, -1)
    , fFrontClusterId(frontClusterId)
    , fBackClusterId(backClusterId)
    , fTime(time)
    , fTimeError(timeError)
{
}



// -----   Destructor
CbmStsHit::~CbmStsHit()
{
}


ClassImp(CbmStsHit)
