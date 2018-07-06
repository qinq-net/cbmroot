#ifndef CBMTRDMODULESIMR_H
#define CBMTRDMODULESIMR_H

#include "CbmTrdModuleSim.h"

class TRandom3;
class CbmTrdParSetAsic;
/**
  * \brief Simulation module implementation for rectangular pad geometry 
  **/
class CbmTrdModuleSimR : public CbmTrdModuleSim
{
public:
  CbmTrdModuleSimR(Int_t mod, Int_t ly, Int_t rot, 
                  Double_t x, Double_t y, Double_t z, 
                  Double_t dx, Double_t dy, Double_t dz);
  virtual ~CbmTrdModuleSimR() {;}
  void      GetCounters(Int_t &nEl, Int_t &nLattice, Int_t &nOverThr) const 
              {nEl=nofElectrons; nLattice=nofLatticeHits; nOverThr=nofPointsAboveThreshold;}
  Int_t     FlushBuffer(ULong64_t time)   { return 0;}
  Bool_t    MakeDigi(CbmTrdPoint *p, Double_t time, Bool_t TR);
  Bool_t    MakeRaw(/*CbmTrdPoint *p*/)   { return kTRUE;}

  void      SetAsicPar(CbmTrdParSetAsic *p=NULL);
  void      SetNCluster(Int_t nCluster)   { fnClusterConst = nCluster;}
  void      SetNoiseLevel(Double_t sigma_keV);
  void      SetTriggerThreshold(Double_t minCharge) { fMinimumChargeTH = minCharge;} //only for debugging. has no impact on the output!!!!
  void      SetPadPlaneScanArea(Int_t column, Int_t row);
  void      ResetCounters() {nofElectrons=0; nofLatticeHits=0; nofPointsAboveThreshold=0;}
  
private:
  CbmTrdModuleSimR& operator=(const CbmTrdModuleSimR&);
  CbmTrdModuleSimR(const CbmTrdModuleSimR&);
  
  void      AddDigi(Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t trigger);
  void      AddDigitoBuffer(Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t trigger);
  Double_t  AddDrifttime(Double_t x);
  Double_t  AddNoise(Double_t charge);
  Double_t  CalcPRF(Double_t x, Double_t W, Double_t h);
  Double_t  CheckTime(Int_t address);
  void      NoiseTime(ULong64_t eventTime);
  void      ProcessBuffer(Int_t address);
  void      ScanPadPlane(const Double_t* local_point, Double_t clusterELoss, Double_t clusterELossTR, Int_t epoints,Int_t ipoints);

  Double_t  fSigma_noise_keV;
  TRandom3* fNoise;
  Double_t  fMinimumChargeTH;
  Double_t  fCurrentTime;
  Double_t  fAddress;
  Double_t  fLastEventTime;
  Double_t  fCollectTime;

  Int_t     fnClusterConst;
  Int_t     fnScanRowConst;
  Int_t     fnScanColConst;
  Int_t     fdtlow;
  Int_t     fdthigh;
  
  //counters
  Int_t nofElectrons;
  Int_t nofLatticeHits;
  Int_t nofPointsAboveThreshold;
  
  std::map<Int_t, std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>>         fAnalogBuffer;
  //std::map<Int_t, std::vector<std::pair<Double_t,Double_t>>>              fChargeBuffer;
  std::map<Int_t, Double_t>                                               fTimeBuffer;
  
  ClassDef(CbmTrdModuleSimR, 1) // Simulation module implementation for rectangular pad geometry
};

#endif
