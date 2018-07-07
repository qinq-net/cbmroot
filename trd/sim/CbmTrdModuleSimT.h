#ifndef CBMTRDMODULESIMT_H
#define CBMTRDMODULESIMT_H

#include "CbmTrdModuleSim.h"

class CbmTrdTrianglePRF;
class CbmTrdParSetAsic;
/**
  * \brief Simulation module implementation for triangular pad geometry
  * \author Alex Bercuci <abercuci@niham.nipne.ro> 
  **/
class CbmTrdModuleSimT : public CbmTrdModuleSim
{
public:
  CbmTrdModuleSimT(Int_t mod, Int_t ly, Int_t rot, 
                  Double_t x, Double_t y, Double_t z, 
                  Double_t dx, Double_t dy, Double_t dz, Bool_t FASP=kTRUE);
  virtual ~CbmTrdModuleSimT();
  
  /**
   * \brief Flush local buffer of digits which can no longer interact with current event
   * \param time current event time or 0 for all
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
  Int_t  FlushBuffer(ULong64_t time);

  /**
   * \brief Steer building of digits for triangular pad geometry
   * \param[in] p MC point to be digitized 
   * \param[in] time Event time 
   * \param[in] TR Request TR generation on top of dEdx  
   * \sa ScanPadPlane()
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
  Bool_t  MakeDigi(CbmTrdPoint *p, Double_t time, Bool_t TR);
  Bool_t  MakeRaw(/*CbmTrdPoint *p*/)  {return kTRUE;}

  void    SetAsic(Bool_t set)     {;}
  void    SetAsicPar(CbmTrdParSetAsic *p=NULL);

private:
  CbmTrdModuleSimT(const CbmTrdModuleSimT& ref);
  const CbmTrdModuleSimT& operator=(const CbmTrdModuleSimT& ref);
  
  /**
   * \brief Build digits for the triangular pad geometry
   * \param point Position of hit on the anode wire in c.s.
   * \param dx    Track projection length on the closest anode wire [cm]
   * \param ELoss Fraction of energy due to ionisation [keV]
   * \param ELossTR Fraction of energy due to TR. [keV]
   * \sa CbmTrdTriangle CbmTrdRadiator AddDigi()
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
  Bool_t ScanPadPlane(const Double_t* point, Double_t dx, Double_t ELoss, Double_t ELossTR, Double_t tdrift);
  /**
   * \brief Adding triangular digits to time slice buffer
   * \param pointId The TRD hit in global coordinates beeing processed
   * \param address column/row unique index
   * \param charge Energy deposit in ADC chs for tilt [0] and rectangular [1] coupled pads
   * \param time   time of the CS for 80MHz clocks
   * \param fTR    TR fraction of total energy
   * \sa FlushBuffer()
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
  void    AddDigi(Int_t address, Double_t *charge, Double_t time, Double_t fTR=0.);
  /**
   * \brief Print current buffer content
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
  void   DumpBuffer() const;

  CbmTrdTrianglePRF  *fTriangleBinning;    ///< Integration of PRF on triangular pad-plane geometry
  
  ClassDef(CbmTrdModuleSimT, 1)
};

#endif