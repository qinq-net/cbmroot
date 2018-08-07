#include "CbmTrdDigi.h"
#include "CbmTrdAddress.h"

#include <FairLogger.h>

#include <sstream>
#include <iomanip>
using std::endl;
using std::stringstream;
using std::string;

/**
 * fAddress defition ATTf.ffnn nnnL.LLMM MMMM.pppp pppp.pppp
 * A - Asic type according to CbmTrdAsicType
 * T - trigger type according to CbmTrdTriggerType
 * f - flags according to CbmTrdDigiDef
 * n - error class
 * L - layer id in the TRD setup
 * M - module id in the layer
 * p - pad address within the module
 */
Float_t CbmTrdDigi::fgClk[] = {62.5, 12.5};
Float_t CbmTrdDigi::fgPrecission[] = {1.e3, 1.e1};
//__________________________________________________________________________________________
CbmTrdDigi::CbmTrdDigi()
  : CbmDigi()
  ,fAddress(0)
  ,fCharge(0)
  ,fTime(0)
{  
}
//__________________________________________________________________________________________
CbmTrdDigi::CbmTrdDigi(Int_t address, Float_t chargeT, Float_t chargeR, ULong64_t time)
  : CbmDigi()
  ,fAddress(0)
  ,fCharge(0.)
  ,fTime(time)
{  
/** Fill data structure according to FASP representation  
 * A - Asic type according to CbmTrdAsicType
 * M - module id in the layer
 * L - layer id in the TRD setup
 * p - pad address within the module
 * 
 * fCharge definition tttt.tttt tttt.tttt rrrr.rrrr rrrr.rrrr
 * t - tilt paired charge
 * r - rectangle paired charge
 */
  SetAsic(kFASP);
  SetChannel(address); 
  SetCharge(chargeT, chargeR);
}

//__________________________________________________________________________________________
CbmTrdDigi::CbmTrdDigi(Int_t address, Float_t charge, ULong64_t time, Int_t triggerType, Int_t errClass)
  : CbmDigi()
  ,fAddress(0)
  ,fCharge(0.)
  ,fTime(time)
{
/**
 * Fill data structure according to SPADIC representation  
 * A - Asic type according to CbmTrdAsicType
 * T - trigger type according to CbmTrdTriggerType
 * n - error class
 * M - module id in the layer
 * L - layer id in the TRD setup
 * p - pad address within the module
 * fCharge definition UInt_t(charge*fgPrecission)
*/
  SetAsic(kSPADIC);
  SetChannel(address); 
  SetCharge(charge);
  SetTriggerType(triggerType);
  SetErrorClass(errClass);
}

//__________________________________________________________________________________________
void CbmTrdDigi::AddCharge(CbmTrdDigi *sd, Double_t f)
{
  if(GetType()!=kFASP){
    LOG(WARNING)<<"CbmTrdDigi::AddCharge(CbmTrdDigi*, Double_t) : Only available for FASP. Use AddCharge(Double_t, Double_t) instead."<<FairLogger::endl;
    return;
  } 
  UInt_t  t = ((fCharge&0xffff0000)>>16),
          r = (fCharge&0xffff),
          ts= ((sd->fCharge&0xffff0000)>>16),
          rs= (sd->fCharge&0xffff);
  // apply correction factor to charge        
  Float_t tsf = f*ts/fgPrecission[kFASP], rsf = f*rs/fgPrecission[kFASP];
  ts = tsf*fgPrecission[kFASP]; rs = rsf*fgPrecission[kFASP]; 
  
  if(t+ts<0xffff) t+=ts;
  else t = 0xffff;
  if(r+rs<0xffff) r+=rs;
  else r = 0xffff;
  fCharge = r|(t<<16);
}

//__________________________________________________________________________________________
void CbmTrdDigi::AddCharge(Double_t c, Double_t f)
{
  if(GetType()!=kSPADIC){
    LOG(WARNING)<<"CbmTrdDigi::AddCharge(Double_t, Double_t) : Only available for SPADIC. Use AddCharge(CbmTrdDigi*, Double_t) instead."<<FairLogger::endl;
    return;
  } 
  SetCharge(GetCharge()+f*c);
}

//__________________________________________________________________________________________
Int_t CbmTrdDigi::GetAddressChannel() const
{
/**  Returns index of the read-out unit in the module in the format row x ncol + col
 */
  return (fAddress>>fgkRoOffset)&0xfff;
}

//__________________________________________________________________________________________
Int_t CbmTrdDigi::GetAddressModule() const
{
/**  Convert internal representation of module address to CBM address as defined in CbmTrdAddress
 */
  return CbmTrdAddress::GetAddress(Layer(), Module(), 0, 0, 0);
}

//__________________________________________________________________________________________
Double_t CbmTrdDigi::GetCharge()  const
{
  if(GetType()!=kSPADIC){
    LOG(WARNING)<<"CbmTrdDigi::GetCharge() : Use Double_t GetCharge(Double_t &tilt) instead."<<FairLogger::endl;
    return 0;
  } 
  return fCharge/fgPrecission[kSPADIC];
}

//__________________________________________________________________________________________
Double_t CbmTrdDigi::GetCharge(Double_t &tilt)  const
{
  if(GetType()!=kFASP){
    LOG(WARNING)<<"CbmTrdDigi::GetCharge(Double_t &) : Use Double_t GetCharge() instead."<<FairLogger::endl;
    return 0;
  } 
  
  tilt = ((fCharge&0xffff0000)>>16)/fgPrecission[kFASP];
  return (fCharge&0xffff)/fgPrecission[kFASP];
}

//__________________________________________________________________________________________
Double_t CbmTrdDigi::GetChargeError()  const
{
  return 0;
}

//__________________________________________________________________________________________
Bool_t CbmTrdDigi::IsFlagged(const Int_t iflag)  const
{
  if(iflag<0||iflag>=kNflags) return kFALSE;
  return (fAddress>>(fgkFlgOffset+iflag))&0x1;
}

//__________________________________________________________________________________________
void CbmTrdDigi::SetAddress(Int_t address)
{
  SetLayer(CbmTrdAddress::GetLayerId(address)); 
  SetModule(CbmTrdAddress::GetModuleId(address));
}

//__________________________________________________________________________________________
void CbmTrdDigi::SetAsic(CbmTrdAsicType ty)
{ 
  if(ty==kSPADIC) CLRBIT(fAddress, fgkTypOffset);
  else  SETBIT(fAddress, fgkTypOffset);
}

//__________________________________________________________________________________________
void CbmTrdDigi::SetCharge(Float_t cT, Float_t cR)   
{ 
  UInt_t r=UInt_t(cR*fgPrecission[kFASP]), t=UInt_t(cT*fgPrecission[kFASP]);
  if(r>0xffff) r=0xffff;
  if(t>0xffff) t=0xffff;
  fCharge = r|(t<<16); 
}

//__________________________________________________________________________________________
void CbmTrdDigi::SetCharge(Float_t c)   
{ 

  //  printf("SetCharge :: prec[%f] c[%f] uint[%d] \n", fgPrecission[kSPADIC],c,UInt_t(c*fgPrecission[kSPADIC]));
  //  std::cout<<" setcharge: "<< UInt_t(c*fgPrecission[kSPADIC])<<std::endl;
  fCharge = UInt_t(c*fgPrecission[kSPADIC]);
  //  fCharge = UInt_t(c*1e9);
}

//__________________________________________________________________________________________
void CbmTrdDigi::SetFlag(const Int_t iflag, Bool_t set)
{
  if(iflag<0||iflag>=kNflags) return;
  if(set) SETBIT(fAddress, fgkFlgOffset+iflag);
  else CLRBIT(fAddress, fgkFlgOffset+iflag);
}

//__________________________________________________________________________________________
void CbmTrdDigi::SetTriggerType(const Int_t ttype)
{
  if(GetType()==kFASP) return;
  if(ttype<0||ttype>=kNTrg) return;
  fAddress|=(ttype<<fgkTrgOffset);
}

//__________________________________________________________________________________________
string CbmTrdDigi::ToString() const 
{
  stringstream ss;
  ss << "CbmTrdDigi("<<(GetType()==kFASP?"T)":"R)")<<" | moduleAddress=" << GetAddressModule() <<" | layer=" << Layer() <<" | moduleId=" << Module() <<" | pad=" << GetAddressChannel() << " | time[ns]=" <<std::fixed<< std::setprecision(1)<< GetTime();
  if(GetType()==kFASP) {
    Double_t t, r = GetCharge(t);
    ss<<" | pu="<<(IsPileUp()?"y":"n")
      <<" | mask="<<(IsMasked()?"y":"n")
      <<" |charge="<<std::fixed<<std::setw(6)<<std::setprecision(1)<<r<<"/"<<t;
  } else {
    ss<< " | charge=" << GetCharge()
      << " TriggerType=" << GetTriggerType()
      << " ErrorClass=" << GetErrorClass();
  }
  ss<< endl;
  return ss.str();
}

ClassImp(CbmTrdDigi)
