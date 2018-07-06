#ifndef CBMTRDPARMODGAS_H
#define CBMTRDPARMODGAS_H

#include "CbmTrdParMod.h"
#include <TString.h>

class TH2F;
class TDirectory;

/** \brief Definition of gas parameters for one TRD module **/
class CbmTrdParModGas : public CbmTrdParMod
{
public:
  enum CbmTrdParModGasDef{
    kDetType = 0      /** Detector type (GSI=0, MB=1) */
   ,kNobleGasType     /** Type of noble gas (Xe=0, Ar=1)*/
   ,kPID              /** PID Method (ANN=0, Likelihood=1)*/
  };
  CbmTrdParModGas(const char* title = "TRD gas properties definition");
  virtual ~CbmTrdParModGas();
  
  Int_t     GetDetType()      const {return TESTBIT(fConfig, kDetType);}
  Double_t  GetCO2()          const {return fPercentCO2;}
  TString   GetFileName()     const {return fFileNamePID;}
  Double_t  GetNobleGas()     const {return 1.-fPercentCO2;}
  Int_t     GetNobleGasType() const {return TESTBIT(fConfig, kNobleGasType)?2:1;}
  Int_t     GetPidType()      const {return TESTBIT(fConfig, kPID);}
  void      GetElectricPotential(Int_t &ua, Int_t &ud) const { ua=fUa; ud = fUd;}
  
  /**
   * \brief Get prompt signal for track segment in the y-z plane
   * \param y0 entry point on y in amplification cell coordinates (+- anode pitch/2)
   * \param z0 entry point on z (+- chmb_height/2)
   * \param dzdy Track angle in the y-z plane
   * \param dy track span on y
   * \sa CbmTrdModuleSimT::ScanPadPlane() CbmTrdModuleSimT::AddDigi()
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
  Double_t  ScanDriftTime(Double_t y0, Double_t z0, Double_t dzdy, Double_t dy) const;
  
  /**
   * \brief Load drift map for current settings from repository
   * \param[in] hm histogram obtained as interpolation
   * \param[in] d ROOT directory of CbmRoot
   * \sa CbmTrdParSetGas::ScanDriftMap()
   **/
  void      SetDriftMap(TH2F *hm, TDirectory *d);
  void      SetDw(Double_t dw)            { fDw = dw;}
  void      SetGasThick(Double_t sz)      { fGasThick = sz;}
  void      SetDetType(Int_t gsi=0)       { gsi?SETBIT(fConfig, kDetType):CLRBIT(fConfig, kDetType);}
  void      SetCO2(Double_t p)            { fPercentCO2=p;}
  void      SetFileName(const Char_t *fn) { fFileNamePID = fn;}
  void      SetNobleGas(Double_t p)       { fPercentCO2 = 1 - p;}
  void      SetNobleGasType(Int_t ar=1)   { ar?SETBIT(fConfig, kNobleGasType):CLRBIT(fConfig, kNobleGasType);}
  void      SetPidType(Int_t like=1)      { like?SETBIT(fConfig, kPID):CLRBIT(fConfig, kPID);}
  void      SetElectricPotential(Int_t ua, Int_t ud) { fUa=ua; fUd=ud;}
private:
  CbmTrdParModGas(const CbmTrdParModGas &ref);
  const CbmTrdParModGas& operator=(const CbmTrdParModGas &ref);

  Int_t     fConfig;        ///< config TRD gas see CbmTrdParModGasDef         
  UShort_t  fUa;            ///< anode voltage
  UShort_t  fUd;            ///< drift voltage
  Double_t  fDw;            ///< anode wire pitch
  Double_t  fGasThick;      ///< gas thickness
  Double_t  fPercentCO2;    ///< Percentage of CO2
  TH2F*     fDriftMap;      ///< drift time map for one amplification cell
  TString   fFileNamePID;   ///< filename for PID database
  
  ClassDef(CbmTrdParModGas, 1)  // Definition of gas parameters for one TRD module
};

#endif
