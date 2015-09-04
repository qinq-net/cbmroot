#include "CbmTrdDigi.h"

#include <sstream>
using std::endl;
using std::stringstream;

CbmTrdDigi::CbmTrdDigi() 
  : CbmDigi(),
    fn_FNR_Triggers(0),
    fAddress(-1),
    fCharge(-1.),
    fChargeTR(-1.),
    fTime(-1.),
    fTriggerType(-1),
    fInfoType(-1),
    fStopType(-1),
    fBufferOverflowCount(-1), 
    fNrSamples(-1),
    fSamples()
{
  for (Int_t i = 0; i < 45; i++)
    fSamples[i] = 0.0;
}

CbmTrdDigi::CbmTrdDigi(
      Int_t address,
      Double_t charge,
      Double_t time)
  : CbmDigi(),
    fn_FNR_Triggers(0),
    fAddress(address),
    fCharge(charge),
    fChargeTR(-1.),
    fTime(time),
    fTriggerType(-1),
    fInfoType(-1),
    fStopType(-1),
    fBufferOverflowCount(-1), 
    fNrSamples(-1),
    fSamples()
{
  for (Int_t i = 0; i < 45; i++)
    fSamples[i] = 0.0;
}

CbmTrdDigi::CbmTrdDigi(
		       Int_t address,
		       Double_t charge,
		       Double_t time, 
		       Int_t triggerType, 
		       Int_t infoType, 
		       Int_t stopType
		       )
  : CbmDigi(),
    fn_FNR_Triggers(0),
    fAddress(address),
    fCharge(charge),
    fChargeTR(-1.),
    fTime(time),
    fTriggerType(triggerType),
    fInfoType(infoType),
    fStopType(stopType),
    fBufferOverflowCount(-1), 
    fNrSamples(-1),
    fSamples()
{
  for (Int_t i = 0; i < 45; i++)
    fSamples[i] = 0.0;
}
// CbmTrdDigi used for Testbeam fles data format CbmSpadicRawMessage
CbmTrdDigi::CbmTrdDigi(Int_t address, Double_t fullTime, Int_t triggerType, Int_t infoType, Int_t stopType, Int_t nrSamples, Float_t* samples)
  : CbmDigi(),
    fn_FNR_Triggers(0),
    fAddress(address),
    fCharge(0.0),
    fChargeTR(0.),
    fTime(Double_t(fullTime)),
    fTriggerType(triggerType),
    fInfoType(infoType),
    fStopType(stopType),
    fBufferOverflowCount(-1), 
    fNrSamples(nrSamples),
    fSamples()
{
  for (Int_t i = 0; i < nrSamples; ++i) {
    fSamples[i] = samples[i];
  }
  for (Int_t i = nrSamples; i < 45; ++i) {
    fSamples[i] = 0.0;
  }
}

CbmTrdDigi::~CbmTrdDigi()
{
}

string CbmTrdDigi::ToString() const {
   stringstream ss;
   ss << "CbmTrdDigi: address=" << fAddress << " charge=" << fCharge
         << " time=" << fTime << endl;
   return ss.str();
}
void CbmTrdDigi::SetPulseShape(Float_t pulse[45]) {
  for (Int_t sample = 0; sample < 45; sample++)
    fSamples[sample] = pulse[sample];
}

ClassImp(CbmTrdDigi)
