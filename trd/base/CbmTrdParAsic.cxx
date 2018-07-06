#include "CbmTrdParAsic.h"

#include <FairLogger.h>
#include <TMath.h>


//___________________________________________________________________
CbmTrdParAsic::CbmTrdParAsic(Int_t address, Int_t FebGrouping, 
           Double_t x, Double_t y, Double_t z)
  : CbmTrdParMod("CbmTrdParAsic", "TRD ASIC definition")
  ,fAddress(address)
  ,fX(x)
  ,fY(y)
  ,fZ(z)
  ,fFebGrouping(FebGrouping)
  ,fChannelAddresses()
{

}

//___________________________________________________________________
void CbmTrdParAsic::Print(Option_t* opt) const
{
  printf("%s @ %d pos[%5.2f %5.2f]\n", (opt?opt:GetName()), fAddress, fX, fY);
}

//___________________________________________________________________
Int_t CbmTrdParAsic::QueryChannel(Int_t chAddress) const
{
  Int_t ich(0);
  for(std::vector<Int_t>::const_iterator it=fChannelAddresses.begin(); it!=fChannelAddresses.end(); it++, ich++){
    if(chAddress == (*it)) return ich;
  }
  return -1;
}

//___________________________________________________________________
void CbmTrdParAsic::SetChannelAddress(Int_t address)
{
  if(QueryChannel(address)>=0){
    LOG(WARNING) << GetName() << "::SetChannelAddress : pad address " << address << " already allocated" << FairLogger::endl;    
    return;
  }
  fChannelAddresses.push_back(address);
}

  //___________________________________________________________________
void CbmTrdParAsic::SetChannelAddresses(std::vector<Int_t> addresses)
{
  Int_t nofChannels = addresses.size();
  if (nofChannels != GetNchannels()){
    LOG(WARNING) << GetName() << "::SetChannelAddresses : input N channels:" << nofChannels << "differs from  definition "<< GetNchannels() << ". Input will be truncated." << FairLogger::endl;
  }
  for (Int_t i = 0; i < TMath::Min(nofChannels, GetNchannels()); i++) SetChannelAddress(addresses[i]);
  addresses.clear();
}

ClassImp(CbmTrdParAsic)
