#include "CbmTrdParSpadic.h"

Double_t CbmTrdParSpadic::fgSizeX = 6.0;
Double_t CbmTrdParSpadic::fgSizeY = 3.0;
Double_t CbmTrdParSpadic::fgSizeZ = 0.5;

//___________________________________________________________________
CbmTrdParSpadic::CbmTrdParSpadic(Int_t address, Int_t FebGrouping, 
           Double_t x, Double_t y, Double_t z)
  : CbmTrdParAsic(address, FebGrouping, x, y, z)
{

}


ClassImp(CbmTrdParSpadic)
