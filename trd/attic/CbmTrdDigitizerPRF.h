#ifndef CBMTRDDIGITIZERPRF_H
#define CBMTRDDIGITIZERPRF_H

#include "CbmDigitize.h"
#include "TRandom3.h"
#include <map>
#include <vector>
#include <tuple>

class CbmTrdPoint;
class CbmTrdDigi;
class CbmTrdDigiPar;
class CbmTrdModule;
class CbmTrdRadiator;
class CbmMatch;
class TClonesArray;
class CbmTrdTriangle;

class CbmTrdDigitizerPRF : public CbmDigitize {
 public:

  /**
   * \brief Standard constructor.
   **/
  CbmTrdDigitizerPRF(CbmTrdRadiator* radiator = nullptr);

  /**
   * \brief Destructor.
   **/
  virtual ~CbmTrdDigitizerPRF();

  /**
   * \brief Inherited from FairTask.
   **/
  virtual InitStatus Init();

  /**
   * \brief Inherited from FairTask.
   **/
  virtual void SetParContainers();

  /**
   * \brief Inherited from FairTask.
   **/
  virtual void Exec(Option_t * option);

  void SetTriangularPads(Bool_t triangles);
  void SetNCluster(Int_t nCluster);
  void SetNoiseLevel(Double_t sigma_keV);
  void SetTriggerThreshold(Double_t minCharge); 
  void SetPadPlaneScanArea(Int_t column, Int_t row);
  void SetCbmLinkWeightDistance(Bool_t dist);

  /** @brief Reset output arrays **/
  virtual void ResetArrays();


  /** @brief Write data to output
   ** @param digi Pointer to digi object to be written
   **/
  virtual void WriteDigi(CbmDigi* digi);


 private:

  CbmTrdDigitizerPRF& operator=(const CbmTrdDigitizerPRF&);
  CbmTrdDigitizerPRF(const CbmTrdDigitizerPRF&);

  //calculation of the PRF for the distribution of the MC charge over the pad plane
  Double_t CalcPRF(Double_t x, Double_t W, Double_t h);

  //looping over the different channels and calculating the charge distribution on each pad 
  void ScanPadPlane(const Double_t* local_point, Double_t clusterELoss, Double_t clusterELossTR);

  void ScanPadPlaneTriangle(const Double_t* local_point, Double_t clusterELoss, Double_t clusterELossTR);

  Double_t TriangleIntegration(Bool_t even, Double_t displacement_x, Double_t W, Double_t displacement_y, Double_t H, Double_t h);

  Double_t TriangleIteration(Bool_t even, Int_t step, Double_t displacement_x, Double_t W, Double_t displacement_y, Double_t H, Double_t h);

  //calculation of the charge positions
  void SplitTrackPath(const CbmTrdPoint* point, Double_t ELoss, Double_t ELossTR);

  //adding the digis to the TClonesAray in event based mode
  void AddDigi(Int_t pointId, Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t up=1);

  //adding the digis to a map, where they are stored until the collection time has passed and following charges can not interfer any more
  void AddDigitoBuffer(Int_t pointId, Int_t address, Double_t charge, Double_t chargeTR, Double_t time, Int_t up=1);

  //processing the stored digis to the daq class
  void ProcessBuffer(Int_t address);
  
  //adding a gaussian distributed noise value to the charge in the digi 
  Double_t AddNoise(Double_t charge);

  //checking the time between the last digi in the channel and actual one; processing the buffer in the channel, if enough time is in between
  void     CheckTime(Int_t address);

  //random time in between events for the addition of noise digis
  void     NoiseTime();

  //adding drifttime based on values of a Garfield simulation; Drifttime is position dependant so it is random until the charge is distributed inside the gas volume
  Double_t AddDrifttime(Double_t x);


  /**
   * \brief Build digits for the triangular pad geometry
   * \param point Position of hit on the anode wire in c.s.
   * \param dx    Track projection length on the closest anode wire [cm]
   * \param ELoss Fraction of energy due to ionisation [keV]
   * \param ELossTR Fraction of energy due to TR. [keV]
   * \sa CbmTrdTriangle CbmTrdRadiator AddDigi()
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
  void ScanPadPlaneTriangleAB(const Double_t* point, Double_t dx, Double_t ELoss, Double_t ELossTR);
  /**
   * \brief Steer building of digits for triangular pad geometry
   * \param point The TRD hit in global coordinates beeing processed
   * \param ELoss Energy deposit due to ionisation
   * \param ELossTR Energy deposit due to TR. (see Radiator parametrization)
   * \sa ScanPadPlaneTriangleAB()
   * \author A.Bercuci <abercuci@niham.nipne.ro>
   **/
  void SplitTrackPathTriang(const CbmTrdPoint* point, Double_t ELoss, Double_t ELossTR);
  
  Bool_t fDebug;
  Bool_t fNoiseDigis;
  Bool_t fTrianglePads;
  Bool_t fCbmLinkWeightDistance;

  Double_t fSigma_noise_keV;
  TRandom3 *fNoise;
  Double_t fMinimumChargeTH;
  Double_t fCurrentTime;
  Double_t fAddress;
  Double_t fLastEventTime;
  Double_t fCollectTime;

  Int_t fnClusterConst;
  Int_t fnScanRowConst;
  Int_t fnScanColConst;
  Int_t fnRow;
  Int_t fnCol;
  Int_t fdtlow;
  Int_t fdthigh;
  Int_t fnoDigis;
  Int_t fpoints;
  Int_t fLayerId;
  Int_t fModuleId;
  Int_t fBufferAddress;
  Int_t fMCPointId;

  TClonesArray* fPoints; //! Trd MC points
  TClonesArray* fDigis; //! TRD digis
  TClonesArray* fDigiMatches; //! Corresponding MCPoints to TRD digis
  TClonesArray* fMCTracks;  //! MC Track information

  CbmTrdDigiPar* fDigiPar;    //!
  CbmTrdModule* fModuleInfo; //!
  CbmTrdRadiator* fRadiator;  //!
  /// pointer to pad plane binning for triangular pad integration
  CbmTrdTriangle  *fTriangleBinning;    //!

  std::map<Int_t, std::pair<CbmTrdDigi*, CbmMatch*>>                      fDigiMap; // Temporary storage for digis.
  std::map<Int_t, std::vector<std::pair<CbmTrdDigi*, CbmMatch*>>>         fAnalogBuffer;
  std::map<Int_t, Double_t>                                               fTimeBuffer;

  ClassDef(CbmTrdDigitizerPRF, 4);
};
#endif // CBMTRDDIGITIZER_PRF_H
