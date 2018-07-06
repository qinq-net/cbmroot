#ifndef CBMTRDPARFASP_H
#define CBMTRDPARFASP_H

#define NFASPCH 16
#include <TObject.h>
#include "CbmTrdParAsic.h"

class FairParamList;
/** \brief Definition of FASP channel calibration container **/
class CbmTrdParFaspChannel : public TObject
{
friend class CbmTrdParFasp;   
public:
  enum CbmTrdParFaspChannelDef{
    kPair=0     //< pad pairing type definition see SetPairing()
  };
  /** \brief   Parametrization of a FASP channel based on CADENCE simulations from 12.01.2018 and 
   * parabolic parametrization of dt(signal). 
   */
  CbmTrdParFaspChannel(Int_t pup=300, Int_t ft=14, Int_t thr=600, Int_t smin=2586, Float_t dtPar= 4.181e-6);

  Int_t     GetFlatTop() const        { return fFlatTop;}
  Int_t     GetMinDelaySignal() const { return fMinDelaySignal;}
  Float_t   GetMinDelayParam() const  { return fMinDelayParam;}
  Int_t     GetPileUpTime() const     { return fPileUpTime;}
  Int_t     GetThreshold() const      { return fThreshold;}
    
  /** \brief Query pad pairing type.
   *\param[in] rect if rect=kTRUE rectangular pairing; tilt otherwise 
   */
  Bool_t    HasPairing(Bool_t rect) const { return (rect&&TESTBIT(fConfig, kPair))||(!rect&&!TESTBIT(fConfig, kPair));}
  void      Print(Option_t *opt="") const;
  /** \brief Specify pad pairing type.
   *\param[in] rect if rect=kTRUE rectangular pairing; tilt otherwise 
   */
  void      SetPairing(Bool_t rect)   { rect?SETBIT(fConfig, kPair):CLRBIT(fConfig, kPair);}
protected:  
  UShort_t  fPileUpTime;      ///< Signal formation time in ns
  UChar_t   fFlatTop;         ///< Length of Flat-Top in clocks
  UChar_t   fConfig;          ///< configuration bit map
  
  // threshold
  UShort_t  fThreshold;       ///< Threshold in ADC units

  // timming
  UShort_t  fMinDelaySignal;  ///< Signal in ADC for minimum delay i.e. fPileUpTime 
  Float_t   fMinDelayParam;   ///< Factor of parabolic dependence dt=fdt*(s-s0)^2 to calculate trigger

  ClassDef(CbmTrdParFaspChannel, 1)  // Definition of FASP channel calibration container
};

/** \brief Definition of FASP parameters **/
class CbmTrdParFasp : public CbmTrdParAsic
{
public:
  CbmTrdParFasp(Int_t address=0, Int_t FebGrouping=-1, 
           Double_t x=0, Double_t y=0, Double_t z=0);
  virtual ~CbmTrdParFasp() {;}
  const CbmTrdParFaspChannel* GetChannel(Int_t pad_address, UChar_t pair) const;
  virtual Int_t GetNchannels() const { return NFASPCH;};
  
  Int_t           GetChannelAddress(Int_t ich) const    { return ((ich<0||ich>=GetNchannels())?0:0.5*fChannelAddresses[ich]); }
  Double_t        GetSizeX() const { return fgSizeX; }
  Double_t        GetSizeY() const { return fgSizeY; }
  Double_t        GetSizeZ() const { return fgSizeZ; }
  virtual void    LoadParams(FairParamList* l);
  virtual void    Print(Option_t *opt="") const;

private:
  static Double_t  fgSizeX;    ///< FASP half size in x [cm]
  static Double_t  fgSizeY;    ///< FASP half size in y [cm]
  static Double_t  fgSizeZ;    ///< FASP half size in z [cm]

  CbmTrdParFaspChannel fCalib[NFASPCH]; ///< calibration map for FASP channels

  ClassDef(CbmTrdParFasp, 1)  // Definition of FASP ASIC parameters
};

#endif
