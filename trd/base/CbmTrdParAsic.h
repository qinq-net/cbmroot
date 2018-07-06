#ifndef CBMTRDPARASIC_H
#define CBMTRDPARASIC_H

#include "CbmTrdParMod.h"
#include <vector>

class FairParamList;
/** \brief Definition of ASIC parameters **/
class CbmTrdParAsic : public CbmTrdParMod
{
public:
  CbmTrdParAsic(Int_t address=0, Int_t FebGrouping=-1, 
           Double_t x=0, Double_t y=0, Double_t z=0);
  virtual ~CbmTrdParAsic() {;}
  
    /** Accessors **/
  virtual Double_t  GetSizeX() const = 0;
  virtual Double_t  GetSizeY() const = 0;
  virtual Double_t  GetX() const                          { return fX; }
  virtual Double_t  GetY() const                          { return fY; }
  virtual Double_t  GetZ() const                          { return fZ; }

  virtual Int_t     GetAddress() const                    { return fAddress; }
  virtual Int_t     GetNchannels() const = 0;
  virtual Int_t     GetFebGrouping() const                { return fFebGrouping; }
  virtual Int_t     GetChannelAddress(Int_t ich) const    { return ((ich<0||ich>=GetNchannels())?0:fChannelAddresses[ich]); }
  virtual std::vector<Int_t> GetChannelAddresses() const  { return fChannelAddresses; }
  virtual void      LoadParams(FairParamList* l) {;}
  virtual void      Print(Option_t *opt="") const;
  /** \brief Query ASIC for specific pad address
   * \param[in] ch pad address within module as provided by CbmTrdModuleAbstract::GetPadAddress()
   * \return channel index within ASIC
   */
  virtual Int_t     QueryChannel(Int_t ch) const;
  virtual void      SetChannelAddress( Int_t address );
  virtual void      SetChannelAddresses( std::vector<Int_t> addresses );
  virtual void      SetFebGrouping(Int_t feb)             { fFebGrouping=feb;}
  virtual void      SetPosition(Double_t x=0, Double_t y=0, Double_t z=0) {fX=x; fY=y; fZ=z;}

protected:
  Int_t     fAddress;  ///< unique ASIC ID
  Double_t  fX;        ///< center of asic in global c.s. [cm]
  Double_t  fY;        ///< center of asic in global c.s. [cm]
  Double_t  fZ;        ///< center of asic in global c.s. [cm]
  Int_t     fFebGrouping; ///< no of ASIC in ROB
  std::vector<Int_t> fChannelAddresses; ///< addresses of individual output channels

  ClassDef(CbmTrdParAsic, 1)  // Definition of common ASIC parameters
};

#endif
