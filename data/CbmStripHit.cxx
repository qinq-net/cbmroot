/**
 * \file CbmStripHit.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 2009
 **/

#include "CbmStripHit.h"

#include "TVector3.h"

#include <sstream>
using std::stringstream;
using std::endl;

CbmStripHit::CbmStripHit():
	CbmHit(),
	fU(0.),
	fDu(0.),
	fPhi(0.),
	fDphi(0.)
{
	SetType(kSTRIPHIT);
}

CbmStripHit::CbmStripHit(
		Int_t address,
		Double_t u,
		Double_t phi,
		Double_t z,
		Double_t du,
		Double_t dphi,
		Double_t dz,
		Int_t refId):
	CbmHit(),
	fU(u),
	fDu(du),
	fPhi(phi),
	fDphi(dphi)
{
	SetType(kSTRIPHIT);
   SetAddress(address);
   SetZ(z);
   SetDz(dz);
   SetRefId(refId);
}

CbmStripHit::CbmStripHit(
		Int_t address,
		const TVector3& pos,
		const TVector3& err,
		Int_t refId):
	CbmHit(),
	fU(pos.X()),
	fDu(err.X()),
	fPhi(pos.Y()),
	fDphi(err.Y())
{
	SetType(kSTRIPHIT);
   SetAddress(address);
   SetZ(pos.Z());
   SetDz(err.Z());
   SetRefId(refId);
}

CbmStripHit::~CbmStripHit()
{
}

std::string CbmStripHit::ToString() const
{
   stringstream ss;
	ss << "CbmStripHit: address=" << GetAddress()
		<< " pos=(" << GetU() << "," << GetPhi() << "," << GetZ()
		<< ") err=(" << GetDu() << "," << GetDphi() << "," << GetDz()
		<< ") refId=" << GetRefId() << endl;
	return ss.str();
}

ClassImp(CbmStripHit);
