/**
 ** \file CbmStsHit.cxx
 ** \author Volker Friese <v.friese@gsi.de>
 ** \since 30.08.06
 **
 ** Updated 14/03/2014 by Andrey Lebedev <andrey.lebedev@gsi.de>.
 ** Updated 10/06/2015 by Volker Friese <v.friese@gsi.de>.
 **/

#include "CbmStsHit.h"

#include <iomanip>
#include <sstream>

using namespace std;

// -----   Default constructor
CbmStsHit::CbmStsHit()
    : CbmPixelHit()
    , fFrontClusterId(-1)
    , fBackClusterId(-1)
    , fDu(-1.)
    , fDv(-1.)
{
	SetTime(-1.);
	SetTimeError(-1.);
}



// -----   Constructor with parameters
CbmStsHit::CbmStsHit(Int_t address, const TVector3& pos, const TVector3& dpos,
		                 Double_t dxy, Int_t frontClusterId, Int_t backClusterId,
		                 Double_t time, Double_t timeError, Double_t du,
		                 Double_t dv)
    : CbmPixelHit(address, pos, dpos, dxy, -1)
    , fFrontClusterId(frontClusterId)
    , fBackClusterId(backClusterId)
    , fDu(du)
    , fDv(dv)
{
	SetTime(time);
	SetTimeError(timeError);
}



// -----   Destructor
CbmStsHit::~CbmStsHit()
{
}


// --- String output
string CbmStsHit::ToString() const {
   stringstream ss;
   ss << "StsHit: address " << GetAddress() << " | time " << GetTime()
      << " +- " << GetTimeError()
      << " | Position (" << std::setprecision(6) << GetX() << ", " << GetY()
	  << ", " << GetZ() << ") cm | Error (" << GetDx() << ", " << GetDy()
	  << ", " << GetDz() << ") cm";
   return ss.str();
}


ClassImp(CbmStsHit)
