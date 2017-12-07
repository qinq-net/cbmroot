/**
 * \file CbmTrdDigi.h
 * \author Andrey Lebedev <andrey.lebedev@gsi.de>
 * \date 03.05.2013
 **/

#ifndef CBMTRDDIGI_H
#define CBMTRDDIGI_H 1

#include "CbmDigi.h"
#include "CbmTrdAddress.h"

#include <string>
#include <vector>

class CbmTrdDigi : public CbmDigi
{
 public:
  enum CbmTrdDigiDef{
    kTriang = BIT(15)     ///< set type of pads on which the digit represent
  };
  /**
   * \brief Default constructor.
   */
  CbmTrdDigi();

  /**
   * \brief Constructor with assignment.
   * \param[in] address Unique channel address.
   * \param[in] charge Charge.
   * \param[in] time Absolute time [ns].
   */
  CbmTrdDigi(Int_t address, Double_t charge, Double_t time, Bool_t padType=kFALSE);
  CbmTrdDigi(Int_t address, Double_t charge, Double_t time, Int_t triggerType, Int_t infoType, Int_t stopType);
  CbmTrdDigi(Int_t address, Double_t fullTime, Int_t triggerType, Int_t infoType, Int_t stopType, Int_t nrSamples, Float_t* samples);
  /**
   * \brief Destructor.
   */
  ~CbmTrdDigi();

  /**
   * \brief Inherited from CbmDigi.
   */
  Int_t GetAddress() const { return fAddress; };

  /**
   * \brief Inherited from CbmDigi.
   */
  Double_t GetCharge(Int_t up=1) const { return up?fCharge:fChargeT; }
  Double_t GetChargeTR(Int_t up=1) const { return up?fChargeTR:fChargeTTR; }

  /**
   * \brief Inherited from CbmDigi.
   */
  Int_t GetSystemId() const { return CbmTrdAddress::GetSystemId(fAddress); }

  /**
   * \brief Inherited from CbmDigi.
   */
  Double_t GetTime() const { return fTime; }

  // Accessors for CbmSpadicRawMessage Testbeam input
  Int_t GetNrSamples() { return fSamples.size(); }
  Float_t* GetSamples() { return fSamples.data(); }
  Int_t GetTriggerType() { return fTriggerType; }
  Int_t GetStopType() { return fStopType; }
  Int_t GetInfoType() { return fInfoType; }
  Bool_t  IsTriangular() const {return TestBit(kTriang);}
  //===============================================

  /** Accessors **/
  void SetAddress(Int_t address) { fAddress = address; }
  void SetCharge(Double_t charge, Int_t up=1) { if(up) fCharge = charge; else  fChargeT = charge; }
  void SetChargeTR(Double_t charge, Int_t up=1) { if(up) fChargeTR = charge;  else  fChargeTTR = charge; }
  void SetTime(Double_t time) { fTime = time; }
  void SetTriangular(Bool_t set=kTRUE) {SetBit(kTriang, set);}
  void SetTriggerType(Int_t triggerType) {fTriggerType = triggerType; }
  void SetInfoType(Int_t infoType) {fInfoType = infoType; }
  void SetStopType(Int_t stopType) {fStopType = stopType; }
  void SetPulseShape(Float_t pulse[45]);
  /** Modifiers **/
  void AddCharge(Double_t charge, Int_t up=1) { if(up) fCharge += charge; else fChargeT += charge; }
  void AddChargeTR(Double_t charge, Int_t up=1) { if(up) fChargeTR += charge;else fChargeTTR += charge; }
  /** Needed for TClonesArray sorting **/
  Int_t Compare(const TObject*) const;
  Bool_t IsSortable() const { return kTRUE; }

  std::string ToString() const;
  
 private:
  Int_t fn_FNR_Triggers;
  Int_t fAddress; ///< Unique channel address
  Double_t fCharge; ///< Charge
  Double_t fChargeTR; ///< Charge TR
  Double_t fChargeT; ///< Charge for tilt coupled pads
  Double_t fChargeTTR; ///< Charge TR for tilt coupled pads
  Double_t fTime; ///< Absolute time [ns]
  Int_t fTriggerType;
  Int_t fInfoType;
  Int_t fStopType;
  Int_t fBufferOverflowCount; 
  std::vector<Float_t> fSamples;

  ClassDef(CbmTrdDigi, 7);
};

#endif
