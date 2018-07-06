#ifndef CBMTRDPARSPADIC_H
#define CBMTRDPARSPADIC_H

#define NSPADICCH 32
#include "CbmTrdParAsic.h"


/** \brief Definition of SPADIC parameters **/
class CbmTrdParSpadic : public CbmTrdParAsic
{
public:
  CbmTrdParSpadic(Int_t address=0, Int_t FebGrouping=-1, 
           Double_t x=0, Double_t y=0, Double_t z=0);
  virtual ~CbmTrdParSpadic() {;}
  
  virtual Int_t GetNchannels() const { return NSPADICCH;};
  
  virtual Double_t GetSizeX() const { return fgSizeX; }
  virtual Double_t GetSizeY() const { return fgSizeY; }
  virtual Double_t GetSizeZ() const { return fgSizeZ; }

private:
  static Double_t  fgSizeX;    ///< SPADIC half size in x [cm]
  static Double_t  fgSizeY;    ///< SPADIC half size in y [cm]
  static Double_t  fgSizeZ;    ///< SPADIC half size in z [cm]

  ClassDef(CbmTrdParSpadic, 1)  // Definition of SPADIC ASIC parameters
};

#endif
