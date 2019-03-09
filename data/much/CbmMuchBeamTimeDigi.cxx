// TODO comment to be changed
/** CbmMuchBeamTimeDigi.cxx
 **@author M.Ryzhinskiy <m.ryzhinskiy@gsi.de>
 **@since 19.03.07
 **@version 1.0
 **@author Vikas Singhal <vikas@vecc.gov.in>
 **@since 06.03.19
 **@version 2.0
 **
 ** Data class for digital MUCH information collected during BeamTime
 ** Data level: RAW
 ** To use reconstruction classes for CbmMuchBeamTimeDigi deriving it from CbmMuchDigi. VS
 **
 **/
#include "CbmMuchBeamTimeDigi.h"
#include "CbmMuchAddress.h"
#include "CbmMuchDigiMatch.h"

#include <memory>

// -------------------------------------------------------------------------
const Int_t CbmMuchBeamTimeDigi::fgkCharBits = 12;
const Int_t CbmMuchBeamTimeDigi::fgkTimeBits = 14;
const Int_t CbmMuchBeamTimeDigi::fgkAddrBits = 32;
// -------------------------------------------------------------------------

// -----   Bit shift for charge and time stamp   ---------------------------
const Int_t CbmMuchBeamTimeDigi::fgkAddrShift = 0;
const Int_t CbmMuchBeamTimeDigi::fgkCharShift = CbmMuchBeamTimeDigi::fgkAddrShift + CbmMuchBeamTimeDigi::fgkAddrBits;
const Int_t CbmMuchBeamTimeDigi::fgkTimeShift = CbmMuchBeamTimeDigi::fgkCharShift + CbmMuchBeamTimeDigi::fgkCharBits;
// -------------------------------------------------------------------------

// -----   Bit masks for charge and time stamp   ---------------------------
const Long64_t CbmMuchBeamTimeDigi::fgkAddrMask = (1LL << CbmMuchBeamTimeDigi::fgkAddrBits) - 1;
const Long64_t CbmMuchBeamTimeDigi::fgkCharMask = (1LL << CbmMuchBeamTimeDigi::fgkCharBits) - 1;
const Long64_t CbmMuchBeamTimeDigi::fgkTimeMask = (1LL << CbmMuchBeamTimeDigi::fgkTimeBits) - 1;
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
CbmMuchBeamTimeDigi::CbmMuchBeamTimeDigi() 
  : CbmMuchDigi(),
    fData(0),
    fTime(0),
    fMatch(0),
    fPadX(-1), fPadY(-1), fRocId(-1), fNxId(-1), fNxCh(-1)
{
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
CbmMuchBeamTimeDigi::CbmMuchBeamTimeDigi(Int_t address, Int_t charge, ULong64_t time)
  : CbmMuchDigi(),
    fData(0),
    fTime(time),
    fMatch(0),
    fPadX(-1), fPadY(-1), fRocId(-1), fNxId(-1), fNxCh(-1)
{
  fData |= (address & fgkAddrMask) << fgkAddrShift;
  fData |= (charge  & fgkCharMask) << fgkCharShift;
  fData |= (time    & fgkTimeMask) << fgkTimeShift;
}
// -------------------------------------------------------------------------

CbmMuchBeamTimeDigi::CbmMuchBeamTimeDigi(CbmMuchBeamTimeDigi* digi)
  : CbmMuchDigi(*digi),
    fData(digi->fData),
    fTime(digi->fTime),
    fMatch(0),
    fPadX(digi->GetPadX()),
    fPadY(digi->GetPadY()),
    fRocId(digi->GetRocId()),
    fNxId(digi->GetNxId()),
    fNxCh(digi->GetNxCh())
{ 
}

CbmMuchBeamTimeDigi::CbmMuchBeamTimeDigi(CbmMuchBeamTimeDigi* digi,CbmMuchDigiMatch* match)
  : CbmMuchDigi(*digi),
    fData(digi->fData),
    fTime(digi->fTime),
    //fMatch(new CbmMuchDigiMatch(match)),
		fPadX(digi->GetPadX()),
		fPadY(digi->GetPadY()),
		fRocId(digi->GetRocId()),
		fNxId(digi->GetNxId()),
		fNxCh(digi->GetNxCh())
{  
		SetMatch(match);
}


CbmMuchBeamTimeDigi::CbmMuchBeamTimeDigi(const CbmMuchBeamTimeDigi& rhs)
 : CbmMuchDigi(rhs),
   fData(rhs.fData),
   fTime(rhs.fTime),
   //fMatch(NULL),
   fMatch(rhs.fMatch),
   fPadX(rhs.fPadX),
   fPadY(rhs.fPadY),
   fRocId(rhs.fRocId),
   fNxId(rhs.fNxId),
   fNxCh(rhs.fNxCh)
{
//  if (NULL != rhs.fMatch) {
//     fMatch = new CbmMuchDigiMatch(*(rhs.fMatch));
//   }
}

CbmMuchBeamTimeDigi& CbmMuchBeamTimeDigi::operator=(const CbmMuchBeamTimeDigi& rhs)
{

  if (this != &rhs) {
    CbmMuchDigi::operator=(rhs);
    fData = rhs.fData;
    fTime = rhs.fTime;
    if (NULL != rhs.fMatch) {
      std::unique_ptr<CbmMuchDigiMatch> tmp(new CbmMuchDigiMatch(*rhs.fMatch));
      delete fMatch;
      fMatch = tmp.release();
    } else {

      fMatch = NULL;
    }
    fPadX = rhs.fPadX;
    fPadY = rhs.fPadY;
    fRocId = rhs.fRocId;
    fNxId = rhs.fNxId;
    fNxCh = rhs.fNxCh;
  }
  return *this;
}


// -----   Add charge   ----------------------------------------------------
void CbmMuchBeamTimeDigi::AddAdc(Int_t adc) {
  Int_t newAdc = GetAdc() + adc;
  if (newAdc > fgkCharMask) newAdc = fgkCharMask;
  SetAdc(newAdc);
}
// -------------------------------------------------------------------------


// -----   Set new charge   ------------------------------------------------
void CbmMuchBeamTimeDigi::SetAdc(Int_t adc) {
  // reset to 0
  fData &= ~(fgkCharMask << fgkCharShift);
  // set new value
  fData |=  (adc & fgkCharMask) << fgkCharShift;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
void CbmMuchBeamTimeDigi::SetTime(ULong64_t time) {
  // reset to 0
  fData &= ~(fgkTimeMask << fgkTimeShift);
  // set new value
  fData |= (time    & fgkTimeMask) << fgkTimeShift; 
  fTime = time;
}
// -------------------------------------------------------------------------



ClassImp(CbmMuchBeamTimeDigi)
