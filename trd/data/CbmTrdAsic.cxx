#include "CbmTrdAsic.h"

#include "CbmTrdPoint.h"
#include "CbmTrdAddress.h"
#include "FairLogger.h"

#include "TGeoManager.h"
#include "TMath.h"

#include <iomanip>

CbmTrdAsic::CbmTrdAsic() 
  : CbmTrdAsic(0, 0, -1, 0., 0., 0., 0., 0., 0.)
{
}

CbmTrdAsic::CbmTrdAsic(
		       Int_t address, Int_t nofChannels, Int_t FebGrouping, 
		       Double_t x, Double_t y, Double_t z,
		       Double_t sizex, Double_t sizey, Double_t sizez)
  : TNamed(),
    fChannelAddresses(),
    fAsicAddress(address),
    fX(x),
    fY(y),
    fZ(z),
    fSizeX(sizex),
    fSizeY(sizey),
    fSizeZ(sizez),
    fnofChannels(nofChannels),
    fFebGrouping(FebGrouping)
{

}

CbmTrdAsic::~CbmTrdAsic() 
{
}

void  CbmTrdAsic::Print(Option_t *opt) const
{
/** 
  Dump formated parameters for this ASIC board
*/
  printf("        CbmTrdAsic @ %5d chs[%2d] FEBs[%d]\n"
         "            center   [%7.2f %7.2f %7.2f]\n"
         "            size     [%7.2f %7.2f %7.2f]\n",
    fAsicAddress, fnofChannels, fFebGrouping,
    fX, fY, fZ, fSizeX, fSizeY, fSizeZ);
  
  if(strcmp(opt, "addr")!=0) return;
  printf("            address  ");
  for(Int_t ia(0); ia<fChannelAddresses.size(); ia++){ 
    if(ia%8==0) printf("\n%20s", "");
    printf(" %10d", fChannelAddresses.at(ia));
  } printf("\n");
}

void CbmTrdAsic::SetChannelAddresses(std::vector<Int_t> addresses){
  Int_t nofChannels = addresses.size();
  if (nofChannels != fnofChannels){
    LOG(WARNING) << "CbmTrdAsic::SetChannelAddresses previous nof channels:" << fnofChannels << " will be set to:" <<  nofChannels << FairLogger::endl;
    fnofChannels = nofChannels;
  }
  for (Int_t i = 0; i < nofChannels; i++)
    fChannelAddresses.push_back(addresses[i]);
  addresses.clear();
}

ClassImp(CbmTrdAsic)
