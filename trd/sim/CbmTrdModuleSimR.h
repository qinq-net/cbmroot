#ifndef CBMTRDMODULESIMR_H
#define CBMTRDMODULESIMR_H

#include "CbmTrdModuleSim.h"

class TRandom3;
class TFile;
class TH2D;
class TH1I;
class TH1D;
class CbmTrdParSetAsic;

using std::pair;
using std::map;
using std::vector;
using std::tuple;

/**
  * \brief Simulation module implementation for rectangular pad geometry 
  **/
class CbmTrdModuleSimR : public CbmTrdModuleSim
{
public:
  CbmTrdModuleSimR(Int_t mod, Int_t ly, Int_t rot);
  virtual ~CbmTrdModuleSimR() {;}
  void      GetCounters(Int_t &nEl, Int_t &nLattice, Int_t &nOverThr) const 
              {nEl=nofElectrons; nLattice=nofLatticeHits; nOverThr=nofPointsAboveThreshold;}
  Int_t     FlushBuffer(ULong64_t time=0);
  Bool_t    MakeDigi(CbmTrdPoint *p, Double_t time, Bool_t TR);
  Bool_t    MakeRaw(/*CbmTrdPoint *p*/)   { return kTRUE;}

  //seter functions
  void      SetAsicPar(CbmTrdParSetAsic *p=NULL);
  void      SetNCluster(Int_t nCluster)   { fnClusterConst = nCluster;}
  void      SetNoiseLevel(Double_t sigma_keV);
  void      SetDistributionPoints(Int_t points);
  void      SetSpadicResponse(Double_t calibration, Double_t tau, Double_t calreco);
  void      SetPulsePars(Int_t mode);
  void      SetPulseMode(Bool_t pulsed);
  void      SetTriggerThreshold(Double_t minCharge) { fMinimumChargeTH = minCharge;} 
  void      SetPadPlaneScanArea(Int_t column, Int_t row);
  void      ResetCounters() {nofElectrons=0; nofLatticeHits=0; nofPointsAboveThreshold=0;}
  
private:
  CbmTrdModuleSimR& operator=(const CbmTrdModuleSimR&);
  CbmTrdModuleSimR(const CbmTrdModuleSimR&);

  //pulsed mode
  void      AddDigitoPulseBuffer(Int_t address, Double_t reldrift,Double_t charge, Double_t chargeTR, Double_t time, Int_t trigger,Int_t epoints, Int_t ipoint,Bool_t finish);
  vector<Int_t>   MakePulse(Double_t charge,vector<Int_t> pulse);
  void      AddToPulse(Int_t address, Double_t charge,Double_t reldrift,vector<Int_t> pulse);
  void      CheckMulti(Int_t address, vector<Int_t> pulse);
  Int_t     CheckTrigger(vector<Int_t> pulse);
  Double_t  CalcResponse(Double_t t);
  void      ProcessPulseBuffer(Int_t address, Bool_t FNcall, Bool_t MultiCall, Bool_t down, Bool_t up);
  Int_t     GetMultiBin(vector<Int_t> pulse);
  
  //vintage EB
  void      AddDigi(Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t trigger);

  //non pulsed TB mode
  void      AddDigitoBuffer(Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t trigger);
  void      ProcessBuffer(Int_t address);
  
  //Buffer managment
  void     CheckBuffer(Bool_t EB);
  void     CleanUp(Bool_t EB);

  //general tools
  Double_t  DistributeCharge(Double_t pointin[3],Double_t pointout[3],Double_t delta[3],Double_t pos[3],Int_t ipoints);
  Double_t  AddDrifttime(Double_t x);
  Double_t  AddNoise(Double_t charge);
  Double_t  GetStep(Double_t gamma, Double_t dist, Int_t roll);
  Double_t  GetBetheBloch(Double_t gamma);
  Int_t     AddNoiseADC();
  Int_t     AddCrosstalk(Double_t address,Int_t i, Int_t sec,Int_t row,Int_t col,Int_t ncols);
  Double_t  CalcPRF(Double_t x, Double_t W, Double_t h);
  void      CheckTime(Int_t address);
  void      NoiseTime(ULong64_t eventTime);

  //general MC data usage - distributing MC charge over the pad plane
  void      ScanPadPlane(const Double_t* local_point, Double_t reldrift,Double_t clusterELoss, Double_t clusterELossTR, Int_t epoints,Int_t ipoint);


  //spadic response parameters
  Double_t  fCalibration;   // calibrating pulse height to MIP
  Double_t  fEReco;         // calibrating energy reconstruction to the amount and position of used samples of the pulse
  Double_t  fTau;           // peaking time of the pulse
  Double_t  fTriggerSlope;  // trigger setting of the pulse
  Int_t  fRecoMode;         // mode for reconstruction samples

  //general globals
  Double_t  fSigma_noise_keV;
  TRandom3* fRandom;
  Double_t  fMinimumChargeTH;
  Double_t  fCurrentTime;
  Double_t  fAddress;
  Double_t  fLastEventTime;
  Double_t  fEventTime;
  Double_t  fLastTime;
  Double_t  fCollectTime;
  Double_t  fCrosstalkLevel;

  Int_t     fepoints;
  Int_t     fnClusterConst;
  Int_t     fnScanRowConst;
  Int_t     fnScanColConst;
  Int_t     fAdcNoise;
  Int_t     fDistributionMode;
  Int_t     fLastPoint;
  Int_t     fLastEvent;
  Int_t     frecostart;
  Int_t     frecostop;
  
  Bool_t    fPulseSwitch;
  Bool_t    fAddCrosstalk;
  Bool_t    fPrintPulse;
  Bool_t    fTimeShift;
  Bool_t    fClipping;

  
    //counters
  Int_t nofElectrons;
  Int_t nofLatticeHits;
  Int_t nofPointsAboveThreshold;
  
  map<Int_t, vector<pair<CbmTrdDigi*, CbmMatch*>>>          fAnalogBuffer;
  map<Int_t, pair<vector<Int_t>, CbmMatch*>>                fPulseBuffer;
  map<Int_t, pair<Double_t,Int_t>>                          fMultiBuffer;
  map<Int_t, Double_t>                                      fTimeBuffer;
  map<Int_t, vector<Int_t>>                                 fMCBuffer;
  
  ClassDef(CbmTrdModuleSimR, 1) // Simulation module implementation for rectangular pad geometry
};

#endif
