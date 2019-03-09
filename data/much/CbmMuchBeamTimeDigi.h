// TODO comment to be changed
/** CbmMuchBeamTimeDigi.h
 **@author M.Ryzhinskiy <m.ryzhinskiy@gsi.de>
 **@since 19.03.07
 **@version 1.0
 **
 **@author Vikas Singhal <vikas@vecc.gov.in>
 **@since 06.03.19
 **@version 2.0
 **
 ** Data class for digital MUCH information collected during BeamTime
 ** Data level: RAW
 ** To use reconstruction classes for CbmMuchBeamTimeDigi deriving it from CbmMuchDigi. VS
 **
 **
 **/


#ifndef CBMMUCHBEAMTIMEDIGI_H
#define CBMMUCHBEAMTIMEDIGI_H 1

#include "CbmMuchDigi.h"
#include "CbmDefs.h"
#include "CbmMuchDigiMatch.h"
#include "CbmMuchAddress.h"

class CbmMuchBeamTimeDigi : public CbmMuchDigi{
 public:
  CbmMuchBeamTimeDigi();
  CbmMuchBeamTimeDigi(Int_t address, Int_t charge=0, ULong64_t time=0);
  CbmMuchBeamTimeDigi(CbmMuchBeamTimeDigi* digi);
  CbmMuchBeamTimeDigi(CbmMuchBeamTimeDigi* digi,CbmMuchDigiMatch* match);
  CbmMuchBeamTimeDigi(const CbmMuchBeamTimeDigi&);
  CbmMuchBeamTimeDigi& operator=(const CbmMuchBeamTimeDigi&);

  
  virtual ~CbmMuchBeamTimeDigi(){}
  Int_t GetSystemId() const { return kMuch; }
  Int_t GetAddress()  const { return (fData >> fgkAddrShift) & fgkAddrMask; }
  Int_t GetAdc()      const { return (fData >> fgkCharShift) & fgkCharMask; }
  //Double_t GetTime()  const { return (fData >> fgkTimeShift) & fgkTimeMask; }
  virtual Double_t GetTime() const { return Double_t(fTime); }
  
  void AddAdc(Int_t adc);
  void SetAdc(Int_t adc);
  void SetTime(ULong64_t time);
  // Deriving from CbmMuchDigi so not needed. VS (060319)
  //void SetMatch(CbmMuchDigiMatch* match) { fMatch = match; }
  //CbmMuchDigiMatch* GetMatch() const { return fMatch; }
 
  // Specially for littrack
  // TODO remove after littrack fix
  Int_t GetDetectorId() const { return CbmMuchAddress::GetElementAddress(GetAddress(),kMuchModule); }
  Int_t GetChannelId()  const { return GetAddress(); }
  Int_t GetADCCharge()  const { return GetAdc(); }
  Int_t GetDTime() const { return 0; }

  void SetPadX(Int_t padX) { fPadX = padX; }
  void SetPadY(Int_t padY) { fPadY = padY; }
  void SetRocId(Int_t rocId) { fRocId = rocId; }
  void SetNxId(Int_t nxId) { fNxId = nxId; }
  void SetNxCh(Int_t nxCh) { fNxCh = nxCh; }

  Int_t GetPadX() const {return fPadX; }
  Int_t GetPadY() const {return fPadY; }
  Int_t GetRocId() const {return fRocId; }
  Int_t GetNxId() const {return fNxId; }
  Int_t GetNxCh() const {return fNxCh; }


  std::string ToString() const { return std::string{""}; }
 
 private:
  Long64_t fData;
  ULong64_t fTime;
  CbmMuchDigiMatch* fMatch;  ///< matches to MC points (to be replaced with Fair links)
  Int_t fPadX;
  Int_t fPadY;
  Int_t fRocId;
  Int_t fNxId;
  Int_t fNxCh;
  
  static const Int_t fgkAddrBits;
  static const Int_t fgkCharBits;
  static const Int_t fgkTimeBits;
  static const Int_t fgkAddrShift;
  static const Int_t fgkCharShift;
  static const Int_t fgkTimeShift;
  static const Long64_t fgkAddrMask;
  static const Long64_t fgkCharMask;
  static const Long64_t fgkTimeMask;

  ClassDef(CbmMuchBeamTimeDigi,2);
};
#endif
