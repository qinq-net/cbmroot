#include "CbmTrdParFasp.h"

#include <FairParamList.h>
#include <TArrayI.h>

Double_t CbmTrdParFasp::fgSizeX = 2.;
Double_t CbmTrdParFasp::fgSizeY = 2.;
Double_t CbmTrdParFasp::fgSizeZ = 0.5;
//___________________________________________________________________
CbmTrdParFasp::CbmTrdParFasp(Int_t address, Int_t FebGrouping, 
           Double_t x, Double_t y, Double_t z)
  : CbmTrdParAsic(address, FebGrouping, x, y, z)
{

}

//___________________________________________________________________
const CbmTrdParFaspChannel* CbmTrdParFasp::GetChannel(Int_t address, UChar_t pairing) const
{
  Int_t id=QueryChannel(2*address+pairing);
  if(id<0) return NULL;
  return &fCalib[id];
}

//___________________________________________________________________
void CbmTrdParFasp::LoadParams(FairParamList* l)
{
  TArrayI value(NFASPCH);
  if (l->fill(Form("%dCHS", fAddress), &value)){
    for(Int_t ich(0); ich<NFASPCH; ich++){ 
      Int_t pair=ich%2;
      SetChannelAddress(2*value[ich]+pair);
      fCalib[ich].SetPairing(pair);
    }
  }
  if (l->fill(Form("%dPUT", fAddress), &value))
    for(Int_t ich(0); ich<NFASPCH; ich++) fCalib[ich].fPileUpTime=value[ich];
  if (l->fill(Form("%dTHR", fAddress), &value))
    for(Int_t ich(0); ich<NFASPCH; ich++) fCalib[ich].fThreshold=value[ich];
  if (l->fill(Form("%dMDS", fAddress), &value))
    for(Int_t ich(0); ich<NFASPCH; ich++) fCalib[ich].fMinDelaySignal=value[ich];
}

//___________________________________________________________________
void CbmTrdParFasp::Print(Option_t *opt) const
{  
  CbmTrdParAsic::Print("TrdParFasp");
  printf("  Nchannels[%2d]\n", (Int_t)fChannelAddresses.size());
  for(Int_t ich(0); ich<TMath::Min((Int_t)GetNchannels(), (Int_t)fChannelAddresses.size()); ich++){
    printf("  %2d pad_addr[%4d]", ich, GetChannelAddress(ich)); 
    fCalib[ich].Print(opt);
  }
}

//___________________________________________________________________
CbmTrdParFaspChannel::CbmTrdParFaspChannel(Int_t pup, Int_t ft, Int_t thr, Int_t mds, Float_t mdp)
  : TObject()
  ,fPileUpTime(pup)
  ,fFlatTop(ft)
  ,fConfig(0)
  ,fThreshold(thr)
  ,fMinDelaySignal(mds)
  ,fMinDelayParam(mdp)
{
    
}

//___________________________________________________________________
void CbmTrdParFaspChannel::Print(Option_t *opt) const
{  
    printf("[%c]; CALIB { PUT[ns]=%3d FT[clk]=%2d THR[ADC]=%4d MDS[ADC]=%4d }\n", 
         (HasPairing(kTRUE)?'R':'T'), fPileUpTime, fFlatTop, fThreshold, fMinDelaySignal); 
}

ClassImp(CbmTrdParFasp)
ClassImp(CbmTrdParFaspChannel)
