#include "CbmMuchSector.h"
#include "CbmMuchPad.h"
#include <vector>

using std::vector;

// -----   Default constructor   -------------------------------------------
CbmMuchSector::CbmMuchSector():
    fAddress(0),
    fNChannels(0),
    fPads()
{
}
// -------------------------------------------------------------------------

// -----  Standard constructor  --------------------------------------------
CbmMuchSector::CbmMuchSector(UInt_t modAddress, UInt_t index, Int_t nChannels):  
    fAddress(CbmMuchAddress::SetElementId(modAddress,kMuchSector,index)),
    fNChannels(nChannels),
    fPads()
{
  fPads.resize(nChannels);
}
// -------------------------------------------------------------------------

CbmMuchPad* CbmMuchSector::GetPadByChannelIndex(Int_t iChannel) const { 
//  LOG(DEBUG) << "iChannel=" << iChannel << " fPads.size()=" << fPads.size()
//             << " fNChannels=" << fNChannels;
  if (iChannel>=static_cast<Int_t>(fPads.size()) || iChannel<0) {
    LOG(ERROR) << "iChannel=" << iChannel << " fPads.size()=" << fPads.size();
    LOG(ERROR) << "  station index=" << CbmMuchAddress::GetStationIndex(fAddress);
    LOG(ERROR) << "    layer index=" << CbmMuchAddress::GetLayerIndex(fAddress);
    LOG(ERROR) << "   module index=" << CbmMuchAddress::GetModuleIndex(fAddress);
    LOG(ERROR) << "   sector index=" << CbmMuchAddress::GetSectorIndex(fAddress);
    return NULL;
  }
  return fPads[iChannel]; 
}



//// -------------------------------------------------------------------------
//void CbmMuchSector::DrawPads(){
//  for(Int_t iChannel = 0; iChannel < fNChannels; iChannel++){
//     CbmMuchPad* pad = fPads[iChannel];
////     pad->Draw("f");
////     pad->Draw();
//  }
//}
//// -------------------------------------------------------------------------
ClassImp(CbmMuchSector)
