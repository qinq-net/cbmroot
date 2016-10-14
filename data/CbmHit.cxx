/**
 * \file CbmHit.cxx
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 *
 * Former name CbmBaseHit. Renamed 11 May 2015.
 **/
#include "CbmHit.h"

CbmHit::CbmHit()
  : CbmHit(kHIT, 0., 0., -1, -1, -1., -1.)
{
}

CbmHit::CbmHit(HitType _type, Double_t _z, Double_t _dz, 
       Int_t _refId,  Int_t _address, 
       Double_t _time, Double_t _timeError)
  : TObject(),
    fType(_type),
    fZ(_z),
    fDz(_dz),
    fRefId(_refId),
    fAddress(_address),
    fTime(_time),
    fTimeError(_timeError),
    fMatch(NULL)
{
}

// Only shallow copy needed
CbmHit::CbmHit(const CbmHit& rhs)
 : TObject(rhs),
   fType(rhs.fType),
   fZ(rhs.fZ),
   fDz(rhs.fDz),
   fRefId(rhs.fRefId),
   fAddress(rhs.fAddress),
   fTime(rhs.fTime),
   fTimeError(rhs.fTimeError),
   fMatch(NULL)
{
}

// Only shallow copy needed
CbmHit& CbmHit::operator=(const CbmHit& rhs)
{

  if (this != &rhs) {
    TObject::operator=(rhs);
    fType = rhs.fType;
    fZ = rhs.fZ;
    fDz = rhs.fDz;
    fRefId = rhs.fRefId;
    fAddress = rhs.fAddress;
    fTime = rhs.fTime;
    fTimeError = rhs.fTimeError;
    fMatch = rhs.fMatch;
  }
  return *this;
}

CbmHit::~CbmHit()
{
}

ClassImp(CbmHit);
