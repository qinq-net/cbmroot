/**
 * \file CbmTofHit.cxx
 * \author E. Cordier
**/

#include "CbmTofHit.h"

#include <sstream>
using std::stringstream;
using std::string;
using std::endl;

CbmTofHit::CbmTofHit()
  : CbmPixelHit(),
    fFlag(1),
    fChannel(0)
{
  SetType(kTOFHIT);
  SetTime(0.);
}

CbmTofHit::CbmTofHit(Int_t address, TVector3 pos, TVector3 dpos, Int_t index, Double_t time, Int_t flag, Int_t channel)
  : CbmPixelHit(address, pos, dpos, 0., index),
    fFlag(flag),
    fChannel(channel)
{
  SetType(kTOFHIT);
  SetTime(time);
}

CbmTofHit::CbmTofHit(Int_t address, TVector3 pos, TVector3 dpos, Int_t index, Double_t time, Int_t flag)
  : CbmPixelHit(address, pos, dpos, 0., index),
    fFlag(flag),
    fChannel(0)
{
  SetType(kTOFHIT);
  SetTime(time);
}

CbmTofHit::CbmTofHit(Int_t address, TVector3 pos, TVector3 dpos, Int_t index, Double_t time)
  : CbmPixelHit(address, pos, dpos, 0., index),
    fFlag(1),
    fChannel(0)    
{
    SetType(kTOFHIT);
    SetTime(time);
}

CbmTofHit::~CbmTofHit()
{

}

string CbmTofHit::ToString() const
{
   stringstream ss;
   ss << "CbmTofHit: address=" << GetAddress()
       << " pos=(" << GetX() << "," << GetY() << "," << GetZ()
       << ") err=(" << GetDx() << "," << GetDy() << "," << GetDz()
       << ") dxy=" << GetDxy() //<< " refId=" << GetRefId()
      << Form(" time=%8.2f",GetTime()) << " flag=" << GetFlag();
     // << " channel=" << GetCh(); // << endl;
   return ss.str();
}

ClassImp(CbmTofHit)
