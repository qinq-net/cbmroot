#include "CbmTrdDigi.h"

#include <sstream>
using std::endl;
using std::stringstream;
using std::string;

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

Int_t CbmTrdDigi::Compare(const TObject *obj) const{
  /** Override TObject::Compare to enable sorting a TClonesArray of CbmTrdDigis.
   *  Sorts CbmTrdDigis with their Timestamp, in spadic granularity,
   *  as the major key. CbmTrdDigis will then be sorted according to their 
   *  LayerID, ModuleID, SectorID, RowID and ColumnID, in that order.
   *  Digis on adjacent columns will be sorted next to each other.
   *  The division by 62.5 in myTime and otherTime results from the sampling frequency of 
   *  SPADICv2.0 it should be sufficient for time sampling of all other SPADIC prototype
   *  frequencies.
   **/
  const CbmTrdDigi*ptr=static_cast<const CbmTrdDigi*>(obj);
  if (!ptr) return -1;
  Long64_t myTime=static_cast<Long64_t>((this->GetTime()+1.0)/62.5);
  Long64_t otherTime=static_cast<Long64_t>((ptr->GetTime()+1.0)/62.5);
  if(myTime<otherTime) return -1;
  if(myTime>otherTime) return  1;
  Int_t otherAddress=ptr->GetAddress();
  std::vector<Int_t> rawAdresses={{fAddress,otherAddress}},sortAdresses(2);
  for (UInt_t i=0;i<2;i++){
    sortAdresses.at(i)+=CbmTrdAddress::GetColumnId(rawAdresses.at(i));
    sortAdresses.at(i)+=CbmTrdAddress::GetRowId(rawAdresses.at(i))*256;
    sortAdresses.at(i)+=CbmTrdAddress::GetSectorId(rawAdresses.at(i))*128*256;
    sortAdresses.at(i)+=CbmTrdAddress::GetModuleId(rawAdresses.at(i))*4*128*256;
    sortAdresses.at(i)+=CbmTrdAddress::GetLayerId(rawAdresses.at(i))*128*4*128*256;
  }
  //sortAdresses.at(0) is the local Address, sortAdresses.at(1) is the foreign Address
  //if(sortAdresses.at(0)<sortAdresses.at(1))
  //  return -1;
  if(sortAdresses.at(0)==sortAdresses.at(1))
    return 0;
  if(sortAdresses.at(0)>sortAdresses.at(1))
    return 1;
  return -1;
}

void CbmTrdDigi::SetPulseShape(Float_t pulse[45]) {
  for (Int_t sample = 0; sample < 45; sample++)
    fSamples[sample] = pulse[sample];
}

ClassImp(CbmTrdDigi)
