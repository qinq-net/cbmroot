/**
 * \file CbmRichHit.cxx
 * \author B. Polichtchouk
 **/

#include "CbmRichHit.h"
#include <sstream>
using std::stringstream;
using std::endl;

CbmRichHit::CbmRichHit()
  : CbmPixelHit(),
    fPmtId(0),
   // fNPhotons(0),
   // fAmplitude(0.),
    fTimestamp(0.),
    fToT(0.)
{
   SetType(kRICHHIT);
}

CbmRichHit::CbmRichHit(Double_t x, Double_t y)
		  : CbmPixelHit(),
		    fPmtId(0),
		  //  fNPhotons(0),
		  //  fAmplitude(0.),
		    fTimestamp(0.),
                    fToT(0.)
{
	SetType(kRICHHIT);
	SetX(x);
	SetY(y);
}

CbmRichHit::CbmRichHit(Double_t x, Double_t y, Double_t ts, Double_t tot)
		  : CbmPixelHit(),
		    fPmtId(0),
		//    fNPhotons(0),
		//    fAmplitude(0.),
		    fTimestamp(ts),
		    fToT(tot)
{
	SetType(kRICHHIT);
	SetX(x);
	SetY(y);
}

CbmRichHit::~CbmRichHit()
{
}

string CbmRichHit::ToString() const {
   stringstream ss;
   ss << "CbmRichHit: address=" << GetAddress()
       << " pos=(" << GetX() << "," << GetY() << "," << GetZ()
       << ") err=(" << GetDx() << "," << GetDy() << "," << GetDz()
       << ") dxy=" << GetDxy() << " refId=" << GetRefId()
     //  << " pmtId=" << GetPmtId() << " nofPhotons=" << GetNPhotons()
     //  << " amplitude=" << GetAmplitude()
    << endl;
   return ss.str();
}

ClassImp(CbmRichHit)
